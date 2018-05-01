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

#ifndef INTERRUPT_HANDLERS_H
#define INTERRUPT_HANDLERS_H

// Exception(Supervisor Instruction)
void INT_Excep_SuperVisorInst(void) __attribute__ ((interrupt));;

// Exception(Access Instruction)

void INT_Excep_AccessInst(void) __attribute__ ((interrupt));;

// Exception(Undefined Instruction)
void INT_Excep_UndefinedInst(void) __attribute__ ((interrupt));;

// Exception(Floating Point)
void INT_Excep_FloatingPoint(void) __attribute__ ((interrupt));;

// NMI
void NonMaskableInterrupt(void) __attribute__ ((interrupt));;

// Dummy
void Dummy(void) __attribute__ ((interrupt));;

// BRK
void INT_Excep_BRK(void) __attribute__ ((interrupt));;

// vector  1 reserved
// vector  2 reserved
// vector  3 reserved
// vector  4 reserved
// vector  5 reserved
// vector  6 reserved
// vector  7 reserved
// vector  8 reserved
// vector  9 reserved
// vector 10 reserved
// vector 11 reserved
// vector 12 reserved
// vector 13 reserved
// vector 14 reserved
// vector 15 reserved

// BSC BUSERR
void INT_Excep_BSC_BUSERR(void) __attribute__ ((interrupt));;

// vector 17 reserved
// vector 18 reserved
// vector 19 reserved
// vector 20 reserved

// FCU_FIFERR
void INT_Excep_FCU_FIFERR(void) __attribute__ ((interrupt));;

// vector 22 reserved

// FCU_FRDYI
void INT_Excep_FCU_FRDYI(void) __attribute__ ((interrupt));;

// vector 24 reserved
// vector 25 reserved
// vector 26 reserved

// ICU SWINT
void INT_Excep_ICU_SWINT(void) __attribute__ ((interrupt));;

// CMT0_CMI0
void INT_Excep_CMT0_CMI0(void) __attribute__ ((interrupt));;

// CMT1_CMI1
void INT_Excep_CMT1_CMI1(void) __attribute__ ((interrupt));;

// CMT2_CMI2
void INT_Excep_CMT2_CMI2(void) __attribute__ ((interrupt));;

// CMT3_CMI3
void INT_Excep_CMT3_CMI3(void) __attribute__ ((interrupt));;

// ETHER EINT
void INT_Excep_ETHER_EINT(void) __attribute__ ((interrupt));;

// USB0 D0FIFO0
void INT_Excep_USB0_D0FIFO0(void) __attribute__ ((interrupt));;

// USB0 D1FIFO0
void INT_Excep_USB0_D1FIFO0(void) __attribute__ ((interrupt));;

// USB0 USBI0
void INT_Excep_USB0_USBI0(void) __attribute__ ((interrupt));;

// USB1 D0FIFO1
void INT_Excep_USB1_D0FIFO1(void) __attribute__ ((interrupt));;

// USB1 D1FIFO1
void INT_Excep_USB1_D1FIFO1(void) __attribute__ ((interrupt));;

// USB1 USBI1
void INT_Excep_USB1_USBI1(void) __attribute__ ((interrupt));;

// RSPI0 SPRI0
void INT_Excep_RSPI0_SPRI0(void) __attribute__ ((interrupt));;

// RSPI0 SPTI0
void INT_Excep_RSPI0_SPTI0(void) __attribute__ ((interrupt));;

// RSPI0 SPII0
void INT_Excep_RSPI0_SPII0(void) __attribute__ ((interrupt));;

// RSPI1 SPRI1
void INT_Excep_RSPI1_SPRI1(void) __attribute__ ((interrupt));;

// RSPI1 SPTI1
void INT_Excep_RSPI1_SPTI1(void) __attribute__ ((interrupt));;

// RSPI1 SPII1
void INT_Excep_RSPI1_SPII1(void) __attribute__ ((interrupt));;

// RSPI2 SPRI2
void INT_Excep_RSPI2_SPRI2(void) __attribute__ ((interrupt));;

// RSPI2 SPTI2
void INT_Excep_RSPI2_SPTI2(void) __attribute__ ((interrupt));;

// RSPI2 SPII2
void INT_Excep_RSPI2_SPII2(void) __attribute__ ((interrupt));;

// CAN0 RXF0
void INT_Excep_CAN0_RXF0(void) __attribute__ ((interrupt));;

