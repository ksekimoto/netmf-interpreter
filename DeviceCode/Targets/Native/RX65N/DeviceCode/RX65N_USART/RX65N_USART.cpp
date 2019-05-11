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
#include "..\iodefine.h"
#include "RX65N_USART.h"
#include "..\RX65N.h"

static void USART_RxISR(UINT32 port);
static void USART_TxISR(UINT32 port);

static inline void rx_enable_irq(void) {
    __asm__ __volatile__ ("setpsw i");
}

static inline void rx_disable_irq(void) {
    __asm__ __volatile__ ("clrpsw i");
}

static UINT8 port_ch[] = {
#if defined(PLATFORM_RX65N_GR_ROSE)
    0,      // COM1 serial1 - ch0
    2,      // COM2 serial2 - ch2
    5,      // COM3 serial3 - ch5
    6,      // COM4 serial4 - ch6
    1,      // COM5 serial5 - ch1
    3,      // COM6 serial6 ESP8266 - ch3
    8       // COM7 serial7 - ch8
#else
#error "port-ch mapping is not defined. Please define port-ch for target platform!"
#endif
};
#define SCI_CH_NUM  (sizeof(port_ch)/sizeof(UINT8))

static UINT8 ch_port[] = {
#if defined(PLATFORM_RX65N_GR_ROSE)
    0,      // ch0 - COM1 index=0
    4,      // ch1 - COM5 index=4
    1,      // ch2 - COM2 index=1
    5,      // ch3 - COM6 index=5
    0xff,   // ch4 - xxxx
    2,      // ch5 - COM3 index=2
    3,      // ch6 - COM4 index=3
    0xff,   // ch7 - xxxx
    6,      // ch8 - COM7 index=6
#else
#error "ch-port mapping is not defined. Please define ch-port for target platform!"
#endif
};
#define SCI_PORT_NUM    (sizeof(ch_port)/sizeof(UINT8))

#define SCI_DEFAULT_PRIORITY 12
#define SCI_DEFAULT_BAUD    115200

#define SCR_CKE_M   (1 << 0)
#define SCR_TEIE_M  (1 << 2)
#define SCR_MPIE_M  (1 << 3)
#define SCR_RE_M    (1 << 4)
#define SCR_TE_M    (1 << 5)
#define SCR_RIE_M   (1 << 6)
#define SCR_TIE_M   (1 << 7)
#define USART_PRIORITY    12

static volatile struct st_sci0 *SCI[] = {
    (volatile struct st_sci0 *)0x8A000, /* sci 0 */
    (volatile struct st_sci0 *)0x8A020, /* sci 1 */
    (volatile struct st_sci0 *)0x8A040, /* sci 2 */
    (volatile struct st_sci0 *)0x8A060, /* sci 3 */
    (volatile struct st_sci0 *)0x8A080, /* sci 4 */
    (volatile struct st_sci0 *)0x8A0A0, /* sci 5 */
    (volatile struct st_sci0 *)0x8A0C0, /* sci 6 */
    (volatile struct st_sci0 *)0x8A0E0, /* sci 7 */
    (volatile struct st_sci0 *)0x8A100, /* sci 8 */
    (volatile struct st_sci0 *)0x8A120, /* sci 9 */
    (volatile struct st_sci0 *)0x8A140, /* sci 10 */
    (volatile struct st_sci0 *)0x8A160, /* sci 11 */
    (volatile struct st_sci0 *)0x8B300, /* sci 12 */
};

static const UINT8 sci_tx_pins[] = {
#if defined(PLATFORM_RX65N_GR_ROSE)
    RX65N_GPIO::P20,    /* ch 0 P20 */
    RX65N_GPIO::P26,    /* ch 1 P16 */
    RX65N_GPIO::P13,    /* ch 2 P50 */
    RX65N_GPIO::P23,    /* ch 3 P23 */
    0xff,   /* ch 4 */
    RX65N_GPIO::PC3,    /* ch 5 PC3 */
    RX65N_GPIO::P32,    /* ch 6 P32 */
    0xff,   /* ch 7 */
    RX65N_GPIO::PC7,   /* ch 8 */
    0xff,   /* ch 9 */
    0xff,   /* ch 10 */
    0xff,   /* ch 11 */
    0xff,   /* ch 12 */
#else
#error "ch-pin mapping is not defined. Please define ch-pin for target platform!"
#endif
};

