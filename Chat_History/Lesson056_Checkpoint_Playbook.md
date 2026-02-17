# Lesson 056 System Verification Playbook
## STM32F401RE Bootloader - BL_GET_VER Command Debug Session

**Last Updated:** 2026-02-07
**Course:** FastBit Embedded Brain Academy - STM32 Bootloader Development
**Checkpoint:** Lesson 056 - First complete system verification (Hardware + Firmware + Host)

---

## Section 1: Overview & Goals

### What This Checkpoint Verifies

This is the **FIRST** time three independent systems communicate:

```
Python Script (PC) → UART → STM32 Bootloader → CRC Engine → ACK → UART → Python
```

**Systems under test:**
1. Hardware: STM32F401RE Nucleo board
2. Firmware: Custom bootloader running on MCU
3. Host software: Python script on PC

### Success Criteria

**Python Terminal shows:**
```
Command == > BL_GET_VER
0x05 0x51 0xe7 0xe9 0xab 0x7c

CRC : SUCCESS Len : 1
Bootloader Ver. :  0x10
```

**IDE Expressions Window shows (at breakpoint 3):**
```
bl_rx_buffer[0] = 0x05
bl_rx_buffer[1] = 0x51
bl_rx_buffer[2..5] = CRC bytes
```

### Failure Criteria

- Python shows: `"Timeout : Bootloader not responding"`
- IDE never hits breakpoint at line 537
- `bl_rx_buffer[]` contains wrong values or all zeros
- CRC verification fails (NACK sent instead of ACK)

### Why This Is Critical

**If this works:**
- ✅ Protocol implementation is correct
- ✅ UART communication verified
- ✅ Hardware CRC peripheral working
- ✅ Boot decision logic functional
- ✅ F401RE port successful

**If this fails:** Something fundamental is broken - can't proceed to advanced commands.

---

## Section 2: Pre-Flight Checklist

**Run through this checklist BEFORE starting the debug session. 80% of issues are caught here.**

### Hardware Checks
- [ ] Nucleo board connected via USB cable (data + power capable)
- [ ] ST-Link LED is green (not red/blinking)
- [ ] Windows Device Manager shows "STMicroelectronics Virtual COM Port (COM4)"
- [ ] No other programs using COM4 (close TeraTerm, PuTTY, Arduino IDE, etc.)

### Software Environment Checks
```bash
# Run these commands and verify versions match:
python --version          # Expected: Python 3.13.1 (or 3.x)
pip show pyserial         # Expected: Version 3.5 (or later)
```
- [ ] Python version confirmed
- [ ] pyserial library installed
- [ ] STM32CubeIDE version 1.17.0 or later

### Firmware Verification

**Use ST-Link Utility or CubeIDE Memory Browser:**

**Check bootloader at 0x08000000:**
- [ ] Memory shows NON-0xFF data (actual code, not blank flash)
- [ ] First word (~0x20018000) = MSP value (stack pointer)
- [ ] Second word (~0x080001xx) = Reset handler address

**Check user app at 0x08008000:**
- [ ] Memory shows NON-0xFF data
- [ ] First word (~0x20018000) = MSP value
- [ ] Second word (~0x080081xx) = Reset handler address

**If all 0xFF at either address:** Binary not flashed - reflash before continuing.

