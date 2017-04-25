#include <SoftwareSerial.h>

// SoftwareSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);
SoftwareSerial swSer(14, 12, false, 256); // RX D5, TX D6

//SIM800L TX ---- D5 NODEMCU
//SIM800L RX ---- D6 NODEMCU

void setup() {
  Serial.begin(115200);
  swSer.begin(115200);

  Serial.println("\nSoftware serial test started");

}

void loop() {
  while (swSer.available() > 0) {
    Serial.write(swSer.read());
  }
  while (Serial.available() > 0) {
    swSer.write(Serial.read());
  }

}
