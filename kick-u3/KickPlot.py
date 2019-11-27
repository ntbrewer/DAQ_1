#!/usr/bin/python3
# /bin/python3
# #####################################################
# This script makes plots of the mtc cycle
# written to the labjack outputs for control and monitoring. 
# This script can be used anywhere by changing the
# location of the python3 directive after #!.
# usage: ./KickPlot.py and use default
#    or: ./KickPlot.py file_name
# Based on TempPlot.py in kelvin
# N.T. BREWER 11-26-19
# ######################################################

# IMPORTS ---------------------------------------------------------------------
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import pexpect as pxp
import time
import numpy as np
import sys
#import binascii as ba
# -----------------------------------------------------------------------------


# DEFINITIONS -----------------------------------------------------------------
def getFileName():
    try:
        s = pxp.run('ls -rlt log/').split()[-1].decode()
        print(s)
    except IndexError:
        print('file not found: input filename')
        s = input()
    if s != '':
        return('log/'+s)
    else:
        file_name = 'log/kick-TueNov2610:58:522019.log'
        print('Warning, standard usage is: ./KickPlot.py file_name\n' +
              'using default file name:' + file_name)


def getLines(filename):
    # Maybe add way to return max number of lines
    # and check that request cannot be bigger?

    inf = open(filename, "r")
    lines = inf.readlines()
    inf.close()
    return(lines)
# -----------------------------------------------------------------------------


# READ IN FILE ----------------------------------------------------------------
try:
    file_name = sys.argv[1]
    # file_name = 'therm-TueAug1408:51:322018.log'
except IndexError:
    file_name = getFileName()

inf = open(file_name)
lines = inf.readlines()
blocks = []
# -----------------------------------------------------------------------------

# PULL DATE FROM FILE ---------------------------------------------------------
date = file_name.split('kick-')[-1].split('.log')[0]

if len(date.split(':')) and len(date.split(':')) < 3:
    date = input('date not found in file name. ' +
                 'Input date as datetime format %m%d%H%M%S%Y' +
                 'i.e. Sep. 9th 2015 at 11:11:03pm is 09092311032015: ')
    s = time.mktime(time.strptime(date, '%m%d%H%M%S%Y')) + 62135665200.0
    d = s/(86400)
else:
    if len(date.split(':')[0]) == 9:
        date = date[0:6] + '0' + date[6:]
    if len(date.split(':')[0]) == 9:
        date = date[0:6] + '0' + date[6:]
    s = time.mktime(time.strptime(date, '%a%b%d%H:%M:%S%Y')) + 62135665200.0
    # s = time.mktime(time.strptime(date, '%a%b%d%H_%M_%S%Y')) + 62135665200.0
    d = s/(86400)
    print(d)

print('start date ' + str(date))
# -----------------------------------------------------------------------------

# PLOT DATA POINTS ------------------------------------------------------------

# Displays plot if the line below is set to True
dispPlot = True
#dispPlot = False

# Plots just the last 24 hours if line below is set to True
#lastDay = False
lastDay = True

if len(lines) > 2000 and lastDay:
    for i in lines[3+len(lines)-2000:]:
        blocks.append(i.split('\t'))
else:
    for i in lines[3:]:
        blocks.append(i.split('\t'))

tdat = []
edat = []
cdat = []
fdat = []

for i in blocks:
    tdat.append(eval(i[0]))
    edat.append(list(bin(int(i[1],16))[2:].zfill(8)))
    cdat.append(list(bin(int(i[2],16))[2:].zfill(8)))
    fdat.append(list(bin(int(i[3],16))[2:].zfill(8)))

for i in range(0,len(edat)):
    for j in range(0,8):
        edat[i][j]=eval(edat[i][j])
        cdat[i][j]=eval(cdat[i][j])
        fdat[i][j]=eval(fdat[i][j])

earr = np.array(edat)
carr = np.array(cdat)
#fig = plt.figure()
#fig.autofmt_xdate()
legList = ['meas On','meas Off', 'bkg On', 'bkg Off', 'kick', 'tape', 'beam On', 'beam Off' , 'tape On', 'tape Off', 'lite On', 'lite Off']
ax = list(range(0,12))
#plt.subplots(12,1,sharex='all')
ax[0] = plt.subplot(12,1,1)

for i in range(0,8):
    ax[i] = plt.subplot(12,1,i+1,sharex=ax[0])
    plt.plot(tdat, earr[:,-(i+1)], drawstyle='steps-post')
    plt.plot(tdat, earr[:,-(i+1)], 'ko')
    plt.legend([legList[i]])
    plt.ylim(-.5,1.5)

for i in range(0,4):
    ax[i] = plt.subplot(12,1,i+9,sharex=ax[0])
    #plt.plot(tdat, earr[:,i], '-')
    plt.plot(tdat, carr[:,-(i+1)], drawstyle='steps-post')
    plt.plot(tdat, carr[:,-(i+1)], 'bo')
    plt.legend([legList[i+8]])
    plt.ylim(-.5,1.5)


plt.figure()
plt.plot(earr[:,-1],earr[:,-2],'ko')
plt.figure()
plt.plot(earr[:,-3],earr[:,-4],'bo')
plt.figure()
plt.plot(earr[:,-7],earr[:,-8],'ro')

plt.show()

plt.savefig('report.png')

if dispPlot:
    plt.show()

# print('done')
# -----------------------------------------------------------------------------
