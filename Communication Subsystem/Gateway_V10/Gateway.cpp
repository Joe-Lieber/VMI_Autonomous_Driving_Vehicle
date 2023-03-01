#include "Gateway.h"
#include <Ethernet.h>
#include "w5100mod.h"  // Adapted from Ethernet library, adds access to socket IR registers
#include <avr/wdt.h>


const int ETHERNET_INT = 2;  // Interrupt pin used by ethernet shield


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


volatile bool ethernet_flag = false;                        // Flag to signal that an ethernet message is ready to be read
IPAddress ip(192, 168, 1, 2);                               // This systems IP address
const byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };  // This systems MAC address
const byte steering_ip[] = { 192, 168, 1, 3 };              // IP address of the Steering subsystem
const byte throttle_braking_ip[] = { 192, 168, 1, 4 };      // IP address of the Throttle / Braking subsystem
unsigned int port1 = 8888;                                  // Port used for instance 1 (Udp1)
unsigned int port2 = 8887;                                  // Port used for instance 2 (Udp2)
EthernetUDP Udp1;                                           // An EthernetUDP instance that opens a socket to send and recieve packets
EthernetUDP Udp2;                                           // An EthernetUDP instance that opens a socket to send and recieve packets


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


const int MESSAGE_LENGTH = 20; // Length of the entire serial message coming in **** If the full message is not being read, increase this value ****
const int STEERING_PACKET_LENGTH = 5;
const int THROTTLE_BRAKING_PACKET_LENGTH = 6;

char steering_packet[STEERING_PACKET_LENGTH+1] = "";          // Used for incoming messages from the Steering subsystem
char previous_steering_packet[STEERING_PACKET_LENGTH+1] = "";
char steering_buffer[STEERING_PACKET_LENGTH+1] = "";          // Used for outgoing messages to the Steering subsystem

char throttle_braking_packet[THROTTLE_BRAKING_PACKET_LENGTH] = "";  // Used for incoming messages from the Throttle / Braking subsystem
char previous_throttle_braking_packet[THROTTLE_BRAKING_PACKET_LENGTH] = "";
char throttle_braking_buffer[THROTTLE_BRAKING_PACKET_LENGTH] = "";  // Used for outgoing messages to the Throttle / Braking subsystem


////////clears the SIR registers so another interrupt can be triggered////////////////////////////////////////////////////////////////////


void clearSIRs() {  // After a socket IR, SnIR and SIR need to be reset
  for (int i = 0; i < 8; i++) {
    W5100.writeSnIR(i, 0xFF);  // Clear socket i interrupt
  }
  W5100.writeSIR(0xFF);  // Clear SIR
}


/////////enable/disable interrupts from ethernet shield///////////////////////////////////////////////////////////////////////////////////


inline void disableSIRs() {
  W5100.writeSIMR(0x00);  // disable interrupts for all sockets
}
inline void enableSIRs() {
  W5100.writeSIMR(0xFF);  // enable interrupts for all sockets
}


////////sets a flag showing an ethernet message was recieved//////////////////////////////////////////////////////////////////////////////


void ethernetFlag() {
  ethernet_flag = true;
}


/////////Initialize generic arduino settings such as input and outputs////////////////////////////////////////////////////////////////////


void initialize() {
  Serial.begin(9600);
  Serial.println("Initializing server....");
  //wdt_enable(WDTO_1S);  // Eneable the watchdog timer to execute an interrupt if it reaches 1 second
}


////////Initialize ethernet setting and makes sure it starts up correctly/////////////////////////////////////////////////////////////////


