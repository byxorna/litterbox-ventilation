/* 
  IR Breakbeam sensor demo!

  Wire up the signal pin of the 5mm IR sensor to D4. Uses D7 LED onboard Particle to signal
  if the beam is broken or not. Blue LED means the beam is interrupted. Off means unbroken,
  or signal missing.
*/

#include "Particle.h"
#include "dct.h"

// LEDPIN is D7 on particle boards
#define LEDPIN D7
#define SENSORPIN D4

SYSTEM_MODE(MANUAL);

// variables will change:
int sensorState = 0, lastState=0;         // variable for reading the pushbutton status

void setup() {
  // initialize the LED pin as an output:
  pinMode(LEDPIN, OUTPUT);      
  // initialize the sensor pin as an input:
  pinMode(SENSORPIN, INPUT);     
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup
  digitalWrite(LEDPIN, LOW); // turn off the LED
  
  Serial.begin(9600);
}

void loop(){
  // read the state of the pushbutton value:
  sensorState = digitalRead(SENSORPIN);

  // check if the sensor beam is broken
  // if it is, the sensorState is LOW:
  if (sensorState == LOW) {     
    // turn LED on:
    digitalWrite(LEDPIN, HIGH);  
  } 
  else {
    // turn LED off:
    digitalWrite(LEDPIN, LOW); 
  }
  
  if (sensorState && !lastState) {
    Serial.println("Unbroken");
  } 
  if (!sensorState && lastState) {
    Serial.println("Broken");
  }
  lastState = sensorState;
}
