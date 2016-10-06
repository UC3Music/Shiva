"""Testing experimental serial port threaded API"""

import serial
import serial.threaded
import time

class MyProtocol(serial.threaded.Protocol):
    def connection_made(self, transport):
        print("Start connection!")

    def data_received(self, data):
        print(str(data, 'UTF-8'))
        pass

    def connection_lost(self, exc):
        print("Connection lost!")



if __name__ == '__main__':
    ser = serial.Serial("/dev/ttyACM0", 9600)
    ser_thread = serial.threaded.ReaderThread(ser, MyProtocol)
    ser_thread.start()
    print("Never arrives here")
    ser.write(bytes("S0\n", "UTF-8"))
    time.sleep(2)
    ser_thread.stop()
    ser.close()
