/**
 * @brief CAN over serial API receive functions
 * 
 * @file can_serial_recv.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"

/* C system */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
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
    pthread_mutex_lock(&gCANSerial[pID].mutex);
    
    /* Check the ID */
    if(pID != gCANSerial[pID].instanceID) {
        printf("[ERROR] <CANSerial_recv> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_rxThread> CANSerial module %u is not initialized.\n", gCANSerial[pID].instanceID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }

    if(NULL == pMsg) {
        printf("[ERROR] <CANSerial_rxThread> Message is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    if(NULL == pReadBytes) {
        printf("[ERROR] <CANSerial_rxThread> pReadBytes output pointer is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    *pReadBytes = 0;
    
    /* Receive the CAN frame */
    errno = 0;

    pthread_mutex_unlock(&gCANSerial[pID].mutex);

    return CAN_SERIAL_ERROR_NONE;
}