static const UINT8 sci_rx_pins[] = {
#if defined(PLATFORM_RX65N_GR_ROSE)
    RX65N_GPIO::P21,    /* ch 0 P21 */
    RX65N_GPIO::P30,    /* ch 1 P15 */
    RX65N_GPIO::P12,    /* ch 2 P52 */
    RX65N_GPIO::P25,    /* ch 3 P25 */
    0xff,   /* ch 4 */
    RX65N_GPIO::PC2,    /* ch 5 PC2 */
    RX65N_GPIO::P33,    /* ch 6 P33 */
    0xff,   /* ch 7 */
    RX65N_GPIO::PC6,    /* ch 8 */
    0xff,   /* ch 9 */
    0xff,   /* ch 10 */
    0xff,   /* ch 11 */
    0xff,   /* ch 12 */
#else
#error "ch-pin mapping is not defined. Please define ch-pin for target platform!"
#endif
};

static void delay_us(volatile unsigned int us) {
    us *= 60;
    while (us-- > 0)
        ;
}

void sci_rx_set_int(int ch, int flag) {
    int idx = 0;
    int bit = 0;
    if (ch < 3) {
        idx = (58 + ch * 2) / 8;
        bit = (58 + ch * 2) & 7;
    } else if (ch < 7) {
        idx = (74 + ch * 2) / 8;
        bit = (74 + ch * 2) & 7;
    } else if (ch < 11) {
        idx = (84 + ch * 2) / 8;
        bit = (84 + ch * 2) & 7;
    } else {
        idx = (92 + ch * 2) / 8;
        bit = (92 + ch * 2) & 7;
    }
    UINT8 mask = (1 << bit);
    ICU.IER[idx].BYTE = (ICU.IER[idx].BYTE & ~mask) | (flag << bit);
}

void sci_rx_int_enable(int ch) {
    sci_rx_set_int(ch, 1);
}

void sci_rx_int_disable(int ch) {
    sci_rx_set_int(ch, 0);
}

void sci_tx_set_int(int ch, int flag) {
    int idx = 0;
    int bit = 0;
    if (ch < 3) {
        idx = (59 + ch * 2) / 8;
        bit = (59 + ch * 2) & 7;
    } else if (ch < 7) {
        idx = (75 + ch * 2) / 8;
        bit = (75 + ch * 2) & 7;
    } else if (ch < 11) {
        idx = (85 + ch * 2) / 8;
        bit = (85 + ch * 2) & 7;
    } else {
        idx = (93 + ch * 2) / 8;
        bit = (93 + ch * 2) & 7;
    }
    UINT8 mask = (1 << bit);
    ICU.IER[idx].BYTE = (ICU.IER[idx].BYTE & ~mask) | (flag << bit);
}

void sci_tx_int_enable(int ch) {
    sci_tx_set_int(ch, 1);
}

void sci_tx_int_disable(int ch) {
    sci_tx_set_int(ch, 0);
}

void sci_te_set_int(int ch, int flag) {
    switch (ch) {
    case 0:
        ICU.GENBL0.BIT.EN0 = flag;  // vec: 110
        break;
    case 1:
        ICU.GENBL0.BIT.EN2 = flag;  // vec: 110
        break;
    case 2:
        ICU.GENBL0.BIT.EN4 = flag;  // vec: 110
        break;
    case 3:
        ICU.GENBL0.BIT.EN6 = flag;  // vec: 110
        break;
    case 4:
        ICU.GENBL0.BIT.EN8 = flag;  // vec: 110
        break;
    case 5:
        ICU.GENBL0.BIT.EN10 = flag; // vec: 110
        break;
    case 6:
        ICU.GENBL0.BIT.EN12 = flag; // vec: 110
        break;
    case 7:
        ICU.GENBL0.BIT.EN14 = flag; // vec: 110
        break;
    case 8:
        ICU.GENBL1.BIT.EN24 = flag; // vec: 111
        break;
    case 9:
        ICU.GENBL1.BIT.EN26 = flag; // vec: 111
        break;
    case 10:
        ICU.GENAL0.BIT.EN8 = flag;  // vec: 112
        break;
    case 11:
        ICU.GENAL0.BIT.EN12 = flag; // vec: 112
        break;
    case 12:
        ICU.GENBL0.BIT.EN16 = flag; // vec: 110
        break;
    }
}

