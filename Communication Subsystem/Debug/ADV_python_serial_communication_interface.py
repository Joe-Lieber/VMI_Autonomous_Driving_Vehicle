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
TBMessage = ""

ser.isOpen()

def on_press(key):
    if key.name == 'w':

        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)

    elif key.name == 'a':
       #Steering
        Smodeselect = "1"
        Semergencyreset = "0"
        Sempty = ""
        Stargetheading = "1111101000100100" #-1500


        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading

        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01000110" #70
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        ser.write(SteeringMessage)

        print('a was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)

    elif key.name == 's':
        #Steering
        Smodeselect = "1"
        Semergencyreset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading

        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "0"
        breakingswitch = "0"
        empty = "000"
        throttlepwm = "00000000" #0
        breakpwm = "11111111" #255

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect
        ser.write(TBMessage)
        print('s was written to serial')

        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)

    elif key.name == 'd':
        #Steering
        Smodeselect = "1"
        Semergencyreset = "0"
        Sempty = ""
        Stargetheading = "0000010111011100" #1500



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01000110" #70
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect
        ser.write(T&BMessage)

        print('d was written to serial')

        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)

    elif key.name == 'q':
        #Steering
        Smodeselect = "1"
        Semergencyreset = "0"
        Sempty = ""
        Stargetheading = "1111110100010010" #-750



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01001011" #75
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        print('q was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
    elif key.name == 'e':
        #Steering
        Smodeselect = "1"
        Semergencyreset = "0"
        Sempty = ""
        Stargetheading = "0000001011101110"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading

        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01001011" #75
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect



        print('e was written to serial')

        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
    elif key.name == '1':
        #Steering
        Smodeselect = "1"
        Semergencyreset = "0"
        Sempty = ""
        Stargetheading = ""



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        print('1 was written to serial')
    elif key.name == '2':
        #Steering
        Smodeselect = "1"
        Semergencyreset = "0"
        Sempty = ""
        Stargetheading = ""



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        print('2 was written to serial')
    elif key.name == '3':
       #Steering
        Smode_select = "1"
        Semergencyreset = "0"
        Sempty = ""
        Stargetheading = ""



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        print('3 was written to serial')

        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
    elif key.name == 'r':
        #Steering
        Smodeselect = "1"
        Semergencyreset = "1"
        Sempty = ""
        Stargetheading = ""


        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading

         #Throttle and Breaking
        mode_select = "1"
        emergencyreset = "1"
        throttleswitch = ""
        breakingswitch = ""
        empty = ""
        throttlepwm = "" #No value
        breakpwm = "" #No value

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + mode_select

        print('r was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
    elif key.name == 'b':
        ser.write(b'b')


         #Throttle and Breaking
        mode_select = "1"
        emergencyreset = "1"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "00000000" #0
        breakpwm = "11111111" #255

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        print('b was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)


    print('')

def on_release(key):
    if key.name == 'w':


        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)

        #ser.write(b'W')
        print('W released')
    elif key.name == 'a':

        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'A')
        print('A released')
    elif key.name == 's':


        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'S')
        print('S released')
    elif key.name == 'd':
        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'D')
        print('D released')
    elif key.name == 'q':
        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'Q')
        print('Q is released')
    elif key.name == 'e':
        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'E')
        print('E is released')
    elif key.name == '1':
        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'1')
        print('1 is released')
    elif key.name == '2':
        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'2')
        print('2 is released')
    elif key.name == '3':
        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        ser.write(b'3')
        #print('3 is released')
    elif key.name == 'r':
        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'r')
        print('r is released')
    elif key.name == 'b':
        #Steering
        Smodeselect = "1"
        Semergency_reset = "0"
        Sempty = ""
        Stargetheading = "0000000000000000"



        SteeringMessage = SteeringMessage + Smodeselect + Semergencyreset + Sempty + Stargetheading
        ser.write(SteeringMessage)
        #Throttle and Breaking
        modeselect = "1"
        emergencyreset = "0"
        throttleswitch = "1"
        breakingswitch = "1"
        empty = "000"
        throttlepwm = "01010000" #80
        breakpwm = "00000000" #0

        TBMessage = TBMessage + breakswitch + breakpwm + throttleswitch + throttleswitch + throttlepwm + empty + emergencyreset + modeselect

        ser.write(TBMessage)
        print('w was written to serial')
        print("Steering")
        print(SteeringMessege)

        print("Throttle and Breaking")
        print(TBMessage)
        #ser.write(b'b')
        print('b is released')



keyboard.on_press(on_press)
keyboard.on_release(on_release)

keyboard.wait()