#ifndef __USB_DESCRIPTOR_H
#define __USB_DESCRIPTOR_H

#include "usbd_conf.h"
/*
#include "usbd_composite.h"
#include "usbd_desc.h"
#include "usbd_audio.h"
#include "usbd_cdc.h"
#include "usbd_ctlreq.h"
*/
#define COMPOSITE_CONFIGURATION_DESCRIPTOR_DESC_SIZE           9U

#define COMPOSITE_AUDIO_INTERFACE_ASSOCIATION_DESC_SIZE        8U
#define COMPOSITE_AUDIO_CONTROL_INTERFACE_DESC_SIZE            9U
#define COMPOSITE_AUDIOCONTROL_HEADER_DESC_SIZE                9U
#define COMPOSITE_AUDIOCONTROL_USB_INPUT_TERMINAL_DESC_SIZE    12U
#define COMPOSITE_AUDIOCONTROL_FEATURE_UNIT_DESC_SIZE          9U
#define COMPOSITE_AUDIOCONTROL_SPK_OUTPUT_TERMINAL_DESC_SIZE   9U
#define COMPOSITE_AUDIOCONTROL_MIC_INPUT_TERMINAL_DESC_SIZE    12U
#define COMPOSITE_AUDIOCONTROL_USB_OUTPUT_TERMINAL_DESC_SIZE   9U
#define COMPOSITE_AUDIO_STREAM_OUT_INTERFACE_ALT1_DESC_SIZE    9U
#define COMPOSITE_AUDIO_STREAM_OUT_INTERFACE_ALT2_DESC_SIZE    9U
#define COMPOSITE_AUDIOSTREAMING_OUT_INTERFACE_1_DESC_SIZE     7U
#define COMPOSITE_AUDIOSTREAMING_OUT_INTERFACE_2_DESC_SIZE     11U
#define COMPOSITE_AUDIOSTREAMING_OUT_ENDPOINT_DESC_SIZE        9U
#define COMPOSITE_AUDIOSTCONTROL_OUT_ENDPOINT_DESC_SIZE        7U
#define COMPOSITE_AUDIO_STREAM_IN_INTERFACE_ALT1_DESC_SIZE     9U
#define COMPOSITE_AUDIO_STREAM_IN_INTERFACE_ALT2_DESC_SIZE     9U
#define COMPOSITE_AUDIOSTREAMING_IN_INTERFACE_1_DESC_SIZE      7U
#define COMPOSITE_AUDIOSTREAMING_IN_INTERFACE_2_DESC_SIZE      11U
#define COMPOSITE_AUDIOSTREAMING_IN_ENDPOINT_DESC_SIZE         9U
#define COMPOSITE_AUDIOSTCONTROL_IN_ENDPOINT_DESC_SIZE         7U

#define COMPOSITE_CDC_INTERFACE_ASSOCIATION_DESC_SIZE          8U
#define COMPOSITE_CDC_CONTROL_INTERFACE_DESC_SIZE              9U
#define COMPOSITE_CDC_CONTROL_HEADER_DESC_SIZE                 5U
#define COMPOSITE_CDC_CONTROL_CALL_MANAGMENT_DESC_SIZE         5U
#define COMPOSITE_CDC_CONTROL_ACM_DESC_SIZE                    4U
#define COMPOSITE_CDC_CONTROL_UNION_DESC_SIZE                  5U
#define COMPOSITE_CDC_CONTROL_ENDPOINT_DESC_SIZE               7U
#define COMPOSITE_CDC_DATA_INTERFACE_DESC_SIZE                 9U
#define COMPOSITE_CDC_DATA_OUT_ENDPOINT_DESC_SIZE              7U
#define COMPOSITE_CDC_DATA_IN_ENDPOINT_DESC_SIZE               7U


#define COMPOSITE_AUDIOCONTROL_INTERFACE_TOTAL_SIZE (COMPOSITE_AUDIOCONTROL_HEADER_DESC_SIZE \
		                                           + COMPOSITE_AUDIOCONTROL_USB_INPUT_TERMINAL_DESC_SIZE \
												   + COMPOSITE_AUDIOCONTROL_FEATURE_UNIT_DESC_SIZE \
												   + COMPOSITE_AUDIOCONTROL_SPK_OUTPUT_TERMINAL_DESC_SIZE)

#define CDC_AUDIO_COMPOSITE_DESCRIPTOR_SIZE         (COMPOSITE_CONFIGURATION_DESCRIPTOR_DESC_SIZE \
                                                   + COMPOSITE_AUDIO_INTERFACE_ASSOCIATION_DESC_SIZE \
                                                   + COMPOSITE_AUDIO_CONTROL_INTERFACE_DESC_SIZE \
                                                   + COMPOSITE_AUDIOCONTROL_HEADER_DESC_SIZE \
                                                   + COMPOSITE_AUDIOCONTROL_USB_INPUT_TERMINAL_DESC_SIZE \
                                                   + COMPOSITE_AUDIOCONTROL_FEATURE_UNIT_DESC_SIZE \
                                                   + COMPOSITE_AUDIOCONTROL_USB_OUTPUT_TERMINAL_DESC_SIZE \
                                                   + COMPOSITE_AUDIO_STREAM_OUT_INTERFACE_ALT1_DESC_SIZE \
                                                   + COMPOSITE_AUDIO_STREAM_OUT_INTERFACE_ALT2_DESC_SIZE \
                                                   + COMPOSITE_AUDIOSTREAMING_OUT_INTERFACE_1_DESC_SIZE \
                                                   + COMPOSITE_AUDIOSTREAMING_OUT_INTERFACE_2_DESC_SIZE \
                                                   + COMPOSITE_AUDIOSTREAMING_OUT_ENDPOINT_DESC_SIZE \
                                                   + COMPOSITE_AUDIOSTCONTROL_OUT_ENDPOINT_DESC_SIZE \
                                                   + COMPOSITE_CDC_INTERFACE_ASSOCIATION_DESC_SIZE \
                                                   + COMPOSITE_CDC_CONTROL_INTERFACE_DESC_SIZE \
                                                   + COMPOSITE_CDC_CONTROL_HEADER_DESC_SIZE \
                                                   + COMPOSITE_CDC_CONTROL_CALL_MANAGMENT_DESC_SIZE \
                                                   + COMPOSITE_CDC_CONTROL_ACM_DESC_SIZE \
                                                   + COMPOSITE_CDC_CONTROL_UNION_DESC_SIZE \
                                                   + COMPOSITE_CDC_CONTROL_ENDPOINT_DESC_SIZE \
                                                   + COMPOSITE_CDC_DATA_INTERFACE_DESC_SIZE \
                                                   + COMPOSITE_CDC_DATA_OUT_ENDPOINT_DESC_SIZE \
                                                   + COMPOSITE_CDC_DATA_IN_ENDPOINT_DESC_SIZE)



#define COMPOSITE_AUDIOCONTROL_USB_INPUT_TERMINAL_ID           1U
#define COMPOSITE_AUDIOCONTROL_FEATURE_UNIT_ID                 2U
#define COMPOSITE_AUDIOCONTROL_SPK_OUTPUT_TERMINAL_ID          3U
//#define COMPOSITE_AUDIOCONTROL_MIN_INPUT_TERMINAL_ID           4U
//#define COMPOSITE_AUDIOCONTROL_USB_OUTPUT_TERMINAL_ID          5U

extern uint8_t USBD_CDC_Audio_Composite_Descriptor[CDC_AUDIO_COMPOSITE_DESCRIPTOR_SIZE];

#endif
