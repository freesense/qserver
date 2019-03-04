
#coding: utf-8

import time, threading, atom, random, struct
from twisted.internet import reactor
from twisted.internet.protocol import Protocol, ClientFactory

fmt_head = '=2BHI'
num_client = 1        #连接数量
st1, st2 = 5, 10
max_request = 1     #最大请求数量

#服务器的ip和端口
#addr = '121.15.4.57'
addr = '127.0.0.1'
#addr = '192.168.11.25'
port = 9999

class CommxHead:
    fmt1 = '2B'
    dict_encode = {0:'gbk', 1:'utf_16_le', 2:'utf8'}
    def __init__(self, head8 = None):
        self.clear()
        if head8 is not None:
            self.setHead(head8)

    def setHead(self, head8):
        self.prop, self.feed = struct.unpack_from(self.fmt1, head8)
        if self.byteorder() == 1:
            self.pre_fmt = '!'
        self.fmt2 = self.pre_fmt + 'HL'
        self.serial, self.length = struct.unpack_from(self.fmt2, head8[2:])

    def clear(self):
        self.prop, self.feed, self.serial, self.length = None, None, None, None
        self.data = ''
        self.fmt2 = ''
        self.pre_fmt = '='

    def MakeProp(self, ver, encoding, byteorder):
        self.prop = ver << 4 | (encoding << 2) & 0x0e | byteorder & 0x01

    def ver(self):
        return self.prop & 0xf0

    def encoding(self):
        return self.dict_encode.get(self.prop & 0x0e, 'gbk')

    def byteorder(self):
        return self.prop & 0x01

    def dump(self):
        print self.prop, self.feed, self.serial, self.length
        print 'data =', self.data
        print self.fmt1, self.fmt2, self.pre_fmt

#连接次数，失败连接次数，请求次数，返回次数
class statistics:
    def __init__(self):
        self.num_con = atom.atom()              #连接次数
        self.num_conok = atom.atom()            #在线数量
        self.num_fail = atom.atom()             #连接失败次数
        self.num_disconnect = atom.atom()       #连接断开次数
        self.num_req = atom.atom()              #请求次数
        self.num_ret = atom.atom()              #应答次数

        self.sum_connect = atom.atom()          #连接时间之和
        self.max_connect = atom.atom()          #最大连接时间
        self.min_connect = atom.atom(999999)    #最小连接时间
        self.sum_business = atom.atom()         #业务时间之和
        self.max_business = atom.atom()         #最大业务时间
        self.min_business = atom.atom(999999)   #最小业务时间

        f = open('test.script')
        lns = f.readlines()
        self.scripts = []
        for script in lns:
            if script.find('#') == -1 and script[0] != '\n':    #没找到注释代码
                self.scripts.append(script)
        if len(self.scripts) == 0:
            raise Exception('测试脚本为空！')

    def avg_connect(self):
        num = int(self.num_con) - int(self.num_fail)
        if num == 0:
            return 0
        else:
            return self.sum_connect/num

    def avg_business(self):
        num = int(self.num_ret)
        if num == 0:
            return 0
        else:
            return self.sum_business/num

class testProtocol(Protocol):
    def __init__(self):
        self.con_status = False
        self.buffer = ''
        self.head = None
        self.timeobj = time.clock()

    def check(self):
        if self.head is not None:
            if len(self.buffer) >= self.head.length:
                self.head.data = self.buffer[0:self.head.length]
                self.buffer = self.buffer[self.head.length:]
                return self.head
            else:
                return None

        if len(self.buffer) >= 8:
            self.head = CommxHead(self.buffer[0:8])
            self.buffer = self.buffer[8:]
            return self.check()

    def connectionMade(self):
        tm = time.clock() - self.timeobj
        st.sum_connect += tm
        st.max_connect = max(tm, st.max_connect)
        st.min_connect = min(tm, st.min_connect)

        st.num_conok += 1
        self.con_status = True
        self.invokeCommand()

    def invokeCommand(self):
        if max_request != 0 and st.num_req >= max_request:
            return

        command = random.choice(st.scripts)
        self.OnCommand(command.split())

    def OnCommand(self, cmd):
        """解析测试命令"""
