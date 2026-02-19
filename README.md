# STM32 Custom Bootloader — FastBit9 Course

**Board:** NUCLEO-F401RE (STM32F401RETx)
**IDE:** STM32CubeIDE
**Terminal:** PuTTY (115200 baud)
**Course:** FastBit Embedded Brain Academy — STM32 Bootloader Development

---

## Overview

A **bootloader** is a small program stored at the beginning of flash memory that runs every time the MCU resets. Its job is to either update the user application (In-Application Programming) or hand off control to it.

This project implements a custom bootloader for the STM32F401RE from the ground up. The bootloader sits in flash **Sectors 0–1** (`0x08000000`) and the user application sits in **Sector 2+** (`0x08008000`). On every reset, the bootloader runs first, checks the user button, and decides:

- **Button NOT held** → jump to user application
- **Button held during reset** → stay in bootloader mode (accept host commands)

By Milestone 3 (Lesson 043), the bootloader can successfully jump to a separately-compiled user application that uses UART and GPIO interrupts — proving the full handoff works correctly.

---

## Bootloader Topics

Three concepts are foundational to understanding everything the bootloader does. Each maps to one milestone.

---

### Milestone 1 — ARM Reset Sequence & Memory Aliasing
**Lesson 007 | Section 3**

**The concept:** Every time an ARM Cortex-M processor resets, it performs exactly two reads from memory before executing any code:

```
Address 0x00000000 → loaded into SP  (Main Stack Pointer)
Address 0x00000004 → loaded into PC  (Reset Handler address → execution starts here)
```

**Memory Aliasing:** The STM32 hardware maps flash (`0x08000000`) onto address `0x00000000` transparently. So reading address `0x00000000` actually returns data from `0x08000000`. The processor never knows — it always reads from "address 0" and the hardware redirects it.

**Why it matters:** The bootloader's jump function replicates this exact sequence manually, but aimed at the user application's base address (`0x08008000`) instead of address 0. Without understanding the reset sequence, the jump code looks like unexplained pointer magic.

```
Flash at 0x08008000 (user app vector table):
┌──────────────┬──────────────────────────────────────┐
│ 0x08008000   │  Initial MSP value  (read → set SP)  │
│ 0x08008004   │  Reset_Handler addr (read → set PC)  │
│ 0x08008008   │  NMI_Handler addr                    │
│ ...          │  ...more interrupt vectors...         │
└──────────────┴──────────────────────────────────────┘
```

---

### Milestone 2 — VTOR: Vector Table Offset Register
**Lesson 042 | Section 12**

