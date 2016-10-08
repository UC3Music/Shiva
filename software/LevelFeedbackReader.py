"""
    LevelFeedbackReader
    -----------------------------------------------------
    Class to read the information sent by Shiva in verbose mode (S0)
    Listeners can be added, and they will be notified when new information arrives
"""

import serial
import serial.threaded
import time

class LevelFeedbackReaderListener(object):
    """
    Objects of this class are notified of incoming data when added to this class
    """
    def notify(self, command, channel, value):
        raise NotImplementedError

class LevelFeedbackReader(serial.threaded.LineReader):
    """
    Reads the output of the command S0 (level feedback) using a dedicated thread
    """

    def __init__(self):
        super().__init__()
        self.listeners = []

    def connection_made(self, transport):
        """
        Callback called when the LineReader is started. Sends to Shiva the command
        to enter verbose mode (S0)
        """
        try:
            transport.write(bytes("S0\n", "UTF-8"))
        except Exception as e:
            print(e)

    def handle_line(self, data):
        """
        Callback called when a new line arrives from Shiva. When the command received
        is a feedback command, it notifies the listeners.
        """
        command, channel, value = self.parse_response(data)
        if command is not None:
            for listener in self.listeners:
                listener.notify(command, channel, value)

    def connection_lost(self, exc):
        print("Connection lost!", exc)

    @staticmethod
    def parse_response(response):
        """
        Parses a received line to extract the command that sent the information, and the
        channel and value of the sender command.
        :return: Three items: command id, channel id and value (all None if the data was invalid)
        """
        print(response)
        s = response.find('S')
        c = response[s+1:].find('C') + s+1
        v = response[c+1:].find('V') + c+1

        try:
            return int(response[s+1:c]), int(response[c+1:v]), int(response[v+1:])
        except ValueError:
            return None, None, None

    def add_listener(self, listener):
        if isinstance(listener, LevelFeedbackReaderListener):
            self.listeners.append(listener)

    def __call__(self, *args, **kwargs):
        """
        With this method, the LevelFeedbackReader becomes a protocol_factory, required
        to create a ReaderThread.
        :return: A reference to this object
        """
        return self

class BasicLevelFeedbackReaderListener(LevelFeedbackReaderListener):
    """
    Very simple listener that dumps received info to stdout
    """
    def notify(self, command, channel, value):
        print("{} {} {}".format(command, channel, value))

if __name__ == '__main__':
    ser = serial.Serial("/dev/ttyACM0", 9600)
    l = BasicLevelFeedbackReaderListener()
    line_reader = LevelFeedbackReader()
    line_reader.add_listener(l)

    ser_thread = serial.threaded.ReaderThread(ser, line_reader)
    ser_thread.start()
    time.sleep(5)
    ser_thread.stop()
    ser.close()