### File Location Checks
- [ ] Python script exists: `FastBit_Badweh_Bootloader\Debug\STM32_Programmer_F401RE.py`
- [ ] User app binary exists: `FastBit_Badweh_Bootloader\Debug\FastBit_Badweh_UserApplication.bin`
- [ ] Current directory when running Python: `FastBit_Badweh_Bootloader\Debug\`

### Known-Good Values Reference
- [ ] Bootloader version: `0x10` (defined in main.h:62)
- [ ] Chip ID: `0x0433` (STM32F401RE specific)
- [ ] User app base address: `0x08008000`
- [ ] Flash sector 2 starts at: `0x08008000`

---

## Section 3: Hardware Setup

### Physical Connections

```
┌─────────────────────────────────────────────────────────┐
│                    STM32F401RE Nucleo                    │
│                                                           │
│  USART2 (PA2/PA3) ◄──► ST-Link ◄──► USB ──► PC (COM4)   │
│       ↑                                       ↑           │
│  Command UART                        Python Script Uses  │
│  (Bootloader Commands)                      THIS!        │
│                                                           │
│  USART6 (PC6/PC7) ◄──► FTDI Converter ──► PC (COM6)     │
│       ↑                                       ↑           │
│  Debug UART                          Optional monitoring  │
│  (printmsg debug)                    (TeraTerm/PuTTY)    │
│                                                           │
│  PC13: Blue User Button (active LOW)                     │
│  Black: Reset Button                                     │
│  PA5:  Green LED (LD2)                                   │
└─────────────────────────────────────────────────────────┘
```

### COM Port Assignments

| Port | Hardware Path | Baud Rate | Purpose | Used By |
|------|---------------|-----------|---------|---------|
| **COM4** | ST-Link Virtual COM → USART2 (PA2/PA3) | 115200 | **Command channel** | **Python script** |
| COM6 | FTDI USB-UART → USART6 (PC6/PC7) | 115200 | Debug messages (optional) | TeraTerm (monitoring) |

**CRITICAL:** Python script MUST use COM4 (not COM6).

### Button Locations

**Blue button** (larger, on board edge): User button (PC13, active LOW)
**Black button** (smaller, near ST-Link): Reset button

**Photo reference:** `Photos/nucleo_button_labels.jpg` (if available)

---

## Section 4: Debug Session Procedure

### Critical Breakpoints for BL_GET_VER Verification

**Set these breakpoints BEFORE starting debug session:**

| # | File:Line | Function | Purpose | Expected Value | What to Verify |
|---|-----------|----------|---------|----------------|----------------|
| 1 | main.c:125 | `main()` | Boot decision | `B1_Pin == GPIO_PIN_RESET (0)` | Button pressed → BL mode |
| 2 | main.c:537 | `bootloader_uart_read_data()` | Length byte received | `bl_rx_buffer[0] = 0x05` | First UART byte from Python |
| 3 | main.c:543 | `bootloader_uart_read_data()` | Full packet received | `bl_rx_buffer[1] = 0x51`<br>`bl_rx_buffer[2..5]` = CRC | All 6 bytes received |
| 4 | main.c:510 | `bootloader_handle_getver_cmd()` | CRC extraction | `host_crc` extracted | CRC from last 4 bytes |
| 5 | main.c:514 | `bootloader_handle_getver_cmd()` | **CRC SUCCESS path** | `verify_crc()` returns `0` | ACK will be sent |
| 6 | main.c:520 | `bootloader_handle_getver_cmd()` | **CRC FAILURE path** | (should NOT hit) | NACK would be sent |
| 7 | main.c:515 | `bootloader_handle_getver_cmd()` | Version read | `bl_version = 0x10` | After `get_bootloader_version()` |

### How to Set Breakpoints in STM32CubeIDE

1. Open `FastBit_Badweh_Bootloader/Core/Src/main.c` in editor
2. Locate line number in left margin (e.g., line 537)
3. Double-click in the gray margin area (blue dot appears = breakpoint active)
4. Repeat for all 7 breakpoint lines above
5. Verify in Breakpoints panel: `Window → Show View → Breakpoints`

**All 7 breakpoints should show as enabled (checked boxes).**

### Step-by-Step Debugging Flow

#### **Step 1: Start Debug Session**

1. Right-click `FastBit_Badweh_Bootloader` project in Project Explorer
2. Select `Debug As → STM32 C/C++ Application`
3. Wait for console message: `"Download verified successfully"`
4. IDE should pause at `main()` entry

#### **Step 2: Run to Boot Decision**

1. Click **Resume** button (▶ icon) or press `F8`
2. Code executes until **Breakpoint #1** (main.c:125)
3. **Expressions Window:** Add variable `B1_Pin` (or check live value)

#### **Step 3: Enter Bootloader Mode (Physical Action)**

**CRITICAL TIMING - Follow exactly:**

```
While paused at Breakpoint #1:
  a) Press and HOLD blue user button (PC13)
  b) Click "Reset" icon (⟳) in debugger toolbar (or Ctrl+R)
  c) Keep holding blue button for 2 seconds
  d) Release blue button
  e) Click Resume (F8)
