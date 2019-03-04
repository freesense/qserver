
#coding:utf-8

import asyncore, asynchat, socket, struct

fmt_head = '=2BHI'
addr = ('', 11001)

class server_handle(asynchat.async_chat):
    """定义行情服务器的连接处理器"""
    def __init__(self, conn):
        asynchat.async_chat.__init__(self)
        self.set_socket(conn)
        self.read_buffer = ''
        self.set_terminator(struct.calcsize(fmt_head))

    def handle_connect(self):
        pass

    def collect_incoming_data(self, data):
        self.read_buffer += data

    def found_terminator(self):
        if len(self.read_buffer) == struct.calcsize(fmt_head):
            head0, head1, head2, length = struct.unpack(fmt_head, self.read_buffer)
            self.set_terminator(length)
        else:
            self.read_buffer = ''
            self.set_terminator(struct.calcsize(fmt_head))

            data = '11111111234566666666666666667889999998uygtfr4esdcfvbnju765432wqasdert5yyyyyyyyyyyyyyyujhyt6';
            head = struct.pack(fmt_head, 48, 0, 0, len(data))
            self.push(head)
            self.push(data)

    def handle_close(self):
        self.close()
        print '>>> 连接断开'

class server(asyncore.dispatcher):
    def __init__(self):
        asyncore.dispatcher.__init__(self)

    def start(self):
        """监听特定地址"""
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)
        self.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, True)
        self.bind(addr)
        self.listen(5)

    def handle_accept(self):
        """行情服务器连接到本机"""
        conn, address = self.accept()
        print '行情服务器', address, '已经连接'
        handler = server_handle(conn)

if __name__ == '__main__':
    s = server()
    s.start()
    asyncore.loop()
