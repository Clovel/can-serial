/**
 * @brief CAN over serial socket management functions
 * 
 * @file can_serial_socket_mgt.c
 */

/* Includes -------------------------------------------- */
#include "can_serial_private.h"
#include "can_serial_error_codes.h"

/* Networking headers */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <ifaddrs.h>

/* C system */
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* errno */
#include <errno.h>

/* Defines --------------------------------------------- */

/* Type definitions ------------------------------------ */

/* Global variables ------------------------------------ */

/* Static variables ------------------------------------ */

/* Extern variables ------------------------------------ */
extern canSerialInternalVars_t gCANSerial[CAN_SERIAL_MAX_NB_MODULES];

/* Socket management functions ------------------------- */
static cipErrorCode_t listNetItfs(void) {
    /* List available network interfaces */
    struct ifaddrs *lIfAddrs = NULL;

    if(0 > getifaddrs(&lIfAddrs)) {
        printf("[ERROR] <CANSerial_initcanSocket> getifaddrs failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }

        return CAN_SERIAL_ERROR_NET;
    }

    struct ifaddrs *lTmpIfAddr = lIfAddrs;
    while (lTmpIfAddr) 
    {
        if (lTmpIfAddr->ifa_addr && lTmpIfAddr->ifa_addr->sa_family == PF_INET)
        {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)lTmpIfAddr->ifa_addr;
            printf("%s: %s\n", lTmpIfAddr->ifa_name, inet_ntoa(pAddr->sin_addr));
        }

        lTmpIfAddr = lTmpIfAddr->ifa_next;
    }

    freeifaddrs(lIfAddrs);

    return CAN_SERIAL_ERROR_NONE;
}

cipErrorCode_t CANSerial_initCanSocket(const canSerialID_t pID) {
    (void) pID;

    /* Construct local address structure */
    gCANSerial[pID].socketInAddress.sin_family         = PF_INET;
    gCANSerial[pID].socketInAddress.sin_port           = htons(gCANSerial[pID].canPort);
    // gCANSerial[pID].socketInAddress.sin_addr.s_addr    = inet_addr(gCANSerial[pID].canIP);
    gCANSerial[pID].socketInAddress.sin_addr.s_addr    = INADDR_ANY; /* Set it to INADDR_ANY to bind */

    printf("[DEBUG] <CANSerial_initcanSocket> IPAddr = %s\n", gCANSerial[pID].canIP);
    printf("[DEBUG] <CANSerial_initcanSocket> Port   = %d\n", gCANSerial[pID].canPort);
    
    /* Create the UDP socket (DGRAM for UDP */
    errno = 0;
    if(0 > (gCANSerial[pID].canSocket = socket(gCANSerial[pID].socketInAddress.sin_family, SOCK_DGRAM, IPPROTO_IP))) {
        printf("[ERROR] <CANSerial_initcanSocket> socket failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }

        return CAN_SERIAL_ERROR_NET;
    }

    /* set socket options */
    errno = 0;

    /* Configure the socket for broadcast */
    const int lBroadcastPermission = 1;
    if(0 > setsockopt(gCANSerial[pID].canSocket, SOL_SOCKET, SO_BROADCAST, (const void *)&lBroadcastPermission, sizeof(lBroadcastPermission))) {
        printf("[ERROR] <CANSerial_initcanSocket> setsockopt SO_BROADCAST failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    /* Set the address to be reusable */
    int lEnable = 1;
    if(0 > setsockopt(gCANSerial[pID].canSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&lEnable, sizeof(lEnable))) {
        printf("[ERROR] <CANSerial_initcanSocket> setsockopt SO_REUSEADDR failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    /* Set the port to be reusable */
    if(0 > setsockopt(gCANSerial[pID].canSocket, SOL_SOCKET, SO_REUSEPORT, (const void *)&lEnable, sizeof(lEnable))) {
        printf("[ERROR] <CANSerial_initcanSocket> setsockopt SO_REUSEPORT failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    /* Set the socket as non-blocking */
    int lFlags = 0;
    if(0 > (lFlags = fcntl(gCANSerial[pID].canSocket, F_GETFL))) {
        printf("[ERROR] <CANSerial_initcanSocket> fcntl F_GETFL failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    lFlags |= O_NONBLOCK;
    if(0 > fcntl(gCANSerial[pID].canSocket, F_SETFL, lFlags)) {
        printf("[ERROR] <CANSerial_initcanSocket> fcntl F_SETFL failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    /* Get socket information */
    // struct addrinfo lHints = {
    //     .ai_family = AF_UNSPEC,
    //     .ai_socktype = SOCK_DGRAM,
    //     .ai_protocol = IPPROTO_UDP
    // };

    // char lPortStr[16U] = "";
    // if(0 > snprintf(lPortStr, 16U, "%d", gCANSerial[pID].canPort)) {
    //     printf("[ERROR] <CANSerial_initcanSocket> snprintf for port failed !\n");
    //     return CAN_SERIAL_ERROR_SYS;
    // }

    // int lFctReturn = getaddrinfo(gCANSerial[pID].canIP, lPortStr, &lHints, &gCANSerial[pID].addrinfo);
    // if(0 != lFctReturn || NULL == gCANSerial[pID].addrinfo) {
    //     printf("[ERROR] <CANSerial_initcanSocket> getaddrinfo failed !\n");
    //     printf("        Invalid address (%s) or port (%d)\n", gCANSerial[pID].canIP, gCANSerial[pID].canPort);
    //     return CAN_SERIAL_ERROR_NET;
    // }

    /* Bind socket for reception */
    errno = 0;
    if(0 > bind(gCANSerial[pID].canSocket, (struct sockaddr *)&gCANSerial[pID].socketInAddress, sizeof(gCANSerial[pID].socketInAddress))) {
        printf("[ERROR] <CANSerial_initcanSocket> bind failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    /* Set the sockInAddress to the specified broadcast address for sending */
    gCANSerial[pID].socketInAddress.sin_addr.s_addr = inet_addr("255.255.255.255");
    //printf("[DEBUG] socketInAddress.sin_addr.s_addr = %ld\n", gCANSerial[pID].socketInAddress.sin_addr.s_addr);

    /* Socket initialized */
    return CAN_SERIAL_ERROR_NONE;
}

cipErrorCode_t CANSerial_closeSocket(const canSerialID_t pID) {
    (void)pID;

    /* Close the socket */
    errno = 0;
    if(0 > close(gCANSerial[pID].canSocket)) {
        printf("[ERROR] <CANSerial_initcanSocket> close failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return CAN_SERIAL_ERROR_NET;
    }

    return CAN_SERIAL_ERROR_NONE;
}
