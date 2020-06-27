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
#include "mode4_ymf825.h"
#include "ymf825.h"
#include "ymf825_note_table.h"
#include <string.h> // memcpy
#include <math.h> // pow

extern const uint8_t ymf825_tone_table[128][30];


#define MAX_TONE_NUMBER		16
#define MAX_CH_NUMBER		16
#define NUM_OF_TONE_CFG		30

typedef struct _MIDI_TuningData {
	uint8_t LSB;
	uint8_t MSB;
}MIDI_TuningData_t;


typedef struct _MIDI_PlayTuning {
	MIDI_TuningData_t RPN;
	MIDI_TuningData_t DAT;
	uint8_t PitchBendSensitibity;
	uint8_t ChannelVolume;
	uint8_t Expression;
}MIDI_PlayTuning_t;


static uint8_t _ch_program_tbl[MAX_TONE_NUMBER][NUM_OF_TONE_CFG];
static uint8_t _tone_noise[NUM_OF_TONE_CFG] ={
  0x01,0x80,
  0x00,0x0F,0xF0,0x00,0x00,0x10,0x07,
  0x40,0xDF,0xF0,0x1C,0x00,0x00,0x00,
  0x00,0x2F,0xF3,0x9B,0x00,0x20,0x41,
  0x00,0xAF,0xA0,0x0E,0x10,0x10,0x40,
};

static MIDI_PlayTuning_t _play_tuning[MAX_CH_NUMBER];

static void _ResetChannelSetting(uint8_t ch);

static void _ymf825_NoteOff(uint8_t ch, uint8_t kk, uint8_t uu);
static void _ymf825_NoteOn(uint8_t ch, uint8_t kk, uint8_t vv);
//static void _ymf825_PolyphonicKeyPressure(uint8_t ch, uint8_t kk, uint8_t vv);
static void _ymf825_ControlChange(uint8_t ch, uint8_t cc, uint8_t vv);
static void _ymf825_ProgramChange(uint8_t ch, uint8_t pp);
//static void _ymf825_ChannelPressure(uint8_t ch, uint8_t vv);
static void _ymf825_PitchBendChange(uint8_t ch, uint8_t ll, uint8_t hh);

static const MIDI_Message_Callbacks_t _ymf825_midi_msg_callbacks = {
	// MIDI_ChannelMessage_t
	{
		// MIDI_ChannelVoiceMessage_t
		{
			_ymf825_NoteOff,
			_ymf825_NoteOn,
			NULL,
			_ymf825_ControlChange,
			_ymf825_ProgramChange,
			NULL,
			_ymf825_PitchBendChange
		}
	},
	// MIDI_SystemMessage_t
	{
		// MIDI_SystemExclusiveMessage_t
		{
			NULL
		}
	}
};

MIDI_Handle_t *MIDI_Mode4_YMF825_Init(void) {

	uint8_t i = 0;
	MIDI_Handle_t *phMIDI = NULL;

	YMF825_Init();

	for ( i = 0; i < 16; i++ ) {
		_ResetChannelSetting(i);
	}

	phMIDI = MIDI_Init(&_ymf825_midi_msg_callbacks);

	return phMIDI;
}

void MIDI_Mode4_YMF825_DeInit(MIDI_Handle_t *phMIDI) {

	MIDI_DeInit(phMIDI);
}

static void _ymf825_NoteOff(uint8_t ch, uint8_t kk, uint8_t uu) {
	
	uint8_t tone_num = 0;
	tone_num = ch;
	(void)kk;
	(void)uu;
	// note off
	YMF825_SelectChannel(ch);
	YMF825_KeyOff(tone_num);
}

static void _ymf825_NoteOn(uint8_t ch, uint8_t kk, uint8_t vv) { 
	
	uint8_t  tone_num = 0;
	tone_num = ch;

	if (vv != 0x00 ) {
		float fvelocity = 0.0F;
		uint8_t ChVol = 0;
		uint8_t VoVol = 0;
		uint16_t fnum = 0;
		uint16_t block = 0;

		fvelocity  = ((float)(vv & 0x7F))/127.0F;
		fvelocity *= ((float)(_play_tuning[ch].Expression & 0x7F)) / 127.0F;

		ChVol = (uint8_t)( 31.0F * fvelocity );
		VoVol = (uint8_t)( 31.0F * (float)(_play_tuning[ch].ChannelVolume & 0x7F) / 127.0F);

		// note on
		YMF825_SelectChannel(ch);
		YMF825_ChangeVoVol(VoVol);
		YMF825_ChangeChVol(ChVol);

		fnum = _note_tbl[kk & 0x7F].FNUM;
		block = _note_tbl[kk & 0x7F].BLOCK;

		YMF825_SelectNoteNumber(fnum, block);
		YMF825_KeyOn(tone_num);
	}
	else {

		// note off
		YMF825_SelectChannel(ch);
		YMF825_KeyOff(tone_num);
	}
}

