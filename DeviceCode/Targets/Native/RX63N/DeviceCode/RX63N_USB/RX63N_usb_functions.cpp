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

////////////////////////////////////////////////////////////////////////////
// Based on RENESAS usb_hal.cpp
////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include <Include\USB_decl.h>
#include <pal\com\usb\USB.h>
#include "..\RX63N.h"
#include "RX63N_usb_functions.h"

//*********************************************************************************
// NETMF default endpoint
// EP0  Control
// EP1  Bulk In
// EP2  Bulk Out
//*********************************************************************************
// GR-SAKURA Board information
//*********************************************************************************
// VBUS         P16
// USB_DM       47pin
// USB_DP       48pin
// USB0_DPRPD   32pin (P25) - DP
// USB0_DPUPE-B 43pin (P14)
// USB0_DRPD    (P22) - DM
//*********************************************************************************
//*********************************************************************************
// Defines
//*********************************************************************************

//#define DEBUG_USB
//#define USB_DUMP_SEND
//#define USB_DUMP_RECEIVE
//#define USB_DUMP_PID
//#define USB_USE_TXZERO_INT

#if defined(DEBUG_USB)
#define USB_DEBUG1(x0)          debug_printf(x0)
#define USB_DEBUG2(x0,x1)       debug_printf(x0,x1)
#define USB_DEBUG3(x0,x1,x2)    debug_printf(x0,x1,x2)
#define USB_DEBUG4(x0,x1,x2,x3) debug_printf(x0,x1,x2,x3)
#else
#define USB_DEBUG1(x0)
#define USB_DEBUG2(x0,x1)
#define USB_DEBUG3(x0,x1,x2)
#define USB_DEBUG4(x0,x1,x2,x3)
#endif

static USB_CONTROLLER_STATE g_USB_ControllerState = {0};
Hal_Queue_KnownSize<USB_PACKET64,USB_QUEUE_PACKET_COUNT> QueueBuffers[USB_MAX_EP - 1];
static UINT8 USB_EP0Buffer[MAX_EP0_SIZE];
static UINT16 USB_EPStatus[USB_MAX_EP];
static const USB_ENDPOINT_DESCRIPTOR *USB_EPDesc[USB_MAX_EP];
static BOOL USB_PinsProtected = FALSE;
static UINT16 USB_EP_Type = 0;
static INT8 USB_Pipe[USB_MAX_EP];
static INT8 USB_Zero = 0;
static INT8 USB_FifoType[USB_MAX_PIPE];
static volatile PIPECTR *pPIPECTR[USB_MAX_PIPE] = {
    (volatile PIPECTR *)&USB0.DCPCTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE1CTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE2CTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE3CTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE4CTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE5CTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE6CTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE7CTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE8CTR.WORD,
    (volatile PIPECTR *)&USB0.PIPE9CTR.WORD
};

static volatile UINT16 *pFIFOSEL[USB_MAX_FIFO] =
{
    (volatile UINT16 *)&USB0.CFIFOSEL.WORD,
    (volatile UINT16 *)&USB0.D0FIFOSEL.WORD,
    (volatile UINT16 *)&USB0.D1FIFOSEL.WORD
};

static volatile UINT16 *pFIFOCTR[USB_MAX_FIFO] =
{
    (volatile UINT16 *)&USB0.CFIFOCTR.WORD,
    (volatile UINT16 *)&USB0.D0FIFOCTR.WORD,
    (volatile UINT16 *)&USB0.D1FIFOCTR.WORD
};

#if 1
static volatile UINT8 *pFIFO[USB_MAX_FIFO] =
{
    (volatile UINT8 *)&USB0.CFIFO,
    (volatile UINT8 *)&USB0.D0FIFO,
    (volatile UINT8 *)&USB0.D1FIFO
};
#else
static volatile UINT16 *pFIFO[USB_MAX_FIFO] =
{
    (volatile UINT16 *)&USB0.CFIFO.WORD,
    (volatile UINT16 *)&USB0.D0FIFO.WORD,
    (volatile UINT16 *)&USB0.D1FIFO.WORD
};
#endif
static UINT32 pipe_bulk = PIPE_BULK_ST;
static UINT32 pipe_interrupt = PIPE_INT_ST;

static void usb_memory_dump(unsigned int addr, int len)
{
    int i;
    char *p = (char *)addr;
    for (i = 0; i < len; i++) {
        if ((i & 0xf) == 0) {
            debug_printf((char *)"%08x ", (unsigned int)p);
        }
        debug_printf((char *)"%02x ", (unsigned char)(*p & 0xff));
        if ((i & 0xf) == 0xf) {
            debug_printf((char *)"\r\n");
        }
        p++;
    }
    debug_printf((char *)"\r\n");
}

static void wait_cnt(volatile UINT32 count)
{
    while (count-- > 0);
}

void usb_int_brdy_enable(UINT16 pipe)
{
    UINT16 tmp;
    tmp = USB0.INTENB0.WORD;
    USB0.INTENB0.WORD = (UINT16)(tmp & (UINT16)~(MASK_BEMPE|MASK_NRDYE|MASK_BRDYE));
    USB0.BRDYENB.WORD |= (1 << pipe);
    USB0.INTENB0.WORD = tmp;
}

void usb_int_brdy_disable(UINT16 pipe)
{
    UINT16  tmp;
    tmp = USB0.INTENB0.WORD;
    USB0.INTENB0.WORD = (UINT16)(tmp & (UINT16)~(MASK_BEMPE|MASK_NRDYE|MASK_BRDYE));
    USB0.BRDYENB.WORD &= ~(1 << pipe);
    USB0.INTENB0.WORD = tmp;
}

