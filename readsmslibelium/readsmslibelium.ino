int8_t answer;
int x;
char aux_string[30];
char SMS[200];


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Starting...");
  power_on();
  delay(3000);
  Serial.println("Setting SMS mode...");
  sendATcommand("AT+CMGF=1", "OK", 1000);    // sets the SMS mode to text
  sendATcommand("AT+CPMS=\"SM\",\"SM\",\"SM\"", "OK", 1000);    // selects the memory

  answer = sendATcommand("AT+CMGR=1", "+CMGR:", 2000);    // reads the first SMS
  if (answer == 1)
  {
    answer = 0;
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
      }
    } while (answer == 0);   // Waits for the asnwer with time out

    SMS[x] = '\0';

    Serial.print(SMS);

  }
  else
  {
    Serial.print("error ");
    Serial.println(answer, DEC);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
void power_on() {

  uint8_t answer = 0;

  // checks if the module is started
  answer = sendATcommand("AT", "OK", 2000);
  if (answer == 0)
  {
    // power on pulse
    //digitalWrite(onModulePin, HIGH);
    delay(3000);
    //digitalWrite(onModulePin, LOW);

    // waits for an answer from the module
    while (answer == 0) {   // Send AT every two seconds and wait for the answer
      answer = sendATcommand("AT", "OK", 2000);
    }
  }

}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialice the string

  delay(100);

  while ( Serial.available() > 0) Serial.read();   // Clean the input buffer

  Serial.println(ATcommand);    // Send the AT command


  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    // if there are data in the UART input buffer, reads it and checks for the asnwer
    if (Serial.available() != 0) {
      response[x] = Serial.read();
      x++;
      // check if the desired answer is in the response of the module
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;
      }
    }
    // Waits for the asnwer with time out
  } while ((answer == 0) && ((millis() - previous) < timeout));

  return answer;
}

