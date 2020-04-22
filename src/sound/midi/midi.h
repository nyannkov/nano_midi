/*
  MIT License

  Copyright (c) 2019 nyannkov

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

#ifndef __MIDI_H__
#define __MIDI_H__

#include "midiconf.h"

typedef struct _MIDI_Channel_Message_Buffer {
    uint8_t msg0; // status byte
    uint8_t msg1; // data byte 1
    uint8_t msg2; // data byte 2
    uint8_t pad;  // padding
}MIDI_Channel_Message_Buffer_t;

typedef struct _MIDI_System_Exclusive_Buffer {
	uint8_t msg[MAX_SYS_EX_BUF_SIZE];
	size_t	len;
}MIDI_System_Exclusive_Buffer_t;


typedef enum _Parse_MIDI_Message_State {
	PARSE_MIDI_IDLE = 0,
	PARSE_MIDI_CH_MSG_1,
	PARSE_MIDI_CH_MSG_2_1,
	PARSE_MIDI_CH_MSG_2_2,
	PARSE_MIDI_CH_MSG_RUNNING_1,
	PARSE_MIDI_CH_MSG_RUNNING_2,
	PARSE_MIDI_SYS_EX,
	NUM_OF_PARSE_MIDI_MESSAGE_STATE
}Parse_MIDI_Message_State_t;


typedef struct _MIDI_ChannelVoiceMessage {
    void (*pNoteOff)(uint8_t ch, uint8_t kk, uint8_t uu);
    void (*pNoteOn)(uint8_t ch, uint8_t kk, uint8_t vv);
    void (*pPolyphonicKeyPressure)(uint8_t ch, uint8_t kk, uint8_t vv);
    void (*pControlChange)(uint8_t ch, uint8_t cc, uint8_t vv);
    void (*pProgramChange)(uint8_t ch, uint8_t pp);
    void (*pChannelPressure)(uint8_t ch, uint8_t vv);
    void (*pPitchBendChange)(uint8_t ch, uint8_t ll, uint8_t hh);
}MIDI_ChannelVoiceMessage_t;

typedef struct _MIDI_ChannelMessage {
    MIDI_ChannelVoiceMessage_t voice_msg;
}MIDI_ChannelMessage_t;

typedef struct _MIDI_SystemExclusiveMessage {
    void (*pSystemExclusive)(uint8_t *dat, size_t len);
}MIDI_SystemExclusiveMessage_t;

typedef struct _MIDI_SystemMessage {
    MIDI_SystemExclusiveMessage_t exclusive_msg;
}MIDI_SystemMessage_t;

typedef struct _MIDI_Message_Callbacks {
    MIDI_ChannelMessage_t       channel;
    MIDI_SystemMessage_t        system;
}MIDI_Message_Callbacks_t;

typedef struct _MIDI_Handle {
	MIDI_Channel_Message_Buffer_t	chmsg_buf;
	MIDI_System_Exclusive_Buffer_t	sysex_buf;
	const MIDI_Message_Callbacks_t	*pcallback;
	Parse_MIDI_Message_State_t	state;
}MIDI_Handle_t;

extern MIDI_Handle_t *MIDI_Init(const MIDI_Message_Callbacks_t *pcallbacks );
extern void MIDI_DeInit( MIDI_Handle_t *phMIDI );
extern int32_t MIDI_Play(MIDI_Handle_t *phMIDI, const uint8_t *midi_msg, size_t len);

#endif // __MIDI_H__