void usb_int_nrdy_enable(UINT16 pipe)
{
    UINT16  tmp;
    tmp = USB0.INTENB0.WORD;
    USB0.INTENB0.WORD = (UINT16)(tmp & (UINT16)~(MASK_BEMPE|MASK_NRDYE|MASK_BRDYE));
    USB0.NRDYENB.WORD |= (1 << pipe);
    USB0.INTENB0.WORD = tmp;
}

void usb_int_nrdy_disable(UINT16 pipe)
{
    UINT16 tmp;
    tmp = USB0.INTENB0.WORD;
    USB0.INTENB0.WORD = (UINT16)(tmp & (UINT16)~(MASK_BEMPE|MASK_NRDYE|MASK_BRDYE));
    USB0.NRDYENB.WORD &= ~(1 << pipe);
    USB0.INTENB0.WORD = tmp;
}

void usb_int_bemp_enable(UINT16 pipe)
{
    UINT16 tmp;
    tmp = USB0.INTENB0.WORD;
    USB0.INTENB0.WORD = (UINT16)(tmp & (UINT16)~(MASK_BEMPE|MASK_NRDYE|MASK_BRDYE));
    USB0.BEMPENB.WORD |= (1 << pipe);
    USB0.INTENB0.WORD = tmp;
}

void usb_int_bemp_disable(UINT16 pipe)
{
    UINT16 tmp;
    tmp = USB0.INTENB0.WORD;
    USB0.INTENB0.WORD = (UINT16)(tmp & (UINT16)~(MASK_BEMPE|MASK_NRDYE|MASK_BRDYE));
    USB0.BEMPENB.WORD &= ~(1 << pipe);
    USB0.INTENB0.WORD = tmp;
}

static void USB_Pipe_Initialize(void)
{
    UINT32 i;
    for (i = 0; i < USB_MAX_PIPE; i++) {
        USB_FifoType[i] = -1;
    }
    USB_FifoType[0] = FIFO_CFIFO;
    USB_FifoType[1] = FIFO_D1FIFO;
    USB_FifoType[2] = FIFO_CFIFO;
    USB_FifoType[3] = FIFO_CFIFO;
    USB_FifoType[4] = FIFO_CFIFO;
    USB_FifoType[5] = FIFO_CFIFO;
    USB0.CFIFOSEL.WORD = 0;
    USB0.D0FIFOSEL.WORD = 0;
    USB0.D1FIFOSEL.WORD = 0;
}

UINT16 GetPipePid(UINT32 pipe)
{
    volatile PIPECTR *pipectr = pPIPECTR[pipe];
    return (pipectr->BIT.PID);
}

UINT16 Pipe_Inbuf(UINT16 pipe)
{
    UINT16 inbuf;
    volatile PIPECTR *pipectr = pPIPECTR[pipe];
    inbuf = pipectr->BIT.INBUFM;
    return inbuf;
}

#ifdef USB_DUMP_PID
void dump_pid(UINT32 pipe)
{
    volatile PIPECTR *pipectr = pPIPECTR[pipe];
    UINT16 pid = pipectr->BIT.PID;
    USB_DEBUG3("Pi%d:%d\r\n", pipe, pid);
}
#endif

void SetPipePidNAK(UINT32 pipe)
{
    volatile PIPECTR *pipectr = pPIPECTR[pipe];
    UINT16 cpid = (pipectr->BIT.PID);
    if (cpid == PID_STALL_2) {
        pipectr->BIT.PID = PID_STALL_1;
    }
    pipectr->BIT.PID = PID_NAK;
    if (cpid == PID_BUF) {
        while (pipectr->BIT.PBUSY == 1);
    }
}

void SetPipePidBUF(UINT32 pipe)
{
    volatile PIPECTR *pipectr = pPIPECTR[pipe];
    pipectr->BIT.PID = PID_BUF;
}

void ClearPipeFIFO(UINT32 pipe)
{
    if (USB0.CFIFOSEL.BIT.CURPIPE == pipe)
        USB0.CFIFOSEL.BIT.CURPIPE = 0;
    if (USB0.D0FIFOSEL.BIT.CURPIPE == pipe)
        USB0.D0FIFOSEL.BIT.CURPIPE = 0;
    if (USB0.D1FIFOSEL.BIT.CURPIPE == pipe)
        USB0.D1FIFOSEL.BIT.CURPIPE = 0;
}

#define FIFO_LOOP       4
#define FIFO_TIMEOUT    2000

void PipeSetFIFO(UINT16 pipe, UINT16 dir, UINT16 mbw)
{
    volatile UINT16 tmp;
    UINT8 type;
    if (pipe >= 0) {
        ClearPipeFIFO(pipe);
        type = USB_FifoType[pipe];
        volatile UINT16 *pfifosel = pFIFOSEL[type];
        volatile UINT16 *pfifoctr = pFIFOCTR[type];
        for (UINT32 i = 0; i < FIFO_LOOP; i++) {
            tmp = *pfifosel;
            if (type == FIFO_CFIFO) {
                tmp &= 0xFBD0;
                tmp |= ((mbw << 10) | (dir << 5) | pipe);
            } else {
                tmp &= 0xFBF0;
                tmp |= ((mbw << 10) | pipe);
            }
            *pfifosel = tmp;
            wait_cnt(FIFO_TIMEOUT);
            volatile UINT16 j = FIFO_TIMEOUT;
            while (j-- > 0) {
                if (*pfifosel == tmp)
                    break;
            }
            j = FIFO_TIMEOUT;
            while (j-- > 0) {  // FRDY
                if ((*pfifoctr & 0x2000) != 0)
                    return;
            }
        }
        USB_DEBUG3("PipeNG%d:%d\r\n", pipe, dir);
    }
}

