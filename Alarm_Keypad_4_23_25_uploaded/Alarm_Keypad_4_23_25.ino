// need to fix toggle issue
// Template ID, Device Name and Auth Token are provided by Blynk
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME "Alarm Keypad"
#define BLYNK_AUTH_TOKEN ""
// not used in agent, just uploaded app
#define BLYNK_FIRMWARE_VERSION "1.0.1"
char auth[] = BLYNK_AUTH_TOKEN;
char version_str[32] = "V.042325";  // month/day/year

char ssid[20] = "";
char pass[20] = ""; // Set password to "" for open networks.

char masterCode[20] = "";

// for SerialLCD from LCD sniffer
#include "HardwareSerial.h"
HardwareSerial SerialLCD(2); // use Uart2 - no pins assigned by default
const int SerialLCD_RX = A6;
const int SerialLCD_TX = A7;

// for eeprom emulation in nano ESP32 flash memory
// esp32 EEPROM retains its value between program uploads
// if the EEPROM is written, then there is a written signature at address 0
// update signature when eeprom data structure is changed
#include <EEPROM.h>
#define EEPROM_SIZE 1000  // This is 1k Byte
uint16_t storedAddress = 0;
int signature;
const int WRITTEN_SIGNATURE = 0xabcdabc4;
char k_GMT_str[20] = "-8"; //-8 for California ST and -7 for California DST
int k_GMT_int;
struct memory  // create data structure for easier EEPROM reads/writes
{ // note - all strings - max length 19
  int eeprom_signature;
  char eeprom_ssid[20];
  char eeprom_pass[20]; 
  char eeprom_k_GMT[20];
} flash;
int eepromFirstWriteFlag = 0;

// for wifi and Blynk
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// for time and clock
#include "time.h"
#include <ESP32Time.h>
ESP32Time rtc(0*3600);
const long  gmtOffset_sec = 3600 * -8; // Pacific Standard Time
const int   daylightOffset_sec = 3600;
const char* ntpServer = "pool.ntp.org";
struct tm timeinfo;

long rssi;
char rssi_str[20];

// for date and time functions
char second_str[20];
int second_int;
char minute_str[20];
int minute_int;
char hour_str[20];
int hour_int;
char days_str[20];
int days_int;
char months_str[20];  // 10 causes sprintf warning
int months_int;
char years_str[20];
int years_int;
char date_str[40]; // date string
char time_str[40]; // time string

// for error logging and resets
char error_type_str[40];
char error_type_display_str[40];
// set the reset flag on error and clear the reset flag when restarting
int nano_reset_flag_int = 0;

// for watch dog timer
#include <esp_task_wdt.h>
#define WDT_TIMEOUT 8 // 3, 8 and 16  seconds tested

// for disarm panel key - not on keypad below
  int keyValueDisarm = 0;

// for keypad panel keys - 19 keys plus a 0 keyValue that clears all keyValues when any key is released
char keyChart_str[21] =  "?123456789*0#<>sacbe"; // 19 keypad plus, '?' for off state and '\0' 
int keyValue[20] = {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0};  // keypad values read 
int keySelectValues[20][2] = { // row and column outputs below used if key value is set to 1 through read
  {0, 0}, // used as a placholder for off state for all keyValues 
  {0, 1}, {0, 2}, {0, 3}, 
  {1, 1}, {1, 2}, {1, 3}, 
  {2, 1}, {2, 2}, {2, 3}, // 1-9
  {3, 1}, {3, 2}, {3, 3}, // * 0 #
  {0, 0}, {1, 0},  // < >
  {0, 4}, {1, 4}, {2, 4}, {3, 4}, {2, 0} // stay, away, chime, bypass, exit
  // 19 keys on keypad
  };

/* 
Keypad Truth Table
    
  1, R0, C1
  2, R0, C2
  3, R0, C3

  4, R1, C1
  5, R1, C2
  6, R1, C3

  7, R2, C1
  8, R2, C2
  9, R2, C3
    
  *, R3, C1  
  0, R3, C2
  #, R3, C3
  
  <, R0, C0
  >, R1, C0

  S, R0, C4
  A, R1, C4
  C, R2, C4
  B, R3, C4
  E, R2, C0

for two 74HC4051 8 to 1 analog multiplexers controlling keypad
  pin numbering 
  1 CH 4 IN/OUT I/O Channel 4 in/out
  2 CH 6 IN/OUT I/O Channel 6 in/out
  3 COM OUT/IN I/O Common out/in  (connected to other output)
  4 CH 7 IN/OUT I/O Channel 7 in/out
  5 CH 5 IN/OUT I/O Channel 5 in/out
  6 INH I Disables all channels (+ voltage inhibits output)
  7 VEE — Negative power input (Ground in this case)
  8 VSS — Ground
  9 C I Channel select C.
  10 B I Channel select B.
  11 A I Channel select A.
  12 CH 3 IN/OUT I/O Channel 3 in/out
  13 CH 0 IN/OUT I/O Channel 0 in/out
  14 CH 1 IN/OUT I/O Channel 1 in/out
  15 CH 2 IN/OUT I/O Channel 2 in/out
  16 VDD — Positive power inpu
One each for rows 0-3 and for columns 0-4 
*/

