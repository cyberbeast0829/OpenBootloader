/**
  ******************************************************************************
  * @file    usart_interface.c
  * @author  MCD Application Team
  * @brief   Contains USART HW configuration
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
#include "openbl_usart_cmd.h"

#include "usart_interface.h"
#include "iwdg_interface.h"
#include "interfaces_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void OPENBL_USART_Init(void);

/* Private functions ---------------------------------------------------------*/

static void OPENBL_USART_Init(void)
{
  USART_InitType USART_InitStructure;

  USART_InitStructure.BaudRate            = 115200U;
  USART_InitStructure.WordLength          = USART_WL_8B;
  USART_InitStructure.StopBits            = USART_STPB_1;
  USART_InitStructure.Parity              = USART_PE_NO;
  USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
  USART_InitStructure.OverSampling        = USART_16OVER;
  USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;

  /*
  if (IS_USART_AUTOBAUDRATE_DETECTION_INSTANCE(USARTx))
  {
    LL_USART_EnableAutoBaudRate(USARTx);
    LL_USART_SetAutoBaudRateMode(USARTx, LL_USART_AUTOBAUD_DETECT_ON_7F_FRAME);
  }
  else
  {
    LL_USART_DisableAutoBaudRate(USARTx);
    USART_InitStructure.BaudRate = 921600U;
  }
  */

  USART_Init(USARTx, &USART_InitStructure);
  USART_Enable(USARTx, ENABLE);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  This function is used to configure USART pins and then initialize the used USART instance.
 * @retval None.
 */
void OPENBL_USART_Configuration(void)
{
  GPIO_InitType GPIO_InitStructure;

  /* Enable all resources clocks --------------------------------------------*/
  /* Enable used GPIOx clocks */
  RCC_EnableAHB1PeriphClk(RCC_AHB_PERIPHEN_GPIOB | RCC_AHB_PERIPHEN_GPIOC, ENABLE);
  RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);

  /* Enable USART clock */
  RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_USART4, ENABLE);

  /* USART4 pins configuration -----------------------------------------------*/
  /*
           +-------------+
           |   USART4    |
     +-----+-------------+
     | TX  |     PC0     |
     +-----+-------------+
     | RX  |     PC1    |
     +-----+-------------+ */

  GPIO_InitStructure.Pin       = USARTx_TX_PIN;
  GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.GPIO_Pull      = GPIO_PULL_UP;
  GPIO_InitStructure.GPIO_Slew_Rate     = GPIO_SLEW_RATE_FAST;
  GPIO_InitStructure.GPIO_Alternate = USARTx_ALTERNATE;
  GPIO_InitPeripheral(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = USARTx_RX_PIN;
  GPIO_InitPeripheral(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);

  OPENBL_USART_Init();
}

/**
 * @brief  This function is used to detect if there is any activity on USART protocol.
 * @retval None.
 */
uint8_t OPENBL_USART_ProtocolDetection(void)
{
  uint8_t detected;

  if (USART_GetFlagStatus(USARTx, USART_FLAG_OREF) == SET)
  {
    if (USART_GetFlagStatus(USARTx, USART_FLAG_RXDNE) == RESET)
    {
      USART_ReceiveData(USARTx); // 读DAT寄存器会清除OREF标志
    }
  }

  /* Check if the USART4 is addressed */
  if (USART_GetFlagStatus(USARTx, USART_FLAG_RXDNE) == SET)
  {
    if (SYNC_BYTE == USART_ReceiveData(USARTx))
    {
      OPENBL_USART_SendByte(ACK_BYTE);
      detected = 1;
    }
    else
    {
      detected = 0;
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
uint8_t OPENBL_USART_GetCommandOpcode(void)
{
  uint8_t command_opc = 0x0;

  /* Get the command opcode */
  command_opc = OPENBL_USART_ReadByte();

  if (command_opc == SYNC_BYTE) {
    return command_opc;
  }

  /* Check the data integrity */
  if ((command_opc ^ OPENBL_USART_ReadByte()) != 0xFF)
  {
    command_opc = ERROR_COMMAND;
  }
  else
  {
    /* nothing to do */
  }

  return command_opc;
}

/**
  * @brief  This function is used to read one byte from USART pipe.
  * @retval Returns the read byte.
  */
uint8_t OPENBL_USART_ReadByte(void)
{
  if (USART_GetFlagStatus(USARTx, USART_FLAG_OREF) == SET)
  {
    if (USART_GetFlagStatus(USARTx, USART_FLAG_RXDNE) == RESET)
    {
      USART_ReceiveData(USARTx); // 读DAT寄存器会清除OREF标志
    }
  }
  
  while (USART_GetFlagStatus(USARTx, USART_FLAG_RXDNE) == RESET)
  {
    OPENBL_IWDG_Refresh();
  }

  return (uint8_t)USART_ReceiveData(USARTx);
}

/**
  * @brief  This function is used to send one byte through USART pipe.
  * @param  Byte The byte to be sent.
  * @retval None.
  */
void OPENBL_USART_SendByte(uint8_t Byte)
{
  USART_SendData(USARTx, (Byte & 0xFF));

  while (USART_GetFlagStatus(USARTx, USART_FLAG_TXDE) == RESET)
  {
  }
}


