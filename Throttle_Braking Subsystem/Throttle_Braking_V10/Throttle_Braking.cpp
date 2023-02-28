#include "Throttle_Braking.h"
#include <Ethernet.h>
#include "w5100mod.h"
#include <avr/wdt.h>


///////// Define all of the pins for the I/O of the system /////////////////////


const int BRAKE_SWITCH_INPUT = 3;     // Manual Brake Switch
const int BRAKE_SENSOR_INPUT = A0;    // Manual Brake Sensor
const int THROTTLE_SWITCH_INPUT = 8;  // Manual Throttle Switch
const int THROTTLE_SENSOR_INPUT = A1; // Manual Throttle Sensor
const int BRAKE_SWITCH_OUTPUT = 6;    // Automated Brake Switch
const int BRAKE_PWM_OUTPUT = 5;       // Automated Brake Sensor
const int THROTTLE_SWITCH_OUTPUT = 4; // Automated Throttle Switch
const int THROTTLE_PWM_OUTPUT = 9;    // Automated Throttl Sensor
const int RELAY_OUTPUT = 7;           // Digital output to control the mode of the system
const int ETHERNET_INT = 2;           // Ethernet shield interrupt pin on INT0


///////// Define a variety of constants that will be used in the logic of the code /////////////////////////////


const int V0_5 = 25;                         // The PWM signal required for 0.5 Volts
const int V4_5 = 225;                        // The PWM signal required for 4.5 Volts
const int THROTTLE_ACCELERATION_PERIOD = 10; // Throttle Acceleration Period, time in [mS] before the throttle can increment up
const int THROTTLE_DECELERATION_PERIOD = 5;  // Throttle Decceleration Period, time in [mS] before the throttle can increment down
const int BRAKE_ACCELERATION_PERIOD = 15;    // Brake Acceleration period, time in [mS] before the brake can increment up
const int FEEDBACK_PERIOD = 1000;            // Amount of time in [mS] between feedback messages


///////// Define shifts used to shift the start bit of each variable to position 0 and masks for each size variable

//Message strcture: [Brake Switch(4 bits), Brake pwm(8 bits), Throttle Switch(4 bits), Throttle pwm(8 bits), EMPTY(2 bit), Emergency Reset(1 bit), Mode Select(1 bits)] <<read right to left/////
const int MODE_SELECT_SHIFT = 0;      // Shift vector for mode_select
const int EMG_RESET_SHIFT  = 1;       // Shift vector for EMG_reset
const int THROTTLE_PWM_SHIFT = 4;     // Shift vector for throttle_pwm
const int THROTTLE_SWITCH_SHIFT = 12; // Shift vector for throttle_switch
const int BRAKE_PWM_SHIFT = 16;       // Shift vector for brake_pwm
const int BRAKE_SWITCH_SHIFT = 24;    // Shift vector for brake_switch

const int MASK_1 = 0x1;               // Mask for 1 bit variables
const int MASK_8 = 0xFF;              // Mask for 8 bit variables


///////// Declare major Logic variables ////////////////////////////////////

// Variables assigned by ethernet
bool mode_select = false;                   // Logic bit that controls the current mode of the system (Manual / Automated)
bool EMG_reset = false;                     // Flag to reset Emergency Brake state
bool target_throttle_switch = false;        // Desired state of the Throttle Switch (initialized OFF)
bool target_brake_switch = false;           // Desired state of the  Brake Switch (initialized ON)
unsigned long target_throttle_pwm = V0_5;   // Desired state of the Throttle "sensor" output (Initialized to V0_5 / 0.5V / OFF)
unsigned long target_brake_pwm = V4_5;      // Desired state of the Brake "Sensor" output (Initialized to V4_5 / 4.5V / ON)

