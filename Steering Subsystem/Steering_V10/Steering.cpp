#include "steering.h"
#include <Ethernet.h>
#include "w5100mod.h"
#include <avr/io.h>
#include <avr/wdt.h>

/////////Define all of the pins for the I/O of the system/////////////////////

//Torque sensor
const int TORQUE_A = A0;     // Input A for the built in torque sensor of Super ATV Power steering
const int TORQUE_B = A1;     // Input A for the built in torque sensor of Super ATV Power steering
const int MAX_TORQUE = 800;  // The threshold for the difference of the two torque value before EMG is thrown
const int POWER_STEERING_THRESHOLD = 200; // The threshold to overcome when using the power steering functionality
// Encoder Values
const int ENCODER_CS = 7;         // Chip Select for encoder
const int ENCODER_CLK = 5;        // Artificial clock signal for encoder
const int ENCODER_DATA = 6;       // Data pin of the encoder
const int encoder_offset = -512;  // While the actual encoder reads 512 at center to help with initialization, we manually shift the center to "0"
//PWM motor controller
const int DIRECTION = 8;          // Digital direction output for PWM Motor Controller
const int PWM = 9;                // PWM output for PWM Motor Controller
const int OVER_CURRENT = 3;       // Over Current LED on PWM motor controller we taped into
const int ADJUSTMENT_SPEED = 35;  // The minimum PWM value sent to the motor
const int max_speed = 240;        // The max PWM value that can be ouput to the PWM Motor controller
const bool direction_bit = 0;     // ******* If the wheel is turning opposite of desired set to the logic opposite
//Ethernet
const int ETHERNET_INT = 2;  // Ethernet shield interrupt pin on INT0
//Timers
const int TURN_PERIOD = 5;         // turn period, time in [mS] before the turn speed can increment up or down
const int FEEDBACK_PERIOD = 1000;  // Amount of time in [mS] between feedback messages


///////// Define shifts used to shift the start bit of each variable to position 0 and masks for each size variable

//Message strcture: [Target Heading(16 bits), Empty(2 bit), Emergency Reset(1 bit), Mode Select(1 bit)] <<read right to left/////
const int MODE_SELECT_SHIFT = 0;     // Shift vector for mode_select
const int EMG_RESET_SHIFT = 1;       // Shift vector for EMG_reset
const int EMG_SHIFT = 2;             // Shift vector for EMG
const int TARGET_HEADING_SHIFT = 3;  // Shift vector for target_heading

const int MASK_1 = 0x1;      // Mask for 1 bit variables
const int MASK_16 = 0xFFFF;  // Mask for 16 bit variables


////////////////// Declare major Logic variables////////////////////////////////////

// Variables assigned by ethernet
bool mode_select = false;  // Logic bit that controls the current mode of the system (Manual / Automated)
bool EMG_reset = false;    // Flag to tell the system to reset the emergency stop state
int target_heading = 0;    // Desired state of the heading of the steering wheel (initialized to 512 / Center)

// Current Automated State
int current_heading = 0;        // Current state of the heading of the steering wheel (initialized to 512 / Center)
int old_current_heading = 512;  // Used to track zero crosses with Encoder
int revolutions = 0;            // Keeps track of how many revolutions was made by the encoder
int current_speed = 0;          // Current value of the speed of the steering motor
bool current_direction = 0;     // State variable to determine what direction the steering motor

// Calculated Values
int mid_point = 0;  // A value calculated that is between the current and target value
int offset = 0;     // An offset from the midpoint the scales based on the difference between current and target heading

// Torque sensor Values
int expected_torque = 0;  // Threshold for the difference between torque A and B to cause a safety infraction
int torque_a = 512;       // Value read from torque sensor
int torque_b = 512;       // Value read from torque sensor

// Logic Variables
int wiggle_room = 1;        // How far from the target the current value can be without correcting
volatile bool EMG = false;  // State variable to indicate possible safety violation

// acceration Variables
unsigned long previous_acc = 0;       // Variable to keep track of the last time the ouput was incremented
unsigned long previous_feedback = 0;  // Variable to keep track of the last time feedback was sent


