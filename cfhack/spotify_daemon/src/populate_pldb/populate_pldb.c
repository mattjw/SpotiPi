/**
 * Copyright (c) 2006-2010 Spotify Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * This example application shows parts of the playlist and player submodules.
 * It also shows another way of doing synchronization between callbacks and
 * the main thread.
 *
 * This file is part of the libspotify examples suite.
 */

#include <errno.h>
#include <libgen.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <libspotify/api.h>

#include "audio.h"

#include <sqlite3.h>


/*********** DATABASE *********/

/*
 * 
 */
typedef struct playlist{
    int pl_id;
    char* name;
    int no_of_songs;
};

int setupDB(char* dbname){
    const char * *docDir;
    sqlite3 *db;

    char *errMsg;

    if(sqlite3_open(dbname, &db) == SQLITE_OK) {
        printf("Database Opened/Created");
        const char *create_playlist_table = "CREATE TABLE IF NOT EXISTS spotify_playlists('pl_id' INTEGER PRIMARY KEY, 'name' VARCHAR(100), 'no_of_songs' INTEGER)";
        if (sqlite3_exec(db, create_playlist_table, NULL, NULL, &errMsg) == SQLITE_OK) {
            printf("Table Detected/Created\n");
        } else {
            printf("Error creating SQLite Table\n");
            printf("%s", sqlite3_errmsg(&db));
            exit(1);
	 }


 	char* qry = "DELETE FROM spotify_playlists";

	if (sqlite3_exec(db, qry, NULL, NULL, &errMsg) == SQLITE_OK) {
		printf("delete all rows\n");
	}
	else
	{
		printf("error deleting all rows\n");
		printf("%s\n", sqlite3_errmsg(&db));
		exit(1);
	}      
	
	sqlite3_close( db );

    }else{
        printf("Issues Opening DB, DB Not Opened/Created\n");
        printf("%s\n", sqlite3_errmsg(db));
        exit(1);
    }
}

int addPlaylist(sqlite3* db, struct playlist pl)
{
	char *errMsg;

	const char insert[500];
	snprintf(insert, sizeof insert, "%s%s%s%d%s%d%s",
		"INSERT INTO spotify_playlists(name, pl_id, no_of_songs) VALUES('",
		pl.name,
		"', ",
		pl.pl_id,
		", ",
		pl.no_of_songs,
		");");
	printf("%s\n",insert);
	if (sqlite3_exec(db, insert, NULL, NULL, &errMsg) == SQLITE_OK) {
		printf("Playlist added\n");
	}
	else
	{
		printf("Error adding playlists\n");
		printf("%s", sqlite3_errmsg(&db));
	}
}


/*********** SPOTIFY ***********/


/* --- Data --- */
/// The application key is specific to each project, and allows Spotify
/// to produce statistics on how our service is used.
extern const uint8_t g_appkey[];
/// The size of the application key.
extern const size_t g_appkey_size;

/// The output queue for audo data
static audio_fifo_t g_audiofifo;
/// Synchronization mutex for the main thread
static pthread_mutex_t g_notify_mutex;
/// Synchronization condition variable for the main thread
static pthread_cond_t g_notify_cond;
/// Synchronization variable telling the main thread to process events
static int g_notify_do;
/// Non-zero when a track has ended and the jukebox has not yet started a new one
static int g_playback_done;
/// The global session handle
static sp_session *g_sess;
/// Handle to the playlist currently being played
static sp_playlist *g_jukeboxlist;
/// Name of the playlist currently being played
const char *g_listname;
/// Handle to the curren track
static sp_track *g_currenttrack;
/// Index to the next track
static int g_track_index = -1;

/// Index to current playlist (PL id) -MJW
static int g_playlist_id = -1;

/// File path to a sqlite3 db
static char* g_dbfpath = NULL;


/* --------------------------  PLAYLIST CALLBACKS  ------------------------- */
/**
 * Callback from libspotify, saying that a track has been added to a playlist.
 *
 * @param  pl          The playlist handle
 * @param  tracks      An array of track handles
 * @param  num_tracks  The number of tracks in the \c tracks array
 * @param  position    Where the tracks were inserted
 * @param  userdata    The opaque pointer
 */