// Current Automated State
bool current_throttle_switch = false;       // Current automated state of the Throttle Switch (Initialized to OFF)
bool current_brake_switch = false;          // Current automated state of the Brake Switch (Initialized to ON)
unsigned long current_throttle_pwm = V0_5;  // Current automated state of the Throttle "Sensor" (Initialized to V0_5 / 0.5V / OFF)
unsigned long current_brake_pwm = V4_5;     // Current automated state of the Brake "Sensor" (Initialized to V4_5 / 4.5 / ON)

// Manual Sensors read from system
bool throttle_switch = false;               // Manual Throttle Switch signal value
bool brake_switch = false;                  // Manual Brake Switch signal value
unsigned long throttle_sensor = 0;          // Manual Throttle Sensor signal value
unsigned long brake_sensor = 0;             // Manual Brake Sensor signal value

// Emergency Flag
volatile bool EMG = false;                  // State variable for Emergency Brake state

// Acceleration Variables
unsigned long previous_acc = 0;             // Variable to keep track of the last time the ouput was incremented
unsigned long previous_feedback = 0;        // Variable to keep track of the last time feedback was sent


///////// Declare everything needed for Ethernet /////////////////////////////////////////////////////////////////////////////////////////////////////////////


IPAddress ip(192, 168, 1, 4);                             // Define the Static IP address of this system
const byte server_ip[] = {192, 168, 1, 2};                // Define the IP address of the server this system will communicate with
const byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};  // Define the MAC address of the system
const int localPort = 8887;                               // Port to use for communication, Each system has a unique port to use
volatile bool ethernet_flag = false;                      // Flag set by Ethernet interrupt to show a message is ready to be read
EthernetUDP Udp;                                          // Creates an instance of the UDP Class and opens a socket for the device to communicate on


///////// Function to clear all ethernet interrupt registers so the next one can trigger ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void clearSIRs() {              // After a socket IR, SnIR and SIR need to be reset
  for (int i = 0; i < 8; i++) {
    W5100.writeSnIR(i, 0xFF);   //Clear socket i interrupt
  }
  W5100.writeSIR(0xFF);         //Clear SIR
}


///////// Functions to disable and enable ethernet interrupts when needed ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


inline void disableSIRs() {
  W5100.writeSIMR(0x00);    // Disable interrupts for all sockets
}
inline void enableSIRs() {
  W5100.writeSIMR(0xFF);    // enable interrupts for all sockets
}


///////// Function to set the flag that an ethernet packet was recieved///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ethernetFlag() {
  ethernet_flag = true;
}


///////// Function that regulates the rate at which a value will increment to meet our performance criteria //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int acc(int reference, unsigned long period, int increment) {   // Takes in reference variable, the period it must wait to be incrimented in milli seconds, and how much to increment it by
  int current_time = millis();                                  // Set the current time in milliseconds using the arduino function millis();

  if ((current_time - previous_acc) > period) {                 // If the period it has been waiting is greater than the period specified,
    reference = reference + increment;                          // Increment the reference variable
    previous_acc = current_time;                                // Update previous time with the last time the variable was incremented
  }
  return reference;                                             // Return the reference variable
}


///////// Interrupt function to set the emergency state to TRUE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void setEMG() {

  if (digitalRead(!BRAKE_SWITCH_INPUT)) { // If the Brake switch is LOW (ON / Depressed)
    EMG = true;                           // Set the Emergency flag
  }
}


///////// Function to initialize all of the generic I/O and Serial interfaces/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void initialize() {
  Serial.begin(9600);
  Serial.println("Initializing....");
  wdt_enable(WDTO_1S);                    // Eneable the watchdog timer to execute an interrupt if it reaches 1 second
  pinMode(BRAKE_SWITCH_INPUT, INPUT);     //input from Brake Sensor
  pinMode(THROTTLE_SWITCH_INPUT, INPUT);  //input from Throttle Sensor
  pinMode(BRAKE_SENSOR_INPUT, INPUT);     //input from Brake Switch
  pinMode(THROTTLE_SENSOR_INPUT, INPUT);  //input from Throttle Sensor
  pinMode(BRAKE_PWM_OUTPUT, OUTPUT);      //output Brake pwmvalue to blackbox
  pinMode(THROTTLE_PWM_OUTPUT, OUTPUT);   //output Throttle pwmvalue to blackbox
  pinMode(BRAKE_SWITCH_OUTPUT, OUTPUT);   //output Brake Switch value to blackbox
  pinMode(THROTTLE_SWITCH_OUTPUT, OUTPUT);//output Throtttle Switch value to blackbox
  pinMode(RELAY_OUTPUT, OUTPUT);          //otput mode(Automonus/Manual)
}


