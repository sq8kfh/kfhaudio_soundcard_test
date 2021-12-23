#ifndef __USBD_AUDIO_IF_H__
#define __USBD_AUDIO_IF_H__

#include <stdint.h>

int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options);
int8_t AUDIO_DeInit_FS(uint32_t options);
int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd);
int8_t AUDIO_VolumeCtl_FS(uint8_t vol);
int8_t AUDIO_MuteCtl_FS(uint8_t cmd);
int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd);
int8_t AUDIO_GetState_FS(void);
uint16_t AUDIO_Get_DMA_Read_Ptr(void);

#endif /* __USBD_AUDIO_IF_H__ */
