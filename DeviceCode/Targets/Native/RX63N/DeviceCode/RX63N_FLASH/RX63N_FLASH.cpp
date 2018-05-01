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
#include "..\RX63N.h"
#include "RX63N_FLASH.h"

////////////////////////////////////////////////////////////////////////////
// From Renesas sample
////////////////////////////////////////////////////////////////////////////

#if defined(RX63N)
#define ROM_AREA_0  (0x00F80000)
#define ROM_AREA_1  (0x00F00000)
#define ROM_AREA_2  (0x00E80000)
#define ROM_AREA_3  (0x00E00000)
#endif

#if defined(RX610) || defined(RX62T) || defined(RX62N)
#define ICLK_HZ (96000000)
#define PCLK_HZ (48000000)
#define FCLK_HZ (48000000)
#define FLASH_CLOCK_HZ PCLK_HZ
#define WAIT_MAX_ROM_WRITE \
    ((UINT32)(12000 * (50.0/(FLASH_CLOCK_HZ/1000000)))*(ICLK_HZ/1000000))

#elif  defined(RX630) || defined(RX63N)
#define ICLK_HZ (96000000)
#define PCLK_HZ (48000000)
#define FCLK_HZ (48000000)
#define FLASH_CLOCK_HZ FCLK_HZ
#define WAIT_MAX_ROM_WRITE \
    ((UINT32)(12000 * (50.0/(FLASH_CLOCK_HZ/1000000)))*(ICLK_HZ/1000000))
#endif

// fcu_Reset()
#define WAIT_TRESW2     2520        /* Standby time data for tRESW2 timeout control */
// fcu_Transition_RomRead_Mode()
#define WAIT_TE16K      7603200     /* Standby time data for tE16K x1.1 timeout control */
// fcu_Transfer_Firmware()
#define FCU_FIRM_TOP    0xFEFFE000  /* FCU firmware store area start address = FEFFE000h */
#define FCU_RAM_TOP     0x007F8000  /* FCU RAM start address = 007F8000h */
#define FCU_RAM_SIZE    0x2000      /* Size of FCU RAM = 8 Kbytes */
// fcu_Notify_Peripheral_Clock()
#define PCKA_48MHZ      0x0030      /* Periphera module clock(PCLK) = 48MHz */
#define WAIT_TPCKA      1636        /* Standby time data for tPCKA timeout control */
// fcu_Write()
#define WAIT_TP256      345600      /* Standby time data for tP256 x1.1 timeout control */

void fcu_Interrupt_Disable(void)__attribute__((section ("SectionForFlashOperations")));
void fcu_Reset(void)__attribute__((section ("SectionForFlashOperations")));
bool fcu_Transition_RomRead_Mode(volatile unsigned char *) __attribute__((section ("SectionForFlashOperations")));
bool fcu_Transition_RomPE_Mode(unsigned int flash_addr) __attribute__((section ("SectionForFlashOperations")));
bool fcu_Transfer_Firmware(volatile unsigned char *) __attribute__((section ("SectionForFlashOperations")));
bool fcu_Notify_Peripheral_Clock(volatile unsigned char *) __attribute__((section ("SectionForFlashOperations")));
bool fcu_Erase(volatile unsigned char *) __attribute__((section ("SectionForFlashOperations")));
bool fcu_Write(volatile unsigned char *, unsigned short *, unsigned short *) __attribute__((section ("SectionForFlashOperations")));
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// For Debugging
////////////////////////////////////////////////////////////////////////////
//#define DEBUG_FLASH
//#define DEBUG_FLASH_SKIP
//#define DEBUG_FLASH_WriteX
//#define DEBUG_FLASH_EraseBlock
//#define DEBUG_FLASH_Read
//#define DEBUG_FLASH_Memset

#define REGION3_SECTOR_SIZE 0x10000     // 64K
#define REGION3_SECTOR_MAX  16
#define REGION2_SECTOR_SIZE 0x8000      // 32K
#define REGION2_SECTOR_MAX  16
#define REGION1_SECTOR_SIZE 0x4000      // 16K
#define REGION1_SECTOR_MAX  30
#define REGION0_SECTOR_SIZE 0x1000      // 4K
#define REGION0_SECTOR_MAX  6

