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

#include <tinyhal.h>
#include "..\RX63N.h"

//#define DEBUG_I2C
//#define DEBUG_I2C_REG_DUMP
//#define DEBUG_I2C_CLK
//#define DEBUG_I2C_INT
//#define DEBUG_I2C_ERR
//#define DEBUG_I2C_NACK
//#define DEBUG_I2C_DETAIL
//#define DEBUG_I2C_TX_DATA
//#define DEBUG_I2C_RX_DATA

#define I2C_TIMEOUT_STOP_CONDITION 100000
#define I2C_TIMEOUT_BUS_BUSY 100000

#define RX63N_SCL RX63N_GPIO::P12;
#define RX63N_SDA RX63N_GPIO::P13;

RX63N_I2C_Driver g_RX63N_I2C_Driver;

GPIO_PIN SCL_PIN[] = {
    RX63N_GPIO::P12,
    RX63N_GPIO::P12,
    RX63N_GPIO::P12,
    RX63N_GPIO::P12
};

GPIO_PIN SDA_PIN[] = {
    RX63N_GPIO::P13,
    RX63N_GPIO::P13,
    RX63N_GPIO::P13,
    RX63N_GPIO::P13
};

typedef volatile struct st_riic riic_t;
riic_t *RIIC[] = {
    (riic_t *)0x88300,
    (riic_t *)0x88320,
    (riic_t *)0x88340,
    (riic_t *)0x88360
};

static void delay(INT32 t)
{
    while (t-- > 0) {
        asm volatile ("nop");
    }
}

static void clear_i2c_line(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];
    if (riicp->ICCR1.BIT.SDAI == 0) {
        INT32 t = 10;
        while (t-- > 0) {
            riicp->ICCR1.BIT.CLO = 1;
            while (riicp->ICCR1.BIT.CLO == 1) ;
            if (riicp->ICCR1.BIT.SDAI == 1)
                break;
        }
    }
}

static void i2c_reg_dump(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];
    debug_printf("CR1=%02X CR2=%02X MR1=%02X MR2=%02X MR3=%02X\r\n",
            riicp->ICCR1.BYTE, riicp->ICCR2.BYTE, riicp->ICMR1.BYTE, riicp->ICMR3.BYTE, riicp->ICMR3.BYTE);
    debug_printf("FER=%02X SER=%02X IER=%02X SR1=%02X SR2=%02X\r\n",
            riicp->ICFER.BYTE, riicp->ICSER.BYTE, riicp->ICIER.BYTE, riicp->ICSR1.BYTE, riicp->ICSR2.BYTE);
}

static void i2c_IPR(UINT32 ch, UINT32 val)
{
    switch (ch) {
    case 0:
        IPR(RIIC0,EEI0) = val;
        IPR(RIIC0,RXI0) = val;
        IPR(RIIC0,TXI0) = val;
        IPR(RIIC0,TEI0) = val;
        break;
    case 1:
        IPR(RIIC1,EEI1) = val;
        IPR(RIIC1,RXI1) = val;
        IPR(RIIC1,TXI1) = val;
        IPR(RIIC1,TEI1) = val;
        break;
    case 2:
        IPR(RIIC2,EEI2) = val;
        IPR(RIIC2,RXI2) = val;
        IPR(RIIC2,TXI2) = val;
        IPR(RIIC2,TEI2) = val;
        break;
    case 3:
        IPR(RIIC3,EEI3) = val;
        IPR(RIIC3,RXI3) = val;
        IPR(RIIC3,TXI3) = val;
        IPR(RIIC3,TEI3) = val;
        break;
    }
}

static void i2c_IR(UINT32 ch, UINT32 val)
{
    switch (ch) {
    case 0:
        IR(RIIC0,EEI0) = val;
        IR(RIIC0,RXI0) = val;
        IR(RIIC0,TXI0) = val;
        IR(RIIC0,TEI0) = val;
        break;
    case 1:
        IR(RIIC1,EEI1) = val;
        IR(RIIC1,RXI1) = val;
        IR(RIIC1,TXI1) = val;
        IR(RIIC1,TEI1) = val;
        break;
    case 2:
        IR(RIIC2,EEI2) = val;
        IR(RIIC2,RXI2) = val;
        IR(RIIC2,TXI2) = val;
        IR(RIIC2,TEI2) = val;
        break;
    case 3:
        IR(RIIC3,EEI3) = val;
        IR(RIIC3,RXI3) = val;
        IR(RIIC3,TXI3) = val;
        IR(RIIC3,TEI3) = val;
        break;
    }
}

