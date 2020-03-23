/**
 * @brief CAN over serial API main functions
 * 
 * @file can_serial.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"
#include "can_serial.h"
#include "can_serial_serial_port_mgt.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h> /* For rand() */

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */
canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];

/* CAN over serial main functions -------------------------- */
canSerialErrorCode_t CANSerial_createModule(canSerialID_t * const pID) {
    if(NULL == pID) {
        printf("[ERROR] <CANSerial_createModule> ID ptr is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Find an empty slot */
    for(uint8_t i = 0U; i < CAN_SERIAL_MAX_NB_MODULES; i++) {
        if(!gCANSerial[i].isCreated) {
            /* This will be where the module lays */
            *pID = i;
            gCANSerial[i].instanceID = i;
        }
    }

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_init(const canSerialID_t pID, const canSerialMode_t pMode, const char * pPort) {
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_init> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check the ID */
    if(pID != gCANSerial[pID].instanceID) {
        printf("[ERROR] <CANSerial_init> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(gCANSerial[pID].isInitialized) {
        /* Module is already initialized,
         * so we do nothing */
        printf("[ERROR] <CANSerial_init> CANSerial module %u is already initialized.\n", pID);
        /* TODO : Maybe reset ? */
        return CAN_SERIAL_ERROR_ALREADY_INIT;
    }

    /* Initialize the module */
    gCANSerial[pID].mode       = pMode;
    //gCANSerial[pID].instanceID = pID; /* Done by CANSerial_createModule */
    gCANSerial[pID].isStopped  = false;

    /* Set port */
    gCANSerial[pID].serialPort = pPort;

    /* Initialize the socket */
    if(CAN_SERIAL_ERROR_NONE != CANSerial_initSerialPort(pID)) {
        printf("[ERROR] <CANSerial_init> Failed to initialize socket w/ CANSerial_initSerialPort\n");
        return CAN_SERIAL_ERROR_NET;
    }

    /* Initialize thread related variables */
    gCANSerial[pID].rxThreadOn    = false;
    gCANSerial[pID].callerID      = 0U;
    gCANSerial[pID].putMessageFct = NULL;

    gCANSerial[pID].isInitialized = true;

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_isInitialized(const canSerialID_t pID, bool * const pIsInitialized) {
    if(NULL != pIsInitialized
        && CANSerial_moduleExists(pID))
    {
        *pIsInitialized = gCANSerial[pID].isInitialized;
    } else {
        printf("[ERROR] <CANSerial_isInitialized> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_reset(const canSerialID_t pID, const canSerialMode_t pMode) {
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_reset> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    if(!gCANSerial[pID].isInitialized) {
        /* You shouldn't "reset" a non-initialized module */
        printf("[ERROR] <CANSerial_reset> CANSerial module %u is not initialized, cannot reset.\n", pID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }

    gCANSerial[pID].isStopped = true;
    gCANSerial[pID].isInitialized = false;

    /* Close the socket */
    if(CAN_SERIAL_ERROR_NONE != CANSerial_closeSerialPort(pID)) {
        return CAN_SERIAL_ERROR_NET;
    }

    return CANSerial_init(pID, pMode, gCANSerial[pID].serialPort);
}

canSerialErrorCode_t CANSerial_stop(const canSerialID_t pID) {
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_stop> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_stop> CANSerial module %u is not initialized, cannot stop it.\n", pID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }
    
    gCANSerial[pID].isStopped = true;

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_restart(const canSerialID_t pID) {
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_restart> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_restart> CANSerial module %u is not initialized, cannot restart it.\n", pID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }
    
    gCANSerial[pID].isStopped = false;

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_process(const canSerialID_t pID) {
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_process> No CANSerial module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_process> CANSerial module %u is not initialized.\n", gCANSerial[pID].instanceID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }

    if(NULL == gCANSerial[pID].putMessageFct) {
        printf("[ERROR] <CANSerial_process> Message buffer getter function is NULL.\n");
        return CAN_SERIAL_ERROR_CONFIG;
    }

    canSerialErrorCode_t lErrorCode = CAN_SERIAL_ERROR_NONE;

    if(!gCANSerial[pID].rxThreadOn) {
        /* Start reception thread */
        lErrorCode = CANSerial_startRxThread(pID);
        if(CAN_SERIAL_ERROR_NONE != lErrorCode) {
            printf("[ERROR] <CANSerial_process> CANSerial_startRxThread failed w/ error code %u\n", lErrorCode);
            return CAN_SERIAL_ERROR_SYS;
        }
    }

    return CAN_SERIAL_ERROR_NONE;
}
