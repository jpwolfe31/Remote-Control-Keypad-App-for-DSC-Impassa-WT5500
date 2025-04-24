void menu() // main waterfall command menu
{ 
  // After menu() runs, com_str is set to "cleared" in the loop timer to 
  // avoid repeated commands running.  A new terminal command 
  // overwrites "cleared" and allows menu() to run again.
  // return if command already executed
  if (strcmp(cmd_str, "cleared") == 0) {return;}
  // new command
  // get command length used for decoding below
  cmd_length_int = strlen(cmd_str);  // note - does not include '\0'.
  lastCommandMillis = currentMillis; // update LCD time out on terminal or keyboard command
  // page two commands start first
  // for second line Wifi ssid
    if ((serial_second_line_flag_int == 2) && (strcmp(cmd_str, "") != 0)) { // returns 0 if equal
     if ((strlen(cmd_str) > 15) || (strlen(cmd_str) < 5)) { // note- null character ‘\0’ not counted
       terminal.println ("Invalid entry");
       terminal.println();  // add line feed
       terminal.flush();
       Blynk.run();
       serial_second_line_flag_int = 0; // reset file read flag 
       return;
       }
     strcpy(ssid, cmd_str);
     terminal.print("Wifi SSID changed to: ");
     terminal.println(ssid); 
     terminal.println();  // add line feed
     terminal.flush();
     Blynk.run();
     EEPROMWrite(); 
     serial_second_line_flag_int = 0; // reset file read flag 
     return;
     }
  // for second line Wifi password
    if ((serial_second_line_flag_int == 3) && (strcmp(cmd_str, "") != 0)) { // returns 0 if equal
     if ((strlen(cmd_str) > 15) || (strlen(cmd_str) < 8)) { // note - null character ‘\0’ not counted
       terminal.println ("Invalid entry");
       terminal.println();  // add line feed
       terminal.flush();
       Blynk.run();
       serial_second_line_flag_int = 0; // reset file read flag 
       return;
       }
     strcpy(pass, cmd_str);
     terminal.print("Wifi password changed to: ");
     terminal.println(pass); 
     terminal.println();  // add line feed
     terminal.flush();
     Blynk.run();
     EEPROMWrite();
     serial_second_line_flag_int = 0; // reset file read flag 
     return;
     }
  // for second line GMT offset
    if ((serial_second_line_flag_int == 4) && (strcmp(cmd_str, "") != 0)) { // returns 0 if equal
      int l = atoi(cmd_str);
      if ((l > 12) || (l < -12)) {
        terminal.println ("Invalid entry");
        terminal.println();  // add line feed
        terminal.flush();
        Blynk.run();
        serial_second_line_flag_int = 0; // reset file read flag 
        return;
        }
      strcpy(k_GMT_str, cmd_str);
      k_GMT_int = atoi(k_GMT_str);
      EEPROMWrite();
      serial_second_line_flag_int = 0; // reset file read flag
      updateRTC();
      // read new time
      updateTime();
      //terminal.print("GMT offset changed to: ");
      //terminal.println(k_GMT_str); 
      //terminal.println("Clock updated"); 
      terminal.println(); // add line feed
      terminal.flush();
      Blynk.run();
      return;
      }
  // for second line LCDCheck()
    if ((serial_second_line_flag_int == 5) && (strcmp(cmd_str, "") != 0)) { // returns 0 if equal
     strcpy(displayLine1In, cmd_str);
     strcat(displayLine1In, "                 ");  // add spaces to end to make at least 16
     displayLine1In[16] = '\0'; // terminate string
     strcpy(displayLine2In, displayLine1In);  
     //terminal.println(strlen(displayLine1In));
     //terminal.println(strlen(displayLine2In));
     unsigned long currentMicros = micros();
     LCDCheck();
     int eti = (int)(micros() - currentMicros);
     terminal.print("Elapsed time was ");
     terminal.print(eti);
     terminal. println(" us");
     terminal.print("Result returned for 1: -");
     terminal.print(displayLine1Out); 
     terminal.println("-");
     //terminal.print("Error count1: ");
     //terminal.println(errorCntLine1); 
     terminal.print("Result returned for 2: -");
     terminal.print(displayLine2Out); 
     terminal.println("-");
     //terminal.print("Error count2: ");
     //terminal.println(errorCntLine2); 
     
     terminal.println();  // add line feed
     terminal.flush();
     Blynk.run();
     serial_second_line_flag_int = 0; // reset file read flag 
     return;
     }
  esp_task_wdt_reset();  // refresh watch dog timer
  // end page two commands

  if (strcmp(cmd_str, "cmd") == 0) { // list commands
    terminal.println("x      - virtual keypad 123456789*0#<>sacbed");
    terminal.println(" xxxx  - virtual keypad multikey");
    terminal.println("ost    - outdoor siren program - 1 and 6 for on");
    terminal.println("rst    - reset controller");
    terminal.println("rst2   - reset LCD buss sniffer");
    terminal.println("sig    - report WiFi signal strength"); 
    terminal.println("v      - report version of code");
    terminal.println("clr    - Blynk terminal clear");
    terminal.println("cmd    - list available commands");
    terminal.println("cmdm   - list more commands");
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    } // note 50 max terminal width
  if (strcmp(cmd_str, "cmdm") == 0) { // list more commands
    terminal.println("tr     - report time once");
    terminal.println("ts     - report/syncs rtc/WiFi times");
    terminal.println("st     - report op status");
    terminal.println("rth    - report thermistor values (toggle)");
    terminal.println("tled   - test nano leds");
    terminal.println("tbled  - test Blynk virtual leds");
    terminal.println("tlc    - test LCDCheck");
    terminal.println("wdt    - test watchdog timer");
    terminal.println("cssid  - change Wifi SSID (eeprom)");  // second page flag 2
    terminal.println("cpass  - change Wifi password (eeprom)");   // second page flag 3
    terminal.println("cgmto  - change GMT offset (eeprom)");  // second page flag 4
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    }
// char keyChart_str[21] =  "?123456789*0#<>sacbe"); // 19 keys possible plus ? and \0
// find key number for key using keyChart_str
if (strlen(cmd_str) == 1){
  char *e;
  e = strchr(keyChart_str, cmd_str[0]);
  int keyPos = (int)(e - keyChart_str);
  if (e) {// valid character found
    terminalKeyPress(keyPos);
    return;
    }
  }  
if (strcmp(cmd_str, "d") == 0) { // disarm - make new cmd_str for virtual multikey
    strcpy(cmd_str, " "); // create command code
    strcat(cmd_str, masterCode);  // add masterCode
    cmd_length_int = strlen(cmd_str);  // update cmd_str length - does not include '\0' - start at [0] char.
    } // no return here so drop down to virtual multikey with new command
  
  if (cmd_str[0] == ' '){  // virtual keypad multikey " xxxx..."
    for (int i = 1; i < cmd_length_int; i++){ // skip first space as control char - length includes [0] char.
      char *e;
      e = strchr(keyChart_str, cmd_str[i]);
      int keyPos = (int)(e - keyChart_str);
      if (e) {// valid character found
        terminalKeyPress(keyPos);  // output char string to keypad
        delay500ms();
        }
      }  
    return;
    }
  if (strcmp(cmd_str, "ost") == 0){  // outdoor siren programming - 1 is siren, 6 is strobe
    strcpy(cmd_str, " *8____804312");
    cmd_length_int = strlen(cmd_str);  // note - does not include '\0'.
    for (int i = 1; i < cmd_length_int; i++){ // skip first space as control char - length includes [0] char.
      char *e;
      e = strchr(keyChart_str, cmd_str[i]);
      int keyPos = (int)(e - keyChart_str);
      if (e) {// valid character found
        terminalKeyPress(keyPos);  // output char string to keypad
        delay500ms();
        }
      }  
    return;
    }

  if (strcmp(cmd_str, "tled") == 0){ // note - does not seem to work with the current nano
    // tled - test leds
    terminal.println();
    terminal.println("Testing Nano ESP32 built-in leds 4 times in 12 seconds");
    terminal.println();
    terminal.flush();
    Blynk.run();
    for (int iled = 0; iled < 4; iled++){
      digitalWrite(LED_BUILTIN, HIGH);  // Note also used by Blynk to show link status active high
      digitalWrite(LEDR, LOW);  // active low red 
      digitalWrite(LEDG, HIGH);
      digitalWrite(LEDB, HIGH);
      delay1s();
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LEDR, HIGH);
      digitalWrite(LEDG, LOW); // active low green
      digitalWrite(LEDB, HIGH);
      delay1s();
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LEDR, HIGH);
      digitalWrite(LEDG, HIGH);
      digitalWrite(LEDB, LOW); // active low blue
      delay1s();
      digitalWrite(LEDR, HIGH);
      digitalWrite(LEDG, HIGH);
      digitalWrite(LEDB, HIGH); // off
      digitalWrite(LED_BUILTIN, LOW);
      delay1s();
      }
    delay(50);
    return;
    }
  
  if (strcmp(cmd_str, "tbled") == 0){ 
    // tbled - test blynk virtual leds
    terminal.println();
    terminal.println("Testing Blynk virtural leds 3 times in 21 seconds");
    terminal.println();
    terminal.flush();
    Blynk.run();
    for (int iled = 0; iled < 3; iled++){
      ledKeyPress.on();
      delay1s();
      ledKeyPress.off();
      ledRTA.on();
      delay1s();
      ledRTA.off();
      ledArmed.on();
      delay1s();
      ledArmed.off();
      ledTrouble.on();
      delay1s();
      ledTrouble.off();
      ledAC.on();
      delay1s();
      ledAC.off();
      ledBell.on();
      delay1s();
      ledBell.off();
      ledWaitLCD.on();
      delay1s();
      ledWaitLCD.off();
      }
    delay(50);
    display_RTALEDValue = 0; // update display values
    display_armedLEDValue = 0; 
    display_troubleLEDValue = 0; 
    display_ACLEDValue = 0;
    display_bellLEDValue = 0;
    return;
    }
  if (strcmp(cmd_str, "rst") == 0) {  // reset nano
    terminal.println(); // add line feed
    terminal.println("device reset through Blynk terminal");
    // report type of error
    strcpy(error_type_str, "BT");
    nanoError();
    return;
    }
  if (strcmp(cmd_str, "rst2") == 0) {  // - reset LCD buss sniffer
    digitalWrite(LCDBSResetPin, LOW); // reset active low
    delay500ms();
    digitalWrite(LCDBSResetPin, HIGH); // reset active low
    terminal.println(); // add line feed
    terminal.println("LCD buss sniffer reset through Blynk terminal");
    terminal.println(); // add line feed
    return;
    }
  if (strcmp(cmd_str, "rth") == 0) {  //  report thermistor values
    if (thermistorReportFlag == 0){thermistorReportFlag = 1;}
    else thermistorReportFlag = 0;
    return;
    }
  if (strcmp(cmd_str, "tr") == 0) {  // report time
    // report time data
    terminal.println(); // add line feed
    // terminal.print("nano rtc ");
    // (String) returns time with specified format 
    //terminal.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));   // for testing
    updateRTC();
    updateDate();
    updateTime();
    terminal.print(date_str);
    terminal.print("  ");
    terminal.println(time_str);
    terminal.println();
    return;
    }  
  if (strcmp(cmd_str, "ts") == 0) {  // report and sync rtc and current wifi times 
    // get time data
    terminal.println(); // add line feed
    updateTime();
    terminal.print("rtc time was ");
    terminal.println(time_str);
    // update rtc
    updateRTC();
    // read new time
    terminal.println("WiFi and rtc time synced");
    updateTime();
    terminal.print("rtc time is now ");
    terminal.println(time_str);
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    } 
  if (strcmp(cmd_str, "clr") == 0) { // Clear the terminal content    // note - returns 0 if equal
    terminal.clear();  // this is the remote clear, not local terminal
    terminal.flush();
    Blynk.run();
    return;
    }

  if (strcmp(cmd_str, "cssid") == 0) { // change Wifi SSID
    terminal.print("Current Wifi SSID: ");
    terminal.println(ssid); 
    terminal.println("Enter new Wifi SSID: ");
    serial_second_line_flag_int = 2;  // set flag for next WifI SSID line read
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    }
  if (strcmp(cmd_str, "cpass") == 0) { // change Wifi password
    terminal.println("Enter new Wifi password: ");
    serial_second_line_flag_int = 3;  // set flag for next WifI password line read
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    }
  if (strcmp(cmd_str, "cgmto") == 0) { // change GMT offset
    terminal.println("Enter new GMT offset: ");
    serial_second_line_flag_int = 4;  // set flag for next GMT offset line read
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    }
  if (strcmp(cmd_str, "tlc") == 0) {  // check LCDCheck function
    terminal.println("LCD Check - Enter string: ");
    serial_second_line_flag_int = 5;  // set flag for LCD string line read
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    }
  if (strcmp(cmd_str, "v") == 0) {  // report version
    terminal.println(); // add line feed
    terminal.print("Version of Controller Code is: ");
    terminal.println(version_str);
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    }
  if (strcmp(cmd_str, "sig") == 0) {  // report wifi signal strength
    rssi = WiFi.RSSI();
    terminal.println(); // add line feed
    terminal.print("Signal strength (RSSI) is ");
    terminal.print(rssi);
    terminal.println(" dBm");
    terminal.println(); // add line feed
    terminal.flush();
    Blynk.run();
    return;
    }
  if (strcmp(cmd_str, "wdt") == 0) {  // check wdt function
    unsigned int t;
    terminal.println("\nWatchdog Test - run 18 seconds with a WDTimer.clear()\n");
    //Serial.println("\nWatchdog Test - run 18 seconds with a WDT.clear()\n");
    for (t = 1; t <= 18; ++t) {
      esp_task_wdt_reset();  // refresh wdt - before it loops
      delay(950);
      terminal.print(t);
      terminal.print(".");
      terminal.flush();
      Blynk.run(); 
      }
    terminal.println("\n\nWatchdog Test - free run wait for reset at 8 seconds\n");
    for (t = 1; t >= 1; ++t) {
      delay(950);
      terminal.print(t);
      terminal.print(".");
      terminal.flush();
      Blynk.run();
      }
    return;
    }   
  if (strcmp(cmd_str, "st") == 0) {
    //terminal.print("redLedState = ");
    //terminal.println(redLedState);
    //terminal.print("blueLedState = ");
    //terminal.println(blueLedState);
    //terminal.flush();  // output to terminal immediately
    terminal.print("uptime = ");
    terminal.print(millis() / 60000);
    terminal.println(" minutes");
    terminal.print("thermistorValue = ");
    terminal.print(thermistorValue); // for testing
    terminal.print("   ");
    terminal.print(thermistorTemp);
    terminal.println(" deg F");
    terminal.print("timeOutLCD = ");
    terminal.println(timeoutLCD);
    terminal.print("ssid = ");
    terminal.println(ssid);
    terminal.print("pass = ");
    terminal.println(pass);
    terminal.print("GMT offset = ");
    terminal.println(k_GMT_int);
    terminal.println(); // add line
    terminal.flush();
    Blynk.run();
    return;
    }
  }
