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

void OPENBL_IWDG_Refresh(void)
{
  /* Refresh IWDG: reload counter */
  //IWDG_ReloadKey();
}


