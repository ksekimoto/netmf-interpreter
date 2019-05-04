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

#ifndef _RX65N_FLASH_H_1
#define _RX65N_FLASH_H_1   1

//--//

#define FLASH_SECTION __attribute__((section("SectionForFlashOperations")))
typedef UINT8 CHIP_WORD;

struct RX65N_BS_Driver
{
    //--// The following values will be modified
    static const CHIP_WORD MANUFACTURER_CODE = 0xffff;
    static const CHIP_WORD DEVICE_CODE = 0xffff;
    static const size_t BLOCK_ERASE_TYPICAL_TIME_USEC = 96000;
    static const size_t SECTOR_WRITE_TYPICAL_TIME_USEC = 48000;
    static const size_t BLOCK_ERASE_MAX_TIME_USEC = 96000;
    static const size_t SECTOR_WRITE_MAX_TIME_USEC = 48000;
    //--//
    static BOOL ChipInitialize(void *context) __attribute__((section("SectionForFlashOperations")));
    static BOOL ChipUnInitialize(void *context) __attribute__((section("SectionForFlashOperations")));
    static const BlockDeviceInfo* GetDeviceInfo(void *context) __attribute__((section("SectionForFlashOperations")));
    static BOOL Read(void *context, ByteAddress Address, UINT32 NumBytes, BYTE *pSectorBuff) __attribute__((section("SectionForFlashOperations")));
    static BOOL Write(void *context, ByteAddress Address, UINT32 NumBytes, BYTE *pSectorBuff, BOOL ReadModifyWrite) __attribute__((section("SectionForFlashOperations")));
    static BOOL Memset(void *context, ByteAddress Address, UINT8 Data, UINT32 NumBytes) __attribute__((section("SectionForFlashOperations")));
    static BOOL GetSectorMetadata(void *context, ByteAddress SectorStart, SectorMetadata *pSectorMetadata) __attribute__((section("SectionForFlashOperations")));
    static BOOL SetSectorMetadata(void *context, ByteAddress SectorStart, SectorMetadata *pSectorMetadata) __attribute__((section("SectionForFlashOperations")));
    static BOOL IsBlockErased(void *context, ByteAddress Sector, UINT32 BlockLength) __attribute__((section("SectionForFlashOperations")));
    static BOOL EraseBlock(void *context, ByteAddress Sector) __attribute__((section("SectionForFlashOperations")));
    static void SetPowerState(void *context, UINT32 State) __attribute__((section("SectionForFlashOperations")));
    static UINT32 MaxSectorWrite_uSec(void *context) __attribute__((section("SectionForFlashOperations")));
    static UINT32 MaxBlockErase_uSec(void *context) __attribute__((section("SectionForFlashOperations")));
    static BOOL ChipReadOnly(void *context, BOOL On, UINT32 ProtectionKey) __attribute__((section("SectionForFlashOperations")));
private:
    static BOOL WriteX(void *context, ByteAddress Address, UINT32 NumBytes, BYTE *pSectorBuff, BOOL ReadModifyWrite, BOOL fIncrementDataPtr) __attribute__((section("SectionForFlashOperations")));
};

//--//

#endif //_RX65N_FLASH_H_1

