// 16x2 LCD 4 bit bus sniffer for nano esp32 and Impassa WT5500 LCD

/*
In the IDE Tools menu, enter Pin Numbering and choose By GPIO number (legacy);
Make sure the sketch always uses labels to refer to pins. If you used the number 2, 
replace this with the symbol D2 everywhere.  This will switch to a more library-compatible 
scheme and avoid the above confusion.  Do not include GPIO in the number. 
*/

// for GPIO reads
#include "soc/gpio_struct.h" // to read all GPIOs at once in 75ns

// for second serial
#include "HardwareSerial.h"
HardwareSerial SerialLCD(2); // use Uart2 - no pins assigned by default
const int SerialLCD_RX = A6;
const int SerialLCD_TX = A7;

int RSPin = D8; 
int RWPin = D7;
int EPin = D6;
int data7Pin = D5;  // aka BD3
int data6Pin = D4;  // aka BD2
int data5Pin = D3;  // aka BD1
int data4Pin = D2;  // aka BD0

int allIn;
int RS;
int RW;
int E;

uint8_t msnArray[200];
uint8_t lsnArray[200];
char charArray[200];
char displayLines[200] = "";
char displayLine1[200] = "";
char displayLine2[200] = "";
int arrayCnt = 0;

void setup() 
{
Serial.begin(115200); // for serial monitor - USB Uart0
//while (!Serial){}; // wait for serial port to connect
delay(500);
SerialLCD.begin(115200, SERIAL_8N1, SerialLCD_RX, SerialLCD_TX);  // Initialize SerialLCD (RX: A6, TX: A7) Uart2
// Note - cross wires between the two ESP32s.
delay(500);

// pull downs seem to work slightly better than ups or none
pinMode(RSPin, INPUT_PULLDOWN); // digital active high
pinMode(RWPin, INPUT_PULLDOWN); // digital active high
pinMode(EPin, INPUT_PULLDOWN); // digital active high
pinMode(data7Pin, INPUT_PULLDOWN); // digital active high
pinMode(data6Pin, INPUT_PULLDOWN); // digital active high
pinMode(data5Pin, INPUT_PULLDOWN); // digital active high
pinMode(data4Pin, INPUT_PULLDOWN); // digital active high

Serial.println("LCD sniffer has started");

pinMode(LED_BUILTIN, OUTPUT);
digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on by making the voltage high (D13)
}
// end of set up

void loop()
{
// reset counter for new read of 32 bytes
arrayCnt = 0;  
// wait for panel's command write of 0x80 that we use to sync reads of the panel's following data writes to the bus
waitSync();
while (arrayCnt <= 32){readBus();} // read 32 chars from the data writes
strcpy(displayLines, charArray);
displayLines[32] = '\0'; // send both lines as a single SerialLCD.println to the other ESP32
SerialLCD.println(displayLines); // println as \n is terminator for protocal used here
// for testing
Serial.println(displayLines);  // for testing on this ESP32's COM port
delay(50); // note - this provides correction of sync issues as any short read gets flushed out
// it appears messages are often sent up to 3 times for each screen refresh
// note - 25 to 50ms seems to work best with 50 having an edge
return;  // arrayCnt resets to 0
}
// end of loop

void readBus()
{
/* 
check if RS is low
then check if RS is high
then, at end of enable high, store data as ms nibble
end of second enable high, store most recent data as ls nibble
create char and store in char array for output
note - interupts do not seem to work with nano-esp32 as interupt latency tests were from 
2-100us and to catch E after RS high this needs to be less than 0.7us.
*/
// state 0 wait for RS low
do {
  allIn = REG_READ(GPIO_IN_REG);
  RS = (allIn >> 17) & 0x01;  // D8
  } while (RS == 1);

// state 1 wait for RS high
do {
  allIn = REG_READ(GPIO_IN_REG);
  RS = (allIn >> 17) & 0x01;  // D8
  } while (RS == 0);

// state 2 wait for E high
do { 
  allIn = REG_READ(GPIO_IN_REG);
  E = (allIn >> 9) & 0x01;  // D6
  } while (E == 0);

// state 3 read data while waiting for E low, then store most recent ms nibble
// per spec, E pulse width is 300ns min, data setup before E low is 60ns min
do {
  allIn = REG_READ(GPIO_IN_REG);
  E = (allIn >> 9) & 0x01;  // D6
  } while (E == 1);

allIn = REG_READ(GPIO_IN_REG);
msnArray[arrayCnt] = (allIn >> 1) & 0xf0; // capture data right after high

// state 4 wait for E high
do {
  allIn = REG_READ(GPIO_IN_REG);
  E = (allIn >> 9) & 0x01;  // D6
} while (E == 0);

// state 5 read data while waiting for E low, then then create byte with most recent ls nibble 
// and set RS state
do {
  allIn = REG_READ(GPIO_IN_REG);
  E = (allIn >> 9) & 0x01;  // D6
  } while (E == 1);
allIn = REG_READ(GPIO_IN_REG);
lsnArray[arrayCnt] = (allIn >> 5) & 0x0f; // capture data right after high
// merge lsn with msn to get character
// data stored in arrays for later serial print
charArray[arrayCnt] = msnArray[arrayCnt] | lsnArray[arrayCnt];
arrayCnt++;
return;
}

