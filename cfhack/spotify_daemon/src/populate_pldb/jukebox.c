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

/**
 * Called on various events to start playback if it hasn't been started already.
 *
 * The function simply starts playing the first track of the playlist.
 */
static void try_jukebox_start(void)
{
	printf( "\n\ntrack idnex %d\n\n", g_track_index );
	printf("[PRT 111a -- TRY JUKE START]  ");

	sp_track *t;

	if (!g_jukeboxlist)
		return;

	if (!sp_playlist_num_tracks(g_jukeboxlist)) {
		fprintf(stderr, "jukebox: No tracks in playlist. Waiting\n");
		return;
	}

	t = sp_playlist_track(g_jukeboxlist, g_track_index);

	if (g_currenttrack && t != g_currenttrack) {
		/* Someone changed the current track */
		audio_fifo_flush(&g_audiofifo);
		sp_session_player_unload(g_sess);
		g_currenttrack = NULL;
	}

	if (!t)
		return;

	if (sp_track_error(t) != SP_ERROR_OK)
		return;

	if (g_currenttrack == t)
		return;

	g_currenttrack = t;

	printf("jukebox: Now playing \"%s\"...\n", sp_track_name(t));
	fflush(stdout);

	sp_session_player_load(g_sess, t);
	sp_session_player_play(g_sess, 1);
}

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



	if (pl != g_jukeboxlist)
		return;

	printf("jukebox: %d tracks were added\n", num_tracks);
	fflush(stdout);
	try_jukebox_start();
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


	int i, k = 0;

	if (pl != g_jukeboxlist)
		return;

	for (i = 0; i < num_tracks; ++i)
		if (tracks[i] < g_track_index)
			++k;

	g_track_index -= k;

	printf("jukebox: %d tracks were removed\n", num_tracks);
	fflush(stdout);
	try_jukebox_start();
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


	if (pl != g_jukeboxlist)
		return;

	printf("jukebox: %d tracks were moved around\n", num_tracks);
	fflush(stdout);
	try_jukebox_start();
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


	const char *name = sp_playlist_name(pl);

	if (!strcasecmp(name, g_listname)) {
		g_jukeboxlist = pl;
		g_track_index = 0;
		try_jukebox_start();
	} else if (g_jukeboxlist == pl) {
		printf("jukebox: current playlist renamed to \"%s\".\n", name);
		g_jukeboxlist = NULL;
		g_currenttrack = NULL;
		sp_session_player_unload(g_sess);
	}
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


	sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);

	if (!strcasecmp(sp_playlist_name(pl), g_listname)) {
		g_jukeboxlist = pl;
		try_jukebox_start();
	}
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
		try_jukebox_start();
	}


	if (!g_jukeboxlist) {
		printf("jukebox: No such playlist. Waiting for one to pop up...\n");
		fflush(stdout);
	}

	/* Initialise the next track id with a random track */
	int pl_num_tracks = sp_playlist_num_tracks( g_jukeboxlist ); 
	g_track_index = rand() % pl_num_tracks; 	
	
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
	//printf("[PRT 111 -- music delivery ]\n");


	audio_fifo_t *af = &g_audiofifo;
	audio_fifo_data_t *afd;
	size_t s;

	if (num_frames == 0)
		return 0; // Audio discontinuity, do nothing

	pthread_mutex_lock(&af->mutex);

	/* Buffer one second of audio */
	if (af->qlen > format->sample_rate) {
		pthread_mutex_unlock(&af->mutex);

		return 0;
	}

	s = num_frames * sizeof(int16_t) * format->channels;

	afd = malloc(sizeof(audio_fifo_data_t) + s);
	memcpy(afd->samples, frames, s);

	afd->nsamples = num_frames;

	afd->rate = format->sample_rate;
	afd->channels = format->channels;

	TAILQ_INSERT_TAIL(&af->q, afd, link);
	af->qlen += num_frames;

	pthread_cond_signal(&af->cond);
	pthread_mutex_unlock(&af->mutex);

	return num_frames;
}


/**
 * This callback is used from libspotify when the current track has ended
 *
 * @sa sp_session_callbacks#end_of_track
 */
static void end_of_track(sp_session *sess)
{	
	printf("PRT 111p\n");


	pthread_mutex_lock(&g_notify_mutex);
	g_playback_done = 1;
	pthread_cond_signal(&g_notify_cond);
	pthread_mutex_unlock(&g_notify_mutex);
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


	try_jukebox_start();
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


	audio_fifo_flush(&g_audiofifo);

	if (g_currenttrack != NULL) {
		sp_session_player_unload(g_sess);
		g_currenttrack = NULL;
	}
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


	int tracks = 0;

	if (g_currenttrack) {
		g_currenttrack = NULL;
		sp_session_player_unload(g_sess);
		
		/* choose new track index at random */ 
		int pl_num_tracks = sp_playlist_num_tracks( g_jukeboxlist ); 
		g_track_index = rand() % pl_num_tracks; 
		// Note: g_track_index -- defined to be the index of the NEXT track	
		// ++g_track_index; // increment track (if sequential play)
		
		try_jukebox_start();
	}
}

/**
 * Show usage information
 *
 * @param  progname  The program name
 */
static void usage(const char *progname)
{	
	printf("PRT 111t\n");


	fprintf(stderr, "usage: %s -u <username> -p <password> -l <listname> [-d]\n", progname);
	fprintf(stderr, "warning: -d will delete the tracks played from the list!\n");
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


	while ((opt = getopt(argc, argv, "u:p:l:")) != EOF) {
		switch (opt) {
		case 'u':
			username = optarg;
			break;

		case 'p':
			password = optarg;
			break;

		case 'l':
			//g_listname = optarg;
			g_playlist_id = atoi( optarg );
			printf( "parse success" ); 
			break;

		default:
			exit(1);
		}
	}

	if (!username || !password || (g_playlist_id==-1)) {
		usage(basename(argv[0]));
		exit(1);
	}


	audio_init(&g_audiofifo);

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
	
	printf("just before sp_playlistcontainer_add_callbacks --~");






	/*  ORIGINALLY HERE	
	sp_playlistcontainer_add_callbacks(
		sp_session_playlistcontainer(g_sess),
		&pc_callbacks,
		NULL);
	*/

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
