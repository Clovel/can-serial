/**
 * @brief CAN over serial basic sender example
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

/* Notes ----------------------------------------------- */

/* Variable declaration -------------------------------- */

/* Type definitions ------------------------------------ */

/* Support functions ----------------------------------- */

/* ----------------------------------------------------- */
/* Main ------------------------------------------------ */
/* ----------------------------------------------------- */
int main(const int argc, const char * const * const argv) {
    
    if(argc != 2) {
        printf("[ERROR] Wrorng argument, please give serial port as argument.\n");
        exit(EXIT_FAILURE);
    }

    unsigned int lErrorCode = 0U;
    const char *lPort = *(argv + 1U);

    /* Initialize the CAN over serial module */
    if(1U != (lErrorCode = CANSerial_init(0U, CAN_SERIAL_MODE_NORMAL, lPort))) {
        printf("[ERROR] CANSerial_init failed w/ error code %u.\n", lErrorCode);
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
        0x00000000U,
        0x00000000U
    };

    ssize_t lReadBytes = 0;

    /* Receive the CAN message over IP */
    while(lErrorCode == CAN_SERIAL_ERROR_NONE) {
        lErrorCode = CANSerial_recv(0U, &lMsg, &lReadBytes);
        if(0 < lReadBytes && sizeof(canMessage_t) == lReadBytes) {
            CANSerial_printMessageShort(&lMsg);
        }
    }

    if(CAN_SERIAL_ERROR_NONE != lErrorCode) {
        printf("[ERROR] CANSerial_recv failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
