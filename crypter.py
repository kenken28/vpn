#!/usr/bin/python

import base64
from Crypto.Cipher import AES
from Crypto import Random

def pad(target):
    genLen = AES.block_size - len(target) % AES.block_size
    if genLen != 0:
        result = target + (genLen) * chr(genLen)
    return result

def unpad(target):
    result = target[:-ord(target[len(target)-1:])]
    return

class AESCipher:
    def __init__(self, key):
        self.key = key

    def encrypt(self, plain):
        print AES.block_size
        raw = pad(plain)
        prefix = Random.new().read(AES.block_size)
        result = AES.new(self.key, AES.MODE_CBC, prefix)
        return base64.b64encode( prefix + result.encrypt(plain) ) 

    def decrypt(self, cipher):
        cipher = base64.b64decode(cipher)
        prefix = cipher[:16]
        result = AES.new(self.key, AES.MODE_CBC, prefix)
        return unpad( result.decrypt(cipher[16:]) )
