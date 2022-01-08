#include "usbd_descriptor.h"
#include "usbd_conf.h"
#include "usbd_cdc.h"
#include "usbd_audio.h"


#define AUDIO_SAMPLE_FREQ(frq)         (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define  USB_DESC_TYPE_DEVICE                           0x01U
#define  USB_DESC_TYPE_CONFIGURATION                    0x02U
#define  USB_DESC_TYPE_STRING                           0x03U
#define  USB_DESC_TYPE_INTERFACE                        0x04U
#define  USB_DESC_TYPE_ENDPOINT                         0x05U
#define  USB_DESC_TYPE_DEVICE_QUALIFIER                 0x06U
#define  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION        0x07U
#define  USB_DESC_TYPE_INTERFACE_ASSOCIATION            0x0BU
#define  USB_DESC_TYPE_BOS                              0x0FU

#define USBD_EP_SYNCH_TYPE_ASYNCHRONOUS                 0x04U

__ALIGN_BEGIN uint8_t USBD_CDC_Audio_Composite_Descriptor[CDC_AUDIO_COMPOSITE_DESCRIPTOR_SIZE] __ALIGN_END = {
    /* Configuration 1 */
	COMPOSITE_CONFIGURATION_DESCRIPTOR_DESC_SIZE,         /* bLength */
	USB_DESC_TYPE_CONFIGURATION,                          /* bDescriptorType */
	LOBYTE(CDC_AUDIO_COMPOSITE_DESCRIPTOR_SIZE),          /* wTotalLength */
	HIBYTE(CDC_AUDIO_COMPOSITE_DESCRIPTOR_SIZE),
	COMPOSITE_INTERFACE_COUNT,                            /* bNumInterfaces */
	0x01,                                                 /* bConfigurationValue */
	0x00,                                                 /* iConfiguration */
	0x80,                                                 /* bmAttributes  BUS Powred*/
	0x32,                                                 /* bMaxPower = 100 mA*/
	/* 09 byte*/

	/*
	 * AUDIO
	*/

	/* IAD - Interface Association - AUDIO */
	COMPOSITE_AUDIO_INTERFACE_ASSOCIATION_DESC_SIZE,      /* bLength */
	USB_DESC_TYPE_INTERFACE_ASSOCIATION,                  /* bDescriptorType */
	COMPOSITE_AUDIO_CONTROL_INTERFACE,                    /* bFirstInterface */
#ifdef USB_AUDIO_MIC_ON
	0x03,                                                 /* bInterfaceCount */
#else
	0x02,                                                 /* bInterfaceCount */
#endif
	USB_DEVICE_CLASS_AUDIO,                               /* bFunctionClass */
	0x00,                                                 /* bFunctionSubClass */
	AUDIO_PROTOCOL_UNDEFINED,                             /* bFunctionProtocol */
	0x02,                                                 /* iFunction */
	/* 08 byte*/

	/* AudioControl - Interface Descriptor */
	COMPOSITE_AUDIO_CONTROL_INTERFACE_DESC_SIZE,          /* bLength */
	USB_DESC_TYPE_INTERFACE,                              /* bDescriptorType */
	COMPOSITE_AUDIO_CONTROL_INTERFACE,                    /* bInterfaceNumber */
	0x00,                                                 /* bAlternateSetting */
	0x00,                                                 /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,                               /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOCONTROL,                          /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED,                             /* bInterfaceProtocol */
	0x00,                                                 /* iInterface */
	/* 09 byte*/

	/* AudioControl - HEADER - Interface Descriptor */
	COMPOSITE_AUDIOCONTROL_HEADER_DESC_SIZE,              /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_CONTROL_HEADER,                                 /* bDescriptorSubtype */
	0x00,                                                 /* bcdADC 0x0100 == 1.00*/
	0x01,
	LOBYTE(COMPOSITE_AUDIOCONTROL_INTERFACE_TOTAL_SIZE),  /* wTotalLength */
	HIBYTE(COMPOSITE_AUDIOCONTROL_INTERFACE_TOTAL_SIZE),
#ifdef USB_AUDIO_MIC_ON
	0x02,                                                 /* bInCollection */
	COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE,              /* baInterfaceNr 0 */
	COMPOSITE_AUDIO_INPUT_STREAM_INTERFACE,               /* baInterfaceNr 1 */
#else
	0x01,                                                 /* bInCollection */
	COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE,              /* baInterfaceNr */
#endif
	//COMPOSITE_AUDIO_INPUT_STREAM_INTERFACE,
	/* 09 byte*/

	/* AudioControl - Speaker - USB Streaming Terminal Descriptor */
	COMPOSITE_AUDIOCONTROL_USB_INPUT_TERMINAL_DESC_SIZE,  /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_CONTROL_INPUT_TERMINAL,                         /* bDescriptorSubtype */
	COMPOSITE_AUDIOCONTROL_USB_INPUT_TERMINAL_ID,         /* bTerminalID */
	0x01,                                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
	0x01,
	0x00,                                                 /* bAssocTerminal */
	0x02,                                                 /* bNrChannels */
	0x03,                                                 /* wChannelConfig 0x0003 left right */
	0x00,
	0x00,                                                 /* iChannelNames */
	0x00,                                                 /* iTerminal */
	/* 12 byte*/

	/* AudioControl - Speaker - Feature Unit Descriptor */
	COMPOSITE_AUDIOCONTROL_FEATURE_UNIT_DESC_SIZE,        /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_CONTROL_FEATURE_UNIT,                           /* bDescriptorSubtype */
	COMPOSITE_AUDIOCONTROL_SPK_FEATURE_UNIT_ID,           /* bUnitID */
	COMPOSITE_AUDIOCONTROL_USB_INPUT_TERMINAL_ID,         /* bSourceID */
	0x01,                                                 /* bControlSize */
	AUDIO_CONTROL_MUTE,                                   /* bmaControls(0) */
	0,                                                    /* bmaControls(1) */
	0x00,                                                 /* iTerminal */
	/* 09 byte*/

	/* AudioControl - Speaker - Output Terminal Descriptor */
	COMPOSITE_AUDIOCONTROL_SPK_OUTPUT_TERMINAL_DESC_SIZE, /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_CONTROL_OUTPUT_TERMINAL,                        /* bDescriptorSubtype */
	COMPOSITE_AUDIOCONTROL_SPK_OUTPUT_TERMINAL_ID,        /* bTerminalID */
	0x01, //0x11,                                         /* wTerminalType  0x0301 - Speaker*/
	0x03, //0x07,
	0x00,                                                 /* bAssocTerminal */
	COMPOSITE_AUDIOCONTROL_SPK_FEATURE_UNIT_ID,           /* bSourceID */
	0x00,                                                 /* iTerminal */
	/* 09 byte*/
#ifdef USB_AUDIO_MIC_ON
	/* AudioControl - LineIN - Input Terminal Descriptor */
	COMPOSITE_AUDIOCONTROL_MIC_INPUT_TERMINAL_DESC_SIZE,  /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_CONTROL_INPUT_TERMINAL,                         /* bDescriptorSubtype */
	COMPOSITE_AUDIOCONTROL_MIC_INPUT_TERMINAL_ID,         /* bTerminalID */
	0x01,                                                 /* wTerminalType  0x0201*/
	0x02,
	0x00,                                                 /* bAssocTerminal */
	0x02,                                                 /* bNrChannels */
	0x03,                                                 /* wChannelConfig 0x0003  left right*/
	0x00,
	0x00,                                                 /* iChannelNames */
	0x00,                                                 /* iTerminal */
	/* 12 byte*/

	/* AudioControl - Speaker - Feature Unit Descriptor */
	COMPOSITE_AUDIOCONTROL_MIC_FEATURE_UNIT_DESC_SIZE,    /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_CONTROL_FEATURE_UNIT,                           /* bDescriptorSubtype */
	COMPOSITE_AUDIOCONTROL_MIC_FEATURE_UNIT_ID,           /* bUnitID */
	COMPOSITE_AUDIOCONTROL_MIC_INPUT_TERMINAL_ID,         /* bSourceID */
	0x01,                                                 /* bControlSize */
	AUDIO_CONTROL_MUTE,                                   /* bmaControls(0) */
	0,                                                    /* bmaControls(1) */
	0x00,                                                 /* iTerminal */
	/* 09 byte*/

	/* AudioControl - LineIN - USB Streaming Terminal Descriptor */
	COMPOSITE_AUDIOCONTROL_USB_OUTPUT_TERMINAL_DESC_SIZE, /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_CONTROL_OUTPUT_TERMINAL,                        /* bDescriptorSubtype */
	COMPOSITE_AUDIOCONTROL_USB_OUTPUT_TERMINAL_ID,        /* bTerminalID */
	0x01,                                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
	0x01,
	0x00,                                                 /* bAssocTerminal */
	COMPOSITE_AUDIOCONTROL_MIC_FEATURE_UNIT_ID,           /* bSourceID */
	0x00,                                                 /* iTerminal */
	/* 9 byte*/
#endif

	/* Audio Speaker - Interface Descriptor */
	/* Interface 1, Alternate Setting 0 */
	COMPOSITE_AUDIO_STREAM_OUT_INTERFACE_ALT1_DESC_SIZE,  /* bLength */
	USB_DESC_TYPE_INTERFACE,                              /* bDescriptorType */
	COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE,              /* bInterfaceNumber */
	0x00,                                                 /* bAlternateSetting */
	0x00,                                                 /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,                               /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOSTREAMING,                        /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED,                             /* bInterfaceProtocol */
	0x00,                                                 /* iInterface */
	/* 09 byte*/

	/* Audio Speaker - Interface Descriptor */
	/* Interface 1, Alternate Setting 1 */
	COMPOSITE_AUDIO_STREAM_OUT_INTERFACE_ALT2_DESC_SIZE,  /* bLength */
	USB_DESC_TYPE_INTERFACE,                              /* bDescriptorType */
	COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE,              /* bInterfaceNumber */
	0x01,                                                 /* bAlternateSetting */
	0x02,                                                 /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,                               /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOSTREAMING,                        /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED,                             /* bInterfaceProtocol */
	0x00,                                                 /* iInterface */
	/* 09 byte*/

	/* Audio Speaker - Streaming Interface Descriptor */
	COMPOSITE_AUDIOSTREAMING_OUT_INTERFACE_1_DESC_SIZE,   /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_STREAMING_GENERAL,                              /* bDescriptorSubtype */
	COMPOSITE_AUDIOCONTROL_USB_INPUT_TERMINAL_ID,         /* bTerminalLink */
	0x01,                                                 /* bDelay */
	0x01,                                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
	0x00,
	/* 07 byte*/

	/* Audio Speaker - Streaming Format Descriptor */
	COMPOSITE_AUDIOSTREAMING_OUT_INTERFACE_2_DESC_SIZE,   /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_STREAMING_FORMAT_TYPE,                          /* bDescriptorSubtype */
	AUDIO_FORMAT_TYPE_I,                                  /* bFormatType */
	0x02,                                                 /* bNrChannels */
	0x04,                                                 /* bSubFrameSize :  4 Bytes per frame (32bits) */
	24,                                                   /* bBitResolution (24-bits per sample) */
	0x01,                                                 /* bSamFreqType only one frequency supported */
	AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),                   /* Audio sampling frequency coded on 3 bytes */
	/* 11 byte*/

	/* Audio Speaker - Audio Data Endpoint Descriptor */
	/* audio10.pdf page: 61*/
	COMPOSITE_AUDIOSTREAMING_OUT_ENDPOINT_DESC_SIZE,     /* bLength */
	USB_DESC_TYPE_ENDPOINT,                              /* bDescriptorType */
	COMPOSITE_AUDIO_OUT_EP,                              /* bEndpointAddress 1 out endpoint*/
	USBD_EP_TYPE_ISOC | USBD_EP_SYNCH_TYPE_ASYNCHRONOUS, /* bmAttributes */
	(uint8_t)(AUDIO_OUT_MAX_PACKET & 0xFFU),             /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
	(uint8_t)((AUDIO_OUT_MAX_PACKET >> 8) & 0xFFU),
	0x01,                                                /* bInterval */
	0x00,                                                /* bRefresh */
	COMPOSITE_AUDIO_OUT_SYNCH_EP,                        /* bSynchAddress */
	/* 09 byte*/

	/* Audio Speaker - Audio Data Endpoint Streaming Descriptor */
	COMPOSITE_AUDIOSTCONTROL_OUT_ENDPOINT_DESC_SIZE,     /* bLength */
	AUDIO_ENDPOINT_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_ENDPOINT_GENERAL,                              /* bDescriptor */
	0x00,                                                /* bmAttributes */
	0x00,                                                /* bLockDelayUnits */
	0x00,                                                /* wLockDelay */
	0x00,
	/* 07 byte*/

	/* Audio Speaker - Audio Sync Endpoint Descriptor */
	/* audio10.pdf page: 61*/
	COMPOSITE_AUDIOSTREAMING_OUT_SYNC_ENDPOINT_DESC_SIZE,/* bLength */
	USB_DESC_TYPE_ENDPOINT,                              /* bDescriptorType */
	COMPOSITE_AUDIO_OUT_SYNCH_EP,                        /* bEndpointAddress 1 out endpoint*/
	USBD_EP_TYPE_ISOC,                                   /* bmAttributes */
	0x03,                                                /* wMaxPacketSize ? */
	0x00,
	0x01,                                                /* bInterval */
	USBD_AUDIO_FEEDBACK_SOF_RATE,                        /* bRefresh */
	0x00,                                                /* bSynchAddress */
	/* 09 byte*/