```

**Expected:** Code should stop at **Breakpoint #2** (line 537) inside `bootloader_uart_read_data()`.

**If code stops at `bootloader_jump_to_user_app()` instead:** Button not detected, repeat Step 3.

#### **Step 4: Open Python Script (Separate Terminal)**

**In Windows Command Prompt or PowerShell:**

```bash
cd C:\Users\Sheen\Desktop\Embedded_System\FastBit9_Bootloader\FastBit_Badweh_Bootloader\Debug
python STM32_Programmer_F401RE.py
```

**When prompted:**
```
Enter the Port Name of your device (Ex: COM4 for STM32F401RE Nucleo): COM4
```

**Menu appears, wait at prompt:**
```
Type the command code here :
```

**Do NOT type anything yet - return to CubeIDE first.**

#### **Step 5: Resume to UART Receive Point**

**In CubeIDE:**
1. Click **Resume** (F8)
2. Code will execute until reaching `HAL_UART_Receive()` on line 537
3. Code is now **WAITING** for UART data (blocking call)

**At this point:**
- IDE appears "frozen" (actually waiting for data)
- Python terminal is waiting for your input
- Bootloader is listening on COM4

#### **Step 6: Send Python Command**

**Switch to Python terminal window:**

1. Type: `1` (BL_GET_VER command)
2. Press Enter

**Python immediately sends packet:**
```
Command == > BL_GET_VER
0x05 0x51 0xe7 0xe9 0xab 0x7c
```

**Python then waits for reply (timeout = 2 seconds).**

**Switch back to CubeIDE immediately.**

#### **Step 7: Verify Reception at Each Breakpoint**

| Breakpoint Hit | Action | Verify in Expressions Window | Expected Value |
|----------------|--------|------------------------------|----------------|
| **BP #2** (537) | Already here | Add `bl_rx_buffer[0]` | `0x05` (5 decimal) |
| → Press Resume (F8) | | | |
| **BP #3** (543) | Stops here | `bl_rx_buffer[1]`<br>`bl_rx_buffer[2]`<br>`bl_rx_buffer[3]`<br>`bl_rx_buffer[4]`<br>`bl_rx_buffer[5]` | `0x51` (81)<br>`0xE7` (231)<br>`0xE9` (233)<br>`0xAB` (171)<br>`0x7C` (124) |
| → Press Resume (F8) | Code enters switch-case, calls handler | | |
| **BP #4** (510) | Stops here | `host_crc` | `0x7CABE9E7` (varies by packet) |
| → Press Resume (F8) | Code calls `bootloader_verify_crc()` | | |
| **BP #5** (514) | Stops here (SUCCESS path) | Return value from `verify_crc()` | `0` (VERIFY_CRC_SUCCESS) |
| → Press Resume (F8) | Code sends ACK, reads version | | |
| **BP #7** (515) | Stops here | `bl_version` | `0x10` (16 decimal) |
| → Press Resume (F8) | Code sends version to UART | | |

**After final Resume:**
- Bootloader sends ACK byte (`0xA5`) + length (`0x01`) + version (`0x10`) to COM4
- Python receives reply and displays it

#### **Step 8: Verify Python Output**

**Switch to Python terminal window:**

**Expected output:**
```
   CRC : SUCCESS Len : 1

   Bootloader Ver. :  0x10

   Press any key to continue  :
```

**✅ SUCCESS!** Full system communication verified.

### How to Add Variables to Expressions Window

1. In CubeIDE, while paused at breakpoint
2. `Window → Show View → Expressions`
3. In Expressions panel, click green `+` icon (Add new expression)
4. Type variable name: `bl_rx_buffer[0]` or `bl_version`
5. Press Enter
6. Value updates automatically at each breakpoint

**Tip:** You can also right-click variable in code → `Add Watch Expression`

---

## Section 5: Python Script Execution (Non-Debug Mode)

**For normal testing WITHOUT debugger:**

### Running the Script

```bash
# Navigate to script location
cd C:\Users\Sheen\Desktop\Embedded_System\FastBit9_Bootloader\FastBit_Badweh_Bootloader\Debug

