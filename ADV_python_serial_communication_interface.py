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

ser.isOpen()

def on_press(key):
    if key.name == 'w':
        SteeringMessage = SteeringMessage + "00001"
        ser.write(SteeringMessage)
        print('w was written to serial')

    elif key.name == 'a':
        SteeringMessage = SteeringMessage + "5DC01" #Note, this does not compensate for the negative number yet
        #How to compensate for the negative number
        ser.write(SteeringMessage)

        print('a was written to serial')

    elif key.name == 's':
        SteeringMessage = SteeringMessage + "00001"
        ser.write(SteeringMessage)
        print('s was written to serial')

    elif key.name == 'd':
        SteeringMessage = SteeringMessage + "5DC01"
        ser.write(SteeringMessage)
        print('d was written to serial')

    elif key.name == 'q':
        SteeringMessage = SteeringMessage + "2EE01" #Does not compensate for the negative number
        ser.write(SteeringMessage)
        print('q was written to serial')
    elif key.name == 'e':
        SteeringMessage = SteeringMessage + "2EE01"
        ser.write(SteeringMessage)
        print('e was written to serial')
    elif key.name == '1':
        SteeringMessage = SteeringMessage + "00001"
        ser.write(SteeringMessage)
        print('1 was written to serial')
    elif key.name == '2':
        SteeringMessage = SteeringMessage + "00001"
        ser.write(SteeringMessage)
        print('2 was written to serial')
    elif key.name == '3':
        SteeringMessage = SteeringMessage + "00001"
        ser.write(SteeringMessage)
        print('3 was written to serial')
    elif key.name == 'r':
        SteeringMessage = SteeringMessage + "00011"
        ser.write(SteeringMessage)
        print('r was written to serial')
    elif key.name == 'b':
        ser.write(b'b')
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
