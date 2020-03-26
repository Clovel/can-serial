/**
 * @brief CAN over serial API receive functions
 * 
 * @file can_serial_recv.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"
#include "can_serial_commands.h"

/* C system */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* errno */
#include <errno.h>

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Static variables ------------------------------------ */

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];

canSerialErrorCode_t CANSerial_recv(const canSerialID_t pID, canMessage_t * const pMsg, ssize_t * const pReadBytes) {    
    /* Check the ID */
    if(pID != gCANSerial[pID].instanceID) {
        printf("[ERROR] <CANSerial_recv> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_recv> CANSerial module %u is not initialized.\n", gCANSerial[pID].instanceID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }

    if(NULL == pMsg) {
        printf("[ERROR] <CANSerial_recv> Message is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    if(NULL == pReadBytes) {
        printf("[ERROR] <CANSerial_recv> pReadBytes output pointer is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    *pReadBytes = 0;

    /* Allocate a buffer to store the received data */
    char *lBuf = (char *)malloc(CAN_SERIAL_CMD_MAX_SIZE);
    memset(lBuf, 0, CAN_SERIAL_CMD_MAX_SIZE);

    /* FD set declaration */
    fd_set lInputSet;
    /* Empty the FD Set */
    FD_ZERO(&lInputSet);
    /* Listen to the input descriptor */
    FD_SET(gCANSerial[pID].fd, &lInputSet);

    /* Check if any data is available */
    errno = 0;
    int lAvailable = select(1, &lInputSet, NULL, NULL, NULL);
    if(0 > lAvailable) {
        /* Some error has occured in the input */
        printf("[ERROR] <CANSerial_recv> select failed\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }

        return CAN_SERIAL_ERROR_SYS;
    } else if(0 < lAvailable) {
        /* Data is available */
        *pReadBytes = read(gCANSerial[pID].fd, lBuf, lAvailable);
        if(0 > *pReadBytes) {
            printf("[ERROR] <CANSerial_recv> read failed\n");
            if(0 != errno) {
                printf("        errno = %d (%s)\n", errno, strerror(errno));
            }

            return CAN_SERIAL_ERROR_SYS;
        } else if (0 < *pReadBytes) {
            
            if(CAN_SERIAL_ERROR_NONE != CANUSBToCANMsg(lBuf, pMsg, pReadBytes)) {
                printf("[ERROR] <CANSerial_recv> CANUSBToCANMsg failed");
                return CAN_SERIAL_ERROR_NET;
            }
        } else {
            /* select() told us nothing was available,
             * following through...
             */

            *pReadBytes = 0U;
        }
    }

    return CAN_SERIAL_ERROR_NONE;
}