// end of command waterfall

// for alarm keypad buttons on Blynk app 0 to 17 for buttons 0-9 * # < > stay away, exit, bypass
// terminal on 18.  note - 19 is still available.  note - do not really need stay, away exit or bypass
// so could add 3 buttons and lcd display

BLYNK_WRITE(V0)  // multiple keys 
{
switch (param.asInt()) {
  case 1: // button 1 etc.
    keyValue[1] = 1;
    break;
  case 2:
    keyValue[2] = 1;
    break;
  case 3:
    keyValue[3] = 1;
    break;
  case 4:
    keyValue[4] = 1;
    break;
  case 5:
    keyValue[5] = 1;
    break;
  case 6:  // button 0
    keyValue[6] = 1;
    break;
  case 7:
    keyValue[7] = 1;
    break;
  case 8:
    keyValue[8] = 1;
    break;
  case 9:
    keyValue[9] = 1;
    break;
  case 10:  // button *
    keyValue[10] = 1;
    break;
  case 11:  // button 0
    keyValue[11] = 1;
    break;
  case 12:  // button #
    keyValue[12] = 1;
    break;
  case 13:  // button <
    keyValue[13] = 1;
    break;
  case 14:  // button >
    keyValue[14] = 1;
    break;
  case 15:  // button Stay
    keyValue[15] = 1;
    break;
  case 16:  // button Away
    keyValue[16] = 1;
    break;
  case 17:  // button Chime
    keyValue[17] = 1;
    break;
  case 18:  // button Bypass
    keyValue[18] = 1;
    break;
  case 19:  // button Exit
    keyValue[19] = 1;
    break;
  case 0: // need to turn off all buttons as previous key not known
    for (int k = 0; k <= 19; k++){keyValue[k] = 0;}
    break;  
  }
}

// for disarm button as not part of keypad
BLYNK_WRITE(V1)  // button Disarm
{
keyValueDisarm = param.asInt(); // assigning incoming value from pin Vx to a variable
}

// V16, V17 LCD Line 1 and 2

// For Blynk terminal commands
BLYNK_WRITE(V18) // for reading terminal commands written from Blynk
{
strcpy(cmd_str, param.asStr());  // copy Blynk app terminal input to cmd_str  
//terminal.println(cmd_str); // for testing
//terminal.flush();
//Blynk.run();
}

