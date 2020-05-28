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
#include "ymz294.h"
#include "freerun_timer.h"
#include <gd32vf103_spi.h>
#include <gd32vf103_timer.h>
#include <gd32vf103_gpio.h>

#define YMZ294_WR_N       GPIO_PIN_0    // A0
#define YMZ294_AO         GPIO_PIN_9    // B9
#define SN74HC164N_CLR_N  GPIO_PIN_4    // A4
#define SN74HC164N_B      GPIO_PIN_6    // A6

static void setup_sound_clock(void);
static void setup_com(void);

static inline void sn74hc164n_init(void)
{
	gpio_bit_set(GPIOA, SN74HC164N_B);
	gpio_bit_set(GPIOA, SN74HC164N_CLR_N);
}

static inline void sn74hc164n_deinit(void)
{
	gpio_bit_reset(GPIOA, SN74HC164N_B);
	gpio_bit_reset(GPIOA, SN74HC164N_CLR_N);
}

static inline void sn74hc164n_clear(void)
{
	gpio_bit_reset(GPIOA, SN74HC164N_CLR_N);
	gpio_bit_set(GPIOA, SN74HC164N_CLR_N);
}

static inline void ymz294_write_enable(void)
{
	gpio_bit_reset(GPIOA, YMZ294_WR_N);
}

static inline void ymz294_write_disable(void)
{
	gpio_bit_set(GPIOA, YMZ294_WR_N);
}

static inline void ymz294_address_mode(void)
{
	gpio_bit_reset(GPIOB, YMZ294_AO);
}

static inline void ymz294_data_mode(void)
{
	gpio_bit_set(GPIOB, YMZ294_AO);
}

static inline int32_t spi_transmit(const uint8_t data, uint16_t timeout_100us)
{
	uint32_t timer_mark = FREERUN_COUNTER_100US;
	while(!(SPI_STAT(SPI0) & SPI_STAT_TBE))
	{// wait until transmit buffer gets empty
		if ( (FREERUN_COUNTER_100US - timer_mark)>= timeout_100us )
		{// timeout
			return -1;
		}
	}

	SPI_DATA(SPI0) = data;
	
	while(SPI_STAT(SPI0) & SPI_STAT_TRANS)
	{// wait until the data is sent. 
		if ( (FREERUN_COUNTER_100US - timer_mark)>= timeout_100us )
		{// timeout
			return -2;
		}
	}

	return 0;
}

int32_t ymz294_init(void)
{
	sn74hc164n_deinit();
	sn74hc164n_init();

    ymz294_write_disable();
	ymz294_address_mode();

	setup_sound_clock(); //  supply to 4 MHz clock to YMZ294
	setup_com(); // setup communication processing

	return 0;
}

void ymz294_deinit(void)
{
	spi_disable(SPI0);
	timer_deinit(TIMER1);
}

int32_t ymz294_write(uint8_t addr, uint8_t data)
{
	// address
	sn74hc164n_clear();
	ymz294_address_mode();
	ymz294_write_enable();
	spi_transmit(addr, 10);
	ymz294_write_disable();

	// data 
	sn74hc164n_clear();
	ymz294_data_mode();
	ymz294_write_enable();
	spi_transmit(data, 10);
	ymz294_write_disable();

	return 0;
}


// setup 4MHz clock source for YMZ294
static void setup_sound_clock(void)
{
	timer_oc_parameter_struct timer_ocinitpara;
	timer_parameter_struct timer_initpara;
	timer_deinit(TIMER1);
	/* initialize TIMER init parameter struct */
	timer_struct_para_init(&timer_initpara);
	/* TIMER1 configuration */
	timer_initpara.prescaler		 = 2;// 96 MHz / 3 = 32 MHz
	timer_initpara.alignedmode	   = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period			= 7;// 32 MHz / 8 = 4 MHz
	timer_initpara.clockdivision	 = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER1, &timer_initpara);

	/* initialize TIMER channel output parameter struct */
	timer_channel_output_struct_para_init(&timer_ocinitpara);
	/* CH0, CH1 and CH2 configuration in PWM mode */
	timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;
	timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;
	timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
	timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
	timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
	timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

	timer_channel_output_config(TIMER1,TIMER_CH_1,&timer_ocinitpara);

	timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_1,4);
	timer_channel_output_mode_config(TIMER1,TIMER_CH_1,TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER1,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

	/* auto-reload preload enable */
	timer_auto_reload_shadow_enable(TIMER1);
	/* auto-reload preload enable */
	timer_enable(TIMER1);
}

static void setup_com(void)
{
	spi_parameter_struct spi_init_struct;

	/* deinitilize SPI and the parameters */
	spi_i2s_deinit(SPI0);
	spi_struct_para_init(&spi_init_struct);

	/* SPI1 parameter config */
	spi_init_struct.trans_mode				= SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode				= SPI_MASTER;
	spi_init_struct.frame_size				= SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase 	= SPI_CK_PL_HIGH_PH_2EDGE;
	spi_init_struct.nss						= SPI_NSS_SOFT;
	spi_init_struct.prescale				= SPI_PSC_16;
	spi_init_struct.endian					= SPI_ENDIAN_MSB;
	spi_init(SPI0, &spi_init_struct);
	spi_enable(SPI0);
}