#include "usbd_cdc_audio_composite.h"
#include "usbd_conf.h"
#include "usbd_descriptor.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_audio_if.h"
#include "usbd_cdc_if.h"
#include "stm32f2xx_hal_dma.h"
#include "stm32f2xx_hal_i2s.h"
#include <math.h>


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
static void AUDIO_IN_StopAndReset(USBD_HandleTypeDef* pdev);
static void AUDIO_IN_Restart(USBD_HandleTypeDef* pdev);


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

uint16_t dummy[AUDIO_OUT_PACKET/2] = {0};

static uint8_t USBD_CDC_Audio_Composite_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx) {

	for (int i = 0; i < AUDIO_OUT_PACKET/4; i++) {
	        int16_t value = (int16_t)(32000.0 * sin(2.0 * M_PI * i / 48.0));
	        dummy[i * 2] = value;
	        dummy[i * 2 + 1] = value;
	    }

	USBD_DbgLog("USBD_Composite_Init cfgidx: %d" ,cfgidx);
	USBD_AUDIO_HandleTypeDef *haudio;
	USBD_CDC_HandleTypeDef   *hcdc;

	pdev->pUserData = NULL;
	pdev->pClassData = &hcomposite;

// AUDIO
    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, COMPOSITE_AUDIO_OUT_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET + 4);
    pdev->ep_out[COMPOSITE_AUDIO_OUT_EP & 0xFU].is_used = 1U;
    //pdev->ep_out[COMPOSITE_AUDIO_OUT_EP & 0xFU].bInterval = AUDIO_FS_BINTERVAL;

    USBD_LL_OpenEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP, USBD_EP_TYPE_ISOC, 3U);
    pdev->ep_in[COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU].is_used = 1U;
    //pdev->ep_in[COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU].bInterval = AUDIO_FS_BINTERVAL;

    USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);


#ifdef USB_AUDIO_MIC_ON
    USBD_LL_OpenEP(pdev, COMPOSITE_AUDIO_IN_EP, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET);
    pdev->ep_in[COMPOSITE_AUDIO_IN_EP & 0xFU].is_used = 1U;

    //USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_IN_EP);
    //USBD_LL_Transmit(pdev, COMPOSITE_AUDIO_IN_EP, dummy, AUDIO_OUT_PACKET);
#endif

    /* Allocate Audio structure */
    haudio = &hcomposite.haudio;

    haudio->alt_output_setting = 0U;
    haudio->alt_input_setting = 0U;
    haudio->offset = AUDIO_OFFSET_UNKNOWN;
    haudio->wr_ptr = 0U;
    haudio->output_synch_feedback = 0xC0000;
    haudio->synch_feedback_tx_inprogress = 1;

    /* Initialize the Audio output Hardware layer */
    if (AUDIO_Init_FS(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0U) != 0) {
      	return USBD_FAIL;
    }

    /* Prepare Out endpoint to receive 1st packet */
    USBD_LL_PrepareReceive(pdev, COMPOSITE_AUDIO_OUT_EP, haudio->buffer, AUDIO_OUT_MAX_PACKET);


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
    //pdev->ep_in[COMPOSITE_CDC_CMD_EP & 0xFU].bInterval = CDC_FS_BINTERVAL;

    hcdc = &hcomposite.hcdc;

    hcdc->CmdLength = 0;
    hcdc->CmdOpCode = 0;
    hcdc->RxBuffer = NULL;
    hcdc->RxLength = 0;
    hcdc->TxBuffer = NULL;
    hcdc->TxLength = 0;

    /* Init  physical Interface components */
    CDC_Init_FS();

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
	//pdev->ep_in[COMPOSITE_CDC_CMD_EP & 0xFU].bInterval = 0U;

	/* DeInit  physical Interface components */
	if (pdev->pClassData != NULL) {
		CDC_DeInit_FS();
	}