// CAN0 TXF0
void INT_Excep_CAN0_TXF0(void) __attribute__ ((interrupt));;

// CAN0 RXM0
void INT_Excep_CAN0_RXM0(void) __attribute__ ((interrupt));;

// CAN0 TXM0
void INT_Excep_CAN0_TXM0(void) __attribute__ ((interrupt));;

// CAN1 RXF1
void INT_Excep_CAN1_RXF1(void) __attribute__ ((interrupt));;

// CAN1 TXF1
void INT_Excep_CAN1_TXF1(void) __attribute__ ((interrupt));;

// CAN1 RXM1
void INT_Excep_CAN1_RXM1(void) __attribute__ ((interrupt));;

// CAN1 TXM1
void INT_Excep_CAN1_TXM1(void) __attribute__ ((interrupt));;

// CAN2 RXF2
void INT_Excep_CAN2_RXF2(void) __attribute__ ((interrupt));;

// CAN2 TXF2
void INT_Excep_CAN2_TXF2(void) __attribute__ ((interrupt));;

// CAN2 RXM2
void INT_Excep_CAN2_RXM2(void) __attribute__ ((interrupt));;

// CAN2 TXM2
void INT_Excep_CAN2_TXM2(void) __attribute__ ((interrupt));;

// vector 60 reserved
// vector 61 reserved

// RTC CUP
void INT_Excep_RTC_CUP(void) __attribute__ ((interrupt));;

// vector 63 reserved

// ICU IRQ0
void INT_Excep_ICU_IRQ0(void) __attribute__ ((interrupt));;

// ICU IRQ1
void INT_Excep_ICU_IRQ1(void) __attribute__ ((interrupt));;

// ICU IRQ2
void INT_Excep_ICU_IRQ2(void) __attribute__ ((interrupt));;

// ICU IRQ3
void INT_Excep_ICU_IRQ3(void) __attribute__ ((interrupt));;

// ICU IRQ4
void INT_Excep_ICU_IRQ4(void) __attribute__ ((interrupt));;

// ICU IRQ5
void INT_Excep_ICU_IRQ5(void) __attribute__ ((interrupt));;

// ICU IRQ6
void INT_Excep_ICU_IRQ6(void) __attribute__ ((interrupt));;

// ICU IRQ7
void INT_Excep_ICU_IRQ7(void) __attribute__ ((interrupt));;

// ICU IRQ8
void INT_Excep_ICU_IRQ8(void) __attribute__ ((interrupt));;

// ICU IRQ9
void INT_Excep_ICU_IRQ9(void) __attribute__ ((interrupt));;

// ICU IRQ10
void INT_Excep_ICU_IRQ10(void) __attribute__ ((interrupt));;

// ICU IRQ11
void INT_Excep_ICU_IRQ11(void) __attribute__ ((interrupt));;

// ICU IRQ12
void INT_Excep_ICU_IRQ12(void) __attribute__ ((interrupt));;

// ICU IRQ13
void INT_Excep_ICU_IRQ13(void) __attribute__ ((interrupt));;

// ICU IRQ14
void INT_Excep_ICU_IRQ14(void) __attribute__ ((interrupt));;

// ICU IRQ15
void INT_Excep_ICU_IRQ15(void) __attribute__ ((interrupt));;

// vector 80 reserved
// vector 81 reserved
// vector 82 reserved
// vector 83 reserved
// vector 84 reserved
// vector 85 reserved
// vector 86 reserved
// vector 87 reserved
// vector 88 reserved
// vector 89 reserved

// USB USBR0
void INT_Excep_USB_USBR0(void) __attribute__ ((interrupt));;

// USB USBR1
void INT_Excep_USB_USBR1(void) __attribute__ ((interrupt));;

// RTC ALM
void INT_Excep_RTC_ALM(void) __attribute__ ((interrupt));;

// RTC PRD
void INT_Excep_RTC_PRD(void) __attribute__ ((interrupt));;

// vector 94 reserved
// vector 95 reserved
// vector 96 reserved
// vector 97 reserved

// AD ADI0
void INT_Excep_AD_ADI0(void) __attribute__ ((interrupt));;

// vector 99 reserved
// vector 100 reserved
// vector 101 reserved

// S12AD S12ADI0
void INT_Excep_S12AD_S12ADI0(void) __attribute__ ((interrupt));;

// vector 103 reserved
// vector 104 reserved
// vector 105 reserved

