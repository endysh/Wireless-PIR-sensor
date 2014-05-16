/*
  Wireless PIR sensor firmware
  revision 1.0
 
  Copyright (c) 2014, Andrey Shigapov, All rights reserved
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  http://www.gnu.org/licenses/gpl.txt
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
  
*/

/////////////////////////////////////////////////////////////////////////////////////////////
// This program continiosly reads PIR module output and sends a single byte command with ASCII value 'o' 
// to the preconfigured node when motion is detected, then waits for 5 sec before it starts monitoring PIR sensor output again.
//
// This is is just a hardware test POC program
// No power interrupts or other saving techniques is used in this version, thus the current power consumption makes it not very suitable for 
// operation from a battery 
/////////////////////////////////////////////////////////////////////////////////////////////
 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
 

int led = A4;         // the pin that the LED is attached to
int buzzer = 5;       // the pin that the Buzzer is attached to
int PIR_enable = 6;   // the pin that controls PIR output
int PIR_input = 2;    // the pin that connected to PIT output

int ledRx = led;
int ledTx = led;


// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xE7EEE7E700LL, 0xE7EEE7E700LL };

char dataToSend = 'z';
char dataRecv = 'z';

void openPipes()
{
    uint64_t txPipe = pipes[0];
    uint64_t rxPipe = pipes[1];

    radio.openWritingPipe(txPipe);
    radio.openReadingPipe(1, rxPipe);
}


// the setup routine runs once when you press reset:
void setup()  { 
  // declare pin 9 to be an output:
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(PIR_input, INPUT);
  pinMode(PIR_enable, OUTPUT);
  digitalWrite(PIR_enable, 1);
 
  
  //
  // Setup and configure rf radio
  //
  radio.begin();

  // set output level
  radio.setPALevel(RF24_PA_MIN);
  
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 5);
  radio.setAutoAck(true);
  
  // set CRC to 8 bit
  radio.setCRCLength(RF24_CRC_8);
  
  // set data rate to 250 Kbps
  radio.setDataRate(RF24_250KBPS);
  
  // set channel 15
  radio.setChannel(15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(1);

  //
  // Dump the configuration of the rf unit for debugging
  //
  radio.printDetails();
  
  //
  // Open pipes to other nodes for communication
  //
  openPipes();
  
} 

// the loop routine runs over and over again forever:
void loop()  { 
  
  if(1 == digitalRead(PIR_input)) {
    digitalWrite(led, 1);
    delay(100);
    digitalWrite(led, 0);
    
    dataToSend = 'o';
    dataRecv = 'z';
  }
  
  if (dataToSend != dataRecv)  {
    // First, stop listening so we can talk.
    radio.stopListening();
    delay(10);

    // Take the time, and send it.  This will block until complete
    digitalWrite(ledTx, HIGH);   
    delay(50);
    bool ok = radio.write( &dataToSend, sizeof(dataToSend) );
    digitalWrite(ledTx, LOW);
    
    // Now, continue listening
    unsigned long started_waiting_at = millis();
    delay(10);
    radio.startListening();
   
    if(ok) {
      delay(5000);
      
    } else {
      // blink LED several times in case of transmission error
      digitalWrite(ledTx, HIGH);
      delay(50);
      digitalWrite(ledTx, LOW);
      delay(30);
      digitalWrite(ledTx, HIGH);
      delay(50);
      digitalWrite(ledTx, LOW);
      delay(30);
      digitalWrite(ledTx, HIGH);
      delay(50);
      digitalWrite(ledTx, LOW);
      
      delay(5000);
    }
    
    dataRecv = dataToSend;
  }
  delay(5); 
}

