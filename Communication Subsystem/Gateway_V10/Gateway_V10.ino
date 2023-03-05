#include "Gateway.h"
#include <avr/wdt.h>


void setup() {
  initialize();
  initializeEthernet();
  initializeInterrupt();
}


void loop() {
  wdt_reset();
  serialHandle();
 // debug();
  readEthernet();
}
