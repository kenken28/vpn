#!/usr/bin/python

import socket
import random

# PRIME is a 128bit prime with a primitive root 2 generated using pgen
PRIME128 = 277666483056069257952962430614829339539
# PRIME is a 2048bit prime with a primitive root 2 given in RFC 3526,
# but it does not give 2 identical key via Diffie Hellman
PRIME2048 = int("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF",16)
P_ROOT = 2
PORT = 50024

def randI(upper, lower):
    return random.randint(upper, lower)

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
