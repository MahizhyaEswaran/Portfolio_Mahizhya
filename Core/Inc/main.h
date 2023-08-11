/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

#include <new_gsm.h>
#include <new_mqtt.h>

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BAT_LEVEL_Pin GPIO_PIN_0
#define BAT_LEVEL_GPIO_Port GPIOA
#define MOIST_Pin GPIO_PIN_1
#define MOIST_GPIO_Port GPIOA
#define BOOST_Pin GPIO_PIN_3
#define BOOST_GPIO_Port GPIOA
#define EC_Pin GPIO_PIN_7
#define EC_GPIO_Port GPIOA
#define Default_Switch_Pin GPIO_PIN_0
#define Default_Switch_GPIO_Port GPIOB
#define BAT_GND_Pin GPIO_PIN_10
#define BAT_GND_GPIO_Port GPIOB
#define GSM_POWER_CTRL_Pin GPIO_PIN_11
#define GSM_POWER_CTRL_GPIO_Port GPIOB
#define MUX2_3_Pin GPIO_PIN_12
#define MUX2_3_GPIO_Port GPIOB
#define PWR_MEASURE_Pin GPIO_PIN_15
#define PWR_MEASURE_GPIO_Port GPIOB
#define Sensor_PWR_Pin GPIO_PIN_15
#define Sensor_PWR_GPIO_Port GPIOA
#define MUX_IN_Pin GPIO_PIN_3
#define MUX_IN_GPIO_Port GPIOB
#define MUX1_3_Pin GPIO_PIN_4
#define MUX1_3_GPIO_Port GPIOB
#define MODEM_PWR_Pin GPIO_PIN_5
#define MODEM_PWR_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