// Can only have 20 datastreams in Blynk plan
// for Blynk  turn on or off with ledKeyPress.on(), ledKeyPress.off(); 
WidgetLED ledKeyPress(V3);
WidgetLED ledRTA(V4);
WidgetLED ledArmed(V5);
WidgetLED ledTrouble(V6);
WidgetLED ledAC(V7);
WidgetLED ledBell(V8);
WidgetLED ledWaitLCD(V9);

// Attach virtual serial terminal to Virtual Pin V18
WidgetTerminal terminal(V18); // 

// for reading a second input line in the Blynk terminal
int terminal_second_line_flag_int = 0;

// for USB Serial and Blynk commands
char cmd_str[40];
int cmd_length_int = 0;
int cmd_flag_int = 0;
int serial_second_line_flag_int = 0;
int timeoutLCD = 3;  // 3 minute timeout one LCD updates if no keypress or command
int onFlagLCD = 1; 
int keyLED = 0;

// for manual timers
unsigned long currentMillis;
unsigned long previousMillis_Blynk = 0;
unsigned long previousMillis_updateInputs = 0;
unsigned long previousMillis_blinkOnBoardLED = 0;
unsigned long previousMillis_nanoReset = 0; 
unsigned long lastCommandMillis = 0;
unsigned long previousMillis_updateTherm = 0;

// for notifications
char notification_str[256]; 

// for time functions 
int currentDisplayHours_int = 999; // will cause time to be set
int currentDisplayMinutes_int = 999;

// for display commands 
// for receicing data from sniffer and sending data to Blynk LCD
int dataLCDFlag = 0;
char dataLCDStr[40] = ""; // both lines  // all these lengths need to be the same or you can overflow
int dataLCDStrLength = 0; // both lines
char displayLines[40] = ""; // both lines
char displayLine1[40] = ""; // line 1 to LCD
char displayLine2[40] = ""; // line 2 to LCD
// for LCD Check function
char displayLine1In[40];
char displayLine1Out[40];
char displayLine2In[40];
char displayLine2Out[40];
char lastDisplayLine1Out[40];
char lastDisplayLine2Out[40];
int errorCntLine1 = 0;
int errorCntLine2 = 0;
int indexLine1Out = -1;
int indexLine2Out = -1;

int alarmFlagLCD = 0;  // set alarm flag off - toggled on/off in LCDCheck function
// for Blynk Bell LED
int bellLEDValue = 0;
int display_bellLEDValue = 0;

/*
If using Nano ESP32 - in the IDE Tools menu, enter Pin Numbering and choose By GPIO number (legacy).
Then sketch should always use labels to refer to Arduino pins (e.g if you used the 
number 2, replace this with the symbol D2).
This is a more library-compatible scheme and avoids confusion.  
Also see pin table at end of this sketch.
*/

// for leds
// configure leds
// Nano ESP32 on board RGB led
#define LEDR 46  // Note - Boot0  active low
#define LEDB 45  // Note - not an on-board pinout  active low
#define LEDG 0   // Note - B00t1 active low
// green power led always on with power
// yellow on board led - also called LED builtin -
//   is on D13 or GPIO48 - SPI Serial Clock
int onBoardLEDValue = 0; // for blinking on board led

// for keypad row and column output drivers
int RS0Pin = D2;
int RS1Pin = D3;
int RS2Pin = D4;
int CS0Pin = D5;
int CS1Pin = D6;
int CS2Pin = D7;
int RENPin = D8;
int CENPin = D9;
//note consdier selecting column 7 as a disable instead of using pins D6 and D7

// for inputs (from panel leds)
// note - display variables used to only update Blynk on change 
// RTA
int RTAPin = A0; // off 0V, on > 0.6V
int RTALEDValue = 0;
int display_RTALEDValue = 0;
// Armed - stay or away
int armedPin = A1;
int armedLEDValue = 0; // off 0V, on > 0.6V
int display_armedLEDValue = 0; 
// Trouble
int troublePin = A2; // off 0V, on > 0.6V
int troubleLEDValue = 0;
int display_troubleLEDValue = 0; 
// AC
int ACPin = A3; // off 0V, on > 0.6V
int ACLEDValue = 0;
int display_ACLEDValue = 0;

int thermistorPin = A5; 
int thermistorValue; // 0-4096 - 12 bit esp32 nano resolution
int thermistorTemp;
int thermistorReportFlag = 0; // for terminal print out thermistor values 

// for LCD buss sniffer reset - active low 
int LCDBSResetPin = D12;