static void i2c_IEN(UINT32 ch, UINT32 val)
{
    switch (ch) {
    case 0:
        IEN(RIIC0,EEI0) = val;
        IEN(RIIC0,RXI0) = val;
        IEN(RIIC0,TXI0) = val;
        IEN(RIIC0,TEI0) = val;
        break;
    case 1:
        IEN(RIIC1,EEI1) = val;
        IEN(RIIC1,RXI1) = val;
        IEN(RIIC1,TXI1) = val;
        IEN(RIIC1,TEI1) = val;
        break;
    case 2:
        IEN(RIIC2,EEI2) = val;
        IEN(RIIC2,RXI2) = val;
        IEN(RIIC2,TXI2) = val;
        IEN(RIIC2,TEI2) = val;
        break;
    case 3:
        IEN(RIIC3,EEI3) = val;
        IEN(RIIC3,RXI3) = val;
        IEN(RIIC3,TXI3) = val;
        IEN(RIIC3,TEI3) = val;
        break;
    }
}

static void i2c_Pin(UINT32 ch)
{
    switch (ch) {
    case 0:
        PORT1.PCR.BIT.B2 = 0;
        PORT1.PCR.BIT.B3 = 0;
        PORT1.PDR.BIT.B2 = 0;
        PORT1.PDR.BIT.B3 = 0;
        PORT1.PMR.BIT.B2 = 0;
        PORT1.PMR.BIT.B3 = 0;
        PORT1.ODR0.BIT.B4 = 1;
        PORT1.ODR0.BIT.B6 = 1;
        MPC.PWPR.BYTE = 0x40;
        MPC.P12PFS.BIT.PSEL = 0x0F;
        MPC.P13PFS.BIT.PSEL = 0x0F;
        //MPC.PWPR.BIT.PFSWE = 0;
        //MPC.PWPR.BIT.B0WI  = 1;
        PORT1.PMR.BIT.B2 = 1;
        PORT1.PMR.BIT.B3 = 1;
        break;
    case 1:
        PORT1.PMR.BIT.B2 = 0;
        PORT1.PMR.BIT.B3 = 0;
        MPC.PWPR.BIT.B0WI  = 0;
        MPC.PWPR.BIT.PFSWE = 1;
        MPC.P12PFS.BYTE = 0x0F;
        MPC.P13PFS.BYTE = 0x0F;
        MPC.PWPR.BIT.PFSWE = 0;
        MPC.PWPR.BIT.B0WI  = 1;
        PORT1.PMR.BIT.B2 = 1;
        PORT1.PMR.BIT.B3 = 1;
        PORT1.ODR0.BIT.B4 = 1;
        PORT1.ODR0.BIT.B6 = 1;
        break;
    case 2:
        PORT1.PMR.BIT.B2 = 0;
        PORT1.PMR.BIT.B3 = 0;
        MPC.PWPR.BIT.B0WI  = 0;
        MPC.PWPR.BIT.PFSWE = 1;
        MPC.P12PFS.BYTE = 0x0F;
        MPC.P13PFS.BYTE = 0x0F;
        MPC.PWPR.BIT.PFSWE = 0;
        MPC.PWPR.BIT.B0WI  = 1;
        PORT1.PMR.BIT.B2 = 1;
        PORT1.PMR.BIT.B3 = 1;
        PORT1.ODR0.BIT.B4 = 1;
        PORT1.ODR0.BIT.B6 = 1;
        break;
    case 3:
        PORT1.PMR.BIT.B2 = 0;
        PORT1.PMR.BIT.B3 = 0;
        MPC.PWPR.BIT.B0WI  = 0;
        MPC.PWPR.BIT.PFSWE = 1;
        MPC.P12PFS.BYTE = 0x0F;
        MPC.P13PFS.BYTE = 0x0F;
        MPC.PWPR.BIT.PFSWE = 0;
        MPC.PWPR.BIT.B0WI  = 1;
        PORT1.PMR.BIT.B2 = 1;
        PORT1.PMR.BIT.B3 = 1;
        PORT1.ODR0.BIT.B4 = 1;
        PORT1.ODR0.BIT.B6 = 1;
        break;
    }
}

