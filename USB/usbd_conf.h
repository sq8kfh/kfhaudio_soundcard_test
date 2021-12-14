/*
 * usbd_config.h
 *
 *  Created on: Dec 13, 2021
 *      Author: crowx
 */

#ifndef USBD_CONF_H_
#define USBD_CONF_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

//#define USBD_MAX_NUM_INTERFACES        1U
#define USBD_MAX_NUM_CONFIGURATION     1U
#define USBD_MAX_STR_DESC_SIZ          512U
#define USBD_DEBUG_LEVEL               3U
#define USBD_LPM_ENABLED               0U
#define USBD_SELF_POWERED              0U

#define USBD_VID                       1155
#define USBD_LANGID_STRING             1033
#define USBD_MANUFACTURER_STRING       "SQ8KFH"
#define USBD_PID_FS                    22336
#define USBD_PRODUCT_STRING_FS         "KFH Audio"
#define USBD_CONFIGURATION_STRING_FS   "AUDIO Config"
#define USBD_INTERFACE_STRING_FS       "AUDIO Interface"

#define USB_SIZ_BOS_DESC               0x0C


#define COMPOSITE_AUDIO_OUT_EP                        0x01U
#define COMPOSITE_CDC_OUT_EP                          0x02U  /* EP2 for data OUT */
#define COMPOSITE_CDC_IN_EP                           0x82U  /* EP2 for data IN */
#define COMPOSITE_CDC_CMD_EP                          0x83U  /* EP3 for CDC commands */
#define COMPOSITE_EP_COUNT                            4


#define COMPOSITE_AUDIO_CONTROL_INTERFACE             0x00
#define COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE       0x01
#define COMPOSITE_CDC_MODEM_INTERFACE                 0x02
#define COMPOSITE_CDC_DATA_INTERFACE                  0x03

#define COMPOSITE_INTERFACE_COUNT                     4


#define USBD_AUDIO_FREQ                48000U




/* #define for FullSpeed and HighSpeed identification */
#define DEVICE_FS 		0
#define DEVICE_HS 		1



#define USBD_malloc         (void *)USBD_static_malloc

/** Alias for memory release. */
#define USBD_free           USBD_static_free

/** Alias for memory set. */
#define USBD_memset         memset

/** Alias for memory copy. */
#define USBD_memcpy         memcpy

/** Alias for delay. */
#define USBD_Delay          HAL_Delay

/* DEBUG macros */

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...)    printf(__VA_ARGS__);\
                            printf("\r\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define USBD_ErrLog(...)    printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\r\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...)    printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\r\n");
#else
#define USBD_DbgLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 3)
#define USBD_Dbg2Log(...)    printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\r\n");
#else
#define USBD_Dbg2Log(...)
#endif

void *USBD_static_malloc(uint32_t size) __attribute__((deprecated));
void USBD_static_free(void *p) __attribute__((deprecated));


#ifdef __cplusplus
}
#endif

#endif /* USBD_CONF_H_ */
