import pexpect as pxp
import time

def getList():
    inf = open("log/therm-test.log",'r')
    lines = inf.readlines()
    List=[0,0,0,0,0]
    for i in range(0,5):
        List[i]=eval(lines[i-5].split()[-2])
    avg = sum(List)/5
    return(avg)

def getListTest(j,nMinutes):
    inf = open("alarmTest1.txt","r")
    lines = inf.readlines()

    # "dynamic" length for list based on nMinutes. Better way?
    List = [0]
    for i in range(1,nMinutes):
        List.append(0)

    for i in range(0,nMinutes):
        List[i] = eval(lines[i+j].split()[-2])
    avg = sum(List)/nMinutes
    #print(List)
    return(avg)

def rateChangeTest(j,nMinutes):
    inf = open("alarmTest1.txt","r")
    lines = inf.readlines()

    # "dynamic" length for list based on nMinutes. Better way?
    List1 = [0]
    List2 = [0]
    List3 = [0]
    runAvg = [0]
    ListTime = [0] # added for testing
    for i in range(1,nMinutes):
        List1.append(0)
        List2.append(0)
        List3.append(0)
        runAvg.append(0)
        ListTime.append(0) # added for testing
    #print(List1)

    for i in range(0,nMinutes):
        List1[i] = eval(lines[i+j].split()[-2])
        List2[i] = eval(lines[i+j].split()[-3])
        List3[i] = eval(lines[i+j].split()[-4])
        ListTime[i] = eval(lines[i+j].split()[-5])
        #print(str(ListTime[i])+"   "+str(List1[i])+"   "+str(List2[i])+"   "+str(List3[i])+"\n" )
    for i in range(0,nMinutes):
        runAvg[i] = (List1[i]+List2[i]+List3[i])/3
    avg = sum(runAvg)/nMinutes
    #print(avg)
    return(avg)

nMinutes = 60    # number of minutes to wait between checks
thresh   = 31    # temperature threshold
dTthresh = 0.5   # 0.5 C per hour
#dTime    = 60 * nMinutes # "dynamic" time in seconds
dTime    = .001  #test time

#cmd = "mutt -s \"MTAS is Warming up " + str(thresh) + "\" brewer.nathant@gmail.com < LNemail.txt"
#cmd = 'mutt -s \"MTAS is Warming up \" brewer.nathant@gmail.com < LNemail.txt '
cmd ='\.\/alarm.sh'

ouf  = open('kelvin.email.txt','w') #open file for writing 
err  = 0   #error flag
send = 0   #send error flag
j    = 3   #test index counter - not used during running

# Sleep, get first average to initialize rAvgLast, sleep again, then start loop
time.sleep(dTime)
#avgLast = getList()
rAvgLast = rateChangeTest(j,nMinutes)
time.sleep(dTime)

while err==0:
    j += 5
    #avg=getList()
    avg = getListTest(j,nMinutes)
    rAvg = rateChangeTest(j,nMinutes)
    #dT = abs( (avg - avgLast)*(60/dTime) )
    #dT = abs( (avg - avgLast)*(60/5) )
    dT = abs( (rAvg - rAvgLast)*(60/nMinutes) )
    #print("avg, rAvg, and rAvgLast: " + str(avg) + "    " + str(rAvg) + "   " + str(rAvgLast))

    if avg>thresh:
        send = 1
        err = 1

    if dT > dTthresh:
        send = 1
        err = 2

    if send == 1:
        if err == 1:
            errMessage = "Temperature threshold ({} C) was crossed.".format(thresh)
        if err == 2:
            errMessage = "Temperature rate of change threshold ({} C/hr) was crossed.".format(dTthresh)
        ouf.write("-------------------------------------------------------------------\n" +
                  errMessage + "\n\n" +
                  "Average Temp at " + str(time.ctime()) + ": {:.2f} C\n\n".format(avg) +
                  "Rate of Change in Average Temp: {:.2f} C/hr\n".format(dT) +
                  "-------------------------------------------------------------------\n")
        ouf.close()
        pxp.run(cmd)
        send = 0
    time.sleep(dTime)
    #avgLast = avg
    rAvgLast = rAvg
    print("---------------------------------------------------------")
    print("Average Temp at " + str(time.ctime()) + ": {:.2f} C".format(avg))
    print("Rate of Change in Average Temp: {:.2f} C/hr".format(dT))
    print("---------------------------------------------------------")
