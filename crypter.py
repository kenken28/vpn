#!/usr/bin/python

import base64
from Crypto.Cipher import AES
from Crypto import Random

class AESc:
    def __init__(self, key):
        self.cipher = AES.new(key)

    def pad(self, target):
        genLen = AES.block_size - len(target) % AES.block_size
        result = target + (genLen) * chr(genLen)
        return result

    def unpad(self, target):
        result = ""
        if len(target) != 0:
            result = target[:-ord(target[len(target)-1:])]
        return result

    def enc(self, plain):
        padded = self.pad(plain)
        ENCed = self.cipher.encrypt(padded)
        result = base64.b64encode(ENCed)
        return result

    def dec(self, cipherT):
        raw = base64.b64decode(cipherT)
        DECed = self.cipher.decrypt(raw)
        result = self.unpad(DECed)
        return result