static void tracks_added(sp_playlist *pl, sp_track * const *tracks,
                         int num_tracks, int position, void *userdata)
{	
	printf("PRT 111b\n");
}

/**
 * Callback from libspotify, saying that a track has been added to a playlist.
 *
 * @param  pl          The playlist handle
 * @param  tracks      An array of track indices
 * @param  num_tracks  The number of tracks in the \c tracks array
 * @param  userdata    The opaque pointer
 */
static void tracks_removed(sp_playlist *pl, const int *tracks,
                           int num_tracks, void *userdata)
{
	printf("PRT 111c\n");
}

/**
 * Callback from libspotify, telling when tracks have been moved around in a playlist.
 *
 * @param  pl            The playlist handle
 * @param  tracks        An array of track indices
 * @param  num_tracks    The number of tracks in the \c tracks array
 * @param  new_position  To where the tracks were moved
 * @param  userdata      The opaque pointer
 */
static void tracks_moved(sp_playlist *pl, const int *tracks,
                         int num_tracks, int new_position, void *userdata)
{
	printf("PRT 111d\n");
}

/**
 * Callback from libspotify. Something renamed the playlist.
 *
 * @param  pl            The playlist handle
 * @param  userdata      The opaque pointer
 */
static void playlist_renamed(sp_playlist *pl, void *userdata)
{
	printf("PRT 111e\n");
}

/**
 * The callbacks we are interested in for individual playlists.
 */
static sp_playlist_callbacks pl_callbacks = {
	.tracks_added = &tracks_added,
	.tracks_removed = &tracks_removed,
	.tracks_moved = &tracks_moved,
	.playlist_renamed = &playlist_renamed,
};


/* --------------------  PLAYLIST CONTAINER CALLBACKS  --------------------- */
/**
 * Callback from libspotify, telling us a playlist was added to the playlist container.
 *
 * We add our playlist callbacks to the newly added playlist.
 *
 * @param  pc            The playlist container handle
 * @param  pl            The playlist handle
 * @param  position      Index of the added playlist
 * @param  userdata      The opaque pointer
 */
static void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
                           int position, void *userdata)
{	
	printf("PRT 111f\n");
}

/**
 * Callback from libspotify, telling us a playlist was removed from the playlist container.
 *
 * This is the place to remove our playlist callbacks.
 *
 * @param  pc            The playlist container handle
 * @param  pl            The playlist handle
 * @param  position      Index of the removed playlist
 * @param  userdata      The opaque pointer
 */
static void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl,
                             int position, void *userdata)
{	
	printf("PRT 111g\n");

	sp_playlist_remove_callbacks(pl, &pl_callbacks, NULL);
}


/**
 * Callback from libspotify, telling us the rootlist is fully synchronized
 * We just print an informational message
 *
 * @param  pc            The playlist container handle
 * @param  userdata      The opaque pointer
 */
static void container_loaded(sp_playlistcontainer *pc, void *userdata)
{	
	printf("PRT 111h\n");

	fprintf(stderr, "jukebox: Rootlist synchronized (%d playlists)\n",
	    sp_playlistcontainer_num_playlists(pc));
}


/**
 * The playlist container callbacks
 */
static sp_playlistcontainer_callbacks pc_callbacks = {
	.playlist_added = &playlist_added,
	.playlist_removed = &playlist_removed,
	.container_loaded = &container_loaded,
};


/* ---------------------------  SESSION CALLBACKS  ------------------------- */
/**
 * This callback is called when an attempt to login has succeeded or failed.
 *
 * @sa sp_session_callbacks#logged_in
 */
