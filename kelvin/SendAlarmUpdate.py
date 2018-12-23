# SendAlarm
#
# Original Author:     NTB
# Contributing Author: DVM
# Updated:             2018-08-26
#
# Update Notes:
#    2018-08-26:
#        Updated to add offline evaluation opt writing output to 'TempEval.txt'
#        Moved print() to before sleep btw temp reads to correct timestamp.
#        Cleaned script. Updated definition and variable names for readability.
#    2018-09-10:
#        Modified to fit python code style guidelines.
#
# To Do:
#    Add functionality to send the log or a portion of the log with an alarm.
#


# IMPORTS ---------------------------------------------------------------------
import pexpect as pxp
import time
import sys
# -----------------------------------------------------------------------------


# FUNCTION DEFINITIONS --------------------------------------------------------
def getFileName():
    try:
        s = pxp.run('ls -lrt log/').split()[-1].decode()
    except IndexError:
        print('File not found: Input filename')
        s = input()
    if s != '':
        return('log/'+s)
    else:
        print('File name failed.')


def getLines(filename):
    """
    maybe add way to return max number of lines
        and check that request cannot be bigger?
    """
    inf = open(filename, "r")
    lines = inf.readlines()
    inf.close()
    return(lines)


def getAvg(nMinutes, lines):
    List = [0]*nMinutes*3
    for i in range(0, nMinutes):
        try:
            List[i] = eval(lines[i-nMinutes].split()[-1])
        except IndexError:
            print(lines, i-nMinutes)
        List[i+nMinutes] = eval(lines[i-nMinutes].split()[-2])
        List[i+2*nMinutes] = eval(lines[i-nMinutes].split()[-3])
    avg = sum(List)/nMinutes/3
    return(avg)


def getAvgOffline(offCount, nMinutes, fileName):
    inf = open(fileName, "r")
    lines = inf.readlines()

    List = [0]*nMinutes*3
    try:
        for i in range(0, nMinutes):
            List[i] = eval(lines[i+offCount].split()[-2])
            List[i+nMinutes] = eval(lines[i+offCount].split()[-3])
            List[i+2*nMinutes] = eval(lines[i+offCount].split()[-4])
        avg = sum(List)/nMinutes/3
        return(avg)
    except IndexError:
        print("---------------------------------------------------------")
        print("")
        print("File Complete")
        print("")
        print("---------------------------------------------------------")
        sys.exit()
# -----------------------------------------------------------------------------


# ONLINE / OFFLINE MODE -------------------------------------------------------
optLoop = 1

while optLoop == 1:
    online = input('Online or Offline: ')

    if online.lower() == 'online':
        online = True
        optLoop = 2
    elif online.lower() == 'offline':
        online = False
        optLoop = 2
    else:
        print('Error: Please type Online or offline.')

if online:
    while optLoop == 2:
        report = input('Send daily report? (Yes or No):')
        if report.lower() == 'yes':
            report = True
            optLoop = 0
        elif report.lower() == 'no':
            report = False
            optLoop = 0
        else:
            print('Error: Please type Yes or No.')

    fileName = getFileName()
    try:
        lines = getLines(fileName)
    except FileNotFoundError:
        print(fileName)
        fileName = input('Input filename:')
        lines = getLines(fileName)
else:
    fileName = input('Input filename:')
# -----------------------------------------------------------------------------

# USER VARIABLES --------------------------------------------------------------
nMinutes = 15      # number of minutes to wait between checks
thresh = 31        # temperature threshold
threshMin = 10     # minimum temperature threshold
dTthresh = 1.0     # 0.5 C per hour + 0.5 variance tolerance
# -----------------------------------------------------------------------------

# OTHER VARIABLES -------------------------------------------------------------
alarmCmd = './alarm.sh'    # script to email alarm
reportCmd = './report.sh'  # script to email daily report
plotCmd = './TempPlot.py'  # script to plot temperatures
err = 0                    # error flag
send = 0                   # send error flag
offCount = nMinutes        # test index counter
numSent = 0                # counter for messages sent
numTrip = 0                # counter for times alarm tripped
timeSec = 60 * nMinutes    # time in seconds
# timeSec = 0                # testing
# -----------------------------------------------------------------------------

