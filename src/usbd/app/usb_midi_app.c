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
#include "usb_midi_app.h"
#include "midi.h"
#include "mode4_ymf825.h"

typedef struct _usb_midi_event_packet
{
	uint8_t header;
	uint8_t midi[3];
} usb_midi_event_packet_t;

static const size_t _cin_midi_x_size_tbl[16] = 
{
	0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1
};

static MIDI_Handle_t *ph_midi;

void init_usb_midi_app(void)
{
	ph_midi = MIDI_Mode4_YMF825_Init();
}

int32_t usb_midi_proc(const uint8_t *mid_msg,  size_t len)
{
	uint32_t i = 0;
	size_t midi_x_size = 0;
	usb_midi_event_packet_t *packet = (usb_midi_event_packet_t *)0;
	uint8_t cin = 0;

	if ( (len % 4) != 0 )
	{
		return -1;
	}
	for ( i = 0; i < len; i += 4 ) 
	{
		packet = (usb_midi_event_packet_t *)&mid_msg[i];
		cin = packet->header & 0x0f;
		midi_x_size = _cin_midi_x_size_tbl[cin];
		if ( midi_x_size != 0 )
		{
			MIDI_Play(ph_midi, &packet->midi[0], midi_x_size);
		}
	}
	return 0;
}