void initializeEthernet() {

  Ethernet.init(10);  // You can use Ethernet.init(pin) to configure the CS pin. Most Arduino shields use pin 10

  // Start the Ethernet connection:
  Serial.println("Initializing Ethernet...");  // Print that the Ethernet connection is starting
  Ethernet.begin(mac, ip);                     // Begin ethernet connection

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {       // Check for Ethernet hardware being present
    Serial.println("Ethernet shield was not found");           // If it is missing, print to serial monitor
    while (Ethernet.hardwareStatus() == EthernetNoHardware) {  // While the Hardware is missing
      delay(1000);                                             // wait for 1 second
      Serial.print(".");                                       // and print a dot to the serial monitor
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (Ethernet.linkStatus() == LinkOFF) {                // Check to see if there is a physical connection to the network
    Serial.println("Ethernet cable is not connected.");  // if there is not then print it to the serial monitor
    while (Ethernet.linkStatus() == LinkOFF) {           // while there is no connection
      delay(1000);                                       // wait for 1 second
      Serial.print(".");                                 // and print a dot to the serial monitor
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  // start UDP
  Udp1.begin(port1);  // start the UDP connection on the port declared
  Udp2.begin(port2);  // start the UDP connection on the port declared

  Serial.println("Ethernet Initialized");  // print to the serial monitor that the ethernet is done initializing
}


///////Initialize all settings for arduino interrupts and configure ethernet shield to produce interrupt//////////////////////////////////


void initializeInterrupt() {
  Serial.println("Initializing Interrupts....");  // Print to the serial monitor that the interrupts are being initialized

  pinMode(ETHERNET_INT, INPUT);                                                 // set the interrupt for the ethernet shield as an input
  attachInterrupt(digitalPinToInterrupt(ETHERNET_INT), ethernetFlag, FALLING);  // set the ethernet shield pin as an interrupt triggered on the falling edge and assign the function ethernetFlag as the ISR

  for (int i = 0; i < 8; i++) {  // Configure Wiznet W5500 interrupts
    W5100.writeSnIMR(i, 0x04);   // socket ISR mask; RECV for all sockets
  }
  enableSIRs();  // Enable ethernet interrupts

  Serial.println("Interrupts Initialized");  // Print to the serial monitor that the interrupts are initialized
}

//////////read the UDP packet that was recieved///////////////////////////////////////////////////////////////////////////////////////////


void readEthernet() {

  if (ethernet_flag) {  // If the Ethernet Flag has been set by an interrupt,
    disableSIRs();      // Disable interrupts so that data is not corrupted while reading and writing the variables below

    int packetSize1 = Udp1.parsePacket();  // Define a local variable equal to the size of the UDP packet
    int packetSize2 = Udp2.parsePacket();  // Define a local variable equal to the size of the UDP packet

    if (packetSize1) {

      if (memcmp(Udp1.remoteIP(), steering_ip, UDP_TX_PACKET_MAX_SIZE)) {  //if the IP address matches the steering subsystem save it there
        Udp1.read(steering_buffer, UDP_TX_PACKET_MAX_SIZE);
        Serial.println(steering_buffer);  // ********** Need to add a process to sort this to several char arrays that holds 8 chars each then converts to unsigned longs to go to ints

        //mode_select = (recieved >> MODE_SELECT_SHIFT) & MASK_1;         // Parse recieved[0] for mode_select
        //EMG_reset = (recieved >> EMG_RESET_SHIFT) & MASK_1;             // Parse recieved[1] for EMG_reset
        //target_heading = (recieved >> TARGET_HEADING_SHIFT) & MASK_16;  // Parse recieved[4:19] for target_heading
      }
    }
    if (packetSize2) {
      if (memcmp(Udp2.remoteIP(), throttle_braking_ip, 4)) {  //if the IP address matches the throttle/braking subsystem save it there
        Udp2.read(throttle_braking_buffer, UDP_TX_PACKET_MAX_SIZE);
        Serial.println(throttle_braking_buffer);  // ********** Need to add a process to sort this to several char arrays that holds 8 chars each then converts to unsigned longs to go to ints

        //mode_select = (recieved >> MODE_SELECT_SHIFT) & MASK_1;         // Parse recieved[0] for mode_select
        //EMG_reset = (recieved >> EMG_RESET_SHIFT) & MASK_1;             // Parse recieved[1] for EMG_reset
        //target_heading = (recieved >> TARGET_HEADING_SHIFT) & MASK_16;  // Parse recieved[4:19] for target_heading
      }
    }

    ethernet_flag = 0;
    clearSIRs();
    enableSIRs();
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// BARKER MAKE THIS WORK :(((((
void errorCheck() {
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void sendEthernet() {

  Udp1.beginPacket(steering_ip, port1);
  Udp2.beginPacket(throttle_braking_ip, port2);

  if(previous_steering_packet != steering_packet) {
    Udp1.write(steering_packet);
    Serial.println(".")
    strcpy(previous_steering_packet, steering_packet);
  }
  if(previous_throttle_braking_packet != throttle_braking_packet) {
    Udp2.write(throttle_braking_packet);
    Serial.println("_")
    strcpy(previous_throttle_braking_packet, throttle_braking_packet);
  } 
  Udp1.endPacket();
  Udp2.endPacket();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//[Brake Switch(4 bits), Brake pwm(8 bits), Throttle Switch(4 bits), Throttle pwm(8 bits), EMPTY(2 bit), Emergency Reset(1 bit), Mode Select(1 bits)]
//[Target Heading(16 bits), Empty(2 bit), Emergency Reset(1 bit), Mode Select(1 bit)]

void serialHandle() {
  char id = "";
  char input[200] = {};
  int current_char = 0;

  int available_bytes = Serial.available();
///////////////////////////////////////////
  if (!available_bytes) return;
  delay(MESSAGE_LENGTH);
///////////////////////////////////////////
  available_bytes = Serial.available();
  Serial.print("available_bytes: ");
  Serial.println(available_bytes);

  for (int i = 0; i < available_bytes; i++) {
    input[i] = Serial.read();
  }
  Serial.print("Packet: ");
  Serial.print(input);
//////////////////////////////////////////
  while (current_char < available_bytes) {
    id = input[current_char];
    current_char++;
    if(id == "") continue;
    
    Serial.print("ID: ");
    Serial.println(id);

    if (id == '0') {  //Steering
      for (int i = current_char; i < current_char + STEERING_PACKET_LENGTH; i++) {
        steering_packet[i - current_char] = input[i];
      }
      current_char += STEERING_PACKET_LENGTH;  //number of characters in the message
      Serial.print("Steering Packet: ");
      Serial.println(steering_packet);
    }

    else if (id == '1') {  //Throttle Braking
      for (int i = current_char; i < current_char + THROTTLE_BRAKING_PACKET_LENGTH; i++) {
        throttle_braking_packet[i - current_char] = input[i];
      }
      current_char += THROTTLE_BRAKING_PACKET_LENGTH;  //number of characters in the message
      Serial.print("Throttle Braking Packet: ");
      Serial.println(throttle_braking_packet);
    }

    else Serial.println("Error! ID not recognized...");
  }
  sendEthernet();
}




void debug() {
  if (Serial.available()) {
    Serial.print(".");
    char x = Serial.read();

    if (x == 's') {
      strcpy(steering_packet, "00003");
      strcpy(throttle_braking_packet, "1FE003");
    } else if (x == 'w') {
      strcpy(steering_packet, "00003");
      strcpy(throttle_braking_packet, "00083B");
    } else if (x == 'a') {
      strcpy(steering_packet, "7D123");
      strcpy(throttle_braking_packet, "00083B");
    } else if (x == 'd') {
      strcpy(steering_packet, "02EE3");
      strcpy(throttle_braking_packet, "00083B");
    } else if (x == 'e') {
      strcpy(steering_packet, "01773");
      strcpy(throttle_braking_packet, "00083B");
    } else if (x == 'q') {
      strcpy(steering_packet, "7E893");
      strcpy(throttle_braking_packet, "00083B");
    }
    sendEthernet();
  }
//char wtb[6] = { "00083B" };
//char wtb[6] = { "000F1B" };
//char wtb[6] = { "00191B" };

//char adtb[6] = { "00083B" };
//char adtb[6] = { "000A1B" };
//char adtb[6] = { "000F1B" };

//char qetb[6] = { "00083B" };
//char qetb[6] = "000C9B";
//char qetb[6] = "0012DB";

//char stb[6] = { "1FE003" };

//char ss[5] = { "00003" };
//char ws[5] = { "00003" };
//char as[5] = { "7D123" };
//char ds[5] = { "02EE3" };
//char qs[5] = { "7E893" };
//char es[5] = { "01773" };
}
