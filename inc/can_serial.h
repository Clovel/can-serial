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

/** @brief Define this beforehand if you want 
 * several CAN over serial modules. 
 */
#ifndef CAN_SERIAL_MAX_NB_MODULES
#define CAN_SERIAL_MAX_NB_MODULES 1U
#endif /* CAN_SERIAL_MAX_NB_MODULES */

/** @brief This defines the max length of the Serial port
 * path length.
 */
#ifndef CAN_SERIAL_MAX_PORT_LEN
#define CAN_SERIAL_MAX_PORT_LEN 1U
#endif /* CAN_SERIAL_MAX_PORT_LEN */

#define CAN_SERIAL_ID_STD_FLAG      0x00000000U     /**< CAN Standard (11 bits) ID flag */
#define CAN_SERIAL_ID_EXT_FLAG      0x00000001U     /**< CAN Extended (29 bits) ID flag */
#define CAN_SERIAL_ID_MASK          0x00000001U     /**< CAN ID size flag mask */

#define CAN_SERIAL_NOT_RTR_FLAG     0x00000000U     /**< Normal non-RTR frame flag */
#define CAN_SERIAL_RTR_FLAG         0x00000010U     /**< RTR frame flag */
#define CAN_SERIAL_RTR_MASK         0x00000010U     /**< RTR flag mask */

#define CAN_SERIAL_ID_STD_MASK      0x000007FFU     /**< CAN Standard (11 bits) ID mask */
#define CAN_SERIAL_ID_EXT_MASK      0x1FFFFFFFU     /**< CAN Extended (29 bits) ID mask */

/* Type definitions ------------------------------------ */
typedef struct _canMessage {
    uint32_t id;
    uint8_t  size;
    uint8_t  data[CAN_MESSAGE_MAX_SIZE];
    uint32_t flags;
} canMessage_t;

typedef enum _modes {
    CAN_SERIAL_MODE_UNKNOWN = 0U,
    CAN_SERIAL_MODE_NORMAL  = 1U,
    CAN_SERIAL_MODE_FD      = 2U
} canSerialMode_t;

typedef canSerialMode_t canMode_t;

typedef uint8_t canSerialID_t;

typedef int (*canSerialPutMessageFct_t)(const uint8_t, const uint32_t, const uint8_t, const uint8_t * const, const uint32_t);

/* CAN over serial interface ------------------------------- */
/**
 * @brief CAN over serial module creation
 * 
 * @param[out]  pID     ID of the driver created.
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_createModule(canSerialID_t * const pID);

/**
 * @brief CAN over serial initialisation
 * 
 * @param[in]   pID     ID of the driver used.
 * @param[in]   pMode   Mode of the CAN link (CAN, CANFD, etc.)
 * @param[in]   pPort   Path ro the serial port used
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_init(const canSerialID_t pID, const canSerialMode_t pMode, const char * pPort);

/**
 * @brief CAN over serial check for initialisation
 * 
 * @param[in]   pID             ID of the driver used.
 * @param[out]  pIsInitialized  Boolean stating if the module is initialized or not
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_isInitialized(const canSerialID_t pID, bool * const pIsInitialized);

/**
 * @brief CAN over serial reset
 * 
 * @param[in]   pID     ID of the driver used.
 * @param[in]   pMode   Mode of the CAN link (CAN, CANFD, etc.)
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_reset(const canSerialID_t pID, const canSerialMode_t pMode);

/**
 * @brief CAN over serial stop
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_stop(const canSerialID_t pID);

/**
 * @brief CAN over serial restart
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_restart(const canSerialID_t pID);

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
canSerialErrorCode_t CANSerial_send(const canSerialID_t pID,
    const uint32_t pCANID,
    const uint8_t pSize,
    const uint8_t * const pData,
    const uint32_t pFlags);

/**
 * @brief CAN over serial recieve
 * Use this function to get a CAN message
 * 
 * @param[in]   pID         ID of the driver used.
 * @param[out]  pMsg        CAN message.
 * @param[out]  pReadBytes  Number of bytes read.
 * 
 * @return error_code
 */
canSerialErrorCode_t CANSerial_recv(const canSerialID_t pID, canMessage_t * const pMsg, ssize_t * const pReadBytes);

/**
 * @brief Sets the function used to give a message to
 * the driver's caller's stack.
 * 
 * @param[in]   pID         ID of the driver used.
 * @param[in]   pCallerID   ID of the caller.
 * @param[in]   pFct        Function used to hand the message over to the caller.
 * 
 * @details The ID of the caller depends on the caller's implementation.
 * For example, this could be the ID of the CANOpen stack using this driver.
 * If the user's code does not use this, just put 0U.
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_setPutMessageFunction(const canSerialID_t pID, const uint8_t pCallerID, const canSerialPutMessageFct_t pFct);

/**
 * @brief Print a CAN over serial message (long format)
 * 
 * @param[in]   pMsg    CAN Message to print
 */
void CANSerial_printMessage(const canMessage_t * const pMsg);

/**
 * @brief Print a CAN over serial message (short format)
 * 
 * @param[in]   pMsg    CAN Message to print
 */
void CANSerial_printMessageShort(const canMessage_t * const pMsg);

/**
 * @brief CAN over serial process
 * This function can only be called from the exterior 
 * if CANSerial is compiled to use a non-threaded process
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_process(const canSerialID_t pID);

/**
 * @brief Starts the receiving thread.
 * 
 * @param[in]   pID     ID of the driver used.
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_startRxThread(const canSerialID_t pID);

/**
 * @brief Getter for the "Thread On" variable
 * 
 * @param[in]   pID     ID of the driver used.
 * @param[out]  pOn     Output ptr. true : thread is running, false : thread is off.
 * 
 * @return Error code
 */
canSerialErrorCode_t CANSerial_isRxThreadOn(const canSerialID_t pID, bool * const pOn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_SERIAL_H */
