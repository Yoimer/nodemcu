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
  Para control el SMS debe ser:
  LED ON,0416, si el el número guardado en posición 1 del simcard
  empieza por 0416
  LED OFF,0416, si el el número guardado en posición 1 del simcard
  empieza por 0416
  Para registro y eliminación de usuarios:
  Registro
  ADD,posicion a guardar en sim,numero a guardar,
  Ejemplo: ADD,6,04168262667,
  DEL,posicion a borrar,
  Ejemplo: DEL,85,
*/

int8_t answer;
bool isIncontact                          = false;
bool isAuthorized                         = false;
int x;
unsigned long xprevious;
char currentLine[500]                     = "";
int currentLineIndex                      = 0;
bool nextLineIsMessage                    = false;
bool nextValidLineIsCall                  = false;
String PhoneCallingIndex                  = "";
String PhoneCalling                       = "";
String OldPhoneCalling                    = "";
String lastLine                           = "";
String phonenum                           = "";
int firstComma                            = -1;
int prende                                =  0;
int secondComma                           = -1;
String Password                           = "";
String indexAndName                       = "";
String newContact                         = "";
String trama                              = "";
int SMSerror                                 = -1;
int thirdComma                            = -1;
int forthComma                            = -1;
int fifthComma                            = -1;
int firstQuote                            = -1;
int secondQuote                           = -1;
int swveces                               = 0;
int len                                   = -1;
int j                                     = -1;
int i                                     = -1;
int f                                     = -1;
int r                                     = 0;
bool isInPhonebook = false;
char contact[13];
char phone[21]; // a global buffer to hold phone number
char message[100];

//**********************************************************

//Valores de inicialización

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // D2 como salida. D2 es GPIO-4
  pinMode(4, OUTPUT);
  Serial.begin(9600);
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
  
  // Lee primer contacto guardado previamente en el SIMcard
  // para ser ultilizado como contraseña en los SMS
  sendATcommand("AT+CPBR=1,1", "OK\r\n", 5000, 1);

  // Configura el módulo para recivir llamadas
  sendATcommand("AT+CLIP=1\r\n", "OK\r\n", 5000, 0);

  Serial.println("Password:");

  // Imprime la contraseña en la consola
  Serial.println(Password);
}


//**********************************************************

// Programa Principal
void loop()
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
// la contraseña consiste en los cuatro primeros números
// guardados en la posición 1 del sim

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
      LastLineIsCLIP();
    }
  // Comprueba que se está recibiendo un SMS
    else if (lastLine.startsWith("+CMT:"))
    {
    //Ejemplo de SMS cuando el número no está registrado
    // +CMT: "04168262667","","17/05/24,12:15:34-16"
    
    //Ejemplo de SMS cuando el número está registrado
    // y el nombre de contancto asignado es "1"
    // +CMT: "04168262667","1","17/05/24,12:15:34-16"
    
    // Los formatos de recepción dependen de la operadora
    // telefónica.
    // En algunos casos pueden incluir el códido internacional
    // del país tal como se muestra a continuación:
    
    //Ejemplo de SMS cuando el número no está registrado
    //+CMT: "+584168262667","","17/05/24,12:15:34-16"
    
    //Ejemplo de SMS cuando el número está registrado
    //y el nombre de contancto asignado es "1"
    //+CMT: "+584168262667","1","17/05/24,12:15:34-16"
    
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
      PhoneCalling      = lastLine.substring((firstComma - 12), (firstComma - 1));
      PhoneCallingIndex = lastLine.substring((firstComma + 2), (secondComma - 1));
      Serial.println(phonenum);        ////////////////////////////////////////////
      Serial.println(PhoneCallingIndex);
      j            = PhoneCallingIndex.toInt();
      isIncontact  = false;
      isAuthorized = false;
    
    // Confirma registro y autorización
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
  
  clearBuffer();
  if (isIncontact)
  {
    // SMS para encender equipo
  if (lastLine.indexOf("LED ON") >= 0)
    {
      // logica inversa
    prendeapaga(0);
    }
  // SMS para apagar equipo
    else if (lastLine.indexOf("LED OFF") >= 0)
    {
    // logica inversa
      prendeapaga(1);
    }
  // SMS para registrar usuario
    else if (lastLine.indexOf("ADD") >= 0)
    {
      DelAdd(1);
    }
  // SMS para eliminar usuario
    else if (lastLine.indexOf("DEL") >= 0)
    {
      DelAdd(2);
    }
    else
    {
      clearBuffer();
    }
  }
  CleanCurrentLine();
  nextLineIsMessage = false;
}

//**********************************************************

// Función que confirma contraseña presente en el SMS

