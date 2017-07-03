/*
 ESP8266 Blink by Simon Peter
 Blink the blue LED on the ESP-01 module
 This example code is in the public domain
 
 The blue LED on the ESP-01 module is connected to GPIO1 
 (which is also the TXD pin; so we cannot use Serial.print() at the same time)
 
 Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/

unsigned long timeOn = 3L * 1000L * 60L; // delay between updates, 3 minutes
unsigned long timeOff = 1L * 1000L * 60L; // delay between updates, 1 minutes

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(D2, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, LOW);     // Turn the LED on (Note that LOW is the voltage level
                                     // but actually the LED is on; this is because 
                                    // it is active low on the ESP-01)
                                    
  digitalWrite(D2, HIGH);            // Digital 2 HIGH                        
  //delay(10000);                      // Wait for a second
  delay(timeOn);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  digitalWrite(D2, LOW);             //Digital 2 LOW  
  //delay(20000);                     // Wait for two seconds (to demonstrate the active low LED)
  delay(timeOff);
}