///////// Declare everything needed for Ethernet /////////////////////////////////////////////////////////////////////////////////////////


IPAddress ip(192, 168, 1, 3);                         // Define the Static IP address of this system
byte server_ip[] = { 192, 168, 1, 2 };                // Define the IP address of the server this system will communicate with
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Define the MAC address of the system
unsigned int localPort = 8888;                        // Port to use for communication, Each system has a unique port to use
volatile bool ethernet_flag = false;                  // Flag set by Ethernet interrupt to show a message is ready to be read
EthernetUDP Udp;                                      // Creates an instance of the UDP Class and opens a socket for the device to communicate on


///////// Function to clear all ethernet interrupt registers so the next one can trigger ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void clearSIRs() {  // After a socket IR, SnIR and SIR need to be reset
  for (int i = 0; i < 8; i++) {
    W5100.writeSnIR(i, 0xFF);  //Clear socket i interrupt
  }
  W5100.writeSIR(0xFF);  //Clear SIR
}


///////// Functions to disable and enable ethernet interrupts when needed ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


inline void disableSIRs() {
  W5100.writeSIMR(0x00);  // Disable interrupts for all sockets
}
inline void enableSIRs() {
  W5100.writeSIMR(0xFF);  // enable interrupts for all sockets
}


///////// Function to set the flag that an ethernet packet was recieved///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ethernetFlag() {
  ethernet_flag = true;
}


///////// Function that regulates the rate at which a value will increment to meet our performance criteria //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int acc(int reference, int increment) {  // Takes in reference variable, the period it must wait to be incrimented in milli seconds, and how much to increment it by

  unsigned long current_time = millis();  // Set the current time in milliseconds using the arduino function millis();

  if ((current_time - previous_acc > TURN_PERIOD) && (reference < max_speed)) {  // If the period it has been waiting is greater than the period specified,
    reference += increment;                                                      // Increment the reference variable
    previous_acc = current_time;                                                 // Update previous time with the last time the variable was incremented
  }
  if (reference < ADJUSTMENT_SPEED) reference = ADJUSTMENT_SPEED;  // If it is trying to increment below the minimum speed for the motor to turn, keep it at ADJUSJUSTMENT_SPEED

  return reference;  // Return the reference variable
}


///////// Interrupt function to set the emergency state to TRUE //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void setEMG() {
  if (!digitalRead(OVER_CURRENT)) {  // If the over current sensor is LOW (ON / pulled low)
    EMG = true;                      // Set the Emergency stop flag
  }
}


///////// Function to initialize all of the generic I/O and Serial interfaces/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initialize() {
  Serial.begin(9600);
  Serial.println("Initializing....");
  wdt_enable(WDTO_1S);         // Eneable the watchdog timer to execute an interrupt if it reaches 1 second
  pinMode(DIRECTION, OUTPUT);  // Direction output to PWM motor controller
  pinMode(PWM, OUTPUT);        // PWM output to PWM motor controller
  pinMode(TORQUE_A, INPUT);    // Torque sensor input
  pinMode(TORQUE_B, INPUT);    // Torque sensor input
  //SETUP BOURNS EMS22A
  pinMode(ENCODER_CS, OUTPUT);   // Define the Chip select pin as an output to tell the encoder when we want to activate it
  pinMode(ENCODER_CLK, OUTPUT);  // Define the Clock pin as an ouptut to provide a clock signal using a digital output
  pinMode(ENCODER_DATA, INPUT);  // Define the DATA pin as an input to read the stream of bits from the encoder

  TCCR1B = TCCR1B & B11111000 | B00000001;  // set timer 1 divisor to 1 for PWM frequency of 31372.55 Hz
}


