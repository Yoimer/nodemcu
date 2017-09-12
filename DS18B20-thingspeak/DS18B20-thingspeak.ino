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

const char* ssid = "FARC-ELN-ISIS";
const char* pass = "remioyroman";
char temperatureString[6];
char message[100];

int8_t answer;
int x;

////Variable to hold last line of serial output from SIM800
char currentLine[500] = "";
int currentLineIndex = 0;

//Boolean to be set to true if message notificaion was found and next
//line of serial output is the actual SMS message content
bool nextLineIsMessage = false;

//Boolean to be set to true if number is found on contact
bool isIncontact = false;

void setup(void){
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize serial communications at 115200 bps:
  Serial.begin(115200);
  Serial.println("");
  // connects to router
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
  
  Serial.println("Starting SIM800L Module..");
  power_on();
  
  Serial.println("Connecting to the network...");

  //checks network registration
  while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 5000) ||
           sendATcommand("AT+CREG?", "+CREG: 0,5", 5000)) == 0 );

  //sets  SMS mode to ASCII
  sendATcommand("AT+CMGF=1", "OK", 5000);

  //new message indication to Terminal Equipment TE.
  sendATcommand("AT+CNMI=1,2,0,0,0", "OK", 5000);
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


void loop()
 {

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
			   


  
  if (temperature >= 30.80) //Threslhold value
  { 
    snprintf( message, sizeof(message), "Warning, temperature value has increased to: %s %s", temperatureString, "Celsius Degrees" );
	sendSMS("04129501619", message);
	Serial.println(message);
  }
			   
  delay(500);
  
  //If there is serial output from SIM800
  /*if (Serial.available() > 0)
  {
    char lastCharRead = Serial.read();
    //Read each character from serial output until \r or \n is reached (which denotes end of line)
    if (lastCharRead == '\r' || lastCharRead == '\n')
    {
      String lastLine = String(currentLine);

      //If last line read +CMT, New SMS Message Indications was received.
      //Hence, next line is the message content.
      if (lastLine.startsWith("+CMT:"))
      {

        Serial.println(lastLine);
        nextLineIsMessage = true;
      }
      else if (lastLine.length() > 0)
      {
        if (nextLineIsMessage)
        {
          Serial.println(lastLine);

          //Read message content and set status according to SMS content
          if (lastLine.indexOf("LED ON") >= 0)
          {
            //ledStatus = 1;
            digitalWrite(LED_BUILTIN, LOW);
            sendSMS("04129501619", "LED IS ON");
            clearBuffer();
          }
          else if (lastLine.indexOf("LED OFF") >= 0)
          {
            //ledStatus = 0;
            digitalWrite(LED_BUILTIN, HIGH);
            sendSMS("04129501619", "LED IS OFF");
            clearBuffer();
          }
          else
          {
            clearBuffer();
          }

          nextLineIsMessage = false;
        }

      }

      //Clear char array for next line of read
      for ( int i = 0; i < sizeof(currentLine);  ++i )
      {
        currentLine[i] = (char)0;
      }
      currentLineIndex = 0;
    }
    else
    {
      currentLine[currentLineIndex++] = lastCharRead;
    }
  }
  delay(1000);*/
  
}


///////////////////////////////////////////////////////
void power_on() {

  uint8_t answer = 0;

  // checks if the module is started
  answer = sendATcommand("AT", "OK", 5000);
  if (answer == 0)
  {
    // waits for an answer from the module
    while (answer == 0) {   // Send AT every two seconds and wait for the answer
      answer = sendATcommand("AT", "OK", 2000);
    }
  }

}
/////////////////////////////////////////////////////////////////
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  //while ( Serial.available() > 0) Serial.read();   // Clean the input buffer

  while (Serial.available()) { //Cleans the input buffer
    Serial.read();
  }

  Serial.println(ATcommand);    // Send the AT command


  x = 0;
  previous = millis();

  // this loop waits for the answer
  do
  {
    if (Serial.available() != 0)
    {
      // if there are data in the UART input buffer, reads it and checks for the asnwer
      response[x] = Serial.read();
      x++;
      // check if the desired answer  is in the response of the module
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;
      }
    }
    // Waits for the asnwer with time out
  } while ((answer == 0) && ((millis() - previous) < timeout));

  return answer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int sendSMS(char *phone_number, char *sms_text)
{

  char aux_string[30];
  //char phone_number[] = "04129501619"; // ********* is the number to call
  //char sms_text[] = "Test-Arduino-Hello World";
  Serial.print("Setting SMS mode...");
  sendATcommand("AT+CMGF=1", "OK", 5000);    // sets the SMS mode to text
  Serial.println("Sending SMS");

  sprintf(aux_string, "AT+CMGS=\"%s\"", phone_number);
  answer = sendATcommand(aux_string, ">", 20000);    // send the SMS number
  if (answer == 1)
  {
    Serial.println(sms_text);
    Serial.write(0x1A);
    answer = sendATcommand("", "OK", 20000);
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
