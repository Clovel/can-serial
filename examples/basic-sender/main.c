/**
 * @brief CAN over serial basic sender example
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

/* Notes ----------------------------------------------- */

/* Variable declaration -------------------------------- */

/* Type definitions ------------------------------------ */

/* Support functions ----------------------------------- */

/* ----------------------------------------------------- */
/* Main ------------------------------------------------ */
/* ----------------------------------------------------- */
int main(const int argc, const char * const * const argv) {
    (void)argc;
    (void)argv;

    unsigned int lErrorCode = 0U;

    /* Initialize the CAN over serial module */
    if(1U != (lErrorCode = CANSerial_init(0U, can_serial_MODE_NORMAL, 15024))) {
        printf("[ERROR] CANSerial_init failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    /* Set up CAN message */
    const cipMessage_t lMsg = {
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
    
    /* Send the CAN message over IP */
    while(can_serial_ERROR_NONE == lErrorCode) {
        if(1U != (lErrorCode = CANSerial_send(0U, lMsg.id, lMsg.size, lMsg.data, lMsg.flags))) {
            printf("[ERROR] CANSerial_send failed w/ error code %u.\n", lErrorCode);
            exit(EXIT_FAILURE);
        }

        CANSerial_printMessageShort(&lMsg);

        sleep(1U);
    }

    return EXIT_SUCCESS;
}
