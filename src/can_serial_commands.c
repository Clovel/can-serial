/**
 * @brief CAN over serial command implementations
 * 
 * @file can_serial_commands.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"
#include "can_serial_commands.h"

/* C system */
#include <stdio.h>      /* printf() */
#include <string.h>     /* strerror() */
#include <sys/types.h>  /* read() */
#include <sys/uio.h>    /* read() */
#include <unistd.h>     /* read() */
#include <sys/time.h>   /* FD timout */

/* errno */
#include <errno.h>

/* Defines --------------------------------------------- */
#define CAN_SERIAL_ANSWER_TIMEOUT_US    100U    /**< Timeout us value */
#define CAN_SERIAL_ANSWER_TIMEOUT_S     0U      /**< Timeout s value */

/**
 * @brief Maximum answer size (6 + 2 for margin)
 * 
 * @details According to the Lawicel CANUSB document,
 * it seems that the maximum size for a command's
 * answer is 6 bytes.
 */
#define CAN_SERIAL_ANSWER_MAX_SIZE      8U

#define CAN_SERIAL_CMD_BELL             "\x07" /**< Indicates an error */
#define CAN_SERIAL_CMD_OK               "\r"   /**< "OK" answer */
#define CAN_SERIAL_CMD_OPEN             "O\r"  /**< Open CAN channel cmd */
#define CAN_SERIAL_CMD_CLOSE            "C\r"  /**< Close CAN channel cmd */
#define CAN_SERIAL_CMD_SN               "N\r"  /**< Get Serial Number cmd */
#define CAN_SERIAL_CMD_VN               "V\r"  /**< Get Version Number cmd */
#define CAN_SERIAL_CMD_STAUTS_FLAGS     "F\r"  /**< Read Status Flags cmd */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Static variables ------------------------------------ */
/**
 * @brief This tieval constant ill be used to set the
 * file descriptor read timeout when waiting for the
 * serial device's answer
 */
static const struct timeval sTimeOut = {
    .tv_sec  = CAN_SERIAL_ANSWER_TIMEOUT_S,
    .tv_usec = CAN_SERIAL_ANSWER_TIMEOUT_US
};

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];

/* Serial port command functions ----------------------- */
/**
 * @brief Send a command to the CANUSB device
 * via the serial port
 * 
 * @param[in]   pID         The Id of the CAN driver
 * @param[in]   pCmd        The string representing the command
 * @param[out]  pResponse   The answer received from the CAN device
 * 
 * $@details
 * 
 * @return Error code
 */
