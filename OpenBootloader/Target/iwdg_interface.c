/**
  ******************************************************************************
  * @file    iwdg_interface.c
  * @author  MCD Application Team
  * @brief   Contains IWDG HW configuration
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

#include "main.h"
#include "iwdg_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void OPENBL_IWDG_Configuration(void)
{
  // IWDG_WriteConfig(IWDG_WRITE_ENABLE);
	// IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV256);
	// IWDG_CntReload(100); // 32K LSI时钟，分频256是125Hz，则100/125 = 0.8S

	// IWDG_ReloadKey();
  // IWDG_Enable();
}

/**
  * @brief  Kick the independent watchdog if the application left it running.
  *
  * 本 bootloader 自己不开 IWDG（见上面的 OPENBL_IWDG_Configuration），但 App 一上电
  * 就启用 IWDG（~4 s），而 IWDG 一旦启动只有上电复位才会停 —— App 用 magic value
  * 软复位进 bootloader 后它仍在计数。不喂狗的话升级到一半就整机复位、magic 已被清掉，
  * bootloader 200 ms 后跳回 App，OTA 永远做不完。
  * IWDG 没启动时写 KR=0xAAAA 是无害的空操作。
  */
void OPENBL_IWDG_Refresh(void)
{
  IWDG_ReloadKey();
}

