/* YourDuinoStarter Example: nRF24L01 Receive Joystick values

 - WHAT IT DOES: Receives data from another transceiver with
   2 Analog values from a Joystick or 2 Potentiometers
   Displays received values on Serial Monitor
 - SEE the comments after "//" on each line below
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
   
   The LCD circuit:
 * LCD RS pin to digital pin 7
 * LCD Enable pin to digital pin 6
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
   
 - V1.00 11/26/13
   Based on examples at http://www.bajdi.com/
   Questions: terry@yourduino.com */

/*-----( Import needed libraries )-----*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>
/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN   9
#define CSN_PIN 10

// NOTE: the "LL" at the end of the constant is "LongLong" type
const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
/*-----( Declare objects )-----*/
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
/*-----( Declare Variables )-----*/
const uint64_t pipes[2] = { 0xe7e7e7e7e7LL, 0xc2c2c2c2c2LL }; //pipe addresses
int joystick[3];  // 2 element array holding Joystick readings
int packetReceived = 0; //for determining the temp only once.
void setup()   /****** SETUP: RUNS ONCE ******/
{
  pinMode(8, INPUT);
  radio.begin();
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);
  radio.startListening();
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Joe's Radio:");
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  while(digitalRead(8) == LOW)
  {
    lcd.clear();
    lcd.print("Press for data");
    delay(100);  //software debounce
    packetReceived = 0;
  }
  if(packetReceived == 1)
  {
    //lcd.clear();
    lcd.setCursor(0, 0);
      lcd.print("Temp = ");
      lcd.setCursor(7, 0);
      lcd.print(1.8*joystick[1] + 32);
      lcd.setCursor(0, 1);
      lcd.print("Humidity = ");
      lcd.setCursor(11, 1);
      lcd.print(joystick[2]);
  }
  else {
  int packetRequest = 99; //arbitrary packetRequest value
  // transmit the data
  radio.stopListening();
  radio.write( &packetRequest, sizeof(packetRequest) );
  radio.startListening();
  // Read the data payload until we've received everything
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  //Either will have new packet of data or a timeout will occur
  while (!radio.available() && ! timeout){
    if (millis() - started_waiting_at > 1000 )
      timeout = true;
  }
  if (timeout){
    lcd.clear();
    lcd.print("Timed out.");
  } 
  else {
    radio.read( &joystick, sizeof(joystick) );
  }
  if(joystick[0] == 0){
      lcd.clear();
      lcd.print("Temp = ");
      lcd.setCursor(7, 0);
      lcd.print(1.8*joystick[1] + 32);
      lcd.setCursor(0, 1);
      lcd.print("Humidity = ");
      lcd.setCursor(11, 1);
      lcd.print(joystick[2]);
      packetReceived = 1;
    }
    else if(joystick[0] == -1){
      lcd.clear();
      lcd.print("Checksum error.");
    }
    else if(joystick[0] == -2){
      lcd.clear();
      lcd.print("Time out error.");
    }
    else if(joystick[0] == -3){  //corrupt packetRequest feedback, send again
      lcd.clear();
      lcd.print("Packet Corrupt.");
    }
    else {
      lcd.clear();
      lcd.print("Sorry nigga");
    }
  }
}//--(end main loop )---
//*********( THE END )***********


