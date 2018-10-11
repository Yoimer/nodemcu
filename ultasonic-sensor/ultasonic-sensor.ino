/* -------------------------------------------------------------conexiones----------------------------------------------- */

/*fuente externa de 12VDC/3A a MP1584

MP1584
 ----------------------
|-in             out - |
|    -------->         |
|+in             out + |
 ----------------------

-in (negativo de fuente externa)
+in (positivo de fuente externa)

// mover perilla hasta 3.3VDC
-out (negativo de salida)
+ out (positivo de salida)

*/


/* MP1584 - SIM800L CoreBoard

   -out----->gnd
   +out----->vcc
*/


/* SIM800L - Nodemcu

   rx------->tx
   tx------->rx
   gnd------>cualquier gnd (otra tierra disponible debe ir conectada a la salida negativa del MP1584)
/*


/* HC-SR04 - Nodemcu
   
   vcc------->vin
   gnd------->cualquier gnd
   trigg----->D4
   echo------>D3
/*


/* RELÊ(5VDC) - Nodemcu

   sig-------->D2
   vcc-------->vin
   gnd-------->cualquier gnd

*/

/*
  formatos de mensajes

  activar alarma (bandera de programación)
  999,4 números grabados en la posición 1 del simcard con el programa cero,
  ejemplo 999,7777,

  forzar relé físico
  000,4 números grabados en la posición 1 del simcard con el programa cero,
  ejemplo 000,7777,

  el programa cero es aquel que permite ingresar la clave en la posicion 1 del simcard
  la cual consiste en 4 números. Al mismo tiempo el número telefónico
  que envía el mensaje queda registrado en la posición 2 del sim card.
  El enlace del programa cero se puede encontrar acá:
  https://raw.githubusercontent.com/Yoimer/nodemcu/b731edd3bafc2de79cd517fb324269fd1d791981/programa-cero/programa-cero.ino

  solo los números guardados desde la posición 2 hasta la 5
  pueden agregar o borrar usuarios al systema (whitelist)
  para agregar usuarios usar el siguiente formato de sms
  ADD,posición a guardar,número telefónico,

  ejemplo ADD,6,04168262668,
  el número 04168262668 queda guardado en la posición 6 del simcard
  el nuevo ingreso recibirá un sms de bienvenida
  el número que realizó el ingreso recibirá un sms de confirmación.

  para borrar usuarios ingresar el siguiente formato de sms
  DEL,posición del sim a borrar,
  por ejemplo
  DEL,36, borra el número guardado en la posición 36 del simcard

  se debe tener guardado en una base de datos u hoja en excel
  los números y posiciones(nombres) asociadas al sistema
  cuando se envía el SMS ADD,6,04168262668,
  el nombre de ese contacto es 6, el cual es la posición donde fue guardado en el sim

*/

/////////comandos////////

// LED ON,password,
// activa relé en D2 (GPIO-4)

// LED OFF,password,
// desactiva relé en D2 (GPIO-4)

// ADD,posicion,
// agrega usuario 

// DEL,posicion,
// elimina usuario

// 999,password,
// activa alarma

// 000,password,
// enciende alarma (forza relé)

// para apagar alarma
// llamar al sim800

#define TIMEOUT 30000

// defines pins numbers
const int trigPin = 2;  //D4
const int echoPin = 0;  //D3

// defines variables
long duration;
int distance;
int onModulePin = 13;
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
char phone[21];
char message[100];
int alarma = -1;
int hasPassword = -1;


void setup()
{
    // Led indicador de relé físico
    pinMode(LED_BUILTIN, OUTPUT);

    // Led indicador de relé físico apagado(lógica inversa)
    digitalWrite(LED_BUILTIN, HIGH);

    // D2 como salida. D2 es GPIO-4
    pinMode(4, OUTPUT);

    // set trigPin como salida
    pinMode(trigPin, OUTPUT); 

    // sets echoPin como entrada
    pinMode(echoPin, INPUT);

    // inicia comunicación serial
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

    // Lee primer contacto guardado previamente en el SIMcard
    // para ser ultilizado como contraseña en los SMS
    sendATcommand("AT+CPBR=1,1", "OK", 5000, 1);

    // Imprime la contraseña en la consola
    Serial.println("Password:");
    Serial.println(Password);

    // configura modulo para recibir llamadas
    sendATcommand("AT+CLIP=1", "OK", 5000, 0);
}

