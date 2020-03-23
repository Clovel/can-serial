/**
 * @brief CAN over serial port management functions
 * 
 * @file can_serial_serial_port_mgt.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"

/* C system */
#include <fcntl.h>

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
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_initSerialPort> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    errno = 0;
    gCANSerial[pID].fd = open(gCANSerial[pID].fd, O_RDWR | O_NONBLOCK);
    if(0 > gCANSerial[pID].fd) {
        /* Could not open the port */
        printf("[ERROR] <CANSerial_initSerialPort> Failed to open the serial port\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }

        return CAN_SERIAL_ERROR_SYS;
    }

    errno = 0;
    if(0 > fcntl(gCANSerial[pID].fd, F_SETFL, 0)) {/* ??? */
        /* Could not set the file descriptor's status flags */
        if(0 != errno) {
            printf("[ERROR] <CANSerial_initSerialPort> Failed to set the file descriptor's status flags\n");
            if(0 != errno) {
                printf("        errno = %d (%s)\n", errno, strerror(errno));
            }
        }
    }

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_closeSerialPort(const canSerialID_t pID) {
    /* Close the socket */
    errno = 0;
    if(0 > close(gCANSerial[pID].fd)) {
        printf("[ERROR] <CANSerial_initcanSocket> close failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    return CAN_SERIAL_ERROR_NONE;
}