void RX_USB_Interrupt_Enable(void)
{
    ICU.IER[0x04].BIT.IEN3  = 1u;   /* USBI0 enable */
    ICU.IPR[35].BYTE    = 0x0E; /* USBI0 */
}

void RX_USB_Pullup_Connect(void)
{
    USB0.SYSCFG.BIT.DPRPU = 1;
}

void RX_USB_Pullup_Disconnect(void)
{
    USB0.SYSCFG.BIT.DPRPU = 0;
}

void RX_USB_Interrupt_Initialize(void)
{
    //USB0.SOFCFG.WORD = 0x20;
    USB0.INTENB0.WORD = 0x9F00;
    //USB0.INTENB0.BIT.VBSE = 1;          // b15
    //USB0.INTENB0.BIT.DVSE = 1;          // b12
    //USB0.INTENB0.BIT.CTRE = 1;          // b11
    //USB0.INTENB0.BIT.BEMPE = 1;         // b10
    //USB0.INTENB0.BIT.NRDYE = 1;         // b9
    //USB0.INTENB0.BIT.BRDYE = 1;         // b8

    USB0.BEMPENB.BIT.PIPE0BEMPE = 1;    // pipe 0 bemp interrupt enable
    USB0.BRDYENB.BIT.PIPE0BRDYE = 0;    // pipe 0 brdy inerrrupt disable
}

void RX_USB_Interrupt_Uninitialize(void)
{
    //USB0.SOFCFG.WORD = 0x20;
    USB0.INTENB0.WORD = 0x0000;
    //USB0.INTENB0.BIT.VBSE = 0;          // b15
    //USB0.INTENB0.BIT.DVSE = 0;          // b12
    //USB0.INTENB0.BIT.CTRE = 0;          // b11
    //USB0.INTENB0.BIT.BEMPE = 0;         // b10
    //USB0.INTENB0.BIT.NRDYE = 0;         // b9
    //USB0.INTENB0.BIT.BRDYE = 0;         // b8
}


void USB0_HW_Initialize(void)
{
    SYSTEM.PRCR.WORD = 0xA502;          /* protect off */
    MSTP(USB0) = 0;
    SYSTEM.PRCR.WORD = 0xA500;          /* protect on */

    USB.DPUSR0R.BIT.FIXPHY0 = 0u;   /* USB0 Transceiver Output fixed */

    MPC.PWPR.BIT.B0WI   = 0u;   // Enable PFSWE bit
    MPC.PWPR.BIT.PFSWE  = 1u;   // Enable PFS register
    PORT1.PMR.BIT.B4    = 1u;   // P14 peripheral
    PORT1.PMR.BIT.B6    = 1u;   // P16 peripheral
    MPC.P14PFS.BYTE = 0x11;     // USB0_DPUPE
    MPC.P16PFS.BYTE = 0x11;     // USB0_VBUS
    MPC.PFUSB0.BIT.PUPHZS = 1u; // Pullup
    MPC.PWPR.BIT.PFSWE  = 0u;   // Disable PFS register
    MPC.PWPR.BIT.B0WI   = 1u;   // Disable PFSWE bit
}

void RX_USB_HW_Initialize(void)
{
    USB0_HW_Initialize();
    USB0.SYSCFG.BIT.SCKE = 1;
    USB0.SYSCFG.BIT.USBE = 1;
    USB0.SYSCFG.BIT.DCFM = 0;
    USB0.SYSCFG.BIT.DRPD = 0;
    USB0.SYSCFG.BIT.DPRPU = 1;
    USB_Pipe_Initialize();
    RX_USB_Interrupt_Initialize();
    RX_USB_Interrupt_Enable();
}

