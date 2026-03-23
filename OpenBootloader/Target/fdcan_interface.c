/**
  ******************************************************************************
  * @file    fdcan_interface.c
  * @author  MCD Application Team
  * @brief   Contains FDCAN HW configuration
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"

#include "openbl_core.h"
#include "openbl_fdcan_cmd.h"

#include "fdcan_interface.h"
#include "iwdg_interface.h"
#include "interfaces_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static FDCAN_InitType       fdcanInitStruct;
static FDCAN_FilterType     sFilterConfig;
static FDCAN_TxHeaderType   TxHeader;
static FDCAN_RxHeaderType   RxHeader;
static FDCAN_MsgRamType     fdcanMsgRamInfo;

/* Exported variables --------------------------------------------------------*/
uint8_t TxData[FDCAN_RAM_BUFFER_SIZE];
uint8_t RxData[FDCAN_RAM_BUFFER_SIZE];
uint32_t FDCAN_ram[512]; // 用于消息缓冲的RAM，总共512 words

/* Private function prototypes -----------------------------------------------*/
static void OPENBL_FDCAN_Init(void);

/* Private functions ---------------------------------------------------------*/

static void OPENBL_FDCAN_Init(void)
{
  // 确定波特率设置，baudrate=FDCAN Clock/((1+BS1+BS2)*Prescaler)
  // APB1Clock = PCLK1 = 40M
  // 固定升级波特率为500Kbps，波特率预分频器和时间段配置如下（采样点为80%）：
  // prescaler = 4;
  // sync_jump = 2;
  // time_seg1 = 15;
  // time_seg2 = 4;
  // data_prescaler = 4;
  // data_sync_jump = 2;
  // data_time_seg1 = 15;
  // data_time_seg2 = 4;
  FDCAN_DeInit(FDCANx);

  /* FDCAN Init */
  fdcanInitStruct.FrameFormat        = FDCAN_FRAME_CLASSIC;
  fdcanInitStruct.Mode               = FDCAN_MODE_NORMAL;
  fdcanInitStruct.AutoRetransmission = DISABLE;
  fdcanInitStruct.TransmitPause      = DISABLE;
  fdcanInitStruct.ProtocolException  = DISABLE;
  fdcanInitStruct.Prescaler          = 4;
  fdcanInitStruct.SyncJumpWidth      = 2;
  fdcanInitStruct.TimeSeg1           = 15;
  fdcanInitStruct.TimeSeg2           = 4;
  fdcanInitStruct.DataPrescaler      = 4;
  fdcanInitStruct.DataSyncJumpWidth  = 2;
  fdcanInitStruct.DataTimeSeg1       = 15;
  fdcanInitStruct.DataTimeSeg2       = 4;

  /* Message RAM configuration */
  fdcanInitStruct.MsgRamStrAddr      = (uint32_t)FDCAN_ram;  // 使用FDCAN_ram作为消息RAM
  fdcanInitStruct.MsgRamOffset       = 0;
  fdcanInitStruct.StdFilterSize      = 4;
  fdcanInitStruct.ExtFilterSize      = 4;
  fdcanInitStruct.RxFifo0Size        = 8;
  fdcanInitStruct.RxFifo1Size        = 0;
  fdcanInitStruct.RxBufferSize       = 0;
  fdcanInitStruct.TxEventSize        = 0;
  fdcanInitStruct.TxBufferSize       = 0;
  fdcanInitStruct.RxFifo0DataSize    = FDCAN_DATA_BYTES_8;
  fdcanInitStruct.RxFifo1DataSize    = FDCAN_DATA_BYTES_8;
  fdcanInitStruct.RxBufferDataSize   = FDCAN_DATA_BYTES_8;
  fdcanInitStruct.TxBufferDataSize   = FDCAN_DATA_BYTES_8;
  fdcanInitStruct.TxFifoQueueMode    = FDCAN_TX_FIFO_MODE;
  fdcanInitStruct.TxFifoQueueSize    = 8;
  fdcanInitStruct.pMsgInfo           = &fdcanMsgRamInfo;

  if (FDCAN_Init(FDCANx, &fdcanInitStruct) != SUCCESS)
  {
    while (1);
  }

  FDCAN_ConfigTSPrescaler(FDCANx, FDCAN_TIMESTAMP_PRESC_16);
  FDCAN_Config_TS(FDCANx, FDCAN_TIMESTAMP_DISABLE);

  // FDCAN_ConfigTxDelayCompensation(FDCANx, fdcanInitStruct.DataTimeSeg1, fdcanInitStruct.DataTimeSeg1);
  // FDCAN_EnableTxDelayCompensation(FDCANx);

  /* Configure Rx filter */
  sFilterConfig.IdType       = FDCAN_STANDARD_ID;
  sFilterConfig.FilterIndex  = 0;
  sFilterConfig.FilterType   = FDCAN_FILTER_RANGE;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1    = 0x128;
  sFilterConfig.FilterID2    = 0x128; // bootloader时ID默认为0x128
  FDCAN_ConfigFilter(FDCANx, &sFilterConfig);

  /* Prepare Tx Header */
  TxHeader.ID            = 0; // 上位机ID默认为0
  TxHeader.IdType        = FDCAN_STANDARD_ID;
  TxHeader.TxFrameType   = FDCAN_DATA_FRAME;
  TxHeader.DataLength    = FDCAN_DLC_BYTES_8;
  TxHeader.ErrorState    = FDCAN_ESI_PASSIVE;
  TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader.FDFormat      = FDCAN_CLASSIC_CAN;
  TxHeader.TxEventFifo   = FDCAN_NO_TX_EVENTS;
  TxHeader.MsgMarker     = 0;

  /* Start the FDCAN module */
  FDCAN_Start(FDCANx);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  This function is used to configure FDCAN pins and then initialize the used FDCAN instance.
 * @retval None.
 */
void OPENBL_FDCAN_Configuration(void)
{
  GPIO_InitType GPIO_InitStructure;

  RCC_ConfigFDCANPllClk(RCC_FDCAN_PLLSRC_DIV6); // 6分频，240M / 6 = 40M
  RCC_ConfigFDCANClksrc(RCC_FDCAN_CLKSRC_PLL); // 使用PLL作为FDCAN时钟源

  /* Enable all resources clocks */
  RCC_EnableAHB1PeriphClk(RCC_AHB_PERIPHEN_GPIOB, ENABLE);
  RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);

  /* Enable FDCAN1 clock */
  RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_FDCAN1, ENABLE);
  RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_FDCAN1);

  /* FDCAN1 pins configuration
           +-------------+
           |   FDCAN1    |
     +-----+-------------+
     | TX  |    PB13     |
     +-----+-------------+
     | RX  |    PB12     |
     +-----+-------------+ */

  GPIO_InitStructure.Pin            = FDCANx_TX_PIN;
  GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.GPIO_Pull      = GPIO_PULL_UP;
  GPIO_InitStructure.GPIO_Current   = GPIO_DC_12mA;
  GPIO_InitStructure.GPIO_Slew_Rate = GPIO_SLEW_RATE_FAST;
  GPIO_InitStructure.GPIO_Alternate = FDCANx_TX_AF;
  GPIO_InitPeripheral(FDCANx_TX_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.Pin            = FDCANx_RX_PIN;
  GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_INPUT;
  GPIO_InitStructure.GPIO_Alternate = FDCANx_RX_AF;
  GPIO_InitPeripheral(FDCANx_RX_GPIO_PORT, &GPIO_InitStructure);

  OPENBL_FDCAN_Init();
}

