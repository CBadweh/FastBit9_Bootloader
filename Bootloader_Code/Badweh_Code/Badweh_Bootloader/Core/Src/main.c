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

// Bootloader receive buffer (200 bytes)
uint8_t bl_rx_buffer[BL_RX_LEN];

// Supported commands array (8 implemented commands, excluding student exercises BL_MEM_READ and BL_OTP_READ)
uint8_t supported_commands[] = {
    BL_GET_VER,
    BL_GET_HELP,
    BL_GET_CID,
    BL_GET_RDP_STATUS,
    BL_GO_TO_ADDR,
    BL_FLASH_ERASE,
    BL_MEM_WRITE,
    BL_EN_RW_PROTECT,
    BL_READ_SECTOR_P_STATUS,
    BL_DIS_R_W_PROTECT
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
// 2. Bootloader Jump to User Application
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
// 3. Bootloader UART Read Data (Command Reception & Dispatch)
// ============================================================================
void bootloader_uart_read_data(void)
{
    uint8_t rcv_len = 0;

    while(1)
    {
        memset(bl_rx_buffer, 0, BL_RX_LEN);

        // Phase 1: Read length byte (blocking)
        HAL_UART_Receive(C_UART, bl_rx_buffer, 1, HAL_MAX_DELAY);
        rcv_len = bl_rx_buffer[0];

        // Phase 2: Read the rest of the packet
        HAL_UART_Receive(C_UART, &bl_rx_buffer[1], rcv_len, HAL_MAX_DELAY);

        // Dispatch command based on command code
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
            case BL_MEM_WRITE:
                bootloader_handle_mem_write_cmd(bl_rx_buffer);
                break;
            case BL_EN_RW_PROTECT:
                bootloader_handle_en_rw_protect(bl_rx_buffer);
                break;
            case BL_MEM_READ:
                bootloader_handle_mem_read(bl_rx_buffer);
                break;
            case BL_READ_SECTOR_P_STATUS:
                bootloader_handle_read_sector_protection_status(bl_rx_buffer);
                break;
            case BL_OTP_READ:
                bootloader_handle_read_otp(bl_rx_buffer);
                break;
            case BL_DIS_R_W_PROTECT:
                bootloader_handle_dis_rw_protect(bl_rx_buffer);
                break;
            default:
                printmsg("BL_DEBUG_MSG: Invalid command code\r\n");
                break;
        }
    }
}

// ============================================================================
// 4. Helper Functions
// ============================================================================

// Send ACK with reply length
void bootloader_send_ack(uint8_t command_code, uint8_t follow_len)
{
    uint8_t ack_buf[2];
    ack_buf[0] = BL_ACK;
    ack_buf[1] = follow_len;
    HAL_UART_Transmit(C_UART, ack_buf, 2, HAL_MAX_DELAY);
}

// Send NACK
void bootloader_send_nack(void)
{
    uint8_t nack = BL_NACK;
    HAL_UART_Transmit(C_UART, &nack, 1, HAL_MAX_DELAY);
}

// Write data to UART
void bootloader_uart_write_data(uint8_t *pBuffer, uint32_t len)
{
    HAL_UART_Transmit(C_UART, pBuffer, len, HAL_MAX_DELAY);
}

// Verify CRC using hardware CRC peripheral
uint8_t bootloader_verify_crc(uint8_t *pData, uint32_t len, uint32_t crc_host)
{
    uint32_t uwCRCValue = 0xff;

    // Calculate CRC byte-by-byte using hardware CRC
    for(uint32_t i = 0; i < len; i++)
    {
        uint32_t i_data = pData[i];
        uwCRCValue = HAL_CRC_Accumulate(&hcrc, &i_data, 1);
    }

    // Reset CRC peripheral for next calculation
    __HAL_CRC_DR_RESET(&hcrc);

    if(uwCRCValue == crc_host)
    {
        return VERIFY_CRC_SUCCESS;
    }

    return VERIFY_CRC_FAIL;
}

// Get bootloader version
uint8_t get_bootloader_version(void)
{
    return (uint8_t)BL_VERSION;
}

// Get MCU chip ID (F401RE returns 0x0433)
uint16_t get_mcu_chip_id(void)
{
    // Read device ID from DBGMCU_IDCODE register (bits [11:0])
    uint16_t cid = (uint16_t)(DBGMCU->IDCODE) & 0x0FFF;
    return cid;
}

// Get Flash RDP (Read Protection) level
uint8_t get_flash_rdp_level(void)
{
    uint8_t rdp_status = 0;

    // Read RDP level from option bytes (bits [15:8] of first word at 0x1FFFC000)
    volatile uint32_t *pOB_addr = (uint32_t*) 0x1FFFC000;
    rdp_status = (uint8_t)(*pOB_addr >> 8);

    return rdp_status;
}

