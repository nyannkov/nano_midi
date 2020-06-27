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

#include "ymf825_note_table.h"

const MIDI_ToneTable_t _note_tbl[MAX_NOTE_NUM] = {
	{ 0, 89  }, // 8.2 Hz
	{ 0, 95  }, // 8.7 Hz
	{ 0, 100 }, // 9.2 Hz
	{ 0, 106 }, // 9.7 Hz
	{ 0, 113 }, // 10.3 Hz
	{ 0, 119 }, // 10.9 Hz
	{ 0, 126 }, // 11.6 Hz
	{ 0, 134 }, // 12.2 Hz
	{ 0, 142 }, // 13.0 Hz
	{ 0, 150 }, // 13.8 Hz
	{ 0, 159 }, // 14.6 Hz
	{ 0, 169 }, // 15.4 Hz
	{ 0, 179 }, // 16.4 Hz
	{ 0, 189 }, // 17.3 Hz
	{ 0, 200 }, // 18.4 Hz
	{ 0, 212 }, // 19.4 Hz
	{ 0, 225 }, // 20.6 Hz
	{ 0, 238 }, // 21.8 Hz
	{ 0, 253 }, // 23.1 Hz
	{ 0, 268 }, // 24.5 Hz
	{ 0, 284 }, // 26.0 Hz
	{ 0, 300 }, // 27.5 Hz
	{ 0, 318 }, // 29.1 Hz
	{ 0, 337 }, // 30.9 Hz
	{ 0, 357 }, // 32.7 Hz
	{ 1, 189 }, // 34.6 Hz
	{ 1, 200 }, // 36.7 Hz
	{ 1, 212 }, // 38.9 Hz
	{ 1, 225 }, // 41.2 Hz
	{ 1, 238 }, // 43.7 Hz
	{ 1, 253 }, // 46.2 Hz
	{ 1, 268 }, // 49.0 Hz
	{ 1, 284 }, // 51.9 Hz
	{ 1, 300 }, // 55.0 Hz
	{ 1, 318 }, // 58.3 Hz
	{ 1, 337 }, // 61.7 Hz
	{ 1, 357 }, // 65.4 Hz
	{ 2, 189 }, // 69.3 Hz
	{ 2, 200 }, // 73.4 Hz
	{ 2, 212 }, // 77.8 Hz
	{ 2, 225 }, // 82.4 Hz
	{ 2, 238 }, // 87.3 Hz
	{ 2, 253 }, // 92.5 Hz
	{ 2, 268 }, // 98.0 Hz
	{ 2, 284 }, // 103.8 Hz
	{ 2, 300 }, // 110.0 Hz
	{ 2, 318 }, // 116.5 Hz
	{ 2, 337 }, // 123.5 Hz
	{ 3, 179 }, // 130.8 Hz
	{ 3, 189 }, // 138.6 Hz
	{ 3, 200 }, // 146.8 Hz
	{ 3, 212 }, // 155.6 Hz
	{ 3, 225 }, // 164.8 Hz
	{ 3, 238 }, // 174.6 Hz
	{ 3, 253 }, // 185.0 Hz
	{ 3, 268 }, // 196.0 Hz
	{ 3, 284 }, // 207.7 Hz
	{ 3, 300 }, // 220.0 Hz
	{ 3, 318 }, // 233.1 Hz
	{ 3, 337 }, // 246.9 Hz
	{ 4, 179 }, // 261.6 Hz
	{ 4, 189 }, // 277.2 Hz
	{ 4, 200 }, // 293.7 Hz
	{ 4, 212 }, // 311.1 Hz
	{ 4, 225 }, // 329.6 Hz
	{ 4, 238 }, // 349.2 Hz
	{ 4, 253 }, // 370.0 Hz
	{ 4, 268 }, // 392.0 Hz
	{ 4, 284 }, // 415.3 Hz
	{ 4, 300 }, // 440.0 Hz
	{ 4, 318 }, // 466.2 Hz
	{ 4, 337 }, // 493.9 Hz
	{ 5, 179 }, // 523.3 Hz
	{ 5, 189 }, // 554.4 Hz
	{ 5, 200 }, // 587.3 Hz
	{ 5, 212 }, // 622.3 Hz
	{ 5, 225 }, // 659.3 Hz
	{ 5, 238 }, // 698.5 Hz
	{ 5, 253 }, // 740.0 Hz
	{ 5, 268 }, // 784.0 Hz
	{ 5, 284 }, // 830.6 Hz
	{ 5, 300 }, // 880.0 Hz
	{ 5, 318 }, // 932.3 Hz
	{ 5, 337 }, // 987.8 Hz
	{ 6, 179 }, // 1046.5 Hz
	{ 6, 189 }, // 1108.7 Hz
	{ 6, 200 }, // 1174.7 Hz
	{ 6, 212 }, // 1244.5 Hz
	{ 6, 225 }, // 1318.5 Hz
	{ 6, 238 }, // 1396.9 Hz
	{ 6, 253 }, // 1480.0 Hz
	{ 6, 268 }, // 1568.0 Hz
	{ 6, 284 }, // 1661.2 Hz
	{ 6, 300 }, // 1760.0 Hz
	{ 6, 318 }, // 1864.7 Hz
	{ 6, 337 }, // 1975.5 Hz
	{ 6, 357 }, // 2093.0 Hz
	{ 6, 378 }, // 2217.5 Hz
	{ 6, 401 }, // 2349.3 Hz
	{ 6, 425 }, // 2489.0 Hz
	{ 6, 450 }, // 2637.0 Hz
	{ 6, 477 }, // 2793.8 Hz
	{ 6, 505 }, // 2960.0 Hz
	{ 6, 535 }, // 3136.0 Hz
	{ 6, 567 }, // 3322.4 Hz
	{ 6, 601 }, // 3520.0 Hz
	{ 6, 636 }, // 3729.3 Hz
	{ 6, 674 }, // 3951.1 Hz
	{ 6, 714 }, // 4186.0 Hz
	{ 6, 757 }, // 4434.9 Hz
	{ 6, 802 }, // 4698.6 Hz
	{ 6, 850 }, // 4978.0 Hz
	{ 6, 900 }, // 5274.0 Hz
	{ 6, 954 }, // 5587.7 Hz
	{ 7, 505 }, // 5919.9 Hz
	{ 7, 535 }, // 6271.9 Hz
	{ 7, 567 }, // 6644.9 Hz
	{ 7, 601 }, // 7040.0 Hz
	{ 7, 636 }, // 7458.6 Hz
	{ 7, 674 }, // 7902.1 Hz
	{ 7, 714 }, // 8372.0 Hz
	{ 7, 757 }, // 8869.8 Hz
	{ 7, 802 }, // 9397.3 Hz
	{ 7, 850 }, // 9956.1 Hz
	{ 7, 900 }, // 10548.1 Hz
	{ 7, 954 }, // 11175.3 Hz
	{ 7, 1010 }, // 11839.8 Hz
	{ 7, 1023 }, // < 12543.9 Hz
};