static void _ymf825_ControlChange(uint8_t ch, uint8_t cc, uint8_t vv) {

	switch (cc) {

		case 6:// Data Entry (MSB)
		{
			uint16_t RPN = 0x7F7F;
			RPN = (_play_tuning[ch].RPN.MSB << 8 ) | (_play_tuning[ch].RPN.LSB); 
			_play_tuning[ch].DAT.MSB = vv;

			switch (RPN) {
				case 0x0000:// Pitch Bend Sensitibity;
				{
					_play_tuning[ch].PitchBendSensitibity = _play_tuning[ch].DAT.MSB;

				}
				break;

				case 0x7F7F:// RPN NULL
				{
					// NOTHING TO DO
				}
				break;

				default:
				break;
			}
		}
		break;

		case 7:// Channel Volume
		{
			float fvelocity = 0.0F;
			uint8_t ChVol = 0;
			uint8_t VoVol = 0;

			_play_tuning[ch].ChannelVolume = vv;

			fvelocity  = ((float)(vv & 0x7F))/127.0F;
			fvelocity *= ((float)(_play_tuning[ch].Expression & 0x7F)) / 127.0F;

			ChVol = (uint8_t)( 31.0F * fvelocity );
			VoVol = (uint8_t)( 31.0F * (float)(_play_tuning[ch].ChannelVolume & 0x7F) / 127.0F);

			// note on
			YMF825_SelectChannel(ch);
			YMF825_ChangeVoVol(VoVol);
			YMF825_ChangeChVol(ChVol);
		}
		break;

		case 11:// Expression
		{
			float fvelocity = 0.0F;
			uint8_t ChVol = 0;
			uint8_t VoVol = 0;
			_play_tuning[ch].Expression = vv;

			fvelocity  = ((float)(vv & 0x7F))/127.0F;
			fvelocity *= ((float)(_play_tuning[ch].Expression & 0x7F)) / 127.0F;

			ChVol = (uint8_t)( 31.0F * fvelocity );
			VoVol = (uint8_t)( 31.0F * (float)(_play_tuning[ch].ChannelVolume & 0x7F) / 127.0F);

			// note on
			YMF825_SelectChannel(ch);
			YMF825_ChangeVoVol(VoVol);
			YMF825_ChangeChVol(ChVol);
		}
		break;

		case 38:// Data Entry (LSB)
		{
			uint16_t RPN = 0x7F7F;
			RPN = (_play_tuning[ch].RPN.MSB << 8 ) | (_play_tuning[ch].RPN.LSB); 
			_play_tuning[ch].DAT.LSB = vv;

			switch (RPN) {
				case 0x0000:// Pitch Bend Sensitibity;
				{
					_play_tuning[ch].PitchBendSensitibity = _play_tuning[ch].DAT.MSB;
				}
				break;

				case 0x7F7F:// RPN NULL
				{
					// NOTHING TO DO
				}
				break;

				default:
				break;
			}
		}
		break;
		case 100:// RPN (LSB)
		{
			_play_tuning[ch].RPN.LSB = vv;
		}
		break;

		case 101:// RPN (MSB)
		{
			_play_tuning[ch].RPN.MSB = vv;
		}
		break;

		case 120:// All Sound Off
		{
			uint8_t tone_num = 0;
			tone_num = ch;
			// note off
			YMF825_SelectChannel(ch);
			YMF825_KeyOff(tone_num);
		}
		break;

		case 121:// Reset All Controller
		{
			_ResetChannelSetting(ch);
		}
		break;

		case 123:// All Note Off
		{
			uint8_t tone_num = 0;
			tone_num = ch;
			// note off
			YMF825_SelectChannel(ch);
			YMF825_KeyOff(tone_num);
		}
		break;

		default:
		break;
	}

}

static void _ymf825_ProgramChange(uint8_t ch, uint8_t pp) {

	// TODO control change
	if ( ch != 9 ) {
		memcpy(_ch_program_tbl[(ch&0x0F)], ymf825_tone_table[(pp&0x7F)], 30);
		YMF825_SetToneParameter(_ch_program_tbl);	
	}
}

static void _ymf825_PitchBendChange(uint8_t ch, uint8_t ll, uint8_t hh) {

	uint16_t INT  = 0;
	uint16_t FRAC = 0;
	uint16_t shift = 0;
	uint16_t pitch = 0;
	float fratio = 0.0F;

	shift = (hh << 7) | ll;
	fratio = (float)pow(2.0, (double)_play_tuning[ch].PitchBendSensitibity / 12.0 * (double)(shift-8192) / 8192.0);

	pitch = (uint16_t)(512.0F * fratio);

	if ( pitch >= 2048) {
		pitch = 2047;
	}

	INT = (pitch >> 9) & 0x03; 
	FRAC = pitch & 0x1FF;
		
	YMF825_SelectChannel(ch);
	YMF825_ChangePitch(INT, FRAC);
}

static void _ResetChannelSetting(uint8_t ch) {

	YMF825_SelectChannel(ch);
	YMF825_KeyOff(ch);

	if ( ch < 16 ) {

		// reset program change
		if ( ch == 9 ) {
			memcpy(_ch_program_tbl[ch], _tone_noise, NUM_OF_TONE_CFG);
		}
		else {
			memcpy(_ch_program_tbl[ch], ymf825_tone_table[0], NUM_OF_TONE_CFG);
		}

		// RPN
		_play_tuning[ch].RPN.MSB = 0x7F;
		_play_tuning[ch].RPN.LSB = 0x7F;
		_play_tuning[ch].DAT.MSB = 0x00;
		_play_tuning[ch].DAT.LSB = 0x00;
		_play_tuning[ch].PitchBendSensitibity = 2;
		_play_tuning[ch].Expression = 0x7F;
		_play_tuning[ch].ChannelVolume = 64;

		YMF825_ChangeVoVol(0);
		YMF825_ChangeChVol(0);
		YMF825_ChangePitch(1, 0);
	}
}
