/*
  Sends SMS Using NODEMCU and SIM800L C oroboard

*/

//int onModulePin = 13;
int8_t answer;
int x;

////Variable to hold last line of serial output from SIM800
char currentLine[500] = "";
int currentLineIndex = 0;

//Boolean to be set to true if message notificaion was found and next
//line of serial output is the actual SMS message content
bool nextLineIsMessage = false;

////Boolean to be set to true if call notificaion was found and next line is NOT empty
bool nextValidLineIsCall = false;

//Boolean to be set to true if number is found on contact
bool isIncontact = false;

// String which holds the last line read from Serial activitiy
String lastLine = "";

// To be saved phonemumber who sent SMS
String phonenum = "";

// Integer indexes
int firstComma = -1;
int secondComma = -1;
int thirdComma = -1;
int forthComma = -1;
int fifthComma = -1;
int firstQuote = -1;
int secondQuote = -1;
int len = -1;
int j = -1;
int i = -1;
int f = -1;
int r = -1;

//Boolean to be set to true if number is found on phonebook
bool isInPhonebook = false;


void setup() {

  //pinMode(onModulePin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize serial communications at 115200 bps:
  Serial.begin(115200);
  Serial.println("Starting...");
  power_on();
  delay(3000);

  Serial.println("Connecting to the network...");

  //checks network registration
  while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 5000) ||
           sendATcommand("AT+CREG?", "+CREG: 0,5", 5000)) == 0 );

  //sets  SMS mode to ASCII
  sendATcommand("AT+CMGF=1", "OK", 5000);

  //new message indication to Terminal Equipment TE.
  sendATcommand("AT+CNMI=1,2,0,0,0", "OK", 5000);

  //sets call notification
  sendATcommand("AT+CLIP=1\r\n", "OK", 5000);

}


void loop()
{
  
  //If there is serial output from SIM800
  if (Serial.available() > 0)
  {
    char lastCharRead = Serial.read();

    //Read each character from serial output until \r or \n is reached (which denotes end of line)
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
  //char phone_number[] = "04168262667"; // ********* is the number to call
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
/////////////////////////////////////////////////////////////////////////////////
void endOfLineReached()
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
  else if ((lastLine.length() > 0) && (nextValidLineIsCall))        // Rejects any empty line
  {
    ////LastLineIsCLIP();
  }
  else if (lastLine.startsWith("+CMT:"))                           // New incoming SMS
  {
    Serial.println(lastLine);
    phonenum = lastLine.substring((lastLine.indexOf(34) + 1),   //Parser to extract phonenumber
                                  lastLine.indexOf(34, lastLine.indexOf(34) + 1));
    ////Serial.println(phonenum);
    nextLineIsMessage = true;

    // Parsing lastLine to determine registration on SIM card
    firstComma = lastLine.indexOf(',');
    Serial.println(firstComma);  //For debugging
    secondComma = lastLine.indexOf(',', firstComma + 1);
    Serial.println(secondComma); //For debugging

    // +CMT: "+584168262667","","17/03/14,16:18:53-16" TelefÃ³nica Movistar Venezuela with no contacts saved
    //firstComma = 21
    //secondComma = 24

    Serial.println("Diff");
    Serial.println(secondComma - firstComma);


  }
  else if ((lastLine.length() > 0) && (nextLineIsMessage))       // Rejects any empty line
  {

    LastLineIsCMT();

  }

  CleanCurrentLine();
}
/////////////////////////////////////////////////////////////////////
void CleanCurrentLine()
{
  //Clear char array for next line of read
  for ( int i = 0; i < sizeof(currentLine);  ++i )
  {
    currentLine[i] = (char)0;
  }
  currentLineIndex = 0;
}
//////////////////////////////////////////////////////////////////////
void LastLineIsCMT()
{
  if (nextLineIsMessage)
  {
    Serial.println(lastLine);


    // If exists on Phonebook
    //if (secondComma > 22)   // Only works with Movilnet and Digitel Venezuela
    //if (secondComma > 24)    // Only works with for TelefÃ³nica Movistar Venezuela
    if ((secondComma - firstComma) > 3)
    {
      Serial.println("In Phonebook"); //For debugging
      isInPhonebook = true;
      Serial.println(isInPhonebook);
      clearBuffer();
    }
    else
    {
      Serial.println("Not in Phonebook"); //For debugging
      isInPhonebook = false;
      clearBuffer();
    }

    //if on phonebook ---------------------------
    if (isInPhonebook)
    {
      // If SMS contains LED ON or LED OFF or #WhiteList
      if (lastLine.indexOf("LED ON") >= 0)
      {
         digitalWrite(LED_BUILTIN, LOW);  // Turns ON LED
         clearBuffer();
      }
      else if (lastLine.indexOf("LED OFF") >= 0)
      {
         digitalWrite(LED_BUILTIN, HIGH);  // Turns OFF LED
         clearBuffer();
      }
      else
      {
        clearBuffer();
      }
    }
    CleanCurrentLine();
    nextLineIsMessage = false;
  }
}
//////////////////////////////////////////////////////////////////////////