void RX_USB_EP_Initialize(UINT32 pipe, UINT32 ep, UINT32 dir, UINT32 attr, UINT32 size)
{
    ClearPipeFIFO(pipe);
    volatile PIPECTR *pipectr = pPIPECTR[pipe];
    USB0.PIPESEL.BIT.PIPESEL = pipe;
    SetPipePidNAK(pipe);
    USB0.PIPECFG.WORD = 0;
    USB0.PIPECFG.BIT.EPNUM = ep;        // Endpoint
    USB0.PIPECFG.BIT.DIR = dir;         // Direction
    USB0.PIPECFG.BIT.DBLB = 1;          // Double Buffer
    USB0.PIPECFG.BIT.BFRE = 0;          // Interrupt
    //USB0.PIPECFG.BIT.SHTNAK = 1;
    if (attr == USB_ENDPOINT_ATTRIBUTE_BULK) {
        USB0.PIPECFG.BIT.TYPE = 1;      // BULK
        if (dir == 0) { // OUT
            //pipectr->BIT.ATREPM = 1;
            pipectr->BIT.SQCLR = 1;
            pipectr->BIT.ACLRM = 1;     // Clear Buffer
            pipectr->BIT.ACLRM = 0;     // Clear Buffer
            pipectr->BIT.PID = PID_BUF;
            //PipeSetFIFO((UINT16)pipe, dir, MBW_BYTE);
            usb_int_bemp_disable(pipe);
            usb_int_brdy_disable(pipe);
            //usb_int_brdy_enable(pipe);
        } else {        // IN
            //pipectr->BIT.ATREPM = 1;
            pipectr->BIT.SQCLR = 1;
            pipectr->BIT.ACLRM = 1;     // Clear Buffer
            pipectr->BIT.ACLRM = 0;     // Clear Buffer
            //PipeSetFIFO((UINT16) pipe, dir, MBW_BYTE);
            usb_int_bemp_disable(pipe);
            usb_int_brdy_disable(pipe);
        }
        USB0.PIPEMAXP.BIT.MXPS = size;  // Packet Size
    } else if (attr == USB_ENDPOINT_ATTRIBUTE_INTERRUPT) {
        USB0.PIPECFG.BIT.TYPE = 2;      // Transfer Type
        USB0.PIPEMAXP.BIT.MXPS = size;  // Packet Size
        //usb_int_bemp_disable(pipe);
        //usb_int_brdy_disable(pipe);
    } else if (attr == USB_ENDPOINT_ATTRIBUTE_ISOCHRONOUS) {
        USB0.PIPECFG.BIT.TYPE = 3;      // Transfer Type
        USB0.PIPEMAXP.BIT.MXPS = size;  // Packet Size
        //usb_int_bemp_disable(pipe);
        //usb_int_brdy_disable(pipe);
    }
    USB0.PIPESEL.BIT.PIPESEL = 0;       // Unselect Pipe
}

void USB_Control_Status(void)
{
    USB0.DCPCTR.BIT.PID = PID_BUF;
    USB0.DCPCTR.BIT.CCPL = 1;
}

void USB_Stall_Clear(INT32 pipe)
{
    volatile PIPECTR *pipectr = pPIPECTR[pipe];
    pipectr->BIT.PID = PID_NAK;
    while (pipectr->BIT.PBUSY == 1){;}
    pipectr->BIT.SQCLR = 1;
    pipectr->BIT.ACLRM = 1;
    pipectr->BIT.ACLRM = 0;
    USB0.PIPESEL.BIT.PIPESEL = pipe;
    if (USB0.PIPECFG.BIT.DIR == 0) {
        // receive direction
        pipectr->BIT.PID = PID_BUF;
    }
}

void Pipe_Config(void)
{
    USB_CONTROLLER_STATE *State = &g_USB_ControllerState;
    const USB_ENDPOINT_DESCRIPTOR *pep = (USB_ENDPOINT_DESCRIPTOR *)NULL;
    const USB_INTERFACE_DESCRIPTOR *pif  = (USB_INTERFACE_DESCRIPTOR *)NULL;
    while (USB_NextEndpoint(State, pep, pif)) {
        UINT32 endpoint = pep->bEndpointAddress & 0x7F;
        UINT32 dir = (pep->bEndpointAddress & 0x80)? 1:0;
        RX_USB_EP_Initialize((UINT32)USB_Pipe[endpoint],
                (UINT32)endpoint, (UINT32)dir, pep->bmAttributes, pep->wMaxPacketSize);
        USB_DEBUG4("E%d:P%d:D%d\r\n", endpoint, USB_Pipe[endpoint], dir);
    }
}

void RX_USB_EP_Reset(void)
{
    USB_DEBUG1("Ep\r\n");
    UINT32 i;
    INT32 pipeno;
    for (i = 0; i < USB_MAX_EP; i++) {
        pipeno = USB_Pipe[i];
        if (pipeno >= 0)
            USB_Stall_Clear((UINT32)pipeno);
    }
}

void RX_USB_Reset(void)
{
    USB_DEBUG1("Rt\r\n");
    UINT32 i;
    INT32 pipeno;
    Pipe_Config();
    RX_USB_EP_Reset();
    RX_USB_Interrupt_Initialize();
}

void Pipe_Read(UINT32 pipe, volatile UINT8 *buf, UINT32 size, volatile UINT32 *count)
{
    UINT8 dummy;
    UINT32 len;
    UINT8 type = USB_FifoType[pipe];
    GLOBAL_LOCK(irq);
    PipeSetFIFO((UINT16)pipe, DIR_OUT, MBW_BYTE);
    volatile UINT16 *pfifosel = (volatile UINT16 *)pFIFOSEL[type];
    volatile UINT16 *pfifoctr = (volatile UINT16 *)pFIFOCTR[type];
    volatile UINT8 *pfifo = (volatile UINT8 *)pFIFO[type];
    *pfifosel |= 0x8000;                // RCNT = 1
    len = *pfifoctr & 0x01FF;           // DTLN
    if (len == 0) {
        *pfifoctr |= 0x4000;            // BCLR = 1
    }
    if (buf != NULL) {
        while ((len!= 0) && (size > *count)) {
            *buf++ = *pfifo;
            (*count)++;
            len--;
        }
    }
    SetPipePidNAK(pipe);
    if ((*count == 0) && (0 != (*pfifoctr & 0x01FF))) {
        lcd_printf("fr error\r\n");
        while (0 != (*pfifoctr & 0x01FF)) {
            dummy = *pfifo;
        }
    }
    USB_DEBUG3("Pr%d:%d\r\n", pipe, *count);
}