void sci_te_int_enable(int ch) {
    sci_te_set_int(ch, 1);
}

void sci_te_int_disable(int ch) {
    sci_te_set_int(ch, 0);
}

void sci_er_set_int(int ch, int flag) {
    switch (ch) {
    case 0:
        ICU.GENBL0.BIT.EN1 = flag;  // vec: 110
        break;
    case 1:
        ICU.GENBL0.BIT.EN3 = flag;  // vec: 110
        break;
    case 2:
        ICU.GENBL0.BIT.EN5 = flag;  // vec: 110
        break;
    case 3:
        ICU.GENBL0.BIT.EN7 = flag;  // vec: 110
        break;
    case 4:
        ICU.GENBL0.BIT.EN9 = flag;  // vec: 110
        break;
    case 5:
        ICU.GENBL0.BIT.EN11 = flag; // vec: 110
        break;
    case 6:
        ICU.GENBL0.BIT.EN13 = flag; // vec: 110
        break;
    case 7:
        ICU.GENBL0.BIT.EN15 = flag; // vec: 110
        break;
    case 8:
        ICU.GENBL1.BIT.EN25 = flag; // vec: 111
        break;
    case 9:
        ICU.GENBL1.BIT.EN27 = flag; // vec: 111
        break;
    case 10:
        ICU.GENBL1.BIT.EN29 = flag; // vec: 110
        break;
    case 11:
        ICU.GENAL0.BIT.EN9 = flag;  // vec: 112
        break;
    case 12:
        ICU.GENAL0.BIT.EN13 = flag; // vec: 112
        break;
    }
}

void sci_er_int_enable(int ch) {
    sci_er_set_int(ch, 1);
}

void sci_er_int_disable(int ch) {
    sci_er_set_int(ch, 0);
}

static void sci_isr_rx(int ch) {
    USART_RxISR((int)ch_port[ch]);
}

void sci_isr_er(int ch) {
    volatile struct st_sci0 *sci = SCI[ch];
    sci->RDR;
    //sci->SSR.BYTE = 0x84;
    while (0 != (sci->SSR.BYTE & 0x38)) {
        sci->RDR;
        sci->SSR.BYTE = (sci->SSR.BYTE & ~0x38) | 0xc0;
        if (0 != (sci->SSR.BYTE & 0x38)) {
            __asm__ __volatile__("nop");
        }
    }
}

static void sci_isr_tx(int ch) {
    USART_TxISR((int)ch_port[ch]);
}

void sci_isr_te(int ch) {
    // ToDo
#if 0
    volatile struct st_sci0 *sci = SCI[ch];
    rx_disable_irq();
    sci->SCR.BYTE &= ~0xa4; /* TIE, TE and TEI reset */
    //if (tx_fifo[ch].len == 0) {
    //    tx_fifo[ch].busy = 0;
    //} else {
        sci->SCR.BYTE |= 0xa0;  /* TIE and TE set */
    //}
    rx_enable_irq();
#endif
}

void sci_int_priority(int ch, int priority) {
    switch (ch) {
    case 0:
        IPR(SCI0, RXI0) = priority;
        IPR(SCI0, TXI0) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    case 1:
        IPR(SCI1, RXI1) = priority;
        IPR(SCI1, TXI1) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    case 2:
        IPR(SCI2, RXI2) = priority;
        IPR(SCI2, TXI2) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    case 3:
        IPR(SCI3, RXI3) = priority;
        IPR(SCI3, TXI3) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    case 4:
        IPR(SCI4, RXI4) = priority;
        IPR(SCI4, TXI4) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    case 5:
        IPR(SCI5, RXI5) = priority;
        IPR(SCI5, TXI5) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    case 6:
        IPR(SCI6, RXI6) = priority;
        IPR(SCI6, TXI6) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    case 7:
        IPR(SCI7, RXI7) = priority;
        IPR(SCI7, TXI7) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    case 8:
        IPR(SCI8, RXI8) = priority;
        IPR(SCI8, TXI8) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl1(priority);
#endif
        break;
    case 9:
        IPR(SCI9, RXI9) = priority;
        IPR(SCI9, TXI9) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl1(priority);
#endif
        break;
    case 10:
        IPR(SCI10, RXI10) = priority;
        IPR(SCI10, TXI10) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupal0(priority);
#endif
        break;
    case 11:
        IPR(SCI11, RXI11) = priority;
        IPR(SCI11, TXI11) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupal0(priority);
#endif
        break;
    case 12:
        IPR(SCI12, RXI12) = priority;
        IPR(SCI12, TXI12) = priority;
#if (defined(RX64M) || defined(RX65N))
        set_int_priority_groupbl0(priority);
#endif
        break;
    default:
        break;
    }
}

