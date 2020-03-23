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
extern cipInternalStruct_t gCIP;


cipErrorCode_t CIP_send(const cipID_t pID,
    const uint32_t pCANID,
    const uint8_t pSize,
    const uint8_t * const pData,
    const uint32_t pFlags)
{
    pthread_mutex_lock(&gCIP.mutex);

    /* Check the ID */
    if(pID != gCIP.cipInstanceID) {
        printf("[ERROR] <CIP_send> No CAN-IP module has the ID %u\n", pID);
        return can_serial_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(!gCIP.isInitialized) {
        printf("[ERROR] <CIP_rxThread> CAN-IP module %u is not initialized.\n", gCIP.cipInstanceID);
        return can_serial_ERROR_NOT_INIT;
    }

    /* Build CIP message */
    cipMessage_t lMsg;
    memset(lMsg.data, 0, CAN_MESSAGE_MAX_SIZE);
    lMsg.id    = pCANID;
    lMsg.size  = pSize;
    lMsg.flags = pFlags;
    for(uint8_t i = 0U; (i < lMsg.size) && (i < CAN_MESSAGE_MAX_SIZE) && (NULL != pData); i++) {
        lMsg.data[i] = pData[i];
    }
    
    /* Set the random ID in the message */
    lMsg.randID = gCIP.randID;

    ssize_t lSentBytes = 0;

    errno = 0;
    lSentBytes = sendto(gCIP.canSocket, (const void *)&lMsg, sizeof(cipMessage_t), 0, 
        (const struct sockaddr *)&gCIP.socketInAddress, sizeof(gCIP.socketInAddress));
    if(sizeof(cipMessage_t) != lSentBytes) {
        printf("[ERROR] <CIP_send> sendto failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    pthread_mutex_unlock(&gCIP.mutex);

    return can_serial_ERROR_NONE;
}
