import serial
import keyboard

ser = serial.Serial(
    port='/dev/ttyACM0',
    baudrate=9600,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)

SteeringMessage = ""
T&BMessage = ""

ser.isOpen()

def on_press(key):
    if key.name == 'w':
        SteeringMessage = SteeringMessage + "000001"
        ser.write(SteeringMessage)
        #Throttle and Breaking
        mode_select = "1"
        emergency_reset = "0"
        throttle_switch = "1"
        breaking_switch = "1"
        empty = "000"
        throttle_pwm = "01010000" #80
        break_pwm = "00000000" #0

        T&BMessage = T&BMessage + break_switch + break_pwm + throttle_switch + throttle_switch + throttle_pwm + empty + emergency_reset + mode_select

        ser.write(T&BMessage)
        print('w was written to serial')

    elif key.name == 'a':
        SteeringMessage = SteeringMessage + "FA2401" #Note, this does compensate for the negative number yet

        #Throttle and Breaking
        mode_select = "1"
        emergency_reset = "0"
        throttle_switch = "1"
        breaking_switch = "1"
        empty = "000"
        throttle_pwm = "01000110" #70
        break_pwm = "00000000" #0

        T&BMessage = T&BMessage + break_switch + break_pwm + throttle_switch + throttle_switch + throttle_pwm + empty + emergency_reset + mode_select

        ser.write(T&BMessage)
        ser.write(SteeringMessage)

        print('a was written to serial')

    elif key.name == 's':
        SteeringMessage = SteeringMessage + "000001"
        ser.write(SteeringMessage)

        #Throttle and Breaking
        mode_select = "1"
        emergency_reset = "0"
        throttle_switch = "0"
        breaking_switch = "0"
        empty = "000"
        throttle_pwm = "00000000" #0
        break_pwm = "11111111" #255

        T&BMessage = T&BMessage + break_switch + break_pwm + throttle_switch + throttle_switch + throttle_pwm + empty + emergency_reset + mode_select

        ser.write(T&BMessage)
        print('s was written to serial')

    elif key.name == 'd':
        SteeringMessage = SteeringMessage + "5DC001"
        ser.write(SteeringMessage)
        #Throttle and Breaking
        mode_select = "1"
        emergency_reset = "0"
        throttle_switch = "1"
        breaking_switch = "1"
        empty = "000"
        throttle_pwm = "01000110" #70
        break_pwm = "00000000" #0

        T&BMessage = T&BMessage + break_switch + break_pwm + throttle_switch + throttle_switch + throttle_pwm + empty + emergency_reset + mode_select

        ser.write(T&BMessage)
        print('d was written to serial')

    elif key.name == 'q':
        SteeringMessage = SteeringMessage + "FD1201" #Does compensate for the negative number
        ser.write(SteeringMessage)
        #Throttle and Breaking
        mode_select = "1"
        emergency_reset = "0"
        throttle_switch = "1"
        breaking_switch = "1"
        empty = "000"
        throttle_pwm = "01001011" #75
        break_pwm = "00000000" #0

        T&BMessage = T&BMessage + break_switch + break_pwm + throttle_switch + throttle_switch + throttle_pwm + empty + emergency_reset + mode_select

        print('q was written to serial')
    elif key.name == 'e':
        SteeringMessage = SteeringMessage + "2EE001"
        ser.write(SteeringMessage)

        #Throttle and Breaking
        mode_select = "1"
        emergency_reset = "0"
        throttle_switch = "1"
        breaking_switch = "1"
        empty = "000"
        throttle_pwm = "01001011" #75
        break_pwm = "00000000" #0

        T&BMessage = T&BMessage + break_switch + break_pwm + throttle_switch + throttle_switch + throttle_pwm + empty + emergency_reset + mode_select



        print('e was written to serial')
    elif key.name == '1':
        SteeringMessage = SteeringMessage + "000001"
        ser.write(SteeringMessage)
        print('1 was written to serial')
    elif key.name == '2':
        SteeringMessage = SteeringMessage + "000001"
        ser.write(SteeringMessage)
        print('2 was written to serial')
    elif key.name == '3':
        SteeringMessage = SteeringMessage + "000001"
        ser.write(SteeringMessage)
        print('3 was written to serial')
    elif key.name == 'r':
        SteeringMessage = SteeringMessage + "000011"
        ser.write(SteeringMessage)

         #Throttle and Breaking
        mode_select = "1"
        emergency_reset = "1"
        throttle_switch = ""
        breaking_switch = ""
        empty = ""
        throttle_pwm = "" #No value
        break_pwm = "" #No value

        T&BMessage = T&BMessage + break_switch + break_pwm + throttle_switch + throttle_switch + throttle_pwm + empty + emergency_reset + mode_select

        print('r was written to serial')
    elif key.name == 'b':
        ser.write(b'b')


         #Throttle and Breaking
        mode_select = "1"
        emergency_reset = "1"
        throttle_switch = "1"
        breaking_switch = "1"
        empty = "000"
        throttle_pwm = "00000000" #0
        break_pwm = "11111111" #255

        T&BMessage = T&BMessage + break_switch + break_pwm + throttle_switch + throttle_switch + throttle_pwm + empty + emergency_reset + mode_select

        print('b was written to serial')


    print('')

def on_release(key):
    if key.name == 'w':
        ser.write(b'W')
        print('W released')
    elif key.name == 'a':
        ser.write(b'A')
        print('A released')
    elif key.name == 's':
        ser.write(b'S')
        print('S released')
    elif key.name == 'd':
        ser.write(b'D')
        print('D released')
    elif key.name == 'q':
        ser.write(b'Q')
        print('Q is released')
    elif key.name == 'e':
        ser.write(b'E')
        print('E is released')
    elif key.name == '1':
        ser.write(b'1')
        print('1 is released')
    elif key.name == '2':
        ser.write(b'2')
        print('2 is released')
    elif key.name == '3':
        ser.write(b'3')
        print('3 is released')
    elif key.name == 'r':
        ser.write(b'r')
        print('r is released')
    elif key.name == 'b':
        ser.write(b'b')
        print('b is released')



keyboard.on_press(on_press)
keyboard.on_release(on_release)

keyboard.wait()