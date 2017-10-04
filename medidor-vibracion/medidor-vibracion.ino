// Watch video here: https://www.youtube.com/watch?v=235BLk7vk00

/* Vibration sensor connected to Arduino pins as follows:
 Arduino            Vibration Sensor
   D9                    DOut
   D1                    DOut
   GND                   GND
   +5V                   VCC     
   
   D13                Indication LED
   LED_BUILTIN        Indication LED
*/

//int ledPin = 13;
//int EP =9;
int EP = D1;
void setup(){
  //pinMode(ledPin, OUTPUT)
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  
  pinMode(EP, INPUT); //set EP input for measurment
  Serial.begin(115200); //init serial 115200
  Serial.println("----------------------Vibration demo------------------------");
}
void loop(){
  long measurement =TP_init();
  delay(50);
 // Serial.print("measurment = ");
  Serial.println(measurement);
  if (measurement > 1000){
    //digitalWrite(ledPin, HIGH);
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is active low on the ESP-01)
  }
  else{
    //digitalWrite(ledPin, LOW);
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

long TP_init(){
  delay(10);
  long measurement=pulseIn (EP, HIGH);  //wait for the pin to get HIGH and returns measurement
  return measurement;
}