void Pipe_Write(UINT32 pipe, volatile UINT8 *buf, UINT32 size, volatile UINT32 len)
{
    USB_DEBUG3("Pw%d:%d\r\n", pipe, len);
    GLOBAL_LOCK(irq);
    PipeSetFIFO((UINT16)pipe, DIR_IN, MBW_BYTE);
    UINT32 count = 0;
    UINT8 type = USB_FifoType[pipe];
    volatile UINT16 *pfifoctr = (volatile UINT16 *)pFIFOCTR[type];
    volatile UINT8 *pfifo = (volatile UINT8 *)pFIFO[type];
    if (buf != NULL)
        while ((len != 0) && (size > count)) {
            *pfifo = (UINT8)*buf;
            buf++;
            count++;
            len--;
        }
    if (count != size) {
        *pfifoctr |= 0x8000;            // BVAL = 1
    }
    SetPipePidBUF(pipe);
    usb_int_brdy_disable(pipe); // disable interrupt
    usb_int_bemp_enable(pipe);  // enable interrupt
    usb_int_nrdy_enable(pipe);  // enable interrupt
}

void HandleSetupCmd(void)
{
    USB_CONTROLLER_STATE *State = &g_USB_ControllerState;
    USB0.INTSTS0.BIT.VALID = 0;
    UINT16 *buf = (UINT16 *)&USB_EP0Buffer[0];
    *buf++ = USB0.USBREQ.WORD;
    *buf++ = USB0.USBVAL;
    *buf++ = USB0.USBINDX;
    *buf = USB0.USBLENG;
    State->Data = (UINT8 *)&USB_EP0Buffer[0];
    State->DataSize = (UINT8)8;
    UINT8 result = USB_ControlCallback(State);
    
    switch (result) {
    case USB_STATE_DATA:
        break;
    case USB_STATE_ADDRESS:
        /* upper layer needs us to change the address */
        // address change delayed to end of reply packet
        break;
    case USB_STATE_DONE:
        USB_DEBUG1("Sd\r\n");
        State->DataCallback = NULL;
        break;
    case USB_STATE_STALL:
        USB_DEBUG1("Ss\r\n");
        return;
    case USB_STATE_STATUS:
        USB_DEBUG1("St\r\n");
        break;
    case USB_STATE_CONFIGURATION:
        USB_DEBUG1("Sc\r\n");
        break;
    case USB_STATE_REMOTE_WAKEUP:
        USB_DEBUG1("Sw\r\n");
        break;
    default:
        break;
    }
    if (result != USB_STATE_STALL) {
    if (State->DataCallback != NULL) {
        State->DataCallback(State);
        if (State->DataSize != 0) {
                //USB_DEBUG1("Si");
                Pipe_Write(0, (UINT8 *)State->Data, (UINT32)MAX_EP0_SIZE, (UINT32)State->DataSize);
            } else {
                //USB_DEBUG1("S0");
                USB_Control_Status();
            }
        }
    }
}

void HandleVBus(void)
{
    USB_DEBUG1("V");
    UINT16 vbsts_value;
    UINT8 count;
    #define CHATTER_COUNT 5
    do {
        count = CHATTER_COUNT;
        vbsts_value = USB0.INTSTS0.BIT.VBSTS;
        while (count != 0) {
            if (USB0.INTSTS0.BIT.VBSTS != vbsts_value) {
                break;
            } else {
                count--;
            }
        }
    } while (count != 0);
    if (0 != vbsts_value) { // connected;
        RX_USB_Reset();
    } else {                // disconnected;
        RX_USB_Reset();
    }
}

void HandleDVST(void)
{
    switch(USB0.INTSTS0.BIT.DVSQ)
    {
        case 0:         // Powered State
            USB_DEBUG1("D0\r\n");
            break;
        case 1:         // Default State
            USB_DEBUG1("D1\r\n");
            if (USB0.DVSTCTR0.BIT.RHST == 4) {  // BUS RESET
                RX_USB_EP_Reset();
            }
            break;
        case 2:         // Address State
            USB_DEBUG1("D2\r\n");
            break;
        case 3:         // Configured State
            USB_DEBUG1("D3\r\n");
            RX_USB_EP_Reset();
#ifdef USB_DUMP_PID
            dump_pid(1);
            dump_pid(2);
#endif
            break;
        case 4:
        default:        // Suspended State
            USB_DEBUG1("D4\r\n");
            break;
    };
}

void HandleCTRT(void)
{
    switch(USB0.INTSTS0.BIT.CTSQ)
    {
        case 0:         // IDLE or Setup
            USB_DEBUG1("C0\r\n");
            break;
        case 1:         // Control Read Data
            USB_DEBUG1("C1");
            HandleSetupCmd();
            break;
        case 2:         // Control Read Status
            USB_DEBUG1("C2");
            break;
        case 3:         // Control Write Data
            USB_DEBUG1("C3");
            HandleSetupCmd();
            break;
        case 4:         // Control Write Status
            USB_DEBUG1("C4");
            break;
        case 5:         // Control Write(no data) Status
            USB_DEBUG1("C5");
            HandleSetupCmd();
        break;
        case 6:         // Control Transfer Error
            USB_DEBUG1("C6");
        break;
        default:        // INVALID
            USB_DEBUG1("C7");
        break;
    };
}

