/*
 
 This code was created by modifying ymf825board_sample1.ino.
 The author and license are as follows.

  MIT License

  Copyright (c) 2017 Yamaha Corporation

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
#include <gd32vf103_gpio.h>
#include <gd32vf103_spi.h>
#include "freerun_timer.h"
#include "ymf825.h"

#define OUTPUT_power 1

#define SPI_TRANSMIT_TIMEOUT 	(10000) // 1000.0 msec.
#define SPI_RECEIVE_TIMEOUT 	(10000) // 1000.0 msec.

#define delay(x) 				delay100us(x*10) // unit: 1msec

static uint8_t tone_data_head[1] ={
	(0x80+16),//header
};
static uint8_t tone_data_tail[4] ={
	0x80,0x03,0x81,0x80,
};

static spi_parameter_struct spi_init_struct;

static void set_ss_low(void);
static void set_ss_high(void);
static void set_rst_low(void);
static void set_rst_high(void);
static void setup(void);

static inline int32_t spi_transmit(const uint8_t *data, uint16_t size, uint16_t timeout_100us)
{
	int i = 0;
	uint32_t timer_mark = FREERUN_COUNTER_100US;
	volatile uint8_t dummy = 0;

	if ( SPI_STAT(SPI1) & SPI_STAT_RBNE )
	{// dummy read
		dummy = SPI_DATA(SPI1);
	}

	for (i = 0; i < size; i++)
	{
		while(!(SPI_STAT(SPI1) & SPI_STAT_TBE))
		{// wait until transmit buffer gets empty
			if ( (FREERUN_COUNTER_100US - timer_mark)>= timeout_100us )
			{// timeout
				return -1;
			}
		}

		// send 1 byte data
		SPI_DATA(SPI1) = data[i];

		while(SPI_STAT(SPI1) & SPI_STAT_TRANS)
		{// wait until the data is sent. 
			if ( (FREERUN_COUNTER_100US - timer_mark)>= timeout_100us )
			{// timeout
				return -2;
			}
		}

		// dummy read
		dummy = SPI_DATA(SPI1);
		(void)dummy;
	}

	return 0;
}

static inline int32_t spi_receive(uint8_t *outbuf, uint16_t size, uint16_t timeout_100us)
{
	int i = 0;
	uint32_t timer_mark = FREERUN_COUNTER_100US;
	volatile uint8_t dummy = 0;

	if ( SPI_STAT(SPI1) & SPI_STAT_RBNE )
	{// dummy read
		dummy = SPI_DATA(SPI1);
		(void)dummy;
	}

	for (i = 0; i < size; i++)
	{
		while(!(SPI_STAT(SPI1) & SPI_STAT_TBE))
		{// wait until transmit buffer gets empty
			if ( (FREERUN_COUNTER_100US - timer_mark)>= timeout_100us )
			{// timeout
				return -1;
			}
		}

		// send dummy data.
		SPI_DATA(SPI1) = 0;

		while(SPI_STAT(SPI1) & SPI_STAT_TRANS)
		{// wait until the data is sent. 
			if ( (FREERUN_COUNTER_100US - timer_mark)>= timeout_100us )
			{// timeout
				return -2;
			}
		}

		// read recv data.
		outbuf[i] = SPI_DATA(SPI1);
	}

	return 0;
}


int32_t YMF825_Init(void) {

	/* deinitilize SPI and the parameters */
	spi_i2s_deinit(SPI1);
	spi_struct_para_init(&spi_init_struct);

	/* SPI1 parameter config */
	spi_init_struct.trans_mode				= SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode				= SPI_MASTER;
	spi_init_struct.frame_size				= SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase 	= SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss						= SPI_NSS_SOFT;
	spi_init_struct.prescale				= SPI_PSC_16;
	spi_init_struct.endian					= SPI_ENDIAN_MSB;
	spi_init(SPI1, &spi_init_struct);
	spi_enable(SPI1);

	set_ss_high();
	set_rst_low();

	setup();

	return 0;
}

