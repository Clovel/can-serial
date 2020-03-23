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
canSerialErrorCode_t CANSerial_createModule(const canSerialID_t pID) {
    if(CAN_SERIAL_MAX_NB_MODULES <= pID) {
        return CAN_SERIAL_ERROR_ARG;
    }

    /* check if the module already exists */
    for(uint8_t i = 0U; i < CAN_SERIAL_MAX_NB_MODULES; i++) {
        if(gCANSerial[i].instanceID != pID) {
            return CAN_SERIAL_ERROR_ARG;
        }
    }

    /* Find an empty slot */
    for(uint8_t i = 0U; i < CAN_SERIAL_MAX_NB_MODULES; i++) {
        if(gCANSerial[i].instanceID != -1) {
            return CAN_SERIAL_ERROR_ARG;
        }
    }

    /* TODO */

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_init(const canSerialID_t pID, const canSerialMode_t pCANSerialMode, const canSerialPort_t pPort) {
    /* Check the ID */
    if(pID != gCANSerial[pID].instanceID) {
        printf("[ERROR] <CANSerial_init> No CAN-IP module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check if the module is already initialized */
    if(gCANSerial[pID].isInitialized) {
        /* Module is already initialized,
         * so we do nothing */
        printf("[ERROR] <CANSerial_init> CAN-IP module %u is already initialized.\n", pID);
        /* TODO : Maybe reset ? */
        return CAN_SERIAL_ERROR_ALREADY_INIT;
    }

    /* Initialize the module */
    gCANSerial[pID].mode       = pCANSerialMode;
    gCANSerial[pID].instanceID = pID;
    gCANSerial[pID].isStopped     = false;

    /* Set port */
    gCANSerial[pID].canPort = pPort;

    /* Initialize the socket */
    if(CAN_SERIAL_ERROR_NONE != CANSerial_initCanSocket(pID)) {
        printf("[ERROR] <CANSerial_init> Failed to initialize socket w/ CANSerial_initCanSocket\n");
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
        && gCANSerial[pID].instanceID == pID)
    {
        *pIsInitialized = gCANSerial[pID].isInitialized;
    } else {
        printf("[ERROR] <CANSerial_isInitialized> No CAN-IP module has the ID %u.\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_reset(const canSerialID_t pID, const canSerialMode_t pCANSerialMode) {
    if(!gCANSerial[pID].isInitialized) {
        /* You shouldn't "reset" a non-initialized module */
        printf("[ERROR] <CANSerial_reset> CAN-IP module %u is not initialized, cannot reset.\n", pID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }

    gCANSerial[pID].isStopped = true;
    gCANSerial[pID].isInitialized = false;

    /* Close the socket */
    if(CAN_SERIAL_ERROR_NONE != CANSerial_closeSocket(pID)) {
        return CAN_SERIAL_ERROR_NET;
    }

    return CANSerial_init(pID, pCANSerialMode, gCANSerial[pID].canPort);
}

canSerialErrorCode_t CANSerial_stop(const canSerialID_t pID) {
    (void)pID; /* TODO : Multiline CAN */

    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_stop> CAN-IP module %u is not initialized, cannot stop it.\n", pID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }
    
    gCANSerial[pID].isStopped = true;

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_restart(const canSerialID_t pID) {
    (void)pID; /* TODO : Multiline CAN */

    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_restart> CAN-IP module %u is not initialized, cannot restart it.\n", pID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }
    
    gCANSerial[pID].isStopped = false;

    return CAN_SERIAL_ERROR_NONE;
}

canSerialErrorCode_t CANSerial_process(const canSerialID_t pID) {
    (void)pID;

    /* Check if the module is already initialized */
    if(!gCANSerial[pID].isInitialized) {
        printf("[ERROR] <CANSerial_rxThread> CAN-IP module %u is not initialized.\n", gCANSerial[pID].instanceID);
        return CAN_SERIAL_ERROR_NOT_INIT;
    }

    if(NULL == gCANSerial[pID].putMessageFct) {
        printf("[ERROR] <CANSerial_rxThread> Message buffer getter function is NULL.\n");
        return CAN_SERIAL_ERROR_CONFIG;
    }

    canSerialErrorCode_t lErrorCode = CAN_SERIAL_ERROR_NONE;

    if(!gCANSerial[pID].rxThreadOn) {
        /* Start reception thread */
        lErrorCode = CANSerial_startRxThread(pID);
        if(CAN_SERIAL_ERROR_NONE != lErrorCode) {
            printf("[ERROR] <CANSerial_rxThread> CANSerial_startRxThread failed w/ error code %u\n", lErrorCode);
            return CAN_SERIAL_ERROR_SYS;
        }
    }

    return CAN_SERIAL_ERROR_NONE;
}