# Run script
python STM32_Programmer_F401RE.py
```

### Entering Bootloader Mode (Without Debugger)

**Physical sequence:**
1. Press and HOLD blue user button
2. Press and release black reset button (quick tap)
3. Keep holding blue button for 2 seconds
4. Release blue button

**Verification (if FTDI connected to USART6):**
- Open TeraTerm on COM6, 115200 baud
- After reset sequence, should see: `"BL_DEBUG_MSG: Button is pressed.. Going to BL mode"`

### Script Interaction

**At port prompt:**
```
Enter: COM4
```

**At menu, type command number:**
```
Type the command code here : 1
```

**Expected full output:**
```
   Command == > BL_GET_VER
   0x05    0x51    0xe7    0xe9    0xab    0x7c

   CRC : SUCCESS Len : 1

   Bootloader Ver. :  0x10

   Press any key to continue  :
```

Press any key → Menu appears again → Can test more commands

### Other Commands to Test

After `BL_GET_VER` works, verify these:

| Command | Code | Expected Output |
|---------|------|-----------------|
| BL_GET_CID | 3 | `Chip Id. : 0x433`<br>`✓ Chip ID matches STM32F401RE (0x0433)` |
| BL_GET_HELP | 2 | `Supported Commands : 0x51 0x52 0x53 0x54 0x55 0x56 0x57 0x58 0x5a 0x5c` |
| BL_GET_RDP_STATUS | 4 | `RDP Status : 0xaa` (Level 0, no protection) |

---

## Section 6: State Verification Worksheet

**Fill this out during your debug session to track values:**

| Step | Breakpoint | Variable | Expected | Your Value | ✓/✗ | Notes |
|------|------------|----------|----------|------------|-----|-------|
| 1 | main.c:125 | `B1_Pin` | `0` (GPIO_PIN_RESET) | _____ | ___ | Button pressed = 0 |
| 2 | main.c:537 | `bl_rx_buffer[0]` | `0x05` | _____ | ___ | Length byte |
| 3 | main.c:543 | `bl_rx_buffer[1]` | `0x51` | _____ | ___ | Command code |
| 3 | main.c:543 | `bl_rx_buffer[2]` | `0xE7` | _____ | ___ | CRC byte 1 |
| 3 | main.c:543 | `bl_rx_buffer[3]` | `0xE9` | _____ | ___ | CRC byte 2 |
| 3 | main.c:543 | `bl_rx_buffer[4]` | `0xAB` | _____ | ___ | CRC byte 3 |
| 3 | main.c:543 | `bl_rx_buffer[5]` | `0x7C` | _____ | ___ | CRC byte 4 |
| 4 | main.c:510 | `host_crc` | `0x7CABE9E7` | _____ | ___ | CRC extracted |
| 5 | main.c:514 | CRC verify return | `0` (SUCCESS) | _____ | ___ | 0=pass, 1=fail |
| 7 | main.c:515 | `bl_version` | `0x10` | _____ | ___ | Bootloader version |

**Python Terminal Output:**

| Field | Expected | Your Output | ✓/✗ |
|-------|----------|-------------|-----|
| Bytes sent | `0x05 0x51 0xe7 0xe9 0xab 0x7c` | ________________ | ___ |
| CRC result | `CRC : SUCCESS Len : 1` | ________________ | ___ |
| Version | `Bootloader Ver. : 0x10` | ________________ | ___ |

---

## Section 7: Failure Diagnosis Flowchart

### Start: Symptom Identification

```
┌─────────────────────────────────────────────────────────┐
│ SYMPTOM: Python shows "Timeout : Bootloader not        │
│          responding"                                     │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│ Q1: Are you in debug mode with breakpoints set?        │
├─────────────────────────────────────────────────────────┤
│ YES → This is EXPECTED. Debugger pauses execution.     │
│       Hit Resume (F8) to continue. Python will receive  │
│       reply after final Resume.                         │
│                                                          │
│ NO  → Continue to Q2                                    │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│ Q2: Did IDE hit breakpoint at main.c:537?              │
├─────────────────────────────────────────────────────────┤
│ YES → Bootloader is running, COM issue. Go to Q5.      │
│                                                          │
│ NO  → Bootloader not in command mode. Go to Q3.        │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│ Q3: Did you hold user button during reset?             │
├─────────────────────────────────────────────────────────┤
│ UNSURE → FIX: Repeat button sequence carefully:        │
│          1. Press and HOLD blue button                  │
│          2. Tap black reset button (or click Reset ⟳)  │
│          3. Keep holding blue for 2 full seconds        │
│          4. Release blue button                         │
│          Then retry Python command.                     │
│                                                          │
│ YES, HELD IT → Go to Q4                                │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│ Q4: Is bootloader actually flashed at 0x08000000?      │
├─────────────────────────────────────────────────────────┤
│ VERIFY:                                                  │
│   - Open ST-Link Utility or CubeIDE Memory Browser      │
│   - View address 0x08000000                             │
│   - Should see NON-0xFF values (actual code)            │
│                                                          │
│ ALL 0xFF → Bootloader not flashed.                     │
│             FIX: Flash FastBit_Badweh_Bootloader.elf    │
│                  to 0x08000000 via ST-Link              │
│                                                          │
│ NON-0xFF → Bootloader is there. Check debug UART.      │
│             FIX: Connect FTDI to USART6 (PC6/PC7),     │
│                  open TeraTerm on COM6, do button       │
│                  sequence. Should see "Going to BL      │
│                  mode" message. If not, button timing   │
│                  is wrong.                              │
└─────────────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────────────┐
│ Q5: Is COM4 the correct port?                          │
├─────────────────────────────────────────────────────────┤
│ VERIFY:                                                  │
│   - Open Device Manager → Ports (COM & LPT)            │
│   - Find "STMicroelectronics Virtual COM Port"          │
│   - Note the COM number (e.g., COM4)                    │
│                                                          │
│ DIFFERENT NUMBER → Update Python script input to use   │
│                    correct COM port                      │
│                                                          │
│ PORT MISSING → Reconnect USB cable or reinstall        │
│                ST-Link drivers                          │
└─────────────────────────────────────────────────────────┘
```

### Other Common Failures

#### Symptom: CRC FAIL (Python shows "CRC: FAIL")

**Cause:** CRC mismatch between Python and bootloader

**Debug:**
1. In Python script, add debug print after line 308:
   ```python
   print("DEBUG: Python CRC =", hex(crc32))
   ```
2. In bootloader main.c:874, add:
   ```c
   printmsg("DEBUG: Host=0x%lX, Calc=0x%lX\r\n", crc_host, uwCRCValue);
   ```
3. Compare values - should match exactly

**Fix:** Verify both use polynomial `0x04C11DB7`, same byte order

#### Symptom: `bl_rx_buffer` contains wrong values

**Cause:** Wrong COM port or serial settings mismatch

**Fix:**
1. Verify Device Manager shows COM4 for ST-Link Virtual COM Port
2. Check USART2 init in main.c:362-388 (should be 115200, 8N1)
3. Close ALL other programs using COM4 (TeraTerm, PuTTY, Arduino IDE)
4. Try different USB port on PC
5. Try different USB cable (must support data, not just power)

#### Symptom: BP #6 (CRC FAIL path) hit instead of BP #5

**Cause:** CRC verification failed

**Debug:** Check Expressions window at BP #4:
- `host_crc` = CRC from packet (last 4 bytes)
- Step into `bootloader_verify_crc()`, watch `uwCRCValue` calculation

**Fix:** Likely serial data corruption or Python CRC calculation wrong

#### Symptom: Python hangs forever (no timeout message)

**Cause:** Serial port not responding at all

**Fix:**
1. Close Python script (Ctrl+C)
2. Unplug/replug Nucleo USB
3. Verify COM port in Device Manager
4. Restart Python script

---

## Section 8: Quick Reference Card

**Print this page and tape to monitor**

```
╔═══════════════════════════════════════════════════════════════╗
║       LESSON 056 QUICK REFERENCE - STM32F401RE BOOTLOADER      ║
╚═══════════════════════════════════════════════════════════════╝

