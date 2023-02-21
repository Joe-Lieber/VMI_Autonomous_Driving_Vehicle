#include "Throttle_Braking.h"
#include <avr/wdt.h>

void setup(){
  initialize();
  initializeEthernet();
  initializeInterrupt();
  modeSelect();
}



void loop() {

  wdt_reset();
  
  readEthernet();

  modeSelect();
  
  readSensor();
  
  // Safety check
  if (checkEMG()) {
    EMGaction();
  }
  else{
    decideAction();
  }

  updateOutput();
  
  feedback();
  
  //debug();

}
