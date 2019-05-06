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
#include "..\iodefine.h"
#include "..\RX65N.h"
#include "..\RX65N_USART\RX65N_USART.h"

//--//

void IRQ_Handler()
{
    // ToDo
}

void CPU_INTC_Initialize()
{
    // ToDo
}

BOOL CPU_INTC_ActivateInterrupt(UINT32 Irq_Index, HAL_CALLBACK_FPN ISR, void* ISR_Param)
{
    // ToDo
    return FALSE;
}

BOOL CPU_INTC_DeactivateInterrupt(UINT32 Irq_Index)
{
    // ToDo
    return FALSE;
}

BOOL CPU_INTC_InterruptEnable(UINT32 Irq_Index)
{
    // ToDo
    return FALSE;
}

BOOL CPU_INTC_InterruptDisable(UINT32 Irq_Index)
{
    // ToDo
    return FALSE;
}

BOOL CPU_INTC_InterruptEnableState(UINT32 Irq_Index)
{
    // ToDo
    return FALSE;
}

BOOL CPU_INTC_InterruptState(UINT32 Irq_Index)
{
    // ToDo
    return FALSE;
}

//extern void sci_isr_te(int ch);
//extern void sci_isr_er(int ch);

void set_int_state_groupbl0(int flag) {
    IEN(ICU, GROUPBL0) = 0;
    IR(ICU, GROUPBL0)  = 0;
    IEN(ICU, GROUPBL0) = flag;
}

void set_int_state_groupbl1(int flag) {
    IEN(ICU, GROUPBL1) = 0;
    IR(ICU, GROUPBL1)  = 0;
    IEN(ICU, GROUPBL1) = flag;
}

void set_int_state_groupal0(int flag) {
    IEN(ICU, GROUPAL0) = 0;
    IR(ICU, GROUPAL0)  = 0;
    IEN(ICU, GROUPAL0) = flag;
}

void set_int_priority_groupbl0(int ipl) {
    int flag = IEN(ICU, GROUPBL0);
    IEN(ICU, GROUPBL0) = 0;
    IR(ICU, GROUPBL0)  = 0;
    IPR(ICU, GROUPBL0) = (UINT8)(ipl > IPR(ICU, GROUPBL0) ? ipl : IPR(ICU, GROUPBL0));
    IEN(ICU, GROUPBL0) = flag;
}

void set_int_priority_groupbl1(int ipl) {
    int flag = IEN(ICU, GROUPBL1);
    IEN(ICU, GROUPBL1) = 0;
    IR(ICU, GROUPBL1)  = 0;
    IPR(ICU, GROUPBL1) = (UINT8)(ipl > IPR(ICU, GROUPBL1) ? ipl : IPR(ICU, GROUPBL1));
    IEN(ICU, GROUPBL1) = flag;
}

void set_int_priority_groupal0(int ipl) {
    int flag = IEN(ICU, GROUPAL0);
    IEN(ICU, GROUPAL0) = 0;
    IR(ICU, GROUPAL0)  = 0;
    IPR(ICU, GROUPAL0) = (UINT8)(ipl > IPR(ICU, GROUPAL0) ? ipl : IPR(ICU, GROUPAL0));
    IEN(ICU, GROUPAL0) = flag;
}

#if 0
// ICU GROUPBL0
// vec: 110
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPBL0(void) {
    if (1 == ICU.GRPBL0.BIT.IS0) {
        sci_isr_te(0);
    }
    if (1 == ICU.GRPBL0.BIT.IS2) {
        sci_isr_te(1);
    }
    if (1 == ICU.GRPBL0.BIT.IS4) {
        sci_isr_te(2);
    }
    if (1 == ICU.GRPBL0.BIT.IS6) {
        sci_isr_te(3);
    }
    if (1 == ICU.GRPBL0.BIT.IS8) {
        sci_isr_te(4);
    }
    if (1 == ICU.GRPBL0.BIT.IS10) {
        sci_isr_te(5);
    }
    if (1 == ICU.GRPBL0.BIT.IS12) {
        sci_isr_te(6);
    }
    if (1 == ICU.GRPBL0.BIT.IS14) {
        sci_isr_te(7);
    }
    if (1 == ICU.GRPBL0.BIT.IS16) {
        sci_isr_te(12);
    }
    if (1 == ICU.GRPBL0.BIT.IS1) {
        sci_isr_er(0);
    }
    if (1 == ICU.GRPBL0.BIT.IS3) {
        sci_isr_er(1);
    }
    if (1 == ICU.GRPBL0.BIT.IS5) {
        sci_isr_er(2);
    }
    if (1 == ICU.GRPBL0.BIT.IS7) {
        sci_isr_er(3);
    }
    if (1 == ICU.GRPBL0.BIT.IS9) {
        sci_isr_er(4);
    }
    if (1 == ICU.GRPBL0.BIT.IS11) {
        sci_isr_er(5);
    }
    if (1 == ICU.GRPBL0.BIT.IS13) {
        sci_isr_er(6);
    }
    if (1 == ICU.GRPBL0.BIT.IS15) {
        sci_isr_er(7);
    }
    if (1 == ICU.GRPBL0.BIT.IS17) {
        sci_isr_er(12);
    }
}

// ICU GROUPBL1
// vec: 111
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPBL1(void) {
    if (1 == ICU.GRPBL1.BIT.IS24) {
        sci_isr_te(8);
    }
    if (1 == ICU.GRPBL1.BIT.IS26) {
        sci_isr_te(9);
    }
    if (1 == ICU.GRPBL1.BIT.IS25) {
        sci_isr_er(8);
    }
    if (1 == ICU.GRPBL1.BIT.IS27) {
        sci_isr_er(9);
    }
}

// ICU GROUPAL0
// vec: 112
void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPAL0(void) {
    if (1 == ICU.GRPAL0.BIT.IS8) {
        sci_isr_te(10);
    }
    if (1 == ICU.GRPAL0.BIT.IS12) {
        sci_isr_te(11);
    }
    if (1 == ICU.GRPAL0.BIT.IS9) {
        sci_isr_er(10);
    }
    if (1 == ICU.GRPAL0.BIT.IS13) {
        sci_isr_er(11);
    }
}

// ICU GROUPAL1
// vec: 113
//void __attribute__ ((interrupt)) INT_Excep_ICU_GROUPAL1(void) {
//#if MICROPY_HW_ETH_RX && MICROPY_PY_LWIP
//    if (1 == ICU.GRPAL1.BIT.IS4) {
//        rx_ether_input_callback();
//    }
//#endif // MICROPY_HW_ETH_RX && MICROPY_PY_LWIP
//}

//void __attribute__ ((interrupt)) INT_Excep_PERIB_INTB110(void)
//{
//    USBHALInterruptHandler();
//    ICU.PIBR7.BYTE |= 0x40;
//}
#endif