**The problem:** After the bootloader jumps to the user application, the ARM processor still looks for interrupt vectors at `0x08000000` (the bootloader's vector table). If the user application triggers any interrupt — like the button press EXTI — the processor would call the **bootloader's** handler, not the user app's. This is a silent failure that's very hard to debug.

**The fix:** The VTOR (Vector Table Offset Register) in the ARM System Control Block tells the processor where to find the vector table at runtime.

```c
// In user app: Bootloader_Code/Badweh_Code/Badweh_UserApplication/Core/Src/system_stm32f4xx.c
#define VECT_TAB_OFFSET  0x00008000U   // Offset from 0x08000000 → points to 0x08008000
// SystemInit() sets: SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
```

**Key constraint:** VTOR must be set in the user app's `SystemInit()` — which runs at the very start of `Reset_Handler`, before `main()`. This ensures interrupts are correctly routed from the moment the user app starts.

---

### Milestone 3 — Bootloader Jump to User Application
**Lesson 043 | Section 12**

**The core bootloader operation.** This function manually replicates the ARM reset sequence, but pointed at the user application stored in Sector 2.

```c
// Bootloader_Code/Badweh_Code/Badweh_Bootloader_Milestone123/Core/Src/main.c
void bootloader_jump_to_user_app(void)
{
    void (*app_reset_handler)(void);

    // Step 1: Read the initial MSP from user app vector table word 0
    uint32_t msp_value = *(volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS;
    __set_MSP(msp_value);                          // Set SP → user app's stack

    // Step 2: Read reset handler address from user app vector table word 1
    uint32_t resethandler_address = *(volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS + 4);
    app_reset_handler = (void *) resethandler_address;

    // Step 3: Jump — this call never returns
    app_reset_handler();
}
```

The address at `FLASH_SECTOR2_BASE_ADDRESS + 4` is the user app's `Reset_Handler` (with bit 0 set = Thumb mode indicator). Once called, the bootloader ceases to exist from the processor's perspective — SP and PC now belong to the user application.

**Boot decision (main.c):**
```c
if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
    // Button pressed (PC13 pulled LOW) → bootloader mode
    bootloader_uart_read_data();
} else {
    // Button not pressed → hand off to user application
    bootloader_jump_to_user_app();
}
```

---

## Requirements

### Hardware
- **NUCLEO-F401RE** board (STM32F401RETx, Cortex-M4, 512KB Flash, 96KB SRAM)
- **USB cable** — powers the board and exposes Virtual COM Port via ST-Link (no extra hardware needed for Command UART)
- **Optional:** External USB-to-UART converter for Debug UART (USART6 on PC6/PC7)

### Software
- **STM32CubeIDE** — build and flash both projects
- **STM32CubeProgrammer** — inspect flash memory contents (verify code placement at correct addresses)
- **PuTTY** (or any serial terminal) — 115200 baud, 8N1, no flow control

### GPIO Pin Map

| Pin | Function | Direction |
|-----|----------|-----------|
| PC13 | User button B1 (active LOW) | Input |
| PA5  | LED LD2 | Output |
| PA2  | USART2 TX (Command UART → Virtual COM Port) | Output |
| PA3  | USART2 RX | Input |
| PC6  | USART6 TX (Debug UART) | Output |
| PC7  | USART6 RX | Input |

---

## Memory Layout

**STM32F401RE Flash (512KB) Sector Map:**

```
Address         Size    Content
─────────────────────────────────────────────────────
0x08000000      16KB    Sector 0 ─┐ Bootloader
0x08004000      16KB    Sector 1 ─┘ (Badweh_Bootloader_Milestone123)
0x08008000      16KB    Sector 2 ─┐ User Application     ← FLASH_SECTOR2_BASE_ADDRESS
0x0800C000      16KB    Sector 3  │ (Badweh_UserApplication)
0x08010000      64KB    Sector 4  │
0x08020000     128KB    Sector 5 ─┘
─────────────────────────────────────────────────────
SRAM: 0x20000000   96KB    Shared (not simultaneously)
```

**Linker Script Settings:**

| Project | FLASH ORIGIN | FLASH LENGTH |
|---------|-------------|--------------|
| Bootloader | `0x08000000` | `512K` |
| User Application | `0x08008000` | `480K` |

User app linker script location:
`Bootloader_Code/Badweh_Code/Badweh_UserApplication/STM32F401RETX_FLASH.ld`

---

## How to Run

### Step 1 — Flash the Bootloader

1. Open `Bootloader_Code/Badweh_Code/Badweh_Bootloader_Milestone123/` in STM32CubeIDE
2. **Build:** `Project → Build Project` (Ctrl+B)
3. **Flash:** `Run → Debug` → click Resume, then Terminate
4. The bootloader is now at `0x08000000`

### Step 2 — Flash the User Application

1. Open `Bootloader_Code/Badweh_Code/Badweh_UserApplication/` in STM32CubeIDE
2. Verify `STM32F401RETX_FLASH.ld` has `ORIGIN = 0x08008000`
3. **Build:** `Project → Build Project` (Ctrl+B)
4. **Flash:** `Run → Debug` → Resume → Terminate
5. The user app is now at `0x08008000` — the bootloader at `0x08000000` is untouched

> **Note:** Flashing the user app from STM32CubeIDE only programs Sector 2 onward. It does not erase Sector 0–1 where the bootloader lives.

### Step 3 — Run and Observe

Open PuTTY on the board's Virtual COM Port (115200 baud, 8N1).

**Normal boot (button not held):**
1. Press reset button on board
2. PuTTY shows debug messages from bootloader (via USART6 if connected, or nothing on USART2):
   ```
   BL_DEBUG_MSG: Button is not pressed.. Jumping to user app
   BL_DEBUG_MSG: bootloader_jump_to_user_app
   BL_DEBUG_MSG: MSP value: 0x20018000
   BL_DEBUG_MSG: App reset handler addr: 0x80088d1
   ```
3. User application starts — PuTTY shows on USART2:
   ```
   Hello From User Application
   ```
4. Press the blue button → LED (PA5) toggles (EXTI interrupt working via VTOR)

**Bootloader mode (button held during reset):**
1. Hold blue button, press reset, release button
2. PuTTY shows:
   ```
   BL_DEBUG_MSG: Button is pressed.. Going to BL mode
   BL_DEBUG_MSG: Bootloader mode entered.
   BL_DEBUG_MSG: Waiting for commands...
   ```
3. Bootloader waits (command handling added in later milestones)

### Verifying Flash Contents (STM32CubeProgrammer)

Connect STM32CubeProgrammer → ST-LINK → Connect → Memory & File Editing:
- Read at `0x08000000`: should show bootloader code (non-0xFF data)
- Read at `0x08008000`: should show user app code (non-0xFF data, starts with MSP value)
- First word at `0x08008000` = `0x20018000` (MSP initial value)
- Second word at `0x08008004` = user app Reset_Handler address (odd number, Thumb bit set)

---

## Project Milestones

### Milestone 1 — ARM Reset Sequence & Memory Aliasing
**Lesson 007 | Conceptual foundation**

The most important lesson in the course. Explains how ARM Cortex-M boots:
- Processor always reads from address `0x00000000` for MSP and reset handler
- Memory aliasing hardware maps flash (`0x08000000`) to address `0x00000000`
- Vector table: word 0 = initial SP, word 1 = Reset_Handler, word 2 = NMI_Handler, ...
- Boot pins (BOOT0/BOOT1) control which memory region is aliased to address 0

This reset sequence is directly reused in `bootloader_jump_to_user_app()` in Milestone 3.

---

### Milestone 2 — VTOR: Vector Table Offset Register
**Lesson 042 | Section 12 | Affects: User Application**

Solves the interrupt routing problem when two applications share one MCU:
- Without VTOR: user app's button interrupt would call bootloader's handler → wrong behavior
- With VTOR: `SCB->VTOR = 0x08008000` → processor uses user app's vector table

Set in the user application at:
`Bootloader_Code/Badweh_Code/Badweh_UserApplication/Core/Src/system_stm32f4xx.c`

```c
#define VECT_TAB_OFFSET  0x00008000U
// SystemInit() executes: SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
```

Test: Press button after bootloader jumps to user app → LED toggles → VTOR is working.

---

### Milestone 3 — Bootloader Jump to User Application
**Lesson 043 | Section 12 | Affects: Bootloader + User Application**

Implements the core bootloader function — the bridge between bootloader and user app.

**Bootloader side** (`Badweh_Bootloader_Milestone123/Core/Src/main.c`):
- `bootloader_jump_to_user_app()`: reads MSP + reset handler from Sector 2, sets SP, jumps
- Boot decision in `main()`: button state determines BL mode vs. jump

**User application side** (`Badweh_UserApplication`):
- Linker script: `FLASH ORIGIN = 0x08008000` — places vector table at Sector 2
- `system_stm32f4xx.c`: `VECT_TAB_OFFSET = 0x8000` — relocates VTOR on startup
- `stm32f4xx_it.c`: `EXTI15_10_IRQHandler` → `HAL_GPIO_EXTI_Callback` → LED toggle
- `main.c`: Prints `"Hello From User Application"` via USART2

---

## Lessons Summary (034–043)

| Lesson | Section | Project | What Was Built |
|--------|---------|---------|----------------|
| 034 | S10 | Bootloader | CubeMX project creation: USART2 (command), USART6 (debug), CRC peripheral |
| 035 | S10 | Bootloader | Explored generated HAL init code: `HAL_Init`, `SystemClock_Config`, GPIO/UART/CRC init |
| 036 | S11 | Bootloader | USART2 TX test using `HAL_UART_Transmit()`, delay via `HAL_GetTick()` |
| 037 | S11 | Bootloader | `printmsg()` debug function — printf wrapper using `va_list`/`vsprintf` over USART6 |
| 038 | S12 | Bootloader | Boot decision logic: button check → `bootloader_uart_read_data()` or `bootloader_jump_to_user_app()` |
| 039 | S12 | User App | Created user app project: EXTI interrupt (button → LED toggle), USART2 output |
| 040 | S12 | User App | Placed user app at `0x08008000` using Keil linker target settings |
| 041 | S12 | User App | Placed user app at `0x08008000` by editing `.ld` linker script (STM32CubeIDE method) |
| 042 | S12 | Both | VTOR theory: why interrupt table must be relocated when two apps share flash |
| 043 | S12 | Both | Implemented `bootloader_jump_to_user_app()` + set `VECT_TAB_OFFSET` in user app |

---

## Course Board vs. Your Board Differences

| Aspect | Course (F446RE / Keil / TeraTerm) | This Project (F401RE / CubeIDE / PuTTY) |
|--------|-----------------------------------|-----------------------------------------|
| MCU | STM32F446RE | STM32F401RE |
| Flash | 512KB, 8 sectors | 512KB, 6 sectors |
| SRAM | 128KB | 96KB |
| Debug UART | USART3 (PC10/PC11) | USART6 (PC6/PC7) |
| Code placement | Keil "Options for Target" dialog | Edit `.ld` linker script directly |
| Flash viewer | ST-LINK Utility | STM32CubeProgrammer |
| Terminal | TeraTerm | PuTTY |
| User app sector | Sector 2 (`0x08008000`) | Sector 2 (`0x08008000`) — same |
| VECT_TAB_OFFSET | `0x8000` | `0x8000` — same |
| FLASH_SECTOR2_BASE_ADDRESS | `0x08008000` | `0x08008000` — same |

All addresses, jump logic, and VTOR settings are identical between boards. Only the UART peripheral and IDE workflow differ.

---

## Key Files

| File | Purpose |
|------|---------|
| `Badweh_Bootloader_Milestone123/Core/Src/main.c` | `bootloader_jump_to_user_app()`, boot decision, `printmsg()` |
| `Badweh_Bootloader_Milestone123/Core/Inc/main.h` | `FLASH_SECTOR2_BASE_ADDRESS`, `BL_VERSION`, `BL_DEBUG_MSG_EN` |
| `Badweh_UserApplication/Core/Src/main.c` | "Hello From User Application" print |
| `Badweh_UserApplication/Core/Src/system_stm32f4xx.c` | `VECT_TAB_OFFSET = 0x8000`, `SCB->VTOR` init |
| `Badweh_UserApplication/Core/Src/stm32f4xx_it.c` | `EXTI15_10_IRQHandler`, LED toggle callback |
| `Badweh_UserApplication/STM32F401RETX_FLASH.ld` | `FLASH ORIGIN = 0x08008000` |
