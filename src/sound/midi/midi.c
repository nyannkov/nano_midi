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

#include "midi.h"


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

typedef enum _Parse_MIDI_Message_Event {
	PARSE_MIDI_EVENT_RCV_STS_CH_MSG_1 = 0,
	PARSE_MIDI_EVENT_RCV_STS_CH_MSG_2,
	PARSE_MIDI_EVENT_RCV_SYS_RT,
	PARSE_MIDI_EVENT_RCV_DAT,
	PARSE_MIDI_EVENT_RCV_SYS_EX_START,
	PARSE_MIDI_EVENT_RCV_SYS_EX_EOX,
	NUM_OF_PARSE_MIDI_MESSAGE_EVENT
}Parse_MIDI_Message_Event_t;

typedef struct _Parse_MIDI_FSM {
	Parse_MIDI_Message_State_t next;
	void (*pTransStateFunc)(MIDI_Handle_t *phMIDI, uint8_t msg);
}Parse_MIDI_FSM_t;

struct _MIDI_Handle {
	MIDI_Channel_Message_Buffer_t	chmsg_buf;
	MIDI_System_Exclusive_Buffer_t	sysex_buf;
	const MIDI_Message_Callbacks_t	*pcallback;
	Parse_MIDI_Message_State_t	state;
};

static inline Parse_MIDI_Message_Event_t _GetParseMIDIMessageEvent(uint8_t msg) {

	if ( (msg & 0x80) == 0 ) {
		return PARSE_MIDI_EVENT_RCV_DAT;
	}
	else if ( (msg & 0xF0) == 0xF0 ) {
		switch ( msg ) {
			case 0xF0:
			return PARSE_MIDI_EVENT_RCV_SYS_EX_START;

			case 0xF7:
			return PARSE_MIDI_EVENT_RCV_SYS_EX_EOX;

			default:
			return PARSE_MIDI_EVENT_RCV_SYS_RT;
		}
	}
	else {

		switch ( msg & 0xF0 ) {
			case 0x80:
			case 0x90:
			case 0xA0:
			case 0xB0:
			case 0xE0:
			return PARSE_MIDI_EVENT_RCV_STS_CH_MSG_2;

			case 0xC0:
			case 0xD0:
			return PARSE_MIDI_EVENT_RCV_STS_CH_MSG_1;

			default:
			// unexpected
			return PARSE_MIDI_EVENT_RCV_SYS_RT;
		}
	}
	
}

static void _StoreChannelMessageStatus(MIDI_Handle_t *phMIDI, uint8_t msg); 
static void _StoreChannelMessageData(MIDI_Handle_t *phMIDI, uint8_t msg);
static void _StoreSystemExclusiveMessage(MIDI_Handle_t *phMIDI, uint8_t msg);
static void _ExecChannelMessage1(MIDI_Handle_t *phMIDI, uint8_t msg);
static void _ExecChannelMessage2(MIDI_Handle_t *phMIDI, uint8_t msg);
static void _ExecSystemExclusiveMessage(MIDI_Handle_t *phMIDI, uint8_t msg);

