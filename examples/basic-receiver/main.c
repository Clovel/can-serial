/**
 * @brief CAN over serial basic sender example
 * 
 * @file main.c
 */

/* Includes -------------------------------------------- */
/* can-ip */
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
    (void)argc;
    (void)argv;

    unsigned int lErrorCode = 0U;

    /* Initialize the CAN over serial module */
    if(1U != (lErrorCode = CIP_init(0U, can_serial_MODE_NORMAL, 15024))) {
        printf("[ERROR] CIP_init failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    /* Set up CAN message */
    cipMessage_t lMsg = {
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
    while(lErrorCode == can_serial_ERROR_NONE) {
        lErrorCode = CIP_recv(0U, &lMsg, &lReadBytes);
        if(0 < lReadBytes && sizeof(cipMessage_t) == lReadBytes) {
            CIP_printMessageShort(&lMsg);
        }
    }

    if(can_serial_ERROR_NONE != lErrorCode) {
        printf("[ERROR] CIP_recv failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
