/* 
Project: litterbox-ventilation
Author: Gabe Conradi
Description: See README.md in this repo for details
TODO:
* attachInterrupt(digitalPinToInterrupt(pin), ISR, mode) for breakbeam wakeup
*/

#include "Particle.h"
#include "dct.h"

SYSTEM_MODE(MANUAL);

// internal blue LED on Particle, indicates breakbeam status
#define LED_STATUS_PIN D7
#define BREAKBEAM_PIN D4
#define FAN_PWM_PIN D5

// how long to run the fan after the litterbox is not occupied for cleanup
#define COOLDOWN_MS 20000
// failsafe for if we get into a strange state where the house is occupied forever. prevents fan from running
// constantly
#define FAN_ACTIVE_MAX_MS 300000

enum BreakbeamState {
  IR_STATE_BROKEN = LOW,
  IR_STATE_UNBROKEN = HIGH
};

enum LitterboxState {
  HOUSE_STATE_VACANT = 0,
  HOUSE_STATE_OCCUPIED,
  HOUSE_STATE_COOLDOWN
};

enum FanState {
  FAN_STATE_OFF = 0,
  FAN_STATE_ON,
  FAN_STATE_COOLDOWN
};

BreakbeamState breakbeam_state = IR_STATE_UNBROKEN;
BreakbeamState breakbeam_last_state = breakbeam_state;
unsigned long breakbeam_last_change_time = 0;

// whether the fan is on or off
FanState fan_state = FAN_STATE_OFF;
// when the fan was last turned on. Keeping track of runtime here
unsigned long fan_enable_time_ms;

unsigned long state_time_ms;
LitterboxState state = HOUSE_STATE_VACANT;
LitterboxState previous_state = state;

void fan_state_change(FanState s) {
  if (fan_state == FAN_STATE_OFF && s != FAN_STATE_OFF) {
    fan_enable_time_ms = millis();
  }
  fan_state = s;
}

void state_change(LitterboxState s) {
  previous_state = state;
  if (state != s) {
    state_time_ms = millis();
  }
  state = s;
}

void print_state(String prefix) {
  String house;
  String oldhouse;
  String ir;
  String fan;
  if (previous_state == HOUSE_STATE_VACANT){
    oldhouse = "house_vacant";
  } else if (previous_state == HOUSE_STATE_OCCUPIED){
    oldhouse = "house_occupied";
  } else {
    oldhouse = "house_cooldown";
  }
  if (state == HOUSE_STATE_VACANT){
    house = "house_vacant";
  } else if (state == HOUSE_STATE_OCCUPIED){
    house = "house_occupied";
  } else {
    house = "house_cooldown";
  }
  if (fan_state == FAN_STATE_ON){
    fan = "fan_on";
  } else if (fan_state == FAN_STATE_OFF){
    fan = "fan_off";
  } else {
    fan = "fan_cooldown";
  }

  if (breakbeam_state == IR_STATE_BROKEN){
    ir = "ir_broken";
  } else {
    ir = "ir_connected";
  }

  String t = "";
  if (state != HOUSE_STATE_VACANT) {
    t = String(millis()-state_time_ms);
  }

  Serial.println(prefix + ": " + house + "(" + oldhouse + ") | " + ir + " | " + fan + "  ---  " + t);
}


void setup() {
  reset_all_state();
  configure_fan_pwm();

  // control the status LED builtin to the particle
  RGB.control(true);

  // initialize the LED pins as an output:
  pinMode(LED_STATUS_PIN, OUTPUT);      
  // initialize the sensor pin as an input:
  pinMode(BREAKBEAM_PIN, INPUT);     
  digitalWrite(BREAKBEAM_PIN, HIGH); // turn on the pullup
  digitalWrite(LED_STATUS_PIN, LOW); // turn off LED to start

  Serial.begin(9600);
}


void read_breakbeam_state(){
  breakbeam_last_state = breakbeam_state;
  unsigned long tnow = millis();
  // read the state of the pushbutton value:
  int curr_value = digitalRead(BREAKBEAM_PIN);
  if (curr_value == LOW) {
    breakbeam_state = IR_STATE_BROKEN;
  } else {
    breakbeam_state = IR_STATE_UNBROKEN;
  }

  if (breakbeam_state != breakbeam_last_state) {
    // if there was a change in state, record when this happened. We will process the time
    // the state change occurred later to determine timer countdown
    breakbeam_last_change_time = tnow;
  } 
}

void reset_all_state(){
  state_time_ms = millis();
  breakbeam_last_change_time = state_time_ms;
  breakbeam_state = IR_STATE_UNBROKEN;
  breakbeam_last_state = breakbeam_state;
  fan_state_change(FAN_STATE_OFF);
  fan_enable_time_ms = state_time_ms;
  state_change(HOUSE_STATE_VACANT);
  previous_state = state;
  Serial.println(String(state_time_ms) + " !!! RESET !!!");
  return;
}

void configure_fan_pwm(){
}

void set_fan_mode(){
  // https://arduino.stackexchange.com/questions/25609/set-pwm-frequency-to-25-khz/25623#25623
}

void set_status_leds() {
  if (state == HOUSE_STATE_COOLDOWN) {
    RGB.color(0,0,255); // blue cooldown
  } else if (state == HOUSE_STATE_OCCUPIED) {
    RGB.color(0,255,0); // green occupied
  } else {
    RGB.color(0,0,0); // black vacant
  }

  // check if the sensor beam is broken
  // if it is, the breakbeam_state is LOW:
  if (breakbeam_state == IR_STATE_BROKEN) {
    digitalWrite(LED_STATUS_PIN, HIGH);
  } else {
    digitalWrite(LED_STATUS_PIN, LOW);
  }
}

// put it all together
void loop(){
  unsigned long tnow = millis();

  // TODO: move this to interrupt
  read_breakbeam_state();

  if (breakbeam_state == IR_STATE_BROKEN) {
    state_change(HOUSE_STATE_OCCUPIED);
  } else if (breakbeam_state == IR_STATE_UNBROKEN && state == HOUSE_STATE_OCCUPIED) {
    state_change(HOUSE_STATE_COOLDOWN);
  }

  switch (state) {
    case HOUSE_STATE_VACANT:
      fan_state_change(FAN_STATE_OFF);
      break;
    case HOUSE_STATE_OCCUPIED:
      fan_state_change(FAN_STATE_ON);
      break;
    case HOUSE_STATE_COOLDOWN:
      fan_state_change(FAN_STATE_COOLDOWN);

      if (tnow > state_time_ms + COOLDOWN_MS) {
        Serial.println("Unoccupied - cooldown completed");
        state_change(HOUSE_STATE_VACANT);
      }
      break;
  }

  if (fan_state != FAN_STATE_OFF && tnow > fan_enable_time_ms + FAN_ACTIVE_MAX_MS) {
    // turn off fan - runtime exceeded
    // NOTE: i do not think this works witththe whole state machine properly, but it is a failsafe here
    // to prevent high power draw in the event i fucked something up and am stuck in a state where we think the house is
    // occupied for a long time. Battery conservation strategy to guard against my shit logic :)
    Serial.println("Fan max runtime exceeded - resetting");
    fan_state_change(FAN_STATE_OFF);
  }

  // set fan PWM control as necessary based on fan_enable state
  set_fan_mode();

  // set LED status based on occupied state
  set_status_leds();

  //print_state("<--");

  delay(100);
}
