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

// For RXMEGA
// SPI SW
// Case 1
// OLED_VCC_ON  CN2-1    P46 AN6
// OLED_RESN    CN3-1    P92
// OLED_CNS     CN3-2    PC1        (PC0 or PC1)
// OLED_SCK     CN2-3    P51
// OLED_SDIN    CN2-2    P50
// Case 2
// OLED_VCC_ON  CN2-1    P46 AN6
// OLED_RESN    CN3-1    P92
// OLED_CNS     CN3-2    PC1        (PC0 or PC1)
// OLED_SCK     CN4-2    PC5
// OLED_SDIN    CN4-4    PC6

// SPI HW
// CLK   PC5
// DI    PC6
// CS    PC1

#define VCC_ON  (PORT4.DR.BIT.B6 = 1)
#define VCC_OFF (PORT4.DR.BIT.B6 = 0)
#define RES_ON  (PORT9.DR.BIT.B2 = 0)
#define RES_OFF (PORT9.DR.BIT.B2 = 1)
#define CS_LOW  (PORTC.DR.BIT.B1 = 0)
#define CS_HIGH (PORTC.DR.BIT.B1 = 1)
#define CK_LOW  (PORT5.DR.BIT.B1 = 0)
#define    CK_HIGH    (PORT5.DR.BIT.B1 = 1)
#define    DO_LOW    (PORT5.DR.BIT.B0 = 0)
#define    DO_HIGH    (PORT5.DR.BIT.B0 = 1)
#define    CLK_OUT    (CLK_LOW;CLK_LOW;CLK_HIGH;CLK_HIGH;)

static void wait(volatile UINT32 n)
{
    while (n-- > 0) ;
}

void SPISW_Initialize(void)
{
    // ToDo change default pins
#if 0
    PORT5.DDR.BIT.B0 = 1;
    PORT5.DDR.BIT.B1 = 1;
    PORTC.DDR.BIT.B1 = 1;
    PORT4.DDR.BIT.B6 = 1;   // P46 output
    PORT9.DDR.BIT.B2 = 1;   // P92 output
#endif

    VCC_OFF;
    RES_ON;
    wait(100000);
    RES_OFF;
    CS_LOW;
    DO_LOW;
    CK_HIGH;
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
    VCC_ON;
}
