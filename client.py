#!/usr/bin/python           # This is client.py file

import socket               # Import socket module
import sys
import mylib as m

    
#HOST = raw_input("Enter Server IP address: ")
HOST = socket.gethostname()
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    s.connect((HOST, m.PORT))
except socket.error as msg:
    s.close()
    s = None
    print 'could not open socket'
    sys.exit(1)

myMsg = 'You are SERVER'
print repr(m.mRecv(s))                 # Test receiveing
m.mSend(s, myMsg)                      # Test sending

b = m.randI(2**2049,2**2050-1)
B = pow(m.P_ROOT, b, m.PRIME128)

A = int(m.mRecv(s))
m.mSend(s, B)

Cs = pow(A, b, m.PRIME)
print Cs
Ss = int(m.mRecv(s))                # varify key, should not exist in real code
if Ss == Cs:
    print "key varified!"



s.close()