#ifdef USB_AUDIO_MIC_ON
	/* Audio LineIN - Interface Descriptor */
	/* Interface 2, Alternate Setting 0 */
	COMPOSITE_AUDIO_STREAM_IN_INTERFACE_ALT1_DESC_SIZE,  /* bLength */
	USB_DESC_TYPE_INTERFACE,                             /* bDescriptorType */
	COMPOSITE_AUDIO_INPUT_STREAM_INTERFACE,              /* bInterfaceNumber */
	0x00,                                                /* bAlternateSetting */
	0x00,                                                /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,                              /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOSTREAMING,                       /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED,                            /* bInterfaceProtocol */
	0x00,                                                /* iInterface */
	/* 09 byte*/

	/* Audio LineIN - Interface Descriptor */
	/* Interface 2, Alternate Setting 1 */
	COMPOSITE_AUDIO_STREAM_IN_INTERFACE_ALT2_DESC_SIZE,  /* bLength */
	USB_DESC_TYPE_INTERFACE,                             /* bDescriptorType */
	COMPOSITE_AUDIO_INPUT_STREAM_INTERFACE,              /* bInterfaceNumber */
	0x01,                                                /* bAlternateSetting */
	0x01,                                                /* bNumEndpoints */
	USB_DEVICE_CLASS_AUDIO,                              /* bInterfaceClass */
	AUDIO_SUBCLASS_AUDIOSTREAMING,                       /* bInterfaceSubClass */
	AUDIO_PROTOCOL_UNDEFINED,                            /* bInterfaceProtocol */
	0x00,                                                /* iInterface */
	/* 09 byte*/

	/* Audio LineIN - Streaming Interface Descriptor */
	COMPOSITE_AUDIOSTREAMING_IN_INTERFACE_1_DESC_SIZE,   /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                     /* bDescriptorType */
	AUDIO_STREAMING_GENERAL,                             /* bDescriptorSubtype */
	COMPOSITE_AUDIOCONTROL_USB_OUTPUT_TERMINAL_ID,       /* bTerminalLink */
	0x01,                                                /* bDelay */
	0x01,                                                /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
	0x00,
	/* 07 byte*/

	/* Audio LineIN - Streaming Format Descriptor */
	COMPOSITE_AUDIOSTREAMING_IN_INTERFACE_2_DESC_SIZE,   /* bLength */
	AUDIO_INTERFACE_DESCRIPTOR_TYPE,                     /* bDescriptorType */
	AUDIO_STREAMING_FORMAT_TYPE,                         /* bDescriptorSubtype */
	AUDIO_FORMAT_TYPE_I,                                 /* bFormatType */
	0x02,                                                /* bNrChannels */
	0x04,                                                /* bSubFrameSize :  4 Bytes per frame (32bits) */
	24,                                                  /* bBitResolution (24-bits per sample) */
	0x01,                                                /* bSamFreqType only one frequency supported */
	AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),                  /* Audio sampling frequency coded on 3 bytes */
	/* 11 byte*/

	/* Audio LineIN - Audio Data Endpoint Descriptor */
	/* audio10.pdf page: 61*/
	COMPOSITE_AUDIOSTREAMING_IN_ENDPOINT_DESC_SIZE,      /* bLength */
	USB_DESC_TYPE_ENDPOINT,                              /* bDescriptorType */
	COMPOSITE_AUDIO_IN_EP,                               /* bEndpointAddress 1 in endpoint*/
	USBD_EP_TYPE_ISOC,                                   /* bmAttributes */
	(uint8_t)(AUDIO_OUT_PACKET & 0xFFU),                 /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
	(uint8_t)((AUDIO_OUT_PACKET >> 8) & 0xFFU),
	0x01,                                                /* bInterval */
	0x00,                                                /* bRefresh */
	0x00,                                                /* bSynchAddress */
	/* 09 byte*/

	/* Audio LineIN - Audio Data Endpoint Streaming Descriptor */
	COMPOSITE_AUDIOSTCONTROL_IN_ENDPOINT_DESC_SIZE,      /* bLength */
	AUDIO_ENDPOINT_DESCRIPTOR_TYPE,                      /* bDescriptorType */
	AUDIO_ENDPOINT_GENERAL,                              /* bDescriptor */
	0x00,                                                /* bmAttributes */
	0x00,                                                /* bLockDelayUnits */
	0x00,                                                /* wLockDelay */
	0x00,
	/* 07 byte*/
