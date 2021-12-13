#ifndef __USB_COMPOSITE_H
#define __USB_COMPOSITE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbd_ioreq.h"
#include "usbd_audio.h"
#include "usbd_cdc.h"

#define COMPOSITE_AUDIO_OUT_EP                        0x01U
#define COMPOSITE_AUDIO_IN_EP                         0x81U
#define COMPOSITE_CDC_OUT_EP                          0x02U  /* EP2 for data OUT */
#define COMPOSITE_CDC_IN_EP                           0x82U  /* EP2 for data IN */
#define COMPOSITE_CDC_CMD_EP                          0x83U  /* EP3 for CDC commands */
#define COMPOSITE_EP_COUNT                            5

#define COMPOSITE_AUDIO_CONTROL_INTERFACE             0x00
#define COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE       0x01
#define COMPOSITE_AUDIO_INPUT_STREAM_INTERFACE        0x02
#define COMPOSITE_CDC_MODEM_INTERFACE                 0x03
#define COMPOSITE_CDC_DATA_INTERFACE                  0x04
#define COMPOSITE_INTERFACE_COUNT                     5

extern USBD_ClassTypeDef USBD_Composite;

typedef struct
{
	USBD_CDC_HandleTypeDef   hcdc;
	USBD_AUDIO_HandleTypeDef haudio;
} USBD_Composite_HandleTypeDef;

typedef struct {
	USBD_CDC_ItfTypeDef   *fops_cdc;
	USBD_AUDIO_ItfTypeDef *fops_audio;
} USBD_Composite_ItfTypeDef;

uint8_t USBD_CDC_Audio_Composite_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CDC_ItfTypeDef *fops_cdc, USBD_AUDIO_ItfTypeDef *fops_audio);

#ifdef __cplusplus
}
#endif

#endif
