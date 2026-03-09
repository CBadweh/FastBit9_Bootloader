/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// UART Handle Aliases (F401RE-specific)
#define C_UART  &huart2   // Command UART (USART2 on PA2/PA3)
#define D_UART  &huart6   // Debug UART (USART6 on PC6/PC7) - F401RE uses USART6, NOT USART3!

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

uint8_t bl_rx_buffer[BL_RX_LEN];

uint8_t supported_commands[] = {
    BL_GET_VER,
    BL_GET_HELP,
    BL_GET_CID,
    BL_GET_RDP_STATUS,
    BL_GO_TO_ADDR,
    BL_FLASH_ERASE
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  // Boot decision: Check button state (PC13, active LOW)
  if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
      // Button is pressed → Enter bootloader mode
      printmsg("BL_DEBUG_MSG: Button is pressed.. Going to BL mode\r\n");
      bootloader_uart_read_data();
  } else {
      // Button is NOT pressed → Jump to user application
      printmsg("BL_DEBUG_MSG: Button is not pressed.. Jumping to user app\r\n");
      bootloader_jump_to_user_app();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

// ============================================================================
// 1. Debug Print Function
// ============================================================================
void printmsg(char *format, ...)
{
#ifdef BL_DEBUG_MSG_EN
    char str[80];
    va_list args;
    va_start(args, format);
    vsprintf(str, format, args);
    HAL_UART_Transmit(D_UART, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
    va_end(args);
#endif
}

// ============================================================================
// 2. Bootloader Jump to User Application (Milestone 3 — Lesson 043)
// ============================================================================
void bootloader_jump_to_user_app(void)
{
    // Function pointer to hold reset handler address
    void (*app_reset_handler)(void);

    printmsg("BL_DEBUG_MSG: bootloader_jump_to_user_app\r\n");

    // 1. Read the MSP value from the base address of user application (sector 2)
    uint32_t msp_value = *(volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS;

    printmsg("BL_DEBUG_MSG: MSP value: %#x\r\n", msp_value);

    // 2. Set the MSP (Main Stack Pointer) to the value read from user app
    __set_MSP(msp_value);

    // 3. Read the reset handler address from user app (base + 4)
    uint32_t resethandler_address = *(volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS + 4);

    app_reset_handler = (void*) resethandler_address;

    printmsg("BL_DEBUG_MSG: App reset handler addr: %#x\r\n", resethandler_address);

    // 4. Jump to user application reset handler (this never returns)
    app_reset_handler();
}

// ============================================================================
// 3. Bootloader UART Read Data (Milestone 4 — Command Framework)
// ============================================================================
void bootloader_uart_read_data(void)
{
    uint8_t rcv_len = 0;

    printmsg("BL_DEBUG_MSG: Bootloader mode entered.\r\n");
    printmsg("BL_DEBUG_MSG: Waiting for commands...\r\n");

    while(1)
    {
        // Clear receive buffer at start of each iteration
        memset(bl_rx_buffer, 0, BL_RX_LEN);

        // Phase 1: Read the "length to follow" byte (1 byte)
        HAL_UART_Receive(C_UART, bl_rx_buffer, 1, HAL_MAX_DELAY);
        rcv_len = bl_rx_buffer[0];

        // Phase 2: Read the remaining 'rcv_len' bytes
        HAL_UART_Receive(C_UART, &bl_rx_buffer[1], rcv_len, HAL_MAX_DELAY);

        // Dispatch based on command code (byte index 1)
        switch(bl_rx_buffer[1])
        {
            case BL_GET_VER:
                bootloader_handle_getver_cmd(bl_rx_buffer);
                break;
            case BL_GET_HELP:
                bootloader_handle_gethelp_cmd(bl_rx_buffer);
                break;
            case BL_GET_CID:
                bootloader_handle_getcid_cmd(bl_rx_buffer);
                break;
            case BL_GET_RDP_STATUS:
                bootloader_handle_getrdp_cmd(bl_rx_buffer);
                break;
            case BL_GO_TO_ADDR:
                bootloader_handle_go_cmd(bl_rx_buffer);
                break;
            case BL_FLASH_ERASE:
                bootloader_handle_flash_erase_cmd(bl_rx_buffer);
                break;
            default:
                printmsg("BL_DEBUG_MSG: Invalid command code received: 0x%x\r\n", bl_rx_buffer[1]);
                break;
        }
    }
}

// ============================================================================
// 4. CRC Verification (Milestone 4)
// ============================================================================
uint8_t bootloader_verify_crc(uint8_t *pData, uint32_t len, uint32_t crc_host)
{
    uint32_t uwCRCValue = 0xFF;
    uint32_t i_data;

    for (uint32_t i = 0; i < len; i++)
    {
        i_data = (uint32_t)pData[i];  // Promote byte to 32-bit
        uwCRCValue = HAL_CRC_Accumulate(&hcrc, &i_data, 1);
    }

    // Reset the CRC data register for next calculation
    __HAL_CRC_DR_RESET(&hcrc);

    if (uwCRCValue == crc_host)
    {
        return VERIFY_CRC_SUCCESS;
    }

    return VERIFY_CRC_FAIL;
}

// ============================================================================
// 5. Send ACK (Milestone 4)
// ============================================================================
void bootloader_send_ack(uint8_t command_code, uint8_t follow_len)
{
    uint8_t ack_buf[2];
    ack_buf[0] = BL_ACK;
    ack_buf[1] = follow_len;
    HAL_UART_Transmit(C_UART, ack_buf, 2, HAL_MAX_DELAY);
}

// ============================================================================
// 6. Send NACK (Milestone 4)
// ============================================================================
void bootloader_send_nack(void)
{
    uint8_t nack = BL_NACK;
    HAL_UART_Transmit(C_UART, &nack, 1, HAL_MAX_DELAY);
}

// ============================================================================
// 7. Bootloader UART Write (Milestone 4)
// ============================================================================
void bootloader_uart_write_data(uint8_t *pBuffer, uint32_t len)
{
    HAL_UART_Transmit(C_UART, pBuffer, len, HAL_MAX_DELAY);
}

// ============================================================================
// 8. Get Bootloader Version (Milestone 5)
// ============================================================================
uint8_t get_bootloader_version(void)
{
    return (uint8_t)BL_VERSION;
}

// ============================================================================
// 9. Handle BL_GET_VER Command (Milestone 5 — Command 0x51)
// ============================================================================
void bootloader_handle_getver_cmd(uint8_t *pBuffer)
{
    printmsg("BL_DEBUG_MSG: bootloader_handle_getver_cmd\r\n");

    // Total length of the command packet
    uint32_t command_packet_len = bl_rx_buffer[0] + 1;

    // Extract the host-sent CRC (last 4 bytes of the packet)
    uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));

    // Verify CRC over all bytes except the CRC field itself
    if (bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc) == VERIFY_CRC_SUCCESS)
    {
        printmsg("BL_DEBUG_MSG: CRC verification success\r\n");

        // Send ACK with follow_len = 1 (we will send 1 byte: the version)
        bootloader_send_ack(pBuffer[1], 1);

        // Get and send the bootloader version
        uint8_t bl_version = get_bootloader_version();
        printmsg("BL_DEBUG_MSG: BL_VER: %d %#x\r\n", bl_version, bl_version);
        bootloader_uart_write_data(&bl_version, 1);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: CRC verification fail\r\n");
        bootloader_send_nack();
    }
}