static void i2c_Power(UINT32 ch)
{
    SYSTEM.PRCR.WORD = 0xA502;
    switch (ch) {
    case 0:
        MSTP(RIIC0) = 0;
        break;
    case 1:
        MSTP(RIIC1) = 0;
        break;
    case 2:
        MSTP(RIIC2) = 0;
        break;
    case 3:
        MSTP(RIIC3) = 0;
        break;
    }
    SYSTEM.PRCR.WORD = 0xA500;
}

BOOL RX63N_I2C_Driver::Initialize(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];

//    if (g_RX63N_I2C_Driver.m_initialized == FALSE) {
        g_RX63N_I2C_Driver.m_currentXAction = NULL;
        g_RX63N_I2C_Driver.m_currentXActionUnit = NULL;

        // CPU_GPIO_DisablePin((GPIO_PIN)RX63N_SCL, RESISTOR_DISABLED, 0, GPIO_ALT_MODE_1);
        // CPU_GPIO_DisablePin((GPIO_PIN)RX63N_SDA, RESISTOR_DISABLED, 0, GPIO_ALT_MODE_1);

        i2c_Power(ch);
        i2c_IPR(ch, 4);
        i2c_IR(ch, 0);
        i2c_IEN(ch, 0);
        i2c_Pin(ch);
        
        riicp->ICCR1.BIT.ICE = 0;       // I2C disable
        riicp->ICCR1.BIT.IICRST = 1;    // I2C internal reset
        riicp->ICCR1.BIT.ICE = 1;       // I2C enable
        // Standard Mode
        riicp->ICMR1.BIT.CKS = 0x03;    // I2C CKS=3 then n=8, PCLK/n = 6000000
        riicp->ICBRH.BIT.BRH = 24;      // Set High width of SCL
        riicp->ICBRL.BIT.BRL = 29;      // Set Low width of SCL
        //riicp->ICFER.BYTE = 0x72;       // Default
        //riicp->ICMR2.BIT.TMOS = 0x01;
        //riicp->ICMR2.BIT.TMOL = 0x01;
        //riicp->ICMR2.BIT.TMOH = 0x01;
        //riicp->ICMR3.BIT.WAIT = 0x01;
        //riicp->ICMR3.BIT.NF = 0x02;
        //riicp->ICSR1.BYTE = 0x00;
        //riicp->ICSR2.BYTE = 0x00;
        riicp->ICSER.BYTE = 0x00;       // I2C reset bus status enable register
        riicp->ICMR3.BIT.ACKWP = 0x01;  // I2C allow to write ACKBT (transfer acknowledge bit)
        riicp->ICIER.BYTE = 0xF0;       // b0: Disable Time Out interrupt */
                                        // b1: Disable Arbitration Lost interrupt */
                                        // b2: Disable Start Condition Detection Interrupt */
                                        // b3: Disable Stop condition detection interrupt */
                                        // b4: Disable NACK reception interrupt */
                                        // b5: Enable Receive Data Full Interrupt */
                                        // b6: Enable Transmit End Interrupt */
                                        // b7: Enable Transmit Data Empty Interrupt */
        riicp->ICCR1.BIT.IICRST = 0;    // I2C internal reset
        clear_i2c_line(ch);
        g_RX63N_I2C_Driver.m_initialized  = TRUE;
#if defined(DEBUG_I2C)
    debug_printf("I2C Init\r\n");
#endif
//    }
    return TRUE;
}

BOOL RX63N_I2C_Driver::Uninitialize(UINT32 ch)
{
    NATIVE_PROFILE_HAL_PROCESSOR_I2C();
    riic_t *riicp = RIIC[ch];

//    if (g_RX63N_I2C_Driver.m_initialized == TRUE) {
        g_RX63N_I2C_Driver.m_initialized = FALSE;
        // CPU_GPIO_EnableInputPin((GPIO_PIN)RX63N_SCL, FALSE, NULL, GPIO_INT_NONE, RESISTOR_DISABLED);
        // CPU_GPIO_EnableInputPin((GPIO_PIN)RX63N_SDA, NULL, GPIO_INT_NONE, RESISTOR_DISABLED);
        riicp->ICIER.BYTE = 0;           // I2C interrupt disable
        riicp->ICCR1.BIT.ICE = 0;        // I2C disable
#if defined(DEBUG_I2C)
    debug_printf("I2C Uninit\r\n");
#endif
//    }
    return TRUE;
}

