/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_types.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HX711_DOUT_Pin GPIO_PIN_7
#define HX711_DOUT_GPIO_Port GPIOE
#define HX711_SCK_Pin GPIO_PIN_8
#define HX711_SCK_GPIO_Port GPIOE
#define PHOTO_IN_Pin GPIO_PIN_9
#define PHOTO_IN_GPIO_Port GPIOE
#define LED_GREEN_Pin GPIO_PIN_10
#define LED_GREEN_GPIO_Port GPIOE
#define LED_RED_Pin GPIO_PIN_11
#define LED_RED_GPIO_Port GPIOE
#define BUZZER_Pin GPIO_PIN_12
#define BUZZER_GPIO_Port GPIOE
#define TFT_RST_Pin GPIO_PIN_1
#define TFT_RST_GPIO_Port GPIOB
#define TFT_DC_Pin GPIO_PIN_11
#define TFT_DC_GPIO_Port GPIOB
#define TFT_CS_Pin GPIO_PIN_12
#define TFT_CS_GPIO_Port GPIOB
#define KP_C3_Pin GPIO_PIN_8
#define KP_C3_GPIO_Port GPIOD
#define KP_C4_Pin GPIO_PIN_9
#define KP_C4_GPIO_Port GPIOD
#define KP_R1_Pin GPIO_PIN_0
#define KP_R1_GPIO_Port GPIOD
#define KP_R2_Pin GPIO_PIN_1
#define KP_R2_GPIO_Port GPIOD
#define KP_R3_Pin GPIO_PIN_2
#define KP_R3_GPIO_Port GPIOD
#define KP_R4_Pin GPIO_PIN_3
#define KP_R4_GPIO_Port GPIOD
#define KP_C1_Pin GPIO_PIN_6
#define KP_C1_GPIO_Port GPIOD
#define KP_C2_Pin GPIO_PIN_7
#define KP_C2_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
