/**
 * @brief CAN over serial error codes
 * 
 * @file can_serial_error_codes.h
 */

#ifndef can_serial_ERROR_CODES_H
#define can_serial_ERROR_CODES_H

/* Includes -------------------------------------------- */

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */
typedef enum _cipErrorCodes {
    can_serial_ERROR_UNKNOWN      = 0U,
    can_serial_ERROR_NONE         = 1,
    can_serial_ERROR_ARG          = 2,
    can_serial_ERROR_SYS          = 3,
    can_serial_ERROR_NET          = 4,
    can_serial_ERROR_ALREADY_INIT = 5,
    can_serial_ERROR_NOT_INIT     = 6,
    can_serial_ERROR_STOPPED      = 7,
    can_serial_ERROR_CONFIG       = 8
} cipErrorCode_t;

#endif /* can_serial_ERROR_CODES_H */
