/**
 * \author Mathieu Erbas
 */

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include "sensor_db.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define RUN_QUERY(connection, callback, query_failed, format...)                \
    do {                                                                        \
        char* sql_query = NULL;                                                 \
        ASSERT_ELSE_PERROR(asprintf(&sql_query, format) > 0);                   \
        char* err_msg = NULL;                                                   \
        query_failed = false;                                                   \
        int retries = 0;                                                        \
        int rc = !SQLITE_OK;                                                    \
        do {                                                                    \
            rc = sqlite3_exec(connection, sql_query, callback, NULL, &err_msg); \
            retries++;                                                          \
        } while (rc != SQLITE_OK && retries < 3);                               \
        if (rc != SQLITE_OK) {                                                  \
            printf("Query \" %s \" Failed :%s\n", sql_query, err_msg);          \
            printf("Connection to SQL server lost\n");                          \
            sqlite3_free(err_msg);                                              \
            sqlite3_close(connection);                                          \
            query_failed = true;                                                \
        }                                                                       \
        free(sql_query);                                                        \
    } while (false)

DBCONN* storagemgr_init_connection(bool clear_up_flag) {
    sqlite3* db = NULL;
    int rc = sqlite3_open(TO_STRING(DB_NAME), &db); // rc stands for result code
    if (rc != SQLITE_OK) {
        printf("Unable to connect to SQL server: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    printf("Connection to SQL server established\n");

    char* query =
        clear_up_flag == 1
            ? "DROP TABLE IF EXISTS " TO_STRING(
                  TABLE_NAME) ";"
                              "CREATE TABLE " TO_STRING(
                                  TABLE_NAME) " (id INTEGER PRIMARY KEY "
                                              "AUTOINCREMENT,sensor_id INT, "
                                              "sensor_value DECIMAL(4,2), "
                                              "timestamp TIMESTAMP);"
            : "CREATE TABLE IF NOT EXISTS " TO_STRING(
                  TABLE_NAME) " (id INTEGER PRIMARY KEY AUTOINCREMENT,sensor_id "
                              "INT, sensor_value DECIMAL(4,2), timestamp "
                              "TIMESTAMP);";
    bool query_failed = false;

    RUN_QUERY(db, NULL, query_failed, query, NULL);

    assert(db != NULL);
    query_failed ? printf("A new table couldn't be created\n")
                 : printf("New table " TO_STRING(TABLE_NAME) " created\n");
    return query_failed ? NULL : db;
}

void storagemgr_disconnect(DBCONN* conn) {
    sqlite3_close(conn);
}

int storagemgr_insert_sensor(DBCONN* conn, sensor_id_t id, sensor_value_t value,
                             sensor_ts_t ts) {
    bool query_failed = false;
    RUN_QUERY(
        conn, NULL, query_failed,
        "INSERT INTO  " TO_STRING(
            TABLE_NAME) "(sensor_id,sensor_value,timestamp) VALUES (%d,%f,%ld);",
        id, value, ts);
    return query_failed;
}