void sci_int_enable(int ch) {
    // ToDo
    sci_rx_set_int(ch, 1);
    sci_tx_set_int(ch, 1);
    //sci_er_set_int(ch, 1);
    //sci_te_set_int(ch, 1);
    //if (ch < 8 || ch == 12) {
    //    set_int_state_groupbl0(1);
    //} else if (ch == 8 || ch == 9) {
    //    set_int_state_groupbl1(1);
    //} else {
    //    set_int_state_groupal0(1);
    //}
}

void sci_int_disable(int ch) {
    // ToDo
    sci_rx_set_int(ch, 0);
    sci_tx_set_int(ch, 0);
    //sci_er_set_int(ch, 0);
    //sci_te_set_int(ch, 0);
    //if (ch < 8 || ch == 12) {
    //    set_int_state_groupbl0(0);
    //} else if (ch == 8 || ch == 9) {
    //    set_int_state_groupbl1(0);
    //} else {
    //    set_int_state_groupal0(0);
    //}
}

void sci_module(int ch, int flag) {
    switch (ch) {
    case 0:
        MSTP_SCI0 = flag;
        break;
    case 1:
        MSTP_SCI1 = flag;
        break;
    case 2:
        MSTP_SCI2 = flag;
        break;
    case 3:
        MSTP_SCI3 = flag;
        break;
    case 4:
        MSTP_SCI4 = flag;
        break;
    case 5:
        MSTP_SCI5 = flag;
        break;
    case 6:
        MSTP_SCI6 = flag;
        break;
    case 7:
        MSTP_SCI7 = flag;
        break;
    case 8:
        MSTP_SCI8 = flag;
        break;
    case 9:
        MSTP_SCI9 = flag;
        break;
    case 10:
        MSTP_SCI10 = flag;
        break;
    case 11:
        MSTP_SCI11 = flag;
        break;
    case 12:
        MSTP_SCI12 = flag;
        break;
    default:
        break;
    }
}

void sci_module_start(int ch) {
    sci_module(ch, 0);
}

void sci_module_stop(int ch) {
    sci_module(ch, 1);
}

// PCLK is defined in platform_selector.h
void sci_set_baud(int ch, int baud) {
    volatile struct st_sci0 *sci = SCI[ch];
    sci->SEMR.BYTE |= 0x50; //  BGDM and ABCS
    if (baud != 0) {
        sci->BRR = (UINT8)((int)PCLK / baud / 8 - 1);
    } else {
        sci->BRR = (UINT8)((int)PCLK / SCI_DEFAULT_BAUD / 8 - 1);
    }
}

/*
 * bits: 7, 8, 9
 * parity: none:0, odd:1, even:2
 */