///////// Function to initialize Ethernet///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void initializeEthernet() {

  Ethernet.init(10);                                              // You can use Ethernet.init(pin) to configure the CS pin. Most Arduino shields use pin 10

  Serial.println("Ethernet Initializing....");                    // Print that the Ethernet connection is starting
  Ethernet.begin(mac, ip);                                        // Begin ethernet connection

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {          // Check for Ethernet hardware being present
    Serial.println("Ethernet shield was not found.");             // If it is missing, print to serial monitor
    while (Ethernet.hardwareStatus() == EthernetNoHardware) {     // While the Hardware is missing
      delay(1000);                                                // wait for 1 second
      Serial.print(".");                                          // and print a dot to the serial monitor
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {                         // Check to see if there is a physical connection to the network
    Serial.println("Ethernet cable is not connected.");           // if there is not then print it to the serial monitor
    while (Ethernet.linkStatus() == LinkOFF) {                    // while there is no connection
      delay(1000);                                                // wait for 1 second
      Serial.print(".");                                          // and print a dot to the serial monitor
    }
  }
  Udp.begin(localPort);                                           // start the UDP connection on the port declared

  Serial.println("Ethernet Initialized");                         // print to the serial monitor that the ethernet is done initializing
}


///////// Function to initialize all of the interrupts needed///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void initializeInterrupt() {
  Serial.println("Initializing Interrupts....");                                // Print to the serial monitor that the interrupts are being initialized

  pinMode(ETHERNET_INT, INPUT);                                                 // set the interrupt for the ethernet shield as an input
  attachInterrupt(digitalPinToInterrupt(ETHERNET_INT), ethernetFlag, FALLING);  // set the ethernet shield pin as an interrupt triggered on the falling edge and assign the function ethernetFlag as the ISR

  pinMode(BRAKE_SWITCH_INPUT, INPUT);                                           // Set up interrupt for the Brake switch
  attachInterrupt(digitalPinToInterrupt(BRAKE_SWITCH_INPUT), setEMG, FALLING);  // set the Brake switch pin as an interrupt triggered on the falling edge and assign the function EMGStop as the ISR

  for (int i = 0; i < 8; i++) {                                                 // Configure Wiznet W5500 interrupts
    W5100.writeSnIMR(i, 0x04);                                                  // socket ISR mask; RECV for all sockets
  }
  enableSIRs();                                                                 // Enable ethernet interrupts

  Serial.println("Interrupts Initialized");                                     // Print to the serial monitor that the interrupts are initialized
}


