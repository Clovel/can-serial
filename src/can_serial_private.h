/**
 * @brief CAN over serial private header
 * 
 * @file can_serial_private.h
 */

#ifndef CAN_SERIAL_PRIVATE_H
#define CAN_SERIAL_PRIVATE_H

/* Includes -------------------------------------------- */
#include "can_serial.h"

#include <netinet/in.h>
#include <pthread.h>

#include <stdint.h>  /* TODO : Delete this and use custom types */
#include <stdbool.h> /* TODO : Delete this and use custom types */

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */
typedef int canSerialPortFd_t;

typedef struct _canSerialInternalVars {
    uint8_t         instanceID;
    canSerialMode_t mode;
    bool            isInitialized;
    bool            isCreated;
    bool            isStopped;

    /* Socket */
    canSerialPortFd_t   fd;  /* The file descriptor used to communicate CAN frames */
    canSerialPort_t     serialPort; /* Serial port */

    /* Rx Thread */
    pthread_t thread;
    bool rxThreadOn;
    uint8_t callerID;
    canSerialPutMessageFct_t putMessageFct;
    pthread_mutex_t mutex;
} canSerialInternalVars_t;

/* Private functions ----------------------------------- */
canSerialErrorCode_t CANSerial_startRxThread(const canSerialID_t pID);

/* Utility functions ----------------------------------- */
bool CANSerial_moduleExists(const canSerialID_t pID);

#endif /* CAN_SERIAL_PRIVATE_H */