void sci_init_with_pins(int ch, int tx_pin, int rx_pin, int baud, int bits, int parity, int stop, int flow) {
    UINT8 smr = 0;
    UINT8 scmr = 0xf2;
    volatile struct st_sci0 *sci = SCI[ch];

    SYSTEM.PRCR.WORD = 0xA502;
    MPC.PWPR.BIT.B0WI = 0; /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
    sci_module_start(ch);
    UINT8 tx_port = GPIO_PORT(tx_pin);
    UINT8 tx_mask = GPIO_MASK(tx_pin);
    UINT8 rx_port = GPIO_PORT(rx_pin);
    UINT8 rx_mask = GPIO_MASK(rx_pin);
    _PMR(tx_port) &= ~tx_mask;
    _PMR(rx_port) &= ~rx_mask;
    _PDR(tx_port) |= tx_mask;
    _PDR(rx_port) &= ~rx_mask;
    _PXXPFS(tx_port, tx_pin & 7) = 0x0a;
    _PXXPFS(rx_port, rx_pin & 7) = 0x0a;
    _PMR(tx_port) |= tx_mask;
    _PMR(rx_port) |= rx_mask;
    //MPC.PWPR.BYTE = 0x80;     /* Disable write to PFSWE and PFS*/
    SYSTEM.PRCR.WORD = 0xA500;
    sci->SCR.BYTE = 0;
    if (bits == 7) {
        smr |= 0x40;
    } else {
        smr &= ~0x40;
    }
    if (parity != 0) {
        smr |= 0x20;
    } else {
        smr &= ~0x20;
    }
    if (parity == 1) {
        smr |= 0x10;
    } else {
        smr &= ~0x10;
    }
    if (stop == 2) {
        smr |= 0x80;
    } else {
        smr &= ~0x80;
    }
    sci->SMR.BYTE = smr;
    if (bits == 9) {
        scmr &= ~0x10;
    } else {
        scmr |= 0x10;
    }
    sci->SCR.BYTE = 0;
    sci->SMR.BYTE = smr;
    sci->SCMR.BYTE = scmr;
    sci_set_baud(ch, baud);
    delay_us(10);
    //sci->SCR.BYTE = 0xd0; // when using TX interrupt
    sci_int_priority(ch, SCI_DEFAULT_PRIORITY);
    sci_int_enable(ch);
    sci->SCR.BYTE = (SCR_RE_M | SCR_RIE_M);
}

void sci_init(int ch, int baud, int bits, int parity, int stop, int flow) {
    int tx_pin = (int)sci_tx_pins[ch];
    int rx_pin = (int)sci_rx_pins[ch];
    if ((tx_pin != 0xff) && (rx_pin != 0xff)) {
        sci_init_with_pins(ch, tx_pin, rx_pin, baud, bits, parity, stop, flow);
    }
}

void sci_init_default(int ch, int baud) {
    sci_init(ch, baud, 8, 0, 1, 0);
}

void sci_deinit(int ch) {
    sci_int_disable(ch);
    SYSTEM.PRCR.WORD = 0xA502;
    MPC.PWPR.BIT.B0WI = 0;      /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1;     /* Enable write to PFS */
    sci_module_stop(ch);
    MPC.PWPR.BYTE = 0x80;       /* Disable write to PFSWE and PFS*/
    SYSTEM.PRCR.WORD = 0xA500;
}

BOOL CPU_USART_Initialize(int ComPortNum, int BaudRate, int Parity, int DataBits, int StopBits, int FlowValue)
{
    UINT8 ch = port_ch[ComPortNum];
    if (ch == 0xff)
        return FALSE;
    sci_init(ch, BaudRate, DataBits, Parity, StopBits, FlowValue);
    return TRUE;
}

BOOL CPU_USART_Uninitialize(int ComPortNum)
{
    UINT8 ch = port_ch[ComPortNum];
    if (ch == 0xff)
        return FALSE;
    sci_deinit(ch);
    return TRUE;
}

BOOL CPU_USART_TxBufferEmpty(int ComPortNum)
{
    return ((*SCI[port_ch[ComPortNum]]).SSR.BIT.TEND != 0);
}

BOOL CPU_USART_TxShiftRegisterEmpty(int ComPortNum)
{
    return CPU_USART_TxBufferEmpty(ComPortNum);
}

void CPU_USART_WriteCharToTxBuffer(int ComPortNum, UINT8 c)
{
    while (FALSE == CPU_USART_TxShiftRegisterEmpty(ComPortNum));
    (*SCI[port_ch[ComPortNum]]).TDR = c;
}

void CPU_USART_TxBufferEmptyInterruptEnable(int ComPortNum, BOOL Enable)
{
    while (FALSE == CPU_USART_TxShiftRegisterEmpty(ComPortNum));
    if (Enable)
        (*SCI[port_ch[ComPortNum]]).SCR.BYTE |= (SCR_TE_M | SCR_TIE_M);
    else
        (*SCI[port_ch[ComPortNum]]).SCR.BYTE &=  ~(SCR_TE_M | SCR_TIE_M);
}

