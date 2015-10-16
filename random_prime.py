#!/usr/bin/python

import random

def randP():
    while True:
        p = random.randrange(10000001, 99999999, 2)
        if all(p % n != 0 for n in range(3, int((p ** 0.5) + 1), 2)):
            return p

def randI(upper, lower):
    return random.randint(upper, lower)
