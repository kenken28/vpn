#!/usr/bin/python           # This is server.py file

import socket
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

HOST = socket.gethostname()
PORT = 50021
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.bind((HOST, PORT))
    s.listen(1)
except socket.error as msg:
    s.close()
    print 'could not open socket'
    sys.exit(1)
conn, addr = s.accept()
print 'Connected by', addr

myMsg = 'You are CLIENT'
mSend(conn, myMsg)                      # Test sending 
print repr(mRecv(conn))                 # Test receiveing

a = r.randint(2**256,2**257)
A = pow(P_ROOT, a, PRIME)

mSend(conn, A)
B = int(mRecv(conn))

Ss = pow(B, a, PRIME)
print Ss
mSend(conn, Ss)

conn.close()
