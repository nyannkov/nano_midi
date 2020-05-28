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
#include <math.h>
#include "single_ymz294.h"
#include "ymz294.h"

#define YMZ294_CHANNEL_A        0
#define YMZ294_CHANNEL_B        1
#define YMZ294_CHANNEL_C        2
#define NUM_OF_YMZ294_CHANNEL   3

#define YMZ294_NOTE_OFF         0 
#define YMZ294_NOTE_ON          1



#pragma pack(1)
typedef struct _MIDI_TuningData
{
	uint8_t LSB;
	uint8_t MSB;
} MIDI_TuningData_t;

typedef struct _MIDI_PlayTuning
{
	// MIDI parameters
	MIDI_TuningData_t RPN;
	MIDI_TuningData_t DAT;
	uint8_t PitchBendSensitibity;
	uint8_t ChannelVolume;
	uint8_t Expression;
	// YMZ294 parameters
	ymz294_setting_t ymz294_setting;
} MIDI_PlayTuning_t;

typedef struct 
{
	uint8_t key_stat; // current key state (on/off)
	uint8_t note_no;  // note number
	uint8_t mid_ch;   // midi channel
} ymz294_ch_stat_t;
#pragma pack()


static MIDI_PlayTuning_t _play_tuning[MAX_MIDI_CH_NUMBER];

static ymz294_ch_stat_t _ch_stat[NUM_OF_YMZ294_CHANNEL] = 
{
	{YMZ294_NOTE_OFF, 0, 0},// CHANNEL A
	{YMZ294_NOTE_OFF, 0, 0},// CHANNEL B
	{YMZ294_NOTE_OFF, 0, 0} // CHANNEL C
};


static const uint16_t _note_tp_tbl[128] = 
{
	0x3B8C, 0x3820, 0x3513, 0x3257, 0x2F68, 0x2CCC, 0x2A18, 0x2806,
	0x258F, 0x2362, 0x2172, 0x1FB5, 0x1DC6, 0x1C39, 0x1A89, 0x192B,
	0x17B4, 0x1666, 0x1523, 0x13EE, 0x12C8, 0x11C1, 0x10C8, 0x0FCD,
	0x0EEF, 0x0E1D, 0x0D4E, 0x0C8D, 0x0BDA, 0x0B2C, 0x0A92, 0x09F7,
	0x0968, 0x08E1, 0x0860, 0x07EA, 0x0777, 0x070C, 0x06A7, 0x0647,
	0x05ED, 0x0598, 0x0547, 0x04FC, 0x04B4, 0x0470, 0x0431, 0x03F4,
	0x03BC, 0x0386, 0x0353, 0x0323, 0x02F6, 0x02CC, 0x02A4, 0x027E,
	0x025A, 0x0238, 0x0218, 0x01FA, 0x01DE, 0x01C3, 0x01AA, 0x0192,
	0x017B, 0x0166, 0x0152, 0x013F, 0x012D, 0x011C, 0x010C, 0x00FD,
	0x00EF, 0x00E1, 0x00D5, 0x00C9, 0x00BE, 0x00B3, 0x00A9, 0x009F,
	0x0096, 0x008E, 0x0086, 0x007F, 0x0077, 0x0071, 0x006A, 0x0064,
	0x005F, 0x0059, 0x0054, 0x0050, 0x004B, 0x0047, 0x0043, 0x003F,
	0x003C, 0x0038, 0x0035, 0x0032, 0x002F, 0x002D, 0x002A, 0x0028,
	0x0026, 0x0024, 0x0022, 0x0020, 0x001E, 0x001C, 0x001B, 0x0019,
	0x0018, 0x0016, 0x0015, 0x0014, 0x0013, 0x0012, 0x0011, 0x0010,
	0x000F, 0x000E, 0x000D, 0x000D, 0x000C, 0x000B, 0x000B, 0x000A,
};

static void _ymz294_NoteOff(uint8_t ch, uint8_t kk, uint8_t uu);
static void _ymz294_NoteOn(uint8_t ch, uint8_t kk, uint8_t vv);
//static void _ymz294_PolyphonicKeyPressure(uint8_t ch, uint8_t kk, uint8_t vv);
static void _ymz294_ControlChange(uint8_t ch, uint8_t cc, uint8_t vv);
//static void _ymz294_ProgramChange(uint8_t ch, uint8_t pp);
//static void _ymz294_ChannelPressure(uint8_t ch, uint8_t vv);
static void _ymz294_PitchBendChange(uint8_t ch, uint8_t ll, uint8_t hh);

