/*
    Copyright (c) 2019, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <stdarg.h>
#include <gd32vf103_timer.h>
#include <gd32vf103_rcu.h>
#include <gd32vf103_eclic.h>
#include <usbd_core.h>
#include <drv_usb_hw.h>
#include <usbd_enum.h>
#include "midi_cdc_desc.h"
#include "midi_cdc_core.h"


// usb cdc acm data send buffer page
#define USB_CDC_TX_BUF_PAGE_NUM                 2

// each usb cdc acm data send buffer size
#ifndef USB_CDC_TX_BUF_SIZE
#define USB_CDC_TX_BUF_SIZE                     (CDC_ACM_DATA_PACKET_SIZE * 5)
#endif

#define SEND_ENCAPSULATED_COMMAND               0x00
#define GET_ENCAPSULATED_RESPONSE               0x01
#define SET_COMM_FEATURE                        0x02
#define GET_COMM_FEATURE                        0x03
#define CLEAR_COMM_FEATURE                      0x04
#define SET_LINE_CODING                         0x20
#define GET_LINE_CODING                         0x21
#define SET_CONTROL_LINE_STATE                  0x22
#define SEND_BREAK                              0x23
#define NO_CMD                                  0xFF

// usb cdc acm data send status
typedef enum
{
   USB_CDC_SEND_STATUS_INIT = 0,
   USB_CDC_SEND_STATUS_BUSY,
   USB_CDC_SEND_STATUS_FINISHED
}usb_cdc_send_status_t;

typedef struct
{
    uint32_t dwDTERate;   /* data terminal rate */
    uint8_t  bCharFormat; /* stop bits */
    uint8_t  bParityType; /* parity */
    uint8_t  bDataBits;   /* data bits */
}line_coding_struct;


static uint32_t cdc_cmd = 0xFFU;
static uint8_t usb_cmd_buffer[CDC_ACM_CMD_PACKET_SIZE];


// current usb cdc acm data send status
static usb_cdc_send_status_t usb_cdc_send_status = USB_CDC_SEND_STATUS_INIT;

// current usb cdc acm data buffer page 
static uint8_t current_buffer_page = 0;

// current usb cdc acm data buffer position
static uint8_t current_end_position = 0;

// usb cdc acm data send buffer (double buffer)
static uint8_t usb_cdc_tx_buffer[USB_CDC_TX_BUF_PAGE_NUM][USB_CDC_TX_BUF_SIZE];

// usb cdc data receive buffer
uint8_t usb_cdc_receive_buffer[CDC_ACM_DATA_PACKET_SIZE];

// usb midi receive buffer
uint8_t usb_midi_receive_buffer[AUDIO_MS_PACKET_SIZE];

// receive callback functions
static pf_usb_midi_receive_callback_t   usb_midi_recv_cb    = (pf_usb_midi_receive_callback_t)0;
static pf_usb_cdc_receive_callback_t    usb_cdc_recv_cb     = (pf_usb_cdc_receive_callback_t)0;


static uint8_t  midi_cdc_init(usb_dev *udev, uint8_t config_index);
static uint8_t  midi_cdc_deinit(usb_dev *udev, uint8_t config_index);
static uint8_t  midi_cdc_req_proc(usb_dev *udev, usb_req *req);
static uint8_t  midi_cdc_data_in(usb_dev *udev, uint8_t ep_num);
static uint8_t  midi_cdc_data_out(usb_dev *udev, uint8_t ep_num);


static uint8_t cdc_acm_req_handler (usb_dev *pudev, usb_req *req);
static uint8_t cdc_acm_EP0_RxReady (usb_dev *pudev);


static void start_usb_cdc_send_service_irq(void);


static line_coding_struct linecoding =
{
    115200, /* baud rate     */
    0x00,   /* stop bits - 1 */
    0x00,   /* parity - none */
    0x08    /* num of bits 8 */
};

static usb_class_core  class_core = 
{
    .command                = NO_CMD,             /*!< device class request command */
    .alter_set              = 0,                  /*!< alternative set */
    .init                   = midi_cdc_init,      /*!< initialize handler */
    .deinit                 = midi_cdc_deinit,    /*!< de-initialize handler */
    .req_proc               = midi_cdc_req_proc,  /*!< device request handler */
    .data_in                = midi_cdc_data_in,   /*!< device data in handler */
    .data_out               = midi_cdc_data_out,  /*!< device data out handler */
    .SOF                    = NULL,               /*!< Start of frame handler */
    .incomplete_isoc_in     = NULL,               /*!< Incomplete synchronization IN transfer handler */
    .incomplete_isoc_out    = NULL,               /*!< Incomplete synchronization OUT transfer handler */
};


