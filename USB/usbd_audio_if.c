#include "usbd_audio_if.h"
#include "usbd_cdc_audio_composite.h"


extern USBD_HandleTypeDef hUsbDeviceFS;
extern I2S_HandleTypeDef hi2s3;

/**
  * @brief  Initializes the AUDIO media low layer over USB FS IP
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
  /* USER CODE BEGIN 0 */
	printf("AUDIO_Init_FS\r\n");
  UNUSED(AudioFreq);
  UNUSED(Volume);
  UNUSED(options);
  return (USBD_OK);
  /* USER CODE END 0 */
}

/**
  * @brief  De-Initializes the AUDIO media low layer
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t AUDIO_DeInit_FS(uint32_t options)
{
  /* USER CODE BEGIN 1 */
	printf("AUDIO_DeInit_FS\r\n");
  UNUSED(options);
  extern I2S_HandleTypeDef hi2s3;
  HAL_I2S_DMAStop(&hi2s3);
  return (USBD_OK);
  /* USER CODE END 1 */
}

/**
  * @brief  Handles AUDIO command.
  * @param  pbuf: Pointer to buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
  /* USER CODE BEGIN 2 */
  printf("AUDIO_AudioCmd_FS size: %lu, cmd: %u\r\n", size, cmd);
  extern I2S_HandleTypeDef hi2s3;
  switch(cmd)
  {
    case AUDIO_CMD_START:
    	HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)pbuf, size);
    break;

    case AUDIO_CMD_PLAY:
    	//HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)pbuf, size);
    break;
  }
  UNUSED(pbuf);
  UNUSED(size);
  UNUSED(cmd);
  return (USBD_OK);
  /* USER CODE END 2 */
}

/**
  * @brief  Controls AUDIO Volume.
  * @param  vol: volume level (0..100)
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t AUDIO_VolumeCtl_FS(uint8_t vol)
{
  /* USER CODE BEGIN 3 */
	//printf("AUDIO_VolumeCtl_FS\n");
  UNUSED(vol);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  Controls AUDIO Mute.
  * @param  cmd: command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t AUDIO_MuteCtl_FS(uint8_t cmd)
{
  /* USER CODE BEGIN 4 */
	//printf("AUDIO_MuteCtl_FS\n");
  UNUSED(cmd);
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  AUDIO_PeriodicT_FS
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd)
{
  /* USER CODE BEGIN 5 */
	//printf("AUDIO_PeriodicTC_FS size: %u\n", size);
  UNUSED(pbuf);
  UNUSED(size);
  UNUSED(cmd);
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Gets AUDIO State.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t AUDIO_GetState_FS(void)
{
  /* USER CODE BEGIN 6 */
	//printf("AUDIO_GetState_FS\n");
  return (USBD_OK);
  /* USER CODE END 6 */
}

uint16_t AUDIO_Get_DMA_Read_Ptr(void)
{
	return ((AUDIO_TOTAL_BUF_SIZE >> 1) -__HAL_DMA_GET_COUNTER(hi2s3.hdmatx)) << 1U; // DMA work in half word mode and counts down
}

