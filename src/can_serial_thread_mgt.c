/**
 * @brief CAN over serial API send functions
 * 
 * @file can_serial_send.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"
#include "can_serial.h"

/* C system */
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Static variables ------------------------------------ */

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];

/* Thread management functions ------------------------- */
canSerialErrorCode_t CANSerial_setPutMessageFunction(const canSerialID_t pID,
    const uint8_t pCallerID,
    const canSerialPutMessageFct_t pFct)
{
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_setPutMessageFunction> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check the ID */
    if(pID != gCANSerial[pID].instanceID) {
        printf("[ERROR] <CANSerial_setPutMessageFunction> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    if(NULL == pFct) {
        printf("[ERROR] <CANSerial_setPutMessageFunction> Function ptr arg is NULL !\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    gCANSerial[pID].callerID      = pCallerID;
    gCANSerial[pID].putMessageFct = pFct;

    return CAN_SERIAL_ERROR_NONE;
}

static void CANSerial_rxThreadCleanup(void *pPtr) {
    const uint8_t lID = *(uint8_t *)pPtr;
    gCANSerial[lID].rxThreadOn = false;
}

static void CANSerial_rxThread(const canSerialID_t * const pID) {
    /* Check if the parameter is NULL */
    if(NULL == pID) {
        printf("[ERROR] <CANSerial_rxThread> ID ptr is NULL !\n");
        return;
    }

    /* Declare the CANSerial message */
    canMessage_t lMsg;

    /* Save the parameter before it is destroyed by the tread creator */
    const canSerialID_t lID = *pID;

    /* Check the ID */
    if(!CANSerial_moduleExists(*pID)) {
        printf("[ERROR] <CANSerial_rxThread> No CANSerial module has the ID %u.\n", *pID);
        return;
    }

    /* Check if the module is already initialized */
    if(!gCANSerial[lID].isInitialized) {
        printf("[ERROR] <CANSerial_rxThread> CANSerial module %u is not initialized.\n", gCANSerial[lID].instanceID);
        return;
    }

    if(NULL == gCANSerial[lID].putMessageFct) {
        printf("[ERROR] <CANSerial_rxThread> Message buffer getter function is NULL.\n");
        return;
    }

    canSerialErrorCode_t  lErrorCode      = CAN_SERIAL_ERROR_NONE;
    int             lGetBufferError = 0;
    ssize_t         lReadBytes      = 0;

    /* Starting thread routine */
    pthread_cleanup_push((void (*)(void *))CANSerial_rxThreadCleanup, (void *)&lID);

    gCANSerial[lID].rxThreadOn = true;

    /* Infinite Rx loop */
    printf("[DEBUG] <CANSerial_rxThread> Starting RX thread.\n");
    while (CAN_SERIAL_ERROR_NONE == lErrorCode) {
        /* Initialize the CANSerial message */
        memset(&lMsg, 0, sizeof(canMessage_t));

        /* Lock the Mutex */
        pthread_mutex_lock(&gCANSerial[lID].mutex);
        
        /* Reading a CAN message */
        lErrorCode = CANSerial_recv(lID, &lMsg, &lReadBytes);
        if(CAN_SERIAL_ERROR_NONE != lErrorCode) {
            printf("[ERROR] <CANSerial_rxThread> CANSerial_recv failed w/ error code %u\n", lErrorCode);

            /* Unlock the Mutex */
            pthread_mutex_unlock(&gCANSerial[lID].mutex);

            break;
        }

        /* Unlock the Mutex */
        pthread_mutex_unlock(&gCANSerial[lID].mutex);

        if(CAN_SERIAL_ERROR_NONE == lErrorCode && 0 > lReadBytes) {
            /* Nothing read, the socket is non-blocking */
            continue;
        }
        
        if(sizeof(canMessage_t) != lReadBytes) {
            printf("[ERROR] <CANSerial_rxThread> CANSerial_recv received inconsistent data of size %ld\n", lReadBytes);
            break;
        }

        /* Get buffer to store this data */
        lGetBufferError = gCANSerial[lID].putMessageFct(gCANSerial[lID].callerID, lMsg.id, lMsg.size, lMsg.data, lMsg.flags);
        if(0 != lGetBufferError) {
            printf("[ERROR] <CANSerial_rxThread> putMessageFct callback failed w/ error code %u\n", lErrorCode);
            break;
        }

        usleep(10000U);
    }

    printf("[ERROR] <CANSerial_rxThread> RX thread shut down. (error code = %d)\n", lErrorCode);

    gCANSerial[lID].rxThreadOn = false;

    /* Mandatory pop */
    pthread_cleanup_pop(1);
}

canSerialErrorCode_t CANSerial_startRxThread(const canSerialID_t pID) {
    /* Check the ID */
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_startRxThread> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    if(NULL == gCANSerial[pID].putMessageFct) {
        printf("[ERROR] <CANSerial_startRxThread> Message buffer getter function is NULL.\n");
        return CAN_SERIAL_ERROR_CONFIG;
    }

    int lSysResult = 0;
    lSysResult = pthread_create(&gCANSerial[pID].thread, NULL, (void * (*)(void *))CANSerial_rxThread, (void *)&pID);
    if (0 < lSysResult) {
        printf("[ERROR] <CANSerial_startRxThread> Thread creation failed\n");
        return CAN_SERIAL_ERROR_SYS;
    } else {
        printf("[INFO ] <CANSerial_startRxThread> Thread creation successful\n");
    }

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_isRxThreadOn(const canSerialID_t pID, bool * const pOn) {
    /* Check the ID */
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_isRxThreadOn> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check argument ptr */
    if(NULL == pOn) {
        printf("[ERROR] <CANSerial_isRxThreadOn> Parameter ptr is NULL !\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    *pOn = gCANSerial[pID].rxThreadOn;

    return CAN_SERIAL_ERROR_NONE;
}
