/* Author: Joseph Post
   Purpose: Transmit data via nRF2401 module.
 - CONNECTIONS: nRF24L01 Modules See:
 http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
   1 - GND
   2 - VCC 3.3V !!! NOT 5V
   3 - CE to Arduino pin 9
   4 - CSN to Arduino pin 10
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - UNUSED

/*-----( Import needed libraries )-----*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <dht11.h>
/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN   9
#define CSN_PIN 10

// NOTE: the "LL" at the end of the constant is "LongLong" type
const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe
/*-----( Declare objects )-----*/
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
dht11 DHT11;
/*-----( Declare Variables )-----*/
const uint64_t pipes[2] = { 0xe7e7e7e7e7LL, 0xc2c2c2c2c2LL }; //pipe addresses
int joystick[3] = {0, 0, 0};  // 3 element array with value, start time, end time
void setup()   /****** SETUP: RUNS ONCE ******/
{
  radio.begin();
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  DHT11.attach(2);
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
radio.startListening();
while(!radio.available())
{
  __asm__("nop\n\t"); 
}
radio.read( &joystick, sizeof(joystick) );

if(joystick[0] == 99) //receive packetRequest variable?
{
  radio.stopListening();
  joystick[0] = DHT11.read();
  joystick[1] = DHT11.temperature; //update the variable incomingAudio with new value from A0 (between 0 and 255)
  joystick[2] = DHT11.humidity;
  radio.write( &joystick, sizeof(joystick));
}
else //packetRequest got corrupted
{
  radio.stopListening();
  joystick[0] = -3; //arbitrary failure to get data value
  radio.write( &joystick, sizeof(joystick));
} 
}
  




