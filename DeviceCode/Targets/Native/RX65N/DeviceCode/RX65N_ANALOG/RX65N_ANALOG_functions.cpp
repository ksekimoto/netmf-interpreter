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
#include "..\RX65N.h"

static UINT8 adc120_pin[] = {
    RX65N_GPIO::P40,    /* AN000 */
    RX65N_GPIO::P41,    /* AN001 */
    RX65N_GPIO::P42,    /* AN002 */
    RX65N_GPIO::P43,    /* AN003 */
    RX65N_GPIO::P44,    /* AN004 */
    RX65N_GPIO::P45,    /* AN005 */
    RX65N_GPIO::P46,    /* AN006 */
    RX65N_GPIO::P47,    /* AN007 */
};
#define ADC120_SIZE  (sizeof(adc120_pin)/sizeof(UINT8))

static UINT8 adc121_pin[] = {
    RX65N_GPIO::PE2,    /* AN100 */
    RX65N_GPIO::PE3,    /* AN101 */
    RX65N_GPIO::PE4,    /* AN102 */
    RX65N_GPIO::PE5,    /* AN103 */
    RX65N_GPIO::PE6,    /* AN104 */
    RX65N_GPIO::PE7,    /* AN005 */
    RX65N_GPIO::PD6,    /* AN106 */
    RX65N_GPIO::PD7,    /* AN107 */
    RX65N_GPIO::PD0,    /* AN108 */
    RX65N_GPIO::PD1,    /* AN109 */
    RX65N_GPIO::PD2,    /* AN110 */
    RX65N_GPIO::PD3,    /* AN111 */
    RX65N_GPIO::PD4,    /* AN112 */
    RX65N_GPIO::PD5,    /* AN113 */
    RX65N_GPIO::P90,    /* AN114 */
    RX65N_GPIO::P91,    /* AN115 */
    RX65N_GPIO::P92,    /* AN116 */
    RX65N_GPIO::P93,    /* AN117 */
    RX65N_GPIO::P00,    /* AN118 */
    RX65N_GPIO::P01,    /* AN119 */
    RX65N_GPIO::P02,    /* AN120 */
};
#define ADC121_SIZE  (sizeof(adc121_pin)/sizeof(UINT8))

static UINT8 ch_pin[] = {
#if defined(PLATFORM_RX65N_GR_ROSE)
    RX65N_GPIO::PD2,    /* AN110 */
    RX65N_GPIO::PD3,    /* AN111 */
    RX65N_GPIO::PD4,    /* AN112 */
    RX65N_GPIO::PD5,    /* AN113 */
    RX65N_GPIO::PD6,    /* AN106 */
    RX65N_GPIO::PD7,    /* AN107 */
#else
#error "ADC pins are not defined. Please define ADC pins for target platform!"
#endif
};
#define ADC_CH_SIZE  (sizeof(ch_pin)/sizeof(UINT8))

bool rx_adc_chk_ad120(UINT8 pin) {
    int i;
    for (i = 0; i < ADC120_SIZE; i++) {
        if (adc120_pin[i] == pin) {
            return true;
        }
    }
    return false;
}

bool rx_adc_chk_ad121(UINT8 pin) {
    int i;
    for (i = 0; i < ADC121_SIZE; i++) {
        if (adc121_pin[i] == pin) {
            return true;
        }
    }
    return false;
}

INT32 rx_adc_get_resolution(UINT8 pin) {
    int i;
    int res = -1;
#if defined(RX_ADC10)
    for (i = 0; i < ADC10_SIZE; i++) {
        if (adc10_pin[i] == pin) {
            return 10;
        }
    }
#endif
    if (rx_adc_chk_ad120(pin) | rx_adc_chk_ad121(pin)) {
        return 12;
    }
    return res;
}

INT32 rx_adc_get_channel(UINT8 pin) {
    int i;
    int ch = -1;
#if defined(RX_ADC10)
    for (i = 0; i < ADC10_SIZE; i++) {
        if (adc10_pin[i] == pin) {
            return i;
        }
    }
#endif
    for (i = 0; i < ADC120_SIZE; i++) {
        if (adc120_pin[i] == pin) {
            return i;
        }
    }
    for (i = 0; i < ADC121_SIZE; i++) {
        if (adc121_pin[i] == pin) {
            return i;
        }
    }
    return ch;
}

#if defined(RX_ADC10)
void rx_adc10_enable(UINT8 pin) {
    UINT32 port = GPIO_PORT(pin);
    UINT8 mask = GPIO_MASK(pin);
    SYSTEM.PRCR.WORD = 0xA502;
    /* 10 bit AD start */
    SYSTEM.MSTPCRA.BIT.MSTPA23 = 0;
    SYSTEM.PRCR.WORD = 0xA500;
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    /* Enable write to PFS */
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin & 7) |= 0x80;
    _PMR(port) |= mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
    // software trigger, PCLK, single mode
    AD.ADCR.BYTE = 0x0C;
    // LSB alignment
    AD.ADCR2.BIT.DPSEL = 0;
}
#endif

void rx_adc12_enable(UINT8 pin) {
    UINT32 port = GPIO_PORT(pin);
    UINT8 mask = GPIO_MASK(pin);
    SYSTEM.PRCR.WORD = 0xA502;
    /* 12 bit AD start */
    if (rx_adc_chk_ad120(pin)) {
        if (SYSTEM.MSTPCRA.BIT.MSTPA17 == 1) {
            SYSTEM.MSTPCRA.BIT.MSTPA17 = 0;
        }
    } else {
        if (SYSTEM.MSTPCRA.BIT.MSTPA16 == 1) {
            SYSTEM.MSTPCRA.BIT.MSTPA16 = 0;
        }
    }
    SYSTEM.PRCR.WORD = 0xA500;
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin & 7) |= 0x80;
    _PMR(port) |= mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
