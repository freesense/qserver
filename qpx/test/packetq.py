
#coding:utf-8

import string, struct

def helpcommand():
    print '%40s: %s' % ('quit', '�˳����Գ���')
    print '%40s: %s' % ('sw hexdump', '�л���ӡ��������״̬��Ĭ�Ϲر�')
    print '%40s: %s' % ('lm', '�г������г�����')
    print '%40s: %s' % ('ls', '�г��������а�����')
    print '%40s: %s' % ('init', '��ʼ��')
    print '%40s: %s' % ('vf <stockcode>', '����У��')
    print '%40s: %s' % ('re <market><stock>', 'ʵʱ��������')
    print '%40s: %s' % ('re2 <market><stock>', '���շ�������')
    print '%40s: %s' % ('tr <market><stock><number><position>', '�ֱʳɽ�')
    print '%40s: %s' % ('k <type><market><stock><number>', '��ʷK��')
    print '%40s: %s' % ('s <type><market><number><position>', '���а�')
    print '%40s: %s' % ('zx <market><stock>...', '��ѡ��')
    print '%40s: %s' % ('tx <market><stock>...', '������ѡ��')
    print '%40s: %s' % ('f10 <market><stock><type>', 'ȡ��Ѷ��Ϣ')
    print '%40s: %s' % ('st <stock><market>', 'ȡ���鱨��')
    print '%40s: %s' % ('allstat <market>', '����֤ȯ�ı�����Ϣ')
    print '%40s: %s' % ('newre <market><stock>', '�·�ʱͳ��Э��')
    print '%40s: %s' % ('overlapre <market><stock>', '�·�ʱͳ��Э��')

def InitMarket():
    return struct.pack('!4s2i4H32s2H', '\r\n\r\n', 44, 0, 0x1000, 0, 0, 0, 'kjava_3.00', 1, 0xffff)

def Verify(code):
    return struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x0900, 0, 0, 0, '', 1, 0xffff, 0, code, '', '')

def Real2(market, code):
    return struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0d00, 0x1000, 0, 0, '', 1, 0xffff, market, code, '')

def Real(market, code):
    return struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0400, 0x1000, 0, 0, '', 1, 0xffff, market, code, '')

def NewReal(market, code):
    return struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0B00, 0x1000, 0, 0, '', 1, 0xffff, market, code, '')
    
def OverlapReal(market, code):
    return struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0F00, 0x1000, 0, 0, '', 1, 0xffff, market, code, '')
    
def SymbolStatus(market, code):
    return struct.pack('!4s2i4H32sHiH6s12s', '\r\n\r\n', 66, 0, 0x0300, 0x1000, 0, 0, '', 1, 0xffff, market, code, '')

def allstat(mcode):
    return struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x1600, mcode, 0, 0, '', 0, 0xffff, 0, '', '', '')

def Trace(market, code, num, pos):
    return struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x0100, 0x1000, 0, 0, '', num, pos, market, code, '', '')

def KLine(ktype, market, code, num):
    return struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x0600 | ktype, 0x1000, 0, 0, '', num, 0, market, code, '', '')

def Sort(stype, market, num, pos):
    return struct.pack('!4s2i4H32s2H', '\r\n\r\n', 44, 0, stype, market, 0, 0, '', num, num * pos)

def Select(symbols):
    fmt_code = '!H6s12s12s'
    data_tail = ''
    for i in xrange(len(symbols) / 2):
        symbol = symbols[i*2:(i+1)*2]
        data_tail += struct.pack(fmt_code, int(symbol[0]), symbol[1], '', '')
    return struct.pack('!4s2i4H32sHi', '\r\n\r\n', len(data_tail) + 42, 0, 0x0200, 0x1000, 0, 0, '', len(symbols) / 2, 0xffffffff)
    data += data_tail