BOOL CPU_USART_TxBufferEmptyInterruptState(int ComPortNum)
{
    return ((*SCI[port_ch[ComPortNum]]).SCR.BIT.TIE != 0);
}

void CPU_USART_RxBufferFullInterruptEnable(int ComPortNum, BOOL Enable)
{
    if (Enable)
        (*SCI[port_ch[ComPortNum]]).SCR.BIT.RIE = 1;
    else
        (*SCI[port_ch[ComPortNum]]).SCR.BIT.RIE = 0;
}

BOOL CPU_USART_RxBufferFullInterruptState(int ComPortNum)
{
    return ((*SCI[port_ch[ComPortNum]]).SCR.BIT.RIE != 0);
}

void CPU_USART_ProtectPins(int ComPortNum, BOOL On)
{
    // ToDo
}

UINT32 CPU_USART_PortsCount()
{
    return TOTAL_USART_PORT;
}

void CPU_USART_GetPins(int ComPortNum, GPIO_PIN& rxPin, GPIO_PIN& txPin, GPIO_PIN& ctsPin, GPIO_PIN& rtsPin)
{
    UINT8 ch = port_ch[ComPortNum];
    if (ch == 0xff)
        ch = 1;
    rxPin = (GPIO_PIN)sci_rx_pins[ch];
    txPin = (GPIO_PIN)sci_tx_pins[ch];
    ctsPin= GPIO_PIN_NONE;
    rtsPin= GPIO_PIN_NONE;
    return;
}

BOOL CPU_USART_SupportNonStandardBaudRate (int ComPortNum)
{
    // ToDO
    return FALSE;
}

void CPU_USART_GetBaudrateBoundary(int ComPortNum, UINT32& maxBaudrateHz, UINT32& minBaudrateHz)
{
    maxBaudrateHz = 115200;
    minBaudrateHz = 2400;
}

BOOL CPU_USART_IsBaudrateSupported(int ComPortNum, UINT32 & BaudrateHz)
{
    // ToDO
    return FALSE;
}

BOOL CPU_USART_TxHandshakeEnabledState(int comPort)
{
    // ToDo
    return TRUE;
}

static void USART_RxISR(UINT32 port)
{
    char c;
    GLOBAL_LOCK(irq);
    c = (*SCI[port_ch[port]]).RDR;
    USART_AddCharToRxBuffer(port, c);
}

static void USART_TxISR(UINT32 port)
{
    char c;
    GLOBAL_LOCK(irq);
    if (USART_RemoveCharFromTxBuffer(port, c)) {
        CPU_USART_WriteCharToTxBuffer(port, c);
        Events_Set(SYSTEM_EVENT_FLAG_COM_OUT);
    } else {
        CPU_USART_TxBufferEmptyInterruptEnable(port, FALSE);
    }
}

