/**
 * @brief CAN over serial utility functions
 * 
 * @file can_serial_tools.c
 */

/* Includes -------------------------------------------- */
#include "can_serial.h"
#include "can_serial_private.h"

/* C System */
#include <stdio.h> /* sscanf() */

/* Defines --------------------------------------------- */
#define CANUSB_STD_HEADER_SIZE      5U
#define CANUSB_EXT_HEADER_SIZE      10U
#define CANUSB_STD_RTR_HEADER_SIZE  CANUSB_STD_HEADER_SIZE
#define CANUSB_EXT_RTR_HEADER_SIZE  CANUSB_EXT_HEADER_SIZE
#define CANUSB_TIMESTAMP_SIZE       4U
#define CANUSB_LONG_TIMESTAMP_SIZE  8U

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

canSerialErrorCode_t CANUSBToCANMsg(char * const pBuf,
    canMessage_t * const pMsg,
    const ssize_t * const pBufSize)
{
    if(NULL == pBuf) {
        printf("[ERROR] <CANUSBToCANMsg> Argument pBuf ptr is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    if(NULL == pMsg) {
        printf("[ERROR] <CANUSBToCANMsg> Argument pMsg ptr is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    uint8_t lDataOffset = 0U;
    ssize_t lMinSize    = 0U; /* Minimum buffer size */

    switch(*pBuf) {
        case 't':
            /* Get the DLC */
            pMsg->size = pBuf[4U] - '0';

            /* Check the bffer's size */
            lMinSize = CANUSB_STD_HEADER_SIZE + 2U * pMsg->size + 1U;
            if(*pBufSize < lMinSize) {
                /* Not enough data */
                printf("[ERROR] <CANUSBToCANMsg> Data buffer size too small (%lu < %lu)", *pBufSize, lMinSize);
                return CAN_SERIAL_ERROR_ARG;
            }

            /* Received Standard CAN frame */
            pMsg->flags |= CAN_SERIAL_NOT_RTR_FLAG;

            /* Set the DLC to '\0' to terminate the ID string */
            pBuf[4U] = '\0';

            /* Data offset */
            lDataOffset = 5U;

            /* Read the CAN ID */
            sscanf(pBuf + 1U, "%lx", &pMsg->id);

            printf("[DEBUG] <CANSerial_recv> Received Standard CAN frame\n");
            break;
        case 'T':
            /* Get the DLC */
            pMsg->size = pBuf[9U] - '0';

            /* Check the bffer's size */
            lMinSize = CANUSB_EXT_HEADER_SIZE + 2U * pMsg->size + 1U;
            if(*pBufSize < lMinSize) {
                /* Not enough data */
                printf("[ERROR] <CANUSBToCANMsg> Data buffer size too small (%lu < %lu)", *pBufSize, lMinSize);
                return CAN_SERIAL_ERROR_ARG;
            }
            
            /* Received Extended CAN frame */
            pMsg->flags |= CAN_SERIAL_NOT_RTR_FLAG;

            /* Set the DLC to '\0' to terminate the ID string */
            pBuf[9U] = '\0';

            /* Data offset */
            lDataOffset = 10U;

            /* Read the CAN ID */
            sscanf(pBuf + 1U, "%lx", &pMsg->id);

            printf("[DEBUG] <CANSerial_recv> Received Extended CAN frame\n");
            break;
        case 'r':
            /* Get the DLC */
            pMsg->size = pBuf[4U] - '0';

            /* Check the bffer's size */
            lMinSize = CANUSB_STD_RTR_HEADER_SIZE + 1U;
            if(*pBufSize < lMinSize) {
                /* Not enough data */
                printf("[ERROR] <CANUSBToCANMsg> Data buffer size too small (%lu < %lu)", *pBufSize, lMinSize);
                return CAN_SERIAL_ERROR_ARG;
            }

            /* Received Standard RTR CAN frame */
            pMsg->flags |= CAN_SERIAL_RTR_FLAG;

            /* Set the DLC to '\0' to terminate the ID string */
            pBuf[4U] = '\0';

            /* Data offset */
            lDataOffset = 5U;

            /* Read the CAN ID */
            sscanf(pBuf + 1U, "%lx", &pMsg->id);

            printf("[DEBUG] <CANSerial_recv> Received Standard RTR CAN frame\n");
            break;
        case 'R':
            /* Get the DLC */
            pMsg->size = pBuf[9U] - '0';

            /* Check the bffer's size */
            lMinSize = CANUSB_EXT_RTR_HEADER_SIZE + 1U;
            if(*pBufSize < lMinSize) {
                /* Not enough data */
                printf("[ERROR] <CANUSBToCANMsg> Data buffer size too small (%lu < %lu)", *pBufSize, lMinSize);
                return CAN_SERIAL_ERROR_ARG;
            }

            /* Received Extended RTR CAN frame */
            pMsg->flags |= CAN_SERIAL_RTR_FLAG;

            /* Set the DLC to '\0' to terminate the ID string */
            pBuf[9U] = '\0';

            /* Data offset */
            lDataOffset = 10U;

            /* Read the CAN ID */
            sscanf(pBuf + 1U, "%lx", &pMsg->id);

            printf("[DEBUG] <CANSerial_recv> Received Extended RTR CAN frame\n");
            break;
        default:
            /* Received unknown command from serial device */
            printf("[ERROR] <CANSerial_recv> Received unknown command from serial device (%s)", pBuf);
            return CAN_SERIAL_ERROR_NET;
    }

    char lSavedChar = 0;
    /* Fill in the data */
    if(!(CAN_SERIAL_RTR_FLAG & pMsg->flags)) {
        for(uint8_t i = 0U; (i < pMsg->size) && (i < CAN_MESSAGE_MAX_SIZE); i++) {
            /* Save the value after the one we are interrested in */
            lSavedChar = *(pBuf + lDataOffset + i + 1U);
            /* Replace it w/ a NULL terminator for sscanf */
            *(pBuf + lDataOffset + i + 1U) = '\0';
            /* Read the data w/ sscanf */
            sscanf(pBuf + lDataOffset + i, "%x", pMsg->data + i);
            /* Put the replaced value back */
            *(pBuf + lDataOffset + i + 1U) = lSavedChar;
        }
    }

    /* TODO : If timestamp is active, fetch it */
    if('\r' != *(pBuf + lMinSize)) {
        /* Either there is trash data after the message (unlikely),
         * either there is a timestamp */

        /* TODO : Timestamps are not yet supported.
         * Place a NULL terminator in it's stead */
        *(pBuf + lMinSize) = '\r';
    }

    return CAN_SERIAL_ERROR_NONE;
}
