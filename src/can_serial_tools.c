/**
 * @brief CAN over serial utility functions
 * 
 * @file can_serial_tools.h
 */

/* Includes -------------------------------------------- */
#include "can_serial.h"
#include "can_serial_private.h"

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];

/* Utility functions ----------------------------------- */
bool CANSerial_moduleExists(const canSerialID_t pID) {
    /* check if the module already exists */
    for(uint8_t i = 0U; i < CAN_SERIAL_MAX_NB_MODULES; i++) {
        if((gCANSerial[i].instanceID == pID)
            && (gCANSerial[i].isCreated)) {
            return true;
        }
    }

    return false;
}