static const Parse_MIDI_FSM_t _midi_trans_state_tbl[NUM_OF_PARSE_MIDI_MESSAGE_STATE][NUM_OF_PARSE_MIDI_MESSAGE_EVENT] = {
	/* IDLE */ 
	{
		{PARSE_MIDI_CH_MSG_1, _StoreChannelMessageStatus}, // RCV_STS_CH_MSG_1
		{PARSE_MIDI_CH_MSG_2_1, _StoreChannelMessageStatus}, // RCV_STS_CH_MSG_2
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_RT
		{PARSE_MIDI_IDLE, NULL}, // RCV_DAT
		{PARSE_MIDI_SYS_EX, _StoreSystemExclusiveMessage}, // RCV_SYS_EX_START
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_EOX
	},
	/* CH_MSG_1 */
	{
		{PARSE_MIDI_IDLE, NULL}, // RCV_STS_CH_MSG_1
		{PARSE_MIDI_IDLE, NULL}, // RCV_STS_CH_MSG_2
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_RT
		{PARSE_MIDI_CH_MSG_RUNNING_1, _ExecChannelMessage1}, // RCV_DAT
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_START
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_EOX
	},
	/* CH_MSG_2_1 */
	{
		{PARSE_MIDI_IDLE, NULL}, // RCV_STS_CH_MSG_1
		{PARSE_MIDI_IDLE, NULL}, // RCV_STS_CH_MSG_2
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_RT
		{PARSE_MIDI_CH_MSG_2_2, _StoreChannelMessageData}, // RCV_DAT
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_START
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_EOX
	},
	/* CH_MSG_2_2 */
	{
		{PARSE_MIDI_IDLE, NULL}, // RCV_STS_CH_MSG_1
		{PARSE_MIDI_IDLE, NULL}, // RCV_STS_CH_MSG_2
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_RT
		{PARSE_MIDI_CH_MSG_RUNNING_2, _ExecChannelMessage2}, // RCV_DAT
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_START
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_EOX
	},
	/* CH_MSG_RUNNING_1 */
	{
		{PARSE_MIDI_CH_MSG_1, _StoreChannelMessageStatus}, // RCV_STS_CH_MSG_1
		{PARSE_MIDI_CH_MSG_2_1, _StoreChannelMessageStatus}, // RCV_STS_CH_MSG_2
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_RT
		{PARSE_MIDI_CH_MSG_RUNNING_1, _ExecChannelMessage1}, // RCV_DAT
		{PARSE_MIDI_SYS_EX, NULL}, // RCV_SYS_EX_START
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_EOX
	},
	/* CH_MSG_RUNNING_2 */
	{
		{PARSE_MIDI_CH_MSG_1, _StoreChannelMessageStatus}, // RCV_STS_CH_MSG_1
		{PARSE_MIDI_CH_MSG_2_1, _StoreChannelMessageStatus}, // RCV_STS_CH_MSG_2
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_RT
		{PARSE_MIDI_CH_MSG_2_2, _StoreChannelMessageData}, // RCV_DAT
		{PARSE_MIDI_SYS_EX, NULL}, // RCV_SYS_EX_START
		{PARSE_MIDI_IDLE, NULL}, // RCV_SYS_EX_EOX
	},
	/* PARSE_MIDI_SYS_EX */
	{
		{PARSE_MIDI_SYS_EX, _StoreSystemExclusiveMessage}, // RCV_STS_CH_MSG_1
		{PARSE_MIDI_SYS_EX, _StoreSystemExclusiveMessage}, // RCV_STS_CH_MSG_2
		{PARSE_MIDI_SYS_EX, _StoreSystemExclusiveMessage}, // RCV_SYS_RT
		{PARSE_MIDI_SYS_EX, _StoreSystemExclusiveMessage}, // RCV_DAT
		{PARSE_MIDI_SYS_EX, _StoreSystemExclusiveMessage}, // RCV_SYS_EX_START
		{PARSE_MIDI_IDLE, _ExecSystemExclusiveMessage}, // RCV_SYS_EX_EOX
	}
};


MIDI_Handle_t *MIDI_Init(const MIDI_Message_Callbacks_t *pcallbacks ) {

	MIDI_Handle_t *phMIDI = NULL;
	phMIDI = MIDI_ALLOC(sizeof(MIDI_Handle_t));

	if ( phMIDI != NULL ) {
		uint32_t i = 0;
		phMIDI->chmsg_buf.msg0 = 0;
		phMIDI->chmsg_buf.msg1 = 0;
		phMIDI->chmsg_buf.msg2 = 0;
		phMIDI->chmsg_buf.pad  = 0;
		phMIDI->sysex_buf.len  = 0;
		for ( i = 0; i < MAX_SYS_EX_BUF_SIZE; i++ ) {
			phMIDI->sysex_buf.msg[i] = 0;
		}

		phMIDI->state = PARSE_MIDI_IDLE;
		phMIDI->pcallback = pcallbacks;
	}

	return phMIDI;
}

void MIDI_DeInit( MIDI_Handle_t *phMIDI ) {
	
	if ( phMIDI != NULL ) {
		MIDI_FREE(phMIDI);
	}
}

int32_t MIDI_Play(MIDI_Handle_t *phMIDI, const uint8_t *midi_msg, size_t len) {

	uint32_t i = 0;
	Parse_MIDI_Message_Event_t event = PARSE_MIDI_EVENT_RCV_STS_CH_MSG_1;
	const Parse_MIDI_FSM_t *pfsm = NULL;
	
	for ( i = 0; i < len; i++ )	{
		event = _GetParseMIDIMessageEvent(midi_msg[i]);

		pfsm = &_midi_trans_state_tbl[phMIDI->state][event];

		if (  pfsm->pTransStateFunc != NULL ) {
			pfsm->pTransStateFunc(phMIDI, midi_msg[i]);
		}
		phMIDI->state = pfsm->next;
	}

	return 0;
}


