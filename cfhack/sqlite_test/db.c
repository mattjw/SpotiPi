/* 
 * File:   main.c
 * Author: whiteh
 *
 * Created on 10 March 2012, 11:50
 */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

/*
 * 
 */
typedef struct playlist{
    int pl_id;
    char* name;
    int no_of_songs;
};


int main(int argc, char** argv) {
	char* dbname = "test.sqlite";

	setupDB( dbname );

	sqlite3 *db;
	char *errMsg;

	if(sqlite3_open(dbname, &db) == SQLITE_OK) {

	    struct playlist test;
	    test.name="test";
	    test.pl_id=90;
	    test.no_of_songs=45;
	    addPlaylist(db, test);


	    test.name="9ui234jasd2";
	    test.pl_id=77;
	    test.no_of_songs=32;
	    addPlaylist(db, test);


	    sqlite3_close(db);

	}
	else
	{
		printf("Issues Opening DB, DB Not Opened/Created");
		printf("%s", sqlite3_errmsg(db));
	}
	return 0;

}


int setupDB(char* dbname){
    const char * *docDir;
    sqlite3 *db;

    char *errMsg;

    if(sqlite3_open(dbname, &db) == SQLITE_OK) {
        printf("Database Opened/Created");
        const char *create_playlist_table = "CREATE TABLE IF NOT EXISTS spotify_playlists('pl_id' INTEGER PRIMARY KEY, 'name' VARCHAR(100), 'no_of_songs' INTEGER)";
        if (sqlite3_exec(db, create_playlist_table, NULL, NULL, &errMsg) == SQLITE_OK) {
            printf("Table Detected/Created");
            sqlite3_close(db);
            return 1;
        } else {
            printf("Error creating SQLite Table");
            printf("%s", sqlite3_errmsg(&db));
            return 0;}
        
    }else{
        printf("Issues Opening DB, DB Not Opened/Created");
        printf("%s", sqlite3_errmsg(db));
        return 0;
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
