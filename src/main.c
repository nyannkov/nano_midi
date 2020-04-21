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
#include <gd32vf103_rcu.h>
#include <drv_usb_hw.h>
#include "midi_cdc_core.h"
#include "freerun_timer.h"
#include "usb_midi_app.h"
#include "usb_cdc_app.h"
#include "memmng.h"

#define BLINK_CYCLE			1000000

static void config_eclic(void);
static void enable_periph_clock(void);
static void config_gpio(void);
static void led_blink(void);

static unsigned char pool[1024];
static memmng_t hmemmng;

int  main(void)
{

	// initialize a memory pool used as a heap.
	memmng_init(&hmemmng, pool, sizeof(pool));

	config_eclic();

	enable_periph_clock();

	config_gpio();

	init_freerun_timer();

	// initialize an application of usb midi.
	init_usb_midi_app();

	// initialize an application of usb cdc.
	init_usb_cdc_app();

	// start the usb midi cdc device.
	init_usbd_midi_cdc(usb_cdc_proc, usb_midi_proc);

	while(1)
	{
		led_blink();
	}

	return 0;
}

void *malloc(size_t size)
{
	return memmng_alloc(&hmemmng, size);
}

void free(void *p)
{
	memmng_free(&hmemmng, p);
}

static void enable_periph_clock(void)
{
	// SPI1, LED
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_SPI1);

	// free-running timer
	rcu_periph_clock_enable(RCU_TIMER5);

	// USB device
	rcu_periph_clock_enable(RCU_TIMER2);
	rcu_periph_clock_enable(RCU_TIMER6);
	usb_rcu_config();

}

static void config_gpio(void)
{
	/* SPI1 GPIO config: SCK/PB13, MOSI/PB15 */
	gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,  GPIO_PIN_13 | GPIO_PIN_15);

	// used as SPI1 NSS but configured as GPIO output.
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12); 

	/* SPI1 GPIO config: MISO/PB14 */
	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);

	// RST_N of ymf825 board
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

	// LED
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
}

static void config_eclic(void)
{
	eclic_global_interrupt_enable();
	eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL2_PRIO2);
}

static void led_blink(void)
{
	static uint32_t cycle_counter = 0;
	static int32_t led_output = 0;

	if ( cycle_counter++ >= BLINK_CYCLE ) 
	{
		cycle_counter = 0;
		if ( led_output == 0 )
		{
			gpio_bit_set(GPIOA, GPIO_PIN_2);
			led_output = 1;
		}
		else
		{
			gpio_bit_reset(GPIOA, GPIO_PIN_2);
			led_output = 0;
		}
	}
}