┌───────────────────────────────────────────────────────────────┐
│ ENTERING BOOTLOADER MODE (Physical Actions)                   │
└───────────────────────────────────────────────────────────────┘
  1. Hold blue button (PC13)
  2. Tap black button (RESET)
  3. Keep holding blue 2 seconds
  4. Release blue button
  → Bootloader active (waiting for commands on COM4)

┌───────────────────────────────────────────────────────────────┐
│ RUNNING PYTHON SCRIPT                                          │
└───────────────────────────────────────────────────────────────┘
  cd FastBit_Badweh_Bootloader\Debug
  python STM32_Programmer_F401RE.py
  Enter: COM4
  Command: 1 (BL_GET_VER)
  → Expected: "Bootloader Ver. : 0x10"

┌───────────────────────────────────────────────────────────────┐
│ CRITICAL BREAKPOINT LOCATIONS                                  │
└───────────────────────────────────────────────────────────────┘
  Line 125:  Boot decision (button check)
  Line 537:  Length byte received (bl_rx_buffer[0])
  Line 543:  Full packet received (bl_rx_buffer[1..5])
  Line 510:  CRC extracted (host_crc)
  Line 514:  CRC SUCCESS path (ACK sent)
  Line 515:  Version read (bl_version = 0x10)

┌───────────────────────────────────────────────────────────────┐
│ EXPECTED VALUES AT BREAKPOINTS                                 │
└───────────────────────────────────────────────────────────────┘
  bl_rx_buffer[0] = 0x05  (length)
  bl_rx_buffer[1] = 0x51  (BL_GET_VER command)
  bl_rx_buffer[2..5]      (CRC32 bytes)
  bl_version      = 0x10  (bootloader version)
  B1_Pin          = 0     (button pressed = GPIO_PIN_RESET)

