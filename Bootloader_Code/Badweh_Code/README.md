# STM32F401RE Custom Bootloader

A custom UART-based bootloader implementation for the STM32F401RE Nucleo board, enabling In-Application Programming (IAP) with dual UART channels and CRC verification.

This project is a port of the FastBit Embedded Brain Academy bootloader course, originally designed for STM32F446RE, adapted for the STM32F401RE microcontroller.

---

## Features

- **In-Application Programming (IAP)**: Upload firmware via UART without external programmer
- **Dual UART Channels**: Separate command (USART2) and debug (USART6) interfaces
- **CRC Verification**: Hardware CRC engine for packet integrity
- **Flash Management**: Sector-based erase and write operations
- **Bootloader Commands**: 8 supported commands (version, chip ID, memory operations, etc.)
- **Boot Decision Logic**: User button selects bootloader or application mode

---

## Hardware Requirements

| Component | Description |
|---|---|
| **STM32 Nucleo-F401RE** | Target MCU board (Cortex-M4, 84MHz, 512KB Flash, 96KB SRAM) |
| **USB Mini-B Cable** | ST-Link debugger + Virtual COM Port (USART2) |
| **USB-to-UART Converter** | FTDI FT232, CP2102, or similar for debug channel (USART6) |
| **Jumper Wires** | 3x Dupont wires for USART6 connection (TX, RX, GND) |

### Pin Configuration

| Peripheral | Pin | Function | Hardware |
|---|---|---|---|
| **USART2** | PA2 | TX | ST-Link Virtual COM Port |
| **USART2** | PA3 | RX | ST-Link Virtual COM Port |
| **USART6** | PC6 | TX | External USB-to-UART converter |
| **USART6** | PC7 | RX | External USB-to-UART converter |
| **GPIO** | PC13 | Input (User Button B1) | Nucleo onboard button |
| **GPIO** | PA5 | Output (LED LD2) | Nucleo onboard green LED |

### Wiring - USART6 Debug Channel

Connect the external USB-to-UART converter as follows:

| Nucleo Pin | Converter Pin | Notes |
|---|---|---|
| **PC6** (USART6_TX) | **RX** | Transmit from Nucleo → Receive on converter |
| **PC7** (USART6_RX) | **TX** | Receive on Nucleo ← Transmit from converter |
| **GND** | **GND** | Common ground required |

**IMPORTANT:** PC6 is accessible on CN10 pin 4 or CN5 (D9). PC7 is accessible on CN10 pin 2.

---

## Software Requirements

- **STM32CubeIDE 1.17.0** (replaces STM32CubeMX + Keil MDK from original course)
- **PuTTY** (or similar serial terminal for monitoring UART)
- **Python 3 + pySerial** (`pip install pyserial`) for host programmer application
- **ST-Link Drivers** (Windows: `dpinst_amd64.exe`)
- **STM32CubeProgrammer** (optional, for flash inspection)

---

## Project Structure

This repository contains two separate STM32CubeIDE projects:

```
FastBit9_Bootloader/
├── README.md                              # This file
├── Bootloader_Claude_Summary.md           # Original course summary
├── Chat_Port_to_STM32F401RE.md           # Porting notes and checklist
├── FastBit_Badweh_Bootloader/            # Bootloader project (STM32CubeIDE)
│   ├── Core/
│   │   ├── Src/
│   │   │   ├── main.c                    # Bootloader main logic
│   │   │   ├── stm32f4xx_hal_msp.c       # Hardware initialization
│   │   │   └── ...
│   │   └── Inc/
│   │       ├── main.h                    # Bootloader headers & command codes
│   │       └── ...
│   ├── Drivers/                           # STM32 HAL drivers
│   ├── FastBit_Badweh_Bootloader.ioc     # CubeMX configuration
│   └── STM32F401RETX_FLASH.ld            # Linker script (Flash origin: 0x08000000)
│
└── FastBit_Badweh_UserApplication/       # User application project (to be created)
    ├── Core/
    │   └── Src/
    │       └── system_stm32f4xx.c        # VTOR offset configuration
    └── STM32F401RETX_FLASH.ld            # Linker script (Flash origin: 0x08008000)
```

### Memory Layout

The STM32F401RE flash (512KB) is partitioned as follows:

```
0x0800_0000 ┌──────────────────┐
            │ Sector 0 (16KB)  │  Bootloader
            │ Sector 1 (16KB)  │  (32KB total)
0x0800_8000 ├──────────────────┤
            │ Sector 2 (16KB)  │
            │ Sector 3 (16KB)  │  User Application
            │ Sector 4 (64KB)  │  (480KB total)
            │ Sector 5 (128KB) │
            │ Sector 6 (128KB) │
            │ Sector 7 (128KB) │
0x0808_0000 └──────────────────┘
```

- **Bootloader**: Resides in sectors 0-1 (`0x08000000` - `0x08007FFF`)
- **User Application**: Starts at sector 2 (`0x08008000`), requires VTOR offset of `0x8000`

---

## Key Differences from Original Course (F446RE → F401RE Port)

