from ftplib import FTP

ftp = FTP()
s = ftp.connect('127.0.0.1', 21)
print s