void RX_USB_Send(USB_CONTROLLER_STATE *State, UINT32 endpoint)
{
    GLOBAL_LOCK(irq);
    USB_PACKET64* Packet64 = USB_TxDequeue(State, endpoint, TRUE);
    UINT32 pipeno = USB_Pipe[endpoint];
    const USB_ENDPOINT_DESCRIPTOR *pep = USB_EPDesc[endpoint];
    if (Packet64) {
        Pipe_Write(pipeno, (UINT8 *)Packet64->Buffer, pep->wMaxPacketSize, (UINT32)Packet64->Size);
        USB_DEBUG3("Us%d:%d\r\n", pipeno, Packet64->Size);
#ifdef USB_USE_TXZERO_INT
        if (pep->wMaxPacketSize == Packet64->Size)
            USB_Zero = 1;
#endif
    } else {
        //USB_Zero = 2;
        //Pipe_Write(pipeno, 0, pep->wMaxPacketSize, 0);
        USB_DEBUG2("Us%d:0\r\n", pipeno);
    }
}

void RX_USB_Receive(USB_CONTROLLER_STATE *State, UINT32 endpoint)
{
    GLOBAL_LOCK(irq);
    BOOL full;
    UINT32 pipeno = USB_Pipe[endpoint];
    USB_PACKET64* Packet64 = USB_RxEnqueue(State, endpoint, full);
    if (Packet64) {
        const USB_ENDPOINT_DESCRIPTOR *pep = USB_EPDesc[endpoint];
        UINT32 size = 0;
        Pipe_Read(pipeno, (UINT8 *)Packet64->Buffer, pep->wMaxPacketSize, &size);
        Packet64->Size = (UINT8)size;
            USB_DEBUG3("Ur%d:%d\r\n", pipeno, Packet64->Size);
        usb_int_brdy_disable(pipeno);
    } else {
        USB_DEBUG2("Ur%d:0\r\n", pipeno);
        //usb_int_brdy_disable(pipeno);
    }

}

// BEMP Interrupt Handler
// TX: when TX completion
// RX: when receiving data > MaxPackectSize
void HandleBEMP(void)
{
    GLOBAL_LOCK(irq);
    USB_CONTROLLER_STATE *State = &g_USB_ControllerState;
    UINT16 bempsts = USB0.BEMPSTS.WORD;
    UINT16 bempenb = USB0.BEMPENB.WORD;
    if (1 == USB0.BEMPSTS.BIT.PIPE0BEMP) {
        USB_DEBUG1("e0");
        USB0.BEMPSTS.WORD = ~0x0001;
        if (State->DataCallback != NULL) {
            State->DataCallback(State);
            if (State->DataSize != 0) {
                Pipe_Write(0, (UINT8 *)State->Data, (UINT32)MAX_EP0_SIZE, (UINT32)State->DataSize);
            } else {
                USB_Control_Status();
            }
        } else {
            USB_Control_Status();
        }
    } else {
        USB0.BEMPSTS.WORD = ~bempsts;
        for (UINT32 endpoint = 1; endpoint < USB_MAX_EP; endpoint++) {
            INT8 pipeno = USB_Pipe[endpoint];
            if (pipeno >= 0) {
                UINT16 mask = (1 << pipeno);
                if ((bempsts & mask) && (bempenb & mask)) {
                    USB_DEBUG2("BE%d\r\n", pipeno);
                    UINT16 pid = GetPipePid(pipeno);
                    if ((pid == PID_STALL_1) || (pid == PID_STALL_2)) {
                        USB_DEBUG3("Ss%d:%d\r\n", pipeno, pid);
                    } else {
                        if (Pipe_Inbuf(pipeno) == 0) {
                            usb_int_bemp_disable(pipeno);
                            SetPipePidNAK(pipeno);
#ifdef USB_USE_TXZERO_INT
                            if (RX64M_USB_Zero == 1) {
                                USB_DEBUG1("Zp\r\n");
                                Pipe_Write(pipeno, 0, 64, 0);
                                RX64M_USB_Zero = 2;
                            }
#endif
                        }
                    }
                }
            }
        }
    }
}

void HandleNRDY(void)
{
    GLOBAL_LOCK(irq);
    UINT16 nrdysts = USB0.NRDYSTS.WORD;
    UINT16 nrdyenb = USB0.NRDYENB.WORD;
    USB0.NRDYSTS.WORD = ~nrdysts;
    if ((USB0.NRDYSTS.BIT.PIPE0NRDY == 1)
            && (USB0.NRDYENB.BIT.PIPE0NRDYE == 1)) {
        USB_DEBUG1("n0");
        USB0.NRDYSTS.WORD = ~0x0001;
    } else {
        for (UINT32 endpoint = 1; endpoint < USB_MAX_EP; endpoint++) {
            INT8 pipeno = USB_Pipe[endpoint];
            if (pipeno >= 0) {
                UINT16 mask = (1 << pipeno);
                if ((nrdysts & mask) && (nrdyenb & mask)) {
                    USB0.NRDYSTS.WORD = ~mask;
                    USB_DEBUG2("NR%d\r\n", pipeno);
#ifdef USB_USE_TXZERO_INT
                    if (RX64M_USB_Zero == 2) {
                        RX64M_USB_Zero = 0;
                    } else {
                        SetPipePidBUF(pipeno);
                    }
#endif
                    usb_int_nrdy_disable(pipeno);
                }
            }
        }
    }
}

