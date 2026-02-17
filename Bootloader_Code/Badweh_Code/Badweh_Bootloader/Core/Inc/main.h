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

// Command handlers (10 total, 8 implemented + 2 student exercises)
void bootloader_handle_getver_cmd(uint8_t *pBuffer);
void bootloader_handle_gethelp_cmd(uint8_t *pBuffer);
void bootloader_handle_getcid_cmd(uint8_t *pBuffer);
void bootloader_handle_getrdp_cmd(uint8_t *pBuffer);
void bootloader_handle_go_cmd(uint8_t *pBuffer);
void bootloader_handle_flash_erase_cmd(uint8_t *pBuffer);
void bootloader_handle_mem_write_cmd(uint8_t *pBuffer);
void bootloader_handle_en_rw_protect(uint8_t *pBuffer);
void bootloader_handle_mem_read(uint8_t *pBuffer);           // Student exercise
void bootloader_handle_read_sector_protection_status(uint8_t *pBuffer);
void bootloader_handle_read_otp(uint8_t *pBuffer);           // Student exercise
void bootloader_handle_dis_rw_protect(uint8_t *pBuffer);

// Helper functions
void bootloader_send_ack(uint8_t command_code, uint8_t follow_len);
void bootloader_send_nack(void);
void bootloader_uart_write_data(uint8_t *pBuffer, uint32_t len);
uint8_t bootloader_verify_crc(uint8_t *pData, uint32_t len, uint32_t crc_host);
uint8_t get_bootloader_version(void);
uint16_t get_mcu_chip_id(void);
uint8_t get_flash_rdp_level(void);
uint8_t verify_address(uint32_t go_address);
uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sector);
uint8_t execute_mem_write(uint8_t *pBuffer, uint32_t mem_address, uint32_t len);

// Flash protection functions
uint16_t read_OB_rw_protection_status(void);
uint8_t configure_flash_sector_rw_protection(uint8_t sector_details, uint8_t protection_mode, uint8_t disable);

// Debug print function
void printmsg(char *format, ...);

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

// Buffer sizes
#define BL_RX_LEN  200

// STM32F401RE Memory Map
#define SRAM1_SIZE            (96 * 1024)     // 96KB (F401RE has 96KB, NOT 128KB)
#define SRAM1_END             (SRAM1_BASE + SRAM1_SIZE)
// Note: F401RE does NOT have SRAM2

#define FLASH_SIZE            (512 * 1024)    // 512KB
#define BKPSRAM_SIZE          (4 * 1024)      // 4KB Backup SRAM

// Flash sector base addresses
#define FLASH_SECTOR2_BASE_ADDRESS  0x08008000U  // User app starts here

// Bootloader Command Codes
#define BL_GET_VER                  0x51
#define BL_GET_HELP                 0x52
#define BL_GET_CID                  0x53
#define BL_GET_RDP_STATUS           0x54
#define BL_GO_TO_ADDR               0x55
#define BL_FLASH_ERASE              0x56
#define BL_MEM_WRITE                0x57
#define BL_EN_RW_PROTECT            0x58
#define BL_MEM_READ                 0x59  // Student exercise (optional)
#define BL_READ_SECTOR_P_STATUS     0x5A
#define BL_OTP_READ                 0x5B  // Student exercise (optional)
#define BL_DIS_R_W_PROTECT          0x5C

// Bootloader Reply Codes
#define BL_ACK   0xA5
#define BL_NACK  0x7F

// CRC Verification Status
#define VERIFY_CRC_FAIL    1
#define VERIFY_CRC_SUCCESS 0

// Address Verification Status
#define ADDR_VALID    0
#define ADDR_INVALID  1

// Flash Operations Status
#define FLASH_HAL_OK       0x00
#define FLASH_HAL_ERROR    0x01

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
