
#coding:utf-8

import sys, struct, os

fmtql = '10I'
oqlsh = 'D:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SHASE\\day\\'
oqlsz = 'D:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SZNSE\\day\\'
qlsh = 'D:\\Programs\\Qianlong\\qijian\\QLDATA\\history1\\SHASE\\day\\'
qlsz = 'D:\\Programs\\Qianlong\\qijian\\QLDATA\\history1\\SZNSE\\day\\'

def default2zero(num):
    if num == None or len(num) == 0:
        return 0
    else:
        return float(num)

def import_zsline(line):
    path = ''
    #opath = ''
    line = line.strip(' \r\n')
    tmp = line.split(',')
    day, symbol, unuse, unuse, jys, unuse, jrkp, zgjg, zdjg, zjjg, cjsl, cjje = tmp
    day = int(''.join(day.split(' ')[0].split('-')))
    symbol = symbol.strip('"')
    jys = jys.strip('"')
    if int(symbol) < 399000 and int(symbol) != 300 and jys == '深交所':
        print line
        return

    if jys == '上交所':
        path = qlsh + symbol + '.day'
        #opath = oqlsh + symbol + '.day'
    elif jys == '深交所':
        path = qlsz + symbol + '.day'
        #opath = oqlsh + symbol + '.day'

    jrkp = int(default2zero(jrkp) * 1000)
    zdjg = int(default2zero(zdjg) * 1000)
    zgjg = int(default2zero(zgjg) * 1000)
    zjjg = int(default2zero(zjjg) * 1000)
    cjsl = int(default2zero(cjsl))
    cjje = int(default2zero(cjje) / 1000)

    fw = open(path, 'a+b')
    fw.write(struct.pack(fmtql, day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, 0, 0, 0))
    fw.close()

def import_gpline(line):
    path = ''
    #opath = ''

    line = line.strip(' \r\n')
    tmp = line.split(',')
    symbol, unuse, day, unuse, jys, unuse, jrkp, zgjg, zdjg, zjjg, cjsl, cjje, unuse, unuse = tmp
    day = int(''.join(day.split(' ')[0].split('-')))
    symbol = symbol.strip('"')
    jys = jys.strip('"')
    if jys == '上交所':
        path = qlsh + symbol + '.day'
        #opath = oqlsh + symbol + '.day'
    elif jys == '深交所':
        path = qlsz + symbol + '.day'
        #opath = oqlsh + symbol + '.day'

    jrkp = int(default2zero(jrkp) * 1000)
    zdjg = int(default2zero(zdjg) * 1000)
    zgjg = int(default2zero(zgjg) * 1000)
    zjjg = int(default2zero(zjjg) * 1000)
    cjsl = int(default2zero(cjsl) / 100)
    cjje = int(default2zero(cjje) / 1000)

    if zjjg != 0 and cjsl != 0:
        fw = open(path, 'a+b')
        fw.write(struct.pack(fmtql, day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, 0, 0, 0))
        fw.close()

def import_ql(latestday, path):
    qlpath = path.replace('history', 'history1')

def import_zs():
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

def import_gp():
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

def import_ql():
    for root, dirs, files in os.walk(oqlsh):
        for f in files:
            lastday = 0
            fpo = open(os.path.join(root, f), 'rb')
            buf = fpo.read()
            fpo.close()

            try:
                fpn = open(qlsh+f, 'rb')
                fpn.seek(-struct.calcsize(fmtql), os.SEEK_END)
                lastday = fpn.read(struct.calcsize(fmtql))
                lastday = struct.unpack(fmtql, lastday)
                lastday = lastday[0]
                fpn.close()
            except:
                pass

            fw = open(qlsh+f, 'a+b')
            while (len(buf)):
                day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, nouse, nouse, nouse = struct.unpack_from(fmtql, buf)
                buf = buf[struct.calcsize(fmtql):]
                if day > lastday:
                    lastday = day
                    fw.write(struct.pack(fmtql, day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, 0, 0, 0))
            fw.close()

    for root, dirs, files in os.walk(oqlsz):
        for f in files:
            lastday = 0
            fpo = open(os.path.join(root, f), 'rb')
            buf = fpo.read()
            fpo.close()

            try:
                fpn = open(qlsz+f, 'rb')
                fpn.seek(-struct.calcsize(fmtql), os.SEEK_END)
                lastday = fpn.read(struct.calcsize(fmtql))
                lastday = struct.unpack(fmtql, lastday)
                lastday = lastday[0]
                fpn.close()
            except:
                pass

            fw = open(qlsz+f, 'a+b')
            while (len(buf)):
                day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, nouse, nouse, nouse = struct.unpack_from(fmtql, buf)
                buf = buf[struct.calcsize(fmtql):]
                if day > lastday:
                    lastday = day
                    fw.write(struct.pack(fmtql, day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, 0, 0, 0))
            fw.close()

if __name__ == '__main__':
    import_zs()
    print '指数导入OK'
    import_gp()
    print '股票导入OK'
    import_ql()
    print '合并最新K线OK'