#if defined(RX63N)
    S12AD.ADCSR.BYTE = 0x0;
    S12AD.ADCER.BIT.ADRFMT = 0;
#endif
#if defined(RX65N)
    if (rx_adc_chk_ad120(pin)) {
        S12AD.ADCSR.WORD = 0x0;
        S12AD.ADCER.BIT.ADRFMT = 0;
    } else {
        S12AD1.ADCSR.WORD = 0x0;
        S12AD1.ADCER.BIT.ADRFMT = 0;
    }
#endif
}

bool rx_adc_enable(UINT8 pin) {
    if (rx_adc_get_channel(pin) == -1) {
        return false;
    }
#if defined(RX_ADC10)
    int resolution;
    resolution = rx_adc_get_resolution(pin);
    if (resolution == 10) {
        rx_adc10_enable(pin);
    } else {
        rx_adc12_enable(pin);
    }
#else
    rx_adc12_enable(pin);
#endif
    return true;
}

#if defined(RX_ADC10)
void rx_adc10_disable(UINT8 pin) {
    UINT32 port = GPIO_PORT(pin);
    UINT8 mask = GPIO_MASK(pin);
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    /* Enable write to PFS */
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin & 7) &= ~0x80;
    _PMR(port) &= ~mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
}
#endif

void rx_adc12_disable(UINT8 pin) {
    UINT32 port = GPIO_PORT(pin);
    UINT8 mask = GPIO_MASK(pin);
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    /* Enable write to PFS */
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin & 7) &= ~0x80;
    _PMR(port) &= ~mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
}

bool rx_adc_disable(UINT8 pin) {
    if (rx_adc_get_channel(pin) == -1) {
        return false;
    }
#if defined(RX_ADC10)
    int resolution;
    resolution = rx_adc_get_resolution(pin);
    if (resolution == 10) {
        rx_adc10_disable(pin);
    } else {
        rx_adc12_disable(pin);
    }
#else
    rx_adc12_disable(pin);
#endif
    return true;
}

#if defined(RX_ADC10)
UINT16 rx_adc10_read(UINT8 pin) {
    UINT16 value16 = 0;
    UINT8 off = (UINT8)rx_adc_get_channel(pin);
    AD.ADCSR.BYTE = 0x20 | off;
    while (AD.ADCSR.BIT.ADST) {
        ;
    }
    value16 = *((UINT16 *)&AD.ADDRA + off);
    return (UINT16)value16;
}
#endif

UINT16 rx_adc12_read(UINT8 pin) {
    UINT16 value16 = 0;
    UINT16 off = (UINT8)rx_adc_get_channel(pin);
#if defined(RX63N)
    S12AD.ADANS0.WORD |= (1 << off);
    S12AD.ADCSR.BIT.ADST = 1;
    while (S12AD.ADCSR.BIT.ADST) {
        ;
    }
    value16 = *((unsigned short*)&S12AD.ADDR0 + off);
#endif
#if defined(RX65N)
    if (rx_adc_chk_ad120(pin)) {
        S12AD.ADANSA0.WORD |= (1 << off);
        S12AD.ADCSR.BIT.ADST = 1;
        while (S12AD.ADCSR.BIT.ADST) {
            ;
        }
        value16 = *((unsigned short*)&S12AD.ADDR0 + off);
    } else {
        S12AD1.ADANSA0.WORD |= (1 << off);
        S12AD1.ADCSR.BIT.ADST = 1;
        while (S12AD1.ADCSR.BIT.ADST) {
            ;
        }
        value16 = *((unsigned short*)&S12AD1.ADDR0 + off);
    }
#endif
    return (UINT16)value16;
}

UINT16 rx_adc_read(UINT8 pin) {
    UINT16 value16;
#if defined(RX_ADC10)
    int resolution = rx_adc_get_resolution(pin);
    if (resolution == 10) {
        value16 = rx_adc10_read(pin);
    } else {
        value16 = rx_adc12_read(pin);
    }
#else
    value16 = rx_adc12_read(pin);
#endif
    return value16;
}

BOOL AD_Initialize(ANALOG_CHANNEL channel, INT32 precisionInBits)
{
    if ((UINT32)channel >= ADC_CH_SIZE)
        return FALSE;
    rx_adc12_enable(ch_pin[channel]);
    return TRUE;
}

void AD_Uninitialize(ANALOG_CHANNEL channel)
{
    if ((UINT32)channel >= ADC_CH_SIZE)
        return FALSE;
    rx_adc12_disable(ch_pin[channel]);
}

INT32 AD_Read(ANALOG_CHANNEL channel)
{
    INT32 value32 = 0;
    if ((UINT32)channel >= ADC_CH_SIZE)
        return FALSE;
    value32 = (INT32)rx_adc_read(ch_pin[channel]);
    return value32;
}

UINT32 AD_ADChannels()
{
    return ADC_CH_SIZE;
}

GPIO_PIN AD_GetPinForChannel(ANALOG_CHANNEL channel)
{
    if ((UINT32)channel >= ADC_CH_SIZE)
        return GPIO_PIN_NONE;
    return ch_pin[channel];
}

BOOL AD_GetAvailablePrecisionsForChannel(ANALOG_CHANNEL channel, INT32* precisions, UINT32& size)
{
    size = 0;
    if (precisions == NULL || (UINT32)channel >= ADC_CH_SIZE)
        return FALSE;
    precisions[0] = 12;
    size = 1;
    return TRUE;
}