void init_usbd_midi_cdc(const pf_usb_cdc_receive_callback_t cdc_recv_cb, const pf_usb_midi_receive_callback_t midi_recv_cb)
{
    register_usb_cdc_receive_callback(cdc_recv_cb);

    register_usb_midi_receive_callback(midi_recv_cb);

    eclic_irq_enable(TIMER2_IRQn, 2, 0);

    /* USB device stack configure */
    usbd_init (&g_midi_cdc_udev, USB_CORE_ENUM_FS, &class_core);

    start_usb_cdc_send_service_irq();

    /* USB interrupt configure */
    usb_intr_config();

    /* check if USB device is enumerated successfully */
    while (g_midi_cdc_udev.dev.cur_status != USBD_CONFIGURED)
    {
    }
}

// register a callback function to be called when usb midi message received.
void register_usb_midi_receive_callback(const pf_usb_midi_receive_callback_t callback)
{
    usb_midi_recv_cb = callback;
}

// register a callback function to be called when usb cdc data received.
void register_usb_cdc_receive_callback(const pf_usb_cdc_receive_callback_t callback)
{
    usb_cdc_recv_cb = callback;
}

int usb_cdc_printf(const char *format, ...)
{
    int written_num = 0;
    size_t max_write_num = 0;

    va_list arg;
    va_start(arg, format);

    // entry critical section
    eclic_global_interrupt_disable();
    max_write_num = USB_CDC_TX_BUF_SIZE - current_end_position;
    written_num = vsnprintf(
        (char *)&usb_cdc_tx_buffer[current_buffer_page][current_end_position]
        ,max_write_num, format, arg); 

    if ( ( 0 < written_num ) && ( written_num < max_write_num ) )
    {
        current_end_position += written_num;
    }
    // leave critical section
    eclic_global_interrupt_enable();

    va_end(arg);

    return 0;
}

// send data and switch send buffer page.
void usb_cdc_send_service_irq(void)
{
    if (RESET != timer_flag_get(TIMER6, TIMER_FLAG_UP))
    {
        if (USBD_CONFIGURED == g_midi_cdc_udev.dev.cur_status)
        {
            // entry critical section
            eclic_global_interrupt_disable();
            if ( usb_cdc_send_status != USB_CDC_SEND_STATUS_BUSY )
            {
                if ( current_end_position > 0 )
                {
                    usb_cdc_send_status = USB_CDC_SEND_STATUS_BUSY;
                    usbd_ep_send(&g_midi_cdc_udev, CDC_IN_EP, &usb_cdc_tx_buffer[current_buffer_page][0] , current_end_position);
                    current_end_position = 0;
                    current_buffer_page++;
                    if ( current_buffer_page >= USB_CDC_TX_BUF_PAGE_NUM )
                    {
                        current_buffer_page = 0;
                    }
                }
            }
            // leave critical section
            eclic_global_interrupt_enable();
        }

        timer_flag_clear(TIMER6, TIMER_FLAG_UP);
    }
}

static uint8_t  midi_cdc_init(usb_dev *udev, uint8_t config_index)
{
    midi_cdc_desc_ep_setup(udev);

    //  prepare receive data
    usbd_ep_recev(udev, MIDI_OUT_EP, usb_midi_receive_buffer, AUDIO_MS_PACKET_SIZE);
    usbd_ep_recev(udev, CDC_OUT_EP,  usb_cdc_receive_buffer,   CDC_ACM_DATA_PACKET_SIZE);

    return 0;
}


static uint8_t  midi_cdc_deinit(usb_dev *udev, uint8_t config_index)
{
    midi_cdc_desc_ep_clear(udev);
    return 0;
}


static uint8_t  midi_cdc_req_proc(usb_dev *udev, usb_req *req)
{
    switch (req->wIndex)
    {
        case AUDIO_AC_ITF_NUMBER:
            break;

        case AUDIO_MS_ITF_NUMBER:
            break;

        case CDC_CDC_ITF_NUMBER:
            cdc_acm_req_handler(udev, req);
            break;

        case CDC_DATA_ITF_NUMBER:
            break;

        default:
            break;
    }
    return 0;
}


static uint8_t  midi_cdc_data_in(usb_dev *udev, uint8_t ep_num)
{
    if ((CDC_IN_EP & 0x7F) == ep_num) 
    {
        usb_transc *transc = &udev->dev.transc_in[EP_ID(ep_num)];

        if ((transc->xfer_len % transc->max_len == 0) && (transc->xfer_len != 0)) 
        {
            usbd_ep_send (udev, ep_num, NULL, 0U);
        }
        else
        {
            usb_cdc_send_status = USB_CDC_SEND_STATUS_FINISHED;
        }
    } 

    return 0;
}


