#pragma once

/**
 * \author Mathieu Erbas
 */

#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Initializes the data manager
 */
void datamgr_init();

/**
 * processes a single temperature measurement
 */
void datamgr_process_reading(const sensor_data_t* data);

/**
 * This method cleans up the datamgr, and frees all used memory.
 */
void datamgr_free();