/**
 * @brief  This function is used to detect if there is any activity on FDCAN protocol.
 * @retval None.
 */
uint8_t OPENBL_FDCAN_ProtocolDetection(void)
{
  uint8_t detected = 0;

  /* check if FIFO 0 receive at least one message */
  if (FDCAN_GetRxFifoFillLevel(FDCANx, FDCAN_RX_FIFO0) > 0)
  {
    if (SYNC_BYTE == OPENBL_FDCAN_ReadByte()) {
      detected = 1;
      OPENBL_FDCAN_SendByte(ACK_BYTE);
    }
  }
  else
  {
    detected = 0;
  }

  return detected;
}

/**
 * @brief  This function is used to get the command opcode from the host.
 * @retval Returns the command.
 */
uint8_t OPENBL_FDCAN_GetCommandOpcode(void)
{
  uint8_t command_opc      = 0x0;
  ErrorStatus status       = SUCCESS;

  /* check if FIFO 0 receive at least one message */
  while (FDCAN_GetRxFifoFillLevel(FDCANx, FDCAN_RX_FIFO0) < 1)
  {}

  /* Retrieve Rx messages from RX FIFO0 */
  status = FDCAN_GetRxMsg(FDCANx, FDCAN_RX_FIFO0, &RxHeader, RxData);

  /* Check for errors */
  if (status == ERROR)
  {
    command_opc = ERROR_COMMAND;
  }
  else
  {
    command_opc  = RxHeader.ID;
    TxHeader.ID  = RxHeader.ID;
  }

  return command_opc;
}

