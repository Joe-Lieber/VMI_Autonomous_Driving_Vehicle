import serial
from pynput import keyboard

########################################################################################################################

ser = serial.Serial(
    port='/dev/ttyACM0',
    baudrate=9600,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)
ser.isOpen()

#########################################################################################################################

SOFT_RIGHT = 0x02EE # 750
SOFT_LEFT = 0xFD12 # -750
HARD_RIGHT = 0x05DC # 1500
HARD_LEFT = 0xFA24 # -1500
CENTER = 0x0 # center is 0

THROTTLE_1 = 0x41 # base value of throttle set to 65
THROTTLE_2 = THROTTLE_1 * 1.75 # base times multiplier 1.75
THROTTLE_3 = THROTTLE_1 * 2.5 # base times multiplier 2.5

SOFT_TURN_2 = THROTTLE_1 * 1.4 # base times multiplier 1.4
SOFT_TURN_3 = THROTTLE_1 * 2 # base times multiplier 2

HARD_TURN_2 = THROTTLE_1 *1.3
HARD_TURN_3 = THROTTLE_1 *1.7

BRAKE = 0xFF # brake PWM will always be 255 unless 0

THROTTLE_BRAKE_ID = 0x1
STEERING_ID = 0x0

#########################################################################################################################

flag = 0x0                     # Logic flag to send one command per key hold

mode_select = 0x0              # Initialize mode to manual
emergency_reset = 0x0          # Do not reset emergency until ordered

throttle_f = THROTTLE_1        # Current throttle value for forward
throttle_q_e = THROTTLE_1      # Current throttle value for soft turns
throttle_a_d = THROTTLE_1      # Current throttle value for hard turns
emergency = 0x0                # No emergency in the syswtem
current_heading = 0x0          # Initialize to center
current_throttle_switch = 0x0  # initialize to OFF
current_throttle_pwm = 0x0     # initialize to OFF
current_brake_switch = 0xFF    # initialize to ON
current_brake_pwm = 0x0        # initialize to ON

#########################################################################################################################

def buildSteering( MODE_SELECT, EMERGENCY_RESET, EMERGENCY, TARGET_HEADING):
    global mode_select
    global emergency
    global current_heading
    mode_select = MODE_SELECT
    emergency = EMERGENCY
    current_heading = TARGET_HEADING
    
    packet = TARGET_HEADING
    packet = (packet << 1) | EMERGENCY
    packet = (packet << 1) | EMERGENCY_RESET
    packet = (packet << 1) | MODE_SELECT
    packet = str(hex(packet))
    return packet

##########################################################################################################################

def buildThrottleBraking( ID, MODE_SELECT,  EMERGENCY_RESET, EMERGENCY, TARGET_THROTTLE_SWITCH, TARGET_BRAKE_SWITCH, TARGET_THROTTLE_PWM, TARGET_BRAKE_PWM):
    global mode_select
    global emergency
    global current_throttle_switch
    global current_throttle_pwm
    global current_brake_switch
    global current_brake_pwm
    mode_select = MODE_SELECT
    emergency = EMERGENCY
    current_throttle_switch = TARGET_THROTTLE_PWM
    current_throttle_pwm = TARGET_THROTTLE_PWM
    current_brake_switch = TARGET_BRAKE_SWITCH
    current_brake_pwm = TARGET_BRAKE_PWM
    
    packet = TARGET_BRAKE_PWM
    packet = (packet << 8) | TARGET_THROTTLE_PWM
    packet = (packet << 1) | TARGET_BRAKE_SWITCH
    packet = (packet << 1) | TARGET_THROTTLE_SWITCH
    packet = (packet << 1) | EMERGENCY
    packet = (packet << 1) | EMERGENCY_RESET
    packet = (packet << 1) | MODE_SELECT
    packet = (packet << 4) | ID
    packet = str(hex(packet))
    return packet

##########################################################################################################################

