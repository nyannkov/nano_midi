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
#include <string.h> //strcmp.h
#include "mshell_conf.h"
#include "mshell_cmd.h"
#include "midi_cdc_core.h"
#include "usb_cdc_app.h"


typedef struct
{
	const char *label;
	int (*command)(int argc, char *argv[]);
} command_table_t;

static int cmd_koncha(int argc, char *argv[]);
static int cmd_test(int argc, char *argv[]);
static int cmd_hexmode(int argc, char *argv[]);

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
	}
};

static const size_t n_default_command_table = sizeof(default_command_table) / sizeof(default_command_table[0]);

__WEAK__ int mshell_execute_command(int argc, char *argv[])
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