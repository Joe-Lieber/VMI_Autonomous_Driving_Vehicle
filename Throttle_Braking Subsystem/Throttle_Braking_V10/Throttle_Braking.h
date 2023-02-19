

#ifndef Library_H
#define Library_H

//declaring the function

void initialize();
void initializeEthernet();
void initializeInterrupt();
void modeSelect();

void modeSelect();
void readEthernet();
void readSensor();

bool checkEMG();
void EMGaction();


void decideAction();
void updateOutput();
void feedback();
void debug();

#endif
