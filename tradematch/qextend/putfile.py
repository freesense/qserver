from ftplib import FTP
from os import path
def putfile(server,user,password,uploaddir,filename):
   try:
      ftp=FTP(server)
      ftp.login(user,password)
      ftp.cwd(uploaddir)
      basefilename=path.basename(filename)
      try:
         ftp.delete(basefilename)
      except:
         pass
      f=open(filename,'rb')
      ftp.storbinary('STOR '+basefilename,f)
      f.close()
      ftp.quit()
   except Exception, msg:
      print msg[0]
