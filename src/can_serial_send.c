/**
 * @brief CAN over serial API send functions
 * 
 * @file can_serial_send.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"

/* C system */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* errno */
#include <errno.h>

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Static variables ------------------------------------ */

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];


canSerialErrorCode_t CANSerial_send(const canSerialID_t pID,
    const uint32_t pCANID,
    const uint8_t pSize,
    const uint8_t * const pData,
    const uint32_t pFlags)
{
    pthread_mutex_lock(&gCANSerial[pID].mutex);

    /* Check the ID */
    if(pID != gCANSerial[pID].instanceID) {
        printf("[ERROR] <CANSerial_send> No CAN-IP module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_rxThread> CAN-IP module %u is not initialized.\n", gCANSerial[pID].instanceID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }

    /* Build CANSerial message */
    canMessage_t lMsg;
    memset(lMsg.data, 0, CAN_MESSAGE_MAX_SIZE);
    lMsg.id    = pCANID;
    lMsg.size  = pSize;
    lMsg.flags = pFlags;
    for(uint8_t i = 0U; (i < lMsg.size) && (i < CAN_MESSAGE_MAX_SIZE) && (NULL != pData); i++) {
        lMsg.data[i] = pData[i];
    }

    ssize_t lSentBytes = 0;

    errno = 0;

    pthread_mutex_unlock(&gCANSerial[pID].mutex);

    return CAN_SERIAL_ERROR_NONE;
}
