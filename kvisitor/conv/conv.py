
#coding: utf-8

import struct, os, sys, sqlite3, re, time

qlfmt = '10I'
qlsh = 'd:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SHASE\\day'
qlsz = 'd:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SZNSE\\day'

fmtzs = re.compile(r'^399\d{3}$')

def readqlfile(symb, filepath, func = None):
    fp = open(filepath, 'rb')
    qlbuf = fp.read()
    fp.close()

    while (len(qlbuf)):
        day, jrkp, zgjg, zdjg, zjjg, cjje, cjsl, nouse, nouse, nouse = struct.unpack_from(qlfmt, qlbuf)
        qlbuf = qlbuf[struct.calcsize(qlfmt):]

        dayk = (day, jrkp, zgjg, zdjg, zjjg, cjsl, cjje)
        if func:
            func(symb, dayk)
        else:
            print symb, '%d:%9d%9d%9d%9d%13d%13d' % dayk

def readqlszdir(func = None):
    for r, d, files in os.walk(qlsz):
        for f in files:
            fn, ext = f.split('.')
            readqlfile(fn, os.path.join(r, f), func)

def readqlshdir(func = None):
    for r, d, files in os.walk(qlsh):
        for f in files:
            fn, ext = f.split('.')
            readqlfile(fn, os.path.join(r, f), func)

year, sqls, sql = 0, {}, ''
def test(symb, dayk):
    if dayk[0] >= 20090601:
        return

    global year, sqls, sql
    if dayk[0] / 10000 != year:
        if len(sql):
            if sqls.has_key(year):
                sqls[year] = sqls[year] + sql
            else:
                sqls[year] = sql
        year = dayk[0] / 10000
        sql = ''

    sql += "update dayk set amount=%d, sum=%d where symbol='%s.SZ' and day=%d;" % (dayk[5], dayk[6], symb, dayk[0])

def checkdayk(symb, day):
    '''
    对比检查钱龙和自己的k线数据
    '''
    def printDayk(symb, dayk):
        if dayk[0] == day:
            print '%8s:%9d%9d%9d%9d%13d%13d' % ('ql', dayk[1], dayk[2], dayk[3], dayk[4], dayk[5], dayk[6])

    print '%s [%d]:' % (symb, day)
    fn, ext = symb.split('.')
    if ext.lower() == 'sh':
        fn = qlsh + '\\' + fn + '.day'
    else:
        fn = qlsz + '\\' + fn + '.day'
    readqlfile(symb, fn, printDayk)

    now = time.localtime().tm_year
    if day/10000 == now:
        sqlfn = '%s.k' % ext.lower()
    else:
        sqlfn = '%s.%d' % (ext.lower(), day / 10000)

    conn = sqlite3.connect(sqlfn)
    c = conn.cursor()
    c.execute("select * from dayk where symbol='%s' and day=%d" % (symb, day))
    buf = c.fetchone()
    if buf:
        print '%8s:%9d%9d%9d%9d%13d%13d' % ('my', buf[1], buf[2], buf[3], buf[4], buf[5], buf[6])
    else:
        print '%8s:' % ('my')
    c.close()
    conn.close()



def upszcjsl():
    """
    深圳指数20100601之前的成交量和成交金额和钱龙对不上
    """
    global year, sqls, sql
    year, sqls, sql = 0, {}, ''
    desfilelst = []
    for r, d, files in os.walk('.'):
        for f in files:
            if f == sys.argv[0]:
                continue
            desfilelst.append(f)

    readqlszdir(test)
    if len(sql):
        if sqls.has_key(year):
            sqls[year] = sqls[year] + sql
        else:
            sqls[year] = sql

    for year in sqls.keys():
        sql = sqls[year]
        print year, len(sql), sql.count(';')
        conn = sqlite3.connect('sz.%d' % year)
        c = conn.cursor()
        sql = ''.join(['begin;', sql])
        sql += 'commit;'
        c.executescript(sql)
        conn.commit()
        c.close()
        conn.close()

def upsh20100603():
    def onshrecord(symb, dayk):
        global year, sqls, sql
        if dayk[0] != 20100826:
            return

        if int(symb) < 2000:
            symb += '.SH'
        else:
            symb += '.sh'

        c.execute("select count(*) from dayk where symbol='%s' and day=20100826" % symb)
        cnt = int(c.fetchone()[0])

        if cnt != 0:
            return

        sql = "insert into dayk values('%s', %d, %d, %d, %d, %d, %d, 1, 20100826)" % (symb, dayk[1], dayk[2], dayk[3], dayk[4], dayk[5], dayk[6])
        try:
            c.execute(sql)
            conn.commit()
        except:
            print sys.exec_info()
            raw_input()

    global year, sqls, sql
    sql = ''
    conn = sqlite3.connect('sh.k')
    c = conn.cursor()
    readqlshdir(onshrecord)
    c.close()
    conn.close()


if __name__ == '__main__':
    upsh20100603()
    checkdayk('000001.SH', 20100826)
    checkdayk('000002.SH', 20100826)
