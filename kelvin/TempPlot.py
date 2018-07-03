#!/usr/bin/python3
#/bin/python3
######################################################
# This script makes temperature plots of the four 
# labjack probes labeled A1,B2,C3,D4 and labeled as 
# such. 
# This script can be used anywhere by changing the 
# location of the python3 directive after #!.
# usage: ./TempPlot.py and use default
#    or: ./TempPlot.py file_name
# Because of the details of install on this memnog (RHEL5)
# it is implemented through software collections (scl)
# and the bash script ./TempPlot.sh
# 
# N.T. BREWER 9-31-2015
#######################################################
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import pexpect as pxp
import time
import numpy as np
import sys

def getFileName():
    try:
        s=pxp.run('ls -rt log/').split()[-1].decode()
    except IndexError:
        print('file not found: input filename')
        s=input()
    if s!='':
        return('log/'+s)
    else:
        file_name='log/therm-WedSep3013:45:052015.log'
        print('Warning, standard usage is: ./TempPlot.py file_name\nusing default file name:' + file_name)

def getLines(filename):
    """
    maybe add way to return max number of lines and check that request cannot be bigger?
    """
    inf = open(filename,"r")
    lines = inf.readlines()
    inf.close()
    return(lines)

try:
    file_name=sys.argv[1]
except IndexError:
    file_name = getFileName()

inf=open(file_name)
lines=inf.readlines()
blocks=[]

date=file_name.split('therm-')[-1].split('.log')[0]
if len(date.split(':'))<3:
   date=input('date not found in file name. Input date as datetime format %m%d%H%M%S%Y i.e. Sep. 9th 2015 at 11:11:03pm is 09092311032015: ')
   s=time.mktime(time.strptime(date,'%m%d%H%M%S%Y'))+62135665200.0
   d=s/(86400)
else:
   if len(date.split(':')[0]) ==9:
       date=date[0:6]+'0'+date[6:]
   s=time.mktime(time.strptime(date,'%a%b%d%H:%M:%S%Y'))+62135665200.0
   d=s/(86400)
   print(d)
   
   
print('start date' +str(date))

for i in lines[3:]:
    blocks.append(i.split('\t'))
tdat=[];a1dat=[];b2dat=[];c3dat=[];d4dat=[];e5dat=[]
for i in blocks:
    tdat.append(eval(i[0])/86400 +d)
    a1dat.append(eval(i[1]))
    b2dat.append(eval(i[2]))
    c3dat.append(eval(i[3]))
    d4dat.append(eval(i[4]))
#    e5dat.append(eval(i[5]))

#print('ok')
fig = plt.figure()
fig.autofmt_xdate()

ax1=plt.subplot(411)
plt.plot_date(tdat,a1dat,'-')
plt.legend(['A1'])
plt.plot_date(tdat,a1dat,'ko')

plt.subplot(412,sharex=ax1)
plt.plot_date(tdat,b2dat,'-')
plt.legend(['B2'])
plt.plot_date(tdat,b2dat,'ko')

ax=plt.subplot(413,sharex=ax1)
fig.autofmt_xdate()
ax.xaxis.set_major_locator(mdates.DayLocator())
ax.xaxis.set_major_formatter(mdates.DateFormatter('%b%d  %Y  .'))
ax.xaxis.set_minor_locator(mdates.HourLocator())
ax.xaxis.set_minor_formatter(mdates.DateFormatter('%H'))

plt.plot_date(tdat,c3dat,'-')
plt.legend(['C3'])
plt.plot_date(tdat,c3dat,'ko')

plt.subplot(414,sharex=ax1)
fig.autofmt_xdate()
ax.xaxis.set_major_locator(mdates.DayLocator())
ax.xaxis.set_major_formatter(mdates.DateFormatter('%b%d  %Y  .'))
ax.xaxis.set_minor_locator(mdates.HourLocator())
ax.xaxis.set_minor_formatter(mdates.DateFormatter('%H'))
plt.plot_date(tdat,d4dat,'-')
plt.legend(['D4'])
plt.plot_date(tdat,d4dat,'ko')

#ax = plt.subplot(515)
#fig.autofmt_xdate()
#ax.xaxis.set_major_locator(mdates.DayLocator())
#ax.xaxis.set_major_formatter(mdates.DateFormatter('%b%d  %Y  .'))
#ax.xaxis.set_minor_locator(mdates.HourLocator())
#ax.xaxis.set_minor_formatter(mdates.DateFormatter('%H'))
#plt.plot_date(tdat,e5dat,'-')
#plt.legend(['E5'])
#plt.plot_date(tdat,e5dat,'ko')
plt.show()
#plt.savefig('62718Kelvin.png')
#print('done')
