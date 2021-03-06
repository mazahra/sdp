/*
 * File: Error.h
 *
 * An error handling module.
 *
 */
#ifndef Error_H
#define Error_H

#include <stdint.h>

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/


/**********************************************************************
 * PUBLIC VARIABLES                                                   *
 **********************************************************************/

typedef enum {
    ERROR_NONE = 0x0,
    ERROR_UNKNOWN,
    ERROR_NO_ACKNOWLEDGEMENT,
    ERROR_GPS_DISCONNECTED,
    ERROR_GPS_NOFIX,
    ERROR_NO_ORIGIN,
    ERROR_NO_STATION,
    ERROR_NO_ALTITUDE,
    ERROR_NO_HEARTBEAT,
    ERROR_MAGNETOMETER,
    ERROR_BAROMETER,
    ERROR_ENCODER,
    ERROR_ACCELEROMETER,
    ERROR_GPS,
    ERROR_XBEE,
    ERROR_TILTCOMPASS,
    ERROR_I2C,
    ERROR_NAVIGATION,
    ERROR_OVERRIDE
} error_t;




/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

const char *getErrorMessage(error_t errorCode);

#endif

