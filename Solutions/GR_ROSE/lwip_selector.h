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
//  Portions Copyright (c) Kentaro Sekimoto All rights reserved.
//
////////////////////////////////////////////////////////////////////////////

#define NETWORK_MEMORY_PROFILE_LWIP__small          1
#define TCPIP_LWIP                                  1

#define LWIP_DHCP                       1
#define LWIP_TCP                        1
#define LWIP_UDP                        1
#define LWIP_DNS                        1
#define LWIP_ICMP                       1
#define LWIP_IGMP                       1
#define LWIP_ARP                        1
#define LWIP_AUTOIP                     0
#define LWIP_SNMP                       0

#define LWIP_NETIF_LOOPBACK             1
#define LWIP_HAVE_LOOPIF                1

#ifdef _DEBUG
#define LWIP_DEBUG                      1
#endif
