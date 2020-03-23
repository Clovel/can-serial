/**
 * @brief CAN over serial socket management functions
 * 
 * @file can_serial_socket_mgt.c
 */

#ifndef can_serial_SOCKET_MGT_H
#define can_serial_SOCKET_MGT_H

/* Includes -------------------------------------------- */
#include "can_serial_error_codes.h"
#include "can_serial.h"

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Socket management functions ------------------------- */
cipErrorCode_t CIP_initCanSocket(const cipID_t pID);
cipErrorCode_t CIP_closeSocket(const cipID_t pID);

#endif /* can_serial_SOCKET_MGT_H */
