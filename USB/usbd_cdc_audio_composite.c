#include "usbd_cdc_audio_composite.h"
#include "usbd_conf.h"
#include "usbd_descriptor.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_i2s.h"


static uint8_t USBD_CDC_Audio_Composite_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CDC_Audio_Composite_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CDC_Audio_Composite_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_CDC_Audio_Composite_EP0_TxSent(USBD_HandleTypeDef *pdev);
static uint8_t USBD_CDC_Audio_Composite_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_CDC_Audio_Composite_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CDC_Audio_Composite_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CDC_Audio_Composite_SOF(USBD_HandleTypeDef *pdev);
static uint8_t USBD_CDC_Audio_Composite_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CDC_Audio_Composite_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_CDC_Audio_Composite_GetCfgDesc(uint16_t *length);
//static uint8_t *USBD_CDC_Audio_Composite_GetHSCfgDesc(uint16_t *length);
//static uint8_t *USBD_CDC_Audio_Composite_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_CDC_Audio_Composite_GetDeviceQualifierDescriptor(uint16_t *length);

static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_OUT_StopAndReset(USBD_HandleTypeDef* pdev);
static void AUDIO_OUT_Restart(USBD_HandleTypeDef* pdev);

volatile uint32_t tx_flag = 1;
volatile uint32_t fnsof = 0;


USBD_ClassTypeDef USBD_Composite = {
    USBD_CDC_Audio_Composite_Init,
	USBD_CDC_Audio_Composite_DeInit,
	USBD_CDC_Audio_Composite_Setup,
	USBD_CDC_Audio_Composite_EP0_TxSent,
	USBD_CDC_Audio_Composite_EP0_RxReady,
	USBD_CDC_Audio_Composite_DataIn,
	USBD_CDC_Audio_Composite_DataOut,
	USBD_CDC_Audio_Composite_SOF,
	USBD_CDC_Audio_Composite_IsoINIncomplete,
	USBD_CDC_Audio_Composite_IsoOutIncomplete,
	USBD_CDC_Audio_Composite_GetCfgDesc,
	USBD_CDC_Audio_Composite_GetCfgDesc,
	USBD_CDC_Audio_Composite_GetCfgDesc,
	USBD_CDC_Audio_Composite_GetDeviceQualifierDescriptor,
};

 USBD_Composite_HandleTypeDef hcomposite;



 USBD_Composite_ItfTypeDef fops_composite;

 extern I2S_HandleTypeDef hi2s3;
 volatile uint32_t feedback_data __attribute__ ((aligned(4))) = 0xC0000;
 volatile uint8_t feedbacktable[3];
 uint16_t last_dma = 0;
 extern USBD_HandleTypeDef hUsbDeviceFS;

 uint8_t USBD_CDC_Audio_Composite_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CDC_ItfTypeDef *fops_cdc, USBD_AUDIO_ItfTypeDef *fops_audio) {
	fops_composite.fops_cdc = fops_cdc;
	fops_composite.fops_audio = fops_audio;

	pdev->pUserData = &fops_composite;
	return USBD_OK;
}

static uint8_t USBD_CDC_Audio_Composite_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
	USBD_DbgLog("USBD_Composite_Init cfgidx: %d" ,cfgidx);
	USBD_AUDIO_HandleTypeDef *haudio;
	USBD_CDC_HandleTypeDef   *hcdc;

	pdev->pClassData = &hcomposite;

// AUDIO
    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, COMPOSITE_AUDIO_OUT_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET + 4);
    pdev->ep_out[COMPOSITE_AUDIO_OUT_EP & 0xFU].is_used = 1U;
    pdev->ep_out[COMPOSITE_AUDIO_OUT_EP & 0xFU].bInterval = AUDIO_FS_BINTERVAL;

    USBD_LL_OpenEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP, USBD_EP_TYPE_ISOC, 3U);
    pdev->ep_in[COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU].is_used = 1U;
    pdev->ep_in[COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU].bInterval = AUDIO_FS_BINTERVAL;

    USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);
    tx_flag = 1U;