def on_press(key):
    global throttle_f      # Current throttle value for forward
    global throttle_q_e    # Current throttle value for soft turns
    global throttle_a_d    # Current throttle value for hard turns
    global flag

    if(flag):
        return
    
    flag = True

    try:
        if key.char == 'b':                     # trigger the emergency stop functionality, FIRST PRIORITY
            ser.write(buildSteering(mode_select, emergency_reset, 0x1, current_heading))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, 0x1, 0x0, 0x0, 0x0, BRAKE))
            print('b Pressed')
            
        elif key.char == 's':                   # Stop the Vehicle, SECOND PRIORITY
            ser.write(buildSteering(mode_select, emergency_reset, emergency, current_heading))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x0, 0x0, 0x0, BRAKE))
            print('s Pressed')
        
        elif key.char == 'w':                   # Move forward
            ser.write(buildSteering(mode_select, emergency_reset, emergency, CENTER))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x1, 0x1, throttle_f, 0x0))
            print('w Pressed')

        elif key.char == 'a':                   # Move Left
            ser.write(buildSteering(mode_select, emergency_reset, emergency, HARD_LEFT))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x1, 0x1, throttle_a_d, 0x0))
            print('a Pressed')

        elif key.char == 'd':                   # Move Right
            ser.write(buildSteering(mode_select, emergency_reset, emergency, HARD_RIGHT))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x1, 0x1, throttle_a_d, 0x0))
            print('d Pressed')

        elif key.char == 'q':                   # Soft left
            ser.write(buildSteering(mode_select, emergency_reset, emergency, SOFT_LEFT))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x1, 0x1, throttle_q_e, 0x0))
            print('q Pressed')

        elif key.char == 'e':                   # Soft Right
            ser.write(buildSteering(mode_select, emergency_reset, emergency, SOFT_RIGHT))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x1, 0x1, throttle_q_e, 0x0))
            print('e Pressed')

        elif key.char == 'r':                   # Reset the Emergency stop bit
            ser.write(buildSteering(mode_select, 0x1, emergency, current_heading))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, 0x1, emergency, current_throttle_switch, current_brake_switch, current_throttle_pwm, current_brake_pwm))
            print('r Pressed')

        elif key.char == 'm':                   # Internally Flip the mode select bit
            ser.write(buildSteering(not mode_select, emergency_reset, emergency, current_heading))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, not mode_select, emergency_reset, emergency, current_throttle_switch, current_brake_switch, current_throttle_pwm, current_brake_pwm))
            print('m Pressed')

        elif key.char == '1':              # Internally update Throttle Values
            throttle_f = THROTTLE_1        # Current throttle value for forward
            throttle_q_e = THROTTLE_1      # Current throttle value for soft turns
            throttle_a_d = THROTTLE_1      # Current throttle value for hard turns
            print('1 Pressed')
            
        elif key.char == '2':              # Internally update Throttle Values
            throttle_f = THROTTLE_2        # Current throttle value for forward
            throttle_q_e = SOFT_TURN_2     # Current throttle value for soft turns
            throttle_a_d = HARD_TURN_2     # Current throttle value for hard turns
            print('2 Pressed')

        elif key.char == '3':              # Internally update Throttle Values
            throttle_f = THROTTLE_3        # Current throttle value for forward
            throttle_q_e = SOFT_TURN_3     # Current throttle value for soft turns
            throttle_a_d = HARD_TURN_3     # Current throttle value for hard turns
            print('3 Pressed')
    except AttributeError:
            flag = False
            print("ERROR. Key pressed is not defined.")

#########################################################################################################################

def on_release(key):
    global flag
    flag = False
    
    try:
        
        if key.char == 'w':                   # Move forward
            ser.write(buildSteering(mode_select, emergency_reset, emergency, CENTER))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x0, 0x1, 0x0 0x0))
            print('w Released')

        elif key.char == 'a':                   # Move Left
            ser.write(buildSteering(mode_select, emergency_reset, emergency, HARD_LEFT))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x0, 0x1, 0x0 0x0))
            print('a Released')

        elif key.char == 'd':                   # Move Right
            ser.write(buildSteering(mode_select, emergency_reset, emergency, HARD_RIGHT))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x0, 0x1, 0x0 0x0))
            print('d Released')

        elif key.char == 'q':                   # Soft left
            ser.write(buildSteering(mode_select, emergency_reset, emergency, SOFT_LEFT))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x0, 0x1, 0x0 0x0))
            print('q Released')

        elif key.char == 'e':                   # Soft Right
            ser.write(buildSteering(mode_select, emergency_reset, emergency, SOFT_RIGHT))
            ser.write(buildThrottleBraking(THROTTLE_BRAKE_ID, mode_select, emergency_reset, emergency, 0x0, 0x1, 0x0 0x0))
            print('e Released')

    except AttributeError:
        if key == keyboard.Key.esc:
            # Stop listener    
            return False
        
    
# Collect events until released
with keyboard.Listener(
        on_press=on_press,
        on_release=on_release) as listener:
    listener.join()