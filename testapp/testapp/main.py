
__author__=' Quebecois'

import sys
from PySide import QtCore, QtGui

from MainWindow import MainWindow

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
