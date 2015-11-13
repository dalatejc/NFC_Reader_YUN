/**************************************************************************/
/*! 
    @file     readMifareClassic.pde
    @author   Adafruit Industries
  @license  BSD (see license.txt)

    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    Reads the 4 byte (32 bit) ID of a MiFare Classic card.
    Since the classic cards have only 32 bit identifiers you can stick
  them in a single variable and use that to compare card ID's as a
  number. This doesn't work for ultralight cards that have longer 7
  byte IDs!
   
    Note that you need the baud rate to be 115200 because we need to
  print out the data and read from the card at the same time!

This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
This library works with the Adafruit NFC breakout 
  ----> https://www.adafruit.com/products/364
 
Check out the links above for our tutorials and wiring diagrams 
These chips use SPI to communicate, 4 required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <EEPROM.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   (6)  // Change and rewire to 6 if using the Yun
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET); //I2C setup

void setup(void) {
  Serial.begin(115200);
  Bridge.begin();
  Serial.println("Initialized");
  pinMode(10,OUTPUT);
  nfc.begin();

  // Configure board to read RFID tags
  nfc.SAMConfig();
  Serial.println("");
  Serial.println("Waiting for card..");Serial.println("");

  // Start Server
  YunServer server;
//  server.listenOnLocalhost();
  server.begin();
}


void loop(void) {

  // Vars for card reading
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  int inByte = 0;
  YunServer server;

  //while(!Serial); // Test code for Yun

  // If statement for matching card number and pin outputs
  if (success) {
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];  
      cardid <<= 8;
      cardid |= uid[3]; 
      Serial.print("DeviceID #");Serial.print(cardid);
      Serial.print(" in range");Serial.println("");
        if (cardid == 73563026) {
          Serial.print("Oyster Card");Serial.println("");Serial.println("");
//          inByte = Serial.read();
          digitalWrite(10,HIGH);
          delay(1000);
          digitalWrite(10,LOW);
          delay(50);
        }
        else {
          Serial.print("Not recognised device");Serial.println("");Serial.println("");
          delay(50);
        }
  }
  else {
    Serial.print("Read Failed");Serial.println("");
  }

// Get clients coming from server
YunClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);
    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

//function to parse input and output to correct function
void process(YunClient client) {
  String command = client.readStringUntil('/');

  if (command == "digital") {
    digitalCommand(client);
  }
//  if (command == "analog") { //no analog cos im lazy
//    analogCommand(client);
//  }
  if (command == "mode") {
    modeCommand(client);
  }
}

// deal with digital pins
void digitalCommand(YunClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  } 
  else {
    value = digitalRead(pin);
  }
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

//hold pin number, create var, check valid pin
void modeCommand(YunClient client) {
  int pin;
  pin = client.parseInt();
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }
  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}

