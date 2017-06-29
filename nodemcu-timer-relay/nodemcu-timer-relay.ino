
unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
unsigned long postingInterval = 3L * 1000L * 60L; // delay between updates, 20 minutes
// the "L" is needed to use long type numbers 1000*60*10;  ten minutes


void setup() {
  // start serial port:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW); // Activates Relay (it is active low on the ESP-01)
}

// the loop function runs over and over again forever
void loop() {

  Serial.println("On Loop!!");
 
 // if 20 minutes have passed since your last connection,
  // then connect again and change relay state:
  if (millis() - lastConnectionTime > postingInterval) {
    //httpRequest();
    changeRelayState();
  }
}

void changeRelayState(){
  Serial.println("On changeRelayState");
  digitalWrite(LED_BUILTIN, HIGH); // Deactivates relay 
  lastConnectionTime = 0;
  postingInterval = 1L * 1000L * 60L; // 1 minutes
  do
  {
  }while(millis() - lastConnectionTime > postingInterval);
  digitalWrite(LED_BUILTIN, LOW); // Activates Relay (it is active low on the ESP-01)
  postingInterval = 3L * 1000L * 60L;
}

