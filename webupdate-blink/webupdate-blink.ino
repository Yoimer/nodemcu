/*
  To upload through terminal you can use: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <SD.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>


File root;
ESP8266WiFiMulti WiFiMulti;

const char* host = "esp8266-webupdate";
const char* ssid = "Casa";
const char* password = "remioy2006202";

ESP8266WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

int8_t answer;
bool isIncontact         = false;
bool isAuthorized        = false;
int x;
unsigned long xprevious;
char currentLine[500] = "";
int currentLineIndex = 0;
bool nextLineIsMessage = false;
bool nextValidLineIsCall = false;
String PhoneCallingIndex = "";
String PhoneCalling      = "";
String OldPhoneCalling   = "";
String DateandHour       = "";
String AnalogInput = "";
String lastLine = "";
String phonenum = "";
int firstComma = -1;
int prende = 0;
int secondComma = -1;
String Password          = "";
int thirdComma = -1;
int forthComma = -1;
int fifthComma = -1;
int firstQuote = -1;
int secondQuote = -1;
int firstColon = -1;
int secondColon = -1;
int thirdColon = -1;
int swveces      = 0;
int len = -1;
int j = -1;
int i = -1;
int f = -1;
int r = 0;
bool isInPhonebook = false;
char contact[13];






void setup(void){
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  WiFi.softAP("hostpot", "11223344");
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

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("done!");

  Serial.println("Starting SIM800L Module...");
  power_on();
  delay(3000);
  Serial.println("Connecting to the network...");
  while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 5000, 0) ||
           sendATcommand("AT+CREG?", "+CREG: 0,5", 5000, 0)) == 0 );
  sendATcommand("AT+CMGF=1", "OK", 5000, 0);
  sendATcommand("AT+CNMI=1,2,0,0,0", "OK", 5000, 0);
  sendATcommand("AT+CPBR=1,1", "OK\r\n", 5000, 1);
  Serial.println("Password:");
Serial.println(Password);

}
 
void loop(void){
  server.handleClient();
  delay(1);
  /*digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is active low on the ESP-01)
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(2000);*/
  if (Serial.available() > 0)
  {
    char lastCharRead = Serial.read();
    if (lastCharRead == '\r' || lastCharRead == '\n')
    {
      endOfLineReached();
    }
    else
    {
      currentLine[currentLineIndex++] = lastCharRead;
    }
  }                 
}

void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}


