#include "UbidotsMicroESP8266.h"

#define TOKEN  "A1E-6TwRB1I2osbXqKcuJYG3WduSoQG5oI"  // Put here your Ubidots TOKEN
#define ID_1 "5a7e5757c03f9767bc12fedf" /* Put your variable ID here */
#define WIFISSID "Casa" // Put here your Wi-Fi SSID
#define PASSWORD "remioy2006202" // Put here your Wi-Fi password

Ubidots client(TOKEN);

void setup() {
    Serial.begin(115200);
    delay(10);
    client.wifiConnection(WIFISSID, PASSWORD);
    //To change the Device Name:
    client.setDataSourceName("NODEMCU");
    //To change the Device label:
    client.setDataSourceLabel("elapsed time");
}

void loop() {
    bool value = client.getValue(ID_1);
    Serial.print("Value: ");
    Serial.println(value);
    delay(10000);
}