// ICU GROUP0
void INT_Excep_ICU_GROUP0(void) __attribute__ ((interrupt));;

// ICU GROUP1
void INT_Excep_ICU_GROUP1(void) __attribute__ ((interrupt));;

// ICU GROUP2
void INT_Excep_ICU_GROUP2(void) __attribute__ ((interrupt));;

// ICU GROUP3
void INT_Excep_ICU_GROUP3(void) __attribute__ ((interrupt));;

// ICU GROUP4
void INT_Excep_ICU_GROUP4(void) __attribute__ ((interrupt));;

// ICU GROUP5
void INT_Excep_ICU_GROUP5(void) __attribute__ ((interrupt));;

// ICU GROUP6
void INT_Excep_ICU_GROUP6(void) __attribute__ ((interrupt));;

// vector 113 reserved

// ICU GROUP12
void INT_Excep_ICU_GROUP12(void) __attribute__ ((interrupt));;

// vector 115 reserved
// vector 116 reserved
// vector 117 reserved
// vector 118 reserved
// vector 119 reserved
// vector 120 reserved
// vector 121 reserved

// SCI12_SCIX0
void INT_Excep_SCI2_SCIX0(void) __attribute__ ((interrupt));;

// SCI12_SCIX1
void INT_Excep_SCI2_SCIX1(void) __attribute__ ((interrupt));;

// SCI12_SCIX2
void INT_Excep_SCI2_SCIX2(void) __attribute__ ((interrupt));;

// SCI12_SCIX3
void INT_Excep_SCI2_SCIX3(void) __attribute__ ((interrupt));;

// TPU0 TGI0A
void INT_Excep_TPU0_TGI0A(void) __attribute__ ((interrupt));;

// TPU0 TGI0B
void INT_Excep_TPU0_TGI0B(void) __attribute__ ((interrupt));;

// TPU0 TGI0C
void INT_Excep_TPU0_TGI0C(void) __attribute__ ((interrupt));;

// TPU0 TGI0D
void INT_Excep_TPU0_TGI0D(void) __attribute__ ((interrupt));;

// TPU1 TGI1A
void INT_Excep_TPU1_TGI1A(void) __attribute__ ((interrupt));;

// TPU1 TGI1B
void INT_Excep_TPU1_TGI1B(void) __attribute__ ((interrupt));;

// TPU2 TGI2A
void INT_Excep_TPU2_TGI2A(void) __attribute__ ((interrupt));;

// TPU2 TGI2B
void INT_Excep_TPU2_TGI2B(void) __attribute__ ((interrupt));;

// TPU3 TGI3A
void INT_Excep_TPU3_TGI3A(void) __attribute__ ((interrupt));;

// TPU3 TGI3B
void INT_Excep_TPU3_TGI3B(void) __attribute__ ((interrupt));;

// TPU3 TGI3C
void INT_Excep_TPU3_TGI3C(void) __attribute__ ((interrupt));;

// TPU3 TGI3D
void INT_Excep_TPU3_TGI3D(void) __attribute__ ((interrupt));;

// TPU4 TGI4A
void INT_Excep_TPU4_TGI4A(void) __attribute__ ((interrupt));;

// TPU4 TGI4B
void INT_Excep_TPU4_TGI4B(void) __attribute__ ((interrupt));;

// TPU5 TGI5A
void INT_Excep_TPU5_TGI5A(void) __attribute__ ((interrupt));;

// TPU5 TGI5B
void INT_Excep_TPU5_TGI5B(void) __attribute__ ((interrupt));;

// TPU6 TGI6A
void INT_Excep_TPU6_TGI6A(void) __attribute__ ((interrupt));;

// MTU0 TGIA0
//void INT_Excep_MTU0_TGIA0(void) __attribute__ ((interrupt));;
// TPU6 TGI6B
void INT_Excep_TPU6_TGI6B(void) __attribute__ ((interrupt));;

// MTU0 TGIB0
//void INT_Excep_MTU0_TGIB0(void) __attribute__ ((interrupt));;
// TPU6 TGI6C
void INT_Excep_TPU6_TGI6C(void) __attribute__ ((interrupt));;

// MTU0 TGIC0
//void INT_Excep_MTU0_TGIC0(void) __attribute__ ((interrupt));;
// TPU6 TGI6D
void INT_Excep_TPU6_TGI6D(void) __attribute__ ((interrupt));;

