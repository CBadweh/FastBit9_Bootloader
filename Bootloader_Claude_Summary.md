# STM32 Custom Bootloader Course — Comprehensive Summary

> **Course:** FastBit Embedded Brain Academy — STM32 Bootloader Development
> **Target MCU:** STM32F446RE (ARM Cortex-M4, 512KB Flash, 128KB SRAM)
> **Board:** STM32 Nucleo-F446RE
> **Source Code:** `BootloaderProjectSTM32-master/SourceCode/`

---

## Table of Contents

- [Section 2: Introduction](#section-2-introduction)
- [Section 3: MCU Memory and Reset Sequence](#section-3-mcu-memory-and-reset-sequence)
- [Section 4: Development Board](#section-4-development-board)
- [Section 8: Exploring STM32 Native Bootloader](#section-8-exploring-stm32-native-bootloader)
- [Section 9: Custom Bootloader Communication Design](#section-9-custom-bootloader-communication-design)
- [Section 10: Bootloader Project Creation](#section-10-bootloader-project-creation)
- [Section 11: UART Testing](#section-11-uart-testing)
- [Section 12: Jumping to User Application](#section-12-jumping-to-user-application)
- [Section 13: Reading Commands from Host](#section-13-reading-commands-from-host)
- [Section 14: Implementing Bootloader Commands](#section-14-implementing-bootloader-commands)
- [Section 21: Option Bytes and Flash Sector Protection](#section-21-option-bytes-and-flash-sector-protection)
- [Section 22: Exploring the Host Application](#section-22-exploring-the-host-application)
- [Course Overview: How Topics Connect](#course-overview-how-topics-connect)

---

## Section 2: Introduction

### Lesson 002 — Course Overview

**Objective:** Provide a roadmap of the entire course and introduce the bootloader project architecture.

**Key Concepts:**
- The course covers 20+ sections progressing from theory to full implementation
- The custom bootloader communicates with a host PC over UART using a custom protocol
- Bootloader commands include: `BL_GET_VER`, `BL_GET_HELP`, `BL_GET_CID`, `BL_GET_RDP_STATUS`, `BL_GO_TO_ADDR`, `BL_FLASH_ERASE`, `BL_MEM_WRITE`, and sector protection commands
- Two UART channels are used: one for commands, one for debug
- The Vector Table Offset Register (VTOR) is introduced as a critical concept for jumping between bootloader and user application

**Source Code Mapping:**
- All command codes defined in `BOOTLOADER/.../Core/Inc/main.h:140-175`
- Command handler prototypes in `main.h:102-113`

---

### Lesson 005 — What is a Bootloader and Why It Is Needed

**Objective:** Define what a bootloader is and explain its purpose through real-world examples.

**Key Concepts:**
- **Bootloader:** A small piece of code stored in MCU flash or ROM that (1) loads user applications into memory and (2) provides a mechanism to update applications
- **In-Application Programming (IAP):** Programming via bootloader over a communication interface (UART, USB, etc.) — Arduino is a classic example
- **In-System Programming (ISP):** Programming directly using an in-circuit debugger/programmer (e.g., ST-Link) — no bootloader involvement
- If a product has no debugging hardware, the bootloader is the only mechanism for firmware updates

**Examples:**
- **Arduino UNO (ATmega328P):** Bootloader runs every reset, receives sketches from the IDE, writes them to flash — pure IAP workflow
- **STM32F446RE Nucleo:** Has an on-chip bootloader in ROM (system memory) that must be explicitly activated via boot pins. Primary programming is through ST-Link (ISP)
- **TI Tiva Launchpad:** Has TivaWare bootloader in ROM, activated via boot pins and GPIO configuration

**Why It Matters:** This lesson establishes the fundamental motivation for the entire course — building a custom bootloader enables field-upgradeable firmware without physical debug access.

---

## Section 3: MCU Memory and Reset Sequence

### Lesson 006 — MCU/Embedded Memory Organization

**Objective:** Understand the internal memory architecture of the STM32F446xx MCU.

**Key Concepts:**

| Memory Region | Size | Type | Base Address | Purpose |
|---|---|---|---|---|
| Internal Flash | 512 KB | Non-volatile | `0x0800_0000` | Code, constants, vector table |
| SRAM 1 | 112 KB | Volatile | `0x2000_0000` | Variables, stack, heap |
| SRAM 2 | 16 KB | Volatile | After SRAM1 | Extension of SRAM1 |
| System Memory (ROM) | 30 KB | Read-only | `0x1FFF_0000` | ST's native bootloader |
| OTP | 528 bytes | Write-once | — | Serial numbers, manufacturing data |
| Option Bytes | 16 bytes | Configurable | `0x1FFF_C000` | Flash security, RDP levels |
| Backup RAM | 4 KB | Battery-backed | — | Data retained via VBAT |

**Flash Sector Organization (STM32F446):**

| Sector | Size |
|---|---|
| 0 | 16 KB |
| 1 | 16 KB |
| 2 | 16 KB |
| 3 | 16 KB |
| 4 | 64 KB |
| 5-7 | 128 KB each |

**Source Code Mapping:**
- Flash sector size constants and memory addresses used throughout `main.h:192-198`
- `FLASH_SECTOR2_BASE_ADDRESS 0x08008000U` defined at `main.h:95`

---

### Lesson 007 — Understanding Reset Sequence and Memory Aliasing

**Objective:** Explain the ARM Cortex-M reset sequence and the memory aliasing mechanism that makes it work.

**Key Concepts:**
- **ARM Cortex-M Reset Sequence (step by step):**
  1. PC is loaded with `0x0000_0000`
  2. Processor reads 32-bit value at `0x0000_0000` → loaded into MSP (Main Stack Pointer)
  3. Processor reads 32-bit value at `0x0000_0004` → loaded into PC (Reset Handler address)
  4. PC jumps to Reset Handler → calls `SystemInit()` → calls `main()`
- **Memory Aliasing:** The hardware maps `0x0800_0000` (flash base) onto `0x0000_0000`. Reading address `0x0000_0000` actually returns data from `0x0800_0000`
- This aliasing is transparent to the processor and is controlled by boot pin configuration

**Demo:** Keil Memory Window shows identical contents at `0x0000_0000` and `0x0800_0000`, proving aliasing is active. Register window confirms MSP contains the first word from flash.

**Why It Matters:** This is the foundation for understanding how the bootloader jumps to the user application — it replicates the reset sequence by reading MSP and reset handler from the user app's base address.

**Source Code Mapping:**
- The `bootloader_jump_to_user_app()` function in `main.c:177-206` directly replicates this reset sequence:
  - Reads MSP from `FLASH_SECTOR2_BASE_ADDRESS` (line 186)
  - Sets MSP via `__set_MSP()` (line 190)
  - Reads reset handler from base+4 (line 197)
  - Jumps via function pointer (line 204)

---

### Lesson 008 — Boot Configurations of the STM32 MCU

**Objective:** Explain how the BOOT0/BOOT1 pins control which memory region is aliased to address 0.

**Key Concepts:**

| BOOT1 | BOOT0 | Boot Source | Aliased to 0x0000_0000 |
|---|---|---|---|
| X | 0 | Main Flash | `0x0800_0000` |
| 0 | 1 | System Memory (ST bootloader) | `0x1FFF_0000` |
| 1 | 1 | Embedded SRAM | `0x2000_0000` |

- "Booting from" a memory region means the processor executes instructions from that region
- Boot pins control a hardware multiplexer that selects which memory is at address 0
- **TI Tiva comparison:** Flash is at `0x0000_0000` directly (no aliasing for normal boot). Uses GPIO pins + Boot Configuration register (BOOTCFG) instead of dedicated boot pins

---

## Section 4: Development Board

### Lesson 010 — About MCU Development Board

**Objective:** Guide board selection for embedded development.

**Key Concepts — 5 Board Selection Criteria:**
1. **Manufacturer reputation** — Branded boards (ST, TI) ensure quality
2. **Documentation/support** — ST provides extensive manuals, app notes, forums
3. **On-board debugger** — ST-Link enables real-time debugging (Arduino lacks this)
4. **Peripheral support** — Verify CAN, USB, SPI, etc. for your project needs
5. **Flash/RAM size** — Recommend 100KB+ RAM, 512KB+ Flash for middleware compatibility

---

### Lesson 011 — STM32F4 Discovery and Nucleo Board Details

**Objective:** Hardware walkthrough of both supported development boards.

**Key Concepts:**
- **Discovery board (STM32F407VG):** Has 4 LEDs, external 8MHz crystal, on-board sensors. **Critical issue:** Virtual COM Port pins are NOT connected to STM32F407's USART — requires external USB-to-UART converter
- **Nucleo board (STM32F446RE):** Simpler design, Arduino-compatible headers. **Key advantage:** Built-in Virtual COM Port via ST-Link connected to USART2 (PA2/PA3) — no extra hardware needed
- The Nucleo board's Virtual COM Port is why USART2 is chosen as the command channel in this course

---

### Lessons 012-013 — ST-Link Driver Installation and Firmware Upgrade

**Objective:** Set up the development environment by installing drivers and updating ST-Link firmware.

**Key Concepts:**
- Two communication interfaces over a single USB cable: (1) COM Port for UART serial, (2) ST-Link USB for debugging/flashing
- ST-Link firmware must be upgraded for IDE compatibility — use ST-Link Firmware Upgrade tool
- **Windows:** Install `dpinst_amd64.exe` driver. **Linux:** Use udev rules. **Mac:** No driver needed

---

## Section 8: Exploring STM32 Native Bootloader

### Lesson 026 — Activating ST's Bootloader Part-1

**Objective:** Learn how to physically activate the built-in bootloader by manipulating boot pins.

**Key Concepts:**
- To boot from system memory (activate ST bootloader): Set BOOT0=HIGH, BOOT1=LOW
- On Nucleo: Short pins 5 (VDD) and 7 (BOOT0) on CN7 Morpho connector with a jumper wire
- BOOT1 is multiplexed with PB2 — already grounded by default on Nucleo
- After placing the jumper, press reset — the MCU samples boot pins and enters bootloader mode

---

### Lesson 027 — Activating ST's Bootloader Part-2

**Objective:** Explore the capabilities of ST's native bootloader through the official application note.

**Key Concepts:**
- ST's bootloader supports multiple interfaces: USART1, USART3, CAN, I2C1, I2C2, USB, CAN2 (for STM32F446xx)
- The bootloader uses a **proprietary communication protocol** (not XMODEM/YMODEM)
- After entering system memory boot mode, the bootloader polls all supported peripherals and locks onto whichever responds first
- **Important:** USART2 is NOT supported by the native bootloader on STM32F446xx

---

### Lesson 028 — Activating ST's Bootloader Part-3

**Objective:** Solve the practical problem of connecting the PC to the bootloader for UART communication.

**Key Concepts:**
- The Nucleo Virtual COM Port uses USART2 (PA2/PA3) — but the native bootloader doesn't support USART2
- Solution: Use USART3 with an external USB-to-UART converter connected to PC10 (TX) and PC11 (RX)
- TX/RX cross-wiring: Board TX → Converter RX, Board RX → Converter TX, plus common ground
- **STM32 Flash Loader Demonstrator:** Windows GUI tool from ST that implements the bootloader protocol

---

### Lesson 029 — Activating ST's Bootloader Part-4

**Objective:** Demonstrate In-Application Programming using the Flash Loader Demonstrator.

**Demo — Complete IAP Workflow:**
1. Connect USB-to-UART converter to USART3 pins (PC10/PC11)
2. Place BOOT0 jumper, press reset
3. Open Flash Loader Demonstrator → select COM port → connect
4. Tool detects STM32F4, 512KB flash, shows sector layout
5. Select a hex file (Blinky app) → "Download to device" → "Jump to user program"
6. LED starts blinking — user application running via bootloader programming

**Key Concepts:**
- **Bootloader lifecycle:** Activate on reset → receive commands → execute → optionally jump to user app → bootloader inactive
- Flash erase strategies: "Erase necessary pages", "No erase", "Global erase"
- After jumping to user app, bootloader commands will timeout (bootloader is no longer running)

---

## Section 9: Custom Bootloader Communication Design

### Lesson 030 — Bootloader Transport

**Objective:** Define the transport layer architecture for the custom bootloader.

**Key Concepts:**
- **USART2 (Command UART):** Bidirectional command/reply channel between host PC and bootloader. Uses the built-in Virtual COM Port — no extra hardware needed
- **USART3 (Debug UART):** Optional unidirectional debug output channel. Requires external USB-to-UART converter
- The custom bootloader focuses on UART for simplicity but the architecture is extensible to USB, SPI, I2C, CAN

**Source Code Mapping:**
- UART handle aliases in `main.c:62-63`:
  ```c
  #define D_UART   &huart3   // Debug UART
  #define C_UART   &huart2   // Command UART
  ```
- USART2 init: `main.c:362-388` (115200 baud, 8N1)
- USART3 init: `main.c:395-421` (115200 baud, 8N1)

---

### Lesson 031 — Bootloader Code Placement

**Objective:** Define how flash memory is partitioned between bootloader and user application.

**Key Concepts:**
- **Bootloader:** Flash Sectors 0-1 (32KB), base address `0x0800_0000`
- **User Application:** Flash Sectors 2-7, base address `0x0800_8000`
- Both ST's native bootloader (in ROM) and the custom bootloader (in flash) coexist
- The bootloader must know the user app's base address to jump to it

**Source Code Mapping:**
- `#define FLASH_SECTOR2_BASE_ADDRESS 0x08008000U` in `main.h:95`

---

### Lesson 032 — Bootloader Supported Commands

**Objective:** Enumerate and explain every command the custom bootloader supports.

**Command Summary:**

| Command | Code | Purpose | Reply |
|---|---|---|---|
| `BL_GET_VER` | `0x51` | Get bootloader version | 1 byte: version |
| `BL_GET_HELP` | `0x52` | List supported commands | Array of command codes |
| `BL_GET_CID` | `0x53` | Get chip ID | 2 bytes: chip ID |
| `BL_GET_RDP_STATUS` | `0x54` | Read flash read protection level | 1 byte: RDP level |
| `BL_GO_TO_ADDR` | `0x55` | Jump to specified address | 1 byte: status |
| `BL_FLASH_ERASE` | `0x56` | Erase flash sectors or mass erase | 1 byte: status |
| `BL_MEM_WRITE` | `0x57` | Write data to memory | 1 byte: status |
| `BL_EN_RW_PROTECT` | `0x58` | Enable sector read/write protection | 1 byte: status |
| `BL_MEM_READ` | `0x59` | Read from memory (student exercise) | — |
| `BL_READ_SECTOR_P_STATUS` | `0x5A` | Read sector protection status | 2 bytes: status |
| `BL_OTP_READ` | `0x5B` | Read OTP memory (student exercise) | — |
| `BL_DIS_R_W_PROTECT` | `0x5C` | Disable all sector protection | 1 byte: status |

**Source Code Mapping:**
- All command codes: `main.h:140-175`
- Supported commands array: `main.c:52-60`
- Command dispatch switch-case: `main.c:127-168`

---

### Lesson 033 — Host-Bootloader Communication Protocol

**Objective:** Define the detailed communication protocol including packet format, CRC, and ACK/NACK.

**Key Concepts:**

**Command Packet Format (Host → Bootloader):**
```
[Length to Follow (1 byte)] [Command Code (1 byte)] [Optional Args] [CRC32 (4 bytes)]
```

**Bootloader Response:**
- **CRC pass:** ACK (`0xA5`) + Length to follow (1 byte) + Reply data
- **CRC fail:** NACK (`0x7F`) — no further data

**CRC Verification:** Uses STM32's hardware CRC engine (32-bit CRC peripheral) for efficient packet integrity checking. The bootloader computes CRC over received data (excluding the 4-byte CRC field) and compares with the host-provided CRC.

**Source Code Mapping:**
- ACK/NACK macros: `main.h:178-179` (`BL_ACK 0xA5`, `BL_NACK 0x7F`)
- CRC macros: `main.h:182-183`
- `bootloader_verify_crc()`: `main.c:865-884`
- `bootloader_send_ack()`: `main.c:848-855`
- `bootloader_send_nack()`: `main.c:858-862`
- CRC peripheral init: `main.c:336-355` (`MX_CRC_Init`)

---

## Section 10: Bootloader Project Creation

### Lesson 034 — Boot-Loader Project Creation

**Objective:** Create the bootloader project in STM32CubeMX with all required peripherals.

**Key Concepts:**
- **Peripherals configured in CubeMX:**
  1. USART2 (async, PA2/PA3) — Command channel via Virtual COM Port
  2. USART3 (async, remapped to PC10/PC11) — Debug channel
  3. CRC engine — For packet integrity verification
- **Pin remapping:** USART3 default pins (PB10/PB11) remapped to PC10/PC11 using CubeMX drag-and-drop
- Project generated for both Keil MDK v5 and STM32 System Workbench

**Source Code Mapping:**
- CubeMX-generated init functions in `main.c`:
  - `MX_GPIO_Init()`: lines 428-492
  - `MX_USART2_UART_Init()`: lines 362-388
  - `MX_USART3_UART_Init()`: lines 395-421
  - `MX_CRC_Init()`: lines 336-355

---

### Lesson 035 — Bootloader Project Exploration Part-1

**Objective:** Understand the auto-generated CubeMX code, focusing on initialization functions.

**Key Concepts:**
- **`HAL_Init()`** — Must be first call in `main()`. Resets peripherals, initializes Flash interface, configures SysTick for 1ms interrupts (required for HAL timeout functionality)
- **`SystemClock_Config()`** — Configures PLL to boost HSI (16 MHz) to 84 MHz. Could be omitted for bootloader (16 MHz is sufficient for 115200 baud UART)
- **`MX_GPIO_Init()`** — Configures user button (input) and LED (output). Enables peripheral clocks via RCC
- **`MX_USART2/3_UART_Init()`** — 115200 baud, 8N1, no hardware flow control
- **`MX_CRC_Init()`** — Initializes the 32-bit hardware CRC engine

**Source Code Mapping:**
- `main()` function calling all inits: `main.c:226-269`
- `SystemClock_Config()`: `main.c:288-329` (PLL: HSI→84MHz)

---

## Section 11: UART Testing

### Lesson 036 — Command UART Testing

**Objective:** Verify USART2 communication by transmitting data to TeraTerm.

**Key Concepts:**
- `HAL_UART_Transmit(&huart2, data, size, HAL_MAX_DELAY)` — Blocking (polling) mode transmission
- Software delay using `HAL_GetTick()` — returns millisecond counter incremented by SysTick interrupt
- Both Keil and System Workbench IDEs demonstrated for building, flashing, and debugging

**Demo:** Transmit "Bootloader\r\n" continuously over USART2 → appears in TeraTerm at 115200 baud on the Virtual COM Port.

---

### Lesson 037 — Debug UART Testing

**Objective:** Verify USART3 communication and implement a printf-like debug function.

**Key Concepts:**
- USART3 requires external USB-to-UART converter (PC10→converter RX, PC11→converter TX)
- **`printmsg()` function** — Custom printf wrapper using `va_list`/`vsprintf` from `<stdarg.h>` and `<string.h>`
- **Conditional compilation:** `#define BL_DEBUG_MSG_EN` enables/disables all debug output at compile time

**Source Code Mapping:**
- `printmsg()` implementation: `main.c:208-219`
- Debug enable macro: `main.c:37` (`#define BL_DEBUG_MSG_EN`)
- UART handle aliases: `main.c:62-63`

---

## Section 12: Jumping to User Application

### Lesson 038 — Boot-loader Jumping to User Application Part-1

**Objective:** Implement the bootloader's boot decision logic based on user button state.

**Key Concepts:**
- On reset, bootloader checks user button (PC13, active low on Nucleo):
  - **Button NOT pressed** → `bootloader_jump_to_user_app()` — hand off to user app
  - **Button pressed** → `bootloader_uart_read_data()` — enter bootloader command mode
- Two stub functions created for later implementation

**Source Code Mapping:**
- Decision logic in `main()`: `main.c:258-269`
  ```c
  if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
      bootloader_uart_read_data();    // Button pressed → bootloader mode
  } else {
      bootloader_jump_to_user_app();  // Button not pressed → user app
  }
  ```
- Button pin definition: `main.h:61` (`B1_Pin = GPIO_PIN_13`, `B1_GPIO_Port = GPIOC`)

---

### Lesson 039 — Boot-loader Jumping to User Application Part-2

**Objective:** Create the user application project with interrupt-driven functionality.

**Key Concepts:**
- User app designed with EXTI interrupt (button press toggles LED) to verify vector table relocation works correctly
- PC13 configured as EXTI with falling edge trigger
- NVIC must explicitly enable EXTI line [15:10] interrupts — two-level enable system (peripheral + NVIC)

**Source Code Mapping:**
- User app source: `USER_APPLICATION/.../Core/Src/main.c`

---

### Lesson 040 — Boot-loader Jumping to User Application Part-3

**Objective:** Relocate user application code to flash sector 2 (0x08008000).

**Key Concepts:**
- **In Keil:** Options for Target → Target tab → Change IROM1 start from `0x08000000` to `0x08008000`
- **In System Workbench:** Edit linker script (`.ld` file) → Change `FLASH ORIGIN` to `0x08008000`
- ST-LINK Utility used to verify: sector 0 = empty (0xFF), sector 2 = user app code
- User app functionality: LED toggle via EXTI callback + "Hello From user application" via UART

---

### Lesson 041 — Flash Code Placement using OpenSTM32 System Workbench

**Objective:** Demonstrate the Eclipse/GCC-based approach to code relocation using linker scripts.

**Key Concepts:**
- Linker script `MEMORY` block defines flash origin and length:
  ```
  FLASH (rx) : ORIGIN = 0x08008000, LENGTH = 512K
  ```
- After modifying linker script, a **clean build** is mandatory
- Memory Browser in Eclipse debug perspective can inspect flash contents during debug sessions

---

### Lesson 042 — Vector Table Offset Register (VTOR) Use Case

**Objective:** Explain why VTOR is essential in a bootloader + user application system.

**Key Concepts:**
- **The Problem:** After bootloader jumps to user app, interrupts still use the bootloader's vector table at `0x0800_0000`. This means user app's interrupt handlers would NOT be called — the bootloader's handlers would run instead
- **The Solution:** The VTOR register (ARM Cortex-M System Control Block) allows relocating the vector table at runtime
- Set `SCB->VTOR = 0x08008000` so the processor uses the user app's vector table for interrupt dispatch
- VTOR value must be aligned to `0x200` (512 bytes) — `0x08008000` satisfies this
- Set VTOR in user app's `system_stm32f4xx.c` via `VECT_TAB_OFFSET = 0x8000`

**Why It Matters:** Without VTOR relocation, any interrupt in the user app would crash or behave incorrectly because the wrong handler would execute.

---

### Lesson 043 — Boot-loader Jumping to User Application Part-4

**Objective:** Implement the complete `bootloader_jump_to_user_app()` function and test end-to-end.

**Key Concepts — The Jump Sequence:**
1. **Read MSP value** from first word at user app base address (`0x08008000`)
2. **Set MSP** via `__set_MSP(msp_value)` — establishes user app's stack
3. **Read reset handler address** from second word at base+4 (`0x08008004`)
4. **Jump via function pointer** — this call never returns

**Demo:** Both bootloader and user app flashed to their sectors. Normal reset → user app runs ("Hello From user application" on TeraTerm, LED toggles on button press). Reset with button held → stays in bootloader mode.

**Source Code Mapping:**
- `bootloader_jump_to_user_app()`: `main.c:177-206`
  ```c
  uint32_t msp_value = *(volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS;
  __set_MSP(msp_value);
  uint32_t resethandler_address = *(volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS + 4);
  void (*app_reset_handler)(void) = (void*) resethandler_address;
  app_reset_handler();
  ```

---

## Section 13: Reading Commands from Host

### Lesson 044 — Boot-loader Command Format

**Objective:** Specify the exact packet format for every bootloader command.

**Key Concepts:**
- Every packet follows: `[Length (1 byte)] [Cmd Code (1 byte)] [Args] [CRC32 (4 bytes)]`
- The "length to follow" byte tells the bootloader how many more bytes to read after the first byte
- CRC32 covers everything except the CRC field itself
- Command codes range from `0x51` to `0x5C`
- `BL_MEM_WRITE` has max 255-byte payload (limited by 1-byte length field)
- `BL_FLASH_ERASE` with sector number `0xFF` = mass erase (special case)
- `BL_EN_RW_PROTECT` uses bitfield encoding: bit N = sector N

---

### Lesson 045 — Boot-loader Read Commands Implementation

**Objective:** Implement the command reception and dispatch mechanism.

**Key Concepts:**
- **Two-phase UART receive:**
  1. Read 1 byte → length byte (`bl_rx_buffer[0]`)
  2. Read `length` more bytes → rest of the packet
- **Switch-case dispatch** on `bl_rx_buffer[1]` (command code) → appropriate handler function
- 200-byte receive buffer: `uint8_t bl_rx_buffer[BL_RX_LEN]`

**Source Code Mapping:**
- `bootloader_uart_read_data()`: `main.c:116-170`
  ```c
  HAL_UART_Receive(C_UART, bl_rx_buffer, 1, HAL_MAX_DELAY);      // Phase 1
  rcv_len = bl_rx_buffer[0];
  HAL_UART_Receive(C_UART, &bl_rx_buffer[1], rcv_len, HAL_MAX_DELAY);  // Phase 2
  switch (bl_rx_buffer[1]) { ... }
  ```
- Buffer declaration: `main.c:82`
- Command code macros: `main.h:140-175`

---

### Lesson 046 — Command Handle Functions Implementation

**Objective:** Create stub implementations for all command handlers to resolve linker errors.

**Key Concepts:**
- All 12 handler functions created as empty stubs in `main.c`
- All share the same signature: `void handler(uint8_t *pBuffer)`
- Student exercises: `BL_MEM_READ` and `BL_OTP_READ` handlers left empty

**Source Code Mapping:**
- Empty handlers: `bootloader_handle_mem_read()` at `main.c:810-813` and `bootloader_handle_read_otp()` at `main.c:841-845`

---

## Section 14: Implementing Bootloader Commands

### Lesson 047 — Boot-loader Command Handling Flow-Chart

**Objective:** Establish the architectural blueprint for all command handlers.

**Key Concepts — Uniform Handler Pattern:**
1. Extract CRC from end of packet
2. Call `bootloader_verify_crc()` → if fail, `bootloader_send_nack()` and return
3. If pass → `bootloader_send_ack(cmd_code, reply_len)`
4. Execute command logic (read register, erase flash, write memory, etc.)
5. Send reply via `bootloader_uart_write_data()`
6. Return to `while(1)` loop in `bootloader_uart_read_data()`

**Four Reusable Infrastructure Functions:**
1. `bootloader_verify_crc()` — validates packet integrity
2. `bootloader_send_ack()` — sends 2-byte ACK (code + reply length)
3. `bootloader_send_nack()` — sends 1-byte NACK
4. `bootloader_uart_write_data()` — transmits reply data

---

### Lesson 048 — BL_GET_VER Handle Function Implementation

**Objective:** Implement the first command handler as a template for all others.

**Source Code Mapping:**
- `bootloader_handle_getver_cmd()`: `main.c:496-523`
  ```c
  uint32_t command_packet_len = bl_rx_buffer[0]+1;
  uint32_t host_crc = *((uint32_t *)(bl_rx_buffer+command_packet_len-4));
  if (!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len-4, host_crc)) {
      bootloader_send_ack(bl_rx_buffer[0], 1);
      bl_version = get_bootloader_version();
      bootloader_uart_write_data(&bl_version, 1);
  } else {
      bootloader_send_nack();
  }
  ```

---

### Lesson 049 — Boot-Loader ACK/NACK Implementation

**Objective:** Implement the ACK and NACK response functions.

**Key Concepts:**
- **NACK:** Single byte `0x7F` → host knows CRC failed, no reply data follows
- **ACK:** Two bytes — `0xA5` (ACK code) + `follow_len` (how many reply bytes follow)
- The `follow_len` field eliminates the need for the host to pre-know reply sizes

**Source Code Mapping:**
- `bootloader_send_ack()`: `main.c:848-855`
- `bootloader_send_nack()`: `main.c:858-862`

---

### Lesson 050 — Boot-Loader Verify CRC

**Objective:** Implement CRC verification using the STM32 hardware CRC engine.

**Key Concepts:**
- CRC computed byte-by-byte using `HAL_CRC_Accumulate()` — each byte promoted to `uint32_t`
- After computation, CRC unit must be reset via `__HAL_CRC_DR_RESET()` to avoid contamination
- Compare computed CRC against host-provided CRC → return `VERIFY_CRC_SUCCESS` (0) or `VERIFY_CRC_FAIL` (1)

**Source Code Mapping:**
- `bootloader_verify_crc()`: `main.c:865-884`
  ```c
  for (uint32_t i=0; i < len; i++) {
      uint32_t i_data = pData[i];
      uwCRCValue = HAL_CRC_Accumulate(&hcrc, &i_data, 1);
  }
  __HAL_CRC_DR_RESET(&hcrc);
  if (uwCRCValue == crc_host) return VERIFY_CRC_SUCCESS;
  return VERIFY_CRC_FAIL;
  ```

---

### Lesson 051 — Sending BL_GET_VER CMD Reply

**Objective:** Complete the BL_GET_VER implementation with version retrieval and UART write.

**Source Code Mapping:**
- `get_bootloader_version()`: `main.c:896-898` — returns `BL_VERSION` macro (`0x10`)
- `bootloader_uart_write_data()`: `main.c:887-891` — wrapper around `HAL_UART_Transmit(C_UART, ...)`
- `BL_VERSION` macro: `main.h:133`

---

### Lessons 052-056 — BL_GET_VER Testing and Debugging

**Objective:** Test the complete system end-to-end and debug step by step.

**Key Concepts:**
- **Host application** available in two versions:
  - **Python:** `HOST/python/STM32_Programmer_V1.py` — cross-platform, uses pySerial
  - **C:** `HOST/C/STM32_Programmer_V1/` — Windows, uses native serial port APIs
- **Testing procedure:** Flash bootloader → hold button + reset → run host script → select command → verify response
- **Verification test:** Change `BL_VERSION` from `0x10` to `0x20`, reflash, confirm host receives new value
- **Debugging:** Keil debugger with breakpoints at each stage — verifies buffer contents, CRC pass/fail, correct dispatch

**Demo — Keil Debug Trace:**
```
Reset → Init → Button Check → bootloader_uart_read_data()
→ HAL_UART_Receive(1 byte: 0x05)
→ HAL_UART_Receive(5 bytes: 0x51, CRC bytes...)
→ switch(0x51) → bootloader_handle_getver_cmd()
→ bootloader_verify_crc() → SUCCESS
→ bootloader_send_ack(0x51, 1) → sends 0xA5, 0x01
→ get_bootloader_version() → returns 0x10
→ bootloader_uart_write_data(0x10) → sends to host
→ Loop back for next command
```

---

### Lesson 057 — BL_GET_HELP CMD Implementation and Testing

**Objective:** Implement the "get help" command that returns all supported command codes.

**Key Concepts:**
- The bootloader maintains a global array `supported_commands[]` containing all command codes
- Reply sends the entire array using `sizeof(supported_commands)` as the length

**Source Code Mapping:**
- `bootloader_handle_gethelp_cmd()`: `main.c:528-546`
- `supported_commands[]` array: `main.c:52-60`
  ```c
  uint8_t supported_commands[] = { BL_GET_VER, BL_GET_HELP, BL_GET_CID,
      BL_GET_RDP_STATUS, BL_GO_TO_ADDR, BL_FLASH_ERASE,
      BL_MEM_WRITE, BL_READ_SECTOR_P_STATUS };
  ```

---

### Lesson 058 — BL_GET_CID CMD Implementation and Testing

**Objective:** Implement chip identification number retrieval.

**Key Concepts:**
- The STM32F446xx MCU ID is stored in the `DBGMCU->IDCODE` register (part of the debug component)
- Device identifier occupies bits [0:11] of the IDCODE register
- For STM32F446xx, the expected device ID is `0x421`
- The register address is defined in `stm32f446xx.h` (CMSIS device header)

**Source Code Mapping:**
- `get_mcu_chip_id()`: `main.c:902-913`
  ```c
  uint16_t cid = (uint16_t)(DBGMCU->IDCODE) & 0x0FFF;
  return cid;
  ```
- `bootloader_handle_getcid_cmd()`: `main.c:549-571` — sends 2-byte CID to host

---

### Lesson 059 — Understanding Flash Read Protection Levels

**Objective:** Explain the three flash Read Protection (RDP) levels on STM32.

**Key Concepts:**

| Level | RDP Value | Protection |
|---|---|---|
| Level 0 | `0xAA` | No protection — full read/write/erase access in all boot modes |
| Level 1 | Any value except `0xAA`/`0xCC` | No debug access to flash (read/erase/program blocked via debugger). Flash access from user code is allowed. **Going back to Level 0 triggers mass erase** |
| Level 2 | `0xCC` | All Level 1 protections + no boot from RAM/system memory, JTAG/SWD disabled, option bytes locked. **IRREVERSIBLE — never use during development** |

- RDP is configured via option bytes at address `0x1FFFC000`, bits [8:15]
- Option bytes are 16 bytes that control flash security, write protection, and BOR level

**Source Code Mapping:**
- `get_flash_rdp_level()`: `main.c:920-935`
  ```c
  volatile uint32_t *pOB_addr = (uint32_t*) 0x1FFFC000;
  rdp_status = (uint8_t)(*pOB_addr >> 8);
  ```
- Alternative HAL implementation (commented out): `HAL_FLASHEx_OBGetConfig(&ob_handle)`

---

### Lesson 060 — BL_GET_RDP_LEVEL Command Testing

**Objective:** Test the RDP level read command.

**Demo:** Host sends command → bootloader reads option bytes at `0x1FFFC000` → returns `0xAA` (Level 0, no protection). Verified by cross-checking with ST-LINK Utility option bytes window.

**Source Code Mapping:**
- `bootloader_handle_getrdp_cmd()`: `main.c:574-595`

---

### Lesson 061 — BL_GO_TO_ADDR Command Implementation

**Objective:** Implement the "go to address" command that jumps to a specified memory location.

**Key Concepts:**
- Host sends a 4-byte target address
- Bootloader **validates the address** before jumping — only allows jumps to: SRAM1, SRAM2, Flash, Backup SRAM, external memory. Peripheral addresses are **rejected**
- The jump address must have bit 0 set (T-bit = 1) for Thumb instruction execution on Cortex-M → `go_address += 1`
- Jump is executed via function pointer — this call never returns

**Source Code Mapping:**
- `bootloader_handle_go_cmd()`: `main.c:598-654`
  ```c
  go_address = *((uint32_t *)&pBuffer[2]);
  if (verify_address(go_address) == ADDR_VALID) {
      bootloader_uart_write_data(&addr_valid, 1);
      go_address += 1;  // make T bit = 1
      void (*lets_jump)(void) = (void *)go_address;
      lets_jump();
  } else {
      bootloader_uart_write_data(&addr_invalid, 1);
  }
  ```
- `verify_address()`: `main.c:938-963` — checks against SRAM1/SRAM2/Flash/BackupSRAM ranges
- Memory range macros: `main.h:192-198`

---

### Lesson 062 — BL_GO_TO_ADDR Command Testing

**Objective:** Test the go-to-address command with valid and invalid addresses.

**Demo:**
- `0x40000000` (peripheral address) → returns "address invalid" (status = 1)
- `0x08000000` (flash base) → returns "address valid" (status = 0)
- `0x080081D8` (user app reset handler) → jumps to user app → "Hello From user application" appears on TeraTerm, LED toggles work

---

### Lessons 063-064 — BL_FLASH_ERASE Command Implementation

**Objective:** Implement flash erase supporting both sector erase and mass erase.

**Key Concepts:**
- **Sector erase:** Specify starting sector (0-7) and number of sectors
- **Mass erase:** Special case — sector number `0xFF` triggers mass erase of all sectors
- Uses HAL Flash driver: `HAL_FLASHEx_Erase(&flashErase_handle, &sectorError)`
- Flash must be unlocked (`HAL_FLASH_Unlock()`) before erase and locked afterward
- `FLASH_EraseInitTypeDef` structure specifies: TypeErase, Sector, NbSectors, VoltageRange
- Voltage range: `FLASH_VOLTAGE_RANGE_3` (2.7V–3.6V for Nucleo board)

**Source Code Mapping:**
- `bootloader_handle_flash_erase_cmd()`: `main.c:657-687`
- `execute_flash_erase()`: `main.c:965-1006`
  ```c
  if (sector_number == 0xff) {
      flashErase_handle.TypeErase = FLASH_TYPEERASE_MASSERASE;
  } else {
      flashErase_handle.TypeErase = FLASH_TYPEERASE_SECTORS;
      flashErase_handle.Sector = sector_number;
      flashErase_handle.NbSectors = number_of_sector;
  }
  HAL_FLASH_Unlock();
  status = HAL_FLASHEx_Erase(&flashErase_handle, &sectorError);
  HAL_FLASH_Lock();
  ```
- LED indicates erase in progress: `main.c:674-676`

---

### Lessons 065-066 — Testing Flash Sector Erase and Mass Erase

**Objective:** Verify sector erase and mass erase using the host application and ST-LINK Utility.

**Demo — Sector Erase:**
- Erase sector 2: Command 7, sector=2, count=1 → success → ST-LINK shows all `0xFF` at `0x08008000`
- Erase sectors 2+3: Command 7, sector=2, count=2 → success → both sectors erased

**Demo — Mass Erase:**
- Command 7, sector=`0xFF` → all sectors erased (including bootloader!)
- After mass erase, bootloader no longer responds (it was erased) — must be re-flashed via ST-Link

---

### Lesson 067 — BL_MEM_WRITE Command Implementation

**Objective:** Implement memory write for In-Application Programming.

**Key Concepts:**
- Command packet: `[length] [0x57] [4-byte address] [1-byte payload_len] [payload] [CRC32]`
- Maximum payload per packet: 255 bytes (limited by 1-byte length field)
- For large binaries, host sends multiple `BL_MEM_WRITE` commands with incrementing addresses
- Uses `HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, data)` — programs byte by byte
- Flash must be unlocked before writing and locked after
- Binary file generated from Keil using `fromelf` post-build command

**Source Code Mapping:**
- `bootloader_handle_mem_write_cmd()`: `main.c:690-749`
- `execute_mem_write()`: `main.c:1011-1026`
  ```c
  HAL_FLASH_Unlock();
  for (uint32_t i = 0; i < len; i++) {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, mem_address+i, pBuffer[i]);
  }
  HAL_FLASH_Lock();
  ```

---

### Lesson 068 — BL_MEM_WRITE Command Testing

**Objective:** Test the complete In-Application Programming workflow.

**Demo — Complete IAP via Custom Bootloader:**
1. Generate `user_app.bin` using Keil `fromelf` command
2. Place binary in same directory as Python host script
3. Erase sector 2 using `BL_FLASH_ERASE` command
4. Run `BL_MEM_WRITE` (command 8) with base address `0x08008000`
5. Host automatically chunks the 4792-byte binary into 255-byte packets (19 iterations)
6. Use `BL_GO_TO_ADDR` (command 5) with reset handler address `0x080081D8`
7. User application starts running — "Hello From user application" on TeraTerm

**Why It Matters:** This demonstrates the full bootloader use case — field-upgrading firmware without a debug probe.

---

## Section 21: Option Bytes and Flash Sector Protection

### Lesson 069 — Option Bytes Programming

**Objective:** Understand flash sector write protection and PCROP (read/write protection).

**Key Concepts:**
- **Option bytes** at address `0x1FFFC008` control per-sector protection via nWRP bits (bits 0-7 = sectors 0-7)
- **SPRMOD bit** (bit 15 of FLASH_OPTCR register at `0x40023C14`) selects protection mode:
  - SPRMOD=0: nWPRi bits control **write protection** (0=protected, 1=unprotected)
  - SPRMOD=1: nWPRi bits control **PCROP** (Proprietary Code Readout Protection = read+write)
- **Changing from PCROP back to no protection triggers full chip mass erase** — important safety consideration
- **Option byte modification procedure:**
  1. Check BSY flag in FLASH_SR
  2. Write desired value to FLASH_OPTCR register
  3. Set OPTSTRT bit in FLASH_OPTCR
  4. Wait for BSY to clear

**Demo:** ST-LINK Utility used to enable write protection on sectors 0+1, verify Keil can't flash (error), remove protection, flash succeeds. Then PCROP enabled — can't even read flash contents. Disabling PCROP triggers mass erase warning.

---

### Lesson 070 — Implementing Flash Sector Protection Commands

**Objective:** Implement enable/disable read-write protection commands.

**Key Concepts:**
- **`BL_EN_RW_PROTECT` (0x58):** Two fields — sector details (8-bit bitfield, each bit = one sector) and protection mode (1=write, 2=read/write)
- **`BL_DIS_R_W_PROTECT` (0x5C):** No special fields — clears all protection, returns to default state
- Both use the same core function: `configure_flash_sector_rw_protection(sector_details, protection_mode, disable)`

**Source Code Mapping:**
- `bootloader_handle_en_rw_protect()`: `main.c:752-777`
- `bootloader_handle_dis_rw_protect()`: `main.c:781-807`
- `configure_flash_sector_rw_protection()`: `main.c:1038-1129`
  - Write protection (mode=1): Clear bit 31, clear nWRP bits for target sectors
  - Read/write protection (mode=2): Set bit 31, set nWRP bits for target sectors
  - Disable (disable=1): Clear bit 31, set all nWRP bits to 1 (unprotected)
  - All paths: unlock option bytes → wait for BSY → modify OPTCR → set OPTSTRT → wait → lock

- `read_OB_rw_protection_status()`: `main.c:1131-1144` — reads current sector protection via `HAL_FLASHEx_OBGetConfig()`
- `bootloader_handle_read_sector_protection_status()`: `main.c:816-838`

---

### Lesson 071 — Summary of the Commands

**Objective:** Verify all implemented commands work correctly in sequence.

**Demo:** All commands tested sequentially: GET_VER → GET_HELP → GET_CID → GET_RDP_STATUS → GO_TO_ADDR (with user app) → FLASH_ERASE → MEM_WRITE → sector protection enable/disable/read status. All pass successfully.

---

## Section 22: Exploring the Host Application

### Lesson 072 — HOST Application Source Files and Details

**Objective:** Walk through the host application's source code architecture.

**Key Concepts:**

**C Host Application Structure (`HOST/C/STM32_Programmer_V1/`):**

| File | Purpose |
|---|---|
| `Sources/main.c` | Menu display, user input |
| `Sources/BlCommands.c` | Command packet construction and sending |
| `Sources/BlReplyProcessing.c` | Parse bootloader responses (ACK/NACK + data) |
| `Sources/WindowsSerialPort.c` | Windows serial port APIs |
| `Sources/LinuxSerialPort.c` | Linux serial port APIs (to be implemented) |
| `Sources/OSxSerialPort.c` | macOS serial port APIs (to be implemented) |
| `Sources/fileops.c` | File I/O for reading binary files |
| `Sources/utilities.c` | CRC calculation, byte conversion helpers |

- OS-dependent files: `WindowsSerialPort.c`, `LinuxSerialPort.c`, `OSxSerialPort.c` — provide the same function signatures but different implementations
- OS-independent files: All other source files — reusable across platforms

**Python Host Application:** `HOST/python/STM32_Programmer_V1.py` — single file, cross-platform using pySerial

---

### Lesson 073 — Procedure to Add Your Own Command

**Objective:** Teach how to extend the bootloader with custom commands.

**Key Concepts — Adding a New Command (Step by Step):**

**On the Host Side:**
1. Define new command code in `main.h` (e.g., `#define COMMAND_BL_MY_NEW_COMMAND 0x5D`)
2. Define command length macro
3. Add case in `decode_menu_command_code()` in `BlCommands.c` — populate data buffer with length, command code, params, CRC
4. Add case in `read_bootloader_reply()` in `BlReplyProcessing.c` — process the reply
5. Add reply processing helper function
6. Add menu entry in `main.c`

**On the Bootloader Side:**
1. Add command code macro in `main.h`
2. Add case in switch-case in `bootloader_uart_read_data()` in `main.c`
3. Implement handle function following the standard pattern (CRC check → ACK/NACK → execute → reply)

---

## Course Overview: How Topics Connect

### The Big Picture

This course builds a **complete In-Application Programming (IAP) system** from the ground up. The system consists of three components that work together:

```
┌─────────────────┐     UART (115200)     ┌──────────────────┐
│   HOST PC        │◄──────────────────────►│   STM32 MCU      │
│                  │     (Command UART)     │                  │
│  Python/C App    │                        │  Custom          │
│  STM32_Programmer│                        │  Bootloader      │
│                  │                        │  (Sectors 0-1)   │
└─────────────────┘                        │                  │
                                           │  User App        │
                                           │  (Sectors 2-7)   │
                                           └──────────────────┘
```

### How the Sections Build on Each Other

**1. Foundation Layer (Sections 2-4):**
- Section 2 establishes *why* bootloaders exist (IAP vs ISP) and *what* they do
- Section 3 provides the *hardware knowledge* that makes everything work: memory map, reset sequence, memory aliasing, and boot pin configuration
- Section 4 sets up the *development environment*: board selection, driver installation, ST-Link firmware
- **Connection:** Understanding the reset sequence (MSP read → reset handler jump) is directly replicated in the bootloader's `bootloader_jump_to_user_app()` function

**2. Exploration Layer (Section 8):**
- Explores ST's *native* bootloader to understand what a working bootloader looks like
- Demonstrates the complete IAP workflow using ST's Flash Loader Demonstrator
- **Connection:** The custom bootloader will replicate this workflow but with a custom protocol and more features

**3. Architecture Layer (Section 9):**
- Designs the custom bootloader's *communication architecture*: dual-UART transport, flash partitioning, command set, packet protocol
- **Connection:** Every design decision here directly shapes the implementation in later sections. The packet format (length + command + args + CRC) and ACK/NACK protocol become the backbone of all command handlers

**4. Implementation Layer (Sections 10-14):**
- Section 10: Project creation and peripheral initialization (CubeMX generates the scaffold)
- Section 11: UART testing validates the communication channels
- Section 12: `bootloader_jump_to_user_app()` implements the most critical feature — the bridge between bootloader and user code. VTOR relocation ensures interrupts work correctly
- Section 13: Command reception infrastructure (two-phase receive, switch-case dispatch, handler stubs)
- Section 14: Individual command handlers, each following the uniform pattern:
  ```
  CRC verify → ACK/NACK → execute logic → send reply
  ```
- **Connection:** Each command builds on the infrastructure from Section 13. The `BL_MEM_WRITE` + `BL_GO_TO_ADDR` combination achieves the full IAP workflow that was demonstrated manually in Section 8

**5. Security Layer (Section 21):**
- Adds flash protection capabilities (write protection, PCROP read/write protection)
- Option bytes programming allows protecting bootloader code from accidental erasure
- **Connection:** Builds on the flash memory knowledge from Section 3 and the command framework from Sections 13-14

**6. Extension Layer (Section 22):**
- Explains the host application architecture and how to add custom commands
- **Connection:** Closes the loop by showing both sides of the system and enabling students to extend it

### Key Technical Threads

**Thread 1: Memory Architecture → Code Placement → VTOR**
- Memory organization (Lesson 006) → Flash partitioning (Lesson 031) → Code relocation (Lesson 040) → VTOR for interrupt routing (Lesson 042) → Jump implementation (Lesson 043)

**Thread 2: Reset Sequence → Boot Decision → User App Jump**
- ARM reset sequence (Lesson 007) → Boot pins (Lesson 008) → Button check logic (Lesson 038) → MSP + reset handler read + function pointer jump (Lesson 043)

**Thread 3: UART Setup → Protocol Design → Command Framework → Individual Commands**
- UART testing (Lessons 036-037) → Packet format design (Lesson 033) → Two-phase receive + dispatch (Lesson 045) → CRC/ACK/NACK infrastructure (Lessons 049-050) → Each command handler (Lessons 057-070)

**Thread 4: Flash Operations Pipeline**
- Flash memory theory (Lesson 006) → Erase implementation (Lessons 063-066) → Write implementation (Lessons 067-068) → Protection management (Lessons 069-070) → Complete IAP demonstration (Lesson 068)

### The Complete IAP Workflow (Tying It All Together)

The ultimate goal of the course is demonstrated in Lesson 068:
1. **Generate binary** from user application project (`fromelf` command)
2. **Enter bootloader mode** (hold button + reset)
3. **Erase target sectors** (`BL_FLASH_ERASE`, command 7)
4. **Write binary to flash** (`BL_MEM_WRITE`, command 8) — host chunks the binary into 255-byte packets
5. **Jump to user app** (`BL_GO_TO_ADDR`, command 5) — bootloader reads reset handler from sector 2 and jumps
6. **User application runs** — with proper interrupt handling via VTOR relocation

This workflow demonstrates true field-upgradeable firmware — the entire user application can be replaced over a simple UART connection without any debugging hardware.