static void _StoreChannelMessageStatus(MIDI_Handle_t *phMIDI, uint8_t msg) {
	phMIDI->chmsg_buf.msg0 = msg;
}

static void _StoreChannelMessageData(MIDI_Handle_t *phMIDI, uint8_t msg) {
	phMIDI->chmsg_buf.msg1 = msg;
}

static void _StoreSystemExclusiveMessage(MIDI_Handle_t *phMIDI, uint8_t msg) {
	MIDI_System_Exclusive_Buffer_t *psys_ex_buf = &phMIDI->sysex_buf;
	if (  psys_ex_buf->len < MAX_SYS_EX_BUF_SIZE ) {
		psys_ex_buf->msg[(psys_ex_buf->len)++] = msg;
	}
}

static void _ExecChannelMessage1(MIDI_Handle_t *phMIDI, uint8_t msg) {

	const MIDI_ChannelVoiceMessage_t *pchvmsg = &phMIDI->pcallback->channel.voice_msg;

	phMIDI->chmsg_buf.msg1 = msg;

	switch (phMIDI->chmsg_buf.msg0 & 0xF0) {
		case 0xC0:
		if ( pchvmsg->pProgramChange != NULL ) {
			pchvmsg->pProgramChange(phMIDI->chmsg_buf.msg0 & 0x0F, phMIDI->chmsg_buf.msg1);
		}
		break;

		case 0xD0:
		if ( pchvmsg->pChannelPressure != NULL ) {
			pchvmsg->pChannelPressure(phMIDI->chmsg_buf.msg0 & 0x0F, phMIDI->chmsg_buf.msg1);
		}
		break;

		default:
		break; 
	}
}

static void _ExecChannelMessage2(MIDI_Handle_t *phMIDI, uint8_t msg) {

	const MIDI_ChannelVoiceMessage_t *pchvmsg = &phMIDI->pcallback->channel.voice_msg;
	phMIDI->chmsg_buf.msg2 = msg;

	switch (phMIDI->chmsg_buf.msg0 & 0xF0) {
		case 0x80:
		if ( pchvmsg->pNoteOff != NULL ) {
			pchvmsg->pNoteOff(phMIDI->chmsg_buf.msg0 & 0x0F, phMIDI->chmsg_buf.msg1, phMIDI->chmsg_buf.msg2);
		}
		break;

		case 0x90:
		if ( pchvmsg->pNoteOn != NULL ) {
			pchvmsg->pNoteOn(phMIDI->chmsg_buf.msg0 & 0x0F, phMIDI->chmsg_buf.msg1, phMIDI->chmsg_buf.msg2);
		}
		break;

		case 0xA0:
		if ( pchvmsg->pPolyphonicKeyPressure != NULL ) {
			pchvmsg->pPolyphonicKeyPressure(phMIDI->chmsg_buf.msg0 & 0x0F, phMIDI->chmsg_buf.msg1, phMIDI->chmsg_buf.msg2);
		}
		break;

		case 0xB0:
		if ( pchvmsg->pControlChange != NULL ) {
			pchvmsg->pControlChange(phMIDI->chmsg_buf.msg0 & 0x0F, phMIDI->chmsg_buf.msg1, phMIDI->chmsg_buf.msg2);
		}
		break;

		case 0xE0:
		if ( pchvmsg->pPitchBendChange != NULL ) {
			pchvmsg->pPitchBendChange(phMIDI->chmsg_buf.msg0 & 0x0F, phMIDI->chmsg_buf.msg1, phMIDI->chmsg_buf.msg2);
		}
		break;

		default:
		break; 
	}
}


static void _ExecSystemExclusiveMessage(MIDI_Handle_t *phMIDI, uint8_t msg) {

	const MIDI_SystemExclusiveMessage_t *psys_ex = &phMIDI->pcallback->system.exclusive_msg;
	MIDI_System_Exclusive_Buffer_t *psys_ex_buf = &phMIDI->sysex_buf;

	(void)msg;

	if ( psys_ex->pSystemExclusive != NULL ) {
		psys_ex->pSystemExclusive(psys_ex_buf->msg, psys_ex_buf->len);
	}

	// clear
	psys_ex_buf->len = 0;
}