void HandleBRDY(void)
{
    UINT16 brdysts = USB0.BRDYSTS.WORD;
    UINT16 brdyenb = USB0.BRDYENB.WORD;
    USB_CONTROLLER_STATE *State = &g_USB_ControllerState;

    if ((USB0.BRDYSTS.BIT.PIPE0BRDY == 1)
            && (USB0.BRDYENB.BIT.PIPE0BRDYE == 1)) {
        USB0.BRDYSTS.WORD = ~0x0001;
        if (USB0.DCPCFG.BIT.DIR == 0) {
        UINT32 size = 0;
            State->Data = (UINT8 *)&USB_EP0Buffer[0];
            Pipe_Read(0, (UINT8 *)State->Data, MAX_EP0_SIZE, &size);
        State->DataSize = (UINT8) size;
        }
        USB_DEBUG2("BR0:%d\r\n", USB0.DCPCFG.BIT.DIR);
    } else {
        for (UINT32 endpoint = 1; endpoint < USB_MAX_EP; endpoint++) {
            INT8 pipeno = USB_Pipe[endpoint];
            if (pipeno >= 0) {
                UINT16 mask = (1 << pipeno);
                if ((brdysts & mask) && (brdyenb & mask)) {
                    USB0.BRDYSTS.WORD = ~mask;
                    USB0.PIPESEL.BIT.PIPESEL = pipeno;
                    USB_DEBUG3("BR%d:%d\r\n", pipeno, USB0.PIPECFG.BIT.DIR);
                    if (USB0.PIPECFG.BIT.DIR == 0) {
                        RX_USB_Receive(State, endpoint);  // OUT
                    } else {
                        RX_USB_Send(State, endpoint);     // IN
                    }
                }
            }
        }
    }
 }

static void USBInterruptHandler(void)
{
    if (1 == USB0.INTSTS0.BIT.VBINT) {
        USB0.INTSTS0.BIT.VBINT = 0;
        HandleVBus();
    } else
    if (1 == USB0.INTSTS0.BIT.DVST) {
        USB0.INTSTS0.BIT.DVST = 0;
        HandleDVST();
    } else
    if (1 == USB0.INTSTS0.BIT.CTRT) {
        HandleCTRT();
        USB0.INTSTS0.BIT.CTRT = 0;
    } else
    if (1 == USB0.INTSTS0.BIT.BEMP) {
        HandleBEMP();
    } else
    if (1 == USB0.INTSTS0.BIT.NRDY) {
        HandleNRDY();
    } else
    if (1 == USB0.INTSTS0.BIT.BRDY) {
        HandleBRDY();
    }
}

USB_CONTROLLER_STATE * CPU_USB_GetState(int Controller)
{
    if (Controller != 0)
        return NULL;
    return (USB_CONTROLLER_STATE *)&g_USB_ControllerState;
}

// CPU_USB_Initialize Function
//  Initializes the client module at the driver layer.
HRESULT CPU_USB_Initialize(int Controller)
{
#if defined(DEBUG_USB)
    DebuggerPort_Initialize(COM1);
#endif
    GLOBAL_LOCK(irq);
    USB_CONTROLLER_STATE *State = &g_USB_ControllerState;
    if (Controller != 0) return S_FALSE;
#ifdef RENESAS_FW
    USB_Init();
#endif
    RX_USB_HW_Initialize();
    USB_PinsProtected = FALSE;
    CPU_USB_ProtectPins(Controller, TRUE);;
    State->DeviceState = USB_DEVICE_STATE_DETACHED;
    USB_StateCallback(State);
    USB_PinsProtected = TRUE;
    State->EndpointCount = USB_MAX_EP;
    State->EndpointStatus = &USB_EPStatus[0];
    State->PacketSize = MAX_EP0_SIZE;
    UINT32 endpoint;
    for (endpoint = 1; endpoint < USB_MAX_EP; endpoint++) {
        State->CurrentPacketOffset[endpoint] = 0;
        State->IsTxQueue[endpoint] = FALSE;
        State->MaxPacketSize[endpoint] = USB_MAX_DATA_PACKET_SIZE;
        USB_Pipe[endpoint] = -1;
    }
    USB_EP_Type = 0;
    USB_Zero = 0;
    pipe_interrupt = PIPE_INT_ST;
    pipe_bulk = PIPE_BULK_ST;
    const USB_ENDPOINT_DESCRIPTOR *pep = (USB_ENDPOINT_DESCRIPTOR *)NULL;
    const USB_INTERFACE_DESCRIPTOR *pif  = (USB_INTERFACE_DESCRIPTOR *)NULL;
    while (USB_NextEndpoint(State, pep, pif)) {
        endpoint = pep->bEndpointAddress & 0x7F;
        UINT8 endpointSize = pep->wMaxPacketSize;
        State->MaxPacketSize[endpoint] = endpointSize;
        QueueBuffers[endpoint-1].Initialize();
        State->Queues[endpoint] = &QueueBuffers[endpoint-1];
        USB_EPDesc[endpoint] = pep;
        switch (pep->bmAttributes) {
        case USB_ENDPOINT_ATTRIBUTE_BULK:
            USB_Pipe[endpoint] = pipe_bulk;
            if (pep->bEndpointAddress & 0x80) {
                RX_USB_EP_Initialize((UINT32)pipe_bulk++,
                        (UINT32)endpoint, (UINT32)1, pep->bmAttributes, endpointSize);
                State->IsTxQueue[endpoint] = TRUE;
            } else {
                RX_USB_EP_Initialize((UINT32)pipe_bulk++,
                        (UINT32)endpoint, (UINT32)0, pep->bmAttributes, endpointSize);
            }
            break;
        case USB_ENDPOINT_ATTRIBUTE_INTERRUPT:
            USB_Pipe[endpoint] = pipe_interrupt;
            if (pep->bEndpointAddress & 0x80) {
                RX_USB_EP_Initialize(pipe_interrupt++,
                        endpoint, 1, pep->bmAttributes, endpointSize);
                State->IsTxQueue[endpoint] = TRUE;
            } else {
                RX_USB_EP_Initialize(pipe_interrupt++,
                        endpoint, 0, pep->bmAttributes, endpointSize);
            }
            break;
        case USB_ENDPOINT_ATTRIBUTE_ISOCHRONOUS:
            // ToDo
        default:
            break;
        }
        USB_EP_Type |= (pep->bmAttributes & 3) << (endpoint * 2);
    }
    if (USB_EP_Type == 0)
        return S_FALSE;
    HAL_Time_Sleep_MicroSeconds(1000);
    RX_USB_Interrupt_Enable();
    CPU_USB_ProtectPins(Controller, FALSE);
    USB_DEBUG1("*I*");
    return S_OK;
}

