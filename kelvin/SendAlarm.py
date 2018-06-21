import pexpect as pxp
import time

def getlist():
    inf = open("log/therm-test.log",'r')
    lines = inf.readlines()
    List=[0,0,0,0,0]
    for i in range(0,5):
        List[i]=eval(lines[i-5].split()[-2])
    avg = sum(List)/5
    return(avg)

thresh = 31
dTthresh = 0.5 # 0.5 C per hour
dTime = 60 * 5
#cmd = "mutt -s \"MTAS is Warming up " + str(thresh) + "\" brewer.nathant@gmail.com < LNemail.txt"
#cmd = 'mutt -s \"MTAS is Warming up \" brewer.nathant@gmail.com < LNemail.txt '
cmd ='\.\/alarm.sh'

x=0

# Sleep, get first average to initialize avgLast, sleep again, then start loop
time.sleep(dTime)
avgLast = getlist()
time.sleep(dTime)

while x==0:
    avg=getlist()
    if avg>thresh:
        pxp.run(cmd)
        x=1
    dT = abs( (avg - avgLast)/(2 * dTime) )
    if dT > dTthresh:
        pxp.run(cmd)
        x = 1
    #pxp.time isn't a thing. Switched to time.sleep - DVM
    time.sleep(dTime)
    print("---------------------------------------------------------")
    print("Average Temp in last " + str(dTime) + " seconds: " + str(avg)+" C")
    print("Rate of Change in Average Temp: " + str(dT)+" C/hr")
    print("---------------------------------------------------------")
