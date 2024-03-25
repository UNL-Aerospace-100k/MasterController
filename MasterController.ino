/*  100k Launch Controller Master V2.3
 *  10/26/2023
 *  Jack Shaver
 */

 /*
  *  This code takes inputs from 3 safety toggle switches, and from monostable buttons
  *  Sends LoRa transmissions to the sub-master controller.
  *  
  *  Written for use with an ARDUINO NANO, SX1278 LoRa module, and the Master Controller V1 PCB
  *  Intended for use with the control panel V1, no lights
  *  All safety checks must be met for each input for an output to be made
  *  All monostable buttons are debounced, and transient proofed
  *  Supports wireless transmission with security check, 1 way transmission
  *  Tracks when the sensor monitor is active, and when the oxidizer valve is open
  *  
  *  Todo:
  *  Construct Launch Controller Box
  *  Re Write all the code, this is terrible embedded programming lol
  *  
  */

//Libraries for the LoRa Transmitter
#include <SPI.h>
#include <LoRa.h>
#include <avr/interrupt.h>

/*The LoRa transmitter must be connected to the NANO via the following 
 * LoRa -> NANO
 * GND -> GND
 * VCC -> 3.3 (NANO output)
 * MISO -> D12
 * MOSI -> D11
 * SLCK -> D13
 * NSS -> D10
 */

//Input pin definitions
#define dataCollect A0
#define coilOneButton A1
#define coilTwoButton A2
#define coilTwoSafety A3
#define coilOneSafety A4
#define mainSafety A5

#define statusLED 2
#define mainSafetyLED 5

//Setup runs once, used for initializing all I/O pins
void setup() {
  //internal pullup resistors are initialized on inputs, to prevent floating values
  //"active" signals must be to ground, not Vcc
  pinMode(dataCollect,INPUT_PULLUP);
  pinMode(coilTwoButton,INPUT_PULLUP);
  pinMode(coilOneButton,INPUT_PULLUP);
  pinMode(coilTwoSafety,INPUT_PULLUP);
  pinMode(coilOneSafety,INPUT_PULLUP);
  pinMode(mainSafety,INPUT_PULLUP);

  pinMode(statusLED,OUTPUT);
  pinMode(mainSafetyLED,OUTPUT);


 //Initialize serial communication and the LoRa Transmitter
  Serial.begin(9600);
  while (!Serial);

  LoRa.setPins(10, -1, 13);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Transmitter Initialization failed!");
    while (1){
      digitalWrite(statusLED, HIGH);
      delay(100);
      digitalWrite(statusLED, LOW);
      delay(100);
    };
  }
  digitalWrite(statusLED, HIGH);
  delay(1000);
  digitalWrite(statusLED, LOW);
}

long debounceTimer = millis();


void loop() { //_______________________________LOOP________________________________________
  digitalWrite(mainSafetyLED, !digitalRead(mainSafety));

  if((digitalRead(mainSafety) == LOW) && ((millis() - debounceTimer) > 1000)){
    if((digitalRead(coilOneSafety) == LOW) && (digitalRead(coilOneButton) == LOW)){
      LoRaTransmit(1);
      debounceTimer = millis();
    }

    if((digitalRead(coilTwoSafety) == LOW) && (digitalRead(coilTwoButton) == LOW)){
      LoRaTransmit(2);
      debounceTimer = millis();
    }

    if(digitalRead(dataCollect) == LOW){
      LoRaTransmit(3);
      debounceTimer = millis();
    }
  }
}

//transmit the propper message via LoRa
void LoRaTransmit(int message){
  char *messages[] = {"", "COIL_ONE", "COIL_TWO", "DATA_COLLECT"};

  LoRa.beginPacket();
  LoRa.print(messages[message]);
  LoRa.endPacket();

  digitalWrite(statusLED, HIGH);
  delay(100);
  digitalWrite(statusLED, LOW);
}
