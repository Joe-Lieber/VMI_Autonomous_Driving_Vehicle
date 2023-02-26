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

### Message Structures
This section will define the Message structure for each message that is sent throughout the system

#### Gateway Command
| Device ID(4 bits) | Command |
| --- | --- |
| ID for device on network | Data for command specified below |

#### Gateway Feedback
| Variables | Target Brake pwm(8_bits) | Target Throttle pwm(8_bits) | Target_Heading(16_bits) | Target Brake Switch(1_bit) | Target Throttle Switch(1_bit) | Emergency (1_bit) | Mode_Select (1_bit) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Input_Range | 0 : 255 | 0 : 255 | -2500 : 2500 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 |

#### Steering Command (19 bits)
| Variables | Target_Heading(16_bits) |  Emergency  (1_bit) | Emergency_Reset (1_bit) | Mode_Select (1_bit) |
| --- | --- | --- | --- | --- |
| Input Range | -2500 : 2500 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 |
| Important Values | 0 = Center | 1 = Set | 1 = Reset | 0 = Manual |
| Important_Values | >0 = Left |  |  | 1 = Automated |
| Important_Values | <0 = Right |  |  |  |

#### Steering Feedback (35 bits)
| Variables | Current_Heading(16_bits) | Target_Heading(16_bits) | Current_Direction (1_bit) |  Emergency  (1_bit) | Mode_Select (1_bit) |
| --- | --- | --- | --- | --- | --- |
| Input_Range | -2500 : 2500 | -2500 : 2500 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 |
| Important_Values | 0 = Center | 0 = Center | 0 = Right | 0 = False | 0 = Manual |
| Important_Values | >0 = Left | >0 = Left | 1 = Left | 1 = True | 1 = Automated |
| Important_Values | <0 = Right | <0 = Right |  |  |  |

#### Throttle / Braking Command (21 bits)
| Variables | Target Brake pwm(8_bits) | Target Throttle pwm(8_bits) | Target Brake Switch(1_bit) | Target Throttle Switch(1_bit) |  Emergency  (1_bit) | Emergency_Reset (1_bit) | Mode_Select (1_bits) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Input_Range | 0 : 255 | 0 : 255 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 |
| Important_Values | min = X | min = x | 0 = ON | 0 = OFF | 1 = Set | 1 = Reset | 0 = Manual |
| Important_Values | max = Y | max = y | 1 = OFF | 1 = ON |  |  | 1 = Automated |

#### Throttle / Braking Feedback (40 bits)
| Variables | Current Brake pwm(8_bit) | Current Throttle pwm(8_bit) | Target Brake pwm(8_bits) | Target Throttle pwm(8_bits) | Brake Switch(1_bit) | Throttle Switch(1_bit) | Current Brake Switch(1_bit) | current Throttle Switch(1_bit) | Target Brake Switch(1_bit) | Target Throttle Switch(1_bit) |  Emergency  (1_bit) | Mode_Select (1_bits) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Input_Range | 0 : 255 | 0 : 255 | 0 : 255 | 0 : 255 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 | Logic 0 or 1 |
| Important_Values | min = 26 | min = 26 | min = 26 | min = 26 | 0 = ON | 0 = OFF | 0 = ON | 0 = OFF | 0 = ON | 0 = OFF | 0 = False | 0 = Manual |
| Important_Values | max = 229 | max = 229 | max = 229 | max = 229 | 1 = OFF | 1 = ON | 1 = OFF | 1 = ON | 1 = OFF | 1 = ON | 1 = True | 1 = Automated |
