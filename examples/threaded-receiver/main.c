/**
 * @brief CAN over serial threaded receiver example
 * 
 * @file main.c
 */

/* Includes -------------------------------------------- */
/* CANSerial */
#include "can_serial.h"
#include "can_serial_error_codes.h"

/* C System */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Defines --------------------------------------------- */
#define THREADED_RECEIVER_ID 0U

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

    if(1U != (lErrorCode = CANSerial_setPutMessageFunction(lID, THREADED_RECEIVER_ID, inputMessage))) {
        printf("[ERROR] CANSerial_setPutMessageFunction failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    if(1U != (lErrorCode = CANSerial_startRxThread(lID))) {
        printf("[ERROR] CANSerial_startRxThread failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    /* Receive the CAN message over Serial */
    while(true);

    if(CAN_SERIAL_ERROR_NONE != lErrorCode) {
        printf("[ERROR] CANSerial_recv failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
