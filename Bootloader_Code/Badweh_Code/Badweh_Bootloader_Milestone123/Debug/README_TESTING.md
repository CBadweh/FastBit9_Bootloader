# STM32F401RE Bootloader Testing Guide

## Files in this Directory

- `FastBit_Badweh_Bootloader.bin` (19 KB) - Custom bootloader binary
- `FastBit_Badweh_UserApplication.bin` (8.5 KB) - User application binary
- `STM32_Programmer_F401RE.py` - Python host script for bootloader commands

## Hardware Setup

### UART Connections
- **USART2 (Command UART):** Uses ST-Link Virtual COM Port (COM4)
  - PA2 = TX (Nucleo to PC)
  - PA3 = RX (PC to Nucleo)
  - **No external wiring needed**

- **USART6 (Debug UART):** Requires external USB-to-UART converter
  - PC6 = TX (Nucleo to converter RX)
  - PC7 = RX (Converter TX to Nucleo)
  - GND = Common ground

### Button & LED
- **Button:** PC13 (active LOW) - User button on Nucleo
- **LED:** PA5 - Green LED on Nucleo

## Flash Memory Map (STM32F401RE)

| Region | Sectors | Address Range | Size | Contents |
|--------|---------|---------------|------|----------|
| Bootloader | 0-1 | 0x08000000 - 0x08007FFF | 32 KB | Custom bootloader code |
| User App | 2-7 | 0x08008000 - 0x0807FFFF | 480 KB | User application code |

**Sector Layout:**
- Sector 0: 16 KB (0x08000000)
- Sector 1: 16 KB (0x08004000)
- Sector 2: 16 KB (0x08008000) ← **User app starts here**
- Sector 3: 16 KB (0x0800C000)
- Sector 4: 64 KB (0x08010000)
- Sector 5: 128 KB (0x08020000)
- Sector 6: 128 KB (0x08040000)
- Sector 7: 128 KB (0x08060000)

## Phase 7: Hardware Integration Testing (Next Steps)

### Step 1: Flash Bootloader
```bash
# Use STM32CubeIDE or ST-Link Utility
# Flash: FastBit_Badweh_Bootloader.elf to address 0x08000000
```

### Step 2: Flash User Application
```bash
# Use STM32CubeIDE or ST-Link Utility
# Flash: FastBit_Badweh_UserApplication.elf to address 0x08008000
```

### Step 3: Test Boot Decision
1. **Normal boot (no button):**
   - Press reset (no button held)
   - User app should run
   - TeraTerm on COM4 shows: "Hello From User Application"
   - Button press toggles LED

2. **Bootloader mode (button held):**
   - Hold user button
   - Press reset
   - Release button after reset
   - Bootloader should be active (no user app output)

### Step 4: Test Python Host Communication

#### Prerequisites
```bash
# Verify Python and pyserial installed
python --version   # Should show Python 3.13.1
pip list | grep serial   # Should show pyserial 3.5
```

#### Run Host Script
```bash
cd "C:\Users\Sheen\Desktop\Embedded_System\FastBit9_Bootloader\FastBit_Badweh_Bootloader\Debug"
python STM32_Programmer_F401RE.py
```

#### Test Commands (in order)
1. **BL_GET_VER (1)** - Should return `0x10`
2. **BL_GET_CID (3)** - Should return `0x0433` with ✓ confirmation
3. **BL_GET_HELP (2)** - Should list 10 command codes
4. **BL_GET_RDP_STATUS (4)** - Should return `0xAA` (Level 0, no protection)

If all 4 pass → **Protocol verification complete!**

## Phase 8: Full IAP Workflow (Final Goal)

### Complete Field Upgrade Procedure

1. **Enter bootloader mode:**
   - Hold button + reset → bootloader active

2. **Erase user app flash:**
   ```
   Command: 7 (BL_FLASH_ERASE)
   Sector: 0x02 (sector 2)
   Count: 6 (erase sectors 2-7)
   ```

3. **Upload new binary:**
   ```
   Command: 8 (BL_MEM_WRITE)
   Address: 0x08008000
   File: FastBit_Badweh_UserApplication.bin (8.5 KB)
   → Script automatically chunks into 128-byte packets
   ```

4. **Jump to new app:**
   ```
   Command: 5 (BL_GO_TO_ADDR)
   Address: 0x08008000
   → Bootloader reads reset handler and jumps
   ```

5. **Verify:**
   - User app should start running
   - TeraTerm shows "Hello From User Application"
   - LED toggle works

## Troubleshooting

### Bootloader doesn't respond
- Check COM port (should be COM4 for USART2)
- Verify bootloader mode (button held during reset)
- Check debug UART (USART6) for error messages

### CRC FAIL errors
- Verify Python CRC matches bootloader hardware CRC
- Check serial port baud rate (115200)
- Try different cable

### Address Status = 0x01 (invalid)
- Verify address is in valid range (Flash, SRAM1, Backup SRAM)
- STM32F401RE does NOT have SRAM2 (only 96KB SRAM1)

### User app doesn't run after jump
- Check VTOR relocation in user app `system_stm32f4xx.c`
- Verify linker script: `FLASH ORIGIN = 0x08008000`
- Use ST-Link Utility to verify binary at correct address

## Key Differences: F401RE vs F446RE (Course Original)

| Feature | F446RE (Course) | F401RE (Ported) |
|---------|-----------------|-----------------|
| Chip ID | 0x0421 | **0x0433** |
| Debug UART | USART3 (PC10/PC11) | **USART6 (PC6/PC7)** |
| SRAM1 Size | 112 KB | **96 KB** |
| SRAM2 | 16 KB | **Does NOT exist** |
| Flash Sectors | 8 (same layout) | 8 (same layout) |

## Success Criteria

✅ Phase 5: User app binary generated (8.5 KB)
✅ Phase 6: Python host script adapted for F401RE
⏳ Phase 7: Hardware boot decision + command protocol verified
⏳ Phase 8: Full IAP workflow (erase → upload → jump) tested

---
**Next Step:** Flash both binaries to hardware and test boot decision!
