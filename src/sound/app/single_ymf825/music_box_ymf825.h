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
#ifndef __MUSIC_BOX_YMF825_H__
#define __MUSIC_BOX_YMF825_H__

#include "midi.h"

#define MUSIC_BOX_YMF825_IGNORE_PERCUSSION_MESSAGE  0
#define MUSIC_BOX_YMF825_ACCEPT_PERCUSSION_MESSAGE  1

#pragma pack(1)
typedef struct
{
  uint8_t percussion_msg; // accept or ignore
  uint8_t program_no;     // Program number to use. The range is [1-128].
} music_box_ymf825_config_t;
#pragma pack()

extern MIDI_Handle_t *MIDI_MUSIC_BOX_YMF825_Init(void);
extern void MIDI_MUSIC_BOX_YMF825_DeInit(MIDI_Handle_t *phMIDI);
extern int32_t SetConfig_MUSIC_BOX_YMF825(const music_box_ymf825_config_t *cfg);
extern int32_t GetConfig_MUSIC_BOX_YMF825(music_box_ymf825_config_t *out);

#endif /* __MUSIC_BOX_YMF825_H__ */