/**
 * @brief CAN over serial private header
 * 
 * @file can_serial_private.h
 */

#ifndef can_serial_PRIVATE_H
#define can_serial_PRIVATE_H

/* Includes -------------------------------------------- */
#include "can_serial.h"

#include <netinet/in.h>
#include <pthread.h>

#include <stdint.h>  /* TODO : Delete this and use custom types */
#include <stdbool.h> /* TODO : Delete this and use custom types */

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */
typedef int cipSocket_t;

typedef struct _cipInternalVariables {
    uint8_t   cipInstanceID; /* TODO : Multiline CAN */
    cipMode_t cipMode;
    bool      isInitialized;
    bool      isStopped;

    /* Random ID */
    uint32_t randID; /**< Random ID to ignore our own messages upon reception */

    /* Socket */
    cipSocket_t         canSocket; /* The socket used to communicate CAN frames */
    struct sockaddr_in  socketInAddress;
    char               *canIP;      /* IP Address */
    cipPort_t           canPort;    /* Server port number */
    struct hostent     *hostPtr;    /* Server information */
    struct addrinfo    *addrinfo;   /* Address information fetched w/ getaddrinfo */

    /* Rx Thread */
    bool rxThreadOn;
    uint8_t callerID;
    cipPutMessageFct_t putMessageFct;
    pthread_mutex_t mutex;
} cipInternalStruct_t;

/* Private functions ----------------------------------- */
cipErrorCode_t CIP_startRxThread(const cipID_t pID);

#endif /* can_serial_PRIVATE_H */
