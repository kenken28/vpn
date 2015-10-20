#!/usr/bin/python           # This is server.py file

import socket
import sys
import crypter
import mylib as m
from threading import Thread

Tkey = "two hashes walked into a bar, one was a salted"
print "NOTE - Enter key at SERVER side first before entering key at CLIENT side!"
print "     - SERVER and CLIENT need to be in the same LAN"
Tkey = raw_input("Enter your key: ")

while len(Tkey) < 8:
    print "Key needs to be at least 8 characters long"
    Tkey = raw_input("Enter your key: ")

HOST = socket.gethostname()
#HOST = raw_input("Enter Server IP address: ")

PORT = raw_input("Enter port number (leave it empty to set to default PORT number): ")
if PORT == "":
    PORT = m.PORT

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    s.bind((HOST, PORT))
    print "Server IP address and Port:", s.getsockname()
    print "\nListening for incoming connection requests..."
    s.listen(1)
except socket.error as msg:
    s.close()
    print 'could not open socket'
    sys.exit(1)
conn, addr = s.accept()
print 'Connected by', addr
MBtitle = "From CLIENT: "

nonceA = str( m.randI(2**256,2**257) )
sID = str( m.randI(2**64,2**65) )
tmpAES = crypter.AESc( m.getMD5(Tkey) )

############### Key establishment starts ###############
a = m.randI(2**2049,2**4096)
A = pow(m.P_ROOT, a, m.PRIME2048)

# send first message with Ra
m.mSend(conn, sID + m.sp + nonceA)

# revieve responce with Rb and B
responceB = m.mRecv(conn)
nonceB, hello2 = responceB.split(m.sp)
hello2raw = tmpAES.dec(hello2)
try:
    cID, nonceA2, strB = hello2raw.split(m.sp)
    if nonceA2 != nonceA:
        conn.close()
        print "Incorrect nonceA or Key!"
        sys.exit()
except:
    conn.close()
    print "Incorrect nonceA or Key!"
    sys.exit()

# send responce with A
responceA = tmpAES.enc(sID + m.sp + nonceB + m.sp + str(A))
m.mSend(conn, responceA)

# calculate session key
B = int(strB)
Ss = pow(B, a, m.PRIME2048)
#print Ss
############### Key establishment ends ###############
# 
myAES = crypter.AESc( m.getMD5(Ss) )

m.loop_send(conn, myAES, MBtitle)


