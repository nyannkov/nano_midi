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
#include <string.h> //strcmp
#include <stdlib.h> //strtoul
#include "main.h"
#include "mshell_conf.h"
#include "mshell_cmd.h"
#include "../usbd_core/midi_cdc_core.h"
#include "usb_cdc_app.h"
#include "usb_midi_app.h"
#include "single_ymz294.h"
#include "music_box_ymf825.h"


typedef struct
{
	const char *label;
	int (*command)(int argc, char *argv[]);
	const char *brief;
} command_table_t;

#ifdef USE_SINGLE_YMZ294

#define PARSE_ERROR_INVALID_CHARACTER 	-1
#define PARSE_ERROR_OUT_OF_RANGE		-2
#define PARSE_ERROR_UNEXPECTED			-100

typedef enum
{
	YMZ294_CMD_OPT_ID_LS = 0,
	YMZ294_CMD_OPT_ID_CH,
	YMZ294_CMD_OPT_ID_EN,
	YMZ294_CMD_OPT_ID_MX,
	YMZ294_CMD_OPT_ID_EM,
	YMZ294_CMD_OPT_ID_ES,
	YMZ294_CMD_OPT_ID_EP,
	YMZ294_CMD_OPT_ID_NP,
}ymz294_cmd_opt_id_t;

static int32_t try_parse_ymz294_setting(ymz294_cmd_opt_id_t opt_id, const char *str, ymz294_setting_t *out);
static int32_t try_parse_uint32(const char *str, uint32_t *out, uint32_t min, uint32_t max);
#endif

static int cmd_koncha(int argc, char *argv[]);
static int cmd_test(int argc, char *argv[]);
static int cmd_hexmode(int argc, char *argv[]);
#ifdef USE_SINGLE_YMZ294
static int cmd_ymz294(int argc, char *argv[]);
#endif
static int cmd_switch(int argc, char *argv[]);
static int cmd_usage(int argc, char *argv[]);
static int cmd_ymf825(int argc, char *argv[]);

static const command_table_t command_table[] =
{
	{
		 .label = "koncha",
		 .command = cmd_koncha,
		 .brief = "Show current version."
	},
	{
		 .label = "test",
		 .command = cmd_test,
		 .brief = "Test of command args."
	},
	{
		 .label = "hexmode",
		 .command = cmd_hexmode,
		 .brief = "Select the sound IC to be connected in hex mode."
	},
#ifdef USE_SINGLE_YMZ294
	{
		 .label = "ymz294",
		 .command = cmd_ymz294,
		 .brief = "Set/Get the playing parameters of YMZ294."
	},
#endif
	{
		 .label = "switch",
		 .command = cmd_switch,
		 .brief = "Switch the sound driver used for MIDI playing, for each sound IC."
	},
	{
		 .label = "usage",
		 .command = cmd_usage,
		 .brief = "Show usage for each command."
	},
	{
		 .label = "ymf825",
		 .command = cmd_ymf825,
		 .brief = "Set/Get the playing parameters of YMF825."
	}
};

static const size_t n_command_table = sizeof(command_table) / sizeof(command_table[0]);

int mshell_execute_command(int argc, char *argv[])
{
	int i = 0;
	int cmd_ret = 0;

	if ( ( argc <= 0 ) || ( argv == (char **)0 ) )
	{
		return -1;
	}

	for ( i = 0; i < n_command_table; i++ )
	{
		if ( !strcmp(argv[0], command_table[i].label) )
		{
			if ( command_table[i].command )
			{
				cmd_ret = command_table[i].command(argc, argv);
				break;
			}
		}
	}

	return cmd_ret;
}