// ============================================================================
// 10. Execute Flash Erase (Milestone 6)
// ============================================================================
uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sector)
{
    FLASH_EraseInitTypeDef flashErase_handle;
    uint32_t sectorError;
    HAL_StatusTypeDef status;

    // Mass erase requested
    if (sector_number == 0xFF)
    {
        printmsg("BL_DEBUG_MSG: Mass erase requested\r\n");
        flashErase_handle.TypeErase = FLASH_TYPEERASE_MASSERASE;
    }
    else
    {
        // Validate sector range for STM32F401RE (sectors 0-5 only)
        if (sector_number >= STM32F401RE_NUM_SECTORS)
        {
            printmsg("BL_DEBUG_MSG: Invalid sector number: %d\r\n", sector_number);
            return HAL_ERROR;
        }

        // Clamp so we don't exceed sector 5
        uint8_t remaining_sectors = STM32F401RE_NUM_SECTORS - sector_number;
        if (number_of_sector > remaining_sectors)
        {
            number_of_sector = remaining_sectors;
        }

        printmsg("BL_DEBUG_MSG: Erasing sector %d, count %d\r\n", sector_number, number_of_sector);

        flashErase_handle.TypeErase = FLASH_TYPEERASE_SECTORS;
        flashErase_handle.Sector = sector_number;
        flashErase_handle.NbSectors = number_of_sector;
    }

    flashErase_handle.VoltageRange = FLASH_VOLTAGE_RANGE_3;  // 2.7V to 3.6V

    // Unlock flash, erase, re-lock
    HAL_FLASH_Unlock();
    status = HAL_FLASHEx_Erase(&flashErase_handle, &sectorError);
    HAL_FLASH_Lock();

    printmsg("BL_DEBUG_MSG: Flash erase status: %d\r\n", status);

    return (uint8_t)status;
}