void setup()
{   
// first thing - turnoff keyboard during reset
// for keypad row and column drivers
pinMode(RENPin, OUTPUT); // configure and turn of 4051s
digitalWrite(RENPin, HIGH); 
pinMode(CENPin, OUTPUT);
digitalWrite(CENPin, HIGH);
pinMode(RS0Pin, OUTPUT); // confiure and set 4051 selects to 0
pinMode(RS1Pin, OUTPUT);
pinMode(RS2Pin, OUTPUT);
digitalWrite(RS0Pin, LOW);
digitalWrite(RS1Pin, LOW);
digitalWrite(RS2Pin, LOW);
pinMode(CS0Pin, OUTPUT); // confiure and set 4051 selects to 0
pinMode(CS1Pin, OUTPUT);
pinMode(CS2Pin, OUTPUT);
digitalWrite(CS0Pin, LOW);
digitalWrite(CS1Pin, LOW);
digitalWrite(CS2Pin, LOW);

// for Serial
Serial.begin(115200); // for serial monitor
// while (!Serial) {}; // wait for serial port to connect.
delay(500);
SerialLCD.begin(115200, SERIAL_8N1, SerialLCD_RX, SerialLCD_TX);  // for LCD Serial data
delay(500);

// for eeprom
// In the ESP32, a typical Flash page is 64-Bytes and you need to read-modify-write
// an entire page at a time.  The library saves the data to a buffer with the write() 
// or put() function and it is not actually written to Flash memory until 
// EEPROM.commit() is called. 
// Write eeprom data or if reset, obtain eeprom data
// Check signature at address 0
// If the eeprom is written, then there is a correct written signature.
// Note - unlike Arduino MKR1010 flash memory, this flash memory persists after  
// reprogramming.
EEPROM.begin(EEPROM_SIZE);
EEPROM.get(storedAddress, signature);
// If the EEPROM is written, then there is a orrect written signature
if (signature == WRITTEN_SIGNATURE){
  EEPROMRead();  // print confirmation of data written and update variables
  }
else { // eeprom is not written and needs to be written
  EEPROMWrite();
  eepromFirstWriteFlag = 1;  // report new write through Blynk terminal
  }

// for watch dog timer
esp_task_wdt_init(WDT_TIMEOUT, true); // enable wdt
esp_task_wdt_add(NULL); //add current thread to WDT watch
// when time runs out, processor does a hardware reset

// read eeprom first - for wifi inputs
// connect to Blynk and WiFi network  // note - this needs to have the eeprom read first to connect to storred ssid
Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);  // this code works well with the nano, not so much with mkr
esp_task_wdt_reset();
// print out the status on the serial port
Serial.print("SSID: ");
Serial.println(WiFi.SSID());
// print out the WiFi IP address:
IPAddress ip = WiFi.localIP();
Serial.print("IP Address: ");
Serial.println(ip);
// print and display the received signal strength
rssi = WiFi.RSSI();
Serial.print("Signal strength (RSSI):");
Serial.print(rssi);
Serial.println(" dBm");
terminal.println();

// The 'ESP32Time' library is just a wrapper interface for the functions available 
// in 'esp_sntp.h'.  There is no real need for ESP32Time.h other than convenience.
//  As long as WiFi is connected, the ESP32's internal RTC will be periodically 
//  synched to NTP.  The synch interval can be reported and can be changed.  

configTime(k_GMT_int *3600, 0, ntpServer);
//configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
// change to dayligh savings time by changing GMT offset to -7 from -8

//struct tm timeinfo; - defined above 
if(!getLocalTime(&timeinfo)){
  Serial.println("Failed to obtain time");
  }
rtc.setTimeStruct(timeinfo); 
delay(500);

esp_task_wdt_reset();  // refresh watch dog timer
updateDate();
updateTime();
Serial.println();  // for testing
Serial.print(date_str);
Serial.print("  ");
Serial.println(time_str);
Serial.println("Alarm Keypad Remote is online.");
Serial.print(ssid);
Serial.print("  ");
rssi = WiFi.RSSI();
Serial.print(rssi);
Serial.println(" dBm");
Serial.println(); // add line feed
Serial.println("Type cmd in Blynk App for list of commands.");
Serial.println(); // add line feed

terminal.clear();
terminal.print(date_str);
terminal.print("  ");
terminal.println(time_str);
terminal.println("Alarm Keypad Remote is online.");
terminal.print(ssid);
terminal.print("  ");
terminal.print(rssi);
terminal.println(" dBm");
terminal.println();  // add line feed
esp_task_wdt_reset();  // refresh watch dog timer

terminal.flush();
Blynk.run();
Blynk.logEvent("alarm_keypad_restarted"); // log event to timeline

// for eeprom if first write
if (eepromFirstWriteFlag == 1){  // report new write through Blynk terminal
  terminal.println("EEPROM did not contain data.  Code defaults written to EEPROM.");
  terminal.println();
  EEPROMRead();  // print confirmation of data written and update variables
  }

terminal.println("Type cmd for list of commands.");
terminal.println(); // add line feed

// Send data to lcd
Blynk.virtualWrite(V16, "Waiting data 1");
Blynk.virtualWrite(V17, "Waiting data 2");
terminal.flush();
Blynk.run();

// for LCD buss sniffer reset
pinMode(LCDBSResetPin, OUTPUT);
digitalWrite(LCDBSResetPin, HIGH); // reset active low

//for Nano LEDS
pinMode(LED_BUILTIN, OUTPUT);
pinMode(LEDR, OUTPUT);
pinMode(LEDG, OUTPUT);
pinMode(LEDB, OUTPUT);
digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
digitalWrite(LEDR, HIGH); // These are active LOW
digitalWrite(LEDG, HIGH); 
digitalWrite(LEDB, HIGH); 


