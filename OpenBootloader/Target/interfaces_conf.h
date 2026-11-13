/**
  ******************************************************************************
  * @file    interfaces_conf.h
  * @author  MCD Application Team
  * @brief   Contains Interfaces configuration
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INTERFACES_CONF_H
#define INTERFACES_CONF_H

#define MEMORIES_SUPPORTED                7U

/* ------------------------- Definitions for USART -------------------------- */
#define USARTx                            USART4

#define USARTx_TX_PIN                     GPIO_PIN_0
#define USARTx_TX_GPIO_PORT               GPIOC
#define USARTx_RX_PIN                     GPIO_PIN_1
#define USARTx_RX_GPIO_PORT               GPIOC
#define USARTx_ALTERNATE                  GPIO_AF10

/* ------------------------- Definitions for FDCAN -------------------------- */
#define FDCANx                            FDCAN2

#define FDCANx_TX_PIN                     GPIO_PIN_6
#define FDCANx_TX_GPIO_PORT               GPIOB
#define FDCANx_TX_AF                      GPIO_AF27
#define FDCANx_RX_PIN                     GPIO_PIN_7
#define FDCANx_RX_GPIO_PORT               GPIOB
#define FDCANx_RX_AF                      GPIO_AF28

/* 握手（SYNC）用的标准帧 ID，默认 0x128。bootloader 的 CAN 滤波器和上位机
 * （tools/blcan.py 的 --sync-id 默认值、n32h473 profile）都以它为准，改这里要两边
 * 一起改，否则握手永远收不到 ACK */
#define SYNC_CAN_ID                       0x128U

/* ------------------------- Definitions for MUX ----------------------------- */
#define MUX1_GPIO_PIN                     GPIO_PIN_15
#define MUX1_GPIO_PORT                    GPIOB
#define MUX2_GPIO_PIN                     GPIO_PIN_13
#define MUX2_GPIO_PORT                    GPIOC

#endif /* INTERFACES_CONF_H */