//#define FLASH_BUF_SIZE 0x100
//#define FLASH_BUF_ADDR_MASK 0xffffff00
//#define FLASH_BUF_OFF_MASK  0x000000ff
#define FLASH_BUF_SIZE 0x80
#define FLASH_BUF_ADDR_MASK 0xffffff80
#define FLASH_BUF_OFF_MASK  0x0000007f

UINT8 fFWLoaded = 0;
UINT8 flash_buf[FLASH_BUF_SIZE]  __attribute__((aligned (2)));

void *lmemset(void *dst, int c, size_t len) __attribute__ ((section ("SectionForFlashOperations")));
void *lmemset(void *dst, int c, size_t len)
{
    char *p;
    for (p = (char *)dst; len > 0; len--)
        *(p++) = c;
    return (void *)dst;
}

void *lmemcpy(void *dst, const void *src, size_t len) __attribute__ ((section ("SectionForFlashOperations")));
void *lmemcpy(void *dst, const void *src, size_t len)
{
    char *d = (char *)dst;
    const char *s = (const char *)src;
    for (; len > 0; len--)
        *(d++) = *(s++);
    return (void *)dst;
}

int lmemcmp(const void *p1, const void *p2, size_t len) __attribute__ ((section ("SectionForFlashOperations")));
int lmemcmp(const void *p1, const void *p2, size_t len)
{
    unsigned char *a, *b;
    size_t i;

    a = (unsigned char *)p1;
    b = (unsigned char *)p2;
    for(i = 0; i < len; i++) {
        if (*a != *b) {
            return (*a < *b) ? -1 : 1;
        }
        a++;
        b++;
    }
    return (int)0;
}

#if 0
UINT32 sector_index(UINT32 addr) __attribute__ ((section ("SectionForFlashOperations")));
UINT32 sector_index(UINT32 addr)
{
    if (addr >= 0xFFFF8000)
        return (0x100000000 - addr)/REGION0_SECTOR_SIZE;
    else if (addr >= 0xFFF80000)
        return REGION0_SECTOR_MAX + ((0xFFFF8000 - addr)/REGION1_SECTOR_SIZE);
    else if (addr >= 0xFFF00000)
        return REGION0_SECTOR_MAX + REGION1_SECTOR_MAX +((0xFFF80000 - addr)/REGION2_SECTOR_SIZE);
    else
        return REGION0_SECTOR_MAX + REGION1_SECTOR_MAX + REGION2_SECTOR_MAX + ((0xFFF00000 - addr)/REGION3_SECTOR_SIZE);
}
#endif

UINT32 sector_size(UINT32 addr) __attribute__ ((section ("SectionForFlashOperations")));
UINT32 sector_size(UINT32 addr)
{
    if (addr >= 0xFFFF8000)
        return REGION0_SECTOR_SIZE;
    else if (addr >= 0xFFF80000)
        return REGION1_SECTOR_SIZE;
    else if (addr >= 0xFFF00000)
        return REGION2_SECTOR_SIZE;
    else
        return REGION3_SECTOR_SIZE;
}

BOOL RX63N_BS_Driver::ChipInitialize(void *context)
{   
#if defined(DEBUG_FLASH)
    debug_printf("ChipInitialize()\r\n");
#endif
//    GLOBAL_LOCK(FlashIrq);
//    fcu_Reset();
    return TRUE;
}

BOOL RX63N_BS_Driver::ChipUnInitialize(void *context)
{
#if defined(DEBUG_FLASH)
    debug_printf("ChipUnInitialize()\r\n");
#endif
    return TRUE;
}

BOOL RX63N_BS_Driver::ChipReadOnly(void *context, BOOL On, UINT32 ProtectionKey)
{
#if defined(DEBUG_FLASH)
    debug_printf("ChipReadOnly()\r\n");
#endif
    return TRUE;
}

const BlockDeviceInfo* RX63N_BS_Driver::GetDeviceInfo(void *context)
{
#if defined(DEBUG_FLASH)
    debug_printf("GetDeviceInfo(context=%x))\r\n", context);
#endif
    BLOCK_CONFIG *config = (BLOCK_CONFIG *)context;
    return config->BlockDeviceInformation;
}

