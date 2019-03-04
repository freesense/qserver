
#coding:utf-8

import asyncore, asynchat, socket, struct, sys, atom, random, threading, time, os

fmt_head = '=2BHI'
mode = 1                #1:ҵ������,2:��������
num_client = 300        #��������
sleep_time = 5

#��������ip�Ͷ˿�
#addr = '121.15.4.57'
#addr = '127.0.0.1'
addr = '192.168.11.25'
port = 11001

#���Ӵ�����ʧ�����Ӵ�����������������ش���
class statistics:
    def __init__(self):
        self.num_con = atom.atom()              #���Ӵ���
        self.num_conok = atom.atom()            #��������
        self.num_fail = atom.atom()             #����ʧ�ܴ���
        self.num_disconnect = atom.atom()       #���ӶϿ�����
        self.num_req = atom.atom()              #�������
        self.num_ret = atom.atom()              #Ӧ�����

        self.sum_connect = atom.atom()          #����ʱ��֮��
        self.max_connect = atom.atom()          #�������ʱ��
        self.min_connect = atom.atom(999999)    #��С����ʱ��
        self.sum_business = atom.atom()         #ҵ��ʱ��֮��
        self.max_business = atom.atom()         #���ҵ��ʱ��
        self.min_business = atom.atom(999999)   #��Сҵ��ʱ��

        f = open('test.script')
        lns = f.readlines()
        self.scripts = []
        for script in lns:
            if script.find('#') == -1 and script[0] != '\n':    #û�ҵ�ע�ʹ���
                self.scripts.append(script)
        if len(self.scripts) == 0:
            raise Exception('���Խű�Ϊ�գ�')

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
        self.con_status = False
        self.remote_address = remote_address
        self.connect_svr()

    def connect_svr(self):
        """���������������֮�������"""
        self.read_buffer = ''
        self.recv_flag = 0
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)
        self.connect(self.remote_address)
        self.set_terminator(struct.calcsize(fmt_head))
        st.num_con += 1
        self.base_time = time.clock()

    def handle_close(self):
        """�����ӶϿ�ʱ��ϵͳ�ص�"""
        self.close()
        st.num_disconnect += 1
        if self.con_status:
            st.num_conok -= 1
        self.con_status = False
        self.connect_svr()

    def handle_expt(self):
        """�������쳣ʱ��ϵͳ�ص�"""
        print '\n>>> �����쳣:', self.socket.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR),
        self.close()
        st.num_fail += 1
        if self.con_status:
            st.num_conok -= 1
        self.con_status = False
        self.connect_svr()

    def handle_connect(self):
        tm = time.clock() - self.base_time
#        print '����ʱ��:', tm
        st.sum_connect += tm
        st.num_conok += 1
        self.con_status = True
        st.max_connect = max(st.max_connect, tm)
        st.min_connect = min(st.min_connect, tm)

        command = random.choice(st.scripts)
        self.OnCommand(command.split())

    def OnCommand(self, cmd):
        """������������"""
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
        elif func == 0x0001:
            data = struct.pack('2H'+cmd[0], func, 1, func)
        else:
            print 'δ֪�Ĳ�������:', func
            return

        if len(data) != 0:
            st.num_req += 1
            head = struct.pack(fmt_head, 48, 0, 0, len(data))
            self.base_time = time.clock()
            self.push(head)
            self.push(data)

    def collect_incoming_data(self, data):
        self.read_buffer += data

    def found_terminator(self):
        if len(self.read_buffer) == struct.calcsize(fmt_head):
            head0, head1, head2, length = struct.unpack(fmt_head, self.read_buffer)
            self.set_terminator(length)
        else:
            #��ӡ��������
            tm = time.clock() - self.base_time
            st.min_business = min(st.min_business, tm)
            st.max_business = max(st.max_business, tm)
#            print 'ҵ��ʱ��:', tm, '���մ�С:', len(self.read_buffer)
#            print self.read_buffer
            st.sum_business += tm
            st.num_ret += 1
            self.read_buffer = ''
            self.set_terminator(struct.calcsize(fmt_head))

            command = random.choice(st.scripts)
            self.OnCommand(command.split())

class shown_thread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.char_num = 0
        self.seconds = 0

    def run(self):
        while True:
            time.sleep(1)
            self.seconds += 1
            print '\b'.zfill(self.char_num+2).replace('0', '\b'),
            s = '����[%d/%d/%d/%d, %.2f/%.2f/%.2f] ҵ��[%d/%d/%d, %.2f/%.2f/%.2f]' % (int(st.num_fail), int(st.num_disconnect), int(st.num_conok), int(st.num_con), float(st.min_connect), st.avg_connect(), float(st.max_connect), int(st.num_ret), int(st.num_req), int(int(st.num_ret)/self.seconds), float(st.min_business), st.avg_business(), float(st.max_business))
            self.char_num = len(s)
            print s,

if __name__ == '__main__':
    print '����[ʧ��/�Ͽ�/����/�����Ӵ���, MIN/AVG/MAX] ҵ��[����/����/�ٶ�, MIN/AVG/MAX]'

    st = statistics()
    trd = shown_thread()
    trd.setDaemon(True)
    trd.start()
    time.clock()
    for i in xrange(num_client):
        s = client((addr, port))
    asyncore.loop()
