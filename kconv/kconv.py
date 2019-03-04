
#coding:utf-8

import sys, struct, os, cPickle

fmtmy, fmtql = '7I', '10I'
dsymbol = {}
dmaxday = {}
_qlsh = 'D:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SHASE\\day\\'
_qlsz = 'D:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SZNSE\\day\\'

class dayk:
    def __init__(self, day, jrkp, zgjg, zdjg, zjjg, cjsl, cjje):
        self.day = day
        self.jrkp = jrkp
        self.zgjg = zgjg
        self.zdjg = zdjg
        self.zjjg = zjjg
        self.cjsl = cjsl
        self.cjje = cjje

    def toStr(self):
        return struct.pack(fmtmy, self.day, self.jrkp, self.zgjg, self.zdjg, self.zjjg, self.cjsl, self.cjje)

def default2zero(num):
    if num == None or len(num) == 0:
        return 0
    else:
        return float(num)

def import_zsline(line):
    line = line.strip(' \r\n')
    tmp = line.split(',')
    day, symbol, unuse, unuse, jys, unuse, jrkp, zgjg, zdjg, zjjg, cjsl, cjje = tmp
    day = int(''.join(day.split(' ')[0].split('-')))
    symbol = symbol.strip('"')
    jys = jys.strip('"')
    if jys == '上交所':
        symbol += '.shidx'
    elif jys == '深交所':
        symbol += '.szidx'

    jrkp = int(default2zero(jrkp) * 1000)
    zdjg = int(default2zero(zdjg) * 1000)
    zgjg = int(default2zero(zgjg) * 1000)
    zjjg = int(default2zero(zjjg) * 1000)
    cjsl = int(default2zero(cjsl) / 100)
    cjje = int(default2zero(cjje) / 1000)

#    if not dsymbol.has_key(symbol):
#        dsymbol[symbol] = []
#        dmaxday[symbol] = 0
#    dsymbol[symbol].append(dayk(day, jrkp, zgjg, zdjg, zjjg, cjsl, cjje))
#    dmaxday[symbol] = max(dmaxday[symbol], day)

def import_gpline(line):
    line = line.strip(' \r\n')
    tmp = line.split(',')
    symbol, unuse, day, unuse, jys, unuse, jrkp, zgjg, zdjg, zjjg, cjsl, cjje, unuse, unuse = tmp
    day = int(''.join(day.split(' ')[0].split('-')))
    symbol = symbol.strip('"')
    jys = jys.strip('"')
    if jys == '上交所':
        symbol += '.sh'
    elif jys == '深交所':
        symbol += '.sz'

    jrkp = int(default2zero(jrkp) * 1000)
    zdjg = int(default2zero(zdjg) * 1000)
    zgjg = int(default2zero(zgjg) * 1000)
    zjjg = int(default2zero(zjjg) * 1000)
    cjsl = int(default2zero(cjsl) / 100)
    cjje = int(default2zero(cjje) / 1000)

    if not dsymbol.has_key(symbol):
        dsymbol[symbol] = []
        dmaxday[symbol] = 0
    dsymbol[symbol].append(dayk(day, jrkp, zgjg, zdjg, zjjg, cjsl, cjje))
    dmaxday[symbol] = max(dmaxday[symbol], day)

def import_ql():
    qlsh = 'D:\\Programs\\Qianlong\\qijian\\QLDATA\\history1\\SHASE\\day\\'
    qlsz = 'D:\\Programs\\Qianlong\\qijian\\QLDATA\\history1\\SZNSE\\day\\'

    for root, dirs, files in os.walk(qlsh):
        for f in files:
            symbol = int(f[0:6])
            if symbol < 696:
                symbol = f[0:6]+'.shidx'
            else:
                symbol = f[0:6]+'.sh'

            fp = open(os.path.join(root, f), 'rb')
            buf = fp.read()
            while (len(buf)):
                day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, nouse, nouse, nouse = struct.unpack_from(fmtql, buf)
                buf = buf[struct.calcsize(fmtql):]
                if not dmaxday.has_key(symbol):
                    dsymbol[symbol] = []
                    dmaxday[symbol] = 0
                if day > dmaxday[symbol]:
                    dsymbol[symbol].append(dayk(day, jrkp, zgjg, zdjg, zjjg, cjsl, cjje))
                    dmaxday[symbol] = max(dmaxday[symbol], day)
            fp.close()

    for root, dirs, files in os.walk(qlsz):
        for f in files:
            symbol = int(f[0:6])
            if symbol < 399999 and symbol >= 399000:
                symbol = f[0:6]+'.szidx'
            else:
                symbol = f[0:6]+'.sz'

            fp = open(os.path.join(root, f), 'rb')
            buf = fp.read()
            while (len(buf)):
                day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, nouse, nouse, nouse = struct.unpack_from(fmtql, buf)
                buf = buf[struct.calcsize(fmtql):]
                if not dmaxday.has_key(symbol):
                    dsymbol[symbol] = []
                    dmaxday[symbol] = 0
                if day > dmaxday[symbol]:
                    dsymbol[symbol].append(dayk(day, jrkp, zgjg, zdjg, zjjg, cjsl, cjje))
                    dmaxday[symbol] = max(dmaxday[symbol], day)
            fp.close()

def importfromtxt():
    f = open('zs', 'r')
    line = f.readline()

    try:
        while len(line):
            import_zsline(line)
            line = f.readline()
    except:
        print line
        print sys.exc_info()

    f.close()

    f = open('zqrhq', 'r')
    line = f.readline()

    try:
        while len(line):
            import_gpline(line)
            line = f.readline()
    except:
        print line
        print sys.exc_info()

    f.close()

if __name__ == '__main__':
    importfromtxt()
    print len(dsymbol)
    import_ql()
    print len(dsymbol)

    f = open('tmp', 'wb')
    cPickle.dump(dsymbol, f)
    f.close()

    #for symbol, dayks in dsymbol.items():
    #    dayks.sort(lambda d1, d2: cmp(d1.day, d2.day))
