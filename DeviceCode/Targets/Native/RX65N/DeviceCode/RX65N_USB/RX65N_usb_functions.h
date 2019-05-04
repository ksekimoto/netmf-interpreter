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

#ifndef RX65N_USB_FUNCTIONS_H_
#define RX65N_USB_FUNCTIONS_H_

#define USB_EP_TX_DIS       0
#define USB_EP_TX_STALL     1
#define USB_EP_TX_NAK       2
#define USB_EP_TX_VALID     3
#define USB_EP_RX_DIS       0
#define USB_EP_RX_STALL     1
#define USB_EP_RX_NAK       2
#define USB_EP_RX_VALID     3

#define USB_MAX_EP      6
#define MAX_EP0_SIZE    64
#define USB_MAX_DATA_PACKET_SIZE    64

#define USB_MAX_PIPE    10  // max number of pipes
#define USB_MAX_FIFO    3   // max number of fifo
#define FIFO_CFIFO      0   // CFIFO
#define FIFO_D0FIFO     1   // DOFIFO
#define FIFO_D1FIFO     2   // D1FIFO

#define MBW_BYTE        0   // byte access
#define MBW_WORD        1   // word access

#define PIPE_BULK_ST    1   // pipe bulk start no
#define PIPE_INT_ST     6   // pipe interrupt startr no

#define PID_NAK     0
#define PID_BUF     1
#define PID_STALL_1 2
#define PID_STALL_2 3

#define DIR_IN      1       // Write
#define DIR_OUT     0       // Read

#define MASK_BEMPE  (0x0400)
#define MASK_NRDYE  (0x0200)
#define MASK_BRDYE  (0x0100)

//*********************************************************************************
// Type Definitions
//*********************************************************************************
typedef union {
    unsigned short WORD;
    struct {
        unsigned short PID:2;
        unsigned short :3;
        unsigned short PBUSY:1;
        unsigned short SQMON:1;
        unsigned short SQSET:1;
        unsigned short SQCLR:1;
        unsigned short ACLRM:1;
        unsigned short ATREPM:1;
        unsigned short :3;
        unsigned short INBUFM:1;
        unsigned short BSTS:1;
    } BIT;
} PIPECTR;

#endif /* RX65N_USB_FUNCTIONS_H_ */