// CPU_USB_Uninitialize Function
//  Uninitializes (shuts down) USB communications at the driver layer.
HRESULT CPU_USB_Uninitialize(int Controller)
{
    USB_CONTROLLER_STATE *State = &g_USB_ControllerState;
    GLOBAL_LOCK(irq);
    memset(&g_USB_ControllerState, 0, sizeof(g_USB_ControllerState));
    CPU_USB_ProtectPins(Controller, TRUE);
    USB_EP_Type = 0;
    State->DeviceState = USB_DEVICE_STATE_DETACHED;
    RX_USB_Interrupt_Uninitialize();
    USB_DEBUG1("*U*");
    return S_OK;
}

// CPU_USB_StartOutput Function
//  Begins sending output to a specified USB port.
BOOL CPU_USB_StartOutput(USB_CONTROLLER_STATE* State, int endpoint)
{
    //GLOBAL_LOCK(irq);
    if (State == NULL || endpoint >= USB_MAX_EP)
        return FALSE;
    if (State->Queues[endpoint] == NULL || !State->IsTxQueue[endpoint])
        return FALSE;
    if (USB_EPStatus[endpoint] & USB_STATUS_ENDPOINT_HALT) {
        while (USB_TxDequeue(State, endpoint, TRUE) != NULL);  // clear TX queue
        return TRUE;
    }
    UINT32 pipeno = USB_Pipe[endpoint];
    if (Pipe_Inbuf(pipeno) == 0) {
        USB_DEBUG2("So%d\r\n", USB_Pipe[endpoint]);
        SetPipePidNAK(pipeno);
        RX_USB_Send(State, endpoint);
        return TRUE;
    } else
        return FALSE;
}

// CPU_USB_RxEnable Function
//  Enables a specified USB port to receive data.
BOOL CPU_USB_RxEnable(USB_CONTROLLER_STATE* State, int endpoint)
{
    GLOBAL_LOCK(irq);
    UINT32 pipeno = USB_Pipe[endpoint];
    USB_DEBUG2("Re%d\r\n", pipeno);
    SetPipePidBUF(pipeno);
    usb_int_brdy_enable(pipeno);
    return TRUE;
}

// CPU_USB_GetInterruptState Function
//  Determines whether there are currently interrupts pending for the USB port.
BOOL CPU_USB_GetInterruptState()
{
#if 0
    BOOL flag = ((USB0.BEMPENB.WORD & USB0.BEMPSTS.WORD) != 0) || ((USB0.BRDYENB.WORD & USB0.BRDYSTS.WORD) != 0);
    USB_DEBUG2("Is%d\r\n", flag);
    return flag;
#else
    return FALSE;
#endif
}

// Puts USB pins into a known state to prevent spurious inputs.
// Parameters
// On
// A value you can set to true to protect the USB pins; set it to false to disable protection.
// Return Value
// true if the operation succeeded; otherwise, false.
//
// Setting the On parameter to true detaches the USB module from the host.
// This protects the USB pins by stopping all USB communications.
// Setting On to false enables the pins, thus enabling the host to detect the USB client.
// As soon as the USB client is detected, the enumeration process begins.
// USB communications resume when the enumeration process has finished.
BOOL CPU_USB_ProtectPins(int Controller, BOOL On)
{
    USB_CONTROLLER_STATE *State = &g_USB_ControllerState;
    GLOBAL_LOCK(irq);
    if (On == TRUE) {
        if (USB_PinsProtected == FALSE) {
            USB_DEBUG1("+");
            USB_PinsProtected = TRUE;
            RX_USB_Pullup_Disconnect();
            RX_USB_Reset();
            for(int ep = 1; ep < USB_MAX_EP; ep++) {
                if (State->Queues[ep] && State->IsTxQueue[ep]) {
                    while (USB_TxDequeue(State, ep, TRUE) != NULL);
                }
            }
            State->DeviceState = USB_DEVICE_STATE_DETACHED;
            USB_StateCallback(State);
        }
    } else {
        if (USB_PinsProtected == TRUE) {
            USB_DEBUG1("-");
            USB_PinsProtected = FALSE;
            RX_USB_Pullup_Connect();
            RX_USB_Reset();
            State->DeviceState = USB_DEVICE_STATE_ATTACHED;
            USB_StateCallback(State);
        }
    }
    return TRUE;
}

#ifdef __cplusplus
extern "C" {
#endif
void __attribute__ ((interrupt)) INT_Excep_USB0_D0FIFO0(void) { }
void __attribute__ ((interrupt)) INT_Excep_USB0_D1FIFO0(void) { }
void __attribute__ ((interrupt)) INT_Excep_USB0_USBI0(void)
{
    USBInterruptHandler();
}
#ifdef __cplusplus
}
#endif
