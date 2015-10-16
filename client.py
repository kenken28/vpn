#!/usr/bin/python           # This is client.py file

import socket               # Import socket module
import sys
import random as r

PRIME = 277666483056069257952962430614829339539
P_ROOT = 2

def mSend(s, m):
    if type(m) is not str:
        m = str(m)
    if len(m)%1024 == 0:
        m += '@end'
    s.sendall(m)

def mRecv(s):
    whole = ''
    while 1:
        chunk = s.recv(1024)
        if len(chunk)<1024:
            if chunk != '@end':
                whole += chunk
            break
        whole += chunk
    return whole
    
#HOST = raw_input("Enter Server IP address: ")
HOST = socket.gethostname()
PORT = 50021
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    s.connect((HOST, PORT))
except socket.error as msg:
    s.close()
    s = None
    print 'could not open socket'
    sys.exit(1)

myMsg = 'You are SERVER'
print repr(mRecv(s))                 # Test receiveing
mSend(s, myMsg)                      # Test sending

b = r.randint(2**256,2**257)
B = pow(P_ROOT, b, PRIME)

A = int(mRecv(s))
mSend(s, B)

Cs = pow(A, b, PRIME)
print Cs
Ss = int(mRecv(s))
if Ss == Cs:
    print "yeah!"

s.close()