void YMF825_DeInit(void) {

	spi_disable(SPI1);
}

void if_write(uint8_t addr, const uint8_t* data, uint16_t size){

	set_ss_low();
	spi_transmit(&addr, 1, SPI_TRANSMIT_TIMEOUT);
	spi_transmit(data, size, SPI_TRANSMIT_TIMEOUT);
	set_ss_high();	
}

void if_s_write(uint8_t addr,uint8_t data){

	if_write(addr,&data,1);
}

uint8_t if_s_read(uint8_t addr){

	uint8_t rcv = 0;
	uint8_t read_addr = addr|0x80;

	set_ss_low();
	spi_transmit(&read_addr, 1, SPI_TRANSMIT_TIMEOUT);
	spi_receive(&rcv, 1, SPI_RECEIVE_TIMEOUT);
	set_ss_high();

	return rcv;	
}


void YMF825_SelectChannel(uint8_t ch) {

	if_s_write(0x0B, (ch&0x0F));
}

void YMF825_ChangeVoVol(uint8_t VoVol) {

	if_s_write(0x0C, ((VoVol&0x1F) << 2));
}

void YMF825_ChangeChVol(uint8_t ChVol) {

	if_s_write(0x10, ((ChVol&0x1F) << 2));
}

void YMF825_SelectNoteNumber(uint16_t fnum, uint16_t block) {

	uint8_t dat[2] = {0x00, 0x00};
	dat[0] = ((fnum & 0x380) >> 4) | (block&0x07);
	dat[1] = fnum & 0x7F;
	if_s_write(0x0D, dat[0]);
	if_s_write(0x0E, dat[1]);
}

void YMF825_KeyOn(uint8_t tone_num) {

	if_s_write(0x0F, (0x40|(tone_num&0x0F)));
}

void YMF825_KeyOff(uint8_t tone_num) {

	if_s_write(0x0F, (0x00|(tone_num&0x0F)));
}

void YMF825_ChangeMASTER_VOL(uint8_t master_vol) {

	if_s_write( 0x19, ((master_vol&0x3F) << 2) );
}

void YMF825_ChangePitch(uint16_t INT, uint16_t FRAC) {

	if_s_write(0x12, (INT<<3) | ((FRAC>>6)&0x07));
	if_s_write(0x13, (FRAC&0x3F)<<1);
}

void YMF825_SetToneParameter(uint8_t tone_matrix[16][30]) {

	uint8_t addr = 0x07;
	int32_t i = 0;

	if_s_write( 0x08, 0xF6 );
	delay(1);
	if_s_write( 0x08, 0x00 );
	set_ss_low();
	spi_transmit(&addr, 1, SPI_TRANSMIT_TIMEOUT);
	spi_transmit(&tone_data_head[0], sizeof(tone_data_head), SPI_TRANSMIT_TIMEOUT);
	for ( i = 0; i < 16; i++ ) {
		spi_transmit(tone_matrix[i], 30, SPI_TRANSMIT_TIMEOUT);
	}
	spi_transmit(&tone_data_tail[0], sizeof(tone_data_tail), SPI_TRANSMIT_TIMEOUT);
	set_ss_high();	

}

static void set_ss_low(void) {

	gpio_bit_reset(GPIOB, GPIO_PIN_12);
}

static void set_ss_high(void) {

	gpio_bit_set(GPIOB, GPIO_PIN_12);
}

static void set_rst_low(void) {

	gpio_bit_reset(GPIOA, GPIO_PIN_8);
}

static void set_rst_high(void) {

	gpio_bit_set(GPIOA, GPIO_PIN_8);
}

