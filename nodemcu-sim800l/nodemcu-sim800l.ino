/*
  Sends SMS Using NODEMCU and SIM800L C oroboard

*/

//int onModulePin = 13;
int8_t answer;
int x;
char SMS[200];

 
//Variable to hold last line of serial output from SIM800
char currentLine[500] = "";
int currentLineIndex = 0;
 
//Boolean to be set to true if message notificaion was found and next
//line of serial output is the actual SMS message content
bool nextLineIsMessage = false;

void setup() {

  //pinMode(onModulePin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize serial communications at 115200 bps:
  Serial.begin(115200);
  Serial.println("Starting...");
  power_on();
  delay(3000);

  Serial.println("Connecting to the network..."); // AT+CFUN=1\r\n","OK\r\n

  while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 5000) ||
           sendATcommand("AT+CREG?", "+CREG: 0,5", 5000)) == 0 );

  sendATcommand("AT+CMGF=1", "OK", 5000);
  sendATcommand("AT+CNMI=1,2,0,0,0", "OK", 5000);
}


void loop()
{
  //If there is serial output from SIM800
  if(Serial.available() > 0)
  {
    char lastCharRead = Serial.read();
    //Read each character from serial output until \r or \n is reached (which denotes end of line)
    if(lastCharRead == '\r' || lastCharRead == '\n')
    {
        String lastLine = String(currentLine);
         
        //If last line read +CMT, New SMS Message Indications was received.
        //Hence, next line is the message content.
        if(lastLine.startsWith("+CMT:"))
        {
           
          Serial.println(lastLine);
          nextLineIsMessage = true;
        } 
        else if (lastLine.length() > 0) 
        {
          if(nextLineIsMessage) 
          {
            Serial.println(lastLine);
             
            //Read message content and set status according to SMS content
            if(lastLine.indexOf("LED ON") >= 0)
            {
              //ledStatus = 1;
              digitalWrite(LED_BUILTIN, LOW);
            } else if(lastLine.indexOf("LED OFF") >= 0) 
            {
              //ledStatus = 0;
              digitalWrite(LED_BUILTIN, HIGH);
            }
             
            nextLineIsMessage = false;
          }
           
        }
         
        //Clear char array for next line of read
        for( int i = 0; i < sizeof(currentLine);  ++i )
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
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {

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
  do {
    if (Serial.available() != 0) {
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
int sendSMS(char *phone_number, char *sms_text) {

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
      Serial.print("Sent ");
    }
    else
    {
      Serial.print("error ");
    }
  }
  else
  {
    Serial.print("error ");
    Serial.println(answer, DEC);
  }
  return answer;
}
//////////////////////////////////////////////////////////////////////////
void readSMS() {
  sendATcommand("AT+CMGF=1", "OK", 5000);
  answer = sendATcommand("AT+CNMI=1,2,0,0,0", "OK", 5000);
  if (answer == 1)
  {
    answer = 0;
    x = 0;
    while (Serial.available() == 0);
    // this loop reads the data of the SMS
    do {
      // if there are data in the UART input buffer, reads it and checks for the asnwer
      if (Serial.available() > 0) {
        SMS[x] = Serial.read();
        x++;
        // check if the desired answer (OK) is in the response of the module
        if (strstr(SMS, "OK") != NULL)
        {
          answer = 1;
        }
        if (strstr(SMS, "LED") != NULL)
        {
          answer = 2;
        }
      }
    } while (answer == 0);   // Waits for the asnwer with time out

    SMS[x] = '\0';

    Serial.println(SMS);

    //memset(SMS, '\0', 200);    // Initialize the string

    x = 0;
    for (x = 0; x < 200; ++x) {
      SMS[x] = '\0';
    }

    while (Serial.available()) { //Cleans the input buffer
      Serial.read();
    }

  }
  else
  {
    Serial.print("error ");
    Serial.println(answer, DEC);
  }

}
//////////////////////////////////////////////////////////////////////////////////////////////
int waitForResp(const char *resp, unsigned int timeout)
{
  int len = strlen(resp);
  int sum = 0;
  unsigned long timerStart, timerEnd;
  timerStart = millis();
  char currentLine[500] = "";
  int currentLineIndex = 0;
  //Clear char array for next line of read
  for ( int i = 0; i < sizeof(currentLine);  ++i )
  {
    currentLine[i] = (char)0;
  }
  String lastLine = "";
  while (1) {
    if (Serial.available()) {
      char c = Serial.read();
      currentLine[currentLineIndex] = c;
      currentLineIndex = currentLineIndex + 1;
      sum = (c == resp[sum]) ? sum + 1 : 0;
      if (sum == len)
      {
        ////Serial.println(sum);
        lastLine = String(currentLine);
        Serial.println(lastLine);
        lastLine = "";
        //Clear char array for next line of read
        for ( int i = 0; i < sizeof(currentLine);  ++i )
        {
          currentLine[i] = (char)0;
        }
        currentLineIndex = 0;
        break;
      }
      //if (sum == len)break;
    }
    timerEnd = millis();
    if (timerEnd - timerStart > 1000 * timeout) {
      return -1;
    }
  }

  while (Serial.available()) {
    Serial.read();
  }

  return 0;
}

