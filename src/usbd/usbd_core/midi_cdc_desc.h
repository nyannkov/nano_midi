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
#ifndef __MIDI_CDC_DESC_H__
#define __MIDI_CDC_DESC_H__


#define CDC_ACM_CMD_PACKET_SIZE         0x0008  //  8 byte
#define CDC_ACM_DATA_PACKET_SIZE        0x0040  // 64 byte
#define AUDIO_MS_PACKET_SIZE            0x0040  // 64 byte

#pragma pack(1)

typedef  struct
{
    uint8_t             bLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint16_t            bcdADC;
    uint16_t            wTotalLength;
    uint8_t             bInCollection;
    uint8_t             baInterfaceNr;
} usb_desc_class_specific_ac_itf;/* 9 byte */


typedef  struct
{
    uint8_t             bLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint16_t            BcdADC;
    uint16_t            wTotalLength;
} usb_desc_class_specific_ms_itf;/* 7 byte */


typedef struct
{
    uint8_t             bLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint8_t             bJackType;
    uint8_t             bJackID;
    uint8_t             iJack;
} usb_desc_midi_in_jack_desc; /* 6 byte */


typedef struct
{
    uint8_t             bLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint8_t             bJackType;
    uint8_t             bJackID;
    uint8_t             bNtInputPins;
    uint8_t             BaSourceID;
    uint8_t             BaSourcePin;
    uint8_t             iJack;
} usb_desc_midi_out_jack_desc; /* 9 byte */


typedef struct
{
    usb_desc_ep         desc_ep;
    uint8_t             bRefresh;
    uint8_t             bSynchAddress;
} usb_desc_ep_audio; /* 9 byte */


typedef struct
{
    uint8_t             bLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint8_t             bNumEmbMIDIJack;
    uint8_t             BaAssocJackID;
} usb_desc_class_specific_ms_ep; /* 5 byte */


typedef struct
{
    usb_desc_itf                        standard_ac_itf;
    usb_desc_class_specific_ac_itf      class_specific_ac_itf;
    usb_desc_itf                        standard_ms_itf;
    usb_desc_class_specific_ms_itf      class_specific_ms_itf;
    usb_desc_midi_in_jack_desc          midi_in_jack_embedded;
    usb_desc_midi_in_jack_desc          midi_in_jack_external;
    usb_desc_midi_out_jack_desc         midi_out_jack_embedded;
    usb_desc_midi_out_jack_desc         midi_out_jack_external;
    usb_desc_ep_audio                   bulk_out_ep_desc;
    usb_desc_class_specific_ms_ep       class_specific_ms_bulk_out_ep;
    usb_desc_ep_audio                   bulk_in_ep_desc;
    usb_desc_class_specific_ms_ep       class_specific_ms_bulk_in_ep;
} usb_desc_audio;


typedef struct
{
    uint8_t             bLength;
    uint8_t             bDescriptorType;
    uint8_t             bFirstInterface;
    uint8_t             bInterfaceCount;
    uint8_t             bFunctionClass;
    uint8_t             bFunctionSubClass;
    uint8_t             bFunctionProtocol;
    uint8_t             iFunction;
} usb_desc_iad; /* 8 byte  */


typedef struct
{
    uint8_t             bLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint16_t            bcdCDC;
} usb_desc_header_func_desc; /* 5 byte */


typedef struct
{
    uint8_t             bFunctionLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint8_t             bmCapabilities;
    uint8_t             bDataInterface;
} usb_desc_call_mng_func_desc; /* 5 byte */


typedef struct
{
    uint8_t             bFunctionLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint8_t             bmCapabilities;
} usb_desc_acm_func_desc; /* 4 byte */


typedef struct
{
    uint8_t             bFunctionLength;
    uint8_t             bDescriptorType;
    uint8_t             bDescriptorSubtype;
    uint8_t             bMasterInterface;
    uint8_t             bSlaveInterface0;
} usb_desc_union_func_desc; /* 5 byte */


typedef struct
{
    usb_desc_itf                        cdc_cdc_itf;
    usb_desc_header_func_desc           header_func_desc;
    usb_desc_call_mng_func_desc         call_mng_func_desc;
    usb_desc_acm_func_desc              acm_func_desc;
    usb_desc_union_func_desc            union_func_desc;
    usb_desc_ep                         cmd_ep;
    usb_desc_itf                        cdc_data_itf;
    usb_desc_ep                         end_out;
    usb_desc_ep                         end_in;
} usb_desc_cdc;


typedef struct
{
    usb_desc_config                     config;
    usb_desc_iad                        usb_iad_audio;
    usb_desc_audio                      audio;
    usb_desc_iad                        usb_iad_cdc;
    usb_desc_cdc                        cdc;
} usb_descriptor_configuration_set_struct;

#pragma pack()



extern usb_core_driver g_midi_cdc_udev; 

extern void midi_cdc_desc_ep_setup(usb_dev *udev);
extern void midi_cdc_desc_ep_clear(usb_dev *udev);

#endif/*__MIDI_CDC_DESC_H__*/
