#include "UbidotsMicroESP8266.h"

#define TOKEN "A1E-6TwRB1I2osbXqKcuJYG3WduSoQG5oI"  // put here your Ubidots TOKEN
#define ID_1 "5a7e5757c03f9767bc12fedf" /* put your variable ID here */
#define WIFISSID "Casa" // put here your Wi-Fi SSID
#define PASSWORD "remioy2006202" // put here your Wi-Fi password

Ubidots client(TOKEN);

void setup() {
    // initialize the LED_BUILTIN pin as an output
    pinMode(LED_BUILTIN, OUTPUT);     
    Serial.begin(115200);
    delay(10);
    client.wifiConnection(WIFISSID, PASSWORD);
    //to change the device name:
    client.setDataSourceName("NODEMCU");
    //to change the device label:
    client.setDataSourceLabel("elapsed time");
}

void loop()
{
    bool led_status = client.getValue(ID_1);
    Serial.print("led_status: ");
    Serial.println(led_status);
    switch (led_status)
    {
        case 0:
            // turn the LED on (Note that LOW is the voltage level
            // but actually the LED is on; this is because 
            // it is active low on the ESP-01)
            digitalWrite(LED_BUILTIN, HIGH);
            break;
        case 1:
            // turn the LED off by making the voltage HIGH
            digitalWrite(LED_BUILTIN, LOW);
            break;
        default:
            break;
    }
    delay(2000);
}