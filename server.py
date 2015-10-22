#!/usr/bin/python           # This is server.py file

import socket
import getpass
import sys
import crypter
import mylib as m

# Prumpt for key and port
print "NOTE - Enter key and PORT at SERVER side first before entering key at CLIENT side!"
print "     - SERVER and CLIENT need to be in the same LAN"
#Tkey = "two hashes walked into a bar, one was a salted"
Tkey = getpass.getpass("Enter your key: ")
while len(Tkey) < 8:
    print "Key needs to be at least 8 characters long"
    Tkey = getpass.getpass("Enter your key: ")

HOST = socket.gethostname()
#HOST = raw_input("Enter Server IP address: ")

PORT = raw_input("Enter port number (leave it empty to set to default PORT number 50007): ")
if PORT == "":
    PORT = m.PORT

# Socket connection
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
MBtitle = "CLIENT"

# Set up variables for Diffie-Hellman protocol
nonceA = str( m.randI(2**256,2**257) )
sID = str( m.randI(2**64,2**65) )
tmpAES = crypter.AESc( m.getMD5(Tkey) )
Tkey = "two hashes walked into a bar, one was a salted" # Erase key

#### Key Establishment and Mutual Authentication starts ####
a = m.randI(2**256,2**257)
A = pow(m.P_ROOT, a, m.PRIME2048)

# send ["I'm Alice", Ra]
m.mSend(conn, sID + m.sp + nonceA)

# revieve [Rb, E("Bob", Ra, gb mod p, Kab)], and varify Ra and Kab
responceB = m.mRecv(conn)
nonceB, hello2 = responceB.split(m.sp)
hello2raw = tmpAES.dec(hello2)
try:
    cID, nonceAecho, strB = hello2raw.split(m.sp)
    if nonceAecho != nonceA:
        conn.close()
        print "Incorrect nonceA or Key!"
        sys.exit()
except:
    conn.close()
    print "Incorrect nonceA or Key!"
    sys.exit()

# send [E("Alice", Rb, ga mod p, Kab)]
responceA = tmpAES.enc(sID + m.sp + nonceB + m.sp + str(A))
m.mSend(conn, responceA)

# calculate session key
B = int(strB)
Ss = pow(B, a, m.PRIME2048)
#print Ss
##### Key Establishment and Mutual Authentication ends #####

# start sending and receiving messages
myAES = crypter.AESc( m.getMD5(Ss) )
m.loop_send(conn, myAES, MBtitle)