// MTU0 TGID0
//void INT_Excep_MTU0_TGID0(void) __attribute__ ((interrupt));;
// MTU0 TGIE0
void INT_Excep_MTU0_TGIE0(void) __attribute__ ((interrupt));;

// MTU0 TGIF0
void INT_Excep_MTU0_TGIF0(void) __attribute__ ((interrupt));;

// TPU7 TGI7A
void INT_Excep_TPU7_TGI7A(void) __attribute__ ((interrupt));;

// MTU1 TGIA1
//void INT_Excep_MTU1_TGIA1(void)
// TPU7 TGI7B
void INT_Excep_TPU7_TGI7B(void) __attribute__ ((interrupt));;

// MTU1 TGIB1
//void INT_Excep_MTU1_TGIB1(void) __attribute__ ((interrupt));;
// TPU8 TGI8A
void INT_Excep_TPU8_TGI8A(void) __attribute__ ((interrupt));;

// MTU2 TGIA2
//void INT_Excep_MTU2_TGIA2(void);
// TPU8 TGI8B
void INT_Excep_TPU8_TGI8B(void) __attribute__ ((interrupt));;

// MTU2 TGIB2
//void INT_Excep_MTU2_TGIB2(void) __attribute__ ((interrupt));;
// TPU9 TGI9A
void INT_Excep_TPU9_TGI9A(void) __attribute__ ((interrupt));;

// MTU3 TGIA3
//void INT_Excep_MTU3_TGIA3(void) __attribute__ ((interrupt));;
// TPU9 TGI9B
void INT_Excep_TPU9_TGI9B(void) __attribute__ ((interrupt));;

// MTU3 TGIB3
//void INT_Excep_MTU3_TGIB3(void) __attribute__ ((interrupt));;
// TPU9 TGI9C
void INT_Excep_TPU9_TGI9C(void) __attribute__ ((interrupt));;

// MTU3 TGIC3
//void INT_Excep_MTU3_TGIC3(void) __attribute__ ((interrupt));;
// TPU9 TGI9D
void INT_Excep_TPU9_TGI9D(void) __attribute__ ((interrupt));;

// MTU3 TGID3
//void INT_Excep_MTU3_TGID3(void) __attribute__ ((interrupt));;
// TPU10 TGI10A
void INT_Excep_TPU10_TGI10A(void) __attribute__ ((interrupt));;

// MTU4 TGIA4
//void INT_Excep_MTU4_TGIA4(void) __attribute__ ((interrupt));;
// TPU10 TGI10B
void INT_Excep_TPU10_TGI10B(void) __attribute__ ((interrupt));;

// MTU4 TGIB4
//void INT_Excep_MTU4_TGIB4(void) __attribute__ ((interrupt));;
// MTU4 TGIC4
void INT_Excep_MTU4_TGIC4(void) __attribute__ ((interrupt));;

// MTU4 TGID4
void INT_Excep_MTU4_TGID4(void) __attribute__ ((interrupt));;

// MTU4 TCIV4
void INT_Excep_MTU4_TCIV4(void) __attribute__ ((interrupt));;

// MTU5 TGIU5
void INT_Excep_MTU5_TGIU5(void) __attribute__ ((interrupt));;

// MTU5 TGIV5
void INT_Excep_MTU5_TGIV5(void) __attribute__ ((interrupt));;

// MTU5 TGIW5
void INT_Excep_MTU5_TGIW5(void) __attribute__ ((interrupt));;

// TPU11 TGI11A
void INT_Excep_TPU11_TGI11A(void) __attribute__ ((interrupt));;

// TPU11 TGI11B
void INT_Excep_TPU11_TGI11B(void) __attribute__ ((interrupt));;

// POE OEI1
void INT_Excep_POE_OEI1(void) __attribute__ ((interrupt));;

// POE OEI2
void INT_Excep_POE_OEI2(void) __attribute__ ((interrupt));;

// vector 168 reserved
// vector 169 reserved

// TMR0_CMIA0
void INT_Excep_TMR0_CMIA0(void) __attribute__ ((interrupt));;

// TMR0_CMIB0
void INT_Excep_TMR0_CMIB0(void) __attribute__ ((interrupt));;

// TMR0_OVI0
void INT_Excep_TMR0_OVI0(void) __attribute__ ((interrupt));;

// TMR1_CMIA1
void INT_Excep_TMR1_CMIA1(void) __attribute__ ((interrupt));;