// Verify address validity (F401RE-specific: NO SRAM2!)
uint8_t verify_address(uint32_t go_address)
{
    // Valid address ranges for F401RE:
    // - SRAM1: 0x20000000 to (0x20000000 + 96KB)
    // - Flash: 0x08000000 to (0x08000000 + 512KB)
    // - Backup SRAM: Check if exists

    if(go_address >= SRAM1_BASE && go_address <= SRAM1_END)
    {
        return ADDR_VALID;
    }
    else if(go_address >= FLASH_BASE && go_address <= (FLASH_BASE + FLASH_SIZE))
    {
        return ADDR_VALID;
    }
    // Note: F401RE does NOT have SRAM2 (unlike F446RE)
    else
    {
        return ADDR_INVALID;
    }
}

// Execute flash erase (sector or mass erase)
uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sector)
{
    // F401RE has 8 sectors (0-7). Reject anything else that isn't mass-erase (0xFF).
    if(sector_number > 7 && sector_number != 0xFF)
    {
        return FLASH_HAL_ERROR;
    }

    FLASH_EraseInitTypeDef flashErase_handle;
    uint32_t sectorError;
    HAL_StatusTypeDef status;

    if(sector_number == 0xFF)
    {
        // Mass erase (all sectors)
        flashErase_handle.TypeErase = FLASH_TYPEERASE_MASSERASE;
    }
    else
    {
        // Sector erase — clamp count so we never go past sector 7
        uint8_t remaining_sector = 8 - sector_number;  // F401RE has 8 sectors (0-7)
        if(number_of_sector > remaining_sector)
        {
            number_of_sector = remaining_sector;
        }
        flashErase_handle.TypeErase = FLASH_TYPEERASE_SECTORS;
        flashErase_handle.Sector = sector_number;
        flashErase_handle.NbSectors = number_of_sector;
    }

    flashErase_handle.Banks = FLASH_BANK_1;
    flashErase_handle.VoltageRange = FLASH_VOLTAGE_RANGE_3;  // 2.7V to 3.6V

    // Unlock flash
    HAL_FLASH_Unlock();

    // Erase
    status = HAL_FLASHEx_Erase(&flashErase_handle, &sectorError);

    // Lock flash
    HAL_FLASH_Lock();

    return status;
}

// Execute memory write (flash programming)
uint8_t execute_mem_write(uint8_t *pBuffer, uint32_t mem_address, uint32_t len)
{
    HAL_StatusTypeDef status = HAL_OK;

    // Unlock flash
    HAL_FLASH_Unlock();

    // Program byte by byte
    for(uint32_t i = 0; i < len; i++)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, mem_address + i, pBuffer[i]);
        if(status != HAL_OK)
        {
            break;
        }
    }

    // Lock flash
    HAL_FLASH_Lock();

    return status;
}

// Read option bytes for sector protection status
uint16_t read_OB_rw_protection_status(void)
{
    FLASH_OBProgramInitTypeDef ob_handle;
    HAL_FLASHEx_OBGetConfig(&ob_handle);

    // Return write protection status (nWRP bits)
    return (uint16_t)ob_handle.WRPSector;
}

// Configure flash sector read/write protection
uint8_t configure_flash_sector_rw_protection(uint8_t sector_details, uint8_t protection_mode, uint8_t disable)
{
    FLASH_OBProgramInitTypeDef ob_handle;
    HAL_StatusTypeDef status;

    // Unlock option bytes
    HAL_FLASH_OB_Unlock();

    ob_handle.OptionType = OPTIONBYTE_WRP;
    ob_handle.Banks = FLASH_BANK_1;

    if(disable == 1)
    {
        // Disable all protection
        ob_handle.WRPState = OB_WRPSTATE_DISABLE;
        ob_handle.WRPSector = 0xFF;  // All sectors
    }
    else
    {
        if(protection_mode == 1)
        {
            // Write protection only
            ob_handle.WRPState = OB_WRPSTATE_ENABLE;
            ob_handle.WRPSector = sector_details;
        }
        else if(protection_mode == 2)
        {
            // Read/Write protection (PCROP) - F401RE may not support this
            // For now, treat as write protection
            ob_handle.WRPState = OB_WRPSTATE_ENABLE;
            ob_handle.WRPSector = sector_details;
        }
    }

    // Program option bytes
    status = HAL_FLASHEx_OBProgram(&ob_handle);

    if(status == HAL_OK)
    {
        // Launch option bytes reload
        HAL_FLASH_OB_Launch();
    }

    // Lock option bytes
    HAL_FLASH_OB_Lock();

    return status;
}