BOOL RX63N_BS_Driver::Read(void *context, ByteAddress Address, UINT32 NumBytes, BYTE *pSectorBuff)
{
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_Read)
    debug_printf("Read(addr=%x, num=%x, psec=%x)\r\n", Address, NumBytes, pSectorBuff);
#endif
    CHIP_WORD *startAddress = (CHIP_WORD *)Address;
    CHIP_WORD *endAddress = (CHIP_WORD *)(Address + NumBytes);
    while (startAddress < endAddress) {
        *pSectorBuff++ = *startAddress++;
    }
    return TRUE;
}

BOOL RX63N_BS_Driver::Write(void *context, ByteAddress Address, UINT32 NumBytes, BYTE *pSectorBuff, BOOL ReadModifyWrite)
{
    return WriteX(context, Address, NumBytes, pSectorBuff, ReadModifyWrite, TRUE);
}

static void wait(volatile int count)
{
    while (count-- > 0);
}

BOOL RX63N_BS_Driver::WriteX(void *context, ByteAddress Address, UINT32 NumBytes, BYTE *pSectorBuff, BOOL ReadModifyWrite, BOOL fIncrementDataPtr)
{
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_WriteX)
    debug_printf("WriteX(addr=%x, num=%x, psec=%x)\r\n", Address, NumBytes, pSectorBuff);
#endif
#ifndef DEBUG_FLASH_SKIP
    GLOBAL_LOCK(irq);
    UINT32 error_code = 0;
    bool flag;
    unsigned char *command_addr = (unsigned char *)((UINT32)Address & 0x00FFFFFF);
    //FLASH_BEGIN_PROGRAMMING_FAST() ;
    UINT32 sector, count;
    UINT32 startAddress  = Address & FLASH_BUF_ADDR_MASK;
    UINT32 offset = Address & FLASH_BUF_OFF_MASK;
    UINT32 endAddress = Address + NumBytes;
    while (startAddress < endAddress) {
        // copy from dst rom addr to flash buffer to keep current data
        memcpy(flash_buf, (void *)startAddress, FLASH_BUF_SIZE);
        //memset(flash_buf, 0xff, FLASH_BUF_SIZE);
        if (NumBytes + offset > FLASH_BUF_SIZE) {
            count = FLASH_BUF_SIZE - offset;
            NumBytes -= count;
        } else
            count = NumBytes;
        // overwrite data from src addr to flash buffer
        if (fIncrementDataPtr)
            memcpy(flash_buf + offset, pSectorBuff, count);
        else
            memset(flash_buf + offset, (int)*pSectorBuff, count);
        command_addr = (unsigned char *)((UINT32)startAddress & 0x00FFFFFF);;
        unsigned short *flash_addr  = (unsigned short *)((UINT32)startAddress & 0x00FFFFFF);
        unsigned short *buf_addr = (unsigned short *)&flash_buf[0];
        wait(100000);
        FLASH_BEGIN_PROGRAMMING_FAST() ;
        fcu_Interrupt_Disable();
        flag = fcu_Transfer_Firmware(command_addr);
        if (!flag) {
            error_code = 1;
            goto WriteX_exit;
        }
        flag = fcu_Transition_RomPE_Mode((unsigned int)command_addr);
        if (!flag) {
            error_code = 2;
            goto WriteX_exit;
        }
        flag = fcu_Notify_Peripheral_Clock(command_addr);
        if (!flag) {
            error_code = 3;
            flag =  fcu_Transition_RomRead_Mode(command_addr);
            goto WriteX_exit;
        }
        flag = fcu_Write(command_addr, flash_addr, buf_addr);
        if (!flag) {
            error_code = 4;
            flag =  fcu_Transition_RomRead_Mode(command_addr);
            goto WriteX_exit;
        }
        flag =  fcu_Transition_RomRead_Mode(command_addr);
        if (!flag) {
            error_code = 5;
            goto WriteX_exit;
        }
        FLASH_END_PROGRAMMING_FAST("", Address);
        if (fIncrementDataPtr) {
            flag= (memcmp((void *)(startAddress+offset), flash_buf+offset, count) == 0);
            if (!flag) {
                error_code = 6;
                break;
            }
        }
        offset = 0;
        startAddress += FLASH_BUF_SIZE;
        pSectorBuff += count;
    }
    //FLASH_END_PROGRAMMING_FAST("", Address);