//AUDIO
	//USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_EP);
	//USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);

	USBD_LL_CloseEP(pdev, COMPOSITE_AUDIO_OUT_EP);
	pdev->ep_out[COMPOSITE_AUDIO_OUT_EP & 0xFU].is_used = 0U;
	//pdev->ep_out[COMPOSITE_AUDIO_OUT_EP & 0xFU].bInterval = 0U;

	USBD_LL_CloseEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);
	pdev->ep_in[COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU].is_used = 0U;
	//pdev->ep_in[COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU].bInterval = 0U;

	/* DeInit  physical Interface components */
	if (pdev->pClassData != NULL) {
		AUDIO_DeInit_FS(0U);
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
		    				CDC_Control_FS(req->bRequest, (uint8_t *)(void *)hcdc->data, req->wLength);

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
		    			CDC_Control_FS(req->bRequest, (uint8_t *)(void *)req, 0U);
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
					    	USBD_CtlSendData(pdev, (uint8_t *)(void *)&haudio->alt_output_setting, 1U);
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
						if (req->wIndex == COMPOSITE_AUDIO_OUTPUT_STREAM_INTERFACE) { //check if audio out interface
							if ((uint8_t)(req->wValue) <= 1) {
								if (haudio->alt_output_setting != (uint8_t)(req->wValue)) {
									haudio->alt_output_setting = (uint8_t)(req->wValue);
								    if (haudio->alt_output_setting == 0U) {
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
						else if (req->wIndex == COMPOSITE_AUDIO_INPUT_STREAM_INTERFACE) { //check if audio in interface
							if ((uint8_t)(req->wValue) <= 1) {
								if (haudio->alt_input_setting != (uint8_t)(req->wValue)) {
									haudio->alt_input_setting = (uint8_t)(req->wValue);
									if (haudio->alt_input_setting == 0U) {
										AUDIO_IN_StopAndReset(pdev);
									}
									else {
										AUDIO_IN_Restart(pdev);
									}
								}
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
	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

	if (epnum == (COMPOSITE_AUDIO_OUT_SYNCH_EP & 0xFU)) {
		HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
		haudio->synch_feedback_tx_inprogress = 0U;

		HAL_GPIO_WritePin(CO_P_GPIO_Port, CO_P_Pin, GPIO_PIN_SET);

		haudio->synch_feedback_tx = 0;
		haudio->synch_feedback_SOF_counter = 1;
	}
	else if (epnum == (COMPOSITE_AUDIO_IN_EP & 0xFU)) {
		//HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
		USBD_LL_Transmit(pdev, COMPOSITE_AUDIO_IN_EP, dummy, AUDIO_OUT_PACKET);
	}
	else if (epnum == (COMPOSITE_CDC_IN_EP & 0x0FU)) {
		USBD_CDC_HandleTypeDef *hcdc = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcdc;
		PCD_HandleTypeDef *hpcd = pdev->pData;

		if (pdev->pClassData != NULL) {
			//USBD_Dbg2Log("%lu %lu %lu\r\n", pdev->ep_in[epnum].total_length, pdev->ep_in[epnum].total_length, hpcd->IN_ep[epnum].maxpacket);
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
		//AUDIO_PeriodicTC_FS(&haudio->buffer[haudio->wr_ptr], PacketSize, AUDIO_OUT_TC);

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
			AUDIO_AudioCmd_FS(&haudio->buffer[0], AUDIO_TOTAL_BUF_SIZE / 2U, AUDIO_CMD_START);

			haudio->offset = AUDIO_OFFSET_NONE;
			haudio->synch_feedback_tx_inprogress = 0U;
		}

		/* Prepare Out endpoint to receive next audio packet */
		(void)USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, &haudio->buffer[haudio->wr_ptr], AUDIO_OUT_MAX_PACKET);

		//USBD_DbgLog("USBD_Composite_DataOut epnum: %u %u %u %u", epnum, haudio->offset, PacketSize, haudio->wr_ptr);
	}
	else if (epnum == (COMPOSITE_CDC_OUT_EP & 0xFU)) { //CDC
		USBD_CDC_HandleTypeDef *hcdc = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->hcdc;

		/* Get the received data length */
		hcdc->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

		/* USB data will be immediately processed, this allow next USB traffic being
		NAKed till the end of the application Xfer */
		if (pdev->pClassData != NULL) {
			CDC_Receive_FS(hcdc->RxBuffer, &hcdc->RxLength);
		    return USBD_OK;
		}
		else {

			return USBD_FAIL;
		}
	}
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_SOF(USBD_HandleTypeDef *pdev) {
	static uint16_t last_dma = 0;
	static uint8_t feedbacktable[3];
	static uint8_t slow_down_flag = 0;
	static uint8_t speed_up_flag = 0;

	uint16_t  dma = AUDIO_Get_DMA_Read_Ptr();

	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

	uint32_t USBx_BASE = (uint32_t)((PCD_HandleTypeDef*)pdev->pData)->Instance;
	uint32_t volatile fnsof_new = (USBx_DEVICE->DSTS & USB_OTG_DSTS_FNSOF) >> 8;

	HAL_GPIO_WritePin(CO_P_GPIO_Port, CO_P_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(IN_P_GPIO_Port, IN_P_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(TX_P_GPIO_Port, TX_P_Pin, GPIO_PIN_RESET);

	if (fnsof_new & 0x1) {
		HAL_GPIO_WritePin(SOF_P_GPIO_Port, SOF_P_Pin, GPIO_PIN_RESET);
	}
	else {
		HAL_GPIO_WritePin(SOF_P_GPIO_Port, SOF_P_Pin, GPIO_PIN_SET);
	}

	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;
	if (haudio->alt_output_setting == 1)
	{
		haudio->synch_feedback_SOF_counter++;
		if (haudio->synch_feedback_SOF_counter == (1 << USBD_AUDIO_FEEDBACK_SOF_RATE) && haudio->offset == AUDIO_OFFSET_NONE) {
			haudio->synch_feedback_SOF_counter = 0;

			if (dma > last_dma) {
				haudio->output_synch_feedback = dma - last_dma;
			}
			else {
				haudio->output_synch_feedback = AUDIO_TOTAL_BUF_SIZE - last_dma + dma;
			}

			last_dma = dma;
			haudio->output_synch_feedback <<= (8 - USBD_AUDIO_FEEDBACK_SOF_RATE);

			uint16_t sync_diff = dma < haudio->wr_ptr ? haudio->wr_ptr - dma : AUDIO_TOTAL_BUF_SIZE - dma + haudio->wr_ptr;

			if (sync_diff < (AUDIO_TOTAL_BUF_SIZE/2 - 2000) || ((sync_diff < AUDIO_TOTAL_BUF_SIZE/2) && speed_up_flag)) speed_up_flag = 1;
			else speed_up_flag = 0;

			if (sync_diff > (AUDIO_TOTAL_BUF_SIZE/2 + 2000) || ((sync_diff > AUDIO_TOTAL_BUF_SIZE/2) && slow_down_flag)) slow_down_flag = 1;
			else slow_down_flag = 0;

			if (speed_up_flag) haudio->output_synch_feedback += (1 << 8); //0.25kHz
			else if (slow_down_flag) haudio->output_synch_feedback -= (1 << 8); //0.25kHz

			haudio->output_synch_feedback <<= 4; // align to the to left of 3-byte

			haudio->synch_feedback_tx = 1;
		}
		else if (haudio->synch_feedback_SOF_counter == (1 << USBD_AUDIO_FEEDBACK_SOF_RATE) && haudio->offset != AUDIO_OFFSET_NONE) {
			haudio->synch_feedback_SOF_counter = 0;
			haudio->synch_feedback_tx = 1;
		}

		/* Transmit feedback only when the last one is transmitted */
		if (haudio->synch_feedback_tx_inprogress == 0U && haudio->synch_feedback_tx == 1) {
		    /* Get FNSOF. Use volatile for fnsof_new since its address is mapped to a hardware register. */
			uint32_t USBx_BASE = (uint32_t)((PCD_HandleTypeDef*)pdev->pData)->Instance;
			uint32_t volatile fnsof_new = (USBx_DEVICE->DSTS & USB_OTG_DSTS_FNSOF) >> 8;

		    if ((haudio->synch_feedback_fnsof & 0x1) == (fnsof_new & 0x1)) {
		    	feedbacktable[0] = haudio->output_synch_feedback;
		    	feedbacktable[1] = haudio->output_synch_feedback >> 8;
		    	feedbacktable[2] = haudio->output_synch_feedback >> 16;

		    	HAL_GPIO_WritePin(TX_P_GPIO_Port, TX_P_Pin, GPIO_PIN_SET);
		    	USBD_LL_Transmit(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP, feedbacktable, 3);

		    	/* Block transmission until it's finished. */
		    	haudio->synch_feedback_tx_inprogress= 1U;
		    }
		}
	}
	else {
		last_dma = dma;
		haudio->synch_feedback_SOF_counter = 0;
	}
	return USBD_OK;
}


static uint8_t USBD_CDC_Audio_Composite_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum) {
	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);

	uint32_t USBx_BASE = (uint32_t)((PCD_HandleTypeDef*)pdev->pData)->Instance;
	uint32_t volatile fnsof_new = (USBx_DEVICE->DSTS & USB_OTG_DSTS_FNSOF) >> 8;

	if (haudio->synch_feedback_tx_inprogress == 1U && (haudio->synch_feedback_fnsof & 0x1) != (fnsof_new & 0x1)) {
		haudio->synch_feedback_tx_inprogress = 0U;
		haudio->synch_feedback_tx = 1;

		haudio->synch_feedback_fnsof = fnsof_new;
		USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);
		HAL_GPIO_WritePin(IN_P_GPIO_Port, IN_P_Pin, GPIO_PIN_SET);
	}
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

	    if (haudio->control.unit == COMPOSITE_AUDIOCONTROL_SPK_FEATURE_UNIT_ID) {
	    	AUDIO_MuteCtl_FS(haudio->control.data[0]);
	    	haudio->control.cmd = 0U;
	    	haudio->control.len = 0U;
	    }
	    else if (haudio->control.unit == COMPOSITE_AUDIOCONTROL_MIC_FEATURE_UNIT_ID) {
	    	AUDIO_MuteCtl_FS(haudio->control.data[0]);
	    	haudio->control.cmd = 0U;
	    	haudio->control.len = 0U;
	    }
	}

	if ((pdev->pUserData != NULL) && (hcdc->CmdOpCode != 0xFFU)) {
		CDC_Control_FS(hcdc->CmdOpCode, (uint8_t *)(void *)hcdc->data, (uint16_t)hcdc->CmdLength);
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
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00
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

	haudio->synch_feedback_tx_inprogress = 1U;
	haudio->offset = AUDIO_OFFSET_UNKNOWN;
	haudio->wr_ptr = 0U;
	haudio->output_synch_feedback = 0xC0000;

	//USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);
	//USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_EP);

	AUDIO_DeInit_FS(0);
}


/**
 * @brief  Restart playing with new parameters
 * @param  pdev: instance
 */
static void AUDIO_OUT_Restart(USBD_HandleTypeDef* pdev)
{
	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

	//AUDIO_OUT_StopAndReset(pdev);

	USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_OUT_SYNCH_EP);

	AUDIO_Init_FS(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0U);

	haudio->output_synch_feedback = 0xC0000;
	haudio->synch_feedback_tx_inprogress = 0U;
	haudio->synch_feedback_tx = 1;
}


static void AUDIO_IN_StopAndReset(USBD_HandleTypeDef* pdev)
{
	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

	USBD_LL_FlushEP(pdev, COMPOSITE_AUDIO_IN_EP);
}


static void AUDIO_IN_Restart(USBD_HandleTypeDef* pdev)
{
	USBD_AUDIO_HandleTypeDef *haudio = &((USBD_Composite_HandleTypeDef *)pdev->pClassData)->haudio;

	USBD_LL_Transmit(pdev, COMPOSITE_AUDIO_IN_EP, dummy, AUDIO_OUT_PACKET);
}
