
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

ESP8266WebServer server(80);

#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;
bool isIncontact         = false;
bool isAuthorized        = false;
char currentLine[500]    = "";
int currentLineIndex     = 0;
bool nextLineIsMessage   = false;
bool nextValidLineIsCall = false;
String PhoneCallingIndex = "";
String PhoneCalling      = "";
String OldPhoneCalling   = "";
String BuildString       = "";
String lastLine          = "";
String id                = "";
String phonenum          = "";
int firstComma           = -1;
int prende               = 0;
int secondComma          = -1;
String Password          = "";
int thirdComma           = -1;
int forthComma           = -1;
int fifthComma           = -1;
int firstQuote           = -1;
int secondQuote          = -1;
int swveces              = 0;
int len                  = -1;
int j                    = -1;
int i                    = -1;
int f                    = -1;
int r                    = 0;
bool isInPhonebook       = false;
int x                    = 0;
int8_t answer;
unsigned long xprevious;
char contact[13];  


/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
  //server.send(200, "text/html", "<h1>You are connected</h1>");
  server.send(200, "text/html", "<h1>Welcome to The Orchid :) ******************</h1>");
}

const char* ssid = "Casa";
const char* password = "remioy2006202";

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("NODE");

  // No authentication by default
  ArduinoOTA.setPassword((const char *)"9080706050");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(1000);
  Serial.print("Configuring access point...");
  /* Set these to your desired credentials for Nodemcu as AP. */
  /* You can remove the password parameter if you want the AP to be open. */
  /*WiFi.softAP("ssid", "password")*/
  //const char* ssid = "Bash-Shell" // Does not work as a variable , has to be hardcoded
  //const char* password = 0123456789  // Does not work as a variable , has to be hardcoded
  WiFi.softAP("Bash-Shell", "0123456789");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
  pinMode(LED_BUILTIN, OUTPUT);
  power_on();
  Serial.println("Connecting to the network...");
  while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 5000,0) ||
           sendATcommand("AT+CREG?", "+CREG: 0,5", 5000,0)) == 0 );
  sendATcommand("AT+CMGF=1", "OK", 5000,0);
  sendATcommand("AT+CNMI=1,2,0,0,0", "OK", 5000,0);
  sendATcommand("AT+CPBR=1,1", "OK\r\n", 5000,1);
  Serial.println("Password:");   
  Serial.println(Password);
  WiFiMulti.addAP("Casa","remioy2006202");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  char msgx[1024];  
  char telx[1024];
  GetInfoFromWeb(0);
  id         = BuildString.substring(BuildString.indexOf("+")+1,BuildString.indexOf("#"));
  String tel = BuildString.substring(BuildString.indexOf("#")+1,BuildString.indexOf("$"));  
  String msg = BuildString.substring(BuildString.indexOf("$")+1); 
  Serial.println("id :"+id);
  Serial.println("tel:"+tel);
  Serial.println("msg:"+msg);
  if ( tel   != "99999999999")
     {  
       strcpy(telx, tel.c_str());
       strcpy(msgx, msg.c_str());
       sendSMS  (telx,msgx) ;
       GetInfoFromWeb(1); 
     } 
}     
  ///////////////////////////////////////////////////////
void power_on() 
///////////////////////////////////////////////////////
{
uint8_t answer = 0;
answer = sendATcommand("AT", "OK", 5000,0);
if (answer == 0)
   {
   while (answer == 0) 
         {   
       answer = sendATcommand("AT", "OK", 2000,0);
       }
   }
}
///////////////////////////////////////////////////////
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout,int xpassword)
///////////////////////////////////////////////////////
{
  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;
  memset(response, '\0', 100);    
  delay(100);
  while (Serial.available()) 
        {  
        Serial.read();
        }
  Serial.println(ATcommand);     
  x = 0;
  previous = millis();
  do
  {
    if (Serial.available() != 0)
       {
       response[x] = Serial.read();
       x++;
       if (strstr(response, expected_answer) != NULL)
          {
          answer = 1;
          String numbFromSim = String(response);
          numbFromSim = numbFromSim.substring(numbFromSim.indexOf(":"), 
                        numbFromSim.indexOf(",129,"));  
          numbFromSim = numbFromSim.substring((numbFromSim.indexOf(34) + 1), 
                        numbFromSim.indexOf(34, numbFromSim.indexOf(34) + 1));     
          if ( xpassword == 1) 
             {  
             numbFromSim = numbFromSim.substring( 0,4); 
             Password = numbFromSim ;   
             return 0;
             }
          else
             {
              numbFromSim = numbFromSim.substring( 0,11 );
             }  
         }
       }
     
  } 
  while ((answer == 0) && ((millis() - previous) < timeout));
  return answer;
}
////////////////////////////////////////////////////////
int sendSMS(char *phone_number, char *sms_text)
///////////////////////////////////////////////////////
{
  char aux_string[30];
  //char phone_number[] = "04168262667"; // ********* is the number to call
  //char sms_text[] = "Test-Arduino-Hello World";
  Serial.print("Setting SMS mode...");
  sendATcommand("AT+CMGF=1", "OK", 5000,0);    // sets the SMS mode to text
  Serial.println("Sending SMS");

  sprintf(aux_string, "AT+CMGS=\"%s\"", phone_number);
  answer = sendATcommand(aux_string, ">", 20000,0);    // send the SMS number
  if (answer == 1)
  {
    Serial.println(sms_text);
    Serial.write(0x1A);
    answer = sendATcommand("", "OK", 20000,0);
    if (answer == 1)
    {
      Serial.println("Sent ");
    }
    else
    {
      Serial.println("error ");
    }
  }
  else
  {
    Serial.println("error ");
    Serial.println(answer, DEC);
  }
  return answer;
}
/////////////////////////////////////////////////////////////////////////////
void clearBuffer()
/////////////////////////////////////////////////////////////////////////////
{
byte w = 0;
for (int i = 0; i < 10; i++)
    {
    while (Serial.available() > 0)
          {
      char k = Serial.read();
      w++;
      delay(1);
      }
    delay(1);
    }
}
////////////////////////////////////////////////////////////////////
int GetInfoFromWeb (int router)
{
delay(10000); 
String xp;
if((WiFiMulti.run() == WL_CONNECTED) ) 
  {  
  Serial.println("[++++++GetInfoFromWeb+++++++");
  xp="http://correosmasivos-com-ve.alpha.ioticos.com/readmensajetexto.php?sw=1";
  if (router == 1)
     {
   xp="http://correosmasivos-com-ve.alpha.ioticos.com/readmensajetexto.php?sw=2&id="+id;
   }
   
  Serial.println(xp); 
  HTTPClient http;  
  http.begin(xp); 
  int httpCode = http.GET();
  if(httpCode > 0) 
  {
  if(httpCode == HTTP_CODE_OK) 
    {
    BuildString = http.getString();
    }
  } 
  else 
  {
  Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  }   
}

