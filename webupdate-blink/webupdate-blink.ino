/*
  To upload through terminal you can use: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;

const char* host = "esp8266-webupdate";
const char* ssid = "Casa";
const char* password = "remioy2006202";

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



ESP8266WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    MDNS.begin(host);
    server.on("/", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });
    server.begin();
    MDNS.addService("http", "tcp", 80);
  
    Serial.printf("Ready! Open http://%s.local in your browser\n", host);
  } else {
    Serial.println("WiFi Failed");
  }

  pinMode(LED_BUILTIN, OUTPUT);
  //Serial.begin(115200);
  Serial.println("Starting SIM800 Module...");
  power_on();
  delay(3000);
  Serial.println("Connecting to the network...");
  while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 5000,0) ||
           sendATcommand("AT+CREG?", "+CREG: 0,5", 5000,0)) == 0 );
  sendATcommand("AT+CMGF=1", "OK", 5000,0);
  sendATcommand("AT+CNMI=1,2,0,0,0", "OK", 5000,0);
  sendATcommand("AT+CPBR=1,1", "OK\r\n", 5000,1);
  Serial.println("Password:");   
  Serial.println(Password);
}
 
void loop(void){
  /*server.handleClient();
  delay(1);*/

    char msgx[1024];  
  char telx[1024];
    GetInfoFromWeb(0); 
  // +ID#XXXXXXXXXX$MENSAJE
  // +9999#9999999999$
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
server.handleClient();  
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
