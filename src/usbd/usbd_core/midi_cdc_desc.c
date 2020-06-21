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
#include <usbd_core.h>
#include <usbd_enum.h>
#include "midi_cdc_desc.h"

#define USBD_VID                        0xaaaa// replace 0xaaaa with your vid
#define USBD_PID                        0xbbbb// replace 0xbbbb with your pid

#define LITTLE_ENDIAN_16(x)                 (x) 

static const usb_desc_dev device_descriptor =
{
    .header = 
     {
         .bLength = USB_DEV_DESC_LEN, 
         .bDescriptorType = USB_DESCTYPE_DEV
     },
    .bcdUSB = 0x0200,
    .bDeviceClass = 0xEF,
    .bDeviceSubClass = 0x02,
    .bDeviceProtocol = 0x01,
    .bMaxPacketSize0 = USB_FS_EP0_MAX_LEN,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = LITTLE_ENDIAN_16(0x0100),
    .iManufacturer = STR_IDX_MFC,
    .iProduct = STR_IDX_PRODUCT,
    .iSerialNumber = STR_IDX_SERIAL,
    .bNumberConfigurations = USBD_CFG_MAX_NUM
};

static const usb_descriptor_configuration_set_struct configuration_descriptor= 
{
    .config = 
    {/* Configuration 1 */
        .header =
        {
            .bLength                = 0x09,
            .bDescriptorType        = USB_DESCTYPE_CONFIG,
        },
        .wTotalLength               = LITTLE_ENDIAN_16(0x00AF),
        .bNumInterfaces             = 0x04, /* audio 2 + cdc 2 = 4 */
        .bConfigurationValue        = 0x01,
        .iConfiguration             = 0x00,
        .bmAttributes               = 0xC0,
        .bMaxPower                  = 0x32,
    },
    .usb_iad_audio = 
    {/* Interface Associated Descriptor */ 
        .bLength                    = 0x08,
        .bDescriptorType            = 0x0B,
        .bFirstInterface            = AUDIO_AC_ITF_NUMBER,
        .bInterfaceCount            = 0x02,
        .bFunctionClass             = 0x01,
        .bFunctionSubClass          = 0x01,
        .bFunctionProtocol          = 0x00,
        .iFunction                  = 0x02,
    },
    .audio = 
    {/* Audio device */
        .standard_ac_itf =
        {/* Standard AC Interface Descriptor */
            .header =
            { 
                .bLength            = 0x09,
                .bDescriptorType    = USB_DESCTYPE_ITF,
            },
            .bInterfaceNumber       = AUDIO_AC_ITF_NUMBER,
            .bAlternateSetting      = 0x00,
            .bNumEndpoints          = 0x00,
            .bInterfaceClass        = 0x01,
            .bInterfaceSubClass     = 0x01,
            .bInterfaceProtocol     = 0x00,
            .iInterface             = 0x00,
        },
        .class_specific_ac_itf = 
        {/* Class-specific AC Interface Descriptor */
            .bLength                = 0x09,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x01,
            .bcdADC                 = LITTLE_ENDIAN_16(0x0100),/* 1.00 */
            .wTotalLength           = LITTLE_ENDIAN_16(0x0009),
            .bInCollection          = 0x01,
            .baInterfaceNr          = AUDIO_MS_ITF_NUMBER,
        },
        .standard_ms_itf =
        {/* Standard MS Interface Descriptor */
            .header =
            {
                .bLength            = 0x09,
                .bDescriptorType    = USB_DESCTYPE_ITF,
            },
            .bInterfaceNumber       = AUDIO_MS_ITF_NUMBER,
            .bAlternateSetting      = 0x00,
            .bNumEndpoints          = 0x02,
            .bInterfaceClass        = 0x01,
            .bInterfaceSubClass     = 0x03,
            .bInterfaceProtocol     = 0x00,
            .iInterface             = 0x00,
        },
        .class_specific_ms_itf = 
        {/* Class-specific MS Interface Descriptor */
             .bLength               = 0x07,
             .bDescriptorType       = 0x24,
             .bDescriptorSubtype    = 0x01,
             .BcdADC                = LITTLE_ENDIAN_16(0x0100),
             .wTotalLength          = LITTLE_ENDIAN_16(0x0025),
        },
        .midi_in_jack_embedded = 
        {/* MIDI IN Jack Descriptor (Embedded) */
            .bLength                = 0x06,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x02,
            .bJackType              = 0x01,/* embedded */
            .bJackID                = 0x01,
            .iJack                  = 0x00,
        },
        .midi_in_jack_external = 
        {/* MIDI In Jack Descriptor (External) */
            .bLength                = 0x06,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x02,
            .bJackType              = 0x02, /* external */
            .bJackID                = 0x02,
            .iJack                  = 0x00,
        },
        .midi_out_jack_embedded =
        {/* MIDI OUT Jack Descriptor (Embedded) */
            .bLength                = 0x09,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x03,
            .bJackType              = 0x01,
            .bJackID                = 0x03,
            .bNtInputPins           = 0x01,
            .BaSourceID             = 0x02,
            .BaSourcePin            = 0x01,
            .iJack                  = 0x00,
        },
        .midi_out_jack_external =
        {/* MIDI OUT Jack Descriptor (External) */
            .bLength                = 0x09,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x03,
            .bJackType              = 0x02,
            .bJackID                = 0x04,
            .bNtInputPins           = 0x01,
            .BaSourceID             = 0x01,
            .BaSourcePin            = 0x01,
            .iJack                  = 0x00,
        },
        .bulk_out_ep_desc =
        {/* Standard Bulk OUT EndPoint Descriptor */
            .desc_ep = 
            {
                .header =
                {
                    .bLength        = 0x09,
                    .bDescriptorType= 0x05,
                },
                .bEndpointAddress   = MIDI_OUT_EP,
                .bmAttributes       = 0x02,
                .wMaxPacketSize     = LITTLE_ENDIAN_16(AUDIO_MS_PACKET_SIZE),
                .bInterval          = 0x01,
            },
            .bRefresh               = 0x00,
            .bSynchAddress          = 0x00,
        },
        .class_specific_ms_bulk_out_ep = 
        {/* Class-specific Bulk OUT EndPoint Descriptor */
            .bLength                = 0x05,
            .bDescriptorType        = 0x25,
            .bDescriptorSubtype     = 0x01,
            .bNumEmbMIDIJack        = 0x01,
            .BaAssocJackID          = 0x01,
        },
        .bulk_in_ep_desc =
        {/* Standard Bulk IN EndPoint Descriptor */
            .desc_ep = 
            {
                .header =
                {
                    .bLength        = 0x09,
                    .bDescriptorType= 0x05,
                },
                .bEndpointAddress   = MIDI_IN_EP,
                .bmAttributes       = 0x02,
                .wMaxPacketSize     = LITTLE_ENDIAN_16(AUDIO_MS_PACKET_SIZE),
                .bInterval          = 0x00,
            },
            .bRefresh               = 0x00,
            .bSynchAddress          = 0x00,
        },
        .class_specific_ms_bulk_in_ep = 
        {/* Class-specific Bulk IN EndPoint Descriptor */
            .bLength                = 0x05,
            .bDescriptorType        = 0x25,
            .bDescriptorSubtype     = 0x01,
            .bNumEmbMIDIJack        = 0x01,
            .BaAssocJackID          = 0x03,
        },
    },
    .usb_iad_cdc = 
    {/* Interface Associated Descriptor */ 
        .bLength                    = 0x08,
        .bDescriptorType            = 0x0B,
        .bFirstInterface            = CDC_CDC_ITF_NUMBER,
        .bInterfaceCount            = 0x02,
        .bFunctionClass             = 0x02,
        .bFunctionSubClass          = 0x02,
        .bFunctionProtocol          = 0x01,
        .iFunction                  = 0x02,
    },
    .cdc = 
    {
        .cdc_cdc_itf = 
        {/* Interface Descriptor */
            .header =
            {
                .bLength            = 0x09,
                .bDescriptorType    = 0x04,
            },
            .bInterfaceNumber       = CDC_CDC_ITF_NUMBER,
            .bAlternateSetting      = 0x00,
            .bNumEndpoints          = 0x01,
            .bInterfaceClass        = 0x02,
            .bInterfaceSubClass     = 0x02,
            .bInterfaceProtocol     = 0x01,
            .iInterface             = 0x00,
        },
        .header_func_desc =
        {/* Header Functional Descriptor */
            .bLength                = 0x05,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x00,
            .bcdCDC                 = LITTLE_ENDIAN_16(0x0110),
        },
        .call_mng_func_desc =
        {/* Call Management Functional Descriptor */
            .bFunctionLength        = 0x05,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x01,
            .bmCapabilities         = 0x00,
            .bDataInterface         = 0x01,
        },
        .acm_func_desc =
        {/* ACM Functional Descriptor */
            .bFunctionLength        = 0x04,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x02,
            .bmCapabilities         = 0x02,
        },
        .union_func_desc =
        {/* Union Functional Descriptor */
            .bFunctionLength        = 0x05,
            .bDescriptorType        = 0x24,
            .bDescriptorSubtype     = 0x06,
            .bMasterInterface       = CDC_CDC_ITF_NUMBER,
            .bSlaveInterface0       = CDC_DATA_ITF_NUMBER,
        },
        .cmd_ep =
        {/* Endpoint Descriptor */
            .header =
            {
                .bLength            = 0x07,
                .bDescriptorType    = 0x05,
            },
            .bEndpointAddress       = CDC_CMD_EP,
            .bmAttributes           = 0x03,
            .wMaxPacketSize         = LITTLE_ENDIAN_16(CDC_ACM_CMD_PACKET_SIZE),
            .bInterval              = 0x10,
        },
        .cdc_data_itf = 
        {/* Data Class Interface Descriptor */
            .header =
            {
                .bLength            = 0x09,
                .bDescriptorType    = 0x04,
            },
            .bInterfaceNumber       = CDC_DATA_ITF_NUMBER,
            .bAlternateSetting      = 0x00,
            .bNumEndpoints          = 0x02,
            .bInterfaceClass        = 0x0A,
            .bInterfaceSubClass     = 0x00,
            .bInterfaceProtocol     = 0x00,
            .iInterface             = 0x00,
        },
        .end_out =
        {/* Endpoint OUT Descriptor */
            .header =
            {
                .bLength            = 0x07,
                .bDescriptorType    = 0x05,
            },
            .bEndpointAddress       = CDC_OUT_EP,
            .bmAttributes           = 0x02,
            .wMaxPacketSize         = LITTLE_ENDIAN_16(CDC_ACM_DATA_PACKET_SIZE),
            .bInterval              = 0x00,
        },
        .end_in =
        {/* Endpoint IN Descriptor */
            .header =
            {
                .bLength            = 0x07,
                .bDescriptorType    = 0x05,
            },
            .bEndpointAddress       = CDC_IN_EP,
            .bmAttributes           = 0x02,
            .wMaxPacketSize         = LITTLE_ENDIAN_16(CDC_ACM_DATA_PACKET_SIZE),
            .bInterval              = 0x00,
        },
    },
};

