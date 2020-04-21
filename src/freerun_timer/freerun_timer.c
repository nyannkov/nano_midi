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
#include "freerun_timer.h"

void init_freerun_timer(void)
{
    // system_clock = 96 MHz
    timer_parameter_struct timer_initpara;

    timer_deinit(TIMER5);
    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);
    /* TIMER5 configuration */
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.prescaler         = 9599;// 96 MHz/ 9600 = 10 kHz
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 65535;

    timer_init(TIMER5, &timer_initpara);

    /* TIMER5 counter enable */
    timer_enable(TIMER5);
}


// unit : 100 usec
void delay100us(int16_t delay)
{
    int16_t time_mark = FREERUN_COUNTER_100US;
    while((int16_t)((int16_t)FREERUN_COUNTER_100US - time_mark) < delay);
}

