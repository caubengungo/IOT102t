#include <EEPROM.h>

#define PIN_ADDRESS 0
char defaultPin[5] = "1234";

void setup() {
  Serial.begin(9600);
  EEPROM.put(PIN_ADDRESS, defaultPin);
  Serial.println("Mã PIN đã được reset về 1234!");
}

void loop() {
  // Không cần thực hiện gì trong loop
}
