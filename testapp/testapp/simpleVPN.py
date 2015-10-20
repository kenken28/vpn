# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'D:\workspace\pyDevTest\src\app\view\simpleVPN.ui'
#
# Created: Sun Oct 18 11:02:20 2015
#      by: pyside-uic 0.2.15 running on PySide 1.2.4
#
# WARNING! All changes made in this file will be lost!

from PySide import QtCore, QtGui

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(729, 600)
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.ipTextEdit = QtGui.QTextEdit(self.centralwidget)
        self.ipTextEdit.setGeometry(QtCore.QRect(130, 20, 171, 31))
        self.ipTextEdit.setObjectName("ipTextEdit")
        self.portTextEdit = QtGui.QTextEdit(self.centralwidget)
        self.portTextEdit.setGeometry(QtCore.QRect(130, 80, 171, 31))
        self.portTextEdit.setObjectName("portTextEdit")
        self.ipLabel = QtGui.QLabel(self.centralwidget)
        self.ipLabel.setGeometry(QtCore.QRect(40, 30, 81, 16))
        self.ipLabel.setObjectName("ipLabel")
        self.portLabel = QtGui.QLabel(self.centralwidget)
        self.portLabel.setGeometry(QtCore.QRect(40, 80, 54, 21))
        font = QtGui.QFont()
        font.setFamily("Arial")
        font.setPointSize(10)
        self.portLabel.setFont(font)
        self.portLabel.setObjectName("portLabel")
        self.receivedTextEdit = QtGui.QTextEdit(self.centralwidget)
        self.receivedTextEdit.setGeometry(QtCore.QRect(120, 280, 241, 91))
        self.receivedTextEdit.setObjectName("receivedTextEdit")
        self.textEdit_2 = QtGui.QTextEdit(self.centralwidget)
        self.textEdit_2.setGeometry(QtCore.QRect(120, 400, 241, 31))
        self.textEdit_2.setObjectName("textEdit_2")
        self.sendLabel = QtGui.QLabel(self.centralwidget)
        self.sendLabel.setGeometry(QtCore.QRect(23, 180, 91, 20))
        self.sendLabel.setObjectName("sendLabel")
        self.receivedLabel = QtGui.QLabel(self.centralwidget)
        self.receivedLabel.setGeometry(QtCore.QRect(13, 310, 101, 31))
        self.receivedLabel.setObjectName("receivedLabel")
        self.keyLabel = QtGui.QLabel(self.centralwidget)
        self.keyLabel.setGeometry(QtCore.QRect(50, 410, 54, 12))
        self.keyLabel.setObjectName("keyLabel")
        self.connectPushButton = QtGui.QPushButton(self.centralwidget)
        self.connectPushButton.setGeometry(QtCore.QRect(120, 490, 75, 23))
        self.connectPushButton.setObjectName("connectPushButton")
        self.sendPushButton = QtGui.QPushButton(self.centralwidget)
        self.sendPushButton.setGeometry(QtCore.QRect(290, 490, 75, 23))
        self.sendPushButton.setObjectName("sendPushButton")
        self.serverRadioButton = QtGui.QRadioButton(self.centralwidget)
        self.serverRadioButton.setGeometry(QtCore.QRect(340, 30, 89, 16))
        self.serverRadioButton.setObjectName("serverRadioButton")
        self.clientRadioButton = QtGui.QRadioButton(self.centralwidget)
        self.clientRadioButton.setGeometry(QtCore.QRect(340, 80, 89, 16))
        self.clientRadioButton.setObjectName("clientRadioButton")
        self.plainTextEdit = QtGui.QPlainTextEdit(self.centralwidget)
        self.plainTextEdit.setGeometry(QtCore.QRect(450, 20, 261, 411))
        self.plainTextEdit.setObjectName("plainTextEdit")
        self.sendTextEdit = QtGui.QTextEdit(self.centralwidget)
        self.sendTextEdit.setGeometry(QtCore.QRect(120, 160, 241, 101))
        self.sendTextEdit.setObjectName("sendTextEdit")
        MainWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtGui.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QtGui.QApplication.translate("MainWindow", "SimpleVPN", None, QtGui.QApplication.UnicodeUTF8))
        self.ipLabel.setText(QtGui.QApplication.translate("MainWindow", "<html><head/><body><p>IP Address:</p></body></html>", None, QtGui.QApplication.UnicodeUTF8))
        self.portLabel.setText(QtGui.QApplication.translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'SimSun\'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Port:</p></body></html>", None, QtGui.QApplication.UnicodeUTF8))
        self.sendLabel.setText(QtGui.QApplication.translate("MainWindow", "Data to be Sent", None, QtGui.QApplication.UnicodeUTF8))
        self.receivedLabel.setText(QtGui.QApplication.translate("MainWindow", "Data as Received", None, QtGui.QApplication.UnicodeUTF8))
        self.keyLabel.setText(QtGui.QApplication.translate("MainWindow", "Key", None, QtGui.QApplication.UnicodeUTF8))
        self.connectPushButton.setText(QtGui.QApplication.translate("MainWindow", "Connect", None, QtGui.QApplication.UnicodeUTF8))
        self.sendPushButton.setText(QtGui.QApplication.translate("MainWindow", "Send", None, QtGui.QApplication.UnicodeUTF8))
        self.serverRadioButton.setText(QtGui.QApplication.translate("MainWindow", "Server Mode", None, QtGui.QApplication.UnicodeUTF8))
        self.clientRadioButton.setText(QtGui.QApplication.translate("MainWindow", "Client Mode", None, QtGui.QApplication.UnicodeUTF8))