///////// Function to initialize Ethernet///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void initializeEthernet() {

  Ethernet.init(10);  // You can use Ethernet.init(pin) to configure the CS pin. Most Arduino shields use pin 10

  Serial.println("Ethernet Initializing....");  // Print that the Ethernet connection is starting
  Ethernet.begin(mac, ip);                      // Begin ethernet connection

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {       // Check for Ethernet hardware being present
    Serial.println("Ethernet shield was not found.");          // If it is missing, print to serial monitor
    while (Ethernet.hardwareStatus() == EthernetNoHardware) {  // While the Hardware is missing
      delay(1000);                                             // wait for 1 second
      Serial.print(".");                                       // and print a dot to the serial monitor
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {                // Check to see if there is a physical connection to the network
    Serial.println("Ethernet cable is not connected.");  // if there is not then print it to the serial monitor
    while (Ethernet.linkStatus() == LinkOFF) {           // while there is no connection
      delay(1000);                                       // wait for 1 second
      Serial.print(".");                                 // and print a dot to the serial monitor
    }
  }
  Udp.begin(localPort);  // start the UDP connection on the port declared

  Serial.println("Ethernet Initialized");  // print to the serial monitor that the ethernet is done initializing
}


///////// Function to initialize all of the interrupts needed///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void initializeInterrupt() {
  Serial.println("Initializing Interrupts....");  // Print to the serial monitor that the interrupts are being initialized

  pinMode(ETHERNET_INT, INPUT);                                                 // set the interrupt for the ethernet shield as an input
  attachInterrupt(digitalPinToInterrupt(ETHERNET_INT), ethernetFlag, FALLING);  // set the ethernet shield pin as an interrupt triggered on the falling edge and assign the function ethernetFlag as the ISR

  pinMode(OVER_CURRENT, INPUT);  // Set up interrupt for the Over Current sensor
  digitalWrite(OVER_CURRENT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(OVER_CURRENT), setEMG, FALLING);  // set the over current pin as an interrupt triggered on the falling edge and assign the function overCurrent as the ISR

  for (int i = 0; i < 8; i++) {  // Configure Wiznet W5500 interrupts
    W5100.writeSnIMR(i, 0x04);   // socket ISR mask; RECV for all sockets
  }
  enableSIRs();  // Enable ethernet interrupts

  Serial.println("Interrupts Initialized");  // Print to the serial monitor that the interrupts are initialized
}


///////// Read the ethernet message recieved and update values ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void readEthernet() {
  if (ethernet_flag) {  // If the Ethernet Flag has been set by an interrupt,
    disableSIRs();      // Disable interrupts so that data is not corrupted while reading and writing the variables below

    unsigned long recieved;                      // Define a long to hold the binary form sent in the UDP packet
    char incoming[UDP_TX_PACKET_MAX_SIZE] = "";  // Define a local variable to hold the string of bits sent

    int packetSize = Udp.parsePacket();  // Define a local variable equal to the size of the UDP packet

    if (packetSize) {                                // If the packet size is larger than zero,
      if (memcmp(Udp.remoteIP(), server_ip, 4)) {    // If the IP address from the UDP message match's that of the server,
        Udp.read(incoming, UDP_TX_PACKET_MAX_SIZE);  // Read the UDP message and assaign it to incoming
        recieved = strtoul(incoming, NULL, 16);      // Convert the character array of hex bits into an unsigned long
        Serial.println(recieved);
      } else {                                         // If the IP address from the UDP message does not match that of the server,
        Serial.print("ERROR... IP address unknown.");  // Print to the serial monitor that there is an error because the sender is unknown
      }
    } else {                                              // If the packet size is equal to zero,
      Serial.print("ERROR... False Ethernet Interrupt");  // Print to the serial monitor that is an error because the ethernet flag was falsely set
    }

    mode_select = (recieved >> MODE_SELECT_SHIFT) & MASK_1;  // Parse recieved[0] for mode_select

    if (mode_select) {                                                // If the system is in autonomous mode, update the rest of the values
      EMG_reset = (recieved >> EMG_RESET_SHIFT) & MASK_1;             // Parse recieved[1] for EMG_reset
      EMG |= (recieved >> EMG_SHIFT) & MASK_1;                        // Parse recieved[2] for EMG
      target_heading = (recieved >> TARGET_HEADING_SHIFT) & MASK_16;  // Parse recieved[3:18] for target_heading
      Serial.print("message recieved, Target heading = ");
      Serial.println(target_heading);
    }

    if (target_heading > current_heading) {
      mid_point = current_heading - ((current_heading - target_heading) / 2);  // Calculate the midpoint with the new target
      offset = (target_heading - current_heading) / 20;                        // Calculate the offset with the new target
    } else {
      mid_point = current_heading + ((target_heading - current_heading) / 2);  // Calculate the midpoint with the new target
      offset = (current_heading - target_heading) / 20;                        // Calculate the offset with the new target
    }

    if (EMG_reset) {  // If the emergency reset bit is high, reset the EMG variable
      EMG = false;
      EMG_reset = false;
    }

    ethernet_flag = 0;  // Reset the ethernet flag so that the next interrupt can set it
    clearSIRs();        // clear all interrupt flags from the Wiznet 5500
    enableSIRs();       // re-enable interrupts now that we are done
  }
}


