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
#ifndef __SINGLE_YMZ294_H__
#define __SINGLE_YMZ294_H__

#include <stdint.h>
#include "midi.h"

#define MAX_MIDI_CH_NUMBER      16

#define YMZ294_ENVELOPE_DISABLE 0
#define YMZ294_ENVELOPE_ENABLE  1

#define YMZ294_MIXER_TONE  		0
#define YMZ294_MIXER_NOISE 		1

typedef enum
{
	PLAY_MODE_0_NO_SKIP_PERCUSSION_CHANNEL,
	PLAY_MODE_1_SKIP_PERCUSSION_CHANNEL
} play_mode_t;

typedef struct
{
	uint8_t  sel_mixer;
	uint8_t  env_mode;
	uint8_t  env_shape;
	uint16_t env_freq;
	uint16_t noise_freq;
} ymz294_setting_t;


extern MIDI_Handle_t *midi_ymz294_init(void);
extern void midi_ymz294_deinit(MIDI_Handle_t *phMIDI);
extern void set_play_mode(play_mode_t mode);
extern play_mode_t get_play_mode(void);
extern int32_t set_ymz294_setting(uint8_t midi_ch, const ymz294_setting_t *p_settings);
extern int32_t get_ymz294_setting(uint8_t midi_ch, ymz294_setting_t *dest_buf);

#endif//__SINGLE_YMZ294_H__