void loop()
{
    // monitorea movimiento
    CheckUltrasoundSensor();

    // monitorea SIM800L
    CheckSIM800L();

    // monitorea distancia y activación de alarma via SMS
    CheckDistance();
}

////////////////////////////////////////////////////
void CheckUltrasoundSensor()
{
    // limpia trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // set trigPIN en HIGH por 10 microsegundos
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // lee echoPin, retorna el viaje en el tiempo de la onda de sonido en microsegundos
    duration = pulseIn(echoPin, HIGH);

    // calcula la distancia
    distance= (duration*0.034)/(2);

    // imprime distancia en consola
    Serial.print("Distance is: ");
    Serial.println(distance, DEC);
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
// la contraseña es los 4 digitos que se guardaron previamente
// en la posicion 1 en el sim
// utilizando el programa de arranque llamado programa cero

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
    // SMS para activar alarma
    else if (lastLine.indexOf("999") >= 0)
    {

      hasPassword = CheckPassword();
      if (hasPassword == 1)
      {
        // Copia número en array phone
        phonenum.toCharArray(phone, 21);

        // Envía SMS de confirmación 
        sendSMS(phone, "Alarma Activada");

        //activa alarma
        alarma = 1;
        Serial.print("alarma");
        Serial.println(DEC, alarma);
      }
      
    }
    // SMS para activar forzar relé
    else if (lastLine.indexOf("000") >= 0)
    {
      hasPassword = CheckPassword();
      if (hasPassword == 1)
      {
        // forza relé(led) en nodemcu
        digitalWrite(LED_BUILTIN, LOW);

        // enciende relé físico en D2
        digitalWrite(4, HIGH);

        // Copia número en array phone
        phonenum.toCharArray(phone, 21);

        // Envía SMS de confirmación 
        sendSMS(phone, "RELE FORZADO");

        // limpia la alarma
        alarma = 0;
        Serial.print("alarma");
        Serial.println(DEC, alarma);
      }
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
            ////digitalWrite(4, LOW);
      
            // Copia número en array phone
            phonenum.toCharArray(phone, 21);
      
            // Envía SMS de confirmación 
            sendSMS(phone, "Rele activado!");
            break;
        case 1:
            // Activa el relé
            /////digitalWrite(4, HIGH);
      
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

    // desactiva el relé físico en D2
    digitalWrite(4, LOW);

    alarma = 0;

     // Copia número en array phone
    PhoneCalling.toCharArray(phone, 21);

    // Envía SMS de confirmación 
    sendSMS(phone, "Alarma desactivada por llamada");

  }
  clearBuffer();
  nextValidLineIsCall = false;
}

//**********************************************************


//**********************************************************

// Función que procesa SIM800L
void CheckSIM800L()
{
  unsigned long previous = millis();
    do
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
    }while((millis() - previous) < 150);  // espera actividad serial por 150 milisegundos
}

//**********************************************************

// Función que procesa distancia

void CheckDistance()
{
  if ((distance <= 30) && (alarma == 1))
    {
        // activa relé (logica inversa en nodemcu)
        digitalWrite(LED_BUILTIN, LOW);

        // enciende relé físico en D2
        digitalWrite(4, HIGH);

        // Copia número en array phone
        phonenum.toCharArray(phone, 21);

        // Envía SMS de confirmación 
        sendSMS(phone, "Alarma activada por movimiento");

      //desactiva alarma
      alarma = 0;
      Serial.print("alarma");
      Serial.println(DEC, alarma);

    }
}

//**********************************************************

// Función que verifica password

int CheckPassword()
{
  int hasPassword = -1;
  Serial.println("Verificando Password....");
  Serial.println(lastLine);
  firstComma    = lastLine.indexOf(',');
  secondComma   = lastLine.indexOf(',', firstComma  + 1);
  String InPassword = lastLine.substring((firstComma + 1), (secondComma));
  Serial.println(InPassword);

  if (InPassword == Password)
  {
    hasPassword = 1;
    Serial.println("Password correcto");
  }
  else
  {
    hasPassword = 0;
    Serial.println("Password incorrecto!!!");
  }

  return hasPassword;
}