static canSerialErrorCode_t CANSerial_sendCmd(const canSerialID_t pID,
    const char * const pCmd,
    char * const pAnswer)
{
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_sendCmd> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Chack the arg ptrs */
    if(NULL == pCmd) {
        printf("[ERROR] <CANSerial_sendCmd> Argument pCmd ptr is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }
    // if(NULL == pAnswer) {
    //     printf("[ERROR] <CANSerial_sendCmd> Argument pAnswer ptr is NULL\n");
    //     return CAN_SERIAL_ERROR_ARG;
    // }

    size_t lStrLen = strlen(pCmd);

    /* Check that the command is at least 2 characters long */
    if(2U > lStrLen) {
        printf("[ERROR] <CANSerial_sendCmd> The command must be at least 2 characters long\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Check that the last character is a CR */
    if('\r' != pCmd[lStrLen - 1U]) {
        printf("[ERROR] <CANSerial_sendCmd> The command must end w/ a CR\n");
        return CAN_SERIAL_ERROR_ARG;
    }

    /* Lock the mutex to block the reception thread.
     * We will wait for our answer here, 
     * so we block the reception thread to prevent it
     * from receiving our answer.
     */
    pthread_mutex_lock(&gCANSerial[pID].mutex);

    errno = 0;
    int lSentBytes = write(gCANSerial[pID].fd, pCmd, lStrLen);
    if(0 > lSentBytes) {
        printf("[ERROR] <CANSerial_sendCmd> Failed to send \"%s\" command !\n", pCmd);
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }

        return CAN_SERIAL_ERROR_SYS;
    }

    /* FD set declaration */
    fd_set lInputSet;
    /* Empty the FD Set */
    FD_ZERO(&lInputSet);
    /* Listen to the input descriptor */
    FD_SET(gCANSerial[pID].fd, &lInputSet);

    int  lReadBytes = 0;
    char lBuf[CAN_SERIAL_ANSWER_MAX_SIZE];

    /* Wait for an answer.
     *
     *  select :
     * - first parameter is number of FDs in the set, 
     * - second is our FD set for reading,
     * - third is the FD set in which any write activity needs to updated,
     *   which is not required in this case. 
     * - fourth is the timeout
     */
    errno = 0;
    int lAvailable = select(1, &lInputSet, NULL, NULL, (struct timeval *)&sTimeOut);
    if(0 > lAvailable) {
        /* Some error has occured in the input */
        printf("[ERROR] <CANSerial_sendCmd> select failed\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }

        return CAN_SERIAL_ERROR_SYS;
    } else if (0 < lAvailable) {
        /* Non-blocking read on the serial port */
        errno = 0;
        lReadBytes = read(gCANSerial[pID].fd, (void *)lBuf, CAN_SERIAL_ANSWER_MAX_SIZE - 1U);
        if(0 != errno) {
            printf("[ERROR] <CANSerial_sendCmd> Read failed\n");
            printf("        errno = %d (%s)\n", errno, strerror(errno));

            /* Unlock the mutex */
            pthread_mutex_unlock(&gCANSerial[pID].mutex);

            return CAN_SERIAL_ERROR_SYS;
        }

        /* Add a NULL terminator to the received string */
        lBuf[lReadBytes] = '\0';
    }

    /* Unlock the mutex */
    pthread_mutex_unlock(&gCANSerial[pID].mutex);

    if(0 < lReadBytes) {
        /* Copy the output */
        strncpy(pAnswer, lBuf, CAN_SERIAL_ANSWER_MAX_SIZE);

        return CAN_SERIAL_ERROR_NONE;
    } else {
        printf("[ERROR] <CANSerial_sendCmd> Device failed to answer our command\n");
        return CAN_SERIAL_ERROR_NET;
    }
}

canSerialErrorCode_t CANSerial_sendOpenChannelCmd(const canSerialID_t pID) {
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_sendOpenChannelCmd> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    char lBuf[CAN_SERIAL_ANSWER_MAX_SIZE];

    /* Send the command */
    if(CAN_SERIAL_ERROR_NONE != CANSerial_sendCmd(pID, CAN_SERIAL_CMD_OPEN, lBuf)) {
        printf("[ERROR] <CANSerial_sendOpenChannelCmd> Failed to send the command \"%s\"\n", CAN_SERIAL_CMD_OPEN);
        return CAN_SERIAL_ERROR_SYS;
    }

    /* Check the answer */
    if(0 == strncmp(CAN_SERIAL_CMD_OK, lBuf, sizeof(CAN_SERIAL_CMD_OK))) {
        /* Got "OK " */
        return CAN_SERIAL_ERROR_NONE;
    } else if(0 == strncmp(CAN_SERIAL_CMD_BELL, lBuf, sizeof(CAN_SERIAL_CMD_BELL))) {
        /* Got "Bell" */
        printf("[ERROR] <CANSerial_sendOpenChannelCmd> Got \"BELL\" (error) from device");
        return CAN_SERIAL_ERROR_NET;
    } else {
        /* Unknown answer... */
        printf("[ERROR] <CANSerial_sendOpenChannelCmd> Got unknown answer from device : %s", lBuf);
        return CAN_SERIAL_ERROR_NET;
    }
}

canSerialErrorCode_t CANSerial_sendCloseChannelCmd(const canSerialID_t pID) {
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_sendCloseChannelCmd> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    char lBuf[CAN_SERIAL_ANSWER_MAX_SIZE];

    /* Send the command */
    if(CAN_SERIAL_ERROR_NONE != CANSerial_sendCmd(pID, CAN_SERIAL_CMD_CLOSE, lBuf)) {
        printf("[ERROR] <CANSerial_sendCloseChannelCmd> Failed to send the command \"%s\"\n", CAN_SERIAL_CMD_OPEN);
        return CAN_SERIAL_ERROR_SYS;
    }

    /* Check the answer */
    if(0 == strncmp(CAN_SERIAL_CMD_OK, lBuf, sizeof(CAN_SERIAL_CMD_OK))) {
        /* Got "OK " */
        return CAN_SERIAL_ERROR_NONE;
    } else if(0 == strncmp(CAN_SERIAL_CMD_BELL, lBuf, sizeof(CAN_SERIAL_CMD_BELL))) {
        /* Got "Bell" */
        printf("[ERROR] <CANSerial_sendCloseChannelCmd> Got \"BELL\" (error) from device");
        return CAN_SERIAL_ERROR_NET;
    } else {
        /* Unknown answer... */
        printf("[ERROR] <CANSerial_sendCloseChannelCmd> Got unknown answer from device : %s", lBuf);
        return CAN_SERIAL_ERROR_NET;
    }
}

canSerialErrorCode_t CANSerial_sendMsgCmd(const canSerialID_t pID, const canMessage_t * const pMsg) {
    if(!CANSerial_moduleExists(pID)) {
        printf("[ERROR] <CANSerial_sendCloseChannelCmd> No CANSerial module has the ID %u\n", pID);
        return CAN_SERIAL_ERROR_ARG;
    }

    if(NULL == pMsg) {
        printf("[ERROR] <CANSerial_sendMsgCmd> Message ptr is NULL\n");
        return CAN_SERIAL_ERROR_ARG;
    }

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
     * Numbers of dd pairsmust match the data length, otherwise an error will occur.
     * [CR] is the end of command identifier. 
     */

    char lBuf[CAN_SERIAL_ANSWER_MAX_SIZE];

    /* Send the command */
    if(CAN_SERIAL_ERROR_NONE != CANSerial_sendCmd(pID, CAN_SERIAL_CMD_CLOSE, lBuf)) {
        printf("[ERROR] <CANSerial_sendCloseChannelCmd> Failed to send the command \"%s\"\n", CAN_SERIAL_CMD_OPEN);
        return CAN_SERIAL_ERROR_SYS;
    }

    /* Check the answer */
    if(0 == strncmp(CAN_SERIAL_CMD_OK, lBuf, sizeof(CAN_SERIAL_CMD_OK))) {
        /* Got "OK " */
        return CAN_SERIAL_ERROR_NONE;
    } else if(0 == strncmp(CAN_SERIAL_CMD_BELL, lBuf, sizeof(CAN_SERIAL_CMD_BELL))) {
        /* Got "Bell" */
        printf("[ERROR] <CANSerial_sendCloseChannelCmd> Got \"BELL\" (error) from device");
        return CAN_SERIAL_ERROR_NET;
    } else {
        /* Unknown answer... */
        printf("[ERROR] <CANSerial_sendCloseChannelCmd> Got unknown answer from device : %s", lBuf);
        return CAN_SERIAL_ERROR_NET;
    }
}