void waitSync() {
// wait for address 00 set command write of 0x80
while(1) // note could change to wait for a certain period of time max
{
// state 1 wait for RS low, RW low and E high
do {
  allIn = REG_READ(GPIO_IN_REG);
  E = (allIn >> 9) & 0x01;
  RW = (allIn >> 10) & 0x01;
  RS = (allIn >> 17) & 0x01;
  } while ( (E == 1) && (RW == 0) && (RS == 0) );

char msn80 = (allIn >> 1) & 0xf0; // capture data right after E low

// state 2 wait for E low
do {
  allIn = REG_READ(GPIO_IN_REG);
  E = (allIn >> 9) & 0x01;
} while (E == 1);

// state 3 wait for E high
do {
  allIn = REG_READ(GPIO_IN_REG);
  E = (allIn >> 9) & 0x01;
} while (E == 0);

// state 4 wait for E low
do {
  allIn = REG_READ(GPIO_IN_REG);
  E = (allIn >> 9) & 0x01;  // D6
  } while (E == 1);
char lsn80 = (allIn >> 5) & 0x0f; // capture data right after E low 
// merge lsn with msn to get character
char full80 = msn80 | lsn80;
if (full80 == 0x80) {break; return;}
}
}
// end of waitSync

/*

Map of LCD board test pins 1-18 to 12 pin connector for LCD Bus Sniffer on Alarm Keyboard PC Board

Test Pin          Connector Pin
1 LCD A           1 - not connected or used
2 LCD K           2 - not connected  or used
3 LCD V0          3 - not connected or used
4 GND             4 GND
5 3.0V            5 -  not connected or used
6 RW              6 RW
7 RS              7 RS
8 E               8 E
9 D4              9 D4  aka BD0
10 D5             10 D5  aka BD1
11 D6             11 D6  aka BD2
12 D7             12 D7  aka BD3

13-18 - no signals detected
*/

/*
// for REG_READ
D2	GPIO5 ls bit nibble  shift 5
D3	GPIO6
D4	GPIO7
D5	GPIO8 ms bit nibble shift 1
D6  GPIO9 E shift 9
D7	GPI010 RS shift 10
D8	GPIO17 RW shift 17
D9  GPIO18 shift 18?? not tested
D10 GPIO21 shift 21?? not tested
D11 GPIO38 shift 38?? not tested
D12 GPIO47 shift 47?? not tested  
D13 GPIO48 shift 48?? not tested 
Shift is unknown or not tested for GPIO18 and above.  
Per technical manual page 506, GPIO 0-31 are in GPIO_IN_REG and GPIO 32-48 are in GPIO_IN1_REG.
However, these may be combined into one value in software so all 0-48 can be read as above?

Below not used here now, but tested and for future ref on shifting
A0	GPIO1 RW  shift 1
A1	GPIO2 RS  shift 2
A2	GPIO3 EN  shift 3
A3	GPIO4  not used
A4	GPIO11 ls bit nibble  shift 11
A5	GPIO12
A6	GPIO13
A7	GPIO14 ms bit nibble shift 4

In the IDE Tools menu, enter Pin Numbering and choose By GPIO number (legacy);
Make sure the sketch always uses labels to refer to pins. If you used the number 2, 
replace this with the symbol D2 everywhere.  This will switch to a more library-compatible 
scheme and avoid the above confusion.  Do not include GPIO in the number. 
See pin table below.

Nano	ESP32
D0	GPIO44
D1	GPIO43
D2	GPIO5  **** is this esp32 pin 5 ss that conflicts with use of eeprom???  however, this is used with i2c now - probapbly not
D3	GPIO6
D4	GPIO7
D5	GPIO8
D6	GPIO9  *** do not want to use gpi0 6-11 as also used for integrated flash?? eeprom??? **** 
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
*/

