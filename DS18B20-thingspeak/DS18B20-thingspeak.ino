// ESP8266 DS18B20 ArduinoIDE Thingspeak IoT Example code
// http://vaasa.hacklab.fi
//
// https://github.com/milesburton/Arduino-Temperature-Control-Library
// https://gist.github.com/jeje/57091acf138a92c4176a

//Pin connection
//NodeMCU 3v3 to the Vin of DS18B20
//NodeMCU D1 to the data of DS18B20
//NodeMCU GND to the GND of DS18B20


#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D1


const char* host = "api.thingspeak.com"; // Your domain  
//String ApiKey = "THINGSPEAK_API_KEY";
String ApiKey = "PJ38C84LVKOUKTSF"; 
String path = "/update?key=" + ApiKey + "&field1=";  


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

const char* ssid = "Casa";
const char* pass = "remioy2006202";


char temperatureString[6];

void setup(void){
  Serial.begin(115200);
  Serial.println("");
  
  WiFi.begin(ssid, pass);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  DS18B20.begin();
   

}

float getTemperature() {
  float temp;
  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}


void loop() {

  float temperature = getTemperature();

  dtostrf(temperature, 2, 2, temperatureString);
  // send temperature to the serial console
  Serial.println(temperatureString);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET ") + path + temperatureString + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: keep-alive\r\n\r\n");
  delay(500);

}