┌───────────────────────────────────────────────────────────────┐
│ SUCCESS LOOKS LIKE                                             │
└───────────────────────────────────────────────────────────────┘
  Python Terminal:
    0x05 0x51 0xe7 0xe9 0xab 0x7c  ← Bytes sent
    CRC : SUCCESS Len : 1          ← ACK received
    Bootloader Ver. : 0x10         ← Version displayed

  IDE Expressions:
    bl_rx_buffer[0] = 0x05 (5)
    bl_rx_buffer[1] = 0x51 (81)
    bl_version      = 0x10 (16)

┌───────────────────────────────────────────────────────────────┐
│ FAILURE QUICK FIXES                                            │
└───────────────────────────────────────────────────────────────┘
  "Timeout" during debug     → Hit Resume (F8) to continue
  "Timeout" normal mode      → Check button timing, retry
  BP never hit               → Not in BL mode, repeat button seq
  Wrong bl_rx_buffer values  → Check COM port (Device Manager)
  CRC FAIL                   → Check CRC calculation in debug

┌───────────────────────────────────────────────────────────────┐
│ COM PORT ASSIGNMENT                                            │
└───────────────────────────────────────────────────────────────┘
  COM4: ST-Link Virtual COM → USART2 → Command channel (Python)
  COM6: FTDI USB-UART → USART6 → Debug messages (optional)

┌───────────────────────────────────────────────────────────────┐
│ KEY VALUES                                                     │
└───────────────────────────────────────────────────────────────┘
  Bootloader version:    0x10
  Chip ID (F401RE):      0x0433
  User app base:         0x08008000
  Bootloader base:       0x08000000
  Command: BL_GET_VER:   0x51
  ACK byte:              0xA5
  NACK byte:             0x7F
```

---

## Section 9: Hardware Setup Diagram

### Complete System Wiring

```
                    ┌─────────────────────────┐
                    │    Windows PC           │
                    │                         │
                    │  Python Script Running  │
                    │  COM4: Command UART     │
                    │  COM6: Debug UART (opt) │
                    └──────────┬──────────────┘
                               │
                ┌──────────────┴──────────────┐
                │                             │
        USB Cable (Data+Power)        USB-UART (Optional)
                │                             │
                ↓                             ↓
    ┌───────────────────────┐    ┌───────────────────┐
    │  ST-Link Debugger     │    │  FTDI Converter   │
    │  (On-board Nucleo)    │    │  (External)       │
    └───────────┬───────────┘    └─────┬─────────────┘
                │                      │
                │                      │ PC6 (TX)
                ↓                      │ PC7 (RX)
    ┌─────────────────────────────────┼──────────────┐
    │       STM32F401RE Nucleo Board  │              │
    │                                 ↓              │
    │   ┌─────────────────────────────────────┐     │
    │   │  STM32F401RE MCU                    │     │
    │   │                                     │     │
    │   │  PA2 (TX) ──┐                      │     │
    │   │  PA3 (RX) ──┼─→ USART2 (Command)   │     │
    │   │             │                       │     │
    │   │  PC6 (TX) ──┐                      │     │
    │   │  PC7 (RX) ──┼─→ USART6 (Debug)     │     │
    │   │             │                       │     │
    │   │  PC13 ──────┼─→ User Button (Blue) │     │
    │   │  PA5  ──────┼─→ LED LD2 (Green)    │     │
    │   │             │                       │     │
    │   │  Bootloader @ 0x08000000 (32KB)    │     │
    │   │  User App   @ 0x08008000 (480KB)   │     │
    │   └─────────────────────────────────────┘     │
    └───────────────────────────────────────────────┘
