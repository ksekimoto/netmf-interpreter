////////////////////////////////////////////////////////////////////////////
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Copyright (C) 2010 Renesas Electronics Corporation. All rights reserved.
//  Portions Copyright (c) Kentaro Sekimoto All rights reserved.
//
////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include "net_decl_lwip.h"
#include "lwip\netif.h"
#include "lwip\tcp.h"
#include "lwip\tcpip.h"
#include "lwip\dhcp.h"
#include "lwip\pbuf.h"
#include "netif\etharp.h"

#include "RX65N_EDMAC_lwip.h"
#include "RX65N_EDMAC_lwip_Adapter.h"

//#define DEBUG_EDMAC

struct netif g_RX_EDMAC_NetIF;

#ifndef HW_INTERRUPT
HAL_COMPLETION InterruptTaskCompletion;
#endif
HAL_COMPLETION LwipUpTimeCompletion;
static BOOL LwipNetworkStatus = 0;
static UINT32 LwipLastIpAddress = 0;

void RX_EDMAC_status_callback(struct netif *netif)
{
    if (LwipLastIpAddress != netif->ip_addr.addr) {
        Network_PostEvent(NETWORK_EVENT_TYPE_ADDRESS_CHANGED, 0);
        LwipLastIpAddress = netif->ip_addr.addr;
    }
#if defined(DEBUG_EDMAC)
    debug_printf("\nLink Update: \r\n");
    debug_printf("IP: %d.%d.%d.%d\r\n",
            (netif->ip_addr.addr >> 0) & 0xFF,
            (netif->ip_addr.addr >> 8) & 0xFF,
            (netif->ip_addr.addr >> 16) & 0xFF,
            (netif->ip_addr.addr >> 24) & 0xFF);
    debug_printf("GW: %d.%d.%d.%d\r\n",
            (netif->gw.addr >> 0) & 0xFF,
            (netif->gw.addr >> 8) & 0xFF,
            (netif->gw.addr >> 16) & 0xFF,
            (netif->gw.addr >> 24) & 0xFF);
#endif
}

#ifndef NETIF_FLAG_ETHERNET
#define NETIF_FLAG_ETHERNET     0x40U
#endif

err_t RX_EDMAC_ethhw_init(struct netif *myNetIf)
{ 
    myNetIf->flags = NETIF_FLAG_IGMP | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_BROADCAST; // Added in 4.3.1
    myNetIf->mtu = RX_EDMAC_MAX_FRAME_SIZE;
    myNetIf->output = etharp_output;
    myNetIf->linkoutput = RX_EDMAC_LWIP_xmit;
    myNetIf->status_callback = RX_EDMAC_status_callback;
    RX_EDMAC_LWIP_open(myNetIf);
    return (err_t)ERR_OK;
}

void lwip_network_uptime_completion(void *arg)
{
    NATIVE_PROFILE_PAL_NETWORK();

    BOOL status = RX_EDMAC_LWIP_GetLinkStatus();
    if (status != LwipNetworkStatus) {
        struct netif* pNetIf = (struct netif*)arg;
        if (status) {
            SOCK_NetworkConfiguration *pNetCfg = &g_NetworkConfig.NetworkInterfaces[0];
            RX_EDMAC_LWIP_AutoNegotiate();
            netif_set_up(pNetIf);

            if (pNetCfg->flags & SOCK_NETWORKCONFIGURATION_FLAGS_DHCP)
                dhcp_start(pNetIf);
            Network_PostEvent(NETWORK_EVENT_TYPE__AVAILABILITY_CHANGED, NETWORK_EVENT_FLAGS_IS_AVAILABLE);
        } else {
            netif_set_down((struct netif*)arg);
            Network_PostEvent(NETWORK_EVENT_TYPE__AVAILABILITY_CHANGED, 0);
        }
        LwipNetworkStatus = status;
    }
    LwipUpTimeCompletion.EnqueueDelta64(UPTIME_PERIOD);
}