void RX63N_I2C_Driver::ReadLastByte(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];
    I2C_HAL_XACTION *xAction = g_RX63N_I2C_Driver.m_currentXAction;
    I2C_HAL_XACTION_UNIT *unit = g_RX63N_I2C_Driver.m_currentXActionUnit;

    //while ((riicp->ICSR2.BIT.RDRF) == 0);
    if(xAction->ProcessingLastUnit()) {
        riicp->ICSR2.BIT.STOP = 0;
        riicp->ICCR2.BIT.SP = 1;
        ReadFromSubordinate(ch, unit);
        riicp->ICMR3.BIT.WAIT = 0;
        UINT32 timeout = I2C_TIMEOUT_STOP_CONDITION;
        while (timeout-- > 0) {
            if (riicp->ICSR2.BIT.STOP != 0)
                break;
        }
        riicp->ICSR2.BIT.NACKF = 0;
        riicp->ICSR2.BIT.STOP = 0;
        xAction->Signal(I2C_HAL_XACTION::c_Status_Completed);//Complete
    } else {
        ReadFromSubordinate(ch, unit);
        MasterXAction_Start(ch, xAction, true);
    }
}

void RX63N_I2C_Driver::StopCondition(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];
    riicp->ICSR2.BIT.NACKF = 0;
    riicp->ICSR2.BIT.STOP = 0;
    riicp->ICCR2.BIT.SP= 1;
    UINT32 timeout = I2C_TIMEOUT_STOP_CONDITION;
    while (riicp->ICSR2.BIT.STOP == 0) {
        if (timeout-- == 0) {
#ifdef DEBUG_I2C
            debug_printf("I2C SC TO\r\n");
#endif
            break;
        }
    }
    riicp->ICSR2.BIT.STOP = 0;
#ifdef DEBUG_I2C
    debug_printf("I2C SC OK\r\n");
#endif
}

void RX63N_I2C_Driver::Abort(UINT32 ch)
{
    UINT8 dummy;
    riic_t *riicp = RIIC[ch];
    I2C_HAL_XACTION *xAction = g_RX63N_I2C_Driver.m_currentXAction;
    StopCondition(ch);
    riicp->ICIER.BYTE = 0x00;
    //while (0x00 != riicp->ICIER.BYTE) ;
    i2c_IR(ch, 0);
    dummy = riicp->ICDRR;
    riicp->ICCR1.BIT.ICE = 0;
    xAction->Signal(I2C_HAL_XACTION::c_Status_Aborted);
}