```

### Pin Assignments Table

| Pin | Function | Direction | Connected To | Purpose |
|-----|----------|-----------|--------------|---------|
| PA2 | USART2_TX | Output | ST-Link → PC COM4 | Send replies to Python |
| PA3 | USART2_RX | Input | ST-Link ← PC COM4 | Receive commands from Python |
| PC6 | USART6_TX | Output | FTDI → PC COM6 | Send debug messages |
| PC7 | USART6_RX | Input | FTDI ← PC COM6 | (Unused, but configured) |
| PC13 | GPIO Input | Input | Blue user button | Boot mode selection |
| PA5 | GPIO Output | Output | Green LED (LD2) | Visual indicator |

---

## Section 10: Clean-Up & Reset Procedure

### After Debug Session Completes

**To exit debugger:**
1. Click **Terminate** button (🔴 red square) in Debug panel
2. Or: `Run → Terminate`

**To reset board to normal mode (run user app):**
1. Press black **Reset** button (do NOT hold blue button)
2. User app should start immediately
3. TeraTerm on COM4 should show: `"Hello From User Application"`
4. Button press should toggle LED

### Verifying User App Works

**Quick test:**
1. Open TeraTerm: COM4, 115200 baud
2. Press Reset button
3. Should see continuous output: `"Hello From User Application"`
4. Press blue button → LED should toggle ON/OFF

**If user app doesn't work:**
- Check user app is flashed at 0x08008000 (use ST-Link Utility)
- Check VTOR relocation in user app `system_stm32f4xx.c` (should be `0x8000`)
- Reflash user app ELF file

### Returning to Bootloader Mode Later

**To test bootloader again:**
1. Close any TeraTerm/serial programs on COM4
2. Do button sequence: Hold blue → Reset → Wait 2s → Release
3. Run Python script as normal

---

## Appendix A: Known-Good System State

**Record your working configuration here for future comparison:**

### Software Versions (2026-02-07)
```
Python version:     ________________ (Expected: 3.13.1)
pyserial version:   ________________ (Expected: 3.5)
STM32CubeIDE:       ________________ (Expected: 1.17.0)
ST-Link firmware:   ________________ (Check ST-Link Utility)
```

### File Checksums

**To generate:**
```bash
certutil -hashfile Debug\FastBit_Badweh_Bootloader.bin MD5
certutil -hashfile Debug\FastBit_Badweh_UserApplication.bin MD5
```

**Record here:**
```
Bootloader binary MD5:  ______________________________________
User app binary MD5:    ______________________________________
```

### COM Port Configuration
```
ST-Link Virtual COM Port:  COM___ (Expected: COM4)
FTDI Debug Port (if used): COM___ (Expected: COM6)
```

### Binary Sizes
```
FastBit_Badweh_Bootloader.bin:     ________ bytes (Expected: ~19 KB)
FastBit_Badweh_UserApplication.bin: ________ bytes (Expected: ~8.5 KB)
```

### Flash Memory Verification (via ST-Link Utility)

**First 8 bytes at 0x08000000 (Bootloader):**
```
0x08000000: ________ ________ (MSP value, should be ~0x20018xxx)
0x08000004: ________ ________ (Reset handler, should be ~0x080001xx)
```

**First 8 bytes at 0x08008000 (User App):**
```
0x08008000: ________ ________ (MSP value, should be ~0x20018xxx)
0x08008004: ________ ________ (Reset handler, should be ~0x080081xx)
```

---

## Appendix B: Troubleshooting Decision Matrix

| Symptom | Likely Cause | Verification Step | Fix |
|---------|--------------|-------------------|-----|
| Python timeout (debug mode) | Debugger paused at breakpoint | Check IDE status bar | Hit Resume (F8) |
| Python timeout (normal mode) | Not in BL mode | Check debug UART or repeat button sequence | Hold button longer during reset |
| BP #2 never hit | User app running instead | Check if LED toggles on button | Repeat boot sequence correctly |
| `bl_rx_buffer` all zeros | No data received | Check COM port in Device Manager | Use correct COM number |
| `bl_rx_buffer` wrong values | Serial corruption | Check baud rate (115200) | Close other serial programs |
| CRC FAIL | CRC mismatch | Compare Python vs bootloader CRC | Check polynomial (0x04C11DB7) |
| IDE can't flash | ST-Link connection issue | Check green LED on ST-Link | Replug USB, update drivers |
| User app doesn't run after | VTOR not set | Check `system_stm32f4xx.c` | Uncomment `USER_VECT_TAB_ADDRESS` |
| "Port not found" in Python | COM port changed/missing | Device Manager → Ports | Reconnect USB, update script |

---

## Appendix C: Source Code References

### Boot Decision Logic (main.c:125-133)

```c
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
```

### UART Reception (main.c:537-544)

```c
// Phase 1: Read length byte
HAL_UART_Receive(C_UART, bl_rx_buffer, 1, HAL_MAX_DELAY);
rcv_len = bl_rx_buffer[0];

