
#coding:utf-8

import re, os, struct

fmtmy = '7I'
fp = open('error.txt')
lines = fp.readlines()
fp.close()
fname = None
cjsl = 0
cjje = 0

for line in lines:
    if line.find('У�Ա���ֹ!') != -1:
        fname = None
        cjsl = 0
        cjje = 0
        continue

    if line.find('У�����') != -1:
        if cjsl or cjje:
            print fname, cjsl, cjje
            fp = open(fname, 'r+b')
            fp.seek(-struct.calcsize(fmtmy), os.SEEK_END)
            day, jrkp, zgjg, zdjg, zjjg, sl, je = struct.unpack(fmtmy, fp.read(struct.calcsize(fmtmy)))
            fp.seek(-struct.calcsize(fmtmy), os.SEEK_END)
            if day == 20100408:
                if cjsl:
                    sl = cjsl
                if cjje:
                    je = cjje
                fp.write(struct.pack(fmtmy, day, jrkp, zgjg, zdjg, zjjg, sl, je))
            fp.close()

        fname = None
        cjsl = 0
        cjje = 0
        continue

    result = re.search('���ڼ���ļ�\[(.+?)\]', line)
    if result:
        fname = os.path.split(result.groups()[0])[1]
        fname = 'day\\'+fname
        continue

    result = re.search('����\[20100408\]�ɽ�����һ�������ϣ�\d+ ���أ�(\d+)', line)
    if result:
        cjsl = int(result.groups()[0])
        continue

    result = re.search('����\[20100408\]�ɽ���һ�������ϣ�\d+ ���أ�(\d+)', line)
    if result:
        cjje = int(result.groups()[0])
        continue