int  prendeapaga (int siono)
{
  Serial.println("KKKKKKKKKKKKKKKKKKKKKKKKKKK");
  Serial.println(lastLine);
  firstComma    = lastLine.indexOf(',');
  secondComma   = lastLine.indexOf(',', firstComma  + 1);
  String InPassword = lastLine.substring((firstComma + 1), (secondComma));
  Serial.println(InPassword);

  if (InPassword == Password)
  {
    // LED en NODEMCU con lógica inversa
    digitalWrite(LED_BUILTIN, siono);
  
  // Relé conectado en puerto digital D2-GPIO-4
  switch (siono) {
    case 0:
      // Desactiva el relé
      digitalWrite(4, HIGH);
    
      // Copia número en array phone
      phonenum.toCharArray(phone, 21);
    
      // Envía SMS de confirmación 
      sendSMS(phone, "Rele activado!");
      break;
    case 1:
      // Activa el relé
      digitalWrite(4, LOW);
    
      // Copia número en array phone
      phonenum.toCharArray(phone, 21);
    
      // Envía SMS de confirmación 
      sendSMS(phone, "Rele desactivado!");
      break;
    default:
    break;
}

  }
  clearBuffer();
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

// Función que Registra y Borra usuario

int DelAdd(int DelOrAdd)
{
  indexAndName = "";
  char aux_string[100];
  firstComma          = lastLine.indexOf(',');
  secondComma         = lastLine.indexOf(',', firstComma  + 1);
  thirdComma          = lastLine.indexOf(',', secondComma + 1);
  indexAndName = lastLine.substring((firstComma + 1), (secondComma));
  newContact = "";
  newContact   = lastLine.substring((secondComma + 1), thirdComma);
  if (!isAuthorized)
  {
    Serial.println(j); //////////////////////////////////////////////////////////////
    Serial.println("Not authorized to Delete/Add"); /////////////////////////////////
    return 0;
  }
  String tmpx;
  
  // Comando AT para agregar y borrar usuarios en el SIM
  
  //Agregar
  // AT+CPBW=posicion en sim,"numero a guadar",129,"nombre de contacto"
  // 129 significa que el numero a guardar es nacional
  // sin incluir el formato telefónico internacional ejemplo +58
  // AT+CPBW=1,"04168262667",129,"1" guarda ese número en la
  // posición 1 del sim y el nombre asignado es un ID de valor 1.
  // cada ID está asignado en la base de datos con el nombre
  // del usuario correspondiente
  
  //Borrar
  // AT+CPBW=posicion en sim
  // AT+CPBW=30 borraría posición 30 en el sim

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
  // llamar a confirmSMS() para decir que se registró
  // o se eliminó con éxito
  confirmSMS(DelOrAdd);
  }
  else
  {
    Serial.println("error ");
  
  // Error de registro
  if (DelOrAdd == 1)
  {
    SMSerror = 1;
  }
  // Error de eliminación
    else if (DelOrAdd == 2)
  {
    SMSerror = 2;
  }
  Serial.println("Va a rutina de error");
  // llamar a confirmSMS() para decir que tipo de error hubo
  confirmSMS(3);
  }
  clearBuffer();
}


//**********************************************************

// Función que envía SMS 

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

// Función que confirma registro,eliminación
// o error vía SMS 

void confirmSMS(int DelOrAdd )
{
  switch (DelOrAdd) {
    // Confirma registro exitoso
    case 1:
      trama = "";
      trama = "El numero: " + newContact + " ha sido registrado con exito en la posicion: " + indexAndName;
      tramaSMS(phonenum, trama); // Envía SMS de confirmación 

      trama = "";
      trama = "Bienvenid@. Su numero fue registrado exitosamente.";
      tramaSMS(newContact, trama); // Envía SMS de confirmación 
      break;
    // Confirma eliminación exitosa
    case 2:
      trama = "";
      trama = "El numero registrado en la posicion: " + indexAndName + " ha sido eliminado exitosamente ";
      tramaSMS(phonenum, trama); // Envía SMS de confirmación 
      break;
    // Reporta error
    case 3:
      Serial.println("On case 3 ");
      Serial.println("Value of DelOrAdd: ");
      Serial.println(DelOrAdd);
      switch (SMSerror) {
        // Error de registro
        case 1:
          trama = "";
          trama = "No se pudo registrar el numero. Revise el formato del mensaje por favor";
          tramaSMS(phonenum, trama); // Envía SMS de confirmación 
          break;
        // Error de eliminación
        case 2:
          trama = "";
          trama = "No se pudo eliminar el numero. Revise el formato del mensaje por favor";
          tramaSMS(phonenum, trama); // Envía SMS de confirmación 
          break;
        default:
        break;
      }
      break;
    default:
    break;
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

// Función que procesa llamada telefónica

void LastLineIsCLIP()
{
  firstComma         = lastLine.indexOf(',');
  secondComma        = lastLine.indexOf(',', firstComma + 1);
  thirdComma         = lastLine.indexOf(',', secondComma + 1);
  forthComma         = lastLine.indexOf(',', thirdComma + 1);
  fifthComma         = lastLine.indexOf(',', forthComma + 1);
  PhoneCalling       = lastLine.substring((firstComma - 12), (firstComma - 1));
  PhoneCallingIndex  = lastLine.substring((forthComma + 2), (fifthComma - 1));
  j                  = PhoneCallingIndex.toInt();

  Serial.println(PhoneCalling);

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
    PhoneCalling.toCharArray(phone, 21);
    sendSMS(phone, "desactivada");
    clearBuffer();
  }
  clearBuffer();
  nextValidLineIsCall = false;
}

//**********************************************************