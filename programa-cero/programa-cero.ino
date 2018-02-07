/*
  Recibe y procesa SMS basados en strings predefinidos
  incluyendo una contraseña para tomar acciones de control,
  registro y eliminación de usuarios.
  Solo los 5 primeros contactos en el simcard
  pueden agregar y eliminar usuarios.
  Solo lo usuarios registrados en la "whitelist"
  pueden tomar acciones de control.
  Cualquier SMS distinto a los strings predefinidos
  no serán tomados en cuenta por el sistema


  NOTA!!! (al ejecutar este programa se borraran todos los contactos guardados previamente)
  
  formato del SMS:
  KEY,4 números que se desean guardar,
  por ejemplo el número 04168262668 envía el SMS KEY,7777,
  en la posición 1 del sim se guarda la clave 7777
  en la posición 2 el número 04168262668.


*/

int8_t answer;
bool isIncontact                              = false;
bool isAuthorized                             = false;
int x;
unsigned long xprevious;
char currentLine[500]                         = "";
int currentLineIndex                          = 0;
bool nextLineIsMessage                        = false;
bool nextValidLineIsCall                      = false;
String PhoneCallingIndex                      = "";
String PhoneCalling                           = "";
String OldPhoneCalling                        = "";
String lastLine                               = "";
String phonenum                               = "";
String indexAndName                           = "";
String tmpx                                   = "";
String trama                                  = "";
int firstComma                                = -1;
int prende                                    = 0;
int secondComma                               = -1;
String Password                               = "";
int thirdComma                                = -1;
int forthComma                                = -1;
int fifthComma                                = -1;
int firstQuote                                = -1;
int secondQuote                               = -1;
int swveces                                   = 0;
int len                                       = -1;
int j                                         = -1;
int i                                         = -1;
int f                                         = -1;
int r                                         = 0;
bool isInPhonebook                            = false;
char contact[13];
char phone[21];
char message[100];

//**********************************************************

//Valores de inicialización

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Starting...");
  
  // Comprueba conexión física con el SIM800L
  power_on(); 
  delay(3000);
  Serial.println("Connecting to the network...");
  
  // Comprueba conexión con red celular
  while ( (sendATcommand("AT+CREG?", "+CREG: 0,1", 5000, 0) ||
           sendATcommand("AT+CREG?", "+CREG: 0,5", 5000, 0)) == 0 );

  // Configura lectura de SMS como caracteres ASCII
  sendATcommand("AT+CMGF=1", "OK", 5000, 0);
  
  // Configura lectura de SMS solo en el buffer serial
  // y no guarda los SMS en la SIMcard
  sendATcommand("AT+CNMI=1,2,0,0,0", "OK", 5000, 0);
  
}


//**********************************************************

// Programa Principal
void loop()
{
  CheckSIM800L();
}

//**********************************************************

// Función que comprueba conexión física con el SIM800L
void power_on()
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

//**********************************************************

// Función que envía comandos AT al SIM800L
// Cuando int xpassword tiene el valor de 0 no consulta contraseña
// Cuando int xpassword tiene el valor de 1 consulta contraseña

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout, int xpassword)
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

//**********************************************************

// Función que lee el fin de linea en el puerto serial

void endOfLineReached()