///////// Read the ethernet message recieved and update values ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void readEthernet() {
  if (ethernet_flag) {                                      // If the Ethernet Flag has been set by an interrupt,
    disableSIRs();                                          // Disable interrupts so that data is not corrupted while reading and writing the variables below

    char incoming[UDP_TX_PACKET_MAX_SIZE] = "";             // Define a character array to hold the array sent in the UDP packet
    unsigned long recieved;                                 // Define a local variable to hold the string of bits sent
    int packetSize = Udp.parsePacket();                     // Define a local variable equal to the size of the UDP packet

    if (packetSize) {                                       // If the packet size is larger than zero,
      if (memcmp(Udp.remoteIP(), server_ip, 4)) {           // If the IP address from the UDP message match's that of the server,
        Udp.read(incoming, UDP_TX_PACKET_MAX_SIZE);         // Read the UDP message and assaign it to incoming
        recieved = strtoul(incoming, NULL, 16);             // Convert the character array of hex bits into an unsigned long
      }
      else {                                                // If the IP address from the UDP message does not match that of the server,
        Serial.print("ERROR... Sender unknown.");           // Print to the serial monitor that there is an error because the sender is unknown
      }
    }
    else {                                                  // If the packet size is equal to zero,
      Serial.print("ERROR... False Ethernet Interrupt");    // Print to the serial monitor that is an error because the ethernet flag was falsely set
    }

    mode_select = (recieved >> MODE_SELECT_SHIFT) & MASK_1;                 // Parse recieved[0] for mode_select
    EMG_reset = (recieved >> EMG_RESET_SHIFT) & MASK_1;                     // Parse recieved[1] for EMG_reset
    target_throttle_pwm = (recieved >> THROTTLE_PWM_SHIFT) & MASK_8;        // Parse recieved[4:11] for target_throttle_pwm
    target_throttle_switch = (recieved >> THROTTLE_SWITCH_SHIFT) & MASK_1;  // Parse recieved[12:15] for target_throttle_switch
    target_brake_pwm = (recieved >> BRAKE_PWM_SHIFT) & MASK_8;              // Parse recieved[16:23] for target_brake_pwm
    target_brake_switch = (recieved >> BRAKE_SWITCH_SHIFT) & MASK_1;        // Parse recieved[24:27] for target_brake_switch
    
    if (target_brake_pwm < V0_5) {              // Check Low side target_brake_pwm
      target_brake_pwm = V0_5;
    }
    if (current_brake_pwm < V0_5) {             // Check Low side current_brake_pwm
      current_brake_pwm = V0_5;
    }
    /////////////////////////////
    if (target_throttle_pwm < V0_5) {           // Check Low side target_throttle_pwm
      target_throttle_pwm = V0_5;
    }
    if (current_throttle_pwm < V0_5) {          // Check Low side current_throttle_pwm
      current_throttle_pwm = V0_5;
    }
    ////////////////////////////
    if (target_brake_pwm > V4_5) {              // Check High side target_brake_pwm
      target_brake_pwm = V4_5;
    }
    if (current_brake_pwm > V4_5) {             // Check High side current_brake_pwm
      current_brake_pwm = V4_5;
    }
    //////////////////////////////
    if (target_throttle_pwm > V4_5) {           // Check High side target_throttle_pwm
      target_throttle_pwm = V4_5;
    }
    if (current_throttle_pwm > V4_5) {
      current_throttle_pwm = V4_5;              // Check High side current_throttle_pwm
    }

    ethernet_flag = 0;                          // Reset the ethernet flag so that the next interrupt can set it
    clearSIRs();                                // clear all interrupt flags from the Wiznet 5500
    enableSIRs();                               // re-enable interrupts now that we are done
    feedback();                                 // Send a feedback message to reaffirm the right values were recieved
  }
}


///////// Function for reading values from Sensors, Switch and Mode ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void readSensor() {
  brake_sensor = analogRead(BRAKE_SENSOR_INPUT);        // read the input pin of Brake Sensor
  throttle_sensor = analogRead(THROTTLE_SENSOR_INPUT);  // read the input pin of Brake Sensor
  brake_switch = digitalRead(BRAKE_SWITCH_INPUT);       // read the input pin of Brake Switch
  throttle_switch = digitalRead(THROTTLE_SWITCH_INPUT); // read the input pin of Brake Switch
}


///////// Function to switch mode between autonomous/manual //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void modeSelect() {

  if (mode_select) {                                                             // If mode select is 1 (Automated mode)
    digitalWrite(RELAY_OUTPUT, HIGH);                                            // Turn on the relays (automated mode)
    attachInterrupt(digitalPinToInterrupt(BRAKE_SWITCH_INPUT), setEMG, FALLING); // Attach the interrupt to the brake switch to see if the user is trying to override the system
  }
  else {                                                                         // if mode select is 0 (Manual mode)
    digitalWrite(RELAY_OUTPUT, LOW);                                             // Turn the relays off (Manual mode
    detachInterrupt(digitalPinToInterrupt(BRAKE_SWITCH_INPUT));                  // Detach the Brake switch interrupt so that it does not trigger while the user operates the vehicle
  }
}


