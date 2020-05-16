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
#ifndef __YMZ294_H__
#define __YMZ294_H__

#include <stdint.h>

/*
+--------------------------------------+------+------+------+------+------+------+------+------+------+
|                                      | ADDR |  D7  |  D6  |  D5  |  D4  |  D3  |  D2  |  D1  |  D0  |
+==========================+===========+======+======+======+======+======+======+======+======+======+
|                          |           | $00  | TP7  | TP6  | TP5  | TP4  | TP3  | TP2  | TP1  | TP0  |
|                          | Channel A +------+------+------+------+------+------+------+------+------+
|                          |           | $01  |  --  |  --  |  --  |  --  | TP11 | TP10 | TP9  | TP8  |
|                          +-----------+------+------+------+------+------+------+------+------+------+
|                          |           | $02  | TP7  | TP6  | TP5  | TP4  | TP3  | TP2  | TP1  | TP0  |
| Tone frequency setting   | Channel B +------+------+------+------+------+------+------+------+------+
|                          |           | $03  |  --  |  --  |  --  |  --  | TP11 | TP10 | TP9  | TP8  |
|                          +-----------+------+------+------+------+------+------+------+------+------+
|                          |           | $04  | TP7  | TP6  | TP5  | TP4  | TP3  | TP2  | TP1  | TP0  |
|                          | Channel C +------+------+------+------+------+------+------+------+------+
|                          |           | $05  |  --  |  --  |  --  |  --  | TP11 | TP10 | TP9  | TP8  |
+--------------------------+-----------+------+------+------+------+------+------+------+------+------+
| Noise frequency setting              | $06  |  --  |  --  |  --  | NP4  | NP3  | NP2  | NP1  | NP0  |
+--------------------------------------+------+------+------+------+------+------+------+------+------+
|                                      |      |  --  |  --  |  C   |  B   |  A   |  C   |  B   |  A   |
| Mixer                                | $07  +------+------+------+------+------+------+------+------+
|                                      |      |  --  |  --  |        Noise       |        Tone        |
+--------------------------+-----------+------+------+------+------+------+------+------+------+------+
|                          | Channel A | $08  |  --  |  --  |  --  |  M   |  L3  |  L2  |  L1  |  L0  |
|                          +-----------+------+------+------+------+------+------+------+------+------+
| Volume control and DAC   | Channel B | $09  |  --  |  --  |  --  |  M   |  L3  |  L2  |  L1  |  L0  |
|                          +-----------+------+------+------+------+------+------+------+------+------+
|                          | Channel C | $0A  |  --  |  --  |  --  |  M   |  L3  |  L2  |  L1  |  L0  |
+--------------------------+-----------+------+------+------+------+------+------+------+------+------+
|                                      | $0B  | EP7  | EP6  | EP5  | EP4  | EP3  | EP2  | EP1  | EP0  |
| Envelope frequency setting           +------+------+------+------+------+------+------+------+------+
|                                      | $0C  | EP15 | EP14 | EP13 | EP12 | EP11 | EP10 | EP9  | EP8  |
+--------------------------------------+------+------+------+------+------+------+------+------+------+
| Envelope shape control               | $0D  |  --  |  --  |  --  |  --  | CONT | ATT  | ALT  | HOLD |
+--------------------------------------+------+------+------+------+------+------+------+------+------+

+------+------+------+------+---------------------------------------------------+
| CONT | ATT  | ALT  | HOLD |                  Envelope shape                   |
+======+======+======+======+===================================================+
|      |      |      |      | o                                                 |
|      |      |      |      |   o                                               |
|  0   |  0   |  x   |  x   |     o                                             |
|      |      |      |      |       o                                           |
|      |      |      |      |         o o o o o o o o o o o o o o o o o o o o o |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      |         o                                         |
|      |      |      |      |       o o                                         |
|  0   |  1   |  x   |  x   |     o   o                                         |
|      |      |      |      |   o     o                                         |
|      |      |      |      | o       o o o o o o o o o o o o o o o o o o o o o |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      | o       o       o       o       o       o         |
|      |      |      |      |   o     o o     o o     o o     o o     o o       |
|  1   |  0   |  0   |  0   |     o   o   o   o   o   o   o   o   o   o   o     |
|      |      |      |      |       o o     o o     o o     o o     o o     o   |
|      |      |      |      |         o       o       o       o       o       o |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      | o                                                 |
|      |      |      |      |   o                                               |
|  1   |  0   |  0   |  1   |     o                                             |
|      |      |      |      |       o                                           |
|      |      |      |      |         o o o o o o o o o o o o o o o o o o o o o |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      | o               o               o               o |
|      |      |      |      |   o           o   o           o   o           o   |
|  1   |  0   |  1   |  0   |     o       o       o       o       o       o     |
|      |      |      |      |       o   o           o   o           o   o       |
|      |      |      |      |         o               o               o         |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      | o       o o o o o o o o o o o o o o o o o o o o o |
|      |      |      |      |   o     o                                         |
|  1   |  0   |  1   |  1   |     o   o                                         |
|      |      |      |      |       o o                                         |
|      |      |      |      |         o                                         |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      |         o       o       o       o       o       o |
|      |      |      |      |       o o     o o     o o     o o     o o     o   |
|  1   |  1   |  0   |  0   |     o   o   o   o   o   o   o   o   o   o   o     |
|      |      |      |      |   o     o o     o o     o o     o o     o o       |
|      |      |      |      | o       o       o       o       o       o         |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      |         o o o o o o o o o o o o o o o o o o o o o |
|      |      |      |      |       o                                           |
|  1   |  1   |  0   |  1   |     o                                             |
|      |      |      |      |   o                                               |
|      |      |      |      | o                                                 |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      |         o               o               o         |
|      |      |      |      |       o   o           o   o           o   o       |
|  1   |  1   |  1   |  0   |     o       o       o       o       o       o     |
|      |      |      |      |   o           o   o           o   o           o   |
|      |      |      |      | o               o               o               o |
+------+------+------+------+---------------------------------------------------+
|      |      |      |      |         o                                         |
|      |      |      |      |       o o                                         |
|  1   |  1   |  1   |  1   |     o   o                                         |
|      |      |      |      |   o     o                                         |
|      |      |      |      | o       o o o o o o o o o o o o o o o o o o o o o |
+------+------+------+------+---------------------------------------------------+
*/

extern int32_t ymz294_init(void);
extern void ymz294_deinit(void);
extern int32_t ymz294_write(uint8_t addr, uint8_t data);

#endif//__YMZ294_H__