// ============================================================================
// 11. Handle BL_FLASH_ERASE Command (Milestone 6 — Command 0x56)
// ============================================================================
void bootloader_handle_flash_erase_cmd(uint8_t *pBuffer)
{
    printmsg("BL_DEBUG_MSG: bootloader_handle_flash_erase_cmd\r\n");

    // Total length of the command packet
    uint32_t command_packet_len = bl_rx_buffer[0] + 1;

    // Extract the host-sent CRC (last 4 bytes of the packet)
    uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));

    // Verify CRC
    if (bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc) == VERIFY_CRC_SUCCESS)
    {
        printmsg("BL_DEBUG_MSG: CRC verification success\r\n");

        // Send ACK with follow_len = 1 (we will send 1 status byte)
        bootloader_send_ack(pBuffer[1], 1);

        // Toggle LED to indicate erase in progress
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

        // Extract erase parameters from the command packet
        // pBuffer[2] = sector number (or 0xFF for mass erase)
        // pBuffer[3] = number of sectors
        uint8_t erase_status = execute_flash_erase(pBuffer[2], pBuffer[3]);

        // Turn LED off after erase
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

        printmsg("BL_DEBUG_MSG: Flash erase status: %#x\r\n", erase_status);
        bootloader_uart_write_data(&erase_status, 1);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: CRC verification fail\r\n");
        bootloader_send_nack();
    }
}

// ============================================================================
// 12. Get MCU Chip ID
// ============================================================================
uint16_t get_mcu_chip_id(void)
{
    // Read DBGMCU IDCODE register, mask bits 0-11 (device identifier)
    // STM32F401RE returns 0x433, STM32F446RE returns 0x421
    uint16_t cid;
    cid = (uint16_t)(DBGMCU->IDCODE) & 0x0FFF;
    return cid;
}

// ============================================================================
// 13. Handle BL_GET_CID Command (Command 0x53)
// ============================================================================
void bootloader_handle_getcid_cmd(uint8_t *pBuffer)
{
    uint16_t bl_cid_num = 0;
    printmsg("BL_DEBUG_MSG: bootloader_handle_getcid_cmd\r\n");

    uint32_t command_packet_len = bl_rx_buffer[0] + 1;
    uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));

    if (bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc) == VERIFY_CRC_SUCCESS)
    {
        printmsg("BL_DEBUG_MSG: CRC verification success\r\n");
        bootloader_send_ack(pBuffer[1], 2);
        bl_cid_num = get_mcu_chip_id();
        printmsg("BL_DEBUG_MSG: MCU id : %d %#x\r\n", bl_cid_num, bl_cid_num);
        bootloader_uart_write_data((uint8_t *)&bl_cid_num, 2);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: CRC verification fail\r\n");
        bootloader_send_nack();
    }
}

// ============================================================================
// 14. Get Flash RDP Level
// ============================================================================
uint8_t get_flash_rdp_level(void)
{
    // Read option bytes from flash storage at 0x1FFFC000, extract bits 8-15 (RDP byte)
    // 0xAA = Level 0 (no protection), 0xCC = Level 2 (irreversible), else Level 1
    volatile uint32_t *pOBaddr = (uint32_t *)0x1FFFC000;
    return (uint8_t)(*pOBaddr >> 8);
}

