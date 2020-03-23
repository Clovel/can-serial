/**
 * @brief CAN over serial error codes
 * 
 * @file can_serial_error_codes.h
 */

#ifndef CAN_SERIAL_ERROR_CODES_H
#define CAN_SERIAL_ERROR_CODES_H

/* Includes -------------------------------------------- */

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */
typedef enum _canSerialErrorCodes {
    CAN_SERIAL_ERROR_UNKNOWN      = 0U,
    CAN_SERIAL_ERROR_NONE         = 1,
    CAN_SERIAL_ERROR_ARG          = 2,
    CAN_SERIAL_ERROR_SYS          = 3,
    CAN_SERIAL_ERROR_NET          = 4,
    CAN_SERIAL_ERROR_ALREADY_INIT = 5,
    CAN_SERIAL_ERROR_NOT_INIT     = 6,
    CAN_SERIAL_ERROR_STOPPED      = 7,
    CAN_SERIAL_ERROR_CONFIG       = 8
} canSerialErrorCode_t;

#endif /* CAN_SERIAL_ERROR_CODES_H */