// TMR1_CMIB1
void INT_Excep_TMR1_CMIB1(void) __attribute__ ((interrupt));;

// TMR1_OVI1
void INT_Excep_TMR1_OVI1(void) __attribute__ ((interrupt));;

// TMR2_CMIA2
void INT_Excep_TMR2_CMIA2(void) __attribute__ ((interrupt));;

// TMR2_CMIB2
void INT_Excep_TMR2_CMIB2(void) __attribute__ ((interrupt));;

// TMR2_OVI2
void INT_Excep_TMR2_OVI2(void) __attribute__ ((interrupt));;

// TMR3_CMIA3
void INT_Excep_TMR3_CMIA3(void) __attribute__ ((interrupt));;

// TMR3_CMIB3
void INT_Excep_TMR3_CMIB3(void) __attribute__ ((interrupt));;

// TMR3_OVI3
void INT_Excep_TMR3_OVI3(void) __attribute__ ((interrupt));;

// RIIC0 EEI0
void INT_Excep_RIIC0_EEI0(void) __attribute__ ((interrupt));;

// RIIC0 RXI0
void INT_Excep_RIIC0_RXI0(void) __attribute__ ((interrupt));;

// RIIC0 TXI0
void INT_Excep_RIIC0_TXI0(void) __attribute__ ((interrupt));;

// RIIC0 TEI0
void INT_Excep_RIIC0_TEI0(void) __attribute__ ((interrupt));;

// RIIC1 EEI1
void INT_Excep_RIIC1_EEI1(void) __attribute__ ((interrupt));;

// RIIC1 RXI1
void INT_Excep_RIIC1_RXI1(void) __attribute__ ((interrupt));;

// RIIC1 TXI1
void INT_Excep_RIIC1_TXI1(void) __attribute__ ((interrupt));;

// RIIC1 TEI1
void INT_Excep_RIIC1_TEI1(void) __attribute__ ((interrupt));;

// RIIC2 EEI2
void INT_Excep_RIIC2_EEI2(void) __attribute__ ((interrupt));;

// RIIC2 RXI2
void INT_Excep_RIIC2_RXI2(void) __attribute__ ((interrupt));;

// RIIC2 TXI2
void INT_Excep_RIIC2_TXI2(void) __attribute__ ((interrupt));;

// RIIC2 TEI2
void INT_Excep_RIIC2_TEI2(void) __attribute__ ((interrupt));;

// RIIC3 EEI3
void INT_Excep_RIIC3_EEI3(void) __attribute__ ((interrupt));;

// RIIC3 RXI3
void INT_Excep_RIIC3_RXI3(void) __attribute__ ((interrupt));;

// RIIC3 TXI3
void INT_Excep_RIIC3_TXI3(void) __attribute__ ((interrupt));;

// RIIC3 TEI3
void INT_Excep_RIIC3_TEI3(void) __attribute__ ((interrupt));;

// DMAC DMAC0I
void INT_Excep_DMAC_DMAC0I(void) __attribute__ ((interrupt));;

// DMAC DMAC1I
void INT_Excep_DMAC_DMAC1I(void) __attribute__ ((interrupt));;

// DMAC DMAC2I
void INT_Excep_DMAC_DMAC2I(void) __attribute__ ((interrupt));;

// DMAC DMAC3I
void INT_Excep_DMAC_DMAC3I(void) __attribute__ ((interrupt));;

// EXDMAC EXDMAC0I
void INT_Excep_EXDMAC_EXDMAC0I(void) __attribute__ ((interrupt));;

// EXDMAC EXDMAC1I
void INT_Excep_EXDMAC_EXDMAC1I(void) __attribute__ ((interrupt));;

// vector 204 reserved
// vector 205 reserved
// vector 206 reserved
// vector 207 reserved
// vector 208 reserved
// vector 209 reserved
// vector 210 reserved
// vector 211 reserved
// vector 212 reserved
// vector 213 reserved

// SCI0_RXI0
void INT_Excep_SCI0_RXI0(void) __attribute__ ((interrupt));;

// SCI0_TXI0
void INT_Excep_SCI0_TXI0(void) __attribute__ ((interrupt));;

// SCI0_TEI0
void INT_Excep_SCI0_TEI0(void) __attribute__ ((interrupt));;

// SCI1_RXI1
void INT_Excep_SCI1_RXI1(void) __attribute__ ((interrupt));;

// SCI1_TXI1
void INT_Excep_SCI1_TXI1(void) __attribute__ ((interrupt));;