// ============================================================================
// 15. Handle BL_GET_RDP_STATUS Command (Command 0x54)
// ============================================================================
void bootloader_handle_getrdp_cmd(uint8_t *pBuffer)
{
    uint8_t rdp_level = 0x00;
    printmsg("BL_DEBUG_MSG: bootloader_handle_getrdp_cmd\r\n");

    uint32_t command_packet_len = bl_rx_buffer[0] + 1;
    uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));

    if (bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc) == VERIFY_CRC_SUCCESS)
    {
        printmsg("BL_DEBUG_MSG: CRC verification success\r\n");
        bootloader_send_ack(pBuffer[1], 1);
        rdp_level = get_flash_rdp_level();
        printmsg("BL_DEBUG_MSG: RDP level: %d %#x\r\n", rdp_level, rdp_level);
        bootloader_uart_write_data(&rdp_level, 1);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: CRC verification fail\r\n");
        bootloader_send_nack();
    }
}

// ============================================================================
// 16. Handle BL_GET_HELP Command (Command 0x52)
// ============================================================================
void bootloader_handle_gethelp_cmd(uint8_t *pBuffer)
{
    printmsg("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd\r\n");

    uint32_t command_packet_len = bl_rx_buffer[0] + 1;
    uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));

    if (bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc) == VERIFY_CRC_SUCCESS)
    {
        printmsg("BL_DEBUG_MSG: CRC verification success\r\n");
        bootloader_send_ack(pBuffer[1], sizeof(supported_commands));
        bootloader_uart_write_data(supported_commands, sizeof(supported_commands));
    }
    else
    {
        printmsg("BL_DEBUG_MSG: CRC verification fail\r\n");
        bootloader_send_nack();
    }
}

// ============================================================================
// 17. Verify Address (for BL_GO_TO_ADDR)
// ============================================================================
uint8_t verify_address(uint32_t go_address)
{
    // F401RE: SRAM1 only (96KB), no SRAM2
    if (go_address >= SRAM1_BASE && go_address <= SRAM1_END)
    {
        return ADDR_VALID;
    }
    else if (go_address >= FLASH_BASE && go_address <= FLASH_END)
    {
        return ADDR_VALID;
    }
    return ADDR_INVALID;
}

// ============================================================================
// 18. Handle BL_GO_TO_ADDR Command (Command 0x55)
// ============================================================================
void bootloader_handle_go_cmd(uint8_t *pBuffer)
{
    uint32_t go_address = 0;
    uint8_t addr_valid = ADDR_VALID;
    uint8_t addr_invalid = ADDR_INVALID;

    printmsg("BL_DEBUG_MSG: bootloader_handle_go_cmd\r\n");

    uint32_t command_packet_len = bl_rx_buffer[0] + 1;
    uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));

    if (bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc) == VERIFY_CRC_SUCCESS)
    {
        printmsg("BL_DEBUG_MSG: CRC verification success\r\n");

        bootloader_send_ack(pBuffer[1], 1);

        // Extract the 4-byte go address from packet (bytes 2-5)
        go_address = *((uint32_t *)&pBuffer[2]);
        printmsg("BL_DEBUG_MSG: GO addr: %#x\r\n", go_address);

        if (verify_address(go_address) == ADDR_VALID)
        {
            bootloader_uart_write_data(&addr_valid, 1);

            // Make T bit = 1 for Thumb execution on Cortex-M
            go_address += 1;

            void (*lets_jump)(void) = (void *)go_address;

            printmsg("BL_DEBUG_MSG: Jumping to go address!\r\n");

            lets_jump();
        }
        else
        {
            printmsg("BL_DEBUG_MSG: GO addr invalid!\r\n");
            bootloader_uart_write_data(&addr_invalid, 1);
        }
    }
    else
    {
        printmsg("BL_DEBUG_MSG: CRC verification fail\r\n");
        bootloader_send_nack();
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