WriteX_exit:
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_WriteX)
    debug_printf("WriteX() error_code=%x\r\n", error_code);
#endif
#endif
    if (error_code == 0)
        return true;
    return false;
}

BOOL RX63N_BS_Driver::Memset(void *context, ByteAddress Address, UINT8 Data, UINT32 NumBytes)
{
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_Memset)
    debug_printf("Memset(addr=%x, num=%x, data=%x)\r\n", Address, NumBytes, Data);
#endif
    CHIP_WORD chipData;
    memset(&chipData, Data, sizeof(CHIP_WORD));
    return WriteX(context, Address, NumBytes, (BYTE *)&chipData, TRUE, FALSE);
}

BOOL RX63N_BS_Driver::GetSectorMetadata(void *context, ByteAddress SectorStart, SectorMetadata *pSectorMetadata)
{
#if defined(DEBUG_FLASH)
    debug_printf("GetSectorMetadata(start=%x, psecmetadata=%x)\r\n", SectorStart, pSectorMetadata);
#endif
    return FALSE;
}

BOOL RX63N_BS_Driver::SetSectorMetadata(void *context, ByteAddress SectorStart, SectorMetadata *pSectorMetadata)
{
#if defined(DEBUG_FLASH)
    debug_printf("SetSectorMetadata(start=%x, psecmetadata=%x)\r\n", SectorStart, pSectorMetadata);
#endif
    return FALSE;
}

BOOL RX63N_BS_Driver::IsBlockErased(void *context, ByteAddress Address, UINT32 BlockLength)
{
#if defined(DEBUG_FLASH)
    debug_printf("IsBlockErased(addr=%x, len=%x)\r\n", Address, BlockLength);
#endif
    CHIP_WORD *startAddress = (CHIP_WORD *)Address;
    CHIP_WORD *endAddress = (CHIP_WORD *)(Address + BlockLength);
    while (startAddress < endAddress)
        if (*startAddress++ != (CHIP_WORD)0xFFFFFFFF)
            return FALSE;
    return TRUE;
}

// erase one page
BOOL RX63N_BS_Driver::EraseBlock(void *context, ByteAddress Address)
{
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    debug_printf("EraseBlock(addr=%x)\r\n", Address);
#endif
    UINT32 error_code = 0;
    bool flag;
#ifndef DEBUG_FLASH_SKIP
    unsigned char *command_addr = (unsigned char *)((UINT32)Address & 0x00FFFFFF);
    unsigned long block_size  = (unsigned long)sector_size((UINT32)Address);

    FLASH_BEGIN_PROGRAMMING_FAST() ;
    fcu_Interrupt_Disable();
    flag = fcu_Transfer_Firmware(command_addr);
    if (!flag) {
        error_code = 1;
        goto EraseBlock_exit;
    }
    flag = fcu_Transition_RomPE_Mode((unsigned int)command_addr);
    if (!flag) {
        error_code = 2;
        goto EraseBlock_exit;
    }
    flag = fcu_Notify_Peripheral_Clock(command_addr);
    if (!flag) {
        error_code = 3;
        flag =  fcu_Transition_RomRead_Mode(command_addr);
        goto EraseBlock_exit;
    }
    flag = fcu_Erase(command_addr);
    if (!flag) {
        error_code = 4;
        flag =  fcu_Transition_RomRead_Mode(command_addr);
        goto EraseBlock_exit;
    }
    flag =  fcu_Transition_RomRead_Mode(command_addr);
    if (!flag) {
        error_code = 5;
        goto EraseBlock_exit;
    }
    flag = IsBlockErased(context, Address, block_size);
    if (!flag) {
        error_code = 6;
    }
    FLASH_END_PROGRAMMING_FAST("", Address);
EraseBlock_exit:
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    debug_printf("EraseBlock() error_code=%x\r\n", error_code);
#endif
#endif
    if (error_code == 0)
        return true;
    return false;
}

void RX63N_BS_Driver::SetPowerState(void *context, UINT32 State)
{
#if defined(DEBUG_FLASH)
    debug_printf("SetPowerState(state=%x)\r\n", State);
#endif
    return;
}

