/**
 * @brief CAN over serial sender receiver example
 * 
 * @file main.c
 */

/* Includes -------------------------------------------- */
#include "can_serial.h"
#include "can_serial_error_codes.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Defines --------------------------------------------- */
#define SENDER_RECEIVER_ID 0U

/* Notes ----------------------------------------------- */

/* Variable declaration -------------------------------- */

/* Type definitions ------------------------------------ */

/* Extern declarations --------------------------------- */
extern void CANSerial_printMessageShort(const canMessage_t * const pMsg);

/* Support functions ----------------------------------- */
int inputMessage(const uint8_t pID, 
    const uint32_t pCOBID,
    const uint8_t pSize,
    const uint8_t * const pData,
    const uint32_t pFlags)
{
    (void)pID;

    /* Set up CAN message */
    canMessage_t lMsg;
    lMsg.id = pCOBID;
    lMsg.size = pSize;
    lMsg.flags = pFlags;

    for(uint8_t i = 0U; (i < CAN_MESSAGE_MAX_SIZE) && (i < lMsg.size); i++) {
        lMsg.data[i] = pData[i];
    }

    CANSerial_printMessageShort(&lMsg);

    return 0;
}

/* ----------------------------------------------------- */
/* Main ------------------------------------------------ */
/* ----------------------------------------------------- */
int main(const int argc, const char * const * const argv) {
    if(argc != 2) {
        printf("[ERROR] Wrorng argument, please give serial port as argument.\n");
        exit(EXIT_FAILURE);
    }

    unsigned int lErrorCode = 0U;
    const char *lPort = (const char *)(argv + 1U);

    /* Create a CANSerial module */
    uint8_t lID = 0U;
    if(1U != (lErrorCode = CANSerial_createModule(&lID))) {
        printf("[ERROR] CANSerial_createModule failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    /* Initialize the CAN over serial module */
    if(1U != (lErrorCode = CANSerial_init(lID, CAN_SERIAL_MODE_NORMAL, lPort))) {
        printf("[ERROR] CANSerial_init failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    if(1U != (lErrorCode = CANSerial_setPutMessageFunction(lID, SENDER_RECEIVER_ID, inputMessage))) {
        printf("[ERROR] CANSerial_setPutMessageFunction failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    /* Set up CAN message */
    canMessage_t lMsg = {
        0x701U,
        8U,
        {
            0x01U,
            0x23U,
            0x45U,
            0x67U,
            0x89U,
            0xABU,
            0xCDU,
            0xEFU
        },
        0x00000000U
    };

    if(1U != (lErrorCode = CANSerial_startRxThread(lID))) {
        printf("[ERROR] CANSerial_startRxThread failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    ssize_t lReadBytes = 0;

    /* Send the CAN message over Serial */
    while(lErrorCode == CAN_SERIAL_ERROR_NONE && 0 >= lReadBytes) {
        if(1U != (lErrorCode = CANSerial_send(lID, lMsg.id, lMsg.size, lMsg.data, lMsg.flags))) {
            printf("[ERROR] CANSerial_send failed w/ error code %u.\n", lErrorCode);
            exit(EXIT_FAILURE);
        }

        sleep(1U);
    }

    if(CAN_SERIAL_ERROR_NONE != lErrorCode) {
        printf("[ERROR] sender-receiver failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    CANSerial_printMessage(&lMsg);

    return EXIT_SUCCESS;
}
