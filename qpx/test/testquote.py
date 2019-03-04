
# -*- coding: utf-8 -*-

import string, random, time, winsound, asyncore, time, itquote, binaryoutput, packetq

AutoCommand = ['re 4097 000001', 'k 61 4100 600012 100', 's 2053 4116 100 0', 'st 000001 4116', 'newre 4116 000001' , 'overlapre 4116 000001']
#AutoCommand = ['re 4097 000001']
TimeOut = 30

def listmarket():
    for key in itquote.marketdict.keys():
        print str(key), itquote.marketdict[key]

def listsort():
    for key in itquote.sortdict.keys():
        print str(key), itquote.sortdict[key]

def OnCommand(command):
    if len(command) == 0:
        packetq.helpcommand()
        return 0

    if command[0] == 'quit':
        return -1
    elif command[0] == 'sw' and command[1] == 'hexdump':
        binaryoutput.gSwitchHexdump = not binaryoutput.gSwitchHexdump
    elif command[0] == 'lm':
        listmarket()
    elif command[0] == 'ls':
        listsort()
    else:
        buf = packetq.make_send_buffer(command)
        if len(buf):
            itrd.send(buf)
    return 0

if __name__ == '__main__':
#    HOST = "221.179.6.205"
    HOST = "127.0.0.1"
    PORT = 8001

    print u'请输入主机地址，否则使用默认地址['+HOST+']:',
    ihost = raw_input()
    if ihost == '':
        ihost = HOST
    print u'请输入主机端口，否则使用默认端口['+str(PORT)+']:',
    iport = raw_input()
    if iport == '':
        iport = PORT
    else:
        iport = int(iport)

    print u'使用人工输入(默认)，自动测试(a)还是压力测试模式(p)？:',
    iMode = raw_input()
    random.seed()
    if iMode == 'p':#压力测试模式
        print u'压力连接数:',
        num_pressure = raw_input()
        for x in xrange(int(num_pressure)):
            xc = itquote.itquote_trd('itquote protocol parser', (ihost, iport), AutoCommand)
            xc.setDaemon(True)
            xc.start()
        while raw_input('') != 'q':
            continue
    elif iMode == 'a':#自动测试模式
        Flag = 0
        timeb = 0
        timee = 0
        itrd = itquote.itquote_trd('itquote protocol parser', (ihost, iport))
        itrd.setDaemon(True)
        itrd.start()

        while Flag == 0:
            if itrd.bAnswer == False:
                timee = time.clock()
                if timee - timeb >= TimeOut:
                    winsound.PlaySound('warning.wav', winsound.SND_FILENAME)
                time.sleep(0.1)
                Flag = 0
            else:
                timee = time.clock()
                print u'业务时间：', str(timee - timeb)
                winsound.PlaySound(None, winsound.SND_PURGE)
                command = random.choice(AutoCommand)
                print command
                itrd.bAnswer = False
                timeb = time.clock()
                Flag = OnCommand(command.split())
                time.sleep(1)
    else:#人工输入模式
        Flag = 0
        itrd = itquote.itquote_trd('itquote protocol parser', (ihost, iport))
        while Flag == 0:
            command = raw_input().split()
            Flag = OnCommand(command)
