# Communication Subsystem

### Purpose:
The communications subsystem is responsible for establishing a communications protocol among all 
components of the vehicle’s automations in order to ensure that every device is able to send and receive data/commands.

### Solution:
This internal network will be a UDP ethernet connection with statically assigned IP addresses for each subsystem.

### Implementation:
This system is implemented using the V2 Arduino Ethernet Shields. As displayed below, the system has an entry node consisting of an Arduino uno and ethernet shield.
The purpose of this node to to provide an easy way to interface with the core functionality of the vehicle while isolating the resources needed for operation. This
node connects to an unmanaged switch where all other devices on the network will be connected. The entry node will recieve a serial message from the another device
based on the protocol defined below. The entry node will then process this message and pass the data specified on to the system or systems specified.
