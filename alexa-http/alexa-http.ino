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
String xp                = "";

char contact[13];  
void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Starting...");
  WiFiMulti.addAP("Casa","remioy2006202");
  
  // tell alexa system is ON
  checkHTTP("http://castillolk.com.ve/proyectos/sms/alexa.php?sw=7");
  
 }

/////////////////////////////////////////////////////////////////
void loop()
{
  GetInfoFromWeb();
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
int GetInfoFromWeb ()
{
	delay(10000); 
	if((WiFiMulti.run() == WL_CONNECTED) ) 
	{  
		checkHTTP("http://castillolk.com.ve/proyectos/sms/alexa.php");

		if (BuildString.endsWith("ON"))
		{
			// Turn the LED on (Note that LOW is the voltage level
			digitalWrite(LED_BUILTIN, LOW);
		}
		else if (BuildString.endsWith("OFF"))
		{
			// Turn the LED off (Note that HIGH is the voltage level
			digitalWrite(LED_BUILTIN, HIGH);
		}  
	}
}

////////////////////////////////////////////////////////////////////
int checkHTTP(String website)
{
	Serial.println("[++++++GetInfoFromWeb+++++++");
	Serial.println(website); 
	HTTPClient http;  
	http.begin(website); 
	int httpCode = http.GET();
	if ((httpCode > 0) && (httpCode == HTTP_CODE_OK))
	{
		Serial.println("Connected");
		BuildString = "";
		BuildString = http.getString();
		Serial.println(BuildString);
	}
	else
	{
		Serial.println("Not connected");
		BuildString = "";
	}

	answer = 1;

	return answer;
}
