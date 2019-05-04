
////////////////////////////////////////////////////////////////////////////
// Portions Copyright (c) 2011-2012 Kentaro Sekimoto  All rights reserved.
////////////////////////////////////////////////////////////////////////////
/*
 * TestlwIP.c
 *
 *  Created on: 2011/12/17
 *      Author: Kentaro Sekimoto
 */
#ifndef __GNUC__
#define __GNUC__
#endif
#include <tinyhal.h>

#ifdef TEST_LWIP
#include "..\..\..\..\DeviceCode\pal\net\Network_defines_lwip.h"
#include "..\..\..\..\DeviceCode\pal\net\net_decl_lwip.h"
#include "../../../DeviceCode/pal/lwip.43/SocketsDriver/lwIP_Sockets.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/ipv4/lwip/ip_addr.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/netif/etharp.h"
//#include "netif\loopif.h"
//#include "loopback_lwip_driver.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/lwip/arch.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/lwip/init.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/lwip/tcpip.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/lwip/opt.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/lwip/netif.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/lwip/tcp.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/lwip/udp.h"
#include "../../../DeviceCode/pal/lwip.43/LWIP/src/include/lwip/sockets.h"

void  Network_PostEvent(UINT32 evt, UINT32 data)
{
}
#define BUF_SIZE    1518
char buffer[BUF_SIZE];
char HTTP_HEADER[] =
    "HTTP/1.0 200 OK\r\n"
    "Content-Length: 20\r\n"
    "Content-Type: text/html\r\n\r\n"
    "Hello\r\n";

void TestlwIP(void)
{
    SOCK_SOCKET server;
    SOCK_SOCKET client;
    SOCK_sockaddr serverAddress;
    SOCK_sockaddr clientAddress;
    struct ip_addr AnyIpAddrLB;
    IP4_ADDR(&AnyIpAddrLB, 0,0,0,0);

    dbg_printf("========== TestLWIP ==========\r\n");
    dbg_printf("Waiting for Link Update\r\n");
//  LWIP_SOCKETS_Driver::Initialize();
    Events_WaitForEvents(0, 3000);
    server = LWIP_SOCKETS_Driver::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1)
        dbg_printf("Socket NG\r\n");
    else
        dbg_printf("Socket OK\r\n");
    ((SOCK_sockaddr_in*)&serverAddress)->sin_family = AF_INET;
    ((SOCK_sockaddr_in*)&serverAddress)->sin_port = htons(80);
    ((SOCK_sockaddr_in*)&serverAddress)->sin_addr.S_un.S_addr = *(u32_t*)&AnyIpAddrLB;
    int len = sizeof(serverAddress);
    if (LWIP_SOCKETS_Driver::Bind(server, &serverAddress, len) == -1)
        dbg_printf("Bind NG\r\n");
    else
        dbg_printf("Bind OK\r\n");
    if (LWIP_SOCKETS_Driver::Listen(server, 4) == -1)
        dbg_printf("Listen NG\r\n");
    else
        dbg_printf("Listen OK\r\n");
    ((SOCK_sockaddr_in*)&clientAddress)->sin_family = AF_INET;
    ((SOCK_sockaddr_in*)&clientAddress)->sin_port = htons(80);
    ((SOCK_sockaddr_in*)&clientAddress)->sin_addr.S_un.S_addr = *(u32_t*)&AnyIpAddrLB;
    while (true) {
        do {
            client = LWIP_SOCKETS_Driver::Accept(server, &clientAddress, &len);
            Events_WaitForEvents(0,100);
        } while (client == -1);
        int recvd;
        do {
            recvd = LWIP_SOCKETS_Driver::Recv(client, buffer, sizeof(buffer), 0);
            if (recvd == -1) {
                dbg_printf("Receive NG errno=%d\r", errno);
                Events_WaitForEvents(0, 5000);
            }
        } while (recvd == -1);
        buffer[recvd] = 0;
        dbg_printf("received request %d bytes\r\n", recvd);
        if (LWIP_SOCKETS_Driver::Send(client, HTTP_HEADER, sizeof(HTTP_HEADER), 0) == -1) {
            dbg_printf("Send NG errno=%d\r\n", errno);
        }
        LWIP_SOCKETS_Driver::Close(client);
    }
    LWIP_SOCKETS_Driver::Close(server);
}
#endif