/**
  * @brief  This function is used to read one byte from FDCAN pipe.
  * @retval Returns the read byte.
  */
uint8_t OPENBL_FDCAN_ReadByte(void)
{
  uint8_t buf[8];

  /* check if FIFO 0 receive at least one message */
  while (FDCAN_GetRxFifoFillLevel(FDCANx, FDCAN_RX_FIFO0) < 1)
  {
    OPENBL_IWDG_Refresh();
  }

  /* Retrieve Rx messages from RX FIFO0 */
  FDCAN_GetRxMsg(FDCANx, FDCAN_RX_FIFO0, &RxHeader, buf);

  return buf[0];
}

/**
  * @brief  This function is used to read bytes from FDCAN pipe.
  * @retval Returns the read byte.
  */
void OPENBL_FDCAN_ReadBytes(uint8_t *Buffer, uint32_t BufferSize)
{
  /* check if FIFO 0 receive at least one message */
  while (FDCAN_GetRxFifoFillLevel(FDCANx, FDCAN_RX_FIFO0) < 1)
  {
    OPENBL_IWDG_Refresh();
  }

  /* Retrieve Rx messages from RX FIFO0 */
  FDCAN_GetRxMsg(FDCANx, FDCAN_RX_FIFO0, &RxHeader, Buffer);
}

/**
  * @brief  This function is used to send one byte through FDCAN pipe.
  * @param  Byte The byte to be sent.
  * @retval None.
  */
void OPENBL_FDCAN_SendByte(uint8_t Byte)
{
  TxHeader.DataLength = FDCAN_DLC_BYTES_1;

  while (FDCAN_GetTxFifoFreeLevel(FDCANx) == 0)
  {}

  FDCAN_AddMsgToTxFifoQ(FDCANx, &TxHeader, &Byte);

  /* Wait that the data is completely sent (Tx FIFO empty) */
  while ((FDCANx->IR & FDCAN_IR_TFE) != FDCAN_IR_TFE)
  {}

  /* Clear the complete flag */
  FDCANx->IR = FDCAN_IR_TFE;
}

/**
  * @brief  This function is used to send a buffer using FDCAN.
  * @param  Buffer The data buffer to be sent.
  * @param  BufferSize The size of the data buffer to be sent.
  * @retval None.
  */
void OPENBL_FDCAN_SendBytes(uint8_t *Buffer, uint32_t BufferSize)
{
  TxHeader.DataLength = BufferSize;

  while (FDCAN_GetTxFifoFreeLevel(FDCANx) == 0)
  {}

  FDCAN_AddMsgToTxFifoQ(FDCANx, &TxHeader, Buffer);

  /* Wait that the data is completely sent (Tx FIFO empty) */
  while ((FDCANx->IR & FDCAN_IR_TFE) != FDCAN_IR_TFE)
  {}

  /* Clear the complete flag */
  FDCANx->IR = FDCAN_IR_TFE;
}
