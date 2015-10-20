
__author__='Quebecois'

from PySide import QtCore, QtGui
from simpleVPN import Ui_MainWindow

class MainWindow(QtGui.QMainWindow):
  
    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        