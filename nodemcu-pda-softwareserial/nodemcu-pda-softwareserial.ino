#include <SoftwareSerial.h>

// SoftwareSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);
SoftwareSerial mySerial(13, 15, false, 256); // RX D7, TX D8

//SIM800L TX ---- D7 NODEMCU
//SIM800L RX ---- D8 NODEMCU


//Mas informacion http://pdacontroles.com/
// More info http://pdacontrolen.com/

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Connect! - Conexion ");
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  mySerial.println("Connect! - Conexion SOFTWARESERIAL");
}

void loop() // run over and over
{
  if (mySerial.available())
    Serial.write(mySerial.read());
  if (Serial.available())
    mySerial.write(Serial.read());
}