#endif

	/*
	 * CDC
	 */

	/* Interface Association */
	COMPOSITE_CDC_INTERFACE_ASSOCIATION_DESC_SIZE,       /* bLength */
	0x0B,                                                /* bDescriptorType */
	COMPOSITE_CDC_MODEM_INTERFACE,                       /* bFirstInterface */
	0x02,                                                /* bInterfaceCount */
	0x02,                                                /* bFunctionClass */
	0x02,                                                /* bFunctionSubClass */
	0x00,                                                /* bFunctionProtocol */
	0x00,                                                /* iFunction */

	/*Interface Descriptor */
	COMPOSITE_CDC_CONTROL_INTERFACE_DESC_SIZE,           /* bLength */
	USB_DESC_TYPE_INTERFACE,                             /* bDescriptorType: Interface */
	COMPOSITE_CDC_MODEM_INTERFACE,                       /* bInterfaceNumber: Number of Interface */
	0x00,                                                /* bAlternateSetting: Alternate setting */
	0x01,                                                /* bNumEndpoints: One endpoints used */
	0x02,                                                /* bInterfaceClass: Communication Interface Class */
	0x02,                                                /* bInterfaceSubClass: Abstract Control Model */
	0x00,                                                /* bInterfaceProtocol */
	0x00,                                                /* iInterface: */

	/*Header Functional Descriptor*/
	COMPOSITE_CDC_CONTROL_HEADER_DESC_SIZE,              /* bLength: Endpoint Descriptor size */
	0x24,                                                /* bDescriptorType: CS_INTERFACE */
	0x00,                                                /* bDescriptorSubtype: Header Func Desc */
	0x10,                                                /* bcdCDC: 1.10 */
	0x01,

	/*Call Management Functional Descriptor*/
	COMPOSITE_CDC_CONTROL_CALL_MANAGMENT_DESC_SIZE,      /* bFunctionLength */
	0x24,                                                /* bDescriptorType: CS_INTERFACE */
	0x01,                                                /* bDescriptorSubtype: Call Management Func Desc */
	0x03,                                                /* bmCapabilities: D0+D1 */
	COMPOSITE_CDC_DATA_INTERFACE,                        /* bDataInterface: 1 */

	/*ACM Functional Descriptor usbcdc11.pdf page: 36*/
	COMPOSITE_CDC_CONTROL_ACM_DESC_SIZE,                 /* bFunctionLength */
	0x24,                                                /* bDescriptorType: CS_INTERFACE */
	0x02,                                                /* bDescriptorSubtype: Abstract Control Management desc */
	0x02,                                                /* bmCapabilities */

	/*Union Functional Descriptor*/
	COMPOSITE_CDC_CONTROL_UNION_DESC_SIZE,               /* bFunctionLength */
	0x24,                                                /* bDescriptorType: CS_INTERFACE */
	0x06,                                                /* bDescriptorSubtype: Union func desc */
	COMPOSITE_CDC_MODEM_INTERFACE,                       /* bMasterInterface: Communication class interface */
	COMPOSITE_CDC_DATA_INTERFACE,                        /* bSlaveInterface0: Data Class Interface */

	/*Endpoint 2 Descriptor*/
	COMPOSITE_CDC_CONTROL_ENDPOINT_DESC_SIZE,            /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,                              /* bDescriptorType: Endpoint */
	COMPOSITE_CDC_CMD_EP,                                /* bEndpointAddress */
	0x03,                                                /* bmAttributes: Interrupt */
	LOBYTE(CDC_CMD_PACKET_SIZE),                         /* wMaxPacketSize: */
	HIBYTE(CDC_CMD_PACKET_SIZE),
	CDC_FS_BINTERVAL,                                    /* bInterval: */

	/*Data class interface descriptor*/
	COMPOSITE_CDC_DATA_INTERFACE_DESC_SIZE,              /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_INTERFACE,                             /* bDescriptorType: */
	COMPOSITE_CDC_DATA_INTERFACE,                        /* bInterfaceNumber: Number of Interface */
	0x00,                                                /* bAlternateSetting: Alternate setting */
	0x02,                                                /* bNumEndpoints: Two endpoints used */
	0x0A,                                                /* bInterfaceClass: CDC */
	0x00,                                                /* bInterfaceSubClass: */
	0x00,                                                /* bInterfaceProtocol: */
	0x00,                                                /* iInterface: */

	/*Endpoint OUT Descriptor*/
	COMPOSITE_CDC_DATA_OUT_ENDPOINT_DESC_SIZE,           /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,                              /* bDescriptorType: Endpoint */
	COMPOSITE_CDC_OUT_EP,                                /* bEndpointAddress */
	0x02,                                                /* bmAttributes: Bulk */
	LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),                 /* wMaxPacketSize: */
	HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
	0x00,                                                /* bInterval: ignore for Bulk transfer */

	/*Endpoint IN Descriptor*/
	COMPOSITE_CDC_DATA_IN_ENDPOINT_DESC_SIZE,            /* bLength: Endpoint Descriptor size */
	USB_DESC_TYPE_ENDPOINT,                              /* bDescriptorType: Endpoint */
	COMPOSITE_CDC_IN_EP,                                 /* bEndpointAddress */
	0x02,                                                /* bmAttributes: Bulk */
	LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),                 /* wMaxPacketSize: */
	HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
	0x00,                                                /* bInterval: ignore for Bulk transfer */
};