# Initialization for Loop -----------------------------------------------------
if online:
    startTime = time.ctime()
    if report:
        timer = time.time()
    # Sleep, get 1st avg to initialize rAvgLast, sleep again, then start loop
    print('Run started ' + str(time.ctime()) + '.')
    print('Sleeping for ' + str(nMinutes+5) + ' minutes.')
    time.sleep(timeSec + (5*60))
    lines = getLines(fileName)
    rAvgLast = getAvg(nMinutes, lines)
    print('Initialization complete at ' + str(time.ctime()) + '.')
    print('Sleeping for ' + str(nMinutes) + ' minutes.')
    time.sleep(timeSec)
else:
    rAvgLast = getAvgOffline(offCount, nMinutes, fileName)
# -----------------------------------------------------------------------------

# Temperature Checking Loop ---------------------------------------------------
while err < 4:
    if online:
        if report and time.time() - timer > 86400: # 24h * 60m * 60s
            # need a try / except statement here for if the plot doesn't work?
            pxp.run(plotCmd)
            pxp.run(reportCmd)
            timer = time.time()

    if not online:
        ouf = open('TempEval.txt', 'a')
        offCount += nMinutes
        rAvg = getAvgOffline(offCount, nMinutes, fileName)
    else:
        ouf = open('kelvin.email.txt', 'w')
        lines = getLines(fileName)
        rAvg = getAvg(nMinutes, lines)

    dT = abs((rAvg - rAvgLast)*(60/nMinutes))

    if rAvg > thresh:
        send = 1
        err = 1

    if dT > dTthresh:
        send = 1
        err = 2

    if rAvg < threshMin:
        send = 1
        err = 3
        
    if err > 0:
        numTrip += 1

    if send == 1:
        if err == 1:
            errMessage = (
                "Temperature threshold ({} C) was crossed.".format(thresh))
        if err == 2:
            errMessage = (
                "Temperature rate of change threshold ({} C/hr) was crossed."
                .format(dTthresh))
        if err == 3:
            errMessage = (
                "Temperature has dropped below minimum threshold ({} C).".format(threshMin))
        if online:
            ouf.write("---------------------------------------------------\n" +
                      errMessage + "\n\n" +
                      "Average Temp at " + str(time.ctime()) +
                      ": {:.2f} C\n\n".format(rAvg) +
                      "Rate of Change in Avg Temp: {:.2f} C/hr\n".format(dT) +
                      "The alarm has tripped " + str(numTrip) +
                      " times since this run started on " +
                      str(startTime) + ".\n" 
                      "---------------------------------------------------\n")
            ouf.close()
            if numSent < 3 or numSent % 5 == 0:
                pxp.run(plotCmd)
                pxp.run(alarmCmd)
            numSent += 1
        else:
            ouf.write("\n" + errMessage.upper() + ":\n\n")

    if online:
        print("---------------------------------------------------")
        print("Average Temp at " + str(time.ctime()) +
              ": {:.2f} C".format(rAvg))
        print("Rate of Change in Average Temp: {:.2f} C/hr".format(dT))
        print("The alarm has tripped " + str(numTrip) +
              " times since this run started on " +
              str(startTime) + ".")
        print("---------------------------------------------------")
        time.sleep(timeSec)
    else:
        ouf.write("---------------------------------------------------\n" +
                  "Average Temp " + str(offCount/60) +
                  " hours after start: {:.2f} C\n\n".format(rAvg) +
                  "Rate of Change in Avg Temp: {:.2f} C/hr\n".format(dT) +
                  "The alarm has tripped " + str(numTrip) +
                  " times since this run started.\n" +
                  "---------------------------------------------------\n")

    send = 0
    err = 0
    rAvgLast = rAvg
# -----------------------------------------------------------------------------