// ============================================================================
// 5. Bootloader Command Handlers
// ============================================================================

// ----------------------------------------------------------------------------
// Handler 1: BL_GET_VER (0x51) - Get Bootloader Version
// ----------------------------------------------------------------------------
void bootloader_handle_getver_cmd(uint8_t *pBuffer)
{
    uint8_t bl_version;

    printmsg("BL_DEBUG_MSG: bootloader_handle_getver_cmd\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 1);

        // 5. Get version
        bl_version = get_bootloader_version();
        printmsg("BL_DEBUG_MSG: BL_VER: %d %#x\r\n", bl_version, bl_version);

        // 6. Send reply
        bootloader_uart_write_data(&bl_version, 1);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 2: BL_GET_HELP (0x52) - Get Supported Commands
// ----------------------------------------------------------------------------
void bootloader_handle_gethelp_cmd(uint8_t *pBuffer)
{
    printmsg("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], sizeof(supported_commands));

        // 5. Send supported commands array
        bootloader_uart_write_data(supported_commands, sizeof(supported_commands));
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 3: BL_GET_CID (0x53) - Get Chip ID
// ----------------------------------------------------------------------------
void bootloader_handle_getcid_cmd(uint8_t *pBuffer)
{
    uint16_t bl_cid_num = 0;

    printmsg("BL_DEBUG_MSG: bootloader_handle_getcid_cmd\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 2);

        // 5. Get chip ID (F401RE returns 0x0433)
        bl_cid_num = get_mcu_chip_id();
        printmsg("BL_DEBUG_MSG: MCU ID: %d %#x\r\n", bl_cid_num, bl_cid_num);

        // 6. Send reply (2 bytes)
        bootloader_uart_write_data((uint8_t *)&bl_cid_num, 2);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 4: BL_GET_RDP_STATUS (0x54) - Get Read Protection Level
// ----------------------------------------------------------------------------
void bootloader_handle_getrdp_cmd(uint8_t *pBuffer)
{
    uint8_t rdp_level = 0;

    printmsg("BL_DEBUG_MSG: bootloader_handle_getrdp_cmd\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 1);

        // 5. Get RDP level
        rdp_level = get_flash_rdp_level();
        printmsg("BL_DEBUG_MSG: RDP level: %d %#x\r\n", rdp_level, rdp_level);

        // 6. Send reply
        bootloader_uart_write_data(&rdp_level, 1);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 5: BL_GO_TO_ADDR (0x55) - Jump to Address
// ----------------------------------------------------------------------------
void bootloader_handle_go_cmd(uint8_t *pBuffer)
{
    uint32_t go_address = 0;
    uint8_t addr_valid = ADDR_VALID;
    uint8_t addr_invalid = ADDR_INVALID;

    printmsg("BL_DEBUG_MSG: bootloader_handle_go_cmd\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 1);

        // 5. Extract address from packet (4 bytes at offset 2)
        go_address = *((uint32_t *)&pBuffer[2]);
        printmsg("BL_DEBUG_MSG: GO addr: %#x\r\n", go_address);

        // 6. Verify address
        if (verify_address(go_address) == ADDR_VALID)
        {
            // Address is valid
            bootloader_uart_write_data(&addr_valid, 1);

            // Set T-bit (bit 0) for Thumb mode
            go_address += 1;

            void (*lets_jump)(void) = (void *)go_address;

            printmsg("BL_DEBUG_MSG: jumping to go address! \r\n");

            lets_jump();  // Jump (never returns)
        }
        else
        {
            printmsg("BL_DEBUG_MSG: GO addr invalid!\r\n");
            // Address is invalid
            bootloader_uart_write_data(&addr_invalid, 1);
        }
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 6: BL_FLASH_ERASE (0x56) - Erase Flash
// ----------------------------------------------------------------------------
void bootloader_handle_flash_erase_cmd(uint8_t *pBuffer)
{
    uint8_t erase_status = 0x00;

    printmsg("BL_DEBUG_MSG: bootloader_handle_flash_erase_cmd\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 1);

        // 5. Turn on LED to indicate erase in progress
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

        // 6. Execute flash erase
        erase_status = execute_flash_erase(pBuffer[2], pBuffer[3]);

        // 7. Turn off LED
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

        printmsg("BL_DEBUG_MSG: flash erase status: %#x\r\n", erase_status);

        // 8. Send reply
        bootloader_uart_write_data(&erase_status, 1);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 7: BL_MEM_WRITE (0x57) - Write to Memory
// ----------------------------------------------------------------------------
void bootloader_handle_mem_write_cmd(uint8_t *pBuffer)
{
    uint8_t addr_valid = ADDR_VALID;
    uint8_t write_status = 0x00;
    uint8_t chksum = 0, len = 0;
    len = pBuffer[0];
    uint8_t payload_len = pBuffer[6];

    printmsg("BL_DEBUG_MSG: bootloader_handle_mem_write_cmd\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 1);

        // 5. Extract base address (4 bytes at offset 2)
        uint32_t mem_address = *((uint32_t *)(&pBuffer[2]));

        printmsg("BL_DEBUG_MSG: mem write addr: %#x\r\n", mem_address);

        // 6. Verify address
        if (verify_address(mem_address) == ADDR_VALID)
        {
            printmsg("BL_DEBUG_MSG: valid mem write address\r\n");

            // 7. Turn on LED to indicate write in progress
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

            // 8. Execute memory write (payload starts at offset 7)
            write_status = execute_mem_write(&pBuffer[7], mem_address, payload_len);

            // 9. Turn off LED
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

            // 10. Send reply
            bootloader_uart_write_data(&write_status, 1);
        }
        else
        {
            printmsg("BL_DEBUG_MSG: invalid mem write address\r\n");
            write_status = ADDR_INVALID;
            // Send reply
            bootloader_uart_write_data(&write_status, 1);
        }
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 8: BL_EN_RW_PROTECT (0x58) - Enable Read/Write Protection
// ----------------------------------------------------------------------------
void bootloader_handle_en_rw_protect(uint8_t *pBuffer)
{
    uint8_t status = 0x00;

    printmsg("BL_DEBUG_MSG: bootloader_handle_en_rw_protect\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 1);

        // 5. Configure protection (sector_details, protection_mode, disable=0)
        status = configure_flash_sector_rw_protection(pBuffer[2], pBuffer[3], 0);

        printmsg("BL_DEBUG_MSG: flash protection status: %#x\r\n", status);

        // 6. Send reply
        bootloader_uart_write_data(&status, 1);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 9: BL_MEM_READ (0x59) - Read Memory (Student Exercise - Stub)
// ----------------------------------------------------------------------------
void bootloader_handle_mem_read(uint8_t *pBuffer)
{
    // Student exercise - not implemented
    printmsg("BL_DEBUG_MSG: bootloader_handle_mem_read (not implemented)\r\n");
}

// ----------------------------------------------------------------------------
// Handler 10: BL_READ_SECTOR_P_STATUS (0x5A) - Read Sector Protection Status
// ----------------------------------------------------------------------------
void bootloader_handle_read_sector_protection_status(uint8_t *pBuffer)
{
    uint16_t status = 0;

    printmsg("BL_DEBUG_MSG: bootloader_handle_read_sector_protection_status\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 2);

        // 5. Read protection status
        status = read_OB_rw_protection_status();

        printmsg("BL_DEBUG_MSG: nWRP status: %#x\r\n", status);

        // 6. Send reply (2 bytes)
        bootloader_uart_write_data((uint8_t *)&status, 2);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
        bootloader_send_nack();
    }
}

// ----------------------------------------------------------------------------
// Handler 11: BL_OTP_READ (0x5B) - Read OTP (Student Exercise - Stub)
// ----------------------------------------------------------------------------
void bootloader_handle_read_otp(uint8_t *pBuffer)
{
    // Student exercise - not implemented
    printmsg("BL_DEBUG_MSG: bootloader_handle_read_otp (not implemented)\r\n");
}

// ----------------------------------------------------------------------------
// Handler 12: BL_DIS_R_W_PROTECT (0x5C) - Disable Read/Write Protection
// ----------------------------------------------------------------------------
void bootloader_handle_dis_rw_protect(uint8_t *pBuffer)
{
    uint8_t status = 0x00;

    printmsg("BL_DEBUG_MSG: bootloader_handle_dis_rw_protect\r\n");

    // 1. Calculate total packet length
    uint32_t command_packet_len = pBuffer[0] + 1;

    // 2. Extract CRC from host
    uint32_t host_crc = *((uint32_t *)(pBuffer + command_packet_len - 4));

    // 3. Verify CRC
    if (!bootloader_verify_crc(&pBuffer[0], command_packet_len - 4, host_crc))
    {
        printmsg("BL_DEBUG_MSG: checksum success\r\n");

        // 4. Send ACK
        bootloader_send_ack(pBuffer[0], 1);

        // 5. Disable all protection (disable=1)
        status = configure_flash_sector_rw_protection(0, 0, 1);

        printmsg("BL_DEBUG_MSG: flash protection disable status: %#x\r\n", status);

        // 6. Send reply
        bootloader_uart_write_data(&status, 1);
    }
    else
    {
        printmsg("BL_DEBUG_MSG: checksum fail\r\n");
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
