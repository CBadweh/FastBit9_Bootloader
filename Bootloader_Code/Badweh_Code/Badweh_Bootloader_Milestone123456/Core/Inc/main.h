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

// Core bootloader functions
void bootloader_uart_read_data(void);
void bootloader_jump_to_user_app(void);

// Debug print function
void printmsg(char *format, ...);

// Milestone 4: Framework utilities
uint8_t bootloader_verify_crc(uint8_t *pData, uint32_t len, uint32_t crc_host);
void bootloader_send_ack(uint8_t command_code, uint8_t follow_len);
void bootloader_send_nack(void);
void bootloader_uart_write_data(uint8_t *pBuffer, uint32_t len);

// Milestone 5: BL_GET_VER
void bootloader_handle_getver_cmd(uint8_t *pBuffer);
uint8_t get_bootloader_version(void);

// Milestone 6: BL_FLASH_ERASE
void bootloader_handle_flash_erase_cmd(uint8_t *pBuffer);
uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sector);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// Bootloader version
#define BL_VERSION  0x10

// Debug message enable/disable
#define BL_DEBUG_MSG_EN

// Flash sector base addresses
#define FLASH_SECTOR2_BASE_ADDRESS  0x08008000U  // User app starts here

// Milestone 4: Command Framework
#define BL_ACK              0xA5
#define BL_NACK             0x7F
#define VERIFY_CRC_SUCCESS  0
#define VERIFY_CRC_FAIL     1
#define BL_RX_LEN           200

// Command codes
#define BL_GET_VER          0x51
#define BL_FLASH_ERASE      0x56

// STM32F401RE: 6 sectors (0-5), not 8 like F446
#define STM32F401RE_NUM_SECTORS  6

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