def Select2(symbols):
    fmt_code = '!H6s12s12s'
    data_tail = ''
    for i in xrange(len(symbols) / 2):
        symbol = symbols[i*2:(i+1)*2]
        data_tail += struct.pack(fmt_code, int(symbol[0]), symbol[1], '', '')
    data = struct.pack('!4s2i4H32sHi', '\r\n\r\n', len(data_tail) + 42, 0, 0x1700, 0x1000, 0, 0, '', len(symbols) / 2, 0xffffffff)
    data += data_tail
    return data

def f10(market, code, itype):
    return struct.pack('!4s2i4H32sHiH6s12s12s', '\r\n\r\n', 78, 0, 0x1200, 0x1000, 0, 0, '', 1, itype, market, code, '', '')

def make_send_buffer(command):
    if command[0] == 'init':
        return InitMarket()
    elif command[0] == 'vf':
        if len(command) == 2:
            return Verify(command[1])
        else:
            print '����У����Ҫ֤ȯ�������'
            return ''
    elif command[0] == 'st':
        if len(command) == 2:
            return SymbolStatus(0, command[1])
        elif len(command) == 3:
            return SymbolStatus(string.atoi(command[2]), command[1])
        else:
            print '����������Ҫ֤ȯ������г�����(��ѡ)����'
            return ''
    elif command[0] == 're':
        if len(command) == 3:
            return Real(string.atoi(command[1]), command[2])
        else:
            print '����ʵʱ����������Ҫ�г����룬֤ȯ�������'
            return ''
    elif command[0] == 'newre':
        if len(command) == 3:
            return NewReal(string.atoi(command[1]), command[2])
        else:
            print '�����·�ʱͳ��Э����Ҫ�г����룬֤ȯ�������'
            return ''
    elif command[0] == 'overlapre':
        if len(command) == 3:
            return OverlapReal(string.atoi(command[1]), command[2])
        else:
            print '������ӷ�ʱЭ����Ҫ�г����룬֤ȯ�������'
            return ''
    elif command[0] == 're2':
        if len(command == 3):
            return Real2(string.atoi(command[1]), command[2])
        else:
            print '�������շ���������Ҫ�г����룬֤ȯ�������'
            return ''
    elif command[0] == 'tr':
        if len(command) == 5:
            return Trace(string.atoi(command[1]), command[2], string.atoi(command[3]), string.atoi(command[4]))
        else:
            print '����ֱʳɽ���Ҫ�г����룬֤ȯ���룬�����������ʼλ�ò���'
            return ''
    elif command[0] == 'k':
        if len(command) == 5:
            return KLine(string.atoi(command[1]), string.atoi(command[2]), command[3], string.atoi(command[4]))
        else:
            print '����K����ҪK�����ͣ��г����룬֤ȯ�����������������'
            return ''
    elif command[0] == 's':
        if len(command) == 5:
            return Sort(string.atoi(command[1]), string.atoi(command[2]), string.atoi(command[3]), string.atoi(command[4]))
        else:
            print '���а���Ҫ���ͣ��г����룬������������ʼλ�ò���'
            return ''
    elif command[0] == 'f10':
        if len(command) == 4:
            return f10(string.atoi(command[1]), command[2], string.atoi(command[3]))
        else:
            print 'F10��Ϣ��Ҫ�г����룬֤ȯ�������Ϣ���Ͳ���'
            return ''
    elif command[0] == 'allstat':
        if len(command) == 2:
            return allstat(string.atoi(command[1]))
        else:
            print '���б�����Ҫ�г��������'
            return ''
    elif command[0] == 'zx':
        if len(command) > 2:
            args = tuple(command[1:])
            return Select(args)
        else:
            print '�������г�����͹�Ʊ����'
            return ''
    elif command[0] == 'tx':
        if len(command) > 2:
            args = tuple(command[1:])
            return Select2(args)
        else:
            print '�������г�����͹�Ʊ����'
            return ''
    else:
    	helpcommand()

if __name__ == '__main__':
    print make_send_buffer(['re', '4097', '000001'])
