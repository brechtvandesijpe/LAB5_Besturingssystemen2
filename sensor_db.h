#pragma once

/**
 * \author Mathieu Erbas
 */

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include "config.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef DB_NAME
    #define DB_NAME Sensor.db
#endif

#ifndef TABLE_NAME
    #define TABLE_NAME SensorData
#endif

#define DBCONN sqlite3

typedef int (*callback_t)(void*, int, char**, char**);

/**
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME
 * \param clear_up_flag if the table existed, clear up the existing data when clear_up_flag is set to 1
 * \return the connection for success, NULL if an error occurs
 */
DBCONN* storagemgr_init_connection(bool clear_up_flag);

/**
 * Disconnect from the database server
 * \param conn pointer to the current connection
 */
void storagemgr_disconnect(DBCONN* conn);

/**
 * Write an INSERT query to insert a single sensor measurement
 * \param conn pointer to the current connection
 * \param id the sensor id
 * \param value the measurement value
 * \param ts the measurement timestamp
 * \return zero for success, and non-zero if an error occurs
 */
int storagemgr_insert_sensor(DBCONN* conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);
