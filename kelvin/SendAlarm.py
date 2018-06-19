import pexpect as pxp

def getlist():
    inf = open("log/therm-MonJul2409:22:182017.log",'r')
    lines = inf.readlines()
    List=[0,0,0,0,0]
    for i in range(0,5):
        List[i]=eval(lines[i-5].split()[-2])
    avg = sum(List)/5
    return(avg)

thresh = 31
#cmd = "mutt -s \"MTAS is Warming up " + str(thresh) + "\" brewer.nathant@gmail.com < LNemail.txt"
#cmd = 'mutt -s \"MTAS is Warming up \" brewer.nathant@gmail.com < LNemail.txt '
cmd ='\.\/alarm.sh'
x=0
while x==0:
   avg=getlist() 
   if avg>thresh:
      pxp.run(cmd)
      x=1
   pxp.time.sleep(3*60)
   print(str(avg))
