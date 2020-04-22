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
#include "mshell_cmd.h"
#include "midi_cdc_core.h"


typedef struct
{
  const char *label;
  int (*command)(int argc, char *argv[]);
} command_table_t;

static int cmd_koncha(int argc, char *argv[]);
static int cmd_test(int argc, char *argv[]);

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
};

static const size_t n_default_command_table = sizeof(default_command_table) / sizeof(default_command_table[0]);

__attribute__((weak)) int mshell_execute_command(int argc, char *argv[])
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