#        print cmd
        func = int(cmd[1], 16)
        if func == 0x0003:
            data = struct.pack('2H'+cmd[0], func, int(cmd[2]), func, int(cmd[2]), cmd[3])
        elif func == 0x0002:
            data = struct.pack('2H'+cmd[0], func, int(cmd[2]), func, int(cmd[2]), cmd[3], int(cmd[4]))
        elif func == 0x0004:
            data = struct.pack('2H'+cmd[0], func, int(cmd[2]), func, int(cmd[2]), cmd[3])
        elif func == 0x0008:
            data = struct.pack('2H'+cmd[0], func, int(cmd[2]), func, int(cmd[2]), cmd[3], int(cmd[4]), int(cmd[5]))
        elif func in [0x0005, 0x0006, 0x0007]:
            data = struct.pack('2H'+cmd[0], func, int(cmd[2]), func, int(cmd[2]), cmd[3], int(cmd[4]), int(cmd[5]))
        elif func in [0x0001, 0x07D4, 0x03F5]:
            data = struct.pack('2H'+cmd[0], func, 1, func)
        elif func in [0x03E9, 0x03EA, 0x03EC, 0x03F6, 0x03F7, 0x0516]:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2])
        elif func in [0x03ED, 0x03F1, 0x03F2]:
            data = struct.pack('2H'+cmd[0], func, int(cmd[2]), func, int(cmd[2]), cmd[3], cmd[4])
        elif func in [0x03EE, 0x03EF, 0x03F4, 0x03F0]:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2], cmd[3])
        elif func in [0x03F3, 0x0517]:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2], int(cmd[3]))
        elif func == 0x07D0:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2], cmd[3])
        elif func == 0x07D1:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2], cmd[3], cmd[4])
        elif func == 0x07D5:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2])
        elif func == 0x04B0:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2], cmd[3], cmd[4])
        elif func == 0x04B1:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2], cmd[3], cmd[4], cmd[5], int(cmd[6]), cmd[7])
        elif func == 0x0514:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2], cmd[3])
        elif func == 0x0515:
            data = struct.pack('2H'+cmd[0], func, 1, func, cmd[2], cmd[3], int(cmd[4]))
        elif func == 0x1772:#排行榜
            data = struct.pack('2H'+cmd[0], func, 1, func, int(cmd[2]), int(cmd[3]), int(cmd[4]))
            data += cmd[5]
        else:
            print '未知的测试命令:', cmd
            return

        if len(data) != 0:
            st.num_req += 1
            head = struct.pack(fmt_head, 48, 0, 0, len(data))
            self.timeobj = time.clock()
            self.transport.write(head)
            self.transport.write(data)

    def connectionLost(self, reason):
        print reason.getErrorMessage()
        if self.con_status == True:
            self.con_status = False
            st.num_conok -= 1

        st.num_con += 1
        reactor.connectTCP(addr, port, factory)

    def dataReceived(self, data):
#        print len(data), data
        self.buffer += data
        ch = self.check()
        if ch is None:
            return

        tm = time.clock() - self.timeobj
        st.min_business = min(st.min_business, tm)
        st.max_business = max(st.max_business, tm)

        st.sum_business += tm
        st.num_ret += 1

        self.buffer = ''
        self.head = None
        print 'OK.'
        self.timeobj = time.clock()
        reactor.callLater(random.uniform(st1, st2), self.invokeCommand)     #随机休息一段时间再发下一个请求包

class testFactory(ClientFactory):
    def __init__(self, protocol):
        self.protocol = protocol

class shown_thread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.char_num = 0
        self.seconds = 0

    def run(self):
        while st.num_req < max_request or max_request == 0:
            time.sleep(1)
            self.seconds += 1
            print '\b'.zfill(self.char_num+2).replace('0', '\b'),
            s = '连接[%d/%d/%d/%d, %.2f/%.2f/%.2f] 业务[%d/%d/%d, %.2f/%.2f/%.2f]' % (int(st.num_fail), int(st.num_disconnect), int(st.num_conok), int(st.num_con), float(st.min_connect), st.avg_connect(), float(st.max_connect), int(st.num_ret), int(st.num_req), int(int(st.num_ret)/self.seconds), float(st.min_business), st.avg_business(), float(st.max_business))
            self.char_num = len(s)
            print s,

if __name__ == '__main__':
    print '连接[失败/断开/在线/总连接次数, MIN/AVG/MAX] 业务[接收/发送/速度, MIN/AVG/MAX]'
    random.seed()
    st = statistics()
    trd = shown_thread()
    trd.setDaemon(True)
    trd.start()

    factory = testFactory(testProtocol)
    for i in xrange(num_client):
        st.num_con += 1
        reactor.connectTCP(addr, port, factory)
    reactor.run()