#ifdef __cplusplus
extern "C" {
#endif

#define INTR __attribute__ ((interrupt))

/* rx interrupt */
void INTR INT_Excep_SCI0_RXI0(void) {
    sci_isr_rx(0);
}
void INTR INT_Excep_SCI1_RXI1(void) {
    sci_isr_rx(1);
}
void INTR INT_Excep_SCI2_RXI2(void) {
    sci_isr_rx(2);
}
void INTR INT_Excep_SCI3_RXI3(void) {
    sci_isr_rx(3);
}
void INTR INT_Excep_SCI4_RXI4(void) {
    sci_isr_rx(4);
}
void INTR INT_Excep_SCI5_RXI5(void) {
    sci_isr_rx(5);
}
void INTR INT_Excep_SCI6_RXI6(void) {
    sci_isr_rx(6);
}
void INTR INT_Excep_SCI7_RXI7(void) {
    sci_isr_rx(7);
}
void INTR INT_Excep_SCI8_RXI8(void) {
    sci_isr_rx(8);
}
void INTR INT_Excep_SCI9_RXI9(void) {
    sci_isr_rx(9);
}
void INTR INT_Excep_SCI10_RXI10(void) {
    sci_isr_rx(10);
}
void INTR INT_Excep_SCI11_RXI11(void) {
    sci_isr_rx(11);
}
void INTR INT_Excep_SCI12_RXI12(void) {
    sci_isr_rx(12);
}

/* tx interrupt */
void INTR INT_Excep_SCI0_TXI0(void) {
    sci_isr_tx(0);
}
void INTR INT_Excep_SCI1_TXI1(void) {
    sci_isr_tx(1);
}
void INTR INT_Excep_SCI2_TXI2(void) {
    sci_isr_tx(2);
}
void INTR INT_Excep_SCI3_TXI3(void) {
    sci_isr_tx(3);
}
void INTR INT_Excep_SCI4_TXI4(void) {
    sci_isr_tx(4);
}
void INTR INT_Excep_SCI5_TXI5(void) {
    sci_isr_tx(5);
}
void INTR INT_Excep_SCI6_TXI6(void) {
    sci_isr_tx(6);
}
void INTR INT_Excep_SCI7_TXI7(void) {
    sci_isr_tx(7);
}
void INTR INT_Excep_SCI8_TXI8(void) {
    sci_isr_tx(8);
}
void INTR INT_Excep_SCI9_TXI9(void) {
    sci_isr_tx(9);
}
void INTR INT_Excep_SCI10_TXI10(void) {
    sci_isr_tx(10);
}
void INTR INT_Excep_SCI11_TXI11(void) {
    sci_isr_tx(11);
}
void INTR INT_Excep_SCI12_TXI12(void) {
    sci_isr_tx(12);
}

#if defined(RX63N)
/* er interrupt */
void INTR INT_Excep_SCI0_ERI0(void) {
    sci_isr_er(0);
}
void INTR INT_Excep_SCI1_ERI1(void) {
    sci_isr_er(1);
}
void INTR INT_Excep_SCI2_ERI2(void) {
    sci_isr_er(2);
}
void INTR INT_Excep_SCI3_ERI3(void) {
    sci_isr_er(3);
}
void INTR INT_Excep_SCI4_ERI4(void) {
    sci_isr_er(4);
}
void INTR INT_Excep_SCI5_ERI5(void) {
    sci_isr_er(5);
}
void INTR INT_Excep_SCI6_ERI6(void) {
    sci_isr_er(6);
}
void INTR INT_Excep_SCI0_ERI7(void) {
    sci_isr_er(7);
}
void INTR INT_Excep_SCI1_ERI8(void) {
    sci_isr_er(8);
}
void INTR INT_Excep_SCI2_ERI9(void) {
    sci_isr_er(9);
}
void INTR INT_Excep_SCI3_ERI10(void) {
    sci_isr_er(10);
}
void INTR INT_Excep_SCI4_ERI11(void) {
    sci_isr_er(11);
}
void INTR INT_Excep_SCI5_ERI2(void) {
    sci_isr_er(12);
}

/* te interrupt */
void INTR INT_Excep_SCI0_TEI0(void) {
    sci_isr_te(0);
}
void INTR INT_Excep_SCI1_TEI1(void) {
    sci_isr_te(1);
}
void INTR INT_Excep_SCI2_TEI2(void) {
    sci_isr_te(2);
}
void INTR INT_Excep_SCI3_TEI3(void) {
    sci_isr_te(3);
}
void INTR INT_Excep_SCI4_TEI4(void) {
    sci_isr_te(4);
}
void INTR INT_Excep_SCI5_TEI5(void) {
    sci_isr_te(5);
}
void INTR INT_Excep_SCI6_TEI6(void) {
    sci_isr_te(6);
}
void INTR INT_Excep_SCI7_TEI7(void) {
    sci_isr_te(7);
}
void INTR INT_Excep_SCI8_TEI8(void) {
    sci_isr_te(8);
}
void INTR INT_Excep_SCI9_TEI9(void) {
    sci_isr_te(9);
}
void INTR INT_Excep_SCI10_TEI10(void) {
    sci_isr_te(10);
}
void INTR INT_Excep_SCI11_TEI11(void) {
    sci_isr_te(11);
}
void INTR INT_Excep_SCI12_TEI12(void) {
    sci_isr_te(12);
}
#endif

#ifdef __cplusplus
}
#endif