UINT32 RX63N_BS_Driver::MaxSectorWrite_uSec(void *context)
{
#if defined(DEBUG_FLASH)
    debug_printf("MaxSectorWrite_uSec()\r\n");
#endif
    NATIVE_PROFILE_PAL_FLASH();
    MEMORY_MAPPED_NOR_BLOCK_CONFIG *config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG *)context;

    return config->BlockConfig.BlockDeviceInformation->MaxSectorWrite_uSec;
}

UINT32 RX63N_BS_Driver::MaxBlockErase_uSec(void *context)
{
#if defined(DEBUG_FLASH)
    debug_printf("MaxBlockErase_uSec()\r\n");
#endif
    NATIVE_PROFILE_PAL_FLASH();
    MEMORY_MAPPED_NOR_BLOCK_CONFIG *config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG *)context;

    return config->BlockConfig.BlockDeviceInformation->MaxBlockErase_uSec;
}

////////////////////////////////////////////////////////////////////////////
// From Renesas sample
////////////////////////////////////////////////////////////////////////////

void fcu_Interrupt_Disable(void)
{
    FLASH.FRDYIE.BYTE &= ~(1 << 0);
#if 0
    FLASH.FAEINT.BYTE &= ~(1 << 7);
    FLASH.FAEINT.BYTE &= ~(1 << 4);
    FLASH.FAEINT.BYTE &= ~(1 << 3);
    FLASH.FAEINT.BYTE &= ~(1 << 1);
    FLASH.FAEINT.BYTE &= ~(1 << 0);
#else
    FLASH.FAEINT.BYTE &= ~0x9b;
#endif
    ICU.IPR[0x01].BIT.IPR = 0;      // FIFERR IPR
    ICU.IER[0x02].BIT.IEN5 = 0;     // FIFERR IEN
    ICU.IPR[0x02].BIT.IPR = 0;      // FRDYI IPR
    ICU.IER[0x02].BIT.IEN7 = 0;     // FRDYI IEN
}

void fcu_Reset(void)
{
    volatile unsigned long count;
    FLASH.FRESETR.WORD = 0xCC01;
    count = WAIT_TRESW2;
    while (count-- != 0) ;
    FLASH.FRESETR.WORD = 0xCC00;
}

bool fcu_Transition_RomRead_Mode(volatile unsigned char *command_addr)
{
    unsigned long count;
    count = WAIT_TE16K;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count --;
        if (count == 0) {
            fcu_Reset();
            FLASH.FENTRYR.WORD = 0xAA00;
            while (0x0000 != FLASH.FENTRYR.WORD) ;
            FLASH.FWEPROR.BYTE = 0x02;
            return false;
        }
    }
    if ((FLASH.FSTATR0.BIT.ILGLERR == 1)
    || (FLASH.FSTATR0.BIT.ERSERR == 1)
    || (FLASH.FSTATR0.BIT.PRGERR == 1)) {
        if (FLASH.FSTATR0.BIT.ILGLERR == 1) {
            if (FLASH.FASTAT.BYTE != 0x10) {
                FLASH.FASTAT.BYTE = 0x10;
            }
        }
        *command_addr = 0x50;
    }
    FLASH.FENTRYR.WORD = 0xAA00;
    while (0x0000 != FLASH.FENTRYR.WORD) ;
    FLASH.FWEPROR.BYTE = 0x02;
    return true;
}

bool fcu_Transition_RomPE_Mode(unsigned int flash_addr)
{
    FLASH.FENTRYR.WORD = 0xAA00;
    while (0x0000 != FLASH.FENTRYR.WORD) ;
#if defined(RX62N)
    FLASH.FENTRYR.WORD = 0xAA01;
#elif defined(RX63N)
    if (flash_addr >= ROM_AREA_0) {
        /* Enter ROM PE mode for addresses 0xFFF80000 - 0xFFFFFFFF */
        FLASH.FENTRYR.WORD = 0xAA01;
    } else if ((flash_addr < ROM_AREA_0) && (flash_addr >= ROM_AREA_1)) {
        /* Enter ROM PE mode for addresses 0xFFF00000 - 0xFFF7FFFF */
        FLASH.FENTRYR.WORD = 0xAA02;
    } else if ((flash_addr < ROM_AREA_1) && (flash_addr >= ROM_AREA_2)) {
        /* Enter ROM PE mode for addresses 0xFFE80000 - 0xFFEFFFFF */
        FLASH.FENTRYR.WORD = 0xAA04;
    } else {
        /* Enter ROM PE mode for addresses 0xFFE00000 - 0xFFE7FFFF */
        FLASH.FENTRYR.WORD = 0xAA08;
    }
#endif
    FLASH.FWEPROR.BYTE = 0x01;
    if ((FLASH.FSTATR0.BIT.ILGLERR == 1)
    || (FLASH.FSTATR0.BIT.ERSERR == 1)
    || (FLASH.FSTATR0.BIT.PRGERR == 1)) {
        return false;
    }
    if (FLASH.FSTATR1.BIT.FCUERR == 1) {
        return false;
    }
    return true;
}