///////////////////////////////////////////////////////
void power_on()
///////////////////////////////////////////////////////
{
  uint8_t answer = 0;
  answer = sendATcommand("AT", "OK", 5000, 0);
  if (answer == 0)
  {
    while (answer == 0)
    {
      answer = sendATcommand("AT", "OK", 2000, 0);
    }
  }
}
///////////////////////////////////////////////////////
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout, int xpassword)
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
          numbFromSim = numbFromSim.substring( 0, 4);
          Password = numbFromSim ;
          return 0;
        }
        else
        {
          numbFromSim = numbFromSim.substring( 0, 11 );
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
  sendATcommand("AT+CMGF=1", "OK", 5000, 0);   // sets the SMS mode to text
  Serial.println("Sending SMS");

  sprintf(aux_string, "AT+CMGS=\"%s\"", phone_number);
  answer = sendATcommand(aux_string, ">", 20000, 0);   // send the SMS number
  if (answer == 1)
  {
    Serial.println(sms_text);
    Serial.write(0x1A);
    answer = sendATcommand("", "OK", 20000, 0);
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
/////////////////////////////////////////////////////////////////////////////
void endOfLineReached()
/////////////////////////////////////////////////////////////////////////////
{
  lastLine = String(currentLine);

  //-----------------------Call---------------------------//
  // If lastLine reads RING, New Call Indication was received.
  // Hence, the THIRD LINE is the caller information.

  // The whole string will be something like this if caller IS registered on SIM Card:
  //RING (First Line)
  //     (Second Line is empty)
  //+CLIP: "04168262667",129,"",0,"Yoimer",0 (Third Line)

  // The whole string will be something like this if caller is NOT registered on SIM Card
  //RING (First Line)
  //     (Second Line is empty)
  //+CLIP: "04168262667",129,"",0,"",0 (Third Line)

  //----------------------SMS-------------------------------------//
  //If lastLine reads +CMT, New SMS Message Indications was received.
  //Hence, next line is the message content.

  if (lastLine.startsWith("RING"))                                   // New incoming call
  {
    Serial.println(lastLine);
    nextValidLineIsCall = true;
  }
  else
  {
    if ((lastLine.length() > 0) && (nextValidLineIsCall))        // Rejects any empty line
    {
      LastLineIsCLIP();
    }
    else if (lastLine.startsWith("+CMT:"))                          // New incoming SMS
    {
      Serial.println(lastLine);
      phonenum = lastLine.substring((lastLine.indexOf(34) + 1),
                                    lastLine.indexOf(34, lastLine.indexOf(34) + 1));
      nextLineIsMessage = true;
      firstComma        = lastLine.indexOf(',');
      secondComma       = lastLine.indexOf(',', firstComma  + 1);
      thirdComma        = lastLine.indexOf(',', secondComma + 1);
      forthComma        = lastLine.indexOf(',', thirdComma  + 1);
      fifthComma        = lastLine.indexOf(',', forthComma  + 1);
      PhoneCalling      = lastLine.substring((firstComma - 12), (firstComma - 1));
      PhoneCallingIndex = lastLine.substring((firstComma + 2), (secondComma - 1));
    firstColon        = lastLine.indexOf(':');
    secondColon       = lastLine.indexOf(':', firstColon + 1);
    thirdColon =        lastLine.indexOf(':', secondColon + 1);
    DateandHour       = lastLine.substring((secondComma + 2), (thirdColon));
    Serial.println(DateandHour);
      Serial.println(PhoneCalling);        ////////////////////////////////////////////
      Serial.println(PhoneCallingIndex);
      j            = PhoneCallingIndex.toInt();
      isIncontact  = false;
      isAuthorized = false;
      if (j > 0)
      {
        isIncontact = true;
        Serial.println("en phonebook"); //////////////////////////////////////////////
        if (j <= 5 )
        {
          Serial.println("autorizada"); //////////////////////////////////////////////
          isAuthorized = true;
        }
      }
      else
      {
      }
    }
    else if ((lastLine.length() > 0) && (nextLineIsMessage))
    {
      LastLineIsCMT();
    }
  }
  CleanCurrentLine();
}
/////////////////////////////////////////////////////////////////////
void CleanCurrentLine()
/////////////////////////////////////////////////////////////////////
{
  for ( int i = 0; i < sizeof(currentLine);  ++i )
  {
    currentLine[i] = (char)0;
  }
  currentLineIndex = 0;
}
//////////////////////////////////////////////////////////////////////
void LastLineIsCMT()
/////////////////////////////////////////////////////////////////////
{
  Serial.println(lastLine);
  clearBuffer();
  if (isIncontact)
  {
    if (lastLine.indexOf("LED ON") >= 0)
    {
      prendeapaga(0);
    }
    else if (lastLine.indexOf("LED OFF") >= 0)
    {
      prendeapaga(1);
    }
    else if (lastLine.indexOf("ADD") >= 0)
    {
      DelAdd(1);
    }
    else if (lastLine.indexOf("DEL") >= 0)
    {
      DelAdd(2);
    }
  else if (lastLine.indexOf("GETA0") >= 0)
    {
      GetValueofAnalog0();
    }
    else
    {
      clearBuffer();
    }
  }
  CleanCurrentLine();
  nextLineIsMessage = false;

}
////////////////////////////////////////////////////////
int  prendeapaga (int siono)
///////////////////////////////////////////////////////
{
  Serial.println("KKKKKKKKKKKKKKKKKKKKKKKKKKK");
  Serial.println(lastLine);
  firstComma    = lastLine.indexOf(',');
  secondComma   = lastLine.indexOf(',', firstComma  + 1);
  String InPassword = lastLine.substring((firstComma + 1), (secondComma));
  Serial.println(InPassword);
  //PasswordOk        =  false ;
  if (InPassword == Password)
  {
    //PasswordOk = true ;
    digitalWrite(LED_BUILTIN, siono);
  }
  clearBuffer();
}
/////////////////////////////////////////////////////////////////////////////////////////
int  DelAdd(int DelOrAdd)
/////////////////////////////////////////////////////////////////////////////////////////////
{
  char aux_string[100];
  firstComma          = lastLine.indexOf(',');
  secondComma         = lastLine.indexOf(',', firstComma  + 1);
  thirdComma          = lastLine.indexOf(',', secondComma + 1);
  String indexAndName = lastLine.substring((firstComma + 1), (secondComma));
  String newContact   = lastLine.substring((secondComma + 1), thirdComma);
  if (!isAuthorized)
  {
    Serial.println(j); //////////////////////////////////////////////////////////////
    Serial.println("Not authorized to Delete/Add"); /////////////////////////////////
    return 0;
  }
  String tmpx;
  tmpx = "AT+CPBW=" + indexAndName + "\r\n\"";
  if ( DelOrAdd == 1 )
  {
    tmpx = "AT+CPBW=" + indexAndName + ",\"" + newContact + "\"" + ",129," + "\"" + indexAndName + "\"" + "\r\n\"";
  }
  tmpx.toCharArray( aux_string, 100 );
  Serial.println(aux_string);
  answer = sendATcommand(aux_string, "OK", 20000, 0);   // send the SMS number
  if (answer == 1)
  {
    Serial.println("Sent ");
  }
  else
  {
    Serial.println("error ");
  }
  clearBuffer();
}
//////////////////////////////////////////////////////////////////////////
void LastLineIsCLIP()
////////////////////////////////////////////////////////////////////////
{
  firstComma         = lastLine.indexOf(',');
  secondComma        = lastLine.indexOf(',', firstComma + 1);
  thirdComma         = lastLine.indexOf(',', secondComma + 1);
  forthComma         = lastLine.indexOf(',', thirdComma + 1);
  fifthComma         = lastLine.indexOf(',', forthComma + 1);
  PhoneCalling       = lastLine.substring((firstComma - 12), (firstComma - 1));
  PhoneCallingIndex  = lastLine.substring((forthComma + 2), (fifthComma - 1));
  j                  = PhoneCallingIndex.toInt();
  if (PhoneCalling == OldPhoneCalling)
  {
    swveces = 1;
    if ((millis() - xprevious ) > 9000)
    {
      swveces   = 0;
      xprevious = millis();
    };
  }
  else
  {
    xprevious       = millis();
    OldPhoneCalling = PhoneCalling;
    swveces         = 0;
  }
  if (j > 0 & swveces == 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if ((WiFiMulti.run() == WL_CONNECTED) & swveces == 0)
  {
    HTTPClient http;
    String xp = "http://estredoyaqueclub.com.ve/arduinoenviacorreo.php?telefono=" + PhoneCalling + "-" + PhoneCallingIndex;
    http.begin(xp);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String BuildStringx = http.getString();
        Serial.println("[+++++++++++++++++++");
        Serial.println(BuildStringx);
        Serial.println("[+++++++++++++++++++");
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  clearBuffer();
  nextValidLineIsCall = false;
}
///////////////////////////////////////////////////////////////////////////////////
void GetValueofAnalog0() // SMS should be GETA0,9999, where 9999 is the first 4 numbers taken from position 1 on SIM
{
  Serial.println("KKKKKKKKKKKKKKKKKKKKKKKKKKK");
  Serial.println(lastLine);
  firstComma    = lastLine.indexOf(',');
  secondComma   = lastLine.indexOf(',', firstComma  + 1);
  String InPassword = lastLine.substring((firstComma + 1), (secondComma));
  Serial.println(InPassword);
  //PasswordOk        =  false ;
  if (InPassword == Password)
  {
  // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    float voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:
    Serial.println(voltage);
  //Converts voltage to an String, to be sent later to server
  AnalogInput = voltage; 
  
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http; //"http://estredoyaqueclub.com.ve/arduinoenviacorreo.php?telefono=" + PhoneCalling + "-" + PhoneCallingIndex;
    String xp = "http://estredoyaqueclub.com.ve/arduinoenviacorreo.php?telefono=" + AnalogInput + "-" + DateandHour;
    http.begin(xp);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String BuildStringx = http.getString();
        Serial.println("[+++++++++++++++++++");
        Serial.println(BuildStringx);
        Serial.println("[+++++++++++++++++++");
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  clearBuffer();
  }
  else
  {
  Serial.println("Password is not correct!");
  clearBuffer();
  }
}
