#!/usr/bin/python

import socket
import sys
import hashlib
import crypter
import random

# A string for separating messages when appending messages
sp = "@spliter"
# PRIME2048 is a 2048bit prime with a primitive root 2 given in RFC 3526
PRIME2048 = int("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF",16)
# P_ROOT is the primitive root for both PRIME2048 and PRIME128
P_ROOT = 2
PORT = 50007

def randI(upper, lower):
    return random.randint(upper, lower)

# Pass it a string of any size, it returns the checksum of that string
def getMD5(rawKey):
    keyMD5 = hashlib.md5()
    if type(rawKey) is str:
        strKey = rawKey
    else:
        strKey = str(rawKey)
    keyMD5.update( strKey )
    keyStr = keyMD5.digest()
    return keyStr

# Pass it a socket and a message, it sends the message via the given socket
def mSend(s, m):
    if type(m) is not str:
        m = str(m)
    if len(m)%1024 == 0:
        m += '@end'
    s.sendall(m)

# Pass it a socket, it receives a message via the given socket and returns it
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

# Pass it a socket, AESc object and a string (needed for do_recv()),
# it will perform an infinite loop which:
# prumpt for message (has to end with "s") to be sent, append
# the message with its checksum and encrypt it via the AESc object,
# then send it via the socket.
# If the message is "@r", do_recv() will be called to receive a 
# message.
# If the message is "@e", close the socket to end chat session.
def loop_send(s, cipher, title):
    print "\n- Enter message followed with '@s' to send a message (Only ASCII characters are supported, including the newline character. '@s' must be at the end to end message input)"
    print "- Enter '@r' to recieve a message (if no message has been sent on the other side, process will wait for a message to be sent)"
    print "- Enter '@e' to exit"
    while 1:
        myMsg = ""
        tmpLine = ""
        print "\n====================================================\nTo send: "
        while myMsg != "@r\n" and myMsg != "@e\n" and myMsg[-3:] != "@s\n":
            tmpLine = raw_input('''''')
            myMsg += tmpLine + '\n'
            #print myMsg[-2:]
        
        if myMsg == "@r\n":
            if do_recv(s, cipher, title) == 1:
                s.close()
                print title + " has closed this chat session"
                sys.exit()
                break
        else:
            if myMsg != "@e\n":
                myMsg = myMsg[:-3]
            myMsgMD5 = getMD5(myMsg)
            encMsg = cipher.enc(myMsg + sp + myMsgMD5)
            mSend(s, encMsg)
            if myMsg == "@e\n":
                s.close()
                print "Closing chat session"
                sys.exit()
                break

# Pass it a socket, AESc object and a string, it will:
# receive a message (encrypted) via the socket, decrypt the
# message via the AES object and checks the checksum for
# message integrety.
# If the message passes the check, it will be displayed,
# else, the socket will be close to end the chat session
# to prevent further attacks
def do_recv(s, cipher, title):
    decMsg = cipher.dec(mRecv(s))
    try:
        rawMsg, rawMsgMD5 = decMsg.split(sp)
        toPrint = "=================== Form " + title + ": =================== \n" + rawMsg
        if getMD5(rawMsg) != rawMsgMD5:
            s.close()
            print "Incorrect message checksum!"
        elif rawMsg == "@e\n":
            return 1
        else:
            print toPrint
    except:
        s.close()
        print "Incorrect message checksum!"
    return
