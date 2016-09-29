"""
    Shiva
    -----------------------------------------------------
    Python module for communications with the Shiva board

"""

__author__ = 'David Estevez Fernandez'
__license__ = 'GPLv3'

import serial


class Shiva:
    resolution = 1024
    num_channels = 8
    drums = {"High Q (GM2)":27, "Slap (GM2)":28, "Scratch Push (GM2)":29, "Scratch Pull (GM2)":30, "Sticks (GM2)":31,
             "Square Click (GM2)":32, "Metronome Click (GM2)":33, "Metronome Bell (GM2)":34, "Bass Drum 2":35,
             "Bass Drum 1":36, "Side Stick":37, "Snare Drum 1":38, "Hand Clap":39, "Snare Drum 2":40, "Low Tom 2":41,
             "Closed Hi-hat":42, "Low Tom 1":43, "Pedal Hi-hat":44, "Mid Tom 2":45, "Open Hi-hat":46, "Mid Tom 1":47,
             "High Tom 2":48, "Crash Cymbal 1":49, "High Tom 1":50, "Ride Cymbal 1":51, "Chinese Cymbal":52, "Ride Bell":53,
             "Tambourine":54, "Splash Cymbal":55, "Cowbell":56, "Crash Cymbal 2":57, "Vibra Slap":58, "Ride Cymbal 2":59,
             "High Bongo":60, "Low Bongo":61, "Mute High Conga":62, "Open High Conga":63, "Low Conga":64, "High Timbale":65,
             "Low Timbale":66, "High Agogo":67, "Low Agogo":68, "Cabasa":69, "Maracas":70, "Short Whistle":71,
             "Long Whistle":72, "Short Guiro":73, "Long Guiro":74, "Claves":75, "High Wood Block":76, "Low Wood Block":77,
             "Mute Cuica":78, "Open Cuica":79, "Mute Triangle":80, "Open Triangle":81, "Shaker (GM2)":82, "Jingle Bell (GM2)":83,
             "Belltree (GM2)":84, "Castanets (GM2)":85, "Mute Surdo (GM2)":86, "Open Surdo (GM2)":87}

    class ShivaNotConnectedException(BaseException):
        pass

    def __init__(self):
        self.serialPort = None


    def connect(self, name, baudRate):
        """
            Creates a serial connection with the specified parameters
        """

        # Create a serial object
        self.serialPort = serial.Serial()

        # Set the main parameters
        self.serialPort.port = name
        self.serialPort.baudrate = baudRate

        # Open port
        self.serialPort.open()

    def close(self):
        self.serialPort.close()

    def sendCommand(self, command):
        """
        Sends a command over serial port
        :param command: Command to send (newline is added by this function)
        """
        try:
            self.serialPort.write(bytes(command+'\n', 'UTF-8'))
        except AttributeError:
            raise Shiva.ShivaNotConnectedException

    def enableFeedback(self):
        """
            Enable channel feedback (Command S0)
        """
        self.sendCommand("S0")

    def disableFeedback(self):
        """
            Disable channel feedback (Command S1)
        """
        self.sendCommand("S1")

    def setSound(self, channel=-1, sound="Crash Cymbal 1"):
        """
        Sets the value of the midi note sent by each channel (command S2)
        :param channel: Channel to modify (-1 means all channels)
        :param sound: Sound to set, 'drums' contains all the allowed sounds and their corresponding code
        """
        if isinstance(sound, int):
            sound_code = sound
        else:
            sound_code = self.drums[sound]

        if channel == -1:
            for i in range(self.num_channels):
                self.sendCommand("S2C{}V{}".format(i, sound_code))
        else:
            self.sendCommand("S2C{}V{}".format(channel, sound_code))

    def setThresholds(self, channel=-1, trigger=None, off=None):
        """
        Sets the value of the thresholds (Command S3 and S4)
        :param channel: Channel to modify (-1 means all channels)
        :param trigger: Value for trigger threshold
        :param off: Value for off threshold
        """
        if trigger is not None:
            if channel == -1:
                for i in range(self.num_channels):
                    self.sendCommand("S3C{}V{}".format(i, trigger))
            else:
                self.sendCommand("S3C{}V{}".format(channel, trigger))

        if off is not None:
            if channel == -1:
                for i in range(self.num_channels):
                    self.sendCommand("S4C{}V{}".format(i, off))
            else:
                self.sendCommand("S4C{}V{}".format(channel, off))




    def _readPort(self):
        """
            Reads a string from the serial port
        """

        string = ''

        while self.serialPort.inWaiting() > 0:
            string += self.serialPort.read(1)

        return string
