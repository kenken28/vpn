#!/usr/bin/python           # This is server.py file

import socket
import sys
import mylib as m

HOST = socket.gethostname()
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.bind((HOST, m.PORT))
    s.listen(1)
except socket.error as msg:
    s.close()
    print 'could not open socket'
    sys.exit(1)
conn, addr = s.accept()
print 'Connected by', addr

myMsg = 'You are CLIENT'
m.mSend(conn, myMsg)                      # Test sending 
print repr(m.mRecv(conn))                 # Test receiveing

a = m.randI(2**2049,2**2050-1)
A = pow(m.P_ROOT, a, m.PRIME128)

m.mSend(conn, A)
B = int(m.mRecv(conn))

Ss = pow(B, a, m.PRIME)
print Ss
m.mSend(conn, Ss)                       # varify key, should not exist in real code




conn.close()
