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

/* Notes ----------------------------------------------- */

/* Variable declaration -------------------------------- */

/* Type definitions ------------------------------------ */


extern void CIP_printMessageShort(const cipMessage_t * const pMsg);

/* Support functions ----------------------------------- */
int inputMessage(const uint8_t pID, 
    const uint32_t pCOBID,
    const uint8_t pSize,
    const uint8_t * const pData,
    const uint32_t pFlags)
{
    (void)pID;

    /* Set up CAN message */
    cipMessage_t lMsg;
    lMsg.id = pCOBID;
    lMsg.size = pSize;
    lMsg.flags = pFlags;

    for(uint8_t i = 0U; (i < CAN_MESSAGE_MAX_SIZE) && (i < lMsg.size); i++) {
        lMsg.data[i] = pData[i];
    }

    CIP_printMessageShort(&lMsg);

    return 0;
}

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

    if(1U != (lErrorCode = CIP_setPutMessageFunction(0U, 0U, inputMessage))) {
        printf("[ERROR] CIP_setPutMessageFunction failed w/ error code %u.\n", lErrorCode);
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

    if(1U != (lErrorCode = CIP_startRxThread(0U))) {
        printf("[ERROR] CIP_startRxThread failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    ssize_t lReadBytes = 0;

    /* Receive the CAN message over IP */
    while(lErrorCode == can_serial_ERROR_NONE && 0 >= lReadBytes) {
        if(1U != (lErrorCode = CIP_send(0U, lMsg.id, lMsg.size, lMsg.data, lMsg.flags))) {
            printf("[ERROR] CIP_send failed w/ error code %u.\n", lErrorCode);
            exit(EXIT_FAILURE);
        }

        sleep(1U);
    }

    if(can_serial_ERROR_NONE != lErrorCode) {
        printf("[ERROR] sender-receiver failed w/ error code %u.\n", lErrorCode);
        exit(EXIT_FAILURE);
    }

    CIP_printMessage(&lMsg);

    return EXIT_SUCCESS;
}
