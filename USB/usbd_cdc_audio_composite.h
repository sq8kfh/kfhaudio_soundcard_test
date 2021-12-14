#ifndef __USB_COMPOSITE_H
#define __USB_COMPOSITE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbd_ioreq.h"
#include "usbd_audio.h"
#include "usbd_cdc.h"


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