static const MIDI_Message_Callbacks_t _ymz294_midi_msg_callbacks =
{
	// MIDI_ChannelMessage_t
	{
		// MIDI_ChannelVoiceMessage_t
		{
			_ymz294_NoteOff,
			_ymz294_NoteOn,
			NULL,
			_ymz294_ControlChange,
			NULL,
			NULL,
			_ymz294_PitchBendChange
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


static uint8_t mixer_value = 0x0000;
static uint16_t env_frq_value = 0x0000;
static uint16_t noise_frq_value = 0x0000;

MIDI_Handle_t *midi_ymz294_init(void) {

	MIDI_Handle_t *phMIDI = NULL;
	uint32_t i = 0;

	ymz294_init();

	// set mixer
	mixer_value = 0x38;
	ymz294_write(0x07, mixer_value);
	// set volume
	ymz294_write(0x08, 0x00);
	ymz294_write(0x09, 0x00);
	ymz294_write(0x0A, 0x00);

	for ( i = 0; i < MAX_MIDI_CH_NUMBER; i++ )
	{
		_play_tuning[i].ymz294_setting.ch_enabled	= YMZ294_CH_ENABLED_TRUE;
		_play_tuning[i].ymz294_setting.env_freq 	= 0x1E85;// 1 Hz
		_play_tuning[i].ymz294_setting.env_mode 	= YMZ294_ENVELOPE_DISABLE;
		_play_tuning[i].ymz294_setting.env_shape	= 0x09;
		_play_tuning[i].ymz294_setting.sel_mixer	= YMZ294_MIXER_TONE;
		_play_tuning[i].ymz294_setting.noise_freq	= 0;
	}
	_play_tuning[9].ymz294_setting.ch_enabled 	= YMZ294_CH_ENABLED_FALSE;// noise channel (default disable)
	_play_tuning[9].ymz294_setting.sel_mixer  	= YMZ294_MIXER_NOISE;
	_play_tuning[9].ymz294_setting.env_mode 	= YMZ294_ENVELOPE_ENABLE;

#if 0
	_play_tuning[0].ymz294_setting.env_mode 	= YMZ294_ENVELOPE_ENABLE;
	_play_tuning[0].ymz294_setting.sel_mixer	= YMZ294_MIXER_TONE;
	_play_tuning[0].ymz294_setting.env_freq 	= 0x3D09; // 0.5 Hz
	_play_tuning[0].ymz294_setting.env_shape 	= 0x09;

	_play_tuning[1].ymz294_setting.sel_mixer	= YMZ294_MIXER_TONE;
	_play_tuning[1].ymz294_setting.env_mode 	= YMZ294_ENVELOPE_DISABLE;

	_play_tuning[2].ymz294_setting.sel_mixer	= YMZ294_MIXER_TONE;
	_play_tuning[2].ymz294_setting.env_mode 	= YMZ294_ENVELOPE_DISABLE;

	_play_tuning[3].ymz294_setting.sel_mixer	= YMZ294_MIXER_TONE;
	_play_tuning[3].ymz294_setting.env_mode 	= YMZ294_ENVELOPE_ENABLE;
	_play_tuning[3].ymz294_setting.env_freq 	= 0x0F42; // 2 Hz
	_play_tuning[3].ymz294_setting.env_shape 	= 0x09;

	_play_tuning[4].ymz294_setting.sel_mixer	= YMZ294_MIXER_TONE;
	_play_tuning[4].ymz294_setting.env_mode 	= YMZ294_ENVELOPE_DISABLE;

	_play_tuning[5].ymz294_setting.sel_mixer 	= YMZ294_MIXER_TONE;
	_play_tuning[5].ymz294_setting.env_mode 	= YMZ294_ENVELOPE_DISABLE;
#endif

	phMIDI = MIDI_Init(&_ymz294_midi_msg_callbacks);

	return phMIDI;
}

void midi_ymz294_deinit(MIDI_Handle_t *phMIDI) {

	MIDI_DeInit(phMIDI);
}


int32_t set_ymz294_setting(uint8_t midi_ch, const ymz294_setting_t *p_setting)
{
	if ( p_setting == (const ymz294_setting_t *)0 )
	{
		return -1;
	}
	else if ( MAX_MIDI_CH_NUMBER <= midi_ch )
	{
		return -2;
	}
	else
	{
		_play_tuning[midi_ch].ymz294_setting.ch_enabled = p_setting->ch_enabled;
		_play_tuning[midi_ch].ymz294_setting.sel_mixer 	= p_setting->sel_mixer;
		_play_tuning[midi_ch].ymz294_setting.env_mode	= p_setting->env_mode;
		_play_tuning[midi_ch].ymz294_setting.env_shape	= p_setting->env_shape;
		_play_tuning[midi_ch].ymz294_setting.env_freq	= p_setting->env_freq;
		_play_tuning[midi_ch].ymz294_setting.noise_freq	= p_setting->noise_freq;

		return 0;
	}
}

int32_t get_ymz294_setting(uint8_t midi_ch, ymz294_setting_t *dest_buf)
{
	if ( dest_buf == (const ymz294_setting_t *)0 )
	{
		return -1;
	}
	else if ( MAX_MIDI_CH_NUMBER <= midi_ch )
	{
		return -2;
	}
	else
	{
		dest_buf->ch_enabled	= _play_tuning[midi_ch].ymz294_setting.ch_enabled; 
		dest_buf->sel_mixer		= _play_tuning[midi_ch].ymz294_setting.sel_mixer; 
		dest_buf->env_mode		= _play_tuning[midi_ch].ymz294_setting.env_mode;
		dest_buf->env_shape		= _play_tuning[midi_ch].ymz294_setting.env_shape;
		dest_buf->env_freq		= _play_tuning[midi_ch].ymz294_setting.env_freq;
		dest_buf->noise_freq	= _play_tuning[midi_ch].ymz294_setting.noise_freq;

		return 0;
	}
}

static void _ymz294_NoteOff(uint8_t ch, uint8_t kk, uint8_t uu)
{
	uint32_t i = 0;
	for ( i = 0; i < NUM_OF_YMZ294_CHANNEL; i++ )
	{
		if (( _ch_stat[i].key_stat  == YMZ294_NOTE_ON )
		&&  ( _ch_stat[i].note_no  == kk )
		&&  ( _ch_stat[i].mid_ch == ch ))
		{
			// note off
			ymz294_write(0x08 + i, 0);
			
			// update a channel status.
			_ch_stat[i].key_stat = YMZ294_NOTE_OFF;
			break;
		}
	}
}


static void key_on(uint8_t ymz294_ch, uint8_t midi_ch, uint8_t kk, uint8_t vv)
{
	// set mixer
	{
		uint8_t mixer_value_tmp = mixer_value;
		if ( _play_tuning[midi_ch].ymz294_setting.sel_mixer == YMZ294_MIXER_TONE )
		{
			mixer_value_tmp |=	 1 << (ymz294_ch + 3);
			mixer_value_tmp &= ~(1 <<  ymz294_ch);
			// set TP
			ymz294_write(2*ymz294_ch+1, (_note_tp_tbl[kk]>>8) & 0x00FFU);
			ymz294_write(2*ymz294_ch, _note_tp_tbl[kk] & 0x00FFU);
		}
		else
		{
			mixer_value_tmp |=	 1 <<  ymz294_ch;
			mixer_value_tmp &= ~(1 << (ymz294_ch + 3));
			// set NP
			if ( noise_frq_value != _play_tuning[midi_ch].ymz294_setting.noise_freq )
			{
				noise_frq_value = _play_tuning[midi_ch].ymz294_setting.noise_freq;
				ymz294_write(0x06, noise_frq_value);
			}
		}

		if ( mixer_value != mixer_value_tmp )
		{
			mixer_value = mixer_value_tmp;
			ymz294_write(0x07, mixer_value);
		}
	}
	// set volume
	{
		if ( _play_tuning[midi_ch].ymz294_setting.env_mode == YMZ294_ENVELOPE_ENABLE )
		{
			// set envelope frequency
			{
				if ( env_frq_value != _play_tuning[midi_ch].ymz294_setting.env_freq )
				{
					env_frq_value = _play_tuning[midi_ch].ymz294_setting.env_freq;
					ymz294_write(0x0B,  env_frq_value       & 0x00FF);
					ymz294_write(0x0C, (env_frq_value >> 8) & 0x00FF);
				}
			}
			ymz294_write(0x08 + ymz294_ch, 0x10);
			ymz294_write(0x0D, _play_tuning[midi_ch].ymz294_setting.env_shape);
		}
		else
		{
			uint8_t level = 0;
			float fvelocity = 0.0F;
			fvelocity  = ((float)(_play_tuning[midi_ch].Expression & 0x7F))/127.0F;
			level = (uint8_t)((float)15 * fvelocity);
			// set volume
			ymz294_write(0x08 + ymz294_ch, level);
		}
	}
}


static void _ymz294_NoteOn(uint8_t ch, uint8_t kk, uint8_t vv)
{
	uint32_t i = 0;

	if ( _play_tuning[ch].ymz294_setting.ch_enabled == YMZ294_CH_ENABLED_FALSE )
	{
		return;
	}
	if ( vv != 0 )
	{// note on
		for ( i = 0; i < NUM_OF_YMZ294_CHANNEL; i++ )
		{
			if ( _ch_stat[i].key_stat == YMZ294_NOTE_OFF )
			{// note on

				key_on(i, ch, kk, vv);

				// update a channel status.
				_ch_stat[i].key_stat = YMZ294_NOTE_ON;
				_ch_stat[i].note_no = kk;
				_ch_stat[i].mid_ch = ch;
				break;
			}
		}
	}
	else
	{// note off
		_ymz294_NoteOff(ch, kk, vv);
	}
}

static void _ymz294_ControlChange(uint8_t ch, uint8_t cc, uint8_t vv)
{
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
			//TODO
		}
		break;

		case 11:// Expression
		{
			int i = 0;
			float fvelocity = 0.0F;
			uint8_t level = 0;
			_play_tuning[ch].Expression = vv;
			fvelocity  = ((float)(vv & 0x7F))/127.0F;

			level = (uint8_t)((float)15 * fvelocity);

			// note on
			for ( i = 0; i < NUM_OF_YMZ294_CHANNEL; i++ )
			{
				if (( _ch_stat[i].mid_ch == ch )
				&&  ( _ch_stat[i].key_stat == YMZ294_NOTE_ON))
				{// note on
					// set TP
					ymz294_write(0x08 + i, level);
				}
			}
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
		/*FALLTHROUGH*/
		case 121:// Reset All Controller
		/*FALLTHROUGH*/
		case 123:// All Note Off
		{
			uint32_t i = 0;
			for ( i = 0; i < NUM_OF_YMZ294_CHANNEL; i++ )
			{
				ymz294_write(0x08 + i, 0); // note off
				_ch_stat[i].key_stat = YMZ294_NOTE_OFF;
			}
			// RPN
			_play_tuning[ch].RPN.MSB = 0x7F;
			_play_tuning[ch].RPN.LSB = 0x7F;
			_play_tuning[ch].DAT.MSB = 0x00;
			_play_tuning[ch].DAT.LSB = 0x00;
			_play_tuning[ch].PitchBendSensitibity = 2;
			_play_tuning[ch].Expression = 0x7F;
			_play_tuning[ch].ChannelVolume = 64;
		}
		break;

		default:
		break;
	}

}

static void _ymz294_PitchBendChange(uint8_t ch, uint8_t ll, uint8_t hh)
{
	uint32_t i = 0;
	uint16_t changed_note = 0;
	uint16_t shift = 0;
	float fratio = 0.0F;

	shift = (hh << 7) | ll;
	fratio = (float)pow(2.0, (double)_play_tuning[ch].PitchBendSensitibity / 12.0 * (double)(shift-8192) / 8192.0 * -1); // @note * The reason for multiplying by -1 is that the frequency is inversely proportional to Tp.

	for ( i = 0; i < NUM_OF_YMZ294_CHANNEL; i++ )
	{
		if (( _ch_stat[i].mid_ch == ch )
		&&  ( _ch_stat[i].key_stat == YMZ294_NOTE_ON))
		{
			changed_note = _note_tp_tbl[_ch_stat[i].note_no] * fratio;
			// set TP
			ymz294_write(2*i+1, (changed_note>>8) & 0x00FFU);
			ymz294_write(2*i,	changed_note	 & 0x00FFU);
		}
	}
}
