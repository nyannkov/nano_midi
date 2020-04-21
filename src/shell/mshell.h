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
#ifndef __MSHELL_H__
#define __MSHELL_H__

#include "mshell_conf.h"

// receive-callback in hex mode.
typedef int32_t (*pf_mshell_hexmode_recv_callback_t)(const uint8_t *bin_array, size_t bin_len);

extern void mshell_init(void);
extern int32_t mshell_proc(const uint8_t *recv_dat, size_t recv_len); 
extern void mshell_register_hexmode_recv_callback(const pf_mshell_hexmode_recv_callback_t callback); 

#endif // __MSHELL_H__