static void logged_in(sp_session *sess, sp_error error)
{	
	printf("PRT 111i\n");


	sp_playlistcontainer *pc = sp_session_playlistcontainer(sess);
	int i;

	if (SP_ERROR_OK != error) {
		fprintf(stderr, "jukebox: Login failed: %s\n",
			sp_error_message(error));
		exit(2);
	}

	/* SEARCHING for playlist id from name */

	int num_pls = sp_playlistcontainer_num_playlists(pc);
	printf("jukebox: Looking at %d playlists\n", num_pls );


	/*
	char *pl_names[num_pls]; 
	int pl_ids[num_pls]; 
	// Not using char array of strings. Risk of seg faults. Need more time to handle.
	*/

	setupDB( g_dbfpath );
	//	creates DB file if there is not one
	//	creates table if there is not one
	//	deletes all rows from table

	char* errMsg;
	sqlite3* db = NULL;
	if(sqlite3_open(g_dbfpath, &db) == SQLITE_OK) {
		
		char* qry = "BEGIN TRANSACTION";
		if (sqlite3_exec(db, qry, NULL, NULL, &errMsg) == SQLITE_OK) {
			printf("begin transaction");
		}
		else
		{
			printf("Error setting transaction");
			printf("%s", sqlite3_errmsg(&db));
			exit(1);
		}

		char *pl_name;
		int id;
		for( id=0; id < num_pls; id++ )
		{
			sp_playlist *pl = sp_playlistcontainer_playlist( pc, id );
					
			sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);
			pl_name = sp_playlist_name(pl); 
			int num_tracks = sp_playlist_num_tracks( pl );
			
			if( strlen(pl_name) > 0 )
			{
				printf( "%d :: %s \n", id, pl_name );
				struct playlist test;
				test.name=pl_name;
				test.pl_id=id;
				test.no_of_songs=num_tracks;
				addPlaylist(db, test);
			}
		}

		qry = "COMMIT TRANSACTION";
		if (sqlite3_exec(db, qry, NULL, NULL, &errMsg) == SQLITE_OK) {
			printf("commit transaction");
		}
		else
		{
			printf("Error setting transaction");
			printf("%s", sqlite3_errmsg(&db));
			exit(1);
		}

		sqlite3_close(db);
		printf( "DB write complete." );
	}
	else
	{
		printf("Issues Opening DB, DB Not Opened/Created");
		printf("%s", sqlite3_errmsg(db));
	}

	/* //uncomment this code if segfaults again	
	
	if( (g_playlist_id < 0) || (g_playlist_id >= num_pls) )
	{
		printf( "Playlist id %d is out of range of available playlists\n", g_playlist_id );
		printf( "\tWILL NOT DO ANYTHING\n" );
	} 
	else
	{
		sp_playlist *pl = sp_playlistcontainer_playlist(pc, g_playlist_id);

		sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);
		g_listname = sp_playlist_name(pl); 
		g_jukeboxlist = pl;
	}


	if (!g_jukeboxlist) {
		printf("jukebox: No such playlist. Waiting for one to pop up...\n");
		fflush(stdout);
	}

		

	int pl_num_tracks = sp_playlist_num_tracks( g_jukeboxlist ); 
	g_track_index = rand() % pl_num_tracks; 	
	*/

	printf( "Log in work done. Finishing. \n" );
	exit(0);	
}

/**
 * This callback is called from an internal libspotify thread to ask us to
 * reiterate the main loop.
 *
 * We notify the main thread using a condition variable and a protected variable.
 *
 * @sa sp_session_callbacks#notify_main_thread
 */
static void notify_main_thread(sp_session *sess)
{	
	printf("[PRT 111j -- NOTIFY MAIN THREAD]  ");


	pthread_mutex_lock(&g_notify_mutex);
	g_notify_do = 1;
	pthread_cond_signal(&g_notify_cond);
	pthread_mutex_unlock(&g_notify_mutex);
}

/**
 * This callback is used from libspotify whenever there is PCM data available.
 *
 * @sa sp_session_callbacks#music_delivery
 */
static int music_delivery(sp_session *sess, const sp_audioformat *format,
                          const void *frames, int num_frames)
{	
	printf("[PRT 111 -- music delivery ]\n");
	return -1;
}


/**
 * This callback is used from libspotify when the current track has ended
 *
 * @sa sp_session_callbacks#end_of_track
 */
static void end_of_track(sp_session *sess)
{	
	printf("PRT 111p\n");
}


/**
 * Callback called when libspotify has new metadata available
 *
 * Not used in this example (but available to be able to reuse the session.c file
 * for other examples.)
 *
 * @sa sp_session_callbacks#metadata_updated
 */
static void metadata_updated(sp_session *sess)
{	
	printf("PRT 111q\n");
}

/**
 * Notification that some other connection has started playing on this account.
 * Playback has been stopped.
 *
 * @sa sp_session_callbacks#play_token_lost
 */
