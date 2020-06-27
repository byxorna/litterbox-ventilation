/* 
Project: litterbox-ventilation
Author: Gabe Conradi
Description: See README.md in this repo for details
TODO:
* attachInterrupt(digitalPinToInterrupt(pin), ISR, mode) for breakbeam wakeup
*/

#define RGB_LED_STATUS_PIN 13
#define BREAKBEAM_PIN 4
#define FAN_PWM_PIN 5

// how long to run the fan after the litterbox is not occupied for cleanup
#define FAN_ACTIVE_COOLDOWN_MS 60000
// failsafe for if we get into a strange state where the house is occupied forever. prevents fan from running
// constantly
#define FAN_ACTIVE_MAX_MS 300000

#define STATUS_VACANT 0
#define STATUS_OCCUPIED 1
#define STATUS_COOLDOWN 2

int breakbeam_current_state = 0, breakbeam_last_state=0;
unsigned long breakbeam_last_change_time = 0;

// whether the fan is on or off
int fan_enable = 0;
// when the fan was last turned on. Keeping track of runtime here
unsigned long fan_enable_time_ms = 0;

int occupied = STATUS_VACANT;
int occupied_previous_state = occupied;

void setup() {
  reset_all_state();
  configure_fan_pwm();

  // initialize the LED pin as an output:
  pinMode(RGB_LED_STATUS_PIN, OUTPUT);      
  // initialize the sensor pin as an input:
  pinMode(BREAKBEAM_PIN, INPUT);     
  digitalWrite(BREAKBEAM_PIN, HIGH); // turn on the pullup
  
  Serial.begin(9600);
}


void read_breakbeam_state(){
  unsigned long tnow = millis();
  // read the state of the pushbutton value:
  breakbeam_current_state = digitalRead(BREAKBEAM_PIN);

  // check if the sensor beam is broken
  // if it is, the breakbeam_current_state is LOW:
  if (breakbeam_current_state == LOW) {     
    // turn LED on:
    digitalWrite(RGB_LED_STATUS_PIN, HIGH);  
  } 
  else {
    // turn LED off:
    digitalWrite(RGB_LED_STATUS_PIN, LOW); 
  }
  
  if (breakbeam_current_state != breakbeam_last_state) {
    // if there was a change in state, record when this happened. We will process the time
    // the state change occurred later to determine timer countdown
    breakbeam_last_change_time = tnow;
  } 

  if (breakbeam_current_state && !breakbeam_last_state) {
    Serial.println("Occupied");
  } 
  if (!breakbeam_current_state && breakbeam_last_state) {
    Serial.println("Vacant");
  }
  breakbeam_last_state = breakbeam_current_state;
}

void reset_all_state(){
  breakbeam_last_change_time = 0;
  breakbeam_last_state = 0;
  fan_enable = 0;
  fan_enable_time_ms = 0;
  occupied = STATUS_VACANT;
  occupied_previous_state = occupied;
  return;
}

void configure_fan_pwm(){
}

void set_fan_mode(){
  // https://arduino.stackexchange.com/questions/25609/set-pwm-frequency-to-25-khz/25623#25623
}

// put it all together
void loop(){
  unsigned long tnow = millis();
  // handle time overflow and reset
  if (tnow < breakbeam_last_change_time) {
    // reset all timers
    reset_all_state();
    // skip this iteration
    return;
  }

  // TODO: move this to interrupt
  read_breakbeam_state();

  // based on breakbeam state, and timing of fan_enabled_time_ms, set occupied state
  occupied_previous_state = occupied;
  if (breakbeam_current_state && occupied == STATUS_VACANT) {
    occupied = STATUS_OCCUPIED;
  } else if (!breakbeam_current_state && occupied == STATUS_OCCUPIED && tnow < breakbeam_last_change_time + FAN_ACTIVE_COOLDOWN_MS) {
    occupied = STATUS_COOLDOWN;
  } else {
    occupied = STATUS_VACANT;
  }

  switch (occupied) {
    case STATUS_VACANT:
      if (occupied_previous_state != STATUS_VACANT) {
        Serial.println("Vacant");
        fan_enable = 0;
      }
      break;
    case STATUS_OCCUPIED:
      if (occupied_previous_state != STATUS_OCCUPIED) {
        Serial.println("Occupied");
        fan_enable = 1;
        fan_enable_time_ms = tnow;
      }
      break;
    case STATUS_COOLDOWN:
      if (occupied_previous_state == STATUS_OCCUPIED) {
        Serial.println("Unoccupied - cooldown started");
      } else if (fan_enable && tnow > breakbeam_last_change_time + FAN_ACTIVE_COOLDOWN_MS) {
        Serial.println("Unoccupied - cooldown completed");
        fan_enable = 0;
      }
      break;
    default:
      Serial.println("Unknown status - resetting state");
      reset_all_state();
      break;
  }

  if (fan_enable && tnow > breakbeam_last_change_time + FAN_ACTIVE_MAX_MS) {
    // turn off fan - runtime exceeded
    // NOTE: i do not think this works witththe whole state machine properly, but it is a failsafe here
    // to prevent high power draw in the event i fucked something up and am stuck in a state where we think the house is
    // occupied for a long time. Battery conservation strategy to guard against my shit logic :)
    Serial.println("Fan max runtime exceeded - resetting");
    fan_enable = 0;
  }

  // TODO: turn on fan based on fan_enable
  set_fan_mode();

  // TODO: set LED status based on occupied state

  sleep TODO
}