// for inputs (from panel leds) (not needed as now analog reads)
//pinMode(RTAPin, INPUT_PULLUP); // OC active low - need pullup
//pinMode(armedPin, INPUT_PULLUP); // OC active low - need pullup
//pinMode(troublePin, INPUT_PULLUP); // OC active low - need pullup
//pinMode(ACPin, INPUT_PULLUP); // OC active low - need pullup

// turn off all Blynk leds
ledKeyPress.off(); // these run as part of Blynk, no need for separate functions
ledWaitLCD.off();
ledBell.off();
ledRTA.off();
ledArmed.off();
ledTrouble.off();
ledAC.off();

thermistorValue = analogRead(thermistorPin);   

}  // end setup

void loop()
{
esp_task_wdt_reset();  // refresh watch dog timer
currentMillis = millis();
getDataLCD(); // read data from ESP32 when available, and set flag if complete string received

// check inputs every 0.25 second
if (currentMillis - previousMillis_updateInputs >= 250) { 
  previousMillis_updateInputs = currentMillis;  // Remember the time
  updateLCD();
  updateBlynkSwitches();  // update Blynk phone keypad buttons
  updatePanelInputs(); // RTA, Armed, Trouble, AC Sense from WT5500 panel leds - for future
  }

// check for Blynk terminal command every 0.5 second
if (currentMillis - previousMillis_Blynk >= 500) { 
  previousMillis_Blynk = currentMillis;  // Remember the time
  Blynk.run();
  updateBellState();  // bell state can be read from lcd panel
  if (strcmp(cmd_str, "cleared") != 0) {
    menu();
    strcpy(cmd_str, "cleared"); // after menu() runs, clear com_str to avoid repeated commands from terminal
    }
  }

// check if LCD should be turned off - this flag enables/disables LCD minute updates on Blynk
if ((currentMillis - lastCommandMillis) >= (timeoutLCD * 60000)) {
  onFlagLCD = 0;
  }
else {
  onFlagLCD = 1;
  }

// blink on board led every 1 second
if(currentMillis - previousMillis_blinkOnBoardLED >= 1000) {
  previousMillis_blinkOnBoardLED = currentMillis;  // Remember the time
  blinkOnBoardLED();
  }

// Thermistor updated every 5 seconds
if (currentMillis - previousMillis_updateTherm >= 5000) { 
  previousMillis_updateTherm = currentMillis;  // Remember the time
  readThermistor();
  }

// error routine after 5 seconds
if((nano_reset_flag_int == 1) && (currentMillis - previousMillis_nanoReset >= 5000)) {  
  nanoReset();
  }

}  
// end of loop

void updateBlynkSwitches()
{ 
// use for WT5500 keyboard 
// MC/AD (usually for disarm) - enter master code using terminal virtual keypad
if (keyValueDisarm == 1) {
  lastCommandMillis = currentMillis; // update LCD display time out on terminal or keyboard command
  ledKeyPress.on(); // turn keypad led on
  int masterCode_length_int = strlen(masterCode);
  for (int i = 0; i < masterCode_length_int; i++){
    char *e;
    e = strchr(keyChart_str, masterCode[i]);
    int keyPos = (int)(e - keyChart_str);
    if (e) {// valid character found
      // below presses panel keys for character k sent over by terminal
      terminalKeyPress(keyPos);  // output char string to keypad
      delay500ms();
      }
    }  
  ledKeyPress.off();
  strcpy(notification_str, "MC/AD was pressed.");
  terminal_output();
  Blynk.run();
  // delay until key released to prevent multiple entries
  // need to add blynk.run as removed from delay25ms
  while (keyValueDisarm == 1) {delay200ms(); Blynk.run(); } 
  }

// for reading from Blynk keypad
// keyValue[20] for key values read - 19 keys plus a 0 for no key press
// keySelectValues[20][2] for row and column outputs if key value read was 1
// update keypad buttons
for (int k = 0; k <= 19; k++){
  if (keyValue[k] == 1) { // pressed key k found
    lastCommandMillis = currentMillis; // update LCD display time out on terminal or keyboard command
    ledKeyPress.on(); // turn keypad led on
    if (k == 10 || k == 12 || k == 13 || k == 14) {ledWaitLCD.on();} // *#<> turn on wait for lcd
    Blynk.run();
    // set rows and columns using bitRead(x, n bit)
    int sel0 = bitRead(keySelectValues[k][0], 0);
    digitalWrite(RS0Pin, sel0); 
    int sel1 = bitRead(keySelectValues[k][0], 1);
    digitalWrite(RS1Pin, sel1); 
    int sel2 = bitRead(keySelectValues[k][0], 2);
    digitalWrite(RS2Pin, sel2); 
    int sel3 = bitRead(keySelectValues[k][1], 0);
    digitalWrite(CS0Pin, sel3); 
    int sel4 = bitRead(keySelectValues[k][1], 1);
    digitalWrite(CS1Pin, sel4);
    int sel5 = bitRead(keySelectValues[k][1], 2);
    digitalWrite(CS2Pin, sel5);
    delay(2);
    // for testing
    char kstr[8]; //find letter from sting based on keypos k
    kstr[0] = '\''; // add single quotes to output
    kstr[1] = keyChart_str[k]; 
    kstr[2] = '\'';
    kstr[3] = '\0';
    strcpy(notification_str, kstr); 
    strcat(notification_str, " was pressed.");
    terminal_output();
    if (k == 15){
      strcpy(notification_str, "Alarm armed in Stay mode."); 
      terminal_output();
      }
    if (k == 16){
      strcpy(notification_str, "Alarm armed in Away mode."); 
      terminal_output();
      }
    //terminal.println();
    //terminal.print(" ");
    //terminal.print(sel2);
    //terminal.print(sel1);
    //terminal.print(sel0);
    //terminal.print(" ");
    //terminal.print(sel5);
    //terminal.print(sel4);
    //terminal.println(sel3);
    terminal.flush();
    Blynk.run();
    // enable
    digitalWrite(RENPin, LOW); 
    digitalWrite(CENPin, LOW);
    delay500ms();
    if ((k >= 15) && (k <= 19)){delay1s();}  // longer key press for these
    ledKeyPress.off();
    Blynk.run();
    // does not work
    // delay until key released to prevent multiple entries
    // need to add blynk.run as removed from delay25ms
    while (keyValue[k] == 1) {delay200ms(); Blynk.run(); } 
    digitalWrite(RENPin, HIGH); 
    digitalWrite(CENPin, HIGH);
        /* for testing
    Serial.print(k);
    Serial.print(" ");
    Serial.print(sel2);
    Serial.print(sel1);
    Serial.print(sel0);
    Serial.print(sel5);
    Serial.print(sel4);
    Serial.println(sel3);
    */
    } 
  }
}

