/**
 * @brief CAN over serial socket management functions
 * 
 * @file can_serial_socket_mgt.c
 */

#ifndef CAN_SERIAL_SOCKET_MGT_H
#define CAN_SERIAL_SOCKET_MGT_H

/* Includes -------------------------------------------- */
#include "can_serial_error_codes.h"
#include "can_serial.h"

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Socket management functions ------------------------- */
cipErrorCode_t CANSerial_initCanSocket(const canSerialID_t pID);
cipErrorCode_t CANSerial_closeSocket(const canSerialID_t pID);

#endif /* CAN_SERIAL_SOCKET_MGT_H */