// Phase 2: Read remaining packet
HAL_UART_Receive(C_UART, &bl_rx_buffer[1], rcv_len, HAL_MAX_DELAY);

// Dispatch based on command code
switch (bl_rx_buffer[1]) {
    case COMMAND_BL_GET_VER:
        bootloader_handle_getver_cmd(bl_rx_buffer);
        break;
    // ... other commands
}
```

### BL_GET_VER Handler (main.c:496-523)

```c
void bootloader_handle_getver_cmd(uint8_t *bl_rx_buffer) {
    uint32_t command_packet_len = bl_rx_buffer[0] + 1;
    uint32_t host_crc = *((uint32_t *)(bl_rx_buffer + command_packet_len - 4));

    if (!bootloader_verify_crc(&bl_rx_buffer[0], command_packet_len - 4, host_crc)) {
        // CRC SUCCESS
        bootloader_send_ack(bl_rx_buffer[0], 1);
        uint8_t bl_version = get_bootloader_version();
        bootloader_uart_write_data(&bl_version, 1);
    } else {
        // CRC FAIL
        bootloader_send_nack();
    }
}
```

### Python Packet Construction (STM32_Programmer_F401RE.py:303-321)

```python
# Build BL_GET_VER command packet
data_buf[0] = COMMAND_BL_GET_VER_LEN - 1  # 0x05 (5 bytes to follow)
data_buf[1] = COMMAND_BL_GET_VER          # 0x51

# Calculate CRC32 over first 2 bytes
crc32 = get_crc(data_buf, COMMAND_BL_GET_VER_LEN - 4)
crc32 = crc32 & 0xffffffff

# Pack CRC into bytes 2-5 (little-endian)
data_buf[2] = word_to_byte(crc32, 1, 1)
data_buf[3] = word_to_byte(crc32, 2, 1)
data_buf[4] = word_to_byte(crc32, 3, 1)
data_buf[5] = word_to_byte(crc32, 4, 1)

# Send packet to COM4
Write_to_serial_port(data_buf[0], 1)
for i in data_buf[1:COMMAND_BL_GET_VER_LEN]:
    Write_to_serial_port(i, COMMAND_BL_GET_VER_LEN - 1)
```

---

## Document Revision History

| Date | Version | Changes | Author |
|------|---------|---------|--------|
| 2026-02-07 | 1.0 | Initial creation - Lesson 056 checkpoint | Claude + User |

---

**END OF PLAYBOOK**

**Next Steps After Passing This Checkpoint:**
- Lesson 057-058: Test BL_GET_HELP and BL_GET_CID commands
- Lesson 063-066: Test flash erase operations
- Lesson 067-068: Test full IAP workflow (BL_MEM_WRITE)
- Phase 8: Complete field firmware update demonstration

**For questions or issues, refer to:**
- Course summary: `FastBit9_Bootloader/Bootloader_Claude_Summary.md`
- Testing guide: `FastBit_Badweh_Bootloader/Debug/README_TESTING.md`
