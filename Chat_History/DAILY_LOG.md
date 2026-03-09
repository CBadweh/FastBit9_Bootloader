❯ from the source code, it's "pBuffer[0], 1" isntead of "pBuffer[1],1" why are they           different?                                                                               
  ⎿  ⧉ Selected 1 lines from                                                                
     Bootloader_Code\Badweh_Code\Badweh_Bootloader_Milestone123456\Core\Src\main.c in Cursor

● Look at your bootloader_send_ack() implementation at main.c:441:                             
  void bootloader_send_ack(uint8_t command_code, uint8_t follow_len)                          {
      uint8_t ack_buf[2];                                                                   
      ack_buf[0] = BL_ACK;
      ack_buf[1] = follow_len;
      HAL_UART_Transmit(C_UART, ack_buf, 2, HAL_MAX_DELAY);
  }

  The command_code parameter is never used in the function body — it's received but
  discarded. Only follow_len matters.

  So:
  - pBuffer[0] = length byte
  - pBuffer[1] = command code

  Both work identically because neither value affects what gets transmitted. The ACK always 
  sends 0xA5 + follow_len regardless.

  The course passing pBuffer[0] is arguably a mistake (passing length instead of command    
  code), but since the parameter is unused it has zero functional impact. Your code passing 
  pBuffer[1] is actually more semantically correct.

❯ the "pBuffer" from "bootloader_handle_go_cmd()" is the same as "bl_rx_buffer" in          
  "bootloader_uart_read_data()" right?
  ⎿  ⧉ Selected 1 lines from                                                                     Bootloader_Code\Badweh_Code\Badweh_Bootloader_Milestone123456\Core\Src\main.c in Cursor                                                                                            ● Yes, exactly. In bootloader_uart_read_data() at main.c:401:                                                                                                                             case BL_GO_TO_ADDR:                                                                             bootloader_handle_go_cmd(bl_rx_buffer);  // passes bl_rx_buffer                       
      break;                                                                                
  
  So inside bootloader_handle_go_cmd(uint8_t *pBuffer), pBuffer is just a pointer to the
  same bl_rx_buffer array. They point to identical data:

  bl_rx_buffer[0] = pBuffer[0] = length byte
  bl_rx_buffer[1] = pBuffer[1] = command code (0x55)
  bl_rx_buffer[2] = pBuffer[2] = go_address byte 0 (LSB)
  bl_rx_buffer[3] = pBuffer[3] = go_address byte 1
  bl_rx_buffer[4] = pBuffer[4] = go_address byte 2
  bl_rx_buffer[5] = pBuffer[5] = go_address byte 3 (MSB)
  bl_rx_buffer[6..9]            = CRC32 (4 bytes)

  pBuffer is just the handler's local name for the same buffer.

✻ Cogitated for 30s 

❯ what's the role of "bootloader_send_ack(pBuffer[1], 1);" and explain its parameters         ⎿  ⧉ Selected 1 lines from                                               
     Bootloader_Code\Badweh_Code\Badweh_Bootloader_Milestone123456\Core\Src\main.c in Cursor                  
● bootloader_send_ack(pBuffer[1], 1) transmits 2 bytes to the host telling it:              
  1. CRC passed — data is coming
  2. How many bytes of reply data to expect                                                                                                                                             
  Parameters:                                                                               
  
  ┌────────────┬──────────────────────┬─────────────────────────────────────────────────┐
  │ Parameter  │        Value         │                     Meaning                     │   
  ├────────────┼──────────────────────┼─────────────────────────────────────────────────┤   
  │ pBuffer[1] │ 0x55 (BL_GO_TO_ADDR  │ Command code — passed in but unused inside the  │   
  │            │ code)                │ function (as discussed)                         │   
  ├────────────┼──────────────────────┼─────────────────────────────────────────────────┤   
  │ 1          │ follow_len           │ "1 byte of reply data will follow this ACK"     │   
  └────────────┴──────────────────────┴─────────────────────────────────────────────────┘   

  What gets transmitted on UART:
  0xA5  0x01
   ↑     ↑
  ACK   1 byte follows

  The host sees this and knows:
  - CRC check passed on the bootloader side
  - Wait to receive exactly 1 more byte — which will be the address status (0x00 = valid,   
  0x01 = invalid)

  So the full exchange for a valid address is:
  Bootloader → Host:  0xA5  0x01       (ACK + 1 byte follows)
  Bootloader → Host:  0x00             (ADDR_VALID)


   This is a pointer address right? We created the function, with our reset handler          
  address,  in line 745 and then jumped to it at 749                                       
  ⎿  ⧉ Selected 1 lines from                                                                     Bootloader_Code\Badweh_Code\Badweh_Bootloader_Milestone123456\Core\Src\main.c in Cursor                                                                                            ● Yes. Let me walk through those lines:                                                                                                                                                   go_address += 1;                                    // T-bit = 1 (Thumb mode)                                                                                                         
  void (*lets_jump)(void) = (void *)go_address;       // line 750: declare function pointer,
   assign address 

  printmsg("BL_DEBUG_MSG: Jumping to go address!\r\n");

  lets_jump();                                        // line 754: jump — never returns     

  Breaking down void (*lets_jump)(void):

  ┌──────────────────────┬──────────────────────────────────────┐
  │         Part         │               Meaning                │
  ├──────────────────────┼──────────────────────────────────────┤
  │ void (return)        │ function returns nothing             │
  ├──────────────────────┼──────────────────────────────────────┤
  │ (*lets_jump)         │ lets_jump is a pointer to a function │
  ├──────────────────────┼──────────────────────────────────────┤
  │ (void) (param)       │ function takes no arguments          │
  ├──────────────────────┼──────────────────────────────────────┤
  │ = (void *)go_address │ point it at our target address       │
  └──────────────────────┴──────────────────────────────────────┘

  So lets_jump is not a function itself — it's just a variable that holds an address.       
  Calling lets_jump() tells the CPU: jump to whatever address is stored in this variable and
   start executing instructions there.

  It's exactly the same mechanism used in bootloader_jump_to_user_app() at main.c:355:      
  app_reset_handler = (void*) resethandler_address;
  app_reset_handler();   // same idea

  The difference is bootloader_jump_to_user_app() also sets MSP first — BL_GO_TO_ADDR skips 
  that and jumps directly.




## BL_FLASH_ERASE

  **Demo — Sector Erase:**
- Erase sector 2: Command 7, sector=2, count=1 → success → ST-LINK shows all `0xFF` at `0x08008000`
	**Commands**:
		command code: 7
		Enter sector number: 2
		Enter number of sectors to erase .. : 1
- Erase sectors 2+3: Command 7, sector=2, count=2 → success → both sectors erased

**Demo — Mass Erase:**
- Command 7, sector=`0xFF` → all sectors erased (including bootloader!)
- After mass erase, bootloader no longer responds (it was erased) — must be re-flashed via ST-Link
		Commands:
			command code: 7
			Enter sector number: 0xff



## BL_MEM_WRITE

Needs to save the .bin in the same folder as the python script

Demo: 
    <!-- move the .bin to where the python script is -->
    move the file
    <!-- Erase sector 2 -->
    cmd: 7
    cmd: 2
    cmd: 2
    <!-- Write command -->
    cmd: 8
    cmd: 0x08008000

    <!-- Trigger the binary with go to command -->
    cmd: 5
    cmd: 0x080088D0