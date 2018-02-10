#include "UbidotsMicroESP8266.h"

#define TOKEN  "A1E-6TwRB1I2osbXqKcuJYG3WduSoQG5oI"  // Put here your Ubidots TOKEN
#define ID_1 "5a7e4c7b7625427107ab3c78" /* Put your variable ID here */
#define WIFISSID "Casa" // Put here your Wi-Fi SSID
#define PASSWORD "remioy2006202" // Put here your Wi-Fi password

Ubidots client(TOKEN);

void setup(){
    Serial.begin(115200);
    client.wifiConnection(WIFISSID, PASSWORD);
    //client.setDebug(true); // Uncomment this line to set DEBUG on

void loop(){
    // float value1 = analogRead(A0);
    /* float value2 = digitalRead(D1) */
    // client.add(ID_1, value1);
    /* client.add(ID_2, value2); */
    float value1 = millis();
    client.add(ID_1, value1);
    client.sendAll(false);
}