///////// Function to return the state of emergency brake ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool checkEMG() {
  return EMG;
}


///////// Function for to execute emergency brake//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void EMGaction() { // Set both current and target values to full braking
  current_throttle_pwm = V0_5;
  current_brake_pwm = V4_5;
  current_throttle_switch = false;
  current_brake_switch = false;
  target_throttle_pwm = V0_5;
  target_brake_pwm = V4_5;
  target_throttle_switch = false;
  target_brake_switch = false;
}


///////// Function that contains the algorith to adjust the automated outputs based on the current and target values to meet our desired performance criteria/////////////////////////////////////////////////////////////////

void decideAction() {

  if (current_throttle_switch) {
    if (current_throttle_pwm > target_throttle_pwm) {
      current_throttle_pwm = acc(current_throttle_pwm, THROTTLE_DECELERATION_PERIOD, -1);
    } else if (current_throttle_pwm < target_throttle_pwm) {
      current_throttle_pwm = acc(current_throttle_pwm, THROTTLE_ACCELERATION_PERIOD, 1);
    }
    else if (current_throttle_switch != target_throttle_switch) {
      current_throttle_switch = !current_throttle_switch;
    }
  }
  ////////////////////////////////////////////////////////////////////////
  else if (!current_brake_switch) {
    if (current_brake_pwm > target_brake_pwm) {
      current_brake_pwm = target_brake_pwm;
    }
    else if (current_throttle_pwm < target_brake_pwm) {
      current_brake_pwm = acc(current_brake_pwm, BRAKE_ACCELERATION_PERIOD, 1);
    }
    else if (current_brake_switch != target_brake_switch) {
      current_brake_switch = !current_brake_switch;
   }
  }
  ////////////////////////////////////////////////////////////////////////
  else {
    if (current_brake_switch != target_brake_switch) {
      current_brake_switch = !current_brake_switch;
    }
    else if (current_throttle_switch != target_throttle_switch) {
      current_throttle_switch = !current_throttle_switch;
    }
  }
}


///////// Function that writes all of the automated outputs for our system /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void updateOutput() {
  analogWrite(BRAKE_PWM_OUTPUT, current_brake_pwm);
  analogWrite(THROTTLE_PWM_OUTPUT, current_throttle_pwm);
  digitalWrite(BRAKE_SWITCH_OUTPUT, current_brake_switch);
  digitalWrite(THROTTLE_SWITCH_OUTPUT, current_throttle_switch);
}


