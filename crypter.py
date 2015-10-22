#!/usr/bin/python

import base64
from Crypto.Cipher import AES

class AESc:
    # Initialize AEScipher by passing a 128bit key
    def __init__(self, key):
        self.cipher = AES.new(key)

    # Pass plain message as parameter, it will be padsed
    # so that the size of the returned message(padded)
    # is divisible by block_size
    def pad(self, target):
        genLen = AES.block_size - len(target) % AES.block_size
        result = target + (genLen) * chr(genLen)
        return result

    # Pass padded plain message as parameter, it will unpad
    # the message and return the original plain text
    def unpad(self, target):
        result = ""
        if len(target) != 0:
            result = target[:-ord(target[len(target)-1:])]
        return result

    # Pass plain message as parameter, it will be padsed
    # and will be encrypted using the stored key and return
    # the cipher text
    def enc(self, plain):
        padded = self.pad(plain)
        ENCed = self.cipher.encrypt(padded)
        result = base64.b64encode(ENCed)
        return result

    # Pass cipher text as parameter, it will be decrypted
    # using the stored key and then unpad it and return
    # the plain text
    def dec(self, cipherT):
        raw = base64.b64decode(cipherT)
        DECed = self.cipher.decrypt(raw)
        result = self.unpad(DECed)
        return result
