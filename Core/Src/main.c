/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @brief   Main program body
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
#include "main.h"
#include "app_openbootloader.h"
#include "common_interface.h"
#include "flash_interface.h"
#include "interfaces_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t appAddr = 0x08008000;
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
extern uint32_t ticks;
/* Private functions ---------------------------------------------------------*/

void GPIO_Init(void)
{
  GPIO_InitType GPIO_InitStructure;

  RCC_EnableAHB1PeriphClk(RCC_AHB_PERIPHEN_GPIOB | RCC_AHB_PERIPHEN_GPIOC, ENABLE);
  RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);

  GPIO_InitStruct(&GPIO_InitStructure);

  GPIO_InitStructure.Pin        = MUX1_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.GPIO_Slew_Rate = GPIO_SLEW_RATE_SLOW;
  GPIO_InitPeripheral(MUX1_GPIO_PORT, &GPIO_InitStructure);
  GPIO_SetBits(MUX1_GPIO_PORT, MUX1_GPIO_PIN);

  GPIO_InitStructure.Pin        = MUX2_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.GPIO_Slew_Rate = GPIO_SLEW_RATE_SLOW;
  GPIO_InitPeripheral(MUX2_GPIO_PORT, &GPIO_InitStructure);
  GPIO_SetBits(MUX2_GPIO_PORT, MUX2_GPIO_PIN);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint32_t last_ticks = 0;

  /* Configure the System clock */
  SystemClock_Config();

  GPIO_Init();

  OpenBootloader_Init();
	
	Common_EnableIrq();

  /* Infinite loop */
  last_ticks = ticks;
  while (1)
  {
    if (0 == OpenBootloader_ProtocolDetection()) {
      if (ticks - last_ticks > 200) { // 如果没有任何接口被检测到，则跳转到APP
        OPENBL_FLASH_JumpToAddress(appAddr);
      }
    }
  }
}

/**
  * @brief  System Clock Configuration
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ConfigHclk(RCC_SYSCLK_DIV1); // HCLK = SYSCLK
  RCC_ConfigPclk1(RCC_HCLK_DIV4); // PCLK1 = 200M / 4
  RCC_ConfigPclk2(RCC_HCLK_DIV2); // PCLK2 = 200M / 2

  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
  SysTick_Config(SystemCoreClock / 1000);
}

void System_DeInit(void)
{
  RCC_DeInit();
  SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; // 关掉SysTick
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


