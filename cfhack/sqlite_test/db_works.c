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
    char* name;
    char* spotID;
};


int main(int argc, char** argv) {
    sqlite3 *db;
    setupDB("test.sqlite");

    struct playlist test[1];
    test[0].name="test";
    test[0].spotID="ifh";
    addPlaylists("test.sqlite", test, 1);
}


int setupDB(char* dbname){
    const char * *docDir;
    sqlite3 *db;

    char *errMsg;

    if(sqlite3_open(dbname, &db) == SQLITE_OK) {
        printf("Database Opened/Created");
        const char *create_playlist_table = "CREATE TABLE IF NOT EXISTS playlist('id' INTEGER PRIMARY KEY, 'name' VARCHAR, 'spotifyID' VARCHAR,'current' BOOL DEFAULT FALSE)";
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


int addPlaylists(char* dbname, struct playlist* playlists, int n)
{
    sqlite3 *db;
    char *errMsg;

    if(sqlite3_open(dbname, &db) == SQLITE_OK) {

        int i;
        for(i=0;i<n;i++)
        {
            const char insert[256];
            snprintf(insert, sizeof insert, "%s%s%s%s%s","INSERT INTO playlist(name, spotifyID) VALUES('",playlists[i].name,"', '",playlists[i].spotID,"');");
            printf("%s",insert);
            if (sqlite3_exec(db, insert, NULL, NULL, &errMsg) == SQLITE_OK) {
                printf("Playlist added\n");
            } else {
                printf("Error adding playlists\n");
                printf("%s", sqlite3_errmsg(&db));
                return 0;}
        }
        sqlite3_close(db);
        return 1;

    }else{
        printf("Issues Opening DB, DB Not Opened/Created");
        printf("%s", sqlite3_errmsg(db));
        return 0;
    }
}
