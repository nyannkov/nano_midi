﻿/*
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
#include <string.h> //strcmp
#include <stdlib.h> //strtoul
#include "mshell_conf.h"
#include "mshell_cmd.h"
#include "midi_cdc_core.h"
#include "usb_cdc_app.h"
#include "single_ymz294.h"


typedef struct
{
	const char *label;
	int (*command)(int argc, char *argv[]);
} command_table_t;

#ifdef USE_SINGLE_YMZ294

#define PARSE_ERROR_INVALID_CHARACTER 	-1
#define PARSE_ERROR_OUT_OF_RANGE		-2
#define PARSE_ERROR_UNEXPECTED			-100

static int32_t try_parse_ymz294_setting(uint8_t opt_idx, const char *str, ymz294_setting_t *out);
static int32_t try_parse_uint32(const char *str, uint32_t *out, uint32_t min, uint32_t max);
#endif

static int cmd_koncha(int argc, char *argv[]);
static int cmd_test(int argc, char *argv[]);
static int cmd_hexmode(int argc, char *argv[]);
#ifdef USE_SINGLE_YMZ294
static int cmd_ymz294(int argc, char *argv[]);
#endif

static const command_table_t default_command_table[] =
{
	{
		 .label = ":koncha",
		 .command = cmd_koncha,
	},
	{
		 .label = ":test",
		 .command = cmd_test,
	},
	{
		 .label = ":hexmode",
		 .command = cmd_hexmode,
	},
#ifdef USE_SINGLE_YMZ294
	{
		 .label = ":ymz294",
		 .command = cmd_ymz294,
	}
#endif
};

static const size_t n_default_command_table = sizeof(default_command_table) / sizeof(default_command_table[0]);

int mshell_execute_command(int argc, char *argv[])
{
	int i = 0;
	int cmd_ret = 0;

	if ( ( argc <= 0 ) || ( argv == (char **)0 ) )
	{
		return -1;
	}

	for ( i = 0; i < n_default_command_table; i++ )
	{
		if ( !strcmp(argv[0], default_command_table[i].label) )
		{
			if ( default_command_table[i].command )
			{
				cmd_ret = default_command_table[i].command(argc, argv);
				break;
			}
		}
	}

	return cmd_ret;
}

static int cmd_koncha(int argc, char *argv[])
{
	usb_cdc_printf("Hi! I'm nano_midi :).\r\n");
	return 0;
}


static int cmd_test(int argc, char *argv[])
{
	int i = 0;
	usb_cdc_printf("argc = %d\r\n", argc);
	for ( i = 0; i < argc; i++ )
	{
		usb_cdc_printf("argv[%d] = %s\r\n", i, argv[i]);
	}
	return 0;
}


static int cmd_hexmode(int argc, char *argv[])
{

	int32_t set_result = -100;
	sound_source_t registered_source = SOUND_SOURCE_YMF825;
	const char *sound_source_name = (const char *)0;

	if ( argv[1] )
	{
		if ( !strcmp(argv[1], "ymf825") )
		{
			set_result = set_hexmode_sound_source(SOUND_SOURCE_YMF825);
		}
#ifdef USE_SINGLE_YMZ294
		else if ( !strcmp(argv[1], "ymz294") )
		{
			set_result = set_hexmode_sound_source(SOUND_SOURCE_YMZ294);
		}
#endif
		else
		{
			set_result = -1;
		}
		
		if ( set_result == 0 )
		{
			usb_cdc_printf("SUCCESS\r\n");
		}
		else
		{
			usb_cdc_printf("FAILED(%d)\r\n", set_result);
		}
	}

	registered_source = get_hexmode_sound_source();
	switch ( registered_source )
	{
		case SOUND_SOURCE_YMF825 :
		{
			sound_source_name = "YMF825";
		}
		break;

		case SOUND_SOURCE_YMZ294 :
		{
			sound_source_name = "YMZ294";
		}
		break;

		default:
		{
			sound_source_name = "UNKNOWN";
		}
		break;
	}
	usb_cdc_printf("SOUND SOURCE: %s\r\n", sound_source_name);

	return 0;
}

#ifdef USE_SINGLE_YMZ294


static const struct ymz294_cmd_opts
{
	const char *opt;
	const char *brief;
} ymz294_cmd_opt[7] = 
{
	{"-ls",  "Show current setting"},
	{"-ch",  "MIDI channel to change setting [0-15]"},
	{"-mx",  "MIXER setting [ tone | noise ]"},
	{"-em",  "Envelope mode [ off | on(use envelop)]"},
	{"-es",  "Envelope shape [0x0-0xF]"},
	{"-ep",  "Envelope frequency (EP) [0x0000-0xFFFF]"},
	{"-np",  "Noise frequency (NP) [0x00-0x1F]"}
};
static const uint8_t n_ymz294_cmd_opt = sizeof(ymz294_cmd_opt)/sizeof(ymz294_cmd_opt[0]);

static int cmd_ymz294(int argc, char *argv[])
{
	static ymz294_setting_t setting;

	if ( argc <= 1)
	{// usage
		uint8_t idx = 0;
		for ( idx = 0; idx < n_ymz294_cmd_opt; idx++ )
		{
			usb_cdc_printf("%s\t%s\r\n", ymz294_cmd_opt[idx].opt, ymz294_cmd_opt[idx].brief);
		}
	}
	else if ( !strcmp(argv[1], "-ls" ) )
	{// show current setting
		uint8_t ch = 0;
		usb_cdc_printf("ch\tmx\tem\tes\tep\tnp\r\n");
		for ( ch = 0; ch < MAX_MIDI_CH_NUMBER; ch++ )
		{
			get_ymz294_setting(ch, &setting);
			usb_cdc_printf("%02u\t%s\t%s\t0x%02X\t0x%04X\t0x%02X\r\n", 
				ch,
				setting.sel_mixer == YMZ294_MIXER_TONE ? "tone" : "noise",
				setting.env_mode == YMZ294_ENVELOPE_ENABLE ? "on" : "off",
				setting.env_shape,
				setting.env_freq,
				setting.noise_freq
			);
		}
	}
	else if ( !strcmp(argv[1], "-ch"))
	{// change current setting of selected channel.
		int i = 0;
		int32_t parse_result = 0;
		uint32_t setting_ch = 0xFF;

		if ( !argv[2] )
		{
			usb_cdc_printf("Channel number not found\r\n");
		}
		else
		{
			parse_result = try_parse_uint32(argv[2], &setting_ch, 0, MAX_MIDI_CH_NUMBER-1);
			if ( parse_result != 0 )
			{
				if ( parse_result == PARSE_ERROR_OUT_OF_RANGE )
				{
					usb_cdc_printf("Option '-ch': '%s' is out of range\r\n", argv[2]);
				}
				else if ( parse_result == PARSE_ERROR_INVALID_CHARACTER )
				{
					usb_cdc_printf("Option '-ch': Could not parse '%s'\r\n", argv[2]);
				}
				else
				{
					usb_cdc_printf("Option '-ch': Internal error\r\n");
				}
			}
			else
			{
				uint8_t process_result = 0; 
				// get current ymz294 setting value.
				get_ymz294_setting(setting_ch, &setting);

				// decrement if argc is even number.
				if (!(argc & 0x01))
				{
					argc--;
				}

				for ( i = 3; i < argc; i+= 2)
				{ 
					uint8_t find = 0;
					uint8_t idx = 0;
					uint8_t hit_idx = 0;
					for ( idx = 2; idx < n_ymz294_cmd_opt; idx++ )
					{// search option (Except -ls and -ch)
						if ( !strcmp(argv[i], ymz294_cmd_opt[idx].opt))
						{
							find = 1;
							hit_idx = idx;
							break;
						}
					}

					if ( !find )
					{
						usb_cdc_printf("Option '%s' not found\r\n", argv[i]);
						process_result = -1;
						break;
					}
					else
					{
						parse_result = try_parse_ymz294_setting(hit_idx, argv[i+1], &setting);
						if ( parse_result != 0 )
						{
							if ( parse_result == PARSE_ERROR_OUT_OF_RANGE )
							{
								usb_cdc_printf("Option '%s': '%s' is out of range\r\n", ymz294_cmd_opt[hit_idx].opt, argv[2]);
							}
							else if ( parse_result == PARSE_ERROR_INVALID_CHARACTER )
							{
								usb_cdc_printf("Option '%s': Could not parse '%s'\r\n", ymz294_cmd_opt[hit_idx].opt, argv[i+1]);
							}
							else
							{
								usb_cdc_printf("Option '%s': Internal error\r\n", ymz294_cmd_opt[hit_idx].opt);
							}
							process_result = -1;
							break;
						}
						else
						{// next option
							process_result = 0;
							continue;
						}
					}
				}

				if ( process_result == 0 )
				{
					set_ymz294_setting(setting_ch, &setting);
					usb_cdc_printf("ch\tmx\tem\tes\tep\tnp\r\n");
					usb_cdc_printf("%02u\t%s\t%s\t0x%02X\t0x%04X\t0x%02X\r\n", 
						setting_ch,
						setting.sel_mixer == YMZ294_MIXER_TONE ? "tone" : "noise",
						setting.env_mode == YMZ294_ENVELOPE_ENABLE ? "on" : "off",
						setting.env_shape,
						setting.env_freq,
						setting.noise_freq
					);
				}
			}
		}
	}
	else
	{
		// TODO
		usb_cdc_printf("usage:\r\n");
	}
	

	return 0;
}

static int32_t try_parse_uint32(const char *str, uint32_t *out, uint32_t min, uint32_t max)
{
	unsigned long value = 0;
	char *endptr = "\255";
	value = strtoul(str, &endptr, 0);
	if ( *endptr == '\0' )
	{
		if ( min < max )
		{// range check
			if ( ( value < min ) || ( max < value ) )
			{// range error
				return PARSE_ERROR_OUT_OF_RANGE;
			}
		}
	}
	else
	{// parse error
		return PARSE_ERROR_INVALID_CHARACTER;
	}

	*out = value;

	return 0;
}

static int32_t try_parse_ymz294_setting(uint8_t opt_idx, const char *str, ymz294_setting_t *out)
{
	uint32_t value = 0;
	int32_t result = 0;
	switch (opt_idx)
	{
		case 2:
		{// -mx
			if (!strcmp(str, "tone"))
			{
				out->sel_mixer = YMZ294_MIXER_TONE;
			}
			else if ( !strcmp(str, "noise"))
			{
				out->sel_mixer = YMZ294_MIXER_NOISE;
			}
			else
			{
				result = PARSE_ERROR_INVALID_CHARACTER;
			}
		}
		break;

		case 3:
		{// -em
			if (!strcmp(str, "on"))
			{
				out->env_mode = YMZ294_ENVELOPE_ENABLE;
			}
			else if ( !strcmp(str, "off"))
			{
				out->env_mode = YMZ294_ENVELOPE_DISABLE;
			}
			else
			{
				result = PARSE_ERROR_INVALID_CHARACTER;
			}
		}
		break;

		case 4:
		{// -es
			result = try_parse_uint32(str, &value, 0x0, 0xF);
			if ( result == 0 )
			{
				out->env_shape = value;
			}
		}
		break;

		case 5:
		{// -ep
			result = try_parse_uint32(str, &value, 0x0000, 0xFFFF);
			if ( result == 0 )
			{
				out->env_freq = value;
			}
		}
		break;
		
		case 6:
		{// -np
			result = try_parse_uint32(str, &value, 0x00, 0x1F);
			if ( result == 0 )
			{
				out->noise_freq = value;
			}
		}
		break;

		default:
		{
			result = PARSE_ERROR_UNEXPECTED;
		}
		break;
	}

	return result;
}


#endif

