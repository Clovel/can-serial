/**
 * @brief CAN over serial API main functions
 * 
 * @file can_serial.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"
#include "can_serial.h"
#include "can_serial_socket_mgt.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h> /* For rand() */

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */
canSerialInternalVars_t gCANSerial;

/* CAN over serial main functions -------------------------- */
cipErrorCode_t CANSerial_createModule(const cipID_t pID) {
    if(can_serial_MAX_NB_MODULES <= pID) {
        return can_serial_ERROR_ARG;
    }

    /* check if the module already exists */
    if(gCANSerial.instanceID != pID) {
        return can_serial_ERROR_ARG;
    }

    /* TODO */

    return can_serial_ERROR_NONE;
}

cipErrorCode_t CANSerial_init(const cipID_t pID, const canSerialMode_t pCANSerialMode, const cipPort_t pPort) {
    /* Check the ID */
    if(pID != gCANSerial.instanceID) {
        printf("[ERROR] <CANSerial_init> No CAN-IP module has the ID %u\n", pID);
        return can_serial_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(gCANSerial.isInitialized) {
        /* Module is already initialized,
         * so we do nothing */
        printf("[ERROR] <CANSerial_init> CAN-IP module %u is already initialized.\n", pID);
        /* TODO : Maybe reset ? */
        return can_serial_ERROR_ALREADY_INIT;
    }

    /* Initialize the module */
    gCANSerial.mode       = pCANSerialMode;
    gCANSerial.instanceID = pID;
    gCANSerial.isStopped     = false;

    /* Set port */
    gCANSerial.canPort = pPort;

    /* Generate random ID */
    time_t lTime;
    srand((unsigned)time(&lTime));
    gCANSerial.randID  = (rand() & 0xFFU) << 0U;
    gCANSerial.randID |= (rand() & 0xFFU) << 8U;
    gCANSerial.randID |= (rand() & 0xFFU) << 16U;
    gCANSerial.randID |= (rand() & 0xFFU) << 24U;
    printf("[DEBUG] Generated random ID : %u\n", gCANSerial.randID);

    /* Initialize the socket */
    if(can_serial_ERROR_NONE != CANSerial_initCanSocket(pID)) {
        printf("[ERROR] <CANSerial_init> Failed to initialize socket w/ CANSerial_initCanSocket\n");
        return can_serial_ERROR_NET;
    }

    /* Initialize thread related variables */
    gCANSerial.rxThreadOn    = false;
    gCANSerial.callerID      = 0U;
    gCANSerial.putMessageFct = NULL;

    gCANSerial.isInitialized = true;

    return can_serial_ERROR_NONE;
}

cipErrorCode_t CANSerial_isInitialized(const cipID_t pID, bool * const pIsInitialized) {
    if(NULL != pIsInitialized
        && gCANSerial.instanceID == pID)
    {
        *pIsInitialized = gCANSerial.isInitialized;
    } else {
        printf("[ERROR] <CANSerial_isInitialized> No CAN-IP module has the ID %u.\n", pID);
        return can_serial_ERROR_ARG;
    }

    return can_serial_ERROR_NONE;
}

cipErrorCode_t CANSerial_reset(const cipID_t pID, const canSerialMode_t pCANSerialMode) {
    if(!gCANSerial.isInitialized) {
        /* You shouldn't "reset" a non-initialized module */
        printf("[ERROR] <CANSerial_reset> CAN-IP module %u is not initialized, cannot reset.\n", pID);
        return can_serial_ERROR_NOT_INIT;
    }

    gCANSerial.isStopped = true;
    gCANSerial.isInitialized = false;

    /* Close the socket */
    if(can_serial_ERROR_NONE != CANSerial_closeSocket(pID)) {
        return can_serial_ERROR_NET;
    }

    return CANSerial_init(pID, pCANSerialMode, gCANSerial.canPort);
}

cipErrorCode_t CANSerial_stop(const cipID_t pID) {
    (void)pID; /* TODO : Multiline CAN */

    if(!gCANSerial.isInitialized) {
        printf("[ERROR] <CANSerial_stop> CAN-IP module %u is not initialized, cannot stop it.\n", pID);
        return can_serial_ERROR_NOT_INIT;
    }
    
    gCANSerial.isStopped = true;

    return can_serial_ERROR_NONE;
}

cipErrorCode_t CANSerial_restart(const cipID_t pID) {
    (void)pID; /* TODO : Multiline CAN */

    if(!gCANSerial.isInitialized) {
        printf("[ERROR] <CANSerial_restart> CAN-IP module %u is not initialized, cannot restart it.\n", pID);
        return can_serial_ERROR_NOT_INIT;
    }
    
    gCANSerial.isStopped = false;

    return can_serial_ERROR_NONE;
}

cipErrorCode_t CANSerial_process(const cipID_t pID) {
    (void)pID;

    /* Check if the module is already initialized */
    if(!gCANSerial.isInitialized) {
        printf("[ERROR] <CANSerial_rxThread> CAN-IP module %u is not initialized.\n", gCANSerial.instanceID);
        return can_serial_ERROR_NOT_INIT;
    }

    if(NULL == gCANSerial.putMessageFct) {
        printf("[ERROR] <CANSerial_rxThread> Message buffer getter function is NULL.\n");
        return can_serial_ERROR_CONFIG;
    }

    cipErrorCode_t lErrorCode = can_serial_ERROR_NONE;

    if(!gCANSerial.rxThreadOn) {
        /* Start reception thread */
        lErrorCode = CANSerial_startRxThread(pID);
        if(can_serial_ERROR_NONE != lErrorCode) {
            printf("[ERROR] <CANSerial_rxThread> CANSerial_startRxThread failed w/ error code %u\n", lErrorCode);
            return can_serial_ERROR_SYS;
        }
    }

    return can_serial_ERROR_NONE;
}
