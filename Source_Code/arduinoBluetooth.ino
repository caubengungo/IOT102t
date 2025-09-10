#include <SoftwareSerial.h>

SoftwareSerial BT(2, 3); // RX, TX cho HC-05

void setup() {
    Serial.begin(9600);
    BT.begin(9600);
    Serial.println("HC-05 Ready! Type something in Serial Monitor:");
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        Serial.write(c);  // Gửi từ Serial Monitor đến HC-05
    }

   
}
