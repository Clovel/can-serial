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
// static const socklen_t sAddrLen = sizeof(struct sockaddr);

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];

canSerialErrorCode_t CANSerial_recv(const canSerialID_t pID, canMessage_t * const pMsg, ssize_t * const pReadBytes) {
    pthread_mutex_lock(&gCANSerial[pID].mutex);
    
    /* Check the ID */
    if(pID != gCANSerial[pID].instanceID) {
        printf("[ERROR] <CANSerial_recv> No CAN-IP module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_rxThread> CAN-IP module %u is not initialized.\n", gCANSerial[pID].instanceID);
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
    struct sockaddr_in lSrcAddr;
    socklen_t lSrcAddrLen = sizeof(lSrcAddr);
    //char lSrcIPAddr[INET_ADDRSTRLEN] = "";
    
    /* Receive the CAN frame */
    *pReadBytes = recvfrom(gCANSerial[pID].canSocket, (void *)pMsg, sizeof(canMessage_t), 0, 
        (struct sockaddr *)&lSrcAddr, &lSrcAddrLen);
    //*pReadBytes = recv(gCANSerial[pID].canSocket, (void *)pMsg, sizeof(canMessage_t), 0);
    if(0 > *pReadBytes) {
        if(EAGAIN != errno && EWOULDBLOCK != errno) {
            printf("[ERROR] <CANSerial_send> recvfrom failed !\n");
            if(0 != errno) {
                printf("        errno = %d (%s)\n", errno, strerror(errno));
            }
            return CAN_SERIAL_ERROR_NET;
        } else {
            /* Nothing to read on the socket */
        }
    } else if (0 == *pReadBytes) {
        /* Nothing was read from the socket */
    } else {
        /* We got our message */
        
        // inet_ntop(PF_INET, &lSrcAddr.sin_addr, lSrcIPAddr, INET_ADDRSTRLEN);
        // printf("[DEBUG] <CANSerial_send> Received %ld bytes from %s\n", *pReadBytes, lSrcIPAddr, lSrcAddrLen);
    }

    pthread_mutex_unlock(&gCANSerial[pID].mutex);

    return CAN_SERIAL_ERROR_NONE;
}