///////// Function for reading values from Sensors, Switch and Mode ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void readSensors() {
  torque_a = analogRead(TORQUE_A);                                                              // Read the input pin of Torque sensor A
  torque_b = analogRead(TORQUE_B);                                                              // Read the input pin of Torque sensor B
  if ((torque_a - torque_b > MAX_TORQUE || torque_b - torque_a > MAX_TORQUE) && mode_select) {  // If the difference of the values is larger than the threshold
    EMG = true;                                                                                 // Set the Emergency stop flag
  }

  //READ BOURNS ENCODER
  digitalWrite(ENCODER_CS, HIGH);  // start with the device deactivated
  digitalWrite(ENCODER_CS, LOW);   // Pull CS low to signal that data is wanted
  current_heading = 0;             // Clear previous data

  for (int i = 0; i < 10; i++) {                                    // cycle through all 16 bits that will be sent
    digitalWrite(ENCODER_CLK, LOW);                                 // pull the clk low to signal we are done with the previous bit
    digitalWrite(ENCODER_CLK, HIGH);                                // pull the clk high to signal that the next bit is desire
    current_heading = current_heading << 1;                         // Shift the bits left by one
    current_heading = current_heading | digitalRead(ENCODER_DATA);  // If the read bit is "1" then assign one to the first bit
  }
  digitalWrite(ENCODER_CLK, LOW);   // signal that we are done with the last bit
  digitalWrite(ENCODER_CLK, HIGH);  // pull the clk high until the next data is desired

  //TRANSITION TESTING
  if (old_current_heading > 800 && current_heading < 200) revolutions++;       // Zero cross low to high test
  else if (old_current_heading < 200 && current_heading > 800) revolutions--;  // Zero cross high to low test

  old_current_heading = current_heading;                                    // Update old_current_heading for future comparison
  current_heading = current_heading + revolutions * 1024 + encoder_offset;  // Incorperate the revolutions and offset into current_heading
}


///////// Function to return the state of emergency stop ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool state() {
  return EMG;
}


///////// Interrupt function to halt steering //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void EMGaction() {
  Serial.println("EMERGENCY ACTION HAS TAKEN PLACE");
  //target_heading = current_heading;
  current_speed = 0;
  current_direction = 0;
  feedback();
}


///////// Function that contains the algorith to adjust the automated outputs based on the current and target values to meet our desired performance criteria/////////////////////////////////////////////////////////////////


void decideAction() {  // Make a decsion based on the target and current values
  if (mode_select) {
    if (current_heading < target_heading - wiggle_room) {
      wiggle_room = 1;
      if (current_heading < mid_point - offset) {
        current_speed = acc(current_speed, 1);
        current_direction = direction_bit;
      } else if (current_heading > mid_point + offset) {
        current_speed = acc(current_speed, -1);
        current_direction = direction_bit;
      } else {
        // Purposefully Blank // In middle gap
      }
    }
    /////////////////////////////////////////////////////////////
    else if (current_heading > target_heading + wiggle_room) {
      wiggle_room = 1;
      if (current_heading > mid_point - offset) {
        current_speed = acc(current_speed, 1);
        current_direction = !direction_bit;
      } else if (current_heading < mid_point + offset) {
        current_speed = acc(current_speed, -1);
        current_direction = !direction_bit;
      } else {
        // Purposefully Blank // In middle gap
      }
    }
    /////////////////////////////////////////////////////////////
    else {
      current_speed = 0;
      current_direction = 0;
      wiggle_room = 8;
    }
  } else {
    if (torque_a - torque_b > POWER_STEERING_THRESHOLD) {
      current_speed = 120;
      current_direction = 1;
    } else if (torque_b - torque_a > POWER_STEERING_THRESHOLD) {
      current_speed = 120;
      current_direction = 0;
    } else {
      current_speed = 0;
      current_direction = 0;
    }
  }
}


