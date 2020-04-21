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
#include "usb_cdc_app.h"
#include "mshell.h"
#include "usbd/midi_cdc_core.h"
#include "ymf825.h"

static int32_t send_recv_ymf825(const uint8_t *bin_array, size_t bin_len);

void init_usb_cdc_app(void)
{
	mshell_init();
	mshell_register_hexmode_recv_callback(send_recv_ymf825);
}

int32_t usb_cdc_proc(const uint8_t *data, size_t len)
{
	return mshell_proc(data, len);
}

static int32_t send_recv_ymf825(const uint8_t *bin_array, size_t bin_len)
{
	int32_t readflg = 0;
	uint8_t recvdat = 0;
	readflg = ( bin_array[0] & 0x80 ) == 0x80 ? 1 : 0;

	if (readflg == 0) 
	{
		if ( bin_len > 1) 
		{
			if_write(bin_array[0], &bin_array[1], bin_len-1);
		}
		else 
		{
			return -1;
		}
	}
	else 
	{
		recvdat = if_s_read(bin_array[0]);
		usb_cdc_printf("%02X\r\n", recvdat);
	}

	return 0;
}