// presses panel keys for character k sent over by terminal
void terminalKeyPress(int k)
{
// use for terminal "key" presses
// set rows and columns using bitRead(x, n bit)
ledKeyPress.on(); // turn keypad led on
if (k == 10 || k == 12 || k == 13 || k == 14) {ledWaitLCD.on();} // *#<> turn on wait for lcd
Blynk.run();
int sel0 = bitRead(keySelectValues[k][0], 0);
digitalWrite(RS0Pin, sel0); 
int sel1 = bitRead(keySelectValues[k][0], 1);
digitalWrite(RS1Pin, sel1); 
int sel2 = bitRead(keySelectValues[k][0], 2);
digitalWrite(RS2Pin, sel2); 
int sel3 = bitRead(keySelectValues[k][1], 0);
digitalWrite(CS0Pin, sel3); 
int sel4 = bitRead(keySelectValues[k][1], 1);
digitalWrite(CS1Pin, sel4);
int sel5 = bitRead(keySelectValues[k][1], 2);
digitalWrite(CS2Pin, sel5);
delay(2);
// for testing
char kstr[8];
kstr[0] = '\''; // find letter from sting based on keypos k
kstr[1] = keyChart_str[k]; 
kstr[2] = '\'';
kstr[3] = '\0';
strcpy(notification_str, kstr); 
strcat(notification_str, " was pressed.");
terminal_output();
//terminal.print(keyChart_str[k]); // find letter from sting based on keypos k
//terminal.println("  was pressed");
//terminal.println();
//terminal.print(" ");
//terminal.print(sel2);
//terminal.print(sel1);
////terminal.print(sel0);
//terminal.print(" ");
//terminal.print(sel5);
//terminal.print(sel4);
//terminal.println(sel3);
terminal.flush();
Blynk.run();
// enable
digitalWrite(RENPin, LOW); 
digitalWrite(CENPin, LOW);
// delay
delay500ms();
if ((k >= 15) && (k <= 19)){delay1s();}  // longer key press for these
ledKeyPress.off();
Blynk.run();
// does not work
// delay until key released to prevent multiple entries
// need to add blynk.run as removed from delay25ms
////while (keyValue[k] == 1) {delay200ms(); Blynk.run(); } 
digitalWrite(RENPin, HIGH); 
digitalWrite(CENPin, HIGH);
/* for testing
Serial.print(k);
Serial.print(" ");
Serial.print(sel2);
Serial.print(sel1);
Serial.print(sel0);
Serial.print(sel5);
Serial.print(sel4);
Serial.println(sel3);
*/
} 

void updateBellState()  // bell state can be read from lcd panel
{
if (alarmFlagLCD == 1){   // standard alarm bell - toggled on/off in LCDCheck function
  bellLEDValue = 1;
  if (bellLEDValue != display_bellLEDValue){ // prevents repeated Blynk writes
    display_bellLEDValue = bellLEDValue;
    ledBell.on();
    // send notification
    strcpy(notification_str, "Alarm Bell on keyboard.");
    // now having jpwolfe@gmail.com filter and forward emails from Blynk to jpwolfe31@yahoo.com and 
    // 5336alarm.  5336alarm is then set to notify for all messages received.
    Blynk.logEvent("alarm_bell", String(notification_str)); 
    //display_bellFireLEDValue = 0;  // turn off fire bell
    // ledFireBell.off();
    terminal_output();  // writes notificaiton_str to terminal with date stamp
    }
  }
if (alarmFlagLCD == 0){   // standard alarm bell off - toggled on/off in LCDCheck function
  bellLEDValue = 0;
  if (bellLEDValue != display_bellLEDValue){ // prevents repeated Blynk writes
    display_bellLEDValue = bellLEDValue;
    ledBell.off();
    // send notification
    strcpy(notification_str, "Alarm Disarmed on keyboard.");
    Blynk.logEvent("alarm_disarmed", String(notification_str)); 
    //display_bellFireLEDValue = 0;  // turn off fire bell
    // ledFireBell.off();
    terminal_output();  // writes notificaiton_str to terminal with date stamp
    }
  }
}

