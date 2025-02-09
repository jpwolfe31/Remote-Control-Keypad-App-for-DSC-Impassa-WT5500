# Remote-Control-Keypad-App-for-DSC-Impassa-WT5500

This Arduino Nano-ESP32 based application allows the user to control the keypad of a WT5500 used in a DSC Impassa Home Alarm System with the user’s iPhone.  Primary functions include, arming, disarming and alarm system monitoring.

A key on the keypad is activated by connecting a row and a column together using two 74LV4051 switch multiplexers (one for the rows and the other for the columns).  Wires are attached to each row and column switch on the WT5500 board (see file photos for connection points).

Inputs to the WT5500 LCD display are sniffed to provide a corresponding LCD display on the iPhone.  These inputs are sensed using the test points on the LCD display board. 

LEDs on the WT500 are also monitored and displayed on the iPhone.

Battery power is bypassed and simulated using a LM317 regulator set at 3.0V (R 240 and R 330 for 2.95V) attached to two of the battery terminals (the four 1.5V batteries for the WT5500 supply 3.0V using two pairs in parallel).

The iPhone uses the Blynk app to send and receive messages to the Nano-ESP32.  The Nano-ESP32 sends alerts and emails to the iPhone through the Blynk app in the event of an alarm.  A separate NANO-ESP32 has the sole responsibility of sniffing and decoding the LCD inputs and then sends that data to the first NANO-ESP32.  Interrupts on the NANO-ESP32 appear to have a 2us interrupt latency and are too slow to capture the inputs to the LCD that change states in less than 1us (see file photos for screen shots from the Blynk app).

Unfortunately, Blynk is not supporting new makers on its app right now, but I believe this program could be modified to work with the Aurduino Cloud IOT and its messenger, button and led widgets.

Happy to answer any questions.