| Parameter | STM32F446RE (Course) | STM32F401RE (This Port) |
|---|---|---|
| **Chip ID** | `0x421` | `0x433` |
| **Max Clock** | 180 MHz | 84 MHz |
| **SRAM** | 128 KB (112+16 KB) | 96 KB (no SRAM2) |
| **Debug UART** | USART3 (PC10/PC11) | USART6 (PC6/PC7) |
| **Flash Sectors** | 0-7 (same layout) | 0-7 (same layout) |

### Critical Code Changes

1. **UART Handle**: Use `&huart6` instead of `&huart3` for debug output
2. **SRAM Address Validation**: Update to 96KB limit (no SRAM2 region)
3. **Chip ID Expectation**: Host app should validate `0x433` instead of `0x421`
4. **CubeMX Peripheral**: Enable USART6 on PC6/PC7 (USART3 unavailable on F401RE)

---

## Build & Flash Instructions

### 1. Build Bootloader Project

1. Open STM32CubeIDE
2. File → Open Projects from File System → Select `FastBit_Badweh_Bootloader/`
3. Project → Build All (Ctrl+B)
4. Verify binary output: Project → Properties → C/C++ Build → Settings → MCU Post build outputs → Check **"Convert to binary file"**
5. Rebuild if checkbox was just enabled
6. Verify `Debug/FastBit_Badweh_Bootloader.bin` exists

### 2. Flash Bootloader

1. Connect Nucleo via USB (ST-Link port)
2. **Ensure BOOT0 is LOW** (no jumper wire to 3.3V)
3. Run → Debug As → STM32 C/C++ Application
4. CubeIDE automatically flashes to `0x08000000`
5. Press Reset button on Nucleo

### 3. Build User Application Project

1. Create new STM32 project for Nucleo-F401RE (if not already created)
2. Configure peripherals (GPIO, USART2 optional)
3. **CRITICAL STEP**: Modify linker script (`STM32F401RETX_FLASH.ld`):
   ```
   FLASH (rx) : ORIGIN = 0x08008000, LENGTH = 480K
   ```
4. **CRITICAL STEP**: Set VTOR offset in `Core/Src/system_stm32f4xx.c`:
   ```c
   #define VECT_TAB_OFFSET  0x00008000U
   ```
5. Enable binary output (same as bootloader)
6. Build project

### 4. Flash User Application

**Method A: Direct Flash via CubeIDE (First Time)**
- Run → Debug → CubeIDE flashes to `0x08008000` (does not erase bootloader)

**Method B: Upload via Bootloader (IAP)**
- Use Python host app: `python STM32_Programmer_V1.py`
- Select command 8 (BL_MEM_WRITE)
- Provide `.bin` file and base address `0x08008000`

---

## Bootloader Commands

The bootloader supports 8 commands sent via USART2:

| Command Code | Name | Description |
|---|---|---|
| `0x51` | BL_GET_VER | Get bootloader version |
| `0x52` | BL_GET_HELP | Get list of supported commands |
| `0x53` | BL_GET_CID | Get chip ID (should return `0x433`) |
| `0x54` | BL_GET_RDP_STATUS | Get flash read protection status |
| `0x55` | BL_GO_TO_ADDR | Jump to specified address (e.g., user app) |
| `0x56` | BL_FLASH_ERASE | Erase flash sectors |
| `0x57` | BL_MEM_WRITE | Write data to flash/RAM |
| `0x58` | BL_READ_SECTOR_P_STATUS | Read sector write protection status |

### Using the Python Host Application

1. Navigate to `HOST/python/` (from course materials)
2. Edit `STM32_Programmer_V1.py` to set correct COM port
3. Hold **User Button (B1)** and press **Reset** to enter bootloader mode
4. Run: `python STM32_Programmer_V1.py`
5. Select desired command and follow prompts

---

## Testing & Verification

### Hardware Verification

- [ ] ST-Link detected in Device Manager (Windows) or `lsusb` (Linux)
- [ ] Virtual COM Port enumerated (e.g., COM4)
- [ ] External USB-to-UART converter detected (e.g., COM6)
- [ ] USART6 wiring: PC6→RX, PC7→TX, GND→GND

### USART Communication Tests

**USART2 (Command Channel):**
1. Open PuTTY: Serial, COM port = ST-Link Virtual COM Port, 115200 baud
2. Flash bootloader test code (current `main.c`)
3. Should see: "Bootloader Test on USART2" repeating every 1 second ✅

**USART6 (Debug Channel):**
1. Open second PuTTY instance: Serial, COM port = USB-to-UART converter, 115200 baud
2. Flash bootloader test code
3. Should see: "Debug Test on USART6" repeating every 1 second ✅

### Boot Decision Logic Test

1. Press **Reset** (button not held) → Should jump to user application
2. Hold **User Button (B1)** + press **Reset** → Should stay in bootloader mode

### Bootloader Command Tests

- [ ] BL_GET_VER → Returns version byte (e.g., `0x10`)
- [ ] BL_GET_CID → Returns `0x0433` (F401RE chip ID)
- [ ] BL_FLASH_ERASE → Sector erased successfully
- [ ] BL_MEM_WRITE → Firmware uploaded via UART
- [ ] BL_GO_TO_ADDR → User application starts