void updatePanelInputs()  // RTA, Armed, Trouble and AC Sense 
{
// RTA
RTALEDValue = analogRead(RTAPin);  // 870 is 0.7V  - set threshold at 400
if (RTALEDValue >= 400) {
  RTALEDValue = 1;
  if (RTALEDValue != display_RTALEDValue){ // prevents repeated Blynk writes
    display_RTALEDValue = RTALEDValue;
    ledRTA.on();
    }
  }
else {
  RTALEDValue = 0;
  if (RTALEDValue != display_RTALEDValue){ // prevents repeated Blynk writes
    display_RTALEDValue = RTALEDValue;
    ledRTA.off();
    }
  }
// Armed
armedLEDValue = analogRead(armedPin);  // 870 is 0.7V  - set threshold at 400
if (armedLEDValue >= 400) {
  armedLEDValue = 1;
  if (armedLEDValue != display_armedLEDValue){ // prevents repeated Blynk writes
    display_armedLEDValue = armedLEDValue;
    strcpy(notification_str, "Alarm Armed on keyboard.");
    Blynk.logEvent("alarm_armed", String(notification_str)); ledArmed.on();
    }
  }
else {
  armedLEDValue = 0;
  if (armedLEDValue != display_armedLEDValue){ // prevents repeated Blynk writes
    display_armedLEDValue = armedLEDValue;
    ledArmed.off();
    strcpy(notification_str, "Alarm Disarmed on keyboard.");
    Blynk.logEvent("alarm_disarmed", String(notification_str)); 
    }
  }
// Trouble
troubleLEDValue = analogRead(troublePin);  // 870 is 0.7V  - set threshold at 400
if (troubleLEDValue >= 400) {
  troubleLEDValue = 1;
  if (troubleLEDValue != display_troubleLEDValue){ // prevents repeated Blynk writes
    display_troubleLEDValue = troubleLEDValue;
    ledTrouble.on();
    }
  }
else {
  troubleLEDValue = 0;
  if (troubleLEDValue != display_troubleLEDValue){ // prevents repeated Blynk writes
    display_troubleLEDValue = troubleLEDValue;
    ledTrouble.off();
    }
  }
// AC - note - this is not "AC Sense" that reads a separate pre backup power 
//  supply value in other black box for Alarm app
ACLEDValue = analogRead(ACPin);  // 870 is 0.7V  - set threshold at 400
if (ACLEDValue >= 400) {
  ACLEDValue = 1;
  if (ACLEDValue != display_ACLEDValue){ // prevents repeated Blynk writes
    display_ACLEDValue = ACLEDValue;
    ledAC.on();
    }
  }
else {
  ACLEDValue = 0;
  if (ACLEDValue != display_ACLEDValue){ // prevents repeated Blynk writes
    display_ACLEDValue = ACLEDValue;
    ledAC.off();
    }
  }
}

void terminal_output()
{
updateTime();
terminal.print(time_str);
terminal.print("  ");
terminal.println(notification_str);
}

void printLocalTime() // prints local time to Blynk terminal
{
// struct tm timeinfo; - defined globally above  
//getLocalTime(&timeinfo);
//terminal.print("ntp ");
//terminal.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");  // see strftime for formating options
// rtc loaded above in setup ??
// terminal.print("nano rtc ");
terminal.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));   // (String) returns time with specified format 
terminal.flush();
Blynk.run();
return;
}

void nanoError()
{
  // do not log new errors if one has been reported 
  //   and now in prcess of logging and resetting
  if (nano_reset_flag_int == 1) {return;}
  // general case errors
  strcpy(error_type_display_str, error_type_str);
  strcat(error_type_display_str, " Error");
  // special case error
  if (strcmp(error_type_str, "BT") == 0) {
    strcpy(error_type_display_str, "BT restart");
    }  
  if (strcmp(error_type_str, "ST") == 0) {
    strcpy(error_type_display_str, "ST restart");
    }  
  // set flag for error reporting and shutdown
  nano_reset_flag_int = 1;
}

// processor software reset 
void nanoReset()  // runs at end of 5 second nanoReset timer
{
  // send notification
  strcpy(notification_str, error_type_display_str);
  Blynk.logEvent("alarm_restarted", String(notification_str)); 
  // above log restart in timeline nnnnn
  Serial.println(notification_str);
  ESP.restart (); 
}

