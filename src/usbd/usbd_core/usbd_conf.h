#ifndef __USBD_CONF_H__
#define __USBD_CONF_H__

#include "usb_conf.h"

#define USBD_CFG_MAX_NUM 1


#define CDC_OUT_EP                      0x01U
#define CDC_CMD_EP                      0x82U
#define CDC_IN_EP                       0x81U

#define MIDI_OUT_EP                     0x02U
#define MIDI_IN_EP                      0x83U

#define AUDIO_AC_ITF_NUMBER             0
#define AUDIO_MS_ITF_NUMBER             1
#define CDC_CDC_ITF_NUMBER              2
#define CDC_DATA_ITF_NUMBER             3
#define USBD_ITF_MAX_NUM                CDC_DATA_ITF_NUMBER 


#endif//__USBD_CONF_H__