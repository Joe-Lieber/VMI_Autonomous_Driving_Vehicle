#ifndef GATEWAY_H
#define GATEWAY_H

void initialize();
void initializeEthernet();
void initializeInterrupt();

void readEthernet();
void serialHandle();

#endif
