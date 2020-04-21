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
#ifndef __YMF825_H__
#define __YMF825_H__

#include <stdint.h>
#include <stddef.h>

extern int32_t YMF825_Init(void);
extern void YMF825_DeInit(void);

extern void YMF825_SelectChannel(uint8_t ch);
extern void YMF825_SelectNoteNumber(uint16_t fnum, uint16_t block);
extern void YMF825_ChangeVoVol(uint8_t VoVol); 
extern void YMF825_ChangeChVol(uint8_t ChVol);
extern void YMF825_ChangeMASTER_VOL(uint8_t master_vol);
extern void YMF825_ChangePitch(uint16_t INT, uint16_t FRAC);
extern void YMF825_KeyOn(uint8_t tone_num);
extern void YMF825_KeyOff(uint8_t tone_num);
extern void YMF825_SetToneParameter(uint8_t tone_matrix[16][30]);

extern void if_write(uint8_t addr, const uint8_t* data, uint16_t size);
extern void if_s_write(uint8_t addr,uint8_t data);
extern uint8_t if_s_read(uint8_t addr);

#endif // __YMF825_H__