---

## Current Project Status

### ✅ Phase 1: Environment Setup (Complete)
- STM32CubeIDE 1.17.0 installed
- PuTTY configured for serial communication
- ST-Link drivers installed
- Hardware connections verified

### ✅ Phase 2: Bootloader Project Creation (Complete)
- STM32F401RE project created in CubeIDE
- USART2 (PA2/PA3) configured and tested ✅
- USART6 (PC6/PC7) configured and tested ✅
- CRC engine enabled
- GPIO configured (button PC13, LED PA5)
- Clock configured to 84 MHz
- Binary output generation enabled
- Test code verified on both UART channels

### ⏳ Phase 3: User Application Project (Next)
- Create second STM32CubeIDE project
- Configure GPIO with EXTI interrupt
- Relocate linker script to `0x08008000`
- Set VTOR offset to `0x8000`
- Test user app independently

### ⏳ Phase 4: Bootloader Code Implementation
- Add bootloader command handlers
- Implement boot decision logic
- Add flash erase/write operations
- Implement address validation (96KB SRAM)
- Add CRC verification

### ⏳ Phase 5: Testing & Verification
- Test all 8 bootloader commands
- Verify IAP workflow (erase → write → jump)
- Test edge cases and error handling

---

## Troubleshooting

### No Output on USART2 (PuTTY Blank)

**Symptoms:** PuTTY connected but no text appears

**Possible Causes:**
1. **BOOT0 jumper wire connected** → MCU running ST's ROM bootloader, not your code
   - **Fix:** Remove jumper from BOOT0, press Reset
2. **Wrong COM port** → Connected to USART6 port instead of USART2
   - **Fix:** Use ST-Link Virtual COM Port (typically lower COM number)
3. **Wrong baud rate** → Should be 115200
4. **Connection type** → Must be "Serial", not SSH/Telnet

### LED Not Blinking / Code Not Running

**Symptoms:** Code flashes successfully but LED doesn't blink

**Possible Causes:**
1. **BOOT0 shorted to 3.3V** → See above
2. **Code stuck in `Error_Handler()`** → Init function failed (likely clock config)
   - **Fix:** Add LED blink in `Error_Handler()` to confirm, then debug clock settings
3. **Not resuming after debug** → Program paused at breakpoint
   - **Fix:** Press Resume (F8) or close debug session and press Reset

### User Application Crashes / Hard Fault

**Symptoms:** Bootloader jumps to user app, but it immediately crashes

**Possible Causes:**
1. **VTOR offset not set** → Interrupt vectors not relocated
   - **Fix:** Set `VECT_TAB_OFFSET = 0x00008000U` in `system_stm32f4xx.c`
2. **Wrong linker origin** → User app compiled for `0x08000000` instead of `0x08008000`
   - **Fix:** Edit `.ld` file, clean and rebuild
3. **Stack pointer corrupted** → Reset handler address invalid
   - **Fix:** Verify first 8 bytes of user app binary

### Bootloader Commands Not Working

**Symptoms:** Python host app connects but no ACK received

**Possible Causes:**
1. **Wrong COM port in host app** → Should use ST-Link Virtual COM (USART2)
2. **Not in bootloader mode** → User button not held during reset
3. **CRC mismatch** → Packet corrupted or CRC calculation error
4. **Baud rate mismatch** → Must be 115200 on both sides

### Wrong Chip ID Returned

**Symptoms:** BL_GET_CID returns `0x0421` instead of `0x0433`

**Possible Causes:**
1. **Using F446RE instead of F401RE** → Wrong board or wrong project target
   - **Fix:** Verify board label, regenerate project for F401RE

---

## References

- **Course**: FastBit Embedded Brain Academy - STM32 Bootloader Development
- **Porting Notes**: [`Chat_Port_to_STM32F401RE.md`](Chat_Port_to_STM32F401RE.md)
- **Course Summary**: [`Bootloader_Claude_Summary.md`](Bootloader_Claude_Summary.md)
- **STM32F401RE Datasheet**: [STMicroelectronics](https://www.st.com/resource/en/datasheet/stm32f401re.pdf)
- **STM32F401RE Reference Manual (RM0368)**: [STMicroelectronics](https://www.st.com/resource/en/reference_manual/dm00096844-stm32f401xb-c-and-stm32f401xd-e-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)

---

## License & Credits

**Course Credit**: FastBit Embedded Brain Academy
**Port Author**: Ported to STM32F401RE - 2026
**Original Target**: STM32F446RE

This project is for educational purposes based on the FastBit Embedded Brain Academy bootloader course.

---

## Notes

- Always verify BOOT0 is LOW (not connected to 3.3V) for normal operation
- The bootloader and user application are **separate projects** with different linker configurations
- When modifying `.ioc` files in CubeMX, always regenerate code and rebuild
- After linker script changes, always perform a **Clean → Build All**
- Keep both PuTTY terminals open (USART2 + USART6) for full visibility during development
