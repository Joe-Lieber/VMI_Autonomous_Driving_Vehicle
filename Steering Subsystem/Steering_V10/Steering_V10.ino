#include "steering.h"
#include <avr/wdt.h>

void setup() {
  // Set input and output pins
  initialize();
  initializeEthernet();
  initializeInterrupt();
  Serial.print("millis,mode_select,EMG,EMG_reset,target_heading,current_heading,mid_point,offset,current_speed,current_direction,torque_a,torque_b,wiggle_room,revolutions");
  Serial.println("");
}

void loop() {

  wdt_reset();

  readEthernet();

  readSensors();// Read sensors

  // Safety check
  if (state()) {
    EMGaction();
  }
  else {
    decideAction();//Decide what the next set of ouputs should be
  }
  
  updateOutput();//send the decided values to the outputs

  feedback();//Feedback message for the server

  //debug();// Serial print important values
}
