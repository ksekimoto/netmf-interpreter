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
//  Copyright (c) Kentaro Sekimoto All rights reserved.
//
////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include "..\..\..\..\DeviceCode\Targets\Native\RX63N\DeviceCode\RX63N.h"

// SPI HW
// CLK  PC5 - Pin13
// DI   PC6 - Pin11
// CS   PC4 - Pin10
// RES  P50 - Pin5
// VCC  P52 - Pin6

//#define VCC_ON  (PORT5.PODR.BIT.B2 = 1)
//#define VCC_OFF (PORT5.PODR.BIT.B2 = 0)
#define RES_ON  (PORT5.PODR.BIT.B0 = 0)
#define RES_OFF (PORT5.PODR.BIT.B0 = 1)
#define CS_LOW  (PORTC.PODR.BIT.B4 = 0)
#define CS_HIGH (PORTC.PODR.BIT.B4 = 1)
#define CK_LOW  (PORTC.PODR.BIT.B5 = 0)
#define CK_HIGH (PORTC.PODR.BIT.B5 = 1)
#define DO_LOW  (PORTC.PODR.BIT.B6 = 0)
#define DO_HIGH (PORTC.PODR.BIT.B6 = 1)
#define CLK_OUT (CLK_LOW;CLK_LOW;CLK_HIGH;CLK_HIGH;)

static void wait(volatile UINT32 n)
{
    while (n-- > 0) ;
}

void SPISW_Initialize(void)
{
    PORT5.PMR.BIT.B0 = 0;
    //PORT5.PMR.BIT.B2 = 0;
    PORTC.PMR.BIT.B4 = 0;
    PORTC.PMR.BIT.B5 = 0;
    PORTC.PMR.BIT.B6 = 0;

    PORT5.PDR.BIT.B0 = 1;
    //PORT5.PDR.BIT.B2 = 1;
    PORTC.PDR.BIT.B4 = 1;
    PORTC.PDR.BIT.B5 = 1;
    PORTC.PDR.BIT.B6 = 1;

    //VCC_OFF;
    //RES_ON;
    //wait(100000);
    //RES_OFF;
    //CS_LOW;
    //DO_LOW;
    //CK_HIGH;
}

void SPISW_Reset(UINT32 v)
{
    if (v) {
        RES_OFF;
    } else {
        RES_ON;
    }
}

void SPISW_Write(UINT8 dat)
{
    (dat & 0x80) ? DO_HIGH : DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    (dat & 0x40) ? DO_HIGH : DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    (dat & 0x20) ? DO_HIGH : DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    (dat & 0x10) ? DO_HIGH : DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    (dat & 0x08) ? DO_HIGH : DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    (dat & 0x04) ? DO_HIGH : DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    (dat & 0x02) ? DO_HIGH : DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    (dat & 0x01) ? DO_HIGH : DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
}

void SPISW_LCD_cmd(UINT8 dat)
{
    // Enter command mode: SDATA=LOW at rising edge of 1st SCLK
    CS_LOW;
    DO_LOW;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    SPISW_Write(dat);
    CS_HIGH;
}
void SPISW_LCD_dat(UINT8 dat)
{
    // Enter data mode: SDATA=HIGH at rising edge of 1st SCLK
    CS_LOW;
    DO_HIGH;
    CK_LOW; CK_LOW; CK_HIGH; CK_HIGH;
    SPISW_Write(dat);
    CS_HIGH;
}
void SPISW_LCD_on(void)
{
    //VCC_ON;
}
