#include "UbidotsMicroESP8266.h"

#define TOKEN  "A1E-6TwRB1I2osbXqKcuJYG3WduSoQG5oI"  // Put here your Ubidots TOKEN
#define WIFISSID "Casa" // Put here your Wi-Fi SSID
#define PASSWORD "remioy2006202" // Put here your Wi-Fi password

Ubidots client(TOKEN);

void setup(){
    Serial.begin(115200);
    client.wifiConnection(WIFISSID, PASSWORD);
    //client.setDebug(true); // Uncomment this line to set DEBUG on
    //To change the Device Name:
    client.setDataSourceName("NODEMCU");
    //To change the Device label:
    client.setDataSourceLabel("elapsed time"); 
}
void loop(){
    //float value1 = analogRead(0);
    float value1 = millis();
    //float value2 = analogRead(2)
    //client.add("temperature", value1);
    //client.add("switch", value2);
    client.add("time", value1);
    client.sendAll(true);
    delay(5000);
}
