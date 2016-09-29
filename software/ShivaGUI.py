from PySide import QtCore,QtGui
from PySide import QtUiTools
import os, sys
import glob
from Shiva import Shiva

def load_ui(file_name, where=None):
    """
    Loads a .UI file into the corresponding Qt Python object
    :param file_name: UI file path
    :param where: Use this parameter to load the UI into an existing class (i.e. to override methods)
    :return: loaded UI
    """
    # Create a QtLoader
    loader = QtUiTools.QUiLoader()

    # Open the UI file
    ui_file = QtCore.QFile(file_name)
    ui_file.open(QtCore.QFile.ReadOnly)

    # Load the contents of the file
    ui = loader.load(ui_file, where)

    # Close the file
    ui_file.close()

    return ui

class ShivaGUI(QtGui.QWidget):
    default_baudrate = 9600

    def __init__(self, parent=None, shiva=None):
        QtGui.QWidget.__init__(self, parent)

        # Interface to board is required
        self.shiva = shiva

        # Widgets to be created
        self.comboBox = None
        self.lineEdit = None
        self.portButton = None
        self.connectButton = None

        self.setupUI()
        self.resetValues()

    def setupUI(self):
        ui_file_path = os.path.join(os.path.realpath(os.path.dirname(__file__)), 'Shiva.ui')
        main_widget = load_ui(ui_file_path, self)
        layout = QtGui.QVBoxLayout()
        layout.addWidget(main_widget)
        self.setLayout(layout)

        # Get a reference to all required widgets
        self.comboBox = self.findChild(QtGui.QComboBox, 'comboBox')
        self.lineEdit = self.findChild(QtGui.QLineEdit, 'lineEdit')
        self.portButton = self.findChild(QtGui.QPushButton, 'portButton')
        self.connectButton = self.findChild(QtGui.QPushButton, 'connectButton')

        # Connect widgets to actions
        self.portButton.clicked.connect(self.resetValues)
        self.connectButton.clicked.connect(self.onConnectClicked)


    def resetValues(self):
        self.lineEdit.setText(str(self.default_baudrate))
        self.comboBox.clear()
        self.comboBox.addItems(self.getSerialPorts())

    def onConnectClicked(self):
        if self.connectButton.text() == 'Connect':
            baudrate = int(self.lineEdit.text())
            port = self.comboBox.currentText()
            self.shiva.connect(port, baudrate)
            self.connectButton.setText('Disconnect')
        elif self.connectButton.text() == 'Disconnect':
            self.shiva.close()
            self.connectButton.setText('Connect')

    def getSerialPorts(self):
        """
			Returns the available serial ports
		"""
        return glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*') + glob.glob("/dev/tty.*") + glob.glob("/dev/cu.*") + glob.glob("/dev/rfcomm*")



if __name__ == '__main__':

    # Create Qt app
    app = QtGui.QApplication(sys.argv)

    # Create a interface to Shiva board
    shiva = Shiva()

    # Create the widget and show it
    gui = ShivaGUI(shiva=shiva)
    gui.show()

    # Run the app
    sys.exit(app.exec_())