static int cmd_koncha(int argc, char *argv[])
{
	usb_cdc_printf("Hi! I'm nano_midi ver.%s :).\r\n", NANO_MIDI_VERSION);
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
} ymz294_cmd_opt[] = 
{
	{"-ls",  "Show current setting"},
	{"-ch",  "MIDI channel to change setting [0-15]"},
	{"-en",  "Channel enabled [ true | false ]"},
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
		usb_cdc_printf("ch\ten\tmx\tem\tes\tep\tnp\r\n");
		for ( ch = 0; ch < MAX_MIDI_CH_NUMBER; ch++ )
		{
			get_ymz294_setting(ch, &setting);
			usb_cdc_printf("%02u\t%s\t%s\t%s\t0x%02X\t0x%04X\t0x%02X\r\n", 
				ch,
				setting.ch_enabled == YMZ294_CH_ENABLED_FALSE ? "false" : "true",
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
					usb_cdc_printf("ch\ten\tmx\tem\tes\tep\tnp\r\n");
					usb_cdc_printf("%02u\t%s\t%s\t%s\t0x%02X\t0x%04X\t0x%02X\r\n", 
						setting_ch,
						setting.ch_enabled == YMZ294_CH_ENABLED_FALSE ? "false" : "true",
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

static int32_t try_parse_ymz294_setting(ymz294_cmd_opt_id_t opt_id, const char *str, ymz294_setting_t *out)
{
	uint32_t value = 0;
	int32_t result = 0;
	switch (opt_id)
	{
		case YMZ294_CMD_OPT_ID_EN:
		{// -mx
			if (!strcmp(str, "false"))
			{
				out->ch_enabled = YMZ294_CH_ENABLED_FALSE;
			}
			else if ( !strcmp(str, "true"))
			{
				out->ch_enabled = YMZ294_CH_ENABLED_TRUE;
			}
			else
			{
				result = PARSE_ERROR_INVALID_CHARACTER;
			}
		}
		break;

		case YMZ294_CMD_OPT_ID_MX:
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

		case YMZ294_CMD_OPT_ID_EM:
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

		case YMZ294_CMD_OPT_ID_ES:
		{// -es
			result = try_parse_uint32(str, &value, 0x0, 0xF);
			if ( result == 0 )
			{
				out->env_shape = value;
			}
		}
		break;

		case YMZ294_CMD_OPT_ID_EP:
		{// -ep
			result = try_parse_uint32(str, &value, 0x0000, 0xFFFF);
			if ( result == 0 )
			{
				out->env_freq = value;
			}
		}
		break;
		
		case YMZ294_CMD_OPT_ID_NP:
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

static int cmd_switch(int argc, char *argv[])
{
	if ( argv[1] )
	{
		if ( !strcmp(argv[1], "ymf825") )
		{
			ymf825_sound_driver_t sound_driver = NUM_OF_YMF825_SOUND_DRIVER;
			const char *selected_driver_name = "";
			if ( argv[2] )
			{
				if ( !strcmp(argv[2], "mode4") )
				{
					switch_ymf825_sound_driver(YMF825_SOUND_DRIVER_MODE4);
				}
				else if ( !strcmp(argv[2], "mbox") )
				{
					switch_ymf825_sound_driver(YMF825_SOUND_DRIVER_MUSIC_BOX);
				}
				else
				{
				}
			}
			sound_driver = get_selected_ymf825_sound_driver();
			if ( sound_driver == YMF825_SOUND_DRIVER_MODE4 )
			{
				selected_driver_name = "mode4";
			}
			else if ( sound_driver == YMF825_SOUND_DRIVER_MUSIC_BOX )
			{
				selected_driver_name = "mbox";
			}
			else
			{
			}
			usb_cdc_printf("sound driver: %s\r\n", selected_driver_name);

		}
#ifdef USE_SINGLE_YMZ294
		else if ( !strcmp(argv[1], "ymz294") )
		{
			usb_cdc_printf("sound driver: single\r\n"); // Modify as needed.
		}
#endif
		else
		{
		}
	}

	return 0;
}

static int cmd_usage(int argc, char *argv[])
{
	uint32_t i = 0;
	uint32_t show_all = 0;
	const char *search_cmd = "";

	if ( !argv[1] )
	{
		show_all = 1;
		search_cmd = "";
	}
	else
	{
		show_all = 0;
		search_cmd = argv[1];
	}

	for ( i = 0; i < n_command_table; i++ )
	{
		if ( !strcmp(search_cmd, command_table[i].label) || show_all )
		{
			usb_cdc_printf("%s\t: %s\r\n", command_table[i].label, command_table[i].brief);
		}
	}

	return 0;
}

static int cmd_ymf825(int argc, char *argv[])
{
	ymf825_sound_driver_t sound_driver = NUM_OF_YMF825_SOUND_DRIVER;
	sound_driver = get_selected_ymf825_sound_driver();

	if ( sound_driver == YMF825_SOUND_DRIVER_MODE4 )
	{
		// Add code as needed.
	}
	else if ( sound_driver == YMF825_SOUND_DRIVER_MUSIC_BOX )
	{
		music_box_ymf825_config_t config;
 		GetConfig_MUSIC_BOX_YMF825(&config);
		if ( !strcmp(argv[1], "perc") )
		{
			if ( argv[2] )
			{
				if ( !strcmp(argv[2], "on") )
				{
					config.percussion_msg = MUSIC_BOX_YMF825_ACCEPT_PERCUSSION_MESSAGE;
					SetConfig_MUSIC_BOX_YMF825(&config);
				}
				else if ( !strcmp(argv[2], "on") )
				{
					config.percussion_msg = MUSIC_BOX_YMF825_IGNORE_PERCUSSION_MESSAGE;
					SetConfig_MUSIC_BOX_YMF825(&config);
				}
				else
				{
				}
			}
			usb_cdc_printf("Percussion: %s\r\n", config.percussion_msg == MUSIC_BOX_YMF825_IGNORE_PERCUSSION_MESSAGE ? "off" : "on" );
		}
		else if ( !strcmp(argv[1], "prog") )
		{
			if ( argv[2] )
			{
				uint32_t program_no = 0;
				program_no = strtoul(argv[2], NULL, 0);
				if ( 1 <= program_no && program_no <= 128 )
				{
					config.program_no = (uint8_t)program_no;
					SetConfig_MUSIC_BOX_YMF825(&config);
				}
			}
			usb_cdc_printf("Program No: %u\r\n", config.program_no);
		}
		else
		{
		}
	}
	else
	{
	}

	return 0;
}