// SCI1_TEI1
void INT_Excep_SCI1_TEI1(void) __attribute__ ((interrupt));;

// SCI2_RXI2
void INT_Excep_SCI2_RXI2(void) __attribute__ ((interrupt));;

// SCI2_TXI2
void INT_Excep_SCI2_TXI2(void) __attribute__ ((interrupt));;

// SCI2_TEI2
void INT_Excep_SCI2_TEI2(void) __attribute__ ((interrupt));;

// SCI3_RXI3
void INT_Excep_SCI3_RXI3(void) __attribute__ ((interrupt));;

// SCI3_TXI3
void INT_Excep_SCI3_TXI3(void) __attribute__ ((interrupt));;

// SCI3_TEI3
void INT_Excep_SCI3_TEI3(void) __attribute__ ((interrupt));;

// SCI4_RXI4
void INT_Excep_SCI4_RXI4(void) __attribute__ ((interrupt));;

// SCI4_TXI4
void INT_Excep_SCI4_TXI4(void) __attribute__ ((interrupt));;

// SCI4_TEI4
void INT_Excep_SCI4_TEI4(void) __attribute__ ((interrupt));;

// SCI5_RXI5
void INT_Excep_SCI5_RXI5(void) __attribute__ ((interrupt));;

// SCI5_TXI5
void INT_Excep_SCI5_TXI5(void) __attribute__ ((interrupt));;

// SCI5_TEI5
void INT_Excep_SCI5_TEI5(void) __attribute__ ((interrupt));;

// SCI6_RXI6
void INT_Excep_SCI6_RXI6(void) __attribute__ ((interrupt));;

// SCI6_TXI6
void INT_Excep_SCI6_TXI6(void) __attribute__ ((interrupt));;

// SCI6_TEI6
void INT_Excep_SCI6_TEI6(void) __attribute__ ((interrupt));;

// SCI7_RXI7
void INT_Excep_SCI7_RXI7(void) __attribute__ ((interrupt));;

// SCI7_TXI7
void INT_Excep_SCI7_TXI7(void) __attribute__ ((interrupt));;

// SCI7_TEI7
void INT_Excep_SCI7_TEI7(void) __attribute__ ((interrupt));;

// SCI8_RXI8
void INT_Excep_SCI8_RXI8(void) __attribute__ ((interrupt));;

// SCI8_TXI8
void INT_Excep_SCI8_TXI8(void) __attribute__ ((interrupt));;

// SCI8_TEI8
void INT_Excep_SCI8_TEI8(void) __attribute__ ((interrupt));;

// SCI9_RXI9
void INT_Excep_SCI9_RXI9(void) __attribute__ ((interrupt));;

// SCI9_TXI9
void INT_Excep_SCI9_TXI9(void) __attribute__ ((interrupt));;

// SCI9_TEI9
void INT_Excep_SCI9_TEI9(void) __attribute__ ((interrupt));;

// SCI10_RXI10
void INT_Excep_SCI10_RXI10(void) __attribute__ ((interrupt));;

// SCI10_TXI10
void INT_Excep_SCI10_TXI10(void) __attribute__ ((interrupt));;

// SCI10_TEI10
void INT_Excep_SCI10_TEI10(void) __attribute__ ((interrupt));;

// SCI11_RXI11
void INT_Excep_SCI11_RXI11(void) __attribute__ ((interrupt));;

// SCI11_TXI11
void INT_Excep_SCI11_TXI11(void) __attribute__ ((interrupt));;

// SCI11_TEI11
void INT_Excep_SCI11_TEI11(void) __attribute__ ((interrupt));;

// SCI12_RXI12
void INT_Excep_SCI12_RXI12(void) __attribute__ ((interrupt));;

// SCI12_TXI12
void INT_Excep_SCI12_TXI12(void) __attribute__ ((interrupt));;

// SCI12_TEI12
void INT_Excep_SCI12_TEI12(void) __attribute__ ((interrupt));;

// IEB IEBINT
void INT_Excep_IEB_IEBINT(void) __attribute__ ((interrupt));;

// vector 254 reserved
// vector 255 reserved

//;<<VECTOR DATA START (POWER ON RESET)>>
//;Power On Reset PC
extern void PowerON_Reset_PC(void) __attribute__ ((interrupt));;                                                                                                                
//;<<VECTOR DATA END (POWER ON RESET)>>

#endif
