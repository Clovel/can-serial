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
extern canSerialInternalVars_t gCANSerial;


cipErrorCode_t CANSerial_send(const cipID_t pID,
    const uint32_t pCANID,
    const uint8_t pSize,
    const uint8_t * const pData,
    const uint32_t pFlags)
{
    pthread_mutex_lock(&gCANSerial.mutex);

    /* Check the ID */
    if(pID != gCANSerial.instanceID) {
        printf("[ERROR] <CANSerial_send> No CAN-IP module has the ID %u\n", pID);
        return can_serial_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(!gCANSerial.isInitialized) {
        printf("[ERROR] <CANSerial_rxThread> CAN-IP module %u is not initialized.\n", gCANSerial.instanceID);
        return can_serial_ERROR_NOT_INIT;
    }

    /* Build CANSerial message */
    cipMessage_t lMsg;
    memset(lMsg.data, 0, CAN_MESSAGE_MAX_SIZE);
    lMsg.id    = pCANID;
    lMsg.size  = pSize;
    lMsg.flags = pFlags;
    for(uint8_t i = 0U; (i < lMsg.size) && (i < CAN_MESSAGE_MAX_SIZE) && (NULL != pData); i++) {
        lMsg.data[i] = pData[i];
    }
    
    /* Set the random ID in the message */
    lMsg.randID = gCANSerial.randID;

    ssize_t lSentBytes = 0;

    errno = 0;
    lSentBytes = sendto(gCANSerial.canSocket, (const void *)&lMsg, sizeof(cipMessage_t), 0, 
        (const struct sockaddr *)&gCANSerial.socketInAddress, sizeof(gCANSerial.socketInAddress));
    if(sizeof(cipMessage_t) != lSentBytes) {
        printf("[ERROR] <CANSerial_send> sendto failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    pthread_mutex_unlock(&gCANSerial.mutex);

    return can_serial_ERROR_NONE;
}