void RX63N_I2C_Driver::MasterXAction_Start(UINT32 ch, I2C_HAL_XACTION* xAction, bool repeatedStart)
{
    NATIVE_PROFILE_HAL_PROCESSOR_I2C();
    UINT8 address;
    UINT32 timeout;
    riic_t *riicp = RIIC[ch];

//    if (g_RX63N_I2C_Driver.m_initialized == FALSE)
//        return;
    g_RX63N_I2C_Driver.m_currentXAction = xAction;
    g_RX63N_I2C_Driver.m_currentXActionUnit = xAction->m_xActionUnits[xAction->m_current++];
    I2C_HAL_XACTION_UNIT *unit = g_RX63N_I2C_Driver.m_currentXActionUnit;

    riicp->ICCR1.BIT.ICE = 0;    // I2C disable
    riicp->ICMR1.BYTE = (riicp->ICMR1.BYTE & 0x8f) | (xAction->m_clockRate2 << 4);
    riicp->ICBRH.BIT.BRH = xAction->m_clockRate & 0x1f;
    riicp->ICBRL.BIT.BRL = xAction->m_clockRate & 0x1f;
    riicp->ICIER.BYTE = 0xF0;
    riicp->ICCR1.BIT.ICE = 1;    // I2C enable
#ifdef DEBUG_I2C_REG_DUMP
    i2c_reg_dump(ch);
#endif

    address  = 0xFE & (xAction->m_address << 1);
    address |= unit->IsReadXActionUnit() ? RX63N_I2C_Driver::c_DirectionRead : RX63N_I2C_Driver::c_DirectionWrite;

    riicp->ICCR2.BIT.MST = 1;    // I2C master
    riicp->ICCR2.BIT.TRS = 1;    // I2C transmit
    if (repeatedStart) {
        while (riicp->ICCR2.BIT.BBSY == 0);
#ifdef DEBUG_I2C
        debug_printf("I2C RS SLA=%02X %02X %02X\r\n", address, riicp->ICSR1.BYTE, riicp->ICSR2.BYTE);
        //debug_printf("I2C RS SLA=%02X icmr1(cks)=%02X brh,brl=%02X\r\n", address, riicp->ICMR1.BYTE, riicp->ICBRH.BIT.BRH);
#endif
        riicp->ICCR2.BIT.RS = 1; // I2C repeatedstart condition
        while (riicp->ICCR2.BIT.RS == 1) ;
        //while (riicp->ICSR2.BIT.TDRE == 0) ;
    } else {
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (riicp->ICCR2.BIT.BBSY) {
            if (timeout-- == 0) {
#ifdef DEBUG_I2C
                debug_printf("I2C BBSY\r\n");
#endif
                Abort(ch);
                return;
            }
        }
#ifdef DEBUG_I2C
        debug_printf("I2C ST SLA=%02X %02X %02X\r\n", address, riicp->ICSR1.BYTE, riicp->ICSR2.BYTE);
        //debug_printf("I2C ST SLA=%02X icmr1(cks)=%02X brh,brl=%02X\r\n", address, riicp->ICMR1.BYTE, riicp->ICBRH.BIT.BRH);
#endif
        riicp->ICCR2.BIT.ST = 1; // I2C start condition
        timeout = I2C_TIMEOUT_BUS_BUSY;
        while (1) {
            if ((riicp->ICSR2.BIT.NACKF == 1) || (timeout-- == 0)) {
#ifdef DEBUG_I2C
                debug_printf("I2C NK %02X TO=%d\r\n", riicp->ICSR2.BYTE, timeout);
#endif
                Abort(ch);
                return;
            }
            if (riicp->ICSR2.BIT.TDRE != 0)
                break;
        }
    }
    riicp->ICDRT = address;      // I2C send slave address
                                // TODO: handling 10 bit address
    if (!unit->IsReadXActionUnit()) {
        while (riicp->ICSR2.BIT.TDRE == 0) ;
        i2c_IEN(ch, 1);
        riicp->ICIER.BYTE |= (RX63N_I2C::ICIER_TIE || RX63N_I2C::ICIER_TEIE);
    } else {
        //riicp->ICSR2.BIT.TEND = 0;
        //riicp->ICCR2.BIT.TRS = 0;
        //delay(1000);
        while (riicp->ICSR2.BIT.RDRF == 0) ;
        i2c_IEN(ch, 1);
        riicp->ICIER.BYTE |= (RX63N_I2C::ICIER_RIE || RX63N_I2C::ICIER_NAKIE || RX63N_I2C::ICIER_SPIE);
        if (unit->m_bytesToTransfer == 1) {
            riicp->ICMR3.BIT.WAIT = 1;
            riicp->ICMR3.BIT.ACKBT = 1;
        }
        UINT8 dummy = riicp->ICDRR;
    }
}

void RX63N_I2C_Driver::MasterXAction_Stop(UINT32 ch)
{
    NATIVE_PROFILE_HAL_PROCESSOR_I2C();
    riic_t *riicp = RIIC[ch];

    i2c_IR(ch, 0);
    i2c_IEN(ch, 0);
    riicp->ICIER.BYTE = 0x00;
    riicp->ICCR1.BIT.ICE = 0;
    g_RX63N_I2C_Driver.m_currentXAction = NULL;
    g_RX63N_I2C_Driver.m_currentXActionUnit = NULL;
#ifdef DEBUG_I2C
    debug_printf("I2C Stop\r\n");
#endif
}

static UINT8 pclk_div[8] = {
    1, 2, 4, 8, 16, 32, 64, 128
};

void RX63N_I2C_Driver::GetClockRate(UINT32 rateKhz, UINT8& clockRate, UINT8& clockRate2)
{
    UINT32 bgr;
    UINT8 div_idx = 3;
    if (rateKhz > 400)
        rateKhz = 400;
    if (rateKhz == 0)
        rateKhz = 1;
    rateKhz *= 1000;
    bgr = PCLK/rateKhz;
    if (bgr <= 480) {
        div_idx = 3;
    } else if (bgr <= 4800) {
        div_idx = 6;
    } else {
        div_idx = 7;
    }
    clockRate2 = (UINT8)div_idx;
    clockRate = (UINT8)(((bgr/pclk_div[div_idx]) & 0xff) / 2);
#ifdef DEBUG_I2C_CLK
    debug_printf("I2C CLK=%d %d\r\n", clockRate, clockRate2);
#endif
}