void updateRTC()
{
configTime(k_GMT_int *3600, 0, ntpServer);
//configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
// change to dayligh savings time by changing GMT offset to -7 from -8
//struct tm timeinfo; - defined above 
if(!getLocalTime(&timeinfo)){
  terminal.println("Failed to obtain time");
  }
rtc.setTimeStruct(timeinfo); 
// printLocalTime();
return;
}

void updateDate()
{
  // get date data
  years_int = rtc.getYear(); // 4 digits 2021
  //sprintf(years_str, "%4d", years_int); 
  sprintf(years_str, "%2d", years_int);  // 2 digits 21 ?
  months_int = rtc.getMonth();  // returns 0-11
  months_int++;
  sprintf(months_str, "%02d", months_int);
  // above converts to 2 character decimal base - pads leading 0s by adding the 0
  days_int = rtc.getDay(); // returns 1-31
  sprintf(days_str, "%02d", days_int);
  strcpy(date_str, years_str);
  strcat(date_str, "-");
  strcat(date_str, months_str);
  strcat(date_str, "-");
  strcat(date_str, days_str);
}  

void updateTime()
{
  //get time data
  hour_int = rtc.getHour(true); // true is 24 hour time 0-23
  sprintf(hour_str, "%02d", hour_int); 
  // above converts to 2 character decimal base - pads leading 0s by adding the 0
  minute_int = rtc.getMinute(); // 0-59
  sprintf(minute_str, "%02d", minute_int); 
  second_int = rtc.getSecond();
  sprintf(second_str, "%02d", second_int); 
  strcpy(time_str, hour_str);
  strcat(time_str, ":");
  strcat(time_str, minute_str);
  strcat(time_str, ":");
  strcat(time_str, second_str);
}  

void EEPROMWrite()
{
// note - function changing these variables must also update
//   the working variables - e.g. k_GMT_int and k_GMT_str
// store signature first
flash.eeprom_signature = WRITTEN_SIGNATURE;
strcpy(flash.eeprom_ssid, ssid);
strcpy(flash.eeprom_pass, pass);
strcpy(flash.eeprom_k_GMT, k_GMT_str);
EEPROM.put(storedAddress, flash);
EEPROM.commit();
EEPROMRead();
}

// read eeprom
void EEPROMRead()
{
EEPROM.get(storedAddress, signature);
// If the EEPROM is written, then there is a written signature
if (signature == WRITTEN_SIGNATURE){
  EEPROM.get(storedAddress, flash);
  // Print a confirmation of the EEPROM data
  terminal.println("EEPROM data:  ");
  terminal.print("ssid ");
  terminal.println(flash.eeprom_ssid); 
  terminal.print("pass: ");  
  terminal.println(flash.eeprom_pass);  
  terminal.print("k_GMT: ");
  terminal.println(flash.eeprom_k_GMT); 
  terminal.println();
  // convert eeprom data to strings and numbers used in the program
  strcpy(ssid, flash.eeprom_ssid);
  strcpy(pass, flash.eeprom_pass);
  strcpy(k_GMT_str, flash.eeprom_k_GMT);
  k_GMT_int = atoi(flash.eeprom_k_GMT);
  }
else { // eeprom is not written and needs to be written
  terminal.println("EEPROM does not contain data.");
  }
terminal.flush();
Blynk.run();
return;
}  

void blinkOnBoardLED() 
{
if (onBoardLEDValue == 1) {
  digitalWrite(LED_BUILTIN, LOW);
  onBoardLEDValue = 0;
  }
else {
  digitalWrite(LED_BUILTIN, HIGH);
  onBoardLEDValue = 1;
  }
}

void readThermistor()
{
// default analog read resolution is 12 bits 0-4096
thermistorValue = analogRead(thermistorPin); 
// 1600 = 110deg, 1700 = 100deg , 1800 = 90deg, 1900 = 80deg  approx
thermistorTemp = 80 + ((1900 - thermistorValue) / 10); // approx
if (thermistorReportFlag == 1){
  terminal.print(thermistorValue); // for testing
  terminal.print("  ");
  terminal.print(thermistorTemp);
  terminal.println(" deg");
  terminal.flush();  // output to terminal immediately
  Blynk.run(); 
  }
}

void updateLCD() // stops running after 3 mintues to abvoid excessive Blynk writes
{
  if (dataLCDFlag == 1){ // output lastest display string recieved
    //strcpy(displayLines, dataLCDStr); // seperate lines
    //displayLines[32] = '\0';
    strcpy(displayLine1In, dataLCDStr);  // seperate lines -  be careful with str lengths copied
    displayLine1In[16] = '\0';
    //Serial.println(displayLine1In);
    strcpy(displayLine2In, (dataLCDStr + 16));
    displayLine2In[16] = '\0';
    //Serial.println(displayLine2In);
    // check lines against correct lines possible
    LCDCheck();
    // only update LCD for short period after command received on terminal or keyboard
    if (onFlagLCD == 1){ 
      Blynk.virtualWrite(V16, displayLine1Out);
      Blynk.virtualWrite(V17, displayLine2Out);  // note - terminal below uses V18
      ledWaitLCD.off(); // turn off LCD wait led when  data received for *#<> keys
      Blynk.run();
      }
    // for testing
    //Serial.println(displayLine1In);
    //Serial.println(displayLine1Out);
    //Serial.println(displayLine2In);
    //Serial.println(displayLine2Out);
    //terminal.println(displayLines);
    //terminal.println(displayLine1);
    //terminal.println(displayLine2);
    //terminal.flush();
    //Blynk.run();//
    dataLCDFlag = 0;  // reset flag to receive more data
    }
}

