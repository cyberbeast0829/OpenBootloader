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
#define USARTx                            USART1

#define USARTx_TX_PIN                     GPIO_PIN_6
#define USARTx_TX_GPIO_PORT               GPIOB
#define USARTx_RX_PIN                     GPIO_PIN_7
#define USARTx_RX_GPIO_PORT               GPIOB
#define USARTx_ALTERNATE                  GPIO_AF1

/* ------------------------- Definitions for FDCAN -------------------------- */
#define FDCANx                            FDCAN1

#define FDCANx_TX_PIN                     GPIO_PIN_13
#define FDCANx_TX_GPIO_PORT               GPIOB
#define FDCANx_TX_AF                      GPIO_AF25
#define FDCANx_RX_PIN                     GPIO_PIN_12
#define FDCANx_RX_GPIO_PORT               GPIOB
#define FDCANx_RX_AF                      GPIO_AF26

/* ------------------------- Definitions for MUX ----------------------------- */
#define MUX1_GPIO_PIN                     GPIO_PIN_15
#define MUX1_GPIO_PORT                    GPIOB
#define MUX2_GPIO_PIN                     GPIO_PIN_13
#define MUX2_GPIO_PORT                    GPIOC

#endif /* INTERFACES_CONF_H */


