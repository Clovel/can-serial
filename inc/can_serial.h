/**
 * @brief CAN over serial API header
 * 
 * @file can_serial.h
 */

#ifndef CAN_SERIAL_H
#define CAN_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes -------------------------------------------- */
#include "can_serial_error_codes.h"

#include <stdint.h>  /* TODO : Delete this and use custom types */
#include <stdbool.h> /* TODO : Delete this and use custom types */

#include <sys/types.h>

/* Defines --------------------------------------------- */
#define CAN_MESSAGE_MAX_SIZE 8U

/* Define this beforehand if you want 
 * several CAN over serial modules. 
 */
#ifndef CAN_SERIAL_MAX_NB_MODULES
#define CAN_SERIAL_MAX_NB_MODULES 1U
#endif /* CAN_SERIAL_MAX_NB_MODULES */

/* Type definitions ------------------------------------ */
typedef struct _cipMessage {
    uint32_t id;
    uint8_t  size;
    uint8_t  data[CAN_MESSAGE_MAX_SIZE];
    uint32_t flags;
} cipMessage_t;

typedef cipMessage_t canMessage_t;

typedef enum _modes {
    can_serial_MODE_UNKNOWN = 0U,
    can_serial_MODE_NORMAL  = 1U,
    can_serial_MODE_FD      = 2U
} canSerialMode_t;

typedef canSerialMode_t canMode_t;

typedef uint8_t cipID_t;
typedef int cipPort_t;

typedef int (*cipPutMessageFct_t)(const uint8_t, const uint32_t, const uint8_t, const uint8_t * const, const uint32_t);

/* CAN over serial interface ------------------------------- */
/**
 * @brief CAN over serial module creation
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_createModule(const cipID_t pID);

/**
 * @brief CAN over serial initialisation
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_init(const cipID_t pID, const canSerialMode_t pCANSerialMode, const cipPort_t pPort);

/**
 * @brief CAN over serial check for initialisation
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_isInitialized(const cipID_t pID, bool * const pIsInitialized);

/**
 * @brief CAN over serial reset
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_reset(const cipID_t pID, const canSerialMode_t pCANSerialMode);

/**
 * @brief CAN over serial stop
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_stop(const cipID_t pID);

/**
 * @brief CAN over serial restart
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_restart(const cipID_t pID);

/** 
 * @brief CAN over serial send
 * Use this function to send a CAN message
 * 
 * @param[in]   pID     ID of the driver used.
 * @param[in]   pCANID  CAN message ID.
 * @param[in]   pSize   CAN message size.
 * @param[in]   pData   CAN message data.
 * @param[in]   pFlags  CAN message flags.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_send(const cipID_t pID,
    const uint32_t pCANID,
    const uint8_t pSize,
    const uint8_t * const pData,
    const uint32_t pFlags);

/**
 * @brief CAN over serial recieve
 * Use this function to get a CAN message
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return error_code
 */
cipErrorCode_t CANSerial_recv(const cipID_t pID, cipMessage_t * const pMsg, ssize_t * const pReadBytes);

/**
 * @brief Sets the function used to give a message to
 * the driver's caller's stack.
 * 
 * @param[in]   pID     ID of the driver used.
 * @param[in]   pFct    Function used to hand the message over to the caller.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_setPutMessageFunction(const cipID_t pID, const uint8_t pCallerID, const cipPutMessageFct_t pFct);

/**
 * @brief Print a CAN over serial message (long format)
 * 
 * @param[in]   pMsg    CAN Message to print
 */
void CANSerial_printMessage(const cipMessage_t * const pMsg);

/**
 * @brief Print a CAN over serial message (short format)
 * 
 * @param[in]   pMsg    CAN Message to print
 */
void CANSerial_printMessageShort(const cipMessage_t * const pMsg);

/**
 * @brief CAN over serial process
 * This function can only be called from the exterior 
 * if CANSerial is compiled to use a non-threaded process
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_process(const cipID_t pID);

/**
 * @brief Starts the receiving thread.
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_startRxThread(const cipID_t pID);

/**
 * @brief Getter for the "Thread On" variable
 * 
 * @param[in]   pID     ID of the driver used.
 * @param[out]  pOn     Output ptr. true : thread is running, false : thread is off.
 * 
 * @return Error code
 */
cipErrorCode_t CANSerial_isRxThreadOn(const cipID_t pID, bool * const pOn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_SERIAL_H */
