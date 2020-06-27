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
#ifndef __USB_MIDI_H__
#define __USB_MIDI_H__

#include <stdint.h>
#include <stddef.h>

typedef enum
{
  YMF825_SOUND_DRIVER_MODE4 = 0,
  YMF825_SOUND_DRIVER_MUSIC_BOX,
  NUM_OF_YMF825_SOUND_DRIVER
} ymf825_sound_driver_t;


extern void    init_usb_midi_app(void);
extern int32_t usb_midi_proc(const uint8_t *mid_msg,  size_t len);
extern int32_t switch_ymf825_sound_driver(ymf825_sound_driver_t driver);
extern ymf825_sound_driver_t get_selected_ymf825_sound_driver(void);

#endif//__USB_MIDI_H__