static const usb_desc_LANGID usbd_language_id_desc = 
{
    .header = 
     {
         .bLength = sizeof(usb_desc_LANGID), 
         .bDescriptorType = USB_DESCTYPE_STR
     },
    .wLANGID = ENG_LANGID
};

static void* const usbd_strings[] = 
{
    [STR_IDX_LANGID] = (uint8_t *)&usbd_language_id_desc,
    [STR_IDX_MFC] = USBD_STRING_DESC("GigaDevice"),
    [STR_IDX_PRODUCT] = USBD_STRING_DESC("nano_midi"),
    [STR_IDX_SERIAL] = USBD_STRING_DESC("GD32XXX-3.0.0-7z8x9yer")
};

usb_core_driver g_midi_cdc_udev = 
{
    .dev = 
    {
        .desc = 
        {
            .dev_desc = (uint8_t *)&device_descriptor,
            .config_desc = (uint8_t *)&configuration_descriptor, 
            .strings = usbd_strings

        }
    }
};

void midi_cdc_desc_ep_setup(usb_dev *udev)
{
    usbd_ep_setup(udev, &configuration_descriptor.audio.bulk_in_ep_desc.desc_ep);
    usbd_ep_setup(udev, &configuration_descriptor.audio.bulk_out_ep_desc.desc_ep);

    usbd_ep_setup(udev, &configuration_descriptor.cdc.end_in);
    usbd_ep_setup(udev, &configuration_descriptor.cdc.end_out);
    usbd_ep_setup(udev, &configuration_descriptor.cdc.cmd_ep);
}

void midi_cdc_desc_ep_clear(usb_dev *udev)
{
    usbd_ep_clear(udev, configuration_descriptor.audio.bulk_in_ep_desc.desc_ep.bEndpointAddress);
    usbd_ep_clear(udev, configuration_descriptor.audio.bulk_out_ep_desc.desc_ep.bEndpointAddress);

    usbd_ep_clear(udev, configuration_descriptor.cdc.end_in.bEndpointAddress);
    usbd_ep_clear(udev, configuration_descriptor.cdc.end_out.bEndpointAddress);
    usbd_ep_clear(udev, configuration_descriptor.cdc.cmd_ep.bEndpointAddress);
}
