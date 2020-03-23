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
extern cipInternalStruct_t gCIP;

/* Socket management functions ------------------------- */
static cipErrorCode_t listNetItfs(void) {
    /* List available network interfaces */
    struct ifaddrs *lIfAddrs = NULL;

    if(0 > getifaddrs(&lIfAddrs)) {
        printf("[ERROR] <CIP_initcanSocket> getifaddrs failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }

        return can_serial_ERROR_NET;
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

    return can_serial_ERROR_NONE;
}

cipErrorCode_t CIP_initCanSocket(const cipID_t pID) {
    (void) pID;

    /* Construct local address structure */
    gCIP.socketInAddress.sin_family         = PF_INET;
    gCIP.socketInAddress.sin_port           = htons(gCIP.canPort);
    // gCIP.socketInAddress.sin_addr.s_addr    = inet_addr(gCIP.canIP);
    gCIP.socketInAddress.sin_addr.s_addr    = INADDR_ANY; /* Set it to INADDR_ANY to bind */

    printf("[DEBUG] <CIP_initcanSocket> IPAddr = %s\n", gCIP.canIP);
    printf("[DEBUG] <CIP_initcanSocket> Port   = %d\n", gCIP.canPort);
    
    /* Create the UDP socket (DGRAM for UDP */
    errno = 0;
    if(0 > (gCIP.canSocket = socket(gCIP.socketInAddress.sin_family, SOCK_DGRAM, IPPROTO_IP))) {
        printf("[ERROR] <CIP_initcanSocket> socket failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }

        return can_serial_ERROR_NET;
    }

    /* set socket options */
    errno = 0;

    /* Configure the socket for broadcast */
    const int lBroadcastPermission = 1;
    if(0 > setsockopt(gCIP.canSocket, SOL_SOCKET, SO_BROADCAST, (const void *)&lBroadcastPermission, sizeof(lBroadcastPermission))) {
        printf("[ERROR] <CIP_initcanSocket> setsockopt SO_BROADCAST failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    /* Set the address to be reusable */
    int lEnable = 1;
    if(0 > setsockopt(gCIP.canSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&lEnable, sizeof(lEnable))) {
        printf("[ERROR] <CIP_initcanSocket> setsockopt SO_REUSEADDR failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    /* Set the port to be reusable */
    if(0 > setsockopt(gCIP.canSocket, SOL_SOCKET, SO_REUSEPORT, (const void *)&lEnable, sizeof(lEnable))) {
        printf("[ERROR] <CIP_initcanSocket> setsockopt SO_REUSEPORT failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    /* Set the socket as non-blocking */
    int lFlags = 0;
    if(0 > (lFlags = fcntl(gCIP.canSocket, F_GETFL))) {
        printf("[ERROR] <CIP_initcanSocket> fcntl F_GETFL failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    lFlags |= O_NONBLOCK;
    if(0 > fcntl(gCIP.canSocket, F_SETFL, lFlags)) {
        printf("[ERROR] <CIP_initcanSocket> fcntl F_SETFL failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    /* Get socket information */
    // struct addrinfo lHints = {
    //     .ai_family = AF_UNSPEC,
    //     .ai_socktype = SOCK_DGRAM,
    //     .ai_protocol = IPPROTO_UDP
    // };

    // char lPortStr[16U] = "";
    // if(0 > snprintf(lPortStr, 16U, "%d", gCIP.canPort)) {
    //     printf("[ERROR] <CIP_initcanSocket> snprintf for port failed !\n");
    //     return can_serial_ERROR_SYS;
    // }

    // int lFctReturn = getaddrinfo(gCIP.canIP, lPortStr, &lHints, &gCIP.addrinfo);
    // if(0 != lFctReturn || NULL == gCIP.addrinfo) {
    //     printf("[ERROR] <CIP_initcanSocket> getaddrinfo failed !\n");
    //     printf("        Invalid address (%s) or port (%d)\n", gCIP.canIP, gCIP.canPort);
    //     return can_serial_ERROR_NET;
    // }

    /* Bind socket for reception */
    errno = 0;
    if(0 > bind(gCIP.canSocket, (struct sockaddr *)&gCIP.socketInAddress, sizeof(gCIP.socketInAddress))) {
        printf("[ERROR] <CIP_initcanSocket> bind failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    /* Set the sockInAddress to the specified broadcast address for sending */
    gCIP.socketInAddress.sin_addr.s_addr = inet_addr("255.255.255.255");
    //printf("[DEBUG] socketInAddress.sin_addr.s_addr = %ld\n", gCIP.socketInAddress.sin_addr.s_addr);

    /* Socket initialized */
    return can_serial_ERROR_NONE;
}

cipErrorCode_t CIP_closeSocket(const cipID_t pID) {
    (void)pID;

    /* Close the socket */
    errno = 0;
    if(0 > close(gCIP.canSocket)) {
        printf("[ERROR] <CIP_initcanSocket> close failed !\n");
        if(0 != errno) {
            printf("        errno = %d (%s)\n", errno, strerror(errno));
        }
        return can_serial_ERROR_NET;
    }

    return can_serial_ERROR_NONE;
}