void RX63N_I2C_Driver::WriteToSubordinate(UINT32 ch, I2C_HAL_XACTION_UNIT *unit)
{
    UINT8 *queueData;
    riic_t *riicp = RIIC[ch];
    queueData = unit->m_dataQueue.Pop();
    ASSERT(queueData);
    //if (queueData == NULL) {
    //    debug_printf("I2C wque err\r\n");
    //    return;
    //}
#ifdef DEBUG_I2C_TX_DATA
    debug_printf("w%02x", *queueData);
    //debug_printf("w");
#endif
    while (riicp->ICSR2.BIT.TDRE == 0) ;
    riicp->ICDRT = *queueData;
    ++unit->m_bytesTransferred;
    --unit->m_bytesToTransfer;
}

void RX63N_I2C_Driver::ReadFromSubordinate(UINT32 ch, I2C_HAL_XACTION_UNIT *unit)
{
    UINT8* queueData;
    riic_t *riicp = RIIC[ch];
    queueData = unit->m_dataQueue.Push();
    ASSERT(queueData);
    //if (queueData == NULL) {
    //    debug_printf("I2C rque err\r\n");
    //    return;
    //}
    UINT8 data = riicp->ICDRR;
#ifdef DEBUG_I2C_RX_DATA
    debug_printf("r%02x", data);
    //debug_printf("r");
#endif
    *queueData = data;
    ++unit->m_bytesTransferred;
    --unit->m_bytesToTransfer;
}

void RX63N_I2C_ICEEI_ISR(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];
#ifdef DEBUG_I2C_INT
    debug_printf("I2C EEI %02X %02X\r\n", riicp->ICSR1.BYTE, riicp->ICSR2.BYTE);
#endif
    // Check Timeout
    if ((riicp->ICSR2.BIT.TMOF!=0) && (riicp->ICIER.BIT.TMOIE!=0)) {
#ifdef DEBUG_I2C_INT
        debug_printf("I2C TO\r\n");
#endif
        riicp->ICSR2.BIT.TMOF = 0;
    }
    // Check Arbitration Lost
    if ((riicp->ICSR2.BIT.AL!=0) && (riicp->ICIER.BIT.ALIE!=0)) {
#ifdef DEBUG_I2C_INT
        debug_printf("I2C AL\r\n");
#endif
        riicp->ICSR2.BIT.AL = 0;
    }
    // Check stop condition detection
    if ((riicp->ICSR2.BIT.STOP!=0) && (riicp->ICIER.BIT.SPIE!=0)) {
#ifdef DEBUG_I2C_INT
        debug_printf("I2C SC\r\n");
#endif
        riicp->ICSR2.BIT.STOP = 0;
        // ToDo:
    }
    // Check NACK reception
    if ((riicp->ICSR2.BIT.NACKF != 0) && (riicp->ICIER.BIT.NAKIE!=0)) {
        UINT8 dummy;
#ifdef DEBUG_I2C_INT
        debug_printf("I2C NK\r\n");
#endif
        riicp->ICSR2.BIT.NACKF = 0;
        g_RX63N_I2C_Driver.Abort(ch);
    }
    // Check start condition detection
    if ((riicp->ICSR2.BIT.START != 0) && (riicp->ICIER.BIT.STIE!=0)) {
#ifdef DEBUG_I2C_INT
        debug_printf("I2C SC\r\n");
#endif
        riicp->ICSR2.BIT.START = 0;
    }
}

void RX63N_I2C_ICRXI_ISR(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];
    I2C_HAL_XACTION_UNIT *unit = g_RX63N_I2C_Driver.m_currentXActionUnit;
    I2C_HAL_XACTION *xAction = g_RX63N_I2C_Driver.m_currentXAction;
    GLOBAL_LOCK(irq);
#ifdef DEBUG_I2C_INT
    debug_printf("I2C RXI\r\n");
#endif
    while (riicp->ICSR2.BIT.RDRF == 0) ;
    if (unit->m_bytesToTransfer > 2) {
        if (unit->m_bytesToTransfer == 3)
            riicp->ICMR3.BIT.WAIT = 1;
        g_RX63N_I2C_Driver.ReadFromSubordinate(ch, unit);
    } else if (unit->m_bytesToTransfer == 2) {
        riicp->ICMR3.BIT.ACKBT = 1;
        g_RX63N_I2C_Driver.ReadFromSubordinate(ch, unit);
    } else {
        g_RX63N_I2C_Driver.ReadLastByte(ch);
    }
}

