#include "UbidotsMicroESP8266.h"

#define TOKEN  "A1E-6TwRB1I2osbXqKcuJYG3WduSoQG5oI"  // Put here your Ubidots TOKEN
#define ID_1 "5a7e4c7b7625427107ab3c78" /* Put your variable ID here */
#define WIFISSID "Casa" // Put here your Wi-Fi SSID
#define PASSWORD "remioy2006202" // Put here your Wi-Fi password

Ubidots client(TOKEN);

void setup() {
    Serial.begin(115200);
    delay(10);
    client.wifiConnection(WIFISSID, PASSWORD);
}

void loop() {
    float value = client.getValue(ID_1);
    Serial.print("Value: ");
    Serial.println(value);
    delay(10000);
}