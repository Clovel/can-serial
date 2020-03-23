/**
 * @brief CAN over serial port management functions
 * 
 * @file can_serial_serial_port_mgt.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"

/* C system */
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* errno */
#include <errno.h>

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Static variables ------------------------------------ */

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];

/* Serial port management functions -------------------- */

canSerialErrorCode_t CANSerial_initSerialPort(const canSerialID_t pID) {
    //
}

canSerialErrorCode_t CANSerial_closeSocket(const canSerialID_t pID) {
    (void)pID;

    /* Close the socket */
    errno = 0;
    if(0 > close(gCANSerial[pID].canSocket)) {
        printf("[ERROR] <CANSerial_initcanSocket> close failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    return CAN_SERIAL_ERROR_NONE;
}