void getDataLCD() // non-blocking serial input routine
{
while (SerialLCD.available() && (dataLCDFlag == 0)) {
  char dataLCD = SerialLCD.read();
  //Serial.println(dataLCD);
  //Serial.println(dataLCD, HEX);
  if (dataLCD == '\r' || dataLCD == '\n') {
    dataLCDStr[dataLCDStrLength] = '\0';
    if (dataLCDStrLength) dataLCDFlag = 1;  // set flag when full string received
    // data received so reset command length
    dataLCDStrLength = 0;
    //Serial.println("dataLCD received back");
    }
  else {
    dataLCDStr[dataLCDStrLength] = dataLCD;
    dataLCDStrLength ++;
    }
  }
}  

// delays that maintain on board led blinking  
//   and WDT clearing
void delay25ms(){ 
  currentMillis = millis();
  if(currentMillis - previousMillis_blinkOnBoardLED >= 1000) {
    previousMillis_blinkOnBoardLED = currentMillis;  // Remember the time
    blinkOnBoardLED();
    } 
  delay(25);
  esp_task_wdt_reset();
  }

void delay30ms(){delay(5); delay25ms();}
void delay50ms(){delay(25); delay25ms();}
void delay75ms(){delay(50); delay25ms();}
void delay100ms(){delay(75); delay25ms();}
void delay150ms(){delay(100); delay50ms();}
void delay200ms(){delay(175); delay25ms();}
void delay250ms(){delay(225); delay25ms();}
void delay300ms(){delay(275); delay25ms();}
void delay350ms(){delay(325); delay25ms();}
void delay400ms(){delay(375); delay25ms();}
void delay450ms(){delay(425); delay25ms();}
void delay500ms(){delay250ms(); delay250ms();}
void delay600ms(){delay300ms(); delay300ms();}
void delay700ms(){delay350ms(); delay350ms();}
void delay750ms(){delay500ms(); delay250ms();}
void delay800ms(){delay400ms(); delay400ms();}
void delay900ms(){delay450ms(); delay450ms();}
void delay1s(){delay500ms(); delay500ms();}
void delay2s(){delay1s(); delay1s();}
void delay3s(){delay2s(); delay1s();}
void delay4s(){delay2s(); delay2s();}
void delay5s(){delay3s(); delay2s();}
void delay6s(){delay4s(); delay2s();}
void delay7s(){delay4s(); delay3s();}
void delay8s(){delay4s(); delay4s();}
void delay9s(){delay5s(); delay4s();}
void delay10s(){delay5s(); delay5s();}

/*
In the IDE Tools menu, enter Pin Numbering and choose By GPIO number (legacy);
Make sure the sketch always uses labels to refer to pins. If you used the number 2, 
replace this with the symbol D2 everywhere.  This will switch to a more library-compatible 
scheme and avoid the above confusion.  Do not include GPIO in the number. 
See pin table below.

Nano	ESP32
D0	GPIO44
D1	GPIO43
D2	GPIO5  *** reported that this esp32 pin 5 may conflict with use of eeprom - have not seen this to date.
D3	GPIO6
D4	GPIO7
D5	GPIO8
D6	GPIO9  *** reported do not want to use gpi0 6-11 as also used for integrated flash?? eeprom?? - have not seen this to date. 
D7	GPIO10
D8	GPIO17
D9	GPIO18
D10	GPIO21
D11	GPIO38
D12	GPIO47
D13	GPIO48  also built in led and SPI clock and used with Blynk for link indication
A0	GPIO1
A1	GPIO2
A2	GPIO3
A3	GPIO4
A4	GPIO11
A5	GPIO12
A6	GPIO13
A7	GPIO14
BOOT0	GPIO46  also Red on rgb led 
BOOT1	GPIO0   also Green on rgb led

GPIO 45 is not on board pinout but is Blue on rgb 
Note - some early boards (not mine) have different rgb colors

w5500 Ethernet module uses 5, 16-19 and 23, so could be a problem with this board as 16 and 19 and 23 have no outputs.
use uno and ethernet board instead?

Impassa commands
Keyboard Commands

  * Menu
  # Exit Menu
  <> Scroll

* 1 Bypass 
      99 recall last group bupasses
      00 clear all bypasses
      95 save bypasses
      91 recall bypasses
* 1 In Stay, arm interior
* 2 Display trouble conditions
* 3 Display alarm memory
* 4 Toggle Chime on/off
* 5 MC User code programming
* 6 User functions
      Time date (ns)
      System test
* 7 Command outputs (ns)
* 8 IC Installer programming 
      Siren off 804 312 toggle off 1 6
* 9 MC No entry arming - stay state
* 0 If disarmed, quick arm
* 0 If armed, quick exit

(ns) Not Supported

*/