bool fcu_Transfer_Firmware(volatile unsigned char *command_addr)
{
    if (FLASH.FENTRYR.WORD != 0x0000) {
        if (!fcu_Transition_RomRead_Mode(command_addr))
            return false;
    }
    FLASH.FCURAME.WORD = 0xC401;
    memcpy((void *)FCU_RAM_TOP, (void *)FCU_FIRM_TOP, (unsigned long)FCU_RAM_SIZE);
    return true;
}

bool fcu_Notify_Peripheral_Clock(volatile unsigned char *command_addr)
{
    unsigned long count;
    FLASH.PCKAR.WORD = PCKA_48MHZ;
    *command_addr = 0xE9;
    *command_addr = 0x03;
    *(volatile unsigned short *)command_addr = 0x0F0F;
    *(volatile unsigned short *)command_addr = 0x0F0F;
    *(volatile unsigned short *)command_addr = 0x0F0F;
    *command_addr = 0xD0;

    count = WAIT_TPCKA;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count --;
        if (count == 0) {
            fcu_Reset();
            return false;
        }
    }
    if (FLASH.FSTATR0.BIT.ILGLERR == 1)
        return false;
    return true;
}

bool fcu_Erase(volatile unsigned char *command_addr)
{
    unsigned long count;
    //FLASH.FWEPROR.BYTE = 0x01;
    FLASH.FPROTR.WORD = 0x5501;
    *command_addr = 0x20;
    *command_addr = 0xD0;
    count = WAIT_TE16K;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count --;
        if (count == 0) {
            fcu_Reset();
            return false;
        }
    }
    if ((FLASH.FSTATR0.BIT.ILGLERR == 1)
    || (FLASH.FSTATR0.BIT.ERSERR == 1)) {
        return false;
    }
    return true;
}

bool fcu_Write(volatile unsigned char *command_addr, unsigned short *flash_addr, unsigned short *buf_addr)
{
    unsigned long cycle_cnt;
    unsigned long count;
    //FLASH.FWEPROR.BYTE = 0x01;
    FLASH.FPROTR.WORD = 0x5501;
    *command_addr = 0xE8;
    *command_addr = 0x40;
    cycle_cnt = 4;
    while (cycle_cnt <= 67) {
        *flash_addr = *buf_addr;
        buf_addr ++;
        flash_addr ++;
        cycle_cnt++;
    }
    *command_addr = 0xD0;
    count = WAIT_TP256;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count --;
        if (count == 0) {
            fcu_Reset();
            return false;
        }
    }
    if ((FLASH.FSTATR0.BIT.ILGLERR == 1)
    || (FLASH.FSTATR0.BIT.PRGERR == 1)) {
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
// IBlockStorage structure
////////////////////////////////////////////////////////////////////////////

struct IBlockStorageDevice g_RX63N_INTERNALFLASH_DeviceTable =
{
    &RX63N_BS_Driver::ChipInitialize,
    &RX63N_BS_Driver::ChipUnInitialize,
    &RX63N_BS_Driver::GetDeviceInfo,
    &RX63N_BS_Driver::Read,
    &RX63N_BS_Driver::Write,
    &RX63N_BS_Driver::Memset,
    &RX63N_BS_Driver::GetSectorMetadata,
    &RX63N_BS_Driver::SetSectorMetadata,
    &RX63N_BS_Driver::IsBlockErased,
    &RX63N_BS_Driver::EraseBlock,
    &RX63N_BS_Driver::SetPowerState,
    &RX63N_BS_Driver::MaxSectorWrite_uSec,
    &RX63N_BS_Driver::MaxBlockErase_uSec,
};
