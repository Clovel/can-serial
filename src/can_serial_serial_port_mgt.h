/**
 * @brief CAN over serial socket management functions
 * 
 * @file can_serial_serial_mgt.h
 */

#ifndef CAN_SERIAL_SERIAL_PORT_MGT_H
#define CAN_SERIAL_SERIAL_PORT_MGT_H

/* Includes -------------------------------------------- */
#include "can_serial_error_codes.h"
#include "can_serial.h"

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Socket management functions ------------------------- */
canSerialErrorCode_t CANSerial_initCanSocket(const canSerialID_t pID);
canSerialErrorCode_t CANSerial_closeSerialPort(const canSerialID_t pID);

#endif /* CAN_SERIAL_SERIAL_PORT_MGT_H */
