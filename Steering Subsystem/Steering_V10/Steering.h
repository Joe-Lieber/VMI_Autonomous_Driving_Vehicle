#ifndef STEERING_H
#define STEERING_H

void initialize();
void initializeEthernet();
void initializeInterrupt();

void readEthernet();
void readSensors();

bool state();
void EMGaction();

void decideAction();
void updateOutput();
void feedback();
void debug();

#endif
