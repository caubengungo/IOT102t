#include <EEPROM.h>

#define PIN_ADDRESS 0
char defaultPin[5] = "1234";

void setup() {
  Serial.begin(9600);
  EEPROM.put(PIN_ADDRESS, defaultPin);
  Serial.println("PIN has been reseted to 1234!");
}

void loop() {
  //Do nothing in loop
}
