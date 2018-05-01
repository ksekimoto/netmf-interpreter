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

extern struct BlockStorageDevice  g_INTERNALFLASH;
extern struct IBlockStorageDevice g_RX63N_INTERNALFLASH_DeviceTable;	// RX63N_FLASH.cpp
extern struct BLOCK_CONFIG        g_INTERNALFLASH_Config;               // InternalFlash_Bl_Config.cpp

void BlockStorage_AddDevices()
{   
    BlockStorageList::AddDevice(&g_INTERNALFLASH, &g_RX63N_INTERNALFLASH_DeviceTable, &g_INTERNALFLASH_Config, FALSE);
}

