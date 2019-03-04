
# -*- coding: utf-8 -*-
# python pressure.py 1 192.168.20.16 8008
# python pressure.py 1 127.0.0.1 8001

import asyncore, asynchat, socket, struct, sys, time, atom, binaryoutput, random, string, threading

binaryoutput.gSwitchHexdump = True

#连接次数，失败连接次数，请求次数，返回次数
class statistics:
    def __init__(self):
        self.num_con = atom.atom()
        self.num_fail = atom.atom()
        self.num_req = atom.atom()
        self.num_ret = atom.atom()

        self.sum_connect = atom.atom()   #连接时间之和
        self.max_connect = atom.atom()
        self.min_connect = atom.atom(999999)
        self.sum_business = atom.atom()  #业务时间之和
        self.max_business = atom.atom()
        self.min_business = atom.atom(999999)

        f = open('pressure.txt')
        lns = f.readlines()
        self.scripts = []
        for script in lns:
            if script.find('#') == -1:
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

class client(asynchat.async_chat):
    def __init__(self, remote_address):
        asynchat.async_chat.__init__(self)
        self.tm_connect = 0
        self.tm_business = 0
        self.base_time = 0
        self.remote_address = remote_address
        self.connect_svr()

    #建立与行情服务器之间的连接
    def connect_svr(self):
        self.read_buffer = ''
        self.recv_flag = 0
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)
        self.connect(self.remote_address)
        st.num_con += 1
        self.set_terminator('\r\n\r\n')
        self.base_time = time.clock()

    #当连接断开时被系统回调
    def handle_close(self):
        self.close()
        self.connect_svr()

    #当连接异常时被系统回调
    def handle_expt(self):
        self.close()
        print os.errno
        st.num_fail += 1
        self.connect_svr()

    def handle_connect(self):
        tm = time.clock() - self.base_time
        st.sum_connect += tm
        st.max_connect = max(st.max_connect, tm)
        st.min_connect = min(st.min_connect, tm)

        command = random.choice(st.scripts)
        self.OnCommand(command.split())

    def collect_incoming_data(self, data):
        self.read_buffer += data

    def found_terminator(self):
        if self.recv_flag == 0:     #http包头
            self.read_buffer = ''
            self.recv_flag = 1
            self.set_terminator(8)
        elif self.recv_flag == 1:   #8字节包头
            size, unuse = struct.unpack_from("!ii", self.read_buffer)
            self.recv_flag = 2
            self.set_terminator(size)
        elif self.recv_flag == 2:   #接收应答包完整
            tm = time.clock() - self.base_time
            st.sum_business += tm
            st.max_business = max(st.max_business, tm)
            st.min_business = min(st.min_business, tm)

            st.num_ret += 1
            self.read_buffer = ''
            self.recv_flag = 0
            self.set_terminator('\r\n\r\n')
            command = random.choice(st.scripts)
            self.OnCommand(command.split())
#            self.handle_close()

    #解析测试命令
    def OnCommand(self, command):
        data = ''
        if command[0] == 'init':
            data = struct.pack('!4s2i4H32s2H', '\r\n\r\n', 44, 0, 0x1000, 0, 0, 0, 'kjava_3.00', 1, 0xffff)
        elif command[0] == 'vf':
            if (len(command) == 2):
                data = struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x0900, 0, 0, 0, '', 1, 0xffff, 0, command[1], '', '')
        elif command[0] == 're':
            if (len(command) == 3):
                data = struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0400, 0x1000, 0, 0, '', 1, 0xffff, string.atoi(command[1]), command[2], '')
        elif command[0] == 'newre':
            if (len(command) == 3):
                data = struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0B00, 0x1000, 0, 0, '', 1, 0xffff, string.atoi(command[1]), command[2], '')
        elif command[0] == 'overlapre':
            if (len(command) == 3):
                data = struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0F00, 0x1000, 0, 0, '', 1, 0xffff, string.atoi(command[1]), command[2], '')
        elif command[0] == 're2':
            if (len(command) == 3):
                data = struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0d00, 0x1000, 0, 0, '', 1, 0xffff, string.atoi(command[1]), command[2], '')
        elif command[0] == 'tr':
            if (len(command) == 5):
                data = struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x0100, 0x1000, 0, 0, '', string.atoi(command[3]), string.atoi(command[4]), string.atoi(command[1]), command[2], '', '')
        elif command[0] == 'k':
            if (len(command) == 5):
                data = struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x0600 | string.atoi(command[1]), 0x1000, 0, 0, '', string.atoi(command[4]), 0, string.atoi(command[2]), command[3], '', '')
        elif command[0] == 's':
            if (len(command) == 5):
                data = struct.pack('!4s2i4H32s2H', '\r\n\r\n', 44, 0, string.atoi(command[1]), string.atoi(command[2]), 0, 0, '', string.atoi(command[3]), string.atoi(command[3]) * string.atoi(command[4]))
        elif command[0] == 'f10':
            if (len(command) == 4):
                data = struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x1200, 0x1000, 0, 0, '', 1, string.atoi(command[3]), string.atoi(command[1]), command[2], '', '')
        elif command[0] == 'allstat':
            if (len(command) == 2):
                data = struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x1600, string.atoi(command[1]), 0, 0, '', 0, 0xffff, 0, '', '', '')

        if len(data) != 0:
            st.num_req += 1
            self.push(data)
            self.base_time = time.clock()
        return data

class shown_thread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.char_num = 0

    def run(self):
        while True:
            time.sleep(1)
            print '\b'.zfill(self.char_num+2).replace('0', '\b'),
            s = 'Connect: %d/%d, %.2f/%.2f/%.2f, Business: %d/%d, %.2f/%.2f/%.2f' % (int(st.num_fail), int(st.num_con), float(st.min_connect), st.avg_connect(), float(st.max_connect), int(st.num_ret), int(st.num_req), float(st.min_business), st.avg_business(), float(st.max_business))
            self.char_num = len(s)
            print s,

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print 'Parameters: sim-number svr-ip svr-port'
        print 'Example: pressure 1 127.0.0.1 8001'
        sys.exit(0)

    st = statistics()
    trd = shown_thread()
    trd.setDaemon(True)
    trd.start()
    time.clock()
    for i in xrange(int(sys.argv[1])):
        s = client((sys.argv[2], int(sys.argv[3])))
    asyncore.loop()
