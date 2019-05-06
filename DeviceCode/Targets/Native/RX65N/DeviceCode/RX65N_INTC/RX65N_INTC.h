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

#ifndef _RX65N_INTC_H_
#define _RX65N_INTC_H_

#ifdef __cplusplus
extern "C" {
#endif

void set_int_state_groupbl0(int flag);
void set_int_state_groupbl1(int flag);
void set_int_state_groupal0(int flag);

void set_int_priority_groupbl0(int ipl);
void set_int_priority_groupbl1(int ipl);
void set_int_priority_groupal0(int ipl);

#ifdef __cplusplus
}
#endif

#endif /* _RX65N_INTC_H_ */
