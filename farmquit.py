
#coding:utf-8

import socket, struct

buf = struct.pack('2BHLH15s', 48, 0, 0, 17, 65535, '')
s = socket.socket()
s.connect(('127.0.0.1', 11000))
s.send(buf)
s.close()