{
  lastLine = String(currentLine);

  // Comprueba que se está recibiendo una llamada
  if (lastLine.startsWith("RING"))
  {
    Serial.println(lastLine);
    nextValidLineIsCall = true;
  }
  else
  {
    if ((lastLine.length() > 0) && (nextValidLineIsCall))
    {
      //LastLineIsCLIP();
    }
  // Comprueba que se está recibiendo un SMS
    else if (lastLine.startsWith("+CMT:"))                         
    {
      // Imprime SMS recibido completo
    // Incluye el número que envía el mensaje
    Serial.println(lastLine);

    // Extrae número telefónico
      phonenum = lastLine.substring((lastLine.indexOf(34) + 1),
                                    lastLine.indexOf(34, lastLine.indexOf(34) + 1));
      nextLineIsMessage = true;
      firstComma        = lastLine.indexOf(',');
      secondComma       = lastLine.indexOf(',', firstComma  + 1);
      thirdComma        = lastLine.indexOf(',', secondComma + 1);
      forthComma        = lastLine.indexOf(',', thirdComma  + 1);
      fifthComma        = lastLine.indexOf(',', forthComma  + 1);
      //PhoneCalling      = lastLine.substring((firstComma - 12), (firstComma - 1));
      PhoneCallingIndex = lastLine.substring((firstComma + 2), (secondComma - 1));
      Serial.println(phonenum);
    }
    else if ((lastLine.length() > 0) && (nextLineIsMessage))
    {
      // Procesa SMS
    LastLineIsCMT();
    }
  }
  // Limpia buffer
  CleanCurrentLine();
}

//**********************************************************

// Función que limpia línea

void CleanCurrentLine()

{
  for ( int i = 0; i < sizeof(currentLine);  ++i )
  {
    currentLine[i] = (char)0;
  }
  currentLineIndex = 0;
}

//**********************************************************

// Función que procesa el SMS

void LastLineIsCMT()
{
  // Solo el contenido del SMS
  Serial.println(lastLine);
  
  //extrae la clave
  // ejemplo de un SMS KEY,0007,
  firstComma        = lastLine.indexOf(',');
  secondComma       = lastLine.indexOf(',', (firstComma + 1));
  String key        = lastLine.substring((firstComma + 1), (secondComma));
  Serial.println(key);

  clearBuffer();
  
    // SMS para ingresar clave
  if (lastLine.indexOf("KEY") >= 0)
    {
		deleteAllContacts();
		addContact("2",phonenum);
		addContact("1",key);
		trama = "";
		trama = "Su numero ha sido registrado exitosamente y la clave principal es: " + key;
		tramaSMS(phonenum, trama);
    }
    else
    {
      clearBuffer();
    }
  CleanCurrentLine();
  nextLineIsMessage = false;
}

//**********************************************************

// Función que limpia todo el buffer

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

//**********************************************************

// Función que borrar todos los contactos del sim

void deleteAllContacts()
{

	char aux_string[100];
	// para simcard de 250 contactos
	for (i = 1; i < 251; i++)
	{
		indexAndName = "";
		indexAndName = String(i);
		tmpx = "";
		tmpx = "AT+CPBW=" + indexAndName + "\r\n\"";
		//Serial.println(tmpx);
		tmpx.toCharArray( aux_string, 100 );
		Serial.println(aux_string);
		answer = sendATcommand(aux_string, "OK", 20000, 0); // envía comando AT
		if (answer == 1)
		{
			Serial.println("Eliminado ");
		}
		else
		{
			Serial.println("error ");
		}
	}
}

//**********************************************************

// Función que agrega número telefónico
// en la posición pre-establecida en la variable position

void addContact(String position, String number)
{
	char aux_string[100];
	tmpx = "AT+CPBW=" + position + ",\"" + number + "\"" + ",129," + "\"" + position + "\"" + "\r\n\"";
	//Serial.println(tmpx);
	tmpx.toCharArray( aux_string, 100 );
	//Serial.println(aux_string);
	answer = sendATcommand(aux_string, "OK", 20000, 0); // envía comando AT
	if (answer == 1)
	{
		Serial.println("Agregado ");
		}
		else
		{
			Serial.println("error ");
		}
}

//**********************************************************

// Función que arma trama de mensaje para enviar notificación
// via SMS

void tramaSMS(String numbertoSend, String messagetoSend)
{
	// Copia número en array phone
	strcpy(phone,numbertoSend.c_str());

	// Convierte trama en mensaje
	strcpy(message, messagetoSend.c_str());

	// Envía SMS de confirmación 
	sendSMS(phone, message);
}

//**********************************************************

// Funcion qne envia SMS

int sendSMS(char *phone_number, char *sms_text)
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

//**********************************************************

// Funcion qne chequea SIM800L

void CheckSIM800L()
{
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