void InitCompletions(struct netif *pNetIf)
{
#ifndef HW_INTERRUPT
    InterruptTaskCompletion.InitializeForISR((HAL_CALLBACK_FPN)RX_EDMAC_LWIP_recv, pNetIf);
    InterruptTaskCompletion.EnqueueDelta64(INT_PERIOD);
#endif
    LwipUpTimeCompletion.InitializeForUserMode((HAL_CALLBACK_FPN)lwip_network_uptime_completion, pNetIf);
    LwipUpTimeCompletion.EnqueueDelta64(UPTIME_PERIOD);
}

BOOL Network_Interface_Bind(int index)
{
    return RX_EDMAC_LWIP_Driver::Bind();
}

int  Network_Interface_Open(int index)
{
    return RX_EDMAC_LWIP_Driver::Open(index);
}

BOOL Network_Interface_Close(int index)
{
    return RX_EDMAC_LWIP_Driver::Close();
}

#define MP_HAL_UNIQUE_ID_ADDRESS (0xFE7F7D90)

void get_unique_id(UINT8 *id) {
    UINT32 *p = (UINT32 *)id;
    p[0] = *(UINT32 *)(MP_HAL_UNIQUE_ID_ADDRESS + 0);
    p[1] = *(UINT32 *)(MP_HAL_UNIQUE_ID_ADDRESS + 4);
    p[2] = *(UINT32 *)(MP_HAL_UNIQUE_ID_ADDRESS + 6);
    p[3] = *(UINT32 *)(MP_HAL_UNIQUE_ID_ADDRESS + 12);
}

int RX_EDMAC_LWIP_Driver::Open(int index)
{
    struct ip_addr ipaddr, subnetmask, gateway;
    struct netif *pNetIF;
    int len;
    const SOCK_NetworkConfiguration *iface;
    UINT8 id[16];

    iface = &g_NetworkConfig.NetworkInterfaces[index];
    len = g_RX_EDMAC_NetIF.hwaddr_len;
    if (len == 0 || iface->macAddressLen < len) {
        len = iface->macAddressLen;
        g_RX_EDMAC_NetIF.hwaddr_len = len;
    }
    memcpy(g_RX_EDMAC_NetIF.hwaddr, iface->macAddressBuffer, len);
    get_unique_id((UINT8 *)&id);
    g_RX_EDMAC_NetIF.hwaddr[5] = id[15];
    g_RX_EDMAC_NetIF.hwaddr[4] = id[14];
    g_RX_EDMAC_NetIF.hwaddr[3] = id[13];
    ipaddr.addr     = iface->ipaddr;
    gateway.addr    = iface->gateway;
    subnetmask.addr = iface->subnetmask;

    pNetIF = netif_add(&g_RX_EDMAC_NetIF, &ipaddr, &subnetmask, &gateway, NULL, RX_EDMAC_ethhw_init, ethernet_input);

    netif_set_default(pNetIF);
    LwipNetworkStatus = RX_EDMAC_LWIP_GetLinkStatus();
    if (LwipNetworkStatus == TRUE) {
#if defined(DEBUG_EDMAC)
        debug_printf("Link OK\r\n");
#endif
        netif_set_up(pNetIF);
    } else {
#if defined(DEBUG_EDMAC)
        debug_printf("Link NG\r\n");
#endif
    }
    InitCompletions(pNetIF);
    return g_RX_EDMAC_NetIF.num;
}

BOOL RX_EDMAC_LWIP_Driver::Close(void)
{
    int retVal = -1;

    LwipUpTimeCompletion.Abort();
#ifndef HW_INTERRUPT
    InterruptTaskCompletion.Abort();
#endif
    LwipNetworkStatus = 0;
    netif_set_down(&g_RX_EDMAC_NetIF);
    netif_remove(&g_RX_EDMAC_NetIF);
    RX_EDMAC_LWIP_close(&g_RX_EDMAC_NetIF);
    memset(&g_RX_EDMAC_NetIF, 0, sizeof g_RX_EDMAC_NetIF);
    return true;
}

BOOL RX_EDMAC_LWIP_Driver::Bind(void)
{
    return true;
}

//BOOL RX_EDMAC_LWIP_GetLinkStatus(void)
//{
//    return true;
//}
