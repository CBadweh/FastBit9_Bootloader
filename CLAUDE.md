# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## Project Overview

A custom STM32 bootloader built from scratch, following the FastBit Embedded Brain Academy course. Ported from the course's STM32F446RE (Keil/TeraTerm) to an **STM32F401RE** (STM32CubeIDE/PuTTY). Two separately-compiled projects share flash: the bootloader in Sectors 0–1 and the user application in Sector 2+.

**Primary board:** NUCLEO-F401RE (Cortex-M4, 512KB Flash, 96KB SRAM)

---

## Build & Flash

No CLI build — all compilation and flashing is done via **STM32CubeIDE**.

**Build:** `Project → Build Project` (Ctrl+B)
**Flash:** `Run → Debug As → STM32 C/C++ Application` (auto-flashes via ST-Link)

Flash the two projects in order:
1. `Badweh_Bootloader_Milestone123456/` → writes to `0x08000000`
2. `Badweh_UserApplication/` → writes to `0x08008000` (does **not** erase bootloader sectors)

To verify flash contents: STM32CubeProgrammer → ST-LINK → Connect → Memory & File Editing, read at `0x08000000` and `0x08008000`.

**Debug output:** Connect external USB-to-UART to PC6/PC7 (USART6) at 115200 8N1. Controlled by `BL_DEBUG_MSG_EN` macro in `main.h`.

---

## Repository Structure

```
Bootloader_Code/Badweh_Code/
├── Badweh_Bootloader/                  ← Initial skeleton project
├── Badweh_Bootloader_Milestone123/     ← Milestones 1–3 (jump to user app)
├── Badweh_Bootloader_Milestone123456/  ← CURRENT: full implementation (Milestones 1–6)
└── Badweh_UserApplication/             ← Separately-compiled user app at 0x08008000
```

All active development is in `Badweh_Bootloader_Milestone123456/` and `Badweh_UserApplication/`.

---

## Architecture

### Memory Layout

```
0x08000000  Sector 0 (16KB) ─┐  Bootloader (Milestones 1–6)
0x08004000  Sector 1 (16KB) ─┘
0x08008000  Sector 2 (16KB) ─┐  User Application
0x0800C000  Sector 3 (16KB)  │
0x08010000  Sector 4 (64KB)  │  (480KB available)
0x08020000  Sector 5 (128KB) │
0x08040000  Sector 6 (128KB) │
0x08060000  Sector 7 (128KB)─┘
0x20000000  SRAM (96KB)
```

The bootloader linker script (`STM32F401RETX_FLASH.ld`) uses `ORIGIN = 0x08000000`.
The user app linker script uses `ORIGIN = 0x08008000`.

### Boot Decision (`main.c`)

On every reset, the bootloader checks PC13 (button, active LOW):
- **Button NOT held** → `bootloader_jump_to_user_app()` — hands off to user app
- **Button held** → `bootloader_uart_read_data()` — stays in BL, waits for host commands

### Jump to User Application

`bootloader_jump_to_user_app()` in `Badweh_Bootloader_Milestone123456/Core/Src/main.c` manually replicates the ARM reset sequence aimed at `0x08008000`:
1. Read word at `0x08008000` → set as MSP (`__set_MSP()`)
2. Read word at `0x08008004` → user app `Reset_Handler` address
3. Call it via function pointer — never returns

### VTOR (Critical for User App Interrupts)

After jumping, the processor would still look for interrupt vectors at `0x08000000` (bootloader's table). The user app relocates VTOR in `SystemInit()` before `main()`:

```c
// Badweh_UserApplication/Core/Src/system_stm32f4xx.c
#define VECT_TAB_OFFSET  0x00008000U
// SystemInit(): SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
```

Without this, the user app's button EXTI interrupt calls the bootloader's handler instead.

### Command Protocol (Bootloader Mode)

All UART commands use USART2 (PA2/PA3, Virtual COM Port via ST-Link):

```
Byte 0:        Length (N = bytes that follow)
Byte 1:        Command code
Bytes 2..N-4:  Parameters
Bytes N-3..N:  CRC32 (4 bytes, little-endian)
```

Response: `0xA5 <follow_len> <data>` (ACK) or `0x7F` (NACK)

| Code | Command | Status |
|------|---------|--------|
| 0x51 | `BL_GET_VER` — returns version `0x10` | Implemented |
| 0x56 | `BL_FLASH_ERASE` — sector or mass erase | Implemented |
| 0x52–0x55, 0x57–0x58 | Other commands | Stubbed |

---

## Key Files

| File | Purpose |
|------|---------|
| `Badweh_Bootloader_Milestone123456/Core/Src/main.c` | All bootloader logic: jump, boot decision, UART read loop, command handlers, CRC verify, ACK/NACK |
| `Badweh_Bootloader_Milestone123456/Core/Inc/main.h` | Command codes (`BL_GET_VER = 0x51`, etc.), `FLASH_SECTOR2_BASE_ADDRESS`, `BL_VERSION`, `BL_DEBUG_MSG_EN` |
| `Badweh_UserApplication/Core/Src/system_stm32f4xx.c` | `VECT_TAB_OFFSET = 0x8000` — VTOR relocation |
| `Badweh_UserApplication/Core/Src/stm32f4xx_it.c` | `EXTI15_10_IRQHandler` → LED (PA5) toggle on button (PC13) |
| `Badweh_UserApplication/STM32F401RETX_FLASH.ld` | `FLASH ORIGIN = 0x08008000` |
| `Badweh_Bootloader_Milestone123456/Debug/README_TESTING.md` | Hardware setup, test procedures, F401RE vs F446RE differences |

---

## F401RE vs F446RE (Course Board Differences)

The course uses F446RE with Keil and TeraTerm. This project uses F401RE with STM32CubeIDE and PuTTY. Key deltas:

| | Course (F446RE) | This Project (F401RE) |
|-|-----------------|-----------------------|
| Chip ID | 0x0421 | **0x0433** |
| SRAM | 128KB | **96KB** (no SRAM2) |
| Clock | 180 MHz | **84 MHz** |
| Debug UART | USART3 (PC10/PC11) | **USART6 (PC6/PC7)** |

All flash addresses, jump logic, VTOR offsets, and command codes are identical between boards.

---

## GPIO Pin Map

| Pin | Function |
|-----|----------|
| PC13 | User button B1 (active LOW) |
| PA5  | LED LD2 |
| PA2/PA3 | USART2 TX/RX — Command UART (Virtual COM Port) |
| PC6/PC7 | USART6 TX/RX — Debug UART (external USB-to-UART) |
