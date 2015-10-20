#!/usr/bin/python

import socket
import sys
import hashlib
import crypter
import random

sp = "@spliter"
# PRIME is a 128bit prime with a primitive root 2 generated using pgen
PRIME128 = 277666483056069257952962430614829339539
# PRIME is a 2048bit prime with a primitive root 2 given in RFC 3526
PRIME2048 = int("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF",16)
P_ROOT = 2
PORT = 50007

def randI(upper, lower):
    return random.randint(upper, lower)

def getMD5(rawKey):
    keyMD5 = hashlib.md5()
    if type(rawKey) is str:
        strKey = rawKey
    else:
        strKey = str(rawKey)
    keyMD5.update( strKey )
    keyStr = keyMD5.digest()
    return keyStr

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

def loop_send(s, cipher, title):
    print "Enter message to send a message"
    print "Enter '@r' to recieve a message (if no message has been sent on the other side, process will wait for a message to be sent)"
    print "Enter '@e' to exit"
    while 1:
        myMsg = raw_input("\nTo send: ")
        if myMsg == "@r":
            do_recv(s, cipher, title)
        else:
            myMsgMD5 = getMD5(myMsg)
            encMsg = cipher.enc(myMsg + sp + myMsgMD5)
            mSend(s, encMsg)
            if myMsg == "@e":
                s.close()
                print "Closing chat session"
                sys.exit()
                break

def do_recv(s, cipher, title):
    decMsg = cipher.dec(mRecv(s))
    try:
        rawMsg, rawMsgMD5 = decMsg.split(sp)
        toPrint = title + rawMsg
        if getMD5(rawMsg) != rawMsgMD5:
            s.close()
            print "Incorrect message checksum!"
        elif rawMsg == "@e":
            s.close()
            print "Chat session has been closed"
            sys.exit()
        else:
            print toPrint
    except:
        s.close()
        print "Incorrect message checksum!"
    return
