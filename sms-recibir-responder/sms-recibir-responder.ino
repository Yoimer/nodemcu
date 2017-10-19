/*
	Recibe y procesa SMS basados en strings predefinidos
	incluyendo una contraseña para tomar acciones de control,
	registro y eliminación de usuarios y monitoreo
	de temperatura con el sensor DS18B20 usando las
	librerías Dallas Temperature y OneWire.
	Solo los 5 primeros contactos en el simcard
	pueden agregar y eliminar usuarios.
	Solo lo usuarios registrados en la "whitelist"
	pueden tomar acciones de control y consultar temperatura
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
	Para monitoreo de temperatura el comando es:
	TEMP?
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
String temperatureString                  = "";
String BuildString                        = "";
String id                                 = "";
String tmpx                               = "";
int SMSerror                              = -1;
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
char aux_string[100];


// Inclusión de librerías de medición de temperatura
// Dallas Temperature y OneWire

// Tutorial de instalación de librerías para Arduino - Nodemcu
//https://www.prometec.net/librerias/#

// Enlace de descarga de librería Dallas Temperature
// https://codeload.github.com/milesburton/Arduino-Temperature-Control-Library/zip/master

#include <OneWire.h>

#include <DallasTemperature.h>

#define ONE_WIRE_BUS D1

#define LOW_LEVEL_TEMPERATURE 28.50

#define HIGH_LEVEL_TEMPERATURE 31.50

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

//Conectiones entre Nodemcu y DS18B20
//NodeMCU 3v3 con Vin de DS18B20
//NodeMCU D1  con Data o Signal de DS18B20
//NodeMCU GND con GND de DS18B20

// Inclusión de librerías WIFI para ESP8266
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;


// Inclusión de plataforma Thing Speak

// Servidor
const char* host = "api.thingspeak.com";

// Clave suministrada por Thing Speak
String ApiKey = "PJ38C84LVKOUKTSF"; 

// Ruta para publicar
String path = "/update?key=" + ApiKey + "&field1="; 


//**********************************************************

//Valores de inicialización

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // D2 como salida. D2 es GPIO-4
  pinMode(4, OUTPUT);
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
  sendATcommand("AT+CPBR=1,1", "OK\r\n", 5000, 1);
  
  Serial.println("Password:");
  
  // Imprime la contraseña en la consola
  Serial.println(Password);
  
  // Conecta con router Wifi
  //WiFiMulti.addAP("Casa","remioy2006202");
  WiFiMulti.addAP("FARC-ELN-ISIS","remioyroman");
}

//**********************************************************

// Función que mide temperatura

float getTemperature() {
  float temp;
  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

//**********************************************************

// Programa Principal

void loop()
{

  unsigned long previous = millis();

  do
  {
	  // Si hay salida serial desde el SIM800
	  if (Serial.available() > 0)
	  {
		char lastCharRead = Serial.read();
		
		// Lee cada caracter desde la salida serial hasta que \r o \n is encontrado (lo cual denota un fin de línea)
		if (lastCharRead == '\r' || lastCharRead == '\n')
		{
		  endOfLineReached();
		}

		else
		{
		  currentLine[currentLineIndex++] = lastCharRead;
		}
      }
  }while((millis() - previous) < 5000);  // espera actividad en puerto serial for 5 segundos

  // Mide temperatura
  float temperature = getTemperature();
  temperatureString = "";
  temperatureString = String(getTemperature());
  // imprime en consola el valor de temperatura
  Serial.println(temperatureString);
  
  // envía SMS cuando la temperatura sobre pasa los umbrales
  // cada 5 segundos a un número telefónico predefinido
  if ((temperature >= 28.50) && (temperature < 31.50))
  {
	trama = "";
	trama = "Alerta, la temperatura ha alcanzado el valor de: " + temperatureString + " grados Centigrados";
	tramaSMS("04129501619", trama);
  }
  // apaga el equipo hasta que la temperatura alcance
  // un valor por debajo de 31.50
  // si se envía un SMS con el comando de encendido
  // pero la temperatura es mayor a 31.50
  // el comando no va a sobremandar el sistema
  else if (temperature >= 31.50)
  {
	trama = "";
	trama = "El equipo se ha apagado debido a que la temperatura alcanzo el valor de: " + temperatureString + " grados Centigrados";
    // desactiva el relé con lógica inversa
	digitalWrite(4, HIGH);
	tramaSMS("04129501619", trama);
  }
  

  
  //Publica temperatura a Thing Speak
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET ") + path + temperatureString + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
			   "Connection: keep-alive\r\n\r\n");


  // Conecta a internet para consultar o publicar resultados
   //consulta en servidor web privado
   GetInfoFromWeb(-1); 
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
        Serial.println("en phonebook");
        if (j <= 5 )
        {
          Serial.println("autorizada");
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
	// SMS para consultar temperatura
	// solo los 5 primeros números registrados pueden
	// leer temperatura
    else if (lastLine.indexOf("TEMP?") >= 0)
    {
	  getTemperatureSMS();
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
			// Activa el relé con lógica inversa
			digitalWrite(4, LOW);
	  
			// Copia número en array phone
			phonenum.toCharArray(phone, 21);
	  
			// Envía SMS de confirmación 
			sendSMS(phone, "Rele activado!");
			
			pushData("ON");
			
			break;
		case 1:
			// desactiva el relé con lógica inversa
			digitalWrite(4, HIGH);
	  
			// Copia número en array phone
			phonenum.toCharArray(phone, 21);
	  
			// Envía SMS de confirmación 
			sendSMS(phone, "Rele desactivado!");
			
			pushData("OFF");
			
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
	// desactiva el relé con lógica inversa
	digitalWrite(4, HIGH);
  }
  if ((WiFiMulti.run() == WL_CONNECTED) & swveces == 0)
  {
    HTTPClient http;
    String xp = "http://estredoyaqueclub.com.ve/arduinoenviacorreo.php?telefono=" + PhoneCalling + "-" + PhoneCallingIndex;
	Serial.println(xp);
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



//**********************************************************

// Función que Registra y Borra usuario

int DelAdd(int DelOrAdd)
{
  indexAndName = "";
  ////char aux_string[100];
  firstComma          = lastLine.indexOf(',');
  secondComma         = lastLine.indexOf(',', firstComma  + 1);
  thirdComma          = lastLine.indexOf(',', secondComma + 1);
  indexAndName = lastLine.substring((firstComma + 1), (secondComma));
  newContact = "";
  newContact   = lastLine.substring((secondComma + 1), thirdComma);
  
  // confirma que está entre los 5 primeros usuarios del sim
  if (!isAuthorized)
  {
    Serial.println(j);
    Serial.println("Not authorized to Delete/Add");
    return 0;
  }
  
  // Limpia variable temporal
  tmpx = "";
  
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
  answer = sendATcommand(aux_string, "OK", 20000, 0);
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

// Función que obtiene temperatura del sensor DS18B20
// y envía SMS

void getTemperatureSMS()
{   
    // confirma que está entre los 5 primeros usuarios del sim
	if (!isAuthorized)
    {
		Serial.println(j);
		Serial.println("No autorizado para consultar temperatura");
    }
	else
	{
		
		trama = "";
		trama = "El valor de temperatura es: " + temperatureString + " grados Celsius";
		tramaSMS(phonenum, trama);
	}
}


//**********************************************************

// Función que se comunica con servidor web y
// verifica comandos de control y registro

int GetInfoFromWeb (int router)
{
//delay(10000);
delay(5000);
String xp;
if((WiFiMulti.run() == WL_CONNECTED) ) 
  {  
	Serial.println("[++++++GetInfoFromWeb+++++++");

  // Servidor web local virtual
  // Debe ser uno real conectado a internet
  //xp = "http://192.168.0.164/sandbox/whitelist.txt";
  //xp = "http://192.168.5.107/sandbox/whitelist.txt";
  //xp = "http://98cc57cb.ngrok.io/sandbox/whitelist.txt";
  xp = "http://192.168.5.107/sandbox/whitelist.txt";
  Serial.println(xp);
  HTTPClient http;
  http.begin(xp);
  int httpCode = http.GET();
  if(httpCode > 0)
  {
  if(httpCode == HTTP_CODE_OK) 
    {
		BuildString = http.getString();
		Serial.println(BuildString);

		// String que viene desde el servidor a modo de espera
		// +9999#99999999999$SMS*AA/position/
   
		// String que viene desde el servidor para tomar acción
		//+9999#99999999999$SMS*2/35/
		//9999                -> ID en base de datos
		//99999999999         -> Celular de 11 dígitos que recibe el mensaje o que va a ser agregado o eliminado del sistema
		//SMS                 -> Contenido del mensaje
		//2                   -> Acción que se toma en el sistema
        //35                  -> Posición en el SIM

		
		//0                   -> Activa Relé por lógica inversa
		//1                   -> Desactiva Relé por lógica inversa
		//2                   -> Agrega número en el SIM
		//3                   -> Borra número en el SIM
		//cada acción debe documentarse acá
		
		char msgx[1024];
		char telx[1024];

		// Extrae ID de la base de datos
		id             = BuildString.substring(BuildString.indexOf("+")+1,BuildString.indexOf("#"));

		// Extrae número telefónico que recibirá el SMS o número que va a ser agregado o eliminado del sistema
		String tel     = BuildString.substring(BuildString.indexOf("#")+1,BuildString.indexOf("$"));
		
		// Extrae SMS
		String msg     = BuildString.substring(BuildString.indexOf("$")+1,BuildString.indexOf("*"));
		
		// Extrae acción a tomar en el sistema
		String action  = BuildString.substring(BuildString.indexOf("*")+1,BuildString.indexOf("/"));
		
		// Extrae position del SIM que será agregada o eliminada del sitema
		String add_del     = BuildString.substring((BuildString.indexOf("/") + 1), BuildString.indexOf("/", BuildString.indexOf("/") + 1));

		Serial.println("id :"+id);
		Serial.println("tel:"+tel);
		Serial.println("msg:"+msg);
		Serial.println("action:"+action);
		Serial.println("add_del:"+add_del);
		
		//Formato de mensaje presente en el servidor
		//+9999#99999999999$SMS*AA/
		if ( action != "AA")
		{
			strcpy(telx, tel.c_str());
			strcpy(msgx, msg.c_str());
			int control = action.toInt();
			Serial.println(control);
			// control de relé desde internet
			// Relé conectado en puerto digital D2-GPIO-4
			switch (control)
			{
				// Activa el relé con lógica inversa 
				case 0:
					Serial.println("Case 0");
					//LED en NODEMCU con lógica inversa
					digitalWrite(LED_BUILTIN, router);
					digitalWrite(4, LOW);
					break;
				// Desactiva el relé con lógica inversa
				case 1:
					Serial.println("Case 1");
					//LED en NODEMCU con lógica inversa
					digitalWrite(LED_BUILTIN, router);
					digitalWrite(4, HIGH);
					break;
				// Agrega número en SIM
				case 2:
					Serial.println("Case 2");
					
					// Limpia variable temporal
					tmpx = "";

					// Limpia indexAndName
					indexAndName = "";
					
					// Limpia newContact
					newContact = "";

					// Asigna posición del sim que se va a incorporar en el sistema
					indexAndName = add_del;

					// Asigna número telefónico que se va a incorporar en el sistema
					newContact = tel;

					// Guarda número en SIM
					tmpx = "AT+CPBW=" + indexAndName + ",\"" + newContact + "\"" + ",129," + "\"" + indexAndName + "\"" + "\r\n\"";
					tmpx.toCharArray( aux_string, 100 );
					answer = sendATcommand(aux_string, "OK", 20000, 0);
					if (answer == 1)
					{
						Serial.println("Agregado en el sistema ");
					}
					else
					{
						Serial.println("Error, verifique formato del string ");
					}
					break;
				// Borra número en SIM
				case 3:
					Serial.println("Case 3");

					// Limpia variable temporal
					tmpx = "";

					// Limpia indexAndName
					indexAndName = "";

					// Asigna posición del sim que se va a borrar
					indexAndName = add_del;
					
					tmpx = "AT+CPBW=" + indexAndName + "\r\n\"";
					tmpx.toCharArray( aux_string, 100 );
					answer = sendATcommand(aux_string, "OK", 20000, 0);
					if (answer == 1)
					{
						Serial.println("Borrado del sistema");
					}
					else
					{
						Serial.println("Error, verifique formato del string ");
					}
					break;
				default:
				break;
			}
			sendSMS  (telx,msgx) ;
		}
    }
  } 
  else 
  {
	Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
	http.end();
  }   
}


//**********************************************************

// Función que publica estado del relé en estredoyaque club
// 

void pushData(String ONOFF)
{
	if ((WiFiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
	//String xp = "http://estredoyaqueclub.com.ve/arduinoenviacorreo.php?";
	String xp = "http://estredoyaqueclub.com.ve/arduinoenviacorreo.php?telefono=" + phonenum + "-" + ONOFF;
	Serial.println(xp);
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
	  else
	  {
		Serial.print("Code: ");
		Serial.println(httpCode);
		Serial.println("NOT OK!");
	  }
    }
    else
    {
      Serial.println("Server is down...");
	  Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  clearBuffer();
}
