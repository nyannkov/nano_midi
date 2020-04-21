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
#include "mshell.h"
#include "mshell_cmd.h"
#include "ymf825.h"


#define MAX_COMMAND_ARG_NUM				(10)

#define MAX_STRING_LENGTH_SIZE			(1024)
#define MAX_BYTE_ARRAY_SIZE				(MAX_STRING_LENGTH_SIZE/2)

typedef struct _CommandArgs
{
	int argc;
	char *argv[MAX_COMMAND_ARG_NUM];
}CommandArgs_t;

typedef struct _CharArray
{
	size_t 		length;	
	uint8_t	 	string[MAX_STRING_LENGTH_SIZE];
}CharArray_t;

typedef struct _ByteArray
{
	size_t 		num;
	uint8_t		bytes[MAX_BYTE_ARRAY_SIZE];
}ByteArray_t;

static CommandArgs_t	_command_args;
static CharArray_t 		_char_array;
static ByteArray_t		_byte_array;

static pf_mshell_hexmode_recv_callback_t _hexmode_recv_callback = (pf_mshell_hexmode_recv_callback_t)0;


static int32_t parse_bin_array(ByteArray_t *p_byte_array, const CharArray_t *p_char_array);
static int32_t parse_command_args(CommandArgs_t *args, CharArray_t *p_char_array);

static inline int8_t parse_hex_number(uint8_t chex)
{

	int8_t hex = 0;

	if ( ( '0' <= chex ) && ( chex <= '9' ) )
	{
		hex = chex - (int8_t)'0';
	}
    else
    {
        chex |= 0x20; // case-insensitive
        if ( ( 'a' <= chex ) && ( chex <= 'f' ) )
		{
            hex = chex - (int8_t)'a' + 0x0A;
        }
        else
        {
            return -1;
        }
	}
	return hex;
}


void mshell_init(void)
{

	int32_t i = 0;

	for ( i = 0; i < sizeof(_command_args); i++ )
	{
		((uint8_t *)&_command_args)[i] = 0;
	}
	for ( i = 0; i < sizeof(_char_array); i++ )
	{
		((uint8_t *)&_char_array)[i] = 0;
	}
	for ( i = 0; i < sizeof(_byte_array); i++ )
	{
		((uint8_t *)&_byte_array)[i] = 0;
	}


	return;
}

void mshell_register_hexmode_recv_callback(const pf_mshell_hexmode_recv_callback_t callback) 
{
	_hexmode_recv_callback = callback;
}

int32_t mshell_proc(const uint8_t *recv_dat, size_t recv_len)
{

	uint32_t i = 0;
	uint8_t * p_str = (uint8_t *)0;
	int32_t cmd_proc_result = 0;

	if ( recv_dat == (uint8_t *)0 )
 	{
		cmd_proc_result = -1;
	}
	else if ( _char_array.length + recv_len >= MAX_STRING_LENGTH_SIZE )
	{
		cmd_proc_result = -2;
	}
	else
	{
		p_str = &_char_array.string[_char_array.length];
		for ( i = 0; i < recv_len; i++ )
		{
			if ( recv_dat[i] == '\n' )
			{
				continue;
			}
			else if ( recv_dat[i] == '\r' )
			{
				p_str[i] = '\0'; 
                if ( _char_array.string[0] == ':')
				{// command mode
					parse_command_args(&_command_args, &_char_array);
					mshell_execute_command(_command_args.argc, _command_args.argv);
                }
                else
				{// hex mode
					int32_t parse_result = -1;
                    parse_result = parse_bin_array(&_byte_array, &_char_array);	
                    if ( !parse_result )
					{
                        if ( _hexmode_recv_callback )
                        {
                            _hexmode_recv_callback(_byte_array.bytes, _byte_array.num);
                        }
                    }
                }
				mshell_init();
				p_str = &_char_array.string[0];
			}
			else
			{
				p_str[i] = recv_dat[i];
				_char_array.length++;
			}
		}
	}

	if ( cmd_proc_result != 0 )
	{
		mshell_init();
	}

	return 0;
}

static int32_t parse_bin_array(ByteArray_t *p_byte_array, const CharArray_t *p_char_array)
{

	uint32_t i = 0;
	int8_t 	hex = 0;
	uint8_t *p_bytes = (uint8_t *)0;

	if ( (p_char_array->length % 2) != 0 )
	{ 
		return (-1);
	}
	if ( p_char_array->length == 0 )
	{
		return (-2);
	}
	
	p_bytes = &p_byte_array->bytes[0];
	p_byte_array->num = 0;
	for ( i = 0; i < p_char_array->length; i++ )
	{
		hex = parse_hex_number(p_char_array->string[i]);
		if ( hex < 0 )
		{
			return (-3);
		}
		if ( (i % 2) == 0 )
		{ 
			*p_bytes  = hex << 4;
		}
		else
		{
			*p_bytes |= hex;
			p_byte_array->num++;
			p_bytes++;
		}
	}

	return 0;
}

static int32_t parse_command_args(CommandArgs_t *args, CharArray_t *p_char_array)
{
	uint32_t i = 0;
	enum
	{
		PARSE_ARGS_STATE_FIND_ARG_TAIL,
		PARSE_ARGS_STATE_FIND_ARG_HEAD,
	} parse_args_state = PARSE_ARGS_STATE_FIND_ARG_HEAD;

	args->argc = 0;

	for ( i = 0; i < p_char_array->length; i++ )
	{
		if ( ( p_char_array->string[i] == ' ' ) || ( p_char_array->string[i] == '\t' ) )
		{
			if ( parse_args_state == PARSE_ARGS_STATE_FIND_ARG_TAIL )
			{
				p_char_array->string[i] = '\0';// replace with null character.
				parse_args_state = PARSE_ARGS_STATE_FIND_ARG_HEAD;
			}
			else
			{// ignore.
			}
		}
		else if ( p_char_array->string[i] != '\0' )
		{
			if ( parse_args_state == PARSE_ARGS_STATE_FIND_ARG_HEAD )
			{
				args->argv[args->argc++] = (char *)&p_char_array->string[i];// store argument head
				parse_args_state = PARSE_ARGS_STATE_FIND_ARG_TAIL;

				if ( args->argc > MAX_COMMAND_ARG_NUM-1 )
				{
					return -1;
				}
			}
		}
		else
		{
			break;
		}
	}

	return 0;
}