void RX63N_I2C_ICTXI_ISR(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];
    I2C_HAL_XACTION_UNIT *unit = g_RX63N_I2C_Driver.m_currentXActionUnit;
    GLOBAL_LOCK(irq);
#ifdef DEBUG_I2C_INT
    debug_printf("I2C TXI\r\n");
#endif
    if (unit->m_bytesToTransfer) {
        g_RX63N_I2C_Driver.WriteToSubordinate(ch, unit);
    } else {
        riicp->ICIER.BIT.TIE = 0;
    }
}

void RX63N_I2C_ICTEI_ISR(UINT32 ch)
{
    riic_t *riicp = RIIC[ch];
    I2C_HAL_XACTION *xAction = g_RX63N_I2C_Driver.m_currentXAction;
    I2C_HAL_XACTION_UNIT *unit = g_RX63N_I2C_Driver.m_currentXActionUnit;
#ifdef DEBUG_I2C_INT
    debug_printf("I2C TEI\r\n");
#endif
    riicp->ICIER.BIT.TEIE = 0;
    if (xAction->ProcessingLastUnit()) {
        g_RX63N_I2C_Driver.StopCondition(ch);
        xAction->Signal(I2C_HAL_XACTION::c_Status_Completed );
    } else {
#if (IIC_READ_WRITE_DELAY == 1)
    HAL_Time_Sleep_MicroSeconds_InterruptEnabled(500);
#endif
        g_RX63N_I2C_Driver.MasterXAction_Start(ch, xAction, true);
    }
}

void  RX63N_I2C_Driver::GetPins(UINT32 ch, GPIO_PIN& scl, GPIO_PIN& sda)
{
    NATIVE_PROFILE_HAL_PROCESSOR_I2C();
    scl = SCL_PIN[ch];
    sda = SDA_PIN[ch];
}

#ifdef __cplusplus
extern "C" {
#endif
void __attribute__ ((interrupt)) INT_Excep_RIIC0_EEI0(void) { RX63N_I2C_ICEEI_ISR(0); }
void __attribute__ ((interrupt)) INT_Excep_RIIC0_RXI0(void) { RX63N_I2C_ICRXI_ISR(0); }
void __attribute__ ((interrupt)) INT_Excep_RIIC0_TXI0(void) { RX63N_I2C_ICTXI_ISR(0); }
void __attribute__ ((interrupt)) INT_Excep_RIIC0_TEI0(void) { RX63N_I2C_ICTEI_ISR(0); }
void __attribute__ ((interrupt)) INT_Excep_RIIC1_EEI1(void) { RX63N_I2C_ICEEI_ISR(1); }
void __attribute__ ((interrupt)) INT_Excep_RIIC1_RXI1(void) { RX63N_I2C_ICRXI_ISR(1); }
void __attribute__ ((interrupt)) INT_Excep_RIIC1_TXI1(void) { RX63N_I2C_ICTXI_ISR(1); }
void __attribute__ ((interrupt)) INT_Excep_RIIC1_TEI1(void) { RX63N_I2C_ICTEI_ISR(1); }
void __attribute__ ((interrupt)) INT_Excep_RIIC2_EEI2(void) { RX63N_I2C_ICEEI_ISR(2); }
void __attribute__ ((interrupt)) INT_Excep_RIIC2_RXI2(void) { RX63N_I2C_ICRXI_ISR(2); }
void __attribute__ ((interrupt)) INT_Excep_RIIC2_TXI2(void) { RX63N_I2C_ICTXI_ISR(2); }
void __attribute__ ((interrupt)) INT_Excep_RIIC2_TEI2(void) { RX63N_I2C_ICTEI_ISR(2); }
void __attribute__ ((interrupt)) INT_Excep_RIIC3_EEI3(void) { RX63N_I2C_ICEEI_ISR(3); }
void __attribute__ ((interrupt)) INT_Excep_RIIC3_RXI3(void) { RX63N_I2C_ICRXI_ISR(3); }
void __attribute__ ((interrupt)) INT_Excep_RIIC3_TXI3(void) { RX63N_I2C_ICTXI_ISR(3); }
void __attribute__ ((interrupt)) INT_Excep_RIIC3_TEI3(void) { RX63N_I2C_ICTEI_ISR(3); }
#ifdef __cplusplus
}
#endif
