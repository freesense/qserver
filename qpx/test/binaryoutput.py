
# -*- coding: utf-8 -*-

#import os, sys

gSwitchHexdump = False

def isprint(ch):
    if ord(ch) >= 0x20 and ord(ch) <= 0x7E:
        return True
    else:
        return False

def hexdump(data, size = 0, text = None):
    if gSwitchHexdump == False:
        return

    if size == 0:
        size = len(data)

    s, t = ('', '')
    for m in range(size):
        idx = m % 16
        s += '%02X ' % (ord(data[m]), )
        if isprint(data[m]):
            t += data[m]
        else:
            t += '.'
        if idx == 15:
            s += ': '
            s += t
            s += '\n'
            t = ''
        elif m == size -1:
            s += '% *c' % ((16 - idx - 1) * 3, ' ')
            s += ': '
            s += t
            s += '\n'
            t = ''
    if text is None:
        print ' '.join(['Hexdump', str(size), 'bytes:'])
    else:
        print ' '.join([text, 'Hexdump', str(size), 'bytes:'])
    print s

if __name__ == '__main__':
    hexdump("I love you, baby!", 17)
