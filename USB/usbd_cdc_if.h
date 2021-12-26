#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

#include "usbd_cdc.h"


#define APP_RX_DATA_SIZE  1000
#define APP_TX_DATA_SIZE  1000


int8_t CDC_Init_FS(void);
int8_t CDC_DeInit_FS(void);
int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);


#endif /* __USBD_CDC_IF_H__ */
