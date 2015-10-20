#!/usr/bin/python           # This is client.py file

import socket
import getpass
import sys
import crypter
import mylib as m
from threading import Thread

# Prumpt for key and port
print "NOTE - Enter key and PORT at SERVER side first before entering key at CLIENT side!"
print "     - SERVER and CLIENT need to be in the same LAN"
#Tkey = "two hashes walked into a bar, one was a salted"
Tkey = getpass.getpass("Enter your key: ")
while len(Tkey) < 8:
    print "Key needs to be at least 8 characters long"
    Tkey = getpass.getpass("Enter your key: ")

HOST = socket.gethostname()
HOST = raw_input("Enter Server IP address: ")

PORT = raw_input("Enter port number (leave it empty to set to default PORT number 50007): ")
if PORT == "":
    PORT = m.PORT

# Socket connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print "Connecting to SERVER..."
try:
    s.connect((HOST, PORT))
except socket.error as msg:
    s.close()
    s = None
    print 'Could not open socket.'
    sys.exit(1)
print "Connected to", s.getsockname()
MBtitle = "SERVER"

# Set up variables for Diffie-Hellman protocol
nonceB = str( m.randI(2**256,2**257) )
cID = str( m.randI(2**64,2**65) )
tmpAES = crypter.AESc( m.getMD5(Tkey) )
Tkey = "two hashes walked into a bar, one was a salted" # Erase key

#### Key Establishment and Mutual Authentication starts ####
b = m.randI(2**256,2**257)
B = pow(m.P_ROOT, b, m.PRIME2048)

# recieve ["I'm Alice", Ra]
hello1 = m.mRecv(s)
sID, nonceA = hello1.split(m.sp)

# send [Rb, E("Bob", Ra, gb mod p, Kab)]
hello2 = tmpAES.enc(cID + m.sp + nonceA + m.sp + str(B))
m.mSend(s, nonceB + m.sp + hello2)

# revieve [E("Alice", Rb, ga mod p, Kab)], and varify Rb and Kab
responceA = m.mRecv(s)
hello3raw = tmpAES.dec(responceA)
try:
    sID2, nonceBecho, strA = hello3raw.split(m.sp)
    if nonceBecho != nonceB or sID != sID2:
        s.close()
        print "Incorrect nonceB or sID or Key!"
        sys.exit()
except:
    s.close()
    print "Incorrect nonceB or Key!"
    sys.exit()

# calculate session key
A = int(strA)
Cs = pow(A, b, m.PRIME2048)
#print Cs
##### Key Establishment and Mutual Authentication ends #####

# start sending and receiving messages
myAES = crypter.AESc( m.getMD5(Cs) )
m.loop_send(s, myAES, MBtitle)