///////// Function to send data back to the server to be checked for errors /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void feedback() {
  int current_time = millis();                                // Set the current time in milliseconds using the arduino function millis();

  if ((current_time - previous_feedback) > FEEDBACK_PERIOD) { // If the period it has been waiting is greater than the period specified,
    
    unsigned long feedback_1;           // Holds part of the feedback message
    unsigned long feedback_2;           // Holds part of the feedback message

    //////////////////////////////////////////////////
    feedback_2 = (feedback_2 << 1) | brake_switch;              // feedback_2 [30]
    feedback_2 = (feedback_2 << 10) | brake_sensor;             // feedback_2 [20:29]
    feedback_2 = (feedback_2 << 1) | throttle_switch;           // feedback_2 [19]
    feedback_2 = (feedback_2 << 10) | throttle_sensor;          // feedback_2 [9:18]
    //////////////////////////////////////////////////
    feedback_2 = (feedback_2 << 1) | current_brake_switch;      // feedback_2 [8]
    feedback_2 = (feedback_2 << 8) | current_brake_pwm;         // feedback_2 [0:7]

    //////////////////////////////////////////////////////////////////////////////////////

    feedback_1 = (feedback_1 << 1) | current_throttle_switch;   // feedback_1 [28]
    feedback_1 = (feedback_1 << 8) | current_throttle_pwm;      // feedback_1 [20:27]
    //////////////////////////////////////////////////
    feedback_1 = (feedback_1 << 1) | target_brake_switch;       // feedback_1 [19]
    feedback_1 = (feedback_1 << 8) | target_brake_pwm;          // feedback_1 [11:18]
    feedback_1 = (feedback_1 << 1) | target_throttle_switch;    // feedback_1 [10]
    feedback_1 = (feedback_1 << 8) | target_throttle_pwm;       // feedback_1 [2:9]
    //////////////////////////////////////////////////
    feedback_1 = (feedback_1 << 1) | EMG;                       // feedback_1 [1]
    feedback_1 = feedback_1 | mode_select;                      // feedback_1 [0]
    //////////////////////////////////////////////////
   

    char first[8];                        // Defined to hold the first half of the resulting char array
    char second[8];                       // Defined to hold the second half of the resulting char array
    char message[UDP_TX_PACKET_MAX_SIZE]; // Defined to hold the final feedback message in a char array
    
    ultoa(feedback_2, first, HEX);        // Convert y into Hexadecimal characters and store them in a char array
    ultoa(feedback_1, second, HEX);       // Convert x into Hexadecimal characters and store them in a char array
    strcpy(message, first);               // copy the first half of the message into the final message char array
    strcat(message, second);              // copy the second half of the message into the final message char array

    Udp.beginPacket(server_ip, localPort);
    Udp.write(message);
    Udp.endPacket();
    
    previous_feedback = current_time;      // Update previous time with the last time the variable was incremented
  }
}


///////// Function used to print important values to the serial monitor for the purpose of debugging /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void debug() {/*
  Serial.print("Mode Select: ");
  Serial.print(mode_select);
  Serial.print("  ");
  Serial.print("Emergency Reset: ");
  Serial.print(EMG_reset);
  Serial.print("  ");
  Serial.print("Target Throttle pwm / Switch: ");
  Serial.print(target_throttle_pwm);
  Serial.print("  ");
  Serial.print(target_throttle_switch);
  Serial.print("  ");*/
  /*Serial.print("Target Brake pwm / Switch: ");
  Serial.print(target_brake_pwm);
  Serial.print("  ");
  Serial.print(target_brake_switch);
  Serial.print("  ");
  //Serial.print("Current Throttle pwm / Switch: ");
  //Serial.print(current_throttle_pwm);
  //Serial.print("  ");
  //Serial.print(current_throttle_switch);
  //Serial.print("  ");
  Serial.print("Current Brake pwm / Switch: ");
  Serial.print(current_brake_pwm);
  Serial.print("  ");
  Serial.print(current_brake_switch);
  Serial.println("");*/
  /*char debugthbk[200];
        sprintf(debugthbk,"%s, %s, %s, %s, %d, %d, %s, %s, %d, %d, %s, %s, %d, %d, %s, %d, %d",
                mode_select, EMG_reset, target_throttle_switch, target_brake_switch, target_throttle_pwm, target_brake_pwm,
                current_throttle_switch, current_brake_switch, current_throttle_pwm, current_brake_pwm,
                throttle_switch, brake_switch, throttle_sensor, brake_sensor,
                EMG,
                previous_acc, previous_feedback
                );
        Serial.print(debugthbk);*/


  Serial.println(
                String(mode_select) + ", " + String(EMG_reset) + ", " + String(target_throttle_switch) + ", " + String(target_brake_switch) +", " + String(target_throttle_pwm) + ", " + String(target_brake_pwm) +
                 ", " + String(current_throttle_switch) + ", " + String(current_brake_switch) + ", " + String(current_throttle_pwm) + ", " + String(current_brake_pwm) + ", " + 
                String(throttle_switch) + ", " + String(brake_switch) + ", " + String(throttle_sensor) + ", " + String(brake_sensor) + ", " +
                String(EMG) + ", " +
                String(previous_acc) + ", " + String(previous_feedback)
                );

}
