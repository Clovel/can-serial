/**
 * @brief CAN over serial API send functions
 * 
 * @file can_serial_send.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"
#include "can_serial_commands.h"

/* C system */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* errno */
#include <errno.h>

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Static variables ------------------------------------ */

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];


canSerialErrorCode_t CANSerial_send(const canSerialID_t pID,
    const uint32_t pCANID,
    const uint8_t pSize,
    const uint8_t * const pData,
    const uint32_t pFlags)
{
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_send> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    if(NULL == pData) {
        printf("[ERROR] <CANSerial_send> Message ptr is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    if(CAN_MESSAGE_MAX_SIZE < pSize) {
        printf("[ERROR] <CANSerial_send> CAN ID out of bounds (%u)\n", pSize);
        return CAN_SERIAL_ERROR_ARG;
    }

    canSerialErrorCode_t lErrorCode = CAN_SERIAL_ERROR_NONE;

    char *lCmd = (char *)malloc(CAN_SERIAL_ANSWER_MAX_SIZE);
    char *lBuf = (char *)malloc(CAN_SERIAL_ANSWER_MAX_SIZE);
    memset(lCmd, 0, CAN_SERIAL_ANSWER_MAX_SIZE);
    memset(lBuf, 0, CAN_SERIAL_ANSWER_MAX_SIZE);

    /* Build the command string.
     * The format for a CAN message is : 
     * 
     * For a standard CAN frame : 
     * tiiildd..[CR]
     * with :
     * - t : command identifier : standard frame
     * - iii : CAN-ID in HEX (11 bits, 0x000-0x7FF)
     * - l : DLC
     * - dd.. : Byte value in hex (00-FF).
     *
     * For an extended CAN frame :
     * Tiiiiiiiildd..[CR]
     * - T : command identifier : extended frame
     * - iiiiiiii : CAN-ID in HEX (29 bits, 00000000-1FFFFFFF)
     * - l : DLC
     * - dd.. : Byte value in hex (00-FF).
     * 
     * Numbers of dd pairs must match the data length, otherwise an error will occur.
     * [CR] is the end of command identifier.
     */
    if(CAN_SERIAL_ID_EXT_FLAG == (CAN_SERIAL_ID_MASK & pFlags)) {
        /* Extended ID */

        if(CAN_SERIAL_ID_EXT_MASK < pCANID) {
            printf("[ERROR] <CANSerial_send> CAN ID out of bounds (%u)\n", pCANID);
            return CAN_SERIAL_ERROR_ARG;
        }

        /* Command identifier */
        *lCmd = 'T';
        lCmd++;

        /* CAN ID */
        snprintf(lCmd, 8U, "%X", pID);
        lCmd += 8U;
    } else {
        /* Standard ID */
        if(CAN_SERIAL_ID_STD_MASK < pCANID) {
            printf("[ERROR] <CANSerial_send> CAN ID out of bounds (%u)\n", pCANID);
            return CAN_SERIAL_ERROR_ARG;
        }

        /* Command identifier */
        *lCmd = 't';
        lCmd++;

        /* CAN ID */
        snprintf(lCmd, 3U, "%X", pID);
        lCmd += 3U;
    }

    /* DLC */
    *lCmd = pSize;
    lCmd++;

    /* Data */
    for(uint8_t i = 0U; (i < pSize); i++) {
        snprintf(lCmd, 1U, "%X", *(pData + i));
        lCmd++;
    }

    /* End-of-command CR */
    *lCmd = CAN_SERIAL_END_OF_CMD;

    pthread_mutex_lock(&gCANSerial[pID].mutex);

    /* Send the command */
    if(CAN_SERIAL_ERROR_NONE != CANSerial_sendCmd(pID, lCmd, lBuf)) {
        printf("[ERROR] <CANSerial_send> Failed to send the command \"%s\"\n", CAN_SERIAL_CMD_OPEN);
        return CAN_SERIAL_ERROR_SYS;
    }

    pthread_mutex_unlock(&gCANSerial[pID].mutex);

    /* Check the answer */
    if(0 == strncmp(CAN_SERIAL_CMD_OK, lBuf, sizeof(CAN_SERIAL_CMD_OK))) {
        /* Got "OK " */
        lErrorCode = CAN_SERIAL_ERROR_NONE;
    } else if(0 == strncmp(CAN_SERIAL_CMD_BELL, lBuf, sizeof(CAN_SERIAL_CMD_BELL))) {
        /* Got "Bell" */
        printf("[ERROR] <CANSerial_send> Got \"BELL\" (error) from device");
        lErrorCode = CAN_SERIAL_ERROR_NET;
    } else {
        /* Unknown answer... */
        printf("[ERROR] <CANSerial_send> Got unknown answer from device : %s", lBuf);
        lErrorCode = CAN_SERIAL_ERROR_NET;
    }

    /* Free allocated memory */
    free(lCmd);
    free(lBuf);

    return lErrorCode;
}