static void setup(void) {
	set_rst_low();
	delay(1);
	set_rst_high();
	if_s_write( 0x1D, OUTPUT_power );
	if_s_write( 0x02, 0x0E );
	delay(1);
	if_s_write( 0x00, 0x01 );//CLKEN
	if_s_write( 0x01, 0x00 ); //AKRST
	if_s_write( 0x1A, 0xA3 );
	delay(1);
	if_s_write( 0x1A, 0x00 );
	delay(30);
	if_s_write( 0x02, 0x04 );//AP1,AP3
	delay(1);
	if_s_write( 0x02, 0x00 );
	//add
	if_s_write( 0x19, (40 << 2) );//MASTER VOL
	if_s_write( 0x1B, 0x3F );//interpolation
	if_s_write( 0x14, 0x00 );//interpolation
	if_s_write( 0x03, 0x01 );//Analog Gain
	
	if_s_write( 0x08, 0xF6 );
	delay(21);
	if_s_write( 0x08, 0x00 );
	if_s_write( 0x09, 0xF8 );
	if_s_write( 0x0A, 0x00 );
	
	if_s_write( 0x17, 0x40 );//MS_S
	if_s_write( 0x18, 0x00 );
}

void init_825(void) {

   set_rst_low();
   delay(1);
   set_rst_high();
   if_s_write( 0x1D, OUTPUT_power );
   if_s_write( 0x02, 0x0E );
   delay(1);
   if_s_write( 0x00, 0x01 );//CLKEN
   if_s_write( 0x01, 0x00 ); //AKRST
   if_s_write( 0x1A, 0xA3 );
   delay(1);
   if_s_write( 0x1A, 0x00 );
   delay(30);
   if_s_write( 0x02, 0x04 );//AP1,AP3
   delay(1);
   if_s_write( 0x02, 0x00 );
   //add
   if_s_write( 0x19, 0x80 );//MASTER VOL
   if_s_write( 0x1B, 0x3F );//interpolation
   if_s_write( 0x14, 0x00 );//interpolation
   if_s_write( 0x03, 0x01 );//Analog Gain
   
   if_s_write( 0x08, 0xF6 );
   delay(21);
   if_s_write( 0x08, 0x00 );
   if_s_write( 0x09, 0xF8 );
   if_s_write( 0x0A, 0x00 );
   
   if_s_write( 0x17, 0x40 );//MS_S
   if_s_write( 0x18, 0x00 );

}

void set_tone(void){
  unsigned char tone_data[35] ={
	0x81,//header
	//T_ADR 0
	0x01,0x85,
	0x00,0x7F,0xF4,0xBB,0x00,0x10,0x40,
	0x00,0xAF,0xA0,0x0E,0x03,0x10,0x40,
	0x00,0x2F,0xF3,0x9B,0x00,0x20,0x41,
	0x00,0xAF,0xA0,0x0E,0x01,0x10,0x40,
	0x80,0x03,0x81,0x80,
  };
  
   if_s_write( 0x08, 0xF6 );
   delay(1);
   if_s_write( 0x08, 0x00 );
  
   if_write( 0x07, &tone_data[0], 35 );//write to FIFO
}

void set_ch(void){
   if_s_write( 0x0F, 0x30 );// keyon = 0
   if_s_write( 0x10, 0x71 );// chvol
   if_s_write( 0x11, 0x00 );// XVB
   if_s_write( 0x12, 0x08 );// FRAC
   if_s_write( 0x13, 0x00 );// FRAC  
}

void keyon(unsigned char fnumh, unsigned char fnuml){
   if_s_write( 0x0B, 0x00 );//voice num
   if_s_write( 0x0C, 0x54 );//vovol
   if_s_write( 0x0D, fnumh );//fnum
   if_s_write( 0x0E, fnuml );//fnum
   if_s_write( 0x0F, 0x40 );//keyon = 1  
}

void keyoff(void){
   if_s_write( 0x0F, 0x00 );//keyon = 0
}

void test_ymf825(void) {
  // put your main code here, to run repeatedly:
  keyon(0x14,0x65);
  delay(500);
  keyoff();
  delay(200);
  keyon(0x1c,0x11);
  delay(500);
  keyoff();
  delay(200);
  keyon(0x1c,0x42);
  delay(500);
  keyoff();
  delay(200);
  keyon(0x1c,0x5d);
  delay(500);
  keyoff();
  delay(200);
  keyon(0x24,0x17);
  delay(500);
  keyoff();
  delay(200);
}