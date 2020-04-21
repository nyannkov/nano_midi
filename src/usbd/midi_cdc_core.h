/*
  MIT License

  Copyright (c) 2020 nyannkov

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef MIDI_CDC_CORE_H
#define MIDI_CDC_CORE_H

#include <stdint.h>
#include <stddef.h>

typedef int32_t (*pf_usb_midi_receive_callback_t)(const uint8_t *recv_msg, size_t len); 
typedef int32_t (*pf_usb_cdc_receive_callback_t)(const uint8_t *recv_data, size_t len);

extern void register_usb_midi_receive_callback(const pf_usb_midi_receive_callback_t callback);
extern void register_usb_cdc_receive_callback(const pf_usb_cdc_receive_callback_t callback);

extern void init_usbd_midi_cdc(
        const pf_usb_cdc_receive_callback_t cdc_recv_cb,
        const pf_usb_midi_receive_callback_t midi_recv_cb);

extern void usb_cdc_send_service_irq(void);

extern int usb_cdc_printf(const char *format, ...);

#endif/* MIDI_CDC_CORE_H */

