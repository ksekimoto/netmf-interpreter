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

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rwdata = "g_AvailableFSInterfaces"
#endif

extern FILESYSTEM_DRIVER_INTERFACE g_FAT32_FILE_SYSTEM_DriverInterface;
extern STREAM_DRIVER_INTERFACE g_FAT32_STREAM_DriverInterface;

//#ifdef RX63N_SERIAL_FLASH
extern struct BlockStorageDevice  g_RX63N_BS;
FileSystemVolume g_RX63N_NOR_FS;
//#else
//extern struct BlockStorageDevice g_SerialFlash_Device;
//FileSystemVolume g_RX63N_SERIAL_FS;
//#endif

extern struct BlockStorageDevice  g_SD_BS;
extern struct IBlockStorageDevice g_SD_BS_DeviceTable;
extern struct SD_BL_CONFIGURATION g_SD_BS_Config;
FileSystemVolume g_SD_FS;

void SD_InsertEjectIsr(GPIO_PIN Pin, BOOL PinState, void* Param)
{
    if (!PinState) {
        if (BlockStorageList::AddDevice(&g_SD_BS, &g_SD_BS_DeviceTable, &g_SD_BS_Config, TRUE)) {
            FS_MountVolume("SD", 0, 0, &g_SD_BS);
        }
    } else {
        FS_UnmountVolume(&g_SD_BS);
        BlockStorageList::RemoveDevice(&g_SD_BS, TRUE);
    }
}

void FS_AddVolumes()
{
//#ifdef RX63N_SERIAL_FLASH
//    FileSystemVolumeList::AddVolume( &g_RX63N_NOR_FS, "ROOT", 0, 0, &g_FAT32_STREAM_DriverInterface, &g_FAT32_FILE_SYSTEM_DriverInterface, &g_RX63N_BS, 1, FALSE );
//#else
//    FileSystemVolumeList::AddVolume( &g_RX63N_SERIAL_FS, "ROOT", 0, 0, &g_FAT32_STREAM_DriverInterface, &g_FAT32_FILE_SYSTEM_DriverInterface, &g_SerialFlash_Device, 1, FALSE );
//#endif
//    FileSystemVolumeList::AddVolume( &g_SD_FS, "SD", 0, 0, &g_FAT32_STREAM_DriverInterface, &g_FAT32_FILE_SYSTEM_DriverInterface, &g_SD_BS, 0, FALSE );
	 CPU_GPIO_EnableInputPin(SD_INSERT_PIN, TRUE, SD_InsertEjectIsr, GPIO_INT_EDGE_BOTH, RESISTOR_PULLUP);
}

void FS_MountRemovableVolumes()
{
    // if the SD card is inserted, try to mount it.
    if (!CPU_GPIO_GetPinState(SD_INSERT_PIN)) {
        if (BlockStorageList::AddDevice(&g_SD_BS, &g_SD_BS_DeviceTable, &g_SD_BS_Config, TRUE)) {
            FS_MountVolume("SD", 0, 0, &g_SD_BS);
        }
    }
}

FILESYSTEM_INTERFACES g_AvailableFSInterfaces[] =
{
    { &g_FAT32_FILE_SYSTEM_DriverInterface, &g_FAT32_STREAM_DriverInterface },
};

//const size_t g_InstalledFSCount = ARRAYSIZE(g_AvailableFSInterfaces);
const size_t g_InstalledFSCount = 1;

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rwdata
#endif
