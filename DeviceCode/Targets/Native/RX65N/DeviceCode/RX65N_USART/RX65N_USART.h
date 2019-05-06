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

#ifndef _RX65N_USART_H_
#define _RX65N_USART_H_

typedef int (*SCI_CALLBACK)(int d);

#ifdef __cplusplus
extern "C" {
#endif

void sci_init(int ch, int baud, int bits, int parity, int stop, int flow);
void sci_init_with_pins(int ch, int tx_pin, int rx_pin, int baud, int bits, int parity, int stop, int flow);
void sci_init_default(int ch, int baud);
void sci_set_baud(int ch, int baud);
UINT8 sci_rx_ch(int ch);
int sci_rx_any(int ch);
int sci_tx_wait(int ch);
void sci_tx_ch(int ch, unsigned char c);
void sci_tx_str(int ch, unsigned char *p);
void sci_deinit(int ch);
typedef int (*SCI_CALLBACK)(int d);
void sci_rx_set_callback(int ch, SCI_CALLBACK callback);
void sci_isr_te(int ch);
void sci_isr_er(int ch);

#ifdef __cplusplus
}
#endif

#endif /* _RX65N_USART_H_ */
