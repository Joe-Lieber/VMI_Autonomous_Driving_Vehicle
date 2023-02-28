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


char steering_packet[UDP_TX_PACKET_MAX_SIZE] = "";          // Used for incoming messages from the Steering subsystem
char steering_buffer[UDP_TX_PACKET_MAX_SIZE] = "";          // Used for outgoing messages to the Steering subsystem
char throttle_braking_packet[UDP_TX_PACKET_MAX_SIZE] = "";  // Used for incoming messages from the Throttle / Braking subsystem
char throttle_braking_buffer[UDP_TX_PACKET_MAX_SIZE] = "";  // Used for outgoing messages to the Throttle / Braking subsystem


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

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {       // Check for Ethernet hardware being present
    Serial.println("Ethernet shield was not found");           // If it is missing, print to serial monitor
    while (Ethernet.hardwareStatus() == EthernetNoHardware) {  // While the Hardware is missing
      delay(1000);                                             // wait for 1 second
      Serial.print(".");                                       // and print a dot to the serial monitor
    }
    if (Ethernet.linkStatus() == LinkOFF) {                // Check to see if there is a physical connection to the network
      Serial.println("Ethernet cable is not connected.");  // if there is not then print it to the serial monitor
      while (Ethernet.linkStatus() == LinkOFF) {           // while there is no connection
        delay(1000);                                       // wait for 1 second
        Serial.print(".");                                 // and print a dot to the serial monitor
      }
    }
    // start UDP
    Udp1.begin(port1);  // start the UDP connection on the port declared
    Udp2.begin(port2);  // start the UDP connection on the port declared

    Serial.println("Ethernet Initialized");  // print to the serial monitor that the ethernet is done initializing
  }
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

  Udp1.write(steering_packet);
  Udp2.write(throttle_braking_packet);

  Udp1.endPacket();
  Udp2.endPacket();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//[Brake Switch(4 bits), Brake pwm(8 bits), Throttle Switch(4 bits), Throttle pwm(8 bits), EMPTY(2 bit), Emergency Reset(1 bit), Mode Select(1 bits)]
//[Target Heading(16 bits), Empty(2 bit), Emergency Reset(1 bit), Mode Select(1 bit)]

void serialHandle() {
  int i = 0;
  char incoming[24];
  char data[8];
  char null_data[8] = {};
  char id_str[2];
  int id = 0;
  int used_chars = 0;
  int length = 0;

  while (Serial.available()) {
    incoming[length] = Serial.read();
    length++;
  }

  while (used_chars < length) {
    id_str[used_chars] = incoming[used_chars];
    id = strtoul(id_str, NULL, 16);

    if (id == 0) { //steering
      strcpy(null_data,data);
      for (i = used_chars; i < used_chars + 5; i++) {
        data[i - used_chars] = incoming[i];
      }
      used_chars = used_chars + 5;
      strcpy(data,steering_packet);
    }

    if (id == 1) { // Throttle Braking
      strcpy(null_data,data);
      for (i = used_chars; i < used_chars + 6; i++) {
        data[i - used_chars] = incoming[i];
      }
      used_chars = used_chars + 6;
      strcpy(data,throttle_braking_packet);
    }
  }
  sendEthernet();
}


char wtb[6] = { "0" "0" "0" "8" "3" "B" };
//char wtb[6] = { "0" "0" "0" "F" "1" "B" };
//char wtb[6] = { "0" "0" "1" "9" "1" "B" };

char adtb[6] = { "0" "0" "0" "8" "3" "B" };
//char adtb[6] = { "0" "0" "0" "A" "1" "B" };
//char adtb[6] = { "0" "0" "0" "F" "1" "B" };

char qetb[6] = { "0" "0" "0" "8" "3" "B" };
//char qetb[6] = "000C9B";
//char qetb[6] = "0012DB";

char stb[6] = { "1" "F" "E" "0" "0" "3" };

char ss[5] = { "0" "0" "0" "0" "3" };
char ws[5] = { "0" "0" "0" "0" "3" };
char as[5] = { "7" "D" "1" "2" "3" };
char ds[5] = { "0" "2" "E" "E" "3" };
char qs[5] = { "7" "E" "8" "9" "3" };
char es[5] = { "0" "1" "7" "7" "3" };

    
void debug(){
  if (Serial.available()){
    Serial.print(".");
    char x = Serial.read();

    if (x == 's'){
      strcpy(ss,steering_packet);
      strcpy(stb,throttle_braking_packet);
    }
    else if (x == 'w'){
      //strcpy(ws,steering_packet);
      strcpy(throttle_braking_packet,"00083B");
    }
    else if (x == 'a'){
      strcpy(as,steering_packet);
      strcpy(adtb,throttle_braking_packet);
    }
    else if (x == 'd'){
      strcpy(ds,steering_packet);
      strcpy(adtb,throttle_braking_packet);
    }
    else if (x == 'e'){
      strcpy(es,steering_packet);
      strcpy(qetb,throttle_braking_packet);
    }
    else if (x == 'q'){
      strcpy(qs,steering_packet);
      strcpy(qetb,throttle_braking_packet);
    }
    sendEthernet();
  }
}