static void play_token_lost(sp_session *sess)
{	
	printf("PRT 111r\n");
}

/**
 * The session callbacks
 */
static sp_session_callbacks session_callbacks = {
	.logged_in = &logged_in,
	.notify_main_thread = &notify_main_thread,
	.music_delivery = &music_delivery,
	.metadata_updated = &metadata_updated,
	.play_token_lost = &play_token_lost,
	.log_message = NULL,
	.end_of_track = &end_of_track,
};

/**
 * The session configuration. Note that application_key_size is an external, so
 * we set it in main() instead.
 */
static sp_session_config spconfig = {
	.api_version = SPOTIFY_API_VERSION,
	.cache_location = "tmp",
	.settings_location = "tmp",
	.application_key = g_appkey,
	.application_key_size = 0, // Set in main()
	.user_agent = "spotify-jukebox-example",
	.callbacks = &session_callbacks,
	NULL,
};
/* -------------------------  END SESSION CALLBACKS  ----------------------- */


/**
 * A track has ended. Remove it from the playlist.
 *
 * Called from the main loop when the music_delivery() callback has set g_playback_done.
 */
static void track_ended(void)
{	
	printf("PRT 111s\n");
}

/**
 * Show usage information
 *
 * @param  progname  The program name
 */
static void usage(const char *progname)
{	
	printf("PRT 111t\n");


	fprintf(stderr, "usage: %s -u <username> -p <password> -f <pathtodb>\n", progname);
}

int main(int argc, char **argv)
{
	printf( "STARTING UP~" );
	
	
	sp_session *sp;
	sp_error err;
	int next_timeout = 0;
	const char *username = NULL;
	const char *password = NULL;
	int opt;


	while ((opt = getopt(argc, argv, "u:p:f:")) != EOF) {
		switch (opt) {
		case 'u':
			username = optarg;
			break;

		case 'p':
			password = optarg;
			break;

		case 'f':
			//g_listname = optarg;
			g_dbfpath = optarg;
			break;

		default:
			exit(1);
		}
	}

	if (!username || !password || !g_dbfpath) {
		usage(basename(argv[0]));
		exit(1);
	}

	printf( "db fpath: %s", g_dbfpath );

	/* Create session */
	spconfig.application_key_size = g_appkey_size;

	err = sp_session_create(&spconfig, &sp);

	if (SP_ERROR_OK != err) {
		fprintf(stderr, "Unable to create session: %s\n",
			sp_error_message(err));
		exit(1);
	}

	g_sess = sp;

	pthread_mutex_init(&g_notify_mutex, NULL);
	pthread_cond_init(&g_notify_cond, NULL);
	

	sp_session_login(sp, username, password, 0);
	pthread_mutex_lock(&g_notify_mutex);

	/* IF EXIT HERE, ALL WORKS ~ */
	/* if sp_playlistcontainer_add_callbacks is used, then we have a problem */

	/* printf("\n\nEND\n\n"); exit(0); */





	for (;;) {
		if (next_timeout == 0) {
			while(!g_notify_do && !g_playback_done)
				pthread_cond_wait(&g_notify_cond, &g_notify_mutex);
		} else {
			struct timespec ts;

#if _POSIX_TIMERS > 0
			clock_gettime(CLOCK_REALTIME, &ts);
#else
			struct timeval tv;
			gettimeofday(&tv, NULL);
			TIMEVAL_TO_TIMESPEC(&tv, &ts);
#endif
			ts.tv_sec += next_timeout / 1000;
			ts.tv_nsec += (next_timeout % 1000) * 1000000;

			pthread_cond_timedwait(&g_notify_cond, &g_notify_mutex, &ts);
		}

		g_notify_do = 0;
		pthread_mutex_unlock(&g_notify_mutex);

		if (g_playback_done) {
			track_ended();
			g_playback_done = 0;
		}

		do {
			sp_session_process_events(sp, &next_timeout);
		} while (next_timeout == 0);

		pthread_mutex_lock(&g_notify_mutex);
	}

	/* MOVED! was originaly plcaed very early in main method. caused issues
	sp_playlistcontainer_add_callbacks(
		sp_session_playlistcontainer(g_sess),
		&pc_callbacks,
		NULL);
	*/

	return 0;
}