static uint8_t  midi_cdc_data_out(usb_dev *udev, uint8_t ep_num)
{
    uint32_t  receive_length = 0U;

    if ((EP0_OUT & 0x7F) == ep_num) 
    {
        cdc_acm_EP0_RxReady (udev);
    } 
    else if ((CDC_OUT_EP & 0x7F) == ep_num) 
    {
        receive_length = usbd_rxcount_get(udev, CDC_OUT_EP);
        if ( usb_cdc_recv_cb )
        {
            usb_cdc_recv_cb(usb_cdc_receive_buffer, receive_length);
        }
        usbd_ep_recev(udev, CDC_OUT_EP, usb_cdc_receive_buffer, CDC_ACM_DATA_PACKET_SIZE);
    }
    else if ((MIDI_OUT_EP & 0x7F) == ep_num)
    {
        receive_length = usbd_rxcount_get(udev, MIDI_OUT_EP);
        if ( usb_midi_recv_cb )
        {
            usb_midi_recv_cb(usb_midi_receive_buffer, receive_length);
        }
        usbd_ep_recev(udev, MIDI_OUT_EP, usb_midi_receive_buffer, AUDIO_MS_PACKET_SIZE);
    }
    else
    {
    }

    return 0;
}

/*!
    \brief      handle the CDC ACM class-specific requests
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t cdc_acm_req_handler (usb_dev *pudev, usb_req *req)
{
    switch (req->bRequest) 
    {
        case SEND_ENCAPSULATED_COMMAND:
            break;
        case GET_ENCAPSULATED_RESPONSE:
            break;
        case SET_COMM_FEATURE:
            break;
        case GET_COMM_FEATURE:
            break;
        case CLEAR_COMM_FEATURE:
            break;
        case SET_LINE_CODING:
            /* set the value of the current command to be processed */
            cdc_cmd = req->bRequest;
            /* enable EP0 prepare to receive command data packet */
            pudev->dev.transc_out[0].xfer_buf = usb_cmd_buffer;
            pudev->dev.transc_out[0].remain_len = req->wLength;
            break;
        case GET_LINE_CODING:
            usb_cmd_buffer[0] = (uint8_t)(linecoding.dwDTERate);
            usb_cmd_buffer[1] = (uint8_t)(linecoding.dwDTERate >> 8);
            usb_cmd_buffer[2] = (uint8_t)(linecoding.dwDTERate >> 16);
            usb_cmd_buffer[3] = (uint8_t)(linecoding.dwDTERate >> 24);
            usb_cmd_buffer[4] = linecoding.bCharFormat;
            usb_cmd_buffer[5] = linecoding.bParityType;
            usb_cmd_buffer[6] = linecoding.bDataBits;
            /* send the request data to the host */
            pudev->dev.transc_in[0].xfer_buf = usb_cmd_buffer;
            pudev->dev.transc_in[0].remain_len = req->wLength;
            break;
        case SET_CONTROL_LINE_STATE:
            break;
        case SEND_BREAK:
            break;
        default:
            break;
    }

    return USBD_OK;
}

/*!
    \brief      command data received on control endpoint
    \param[in]  pudev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t cdc_acm_EP0_RxReady (usb_dev *pudev)
{
    if (NO_CMD != cdc_cmd) {
        /* process the command data */
        linecoding.dwDTERate = (uint32_t)(usb_cmd_buffer[0] | 
                                         (usb_cmd_buffer[1] << 8) |
                                         (usb_cmd_buffer[2] << 16) |
                                         (usb_cmd_buffer[3] << 24));

        linecoding.bCharFormat = usb_cmd_buffer[4];
        linecoding.bParityType = usb_cmd_buffer[5];
        linecoding.bDataBits = usb_cmd_buffer[6];

        cdc_cmd = NO_CMD;
    }

    return USBD_OK;
}

// start usb cdc send service routine.
static void start_usb_cdc_send_service_irq(void)
{
    timer_parameter_struct timer_initpara;

    eclic_irq_enable(TIMER6_IRQn, 2, 0);

    timer_struct_para_init(&timer_initpara);

    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.prescaler         = 9599;// 96 MHz/ 9600 = 10 kHz
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 10;

    timer_deinit(TIMER6);
    timer_init(TIMER6, &timer_initpara);

    timer_update_event_enable(TIMER6);
    timer_interrupt_enable(TIMER6,TIMER_INT_UP);
    timer_flag_clear(TIMER6, TIMER_FLAG_UP);
    timer_update_source_config(TIMER6, TIMER_UPDATE_SRC_GLOBAL);
  
    /* TIMER6 counter enable */
    timer_enable(TIMER6);
}