///////// Function that writes all of the automated outputs for our system /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void updateOutput() {  //
  analogWrite(PWM, current_speed);
  digitalWrite(DIRECTION, current_direction);
}


///////// Function to send data back to the server to be checked for errors /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void feedback() {
  int current_time = millis();  // Set the current time in milliseconds using the arduino function millis();

  if ((current_time - previous_feedback) > FEEDBACK_PERIOD) {  // If the period it has been waiting is greater than the period specified,

    unsigned long feedback_1 = 0;  // Holds part of the feedback message in binary form
    unsigned long feedback_2 = 0;  // Holds part of the feedback message in binary form

    ////////////////////////////////////////////
    feedback_2 = (feedback_2 << 16) | current_heading;  // feedback_2 [16:31]
    feedback_2 = (feedback_2 << 16) | target_heading;   // feedback_2 [0:15]
    /////////////////////////////////////
    feedback_1 = (feedback_1 << 1) | current_direction;  // feedback_1 [2]
    /////////////////////////////////////
    feedback_1 = (feedback_1 << 1) | EMG;          // feedback_1 [1]
    feedback_1 = (feedback_1 << 1) | mode_select;  // feedback_1 [0]

    char first[UDP_TX_PACKET_MAX_SIZE];    // Defined to hold the first half of the resulting char array
    char second[UDP_TX_PACKET_MAX_SIZE];   // Defined to hold the second half of the resulting char array
    char message[UDP_TX_PACKET_MAX_SIZE];  // Defined to hold the final feedback message in a char array

    ultoa(feedback_2, second, HEX);  // Convert feedback_2 into Hexadecimal characters and store them in a char array
    ultoa(feedback_1, first, HEX);   // Convert feedback_1 into Hexadecimal characters and store them in a char array
    strcpy(message, second);         // copy the first half of the message into the final message char array
    strcat(message, first);          // copy the second half of the message into the final message char array

    Udp.beginPacket(server_ip, localPort);
    Udp.write(message);
    Udp.endPacket();

    previous_feedback = current_time;  // Update previous time with the last time the variable was incremented
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void debug() {  // Provide feedback to the executive processor about the
  //  Serial.print(millis());
  //  Serial.print(",");
  //  Serial.print(mode_select);
  //  Serial.print(",");
    Serial.print(EMG);
    Serial.print("    ");
  //  Serial.print(EMG_reset);
  //  Serial.print(",");
    Serial.print(target_heading);
    Serial.print("    ");
    Serial.println(current_heading);
  //  Serial.print(",");
  //  Serial.print(mid_point);
  //  Serial.print(",");
  //  Serial.print(offset);
  //  Serial.print(",");
  //  Serial.print(current_speed);
  //  Serial.print(",");
  //  Serial.print(current_direction);
  //  Serial.print(",");
  //  Serial.print(torque_a);
  //  Serial.print(",");
  //  Serial.print(torque_b);
  //  Serial.print(",");
  //  Serial.print(wiggle_room);
  //  Serial.print(",");
  //  Serial.print(revolutions);
  //  Serial.println("");

  /*Serial.print(String(millis()) + ", " + String(mode_select) + ", "
               + String(EMG) + ", " + String(EMG_reset) + ", " + String(target_heading) + ", "
               + String(current_heading) + ", " + String(mid_point) + ", " + String(offset)
               + ", " + String(current_speed) + ", " + String(current_direction) + ", " + String(torque_a)
               + ", " + String(torque_b) + ", " + String(wiggle_room) + ", " + String(revolutions));

  Serial.println("");*/
}