#ifdef USB_AUDIO_MIC_ON
    USBD_LL_OpenEP(pdev, COMPOSITE_AUDIO_IN_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET);
    pdev->ep_in[COMPOSITE_AUDIO_IN_EP & 0xFU].is_used = 1U;
#endif

    /* Allocate Audio structure */
    //haudio = (USBD_AUDIO_HandleTypeDef*)USBD_malloc(sizeof(USBD_AUDIO_HandleTypeDef));
    haudio = &hcomposite.haudio;

    haudio->alt_setting = 0U;
    haudio->alt_input_setting = 0U;
    haudio->offset = AUDIO_OFFSET_UNKNOWN;
    haudio->wr_ptr = 0U;
    haudio->rd_ptr = 0U;
    haudio->rd_enable = 0U;
    feedback_data = 0xC0000;

    /* Initialize the Audio output Hardware layer */
    if (((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_audio->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0U) != 0) {
      	return USBD_FAIL;
    }

    /* Prepare Out endpoint to receive 1st packet */
    USBD_LL_PrepareReceive(pdev, COMPOSITE_AUDIO_OUT_EP, haudio->buffer, AUDIO_OUT_PACKET + 4);

    //extern I2S_HandleTypeDef hi2s3;
    //HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)&haudio->buffer[0], AUDIO_TOTAL_BUF_SIZE / 2U);

// CDC

    /* Open EP IN */
    USBD_LL_OpenEP(pdev, COMPOSITE_CDC_IN_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_IN_PACKET_SIZE);
    pdev->ep_in[COMPOSITE_CDC_IN_EP & 0xFU].is_used = 1U;

    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, COMPOSITE_CDC_OUT_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_OUT_PACKET_SIZE);
    pdev->ep_out[COMPOSITE_CDC_OUT_EP & 0xFU].is_used = 1U;

    /* Open EP OUT - CMD */
    USBD_LL_OpenEP(pdev, COMPOSITE_CDC_CMD_EP, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE);
    pdev->ep_in[COMPOSITE_CDC_CMD_EP & 0xFU].is_used = 1U;
    pdev->ep_in[COMPOSITE_CDC_CMD_EP & 0xFU].bInterval = CDC_FS_BINTERVAL;

    //hcdc = (USBD_CDC_HandleTypeDef*)USBD_malloc(sizeof(USBD_CDC_HandleTypeDef));
    hcdc = &hcomposite.hcdc;

    hcdc->CmdLength = 0;
    hcdc->CmdOpCode = 0;
    hcdc->RxBuffer = NULL;
    hcdc->RxLength = 0;
    hcdc->TxBuffer = NULL;
    hcdc->TxLength = 0;

    /* Init  physical Interface components */
    ((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_cdc->Init();

    /* Init Xfer states */
    hcdc->TxState = 0U;
    hcdc->RxState = 0U;

    /* Prepare CDC Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev, COMPOSITE_CDC_OUT_EP, hcdc->RxBuffer, CDC_DATA_FS_OUT_PACKET_SIZE);

	return USBD_OK;
}

static uint8_t  USBD_CDC_Audio_Composite_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
	USBD_DbgLog("USBD_Composite_DeInit cfgidx: %d" ,cfgidx);
//CDC
	/* Close EP IN */
	USBD_LL_CloseEP(pdev, COMPOSITE_CDC_IN_EP);
	pdev->ep_in[COMPOSITE_CDC_IN_EP & 0xFU].is_used = 0U;

	/* Close EP OUT */
	USBD_LL_CloseEP(pdev, COMPOSITE_CDC_OUT_EP);
	pdev->ep_out[COMPOSITE_CDC_OUT_EP & 0xFU].is_used = 0U;

	  /* Close Command IN EP */
	USBD_LL_CloseEP(pdev, COMPOSITE_CDC_CMD_EP);
	pdev->ep_in[COMPOSITE_CDC_CMD_EP & 0xFU].is_used = 0U;
	pdev->ep_in[COMPOSITE_CDC_CMD_EP & 0xFU].bInterval = 0U;

	/* DeInit  physical Interface components */
	if (pdev->pClassData != NULL) {
		((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_cdc->DeInit();

	    //USBD_free(((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcdc);
	    //((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcdc = NULL;
	}

//AUDIO
	//USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_EP);
	//USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);

	USBD_LL_CloseEP(pdev, COMPOSITE_AUDIO_OUT_EP);
	pdev->ep_out[COMPOSITE_AUDIO_OUT_EP & 0xFU].is_used = 0U;
	pdev->ep_out[COMPOSITE_AUDIO_OUT_EP & 0xFU].bInterval = 0U;

	USBD_LL_CloseEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);
	pdev->ep_in[COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU].is_used = 0U;
	pdev->ep_in[COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU].bInterval = 0U;

	/* DeInit  physical Interface components */
	if (pdev->pClassData != NULL) {
		((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_audio->DeInit(0U);
	}

	pdev->pClassData = NULL;
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	USBD_DbgLog("USBD_Composite_Setup bmRequest: %u bRequest: %u wValue: %u wIndex: %u wLength: %u", req->bmRequest, req->bRequest, req->wValue, req->wIndex, req->wLength);

	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;
	USBD_CDC_HandleTypeDef   *hcdc = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcdc;

	uint16_t status_info = 0U;
	uint8_t ret = USBD_OK;
	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
		case USB_REQ_TYPE_CLASS:	//bmRequest==161
			switch (req->bRequest) {
		    	case CDC_SEND_ENCAPSULATED_COMMAND:
		    	//case CDC_GET_ENCAPSULATED_RESPONSE:
		    	case CDC_SET_COMM_FEATURE:
		    	case CDC_GET_COMM_FEATURE:
		   		case CDC_CLEAR_COMM_FEATURE:
		   		case CDC_SET_LINE_CODING:
		    	case CDC_GET_LINE_CODING:
		    	case CDC_SET_CONTROL_LINE_STATE:
		    	case CDC_SEND_BREAK:
		    		//USBD_DbgLog("USBD_Composite_Setup - CDC");
		    		if (req->wLength) {
		    			if (req->bmRequest & 0x80U) {
		    		    	((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_cdc->Control(req->bRequest,
		    	                                                            (uint8_t *)(void *)hcdc->data,
		    	                                                            req->wLength);

		    		    	uint16_t len = MIN(CDC_REQ_MAX_DATA_SIZE, req->wLength);
		    		    	(void)USBD_CtlSendData(pdev, (uint8_t *)hcdc->data, len);
		    		    }
		    			else {
		    				hcdc->CmdOpCode = req->bRequest;
		    			   	hcdc->CmdLength = (uint8_t)req->wLength;

		    			   	USBD_CtlPrepareRx(pdev, (uint8_t *)(void *)hcdc->data, req->wLength);
		    			}
		    		}
		    		else {
		    			((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_cdc->Control(req->bRequest,
		    			                                                          (uint8_t *)(void *)req, 0U);
		    		}
		    		break;
			    case AUDIO_REQ_GET_CUR:
			    	USBD_DbgLog("USBD_Composite_Setup - Audio");
			       	AUDIO_REQ_GetCurrent(pdev, req);
			       	break;

			    case AUDIO_REQ_SET_CUR:
			    	USBD_DbgLog("USBD_Composite_Setup - Audio");
			      	AUDIO_REQ_SetCurrent(pdev, req);
			       	break;

			    default:
			        USBD_CtlError(pdev, req);
			        ret = USBD_FAIL;
			        break;
			}
			break;
		case USB_REQ_TYPE_STANDARD:
			switch (req->bRequest) {
				case USB_REQ_GET_STATUS:
					if (pdev->dev_state == USBD_STATE_CONFIGURED) {
						USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
			        }
			        else {
			            USBD_CtlError(pdev, req);
			            ret = USBD_FAIL;
			        }
			        break;
				case USB_REQ_GET_DESCRIPTOR:
					if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE) {
						uint16_t len;
						uint8_t *pbuf;
						pbuf = USBD_CDC_Audio_Composite_Descriptor + COMPOSITE_AUDIOCONTROL_HEADER_DESC_SHIFT;
					    len = MIN(COMPOSITE_AUDIOCONTROL_HEADER_DESC_SIZE, req->wLength);

					    USBD_CtlSendData(pdev, pbuf, len);
					}
					break;
				case USB_REQ_GET_INTERFACE:
					if (pdev->dev_state == USBD_STATE_CONFIGURED) {
						if (req->wIndex == COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE) { //check if audio interface
					    	USBD_CtlSendData(pdev, (uint8_t *)(void *)&haudio->alt_setting, 1U);
						}
#ifdef USB_AUDIO_MIC_ON
						else if (req->wIndex == COMPOSITE_AUDIO_INPUT_STREAM_INTERFACE) { //check if audio interface
							USBD_CtlSendData(pdev, (uint8_t *)(void *)&haudio->alt_input_setting, 1U);
						}
#endif
						else { // CDC
							uint8_t ifalt = 0U;
							USBD_CtlSendData(pdev, &ifalt, 1U);
						}
					}
					else {
						USBD_CtlError(pdev, req);
						ret = USBD_FAIL;
					}
					break;
				case USB_REQ_SET_INTERFACE: //bmRequest == 1
					if (pdev->dev_state == USBD_STATE_CONFIGURED) {
						if (req->wIndex == COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE) { //check if audio interface
							if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES) {
								if (haudio->alt_setting != (uint8_t)(req->wValue)) {
									haudio->alt_setting = (uint8_t)(req->wValue);
								    if (haudio->alt_setting == 0U) {
								    	AUDIO_OUT_StopAndReset(pdev);
								    }
								    else {
								    	AUDIO_OUT_Restart(pdev);
								    }
								}
								USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);
							}
							else {
								/* Call the error management function (command will be nacked */
								USBD_CtlError(pdev, req);
								ret = USBD_FAIL;
							}
						}
#ifdef USB_AUDIO_MIC_ON
						else if (req->wIndex == COMPOSITE_AUDIO_INPUT_STREAM_INTERFACE) { //check if audio interface
							if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES) {
								haudio->alt_input_setting = (uint8_t)(req->wValue);
							}
							else {
								//Call the error management function (command will be nacked
								USBD_CtlError(pdev, req);
								ret = USBD_FAIL;
							}
						}
#endif
					}
					else {
						USBD_CtlError(pdev, req);
					    ret = USBD_FAIL;
					}
					break;
				default:
					USBD_CtlError(pdev, req);
			        ret = USBD_FAIL;
			        break;
			}
			break;
		default:
			USBD_CtlError(pdev, req);
			ret = USBD_FAIL;
			break;
	}

	return ret;
}


static uint8_t USBD_CDC_Audio_Composite_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum) {
	//USBD_DbgLog("USBD_Composite_DataIn epnum: %u", epnum);

	if (epnum == (COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU)) {
		HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin, GPIO_PIN_SET);
		tx_flag = 0U;
	}
	else if (epnum == (COMPOSITE_CDC_IN_EP & 0x0FU)) {
		USBD_CDC_HandleTypeDef *hcdc = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcdc;
		PCD_HandleTypeDef *hpcd = pdev->pData;

		if (pdev->pClassData != NULL) {
			USBD_DbgLog("%lu %lu %lu\r\n", pdev->ep_in[epnum].total_length, pdev->ep_in[epnum].total_length, hpcd->IN_ep[epnum].maxpacket);
			if ((pdev->ep_in[epnum].total_length > 0U) && ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U)) {
		    	/* Update the packet total length */
				pdev->ep_in[epnum].total_length = 0U;

				/* Send ZLP */
				USBD_LL_Transmit(pdev, epnum, NULL, 0U);
		    }
		    else {
		    	hcdc->TxState = 0U;
		    }
		    return USBD_OK;
		}
		else {
		    return USBD_FAIL;
		}
	}
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum) {
	if (epnum == (AUDIO_OUT_EP & 0xFU)) {
		USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

		/* Get received data packet length */
		uint16_t PacketSize = (uint16_t)USBD_LL_GetRxDataSize(pdev, epnum);



		/* Packet received Callback */
		//((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_audio->PeriodicTC(&haudio->buffer[haudio->wr_ptr],
		//		PacketSize, AUDIO_OUT_TC);

		/* Increment the Buffer pointer or roll it back when all buffers are full */
		haudio->wr_ptr += PacketSize;

		if (haudio->wr_ptr >= AUDIO_TOTAL_BUF_SIZE)
		{
			/* All buffers are full: roll back */
			//haudio->wr_ptr = 0U;
			haudio->wr_ptr -= AUDIO_TOTAL_BUF_SIZE;

			for (uint16_t i = 0; i < haudio->wr_ptr; ++i) {
				haudio->buffer[i] = haudio->buffer[i + AUDIO_TOTAL_BUF_SIZE];
			}
		}
		if (haudio->offset == AUDIO_OFFSET_UNKNOWN && haudio->wr_ptr >= AUDIO_TOTAL_BUF_SIZE / 2U)
		{
			((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_audio->AudioCmd(&haudio->buffer[0],
					AUDIO_TOTAL_BUF_SIZE / 2U,
					AUDIO_CMD_START);
			haudio->offset = AUDIO_OFFSET_NONE;
			tx_flag = 0U;
		}
		/* Prepare Out endpoint to receive next audio packet */

		(void)USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, &haudio->buffer[haudio->wr_ptr], AUDIO_OUT_PACKET + 4);

		//USBD_DbgLog("USBD_Composite_DataOut epnum: %u %u %u %u", epnum, haudio->offset, PacketSize, haudio->wr_ptr);
	}
	else if (epnum == (COMPOSITE_CDC_OUT_EP & 0xFU)) { //CDC
		USBD_CDC_HandleTypeDef *hcdc = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcdc;

		/* Get the received data length */
		hcdc->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

		USBD_DbgLog("USBD_Composite_DataOut - CDC: epnum: %u RxLength: %lu", epnum, hcdc->RxLength);

		/* USB data will be immediately processed, this allow next USB traffic being
		NAKed till the end of the application Xfer */
		if (pdev->pClassData != NULL) {
			((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_cdc->Receive(hcdc->RxBuffer, &hcdc->RxLength);
			USBD_DbgLog("USBD_Composite_DataOut -OK");
		    return USBD_OK;
		}
		else {

			return USBD_FAIL;
		}
	}
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_SOF(USBD_HandleTypeDef *pdev) {
	static uint16_t SOF_num = 0;
	uint16_t  dma = __HAL_DMA_GET_COUNTER(hi2s3.hdmatx);

	HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;
	if (haudio->alt_setting==1)
	{
		SOF_num++;
		if (SOF_num == (1 << USBD_AUDIO_FEEDBACK_SOF_RATE) && haudio->offset == AUDIO_OFFSET_NONE) {
			SOF_num = 0;

			if (dma < last_dma) {
				feedback_data = last_dma - dma;
			}
			else {
				feedback_data = AUDIO_TOTAL_BUF_SIZE / 2 - dma + last_dma;
			}
			feedback_data <<= 1; // DMA work in half word mode
			last_dma = dma;
			feedback_data <<= (8 - USBD_AUDIO_FEEDBACK_SOF_RATE);
			feedback_data <<= 4; // align to the to left of 3-byte

			feedbacktable[0] = feedback_data;
			feedbacktable[1] = feedback_data >> 8;
			feedbacktable[2] = feedback_data >> 16;
		}
		else if (haudio->offset != AUDIO_OFFSET_NONE) {
			SOF_num = 0;
		}

		/* Transmit feedback only when the last one is transmitted */
		if (tx_flag == 0U) {

		      /* Get FNSOF. Use volatile for fnsof_new since its address is mapped to a hardware register. */
		      USB_OTG_GlobalTypeDef* USBx = USB_OTG_FS;
		      uint32_t USBx_BASE = (uint32_t)USBx;
		      uint32_t volatile fnsof_new = (USBx_DEVICE->DSTS & USB_OTG_DSTS_FNSOF) >> 8;

		      if ((fnsof & 0x1) == (fnsof_new & 0x1)) {
		    	  USBD_LL_Transmit(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP, (uint8_t *) &feedbacktable[0], 3);
		        /* Block transmission until it's finished. */
		    	  tx_flag = 1U;
		      }
		 }
	}
	else {
		last_dma = dma;
	}
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	// epnum is always 0
	//USBD_DbgLog("USBD_Composite_IsoINIncomplete epnum: %u", epnum);

	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);

	//if (epnum == (COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU)) {



	  USB_OTG_GlobalTypeDef* USBx = USB_OTG_FS;
	  uint32_t USBx_BASE = (uint32_t)USBx;
	  fnsof = (USBx_DEVICE->DSTS & USB_OTG_DSTS_FNSOF) >> 8;

	  if (tx_flag == 1U) {
	    tx_flag = 0U;
	    USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);
	  }
	//}
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	// epnum is always 0
	//USBD_DbgLog("USBD_Composite_IsoOutIncomplete epnum: %u", epnum);
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_EP0_TxSent(USBD_HandleTypeDef *pdev) {
	USBD_Dbg2Log("USBD_Composite_EP0_TxSent");
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_EP0_RxReady (USBD_HandleTypeDef *pdev) {
	USBD_Dbg2Log("USBD_Composite_EP0_RxReady");
	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;
	USBD_CDC_HandleTypeDef   *hcdc = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcdc;

	if (haudio->control.cmd == AUDIO_REQ_SET_CUR) {
	    /* In this driver, to simplify code, only SET_CUR request is managed */

	    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL) {
	    	((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_audio->MuteCtl(haudio->control.data[0]);
	    	haudio->control.cmd = 0U;
	    	haudio->control.len = 0U;
	    }
	}

	if ((pdev->pUserData != NULL) && (hcdc->CmdOpCode != 0xFFU)) {
		((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_cdc->Control(hcdc->CmdOpCode,
	                                                      (uint8_t *)(void *)hcdc->data,
	                                                      (uint16_t)hcdc->CmdLength);
	    hcdc->CmdOpCode = 0xFFU;
	}
	return USBD_OK;
}

static uint8_t *USBD_CDC_Audio_Composite_GetCfgDesc (uint16_t *length) {
    *length = (uint16_t)sizeof(USBD_CDC_Audio_Composite_Descriptor);
    return USBD_CDC_Audio_Composite_Descriptor;
}


/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDC_Audio_Composite_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
    USB_LEN_DEV_QUALIFIER_DESC,
	USB_DESC_TYPE_DEVICE_QUALIFIER,
	0x00,                       /* bcdUSB */
	0x02,
	0xEF,                       /* bDeviceClass */
	0x02,                       /* bDeviceSubClass */
	0x01,                       /* bDeviceProtocol */
	0x40,                       /* bMaxPacketSize0 */
	0x01,                       /* bNumConfigurations */
	0x00,
};

uint8_t  *USBD_CDC_Audio_Composite_GetDeviceQualifierDescriptor (uint16_t *length) {
	*length = sizeof (USBD_CDC_Audio_Composite_DeviceQualifierDesc);
	return USBD_CDC_Audio_Composite_DeviceQualifierDesc;
}


/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

  memset(haudio->control.data, 0, 64U);

  /* Send the current mute state */
  USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
}


/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx(pdev, haudio->control.data, req->wLength);

    haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    haudio->control.len = (uint8_t)req->wLength; /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
  }
}


/**
 * @brief  Stop playing and reset buffer pointers
 * @param  pdev: instance
 */
static void AUDIO_OUT_StopAndReset(USBD_HandleTypeDef* pdev)
{
	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

  tx_flag = 1U;
  haudio->offset = AUDIO_OFFSET_UNKNOWN;
  haudio->rd_enable = 0U;
  haudio->rd_ptr = 0U;
  haudio->wr_ptr = 0U;
  feedback_data = 0xC0000;

  //USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);
  //USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_EP);

  ((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_audio->DeInit(0);
}


/**
 * @brief  Restart playing with new parameters
 * @param  pdev: instance
 */
static void AUDIO_OUT_Restart(USBD_HandleTypeDef* pdev)
{
	//USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

  //AUDIO_OUT_StopAndReset(pdev);

  ((USBD_Composite_ItfTypeDef *)pdev->pUserData)->fops_audio->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0U);

  feedback_data = 0xC0000;
  tx_flag = 0U;
}
