# coding: utf-8

import u3
ljh = u3.U3()
ljh.getCalibrationData()
get_ipython().run_line_magic('pinfo', 'ljh.getDIOState')
get_ipython().run_line_magic('pinfo', 'ljh.setDIOState')
get_ipython().run_line_magic('pinfo2', 'ljh.setDIOState')
ljh.setDIOState(8,1)
ljh.setDIOState(15,1)
ljh.setDIOState(8,0)
ljh.setDIOState(15,0)
ljh.setDIOState(8,1)
ljh.setDIOState(9,1)
ljh.setDIOState(10,1)
ljh.setDIOState(11,1)
ljh.setDIOState(12,1)
ljh.setDIOState(12,0)
ljh.setDIOState(14,1)
ljh.setDIOState(15,1)
ljh.setDIOState(16,1)
ljh.setDIOState(17,1)
ljh.setDIOState(18,1)
ljh.setDIOState(19,1)
DIOlist = [8,9,10,11,14,15,16,17,18,19]
def allOn():
    for i in DIOlist:
        ljh.setDIOState(i,1)
        
def allOff():
    for i in DIOlist:
        ljh.setDIOState(i,0)
        
allOff()
allOn()
allOff()
def listOn(inList):
    for i in inList:
        ljh.setDIOState(i,1)
        
def listOff(inList):
    for i in inList:
        ljh.setDIOState(i,0)
        
get_ipython().run_line_magic('pinfo', 'ljh.write')
get_ipython().run_line_magic('cd', '../')
get_ipython().run_line_magic('ls', '')
get_ipython().run_line_magic('cd', 'DAQ_1/')
get_ipython().run_line_magic('cd', 'include/')
get_ipython().run_line_magic('ls', '')
get_ipython().run_line_magic('less', 'labjackusb.h')
def listOff(inList):
    for i in inList:
        ljh.setDIOState(i,0)
        
get_ipython().run_line_magic('less', 'labjackusb.h')
get_ipython().run_line_magic('pinfo', 'ljh._writeToExodriver')
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,3])
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,3],[])
ljh.write([76,248,4,0,77,2,0,27,240,255,15,0,49,3])
allOn()
ljh = u3.U3()
ljh.getCalibrationData()
allOn()
allOff()
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,3],[])
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,0],[])
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,3],[])
allOff()
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,0],[])
ljh._writeToExodriver([76,248,4,0,77,0,0,27,240,255,15,0,49,0],[])
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,3],[])
allOff()
ljh._writeToExodriver([76,248,4,0,209,2,0,27,240,255,15,0,178,6],[])
ljh = u3.U3()
ljh = u3.U3()
ljh.getCalibrationData()
ljh._writeToExodriver([76,248,4,0,209,2,0,27,240,255,15,0,178,6],[])
ljh._writeToExodriver([76,248,4,0,209,2,0,27,240,255,15,0,178,6],[])
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,3],[])
allOff()
ljh._writeToExodriver([76,248,4,0,77,2,0,27,240,255,15,0,49,3],[])
allOff()
arrList = [[ 76, 248,4,0,77,2,0,27,240,255,15,0,49,3 ],
[ 208, 248,4,0,209,2,0,27,240,255,15,0,178,6 ],
[ 223, 248,4,0,224,2,0,27,240,255,15,0,192,7 ],
[ 253, 248,4,0,254,2,0,27,240,255,15,0,225,4 ],
[ 227, 248,4,0,228,2,0,27,240,255,15,0,134,69 ],
[ 6, 248,4,0,6,3,0,27,240,255,15,0,167,70 ],
[ 79, 248,4,0,79,3,0,27,240,255,15,0,242,68 ],
[ 83, 248,4,0,83,3,0,27,240,255,15,0,243,71 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,163,5 ],
[ 189, 248,4,0,190,2,0,27,240,255,15,0,164,1 ],
[ 159, 248,4,0,160,2,0,27,240,255,15,0,133,2 ],
[ 69, 248,4,0,70,2,0,27,240,255,15,0,38,7 ],
[ 69, 248,4,0,70,2,0,27,240,255,15,0,43,2 ],
[ 74, 248,4,0,75,2,0,27,240,255,15,0,45,5 ],
[ 107, 248,4,0,108,2,0,27,240,255,15,0,79,4 ],
[ 42, 248,4,0,43,2,0,27,240,255,15,0,12,6 ],
[ 72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[ 72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 160, 248,4,0,161,2,0,27,240,255,15,0,68,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ]]
arrList = [[ 76, 248,4,0,77,2,0,27,240,255,15,0,49,3 ],
[ 208, 248,4,0,209,2,0,27,240,255,15,0,178,6 ],
[ 223, 248,4,0,224,2,0,27,240,255,15,0,192,7 ],
[ 253, 248,4,0,254,2,0,27,240,255,15,0,225,4 ],
[ 227, 248,4,0,228,2,0,27,240,255,15,0,134,69 ],
[ 6, 248,4,0,6,3,0,27,240,255,15,0,167,70 ],
[ 79, 248,4,0,79,3,0,27,240,255,15,0,242,68 ],
[ 83, 248,4,0,83,3,0,27,240,255,15,0,243,71 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,163,5 ],
[ 189, 248,4,0,190,2,0,27,240,255,15,0,164,1 ],
[ 159, 248,4,0,160,2,0,27,240,255,15,0,133,2 ],
[ 69, 248,4,0,70,2,0,27,240,255,15,0,38,7 ],
[ 69, 248,4,0,70,2,0,27,240,255,15,0,43,2 ],
[ 74, 248,4,0,75,2,0,27,240,255,15,0,45,5 ],
[ 107, 248,4,0,108,2,0,27,240,255,15,0,79,4 ],
[ 42, 248,4,0,43,2,0,27,240,255,15,0,12,6 ],
[ 72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[ 72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 160, 248,4,0,161,2,0,27,240,255,15,0,68,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ]]
arrList[0]
ljh = u3.U3()
ljh = u3.U3()
ljh.getCalibrationData()
ljh._writeToExodriver(arrList[0],[])
ljh._writeToExodriver(arrList[1],[])
ljh._writeToExodriver(arrList[2],[])
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    input('ok?')
    
len(arrList)
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    input('ok?')
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    input('ok?')
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    
allOff()
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    
allOff()
allOn()
allOff()
get_ipython().run_line_magic('save', "'../kick-u3/manualSession.py' '0-88'")
get_ipython().run_line_magic('less', '../kick-u3/manualSession.py')
npa = np.array(arrList))
npa = np.array(arrList)
npa[:,0]
plt.plot(npa[:,0],npa[:,-2])
plt.plot(npa[:,0],npa[:,-1])
plt.plot(npa[:,0],npa[:,-1]+npa[:,-1])
plt.plot(npa[:,0],npa[:,-1]+npa[:,-2])
plt.cla()
plt.plot(npa[:,0],npa[:,-1]+npa[:,-2])
plt.plot(npa[:,0],npa[:,-1]*npa[:,-2])
plt.plot(npa[:,0],npa[:,-1]+npa[:,-2])
plt.plot(npa[:,0],npa[:,-1]+npa[:,-2])
plt.cla()
plt.plot(npa[:,0],npa[:,-1]+npa[:,-2])
49+3
49+3-22
76/(49+3-22)
2.5*(49+3-22)
2.5*(49+3-21)
2.533*(49+3-22)
2.5333*(49+3-22)
2.5333*(178+6-22)
2.5333*(192+7-22)
res = []
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    a = input('ok?')
    res.append(a)
    
npa[res]
res
[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,20,21,22,23]
res2 = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,20,21,22,23]
npa[res2]
npa[res2]
for i in range(0,len(npa2)):
    ljh._writeToExodriver(arrList[npa2[i]],[])
    a = input('ok?')
    
for i in range(0,len(res2)):
    ljh._writeToExodriver(arrList[res2[i]],[])
    a = input('ok?')
    
allOff()
res2
ljh._writeToExodriver(arrList[16],[])
ljh._writeToExodriver(arrList[17],[])
ljh._writeToExodriver(arrList[18],[])
ljh._writeToExodriver(arrList[19],[])
ljh._writeToExodriver(arrList[20],[])
ljh._writeToExodriver(arrList[0],[])
ljh._writeToExodriver(arrList[18],[])
allOff()
res2
allList2 = [[76, 248,4,0,77,2,0,27,240,255,15,0,49,3 ],
[208, 248,4,0,209,2,0,27,240,255,15,0,178,6 ],
[223, 248,4,0,224,2,0,27,240,255,15,0,192,7 ],
[253, 248,4,0,254,2,0,27,240,255,15,0,225,4 ],
[91, 248,4,0,92,2,0,27,240,255,15,0,66,1 ],
[125, 248,4,0,126,2,0,27,240,255,15,0,99,2 ],
[198, 248,4,0,199,2,0,27,240,255,15,0,174,0 ],
[202, 248,4,0,203,2,0,27,240,255,15,0,175,3 ],
[192, 248,4,0,193,2,0,27,240,255,15,0,163,5 ],
[189, 248,4,0,190,2,0,27,240,255,15,0,164,1 ],
[159, 248,4,0,160,2,0,27,240,255,15,0,133,2 ],
[69, 248,4,0,70,2,0,27,240,255,15,0,38,7 ],
[69, 248,4,0,70,2,0,27,240,255,15,0,43,2 ],
[74, 248,4,0,75,2,0,27,240,255,15,0,45,5 ],
[107, 248,4,0,108,2,0,27,240,255,15,0,79,4 ],
[42, 248,4,0,43,2,0,27,240,255,15,0,12,6 ],
[72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ]]
for i in range(0,len(arrList2)):
    ljh._writeToExodriver(arrList2[i],[])
    
arrList2 = [[76, 248,4,0,77,2,0,27,240,255,15,0,49,3 ],
[208, 248,4,0,209,2,0,27,240,255,15,0,178,6 ],
[223, 248,4,0,224,2,0,27,240,255,15,0,192,7 ],
[253, 248,4,0,254,2,0,27,240,255,15,0,225,4 ],
[91, 248,4,0,92,2,0,27,240,255,15,0,66,1 ],
[125, 248,4,0,126,2,0,27,240,255,15,0,99,2 ],
[198, 248,4,0,199,2,0,27,240,255,15,0,174,0 ],
[202, 248,4,0,203,2,0,27,240,255,15,0,175,3 ],
[192, 248,4,0,193,2,0,27,240,255,15,0,163,5 ],
[189, 248,4,0,190,2,0,27,240,255,15,0,164,1 ],
[159, 248,4,0,160,2,0,27,240,255,15,0,133,2 ],
[69, 248,4,0,70,2,0,27,240,255,15,0,38,7 ],
[69, 248,4,0,70,2,0,27,240,255,15,0,43,2 ],
[74, 248,4,0,75,2,0,27,240,255,15,0,45,5 ],
[107, 248,4,0,108,2,0,27,240,255,15,0,79,4 ],
[42, 248,4,0,43,2,0,27,240,255,15,0,12,6 ],
[72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ]]
for i in range(0,len(arrList2)):
    ljh._writeToExodriver(arrList2[i],[])
    
ljh = u3.U3()
ljh.getCalibrationData()
for i in range(0,len(arrList2)):
    ljh._writeToExodriver(arrList2[i],[])
    
allOff()
allOff()
for i in range(0,len(arrList2)):
    ljh._writeToExodriver(arrList2[i],[])
    input('ok?')
    
for i in range(0,len(arrList2)):
    ljh._writeToExodriver(arrList2[i],[])
    input('ok?')
    

get_ipython().run_line_magic('cd', 'Programs/DAQ_1/kick-u3/')
get_ipython().run_line_magic('ls', '')
import u3
ljh = u3.U3()
ljh.getCalibrationData()
arrList3 = [[166, 248,5,0,166,2,0,27,240,255,15,0,7,4,5,125 ],
[75, 248,5,0,74,3,0,27,240,255,15,0,168,7,5,125 ],
[73, 248,5,0,72,3,0,27,240,255,15,0,169,4,5,125 ],
[166, 248,5,0,166,2,0,27,240,255,15,0,10,1,5,125 ],
[155, 248,5,0,155,2,0,27,240,255,15,0,0,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ]]
ljh._writeToExodriver(arrList3[0],[])
def allOff():
    for i in DIOlist:
        ljh.setDIOState(i,0)
        
allOff()
def listOff(inList):
    for i in inList:
        ljh.setDIOState(i,0)
        
DIOlist = [8,9,10,11,14,15,16,17,18,19]
def allOff():
    for i in range(0,32):
        ljh.setDIOState(i,0)
        
allOff()
allOff()
def allOff():
    for i in range(0,24):
        ljh.setDIOState(i,0)
        
allOff()
def allOff():
    for i in range(0,20):
        ljh.setDIOState(i,0)
        
allOff()
ljh._writeToExodriver(arrList3[1],[])
ljh._writeToExodriver(arrList3[0],[])
ljh._writeToExodriver(arrList3[1],[])
ljh._writeToExodriver(arrList3[2],[])
ljh._writeToExodriver(arrList3[3],[])
ljh._writeToExodriver(arrList3[4],[])
ljh._writeToExodriver(arrList3[5],[])
ljh._writeToExodriver(arrList3[6],[])
ljh._writeToExodriver(arrList3[7],[])
allOff()
ljh.close()
ljh = u3.U3()
ljh.getCalibrationData()
ljh._writeToExodriver(arrList3[0],[])
ljh._writeToExodriver(arrList3[1],[])
arrList = [[ 76, 248,4,0,77,2,0,27,240,255,15,0,49,3 ],
[ 208, 248,4,0,209,2,0,27,240,255,15,0,178,6 ],
[ 223, 248,4,0,224,2,0,27,240,255,15,0,192,7 ],
[ 253, 248,4,0,254,2,0,27,240,255,15,0,225,4 ],
[ 227, 248,4,0,228,2,0,27,240,255,15,0,134,69 ],
[ 6, 248,4,0,6,3,0,27,240,255,15,0,167,70 ],
[ 79, 248,4,0,79,3,0,27,240,255,15,0,242,68 ],
[ 83, 248,4,0,83,3,0,27,240,255,15,0,243,71 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,163,5 ],
[ 189, 248,4,0,190,2,0,27,240,255,15,0,164,1 ],
[ 159, 248,4,0,160,2,0,27,240,255,15,0,133,2 ],
[ 69, 248,4,0,70,2,0,27,240,255,15,0,38,7 ],
[ 69, 248,4,0,70,2,0,27,240,255,15,0,43,2 ],
[ 74, 248,4,0,75,2,0,27,240,255,15,0,45,5 ],
[ 107, 248,4,0,108,2,0,27,240,255,15,0,79,4 ],
[ 42, 248,4,0,43,2,0,27,240,255,15,0,12,6 ],
[ 72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[ 72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 160, 248,4,0,161,2,0,27,240,255,15,0,68,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 192, 248,4,0,193,2,0,27,240,255,15,0,100,68 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[ 24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ]]
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    input('ok?')
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    input('ok?')
    
ljh = u3.U3()
ljh.getCalibrationData()
allOff()
ljh = u3.U3()
ljh.getCalibrationData()
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    sleep(10)
    
allOff()
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    time.sleep(10)
    
allOff()
import time
time.sleep(10)
time.sleep(1)
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    time.sleep(10)
    
time.sleep(.1)
time.sleep(.01)
time.sleep(.001)
allOff()
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    time.sleep(.1)
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    time.sleep(.1)
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    time.sleep(.01)
    
for i in range(0,len(arrList)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList[i+len(arrList)//2],[])
        time.sleep(2)
    
for i in range(0,len(arrList)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList[i+len(arrList)//2],[])
        time.sleep(2)
    
for i in range(0,len(arrList)):
    ljh._writeToExodriver(arrList[i],[])
    time.sleep(.5)
    
ljh._writeToExodriver(arrList[15],[])
ljh._writeToExodriver(arrList[16],[])
ljh._writeToExodriver(arrList[14],[])
ljh._writeToExodriver(arrList[17],[])
ljh._writeToExodriver(arrList[18],[])
ljh._writeToExodriver(arrList[19],[])
arrList2 = [[76, 248,4,0,77,2,0,27,240,255,15,0,49,3 ],
[208, 248,4,0,209,2,0,27,240,255,15,0,178,6 ],
[223, 248,4,0,224,2,0,27,240,255,15,0,192,7 ],
[253, 248,4,0,254,2,0,27,240,255,15,0,225,4 ],
[91, 248,4,0,92,2,0,27,240,255,15,0,66,1 ],
[125, 248,4,0,126,2,0,27,240,255,15,0,99,2 ],
[198, 248,4,0,199,2,0,27,240,255,15,0,174,0 ],
[202, 248,4,0,203,2,0,27,240,255,15,0,175,3 ],
[192, 248,4,0,193,2,0,27,240,255,15,0,163,5 ],
[189, 248,4,0,190,2,0,27,240,255,15,0,164,1 ],
[159, 248,4,0,160,2,0,27,240,255,15,0,133,2 ],
[69, 248,4,0,70,2,0,27,240,255,15,0,38,7 ],
[69, 248,4,0,70,2,0,27,240,255,15,0,43,2 ],
[74, 248,4,0,75,2,0,27,240,255,15,0,45,5 ],
[107, 248,4,0,108,2,0,27,240,255,15,0,79,4 ],
[42, 248,4,0,43,2,0,27,240,255,15,0,12,6 ],
[72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[72, 248,4,0,73,2,0,27,240,255,15,0,48,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[56, 248,4,0,57,2,0,27,240,255,15,0,32,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ],
[24, 248,4,0,25,2,0,27,240,255,15,0,0,0 ]]
for i in range(0,len(arrList2)):
    ljh._writeToExodriver(arrList2[i],[])
    time.sleep(.5)
    
for i in range(0,len(arrList2)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList2[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList2[i+len(arrList2)//2],[])
        time.sleep(2)
    
allOff()
for i in range(0,len(arrList2)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList2[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList2[i+len(arrList2)//2],[])
        time.sleep(2)
    
for i in range(0,len(arrLis3)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList3[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList3[i+len(arrList3)//2],[])
        time.sleep(2)
    
for i in range(0,len(arrList3)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList3[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList3[i+len(arrList3)//2],[])
        time.sleep(2)
    
for i in range(0,len(arrList3)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList3[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList3[i+len(arrList3)//2],[])
        time.sleep(2)
    
for i in range(0,len(arrList3)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList3[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList3[i+len(arrList3)//2],[])
        time.sleep(2)
    
allOff()
allOff()
arrList3 = [[166, 248,5,0,166,2,0,27,240,255,15,0,7,4,5,125 ],
[75, 248,5,0,74,3,0,27,240,255,15,0,168,7,5,125 ],
[73, 248,5,0,72,3,0,27,240,255,15,0,169,4,5,125 ],
[166, 248,5,0,166,2,0,27,240,255,15,0,10,1,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ]]
arrList3 = [[166, 248,5,0,166,2,0,27,240,255,15,0,7,4,5,125 ],
[75, 248,5,0,74,3,0,27,240,255,15,0,168,7,5,125 ],
[73, 248,5,0,72,3,0,27,240,255,15,0,169,4,5,125 ],
[166, 248,5,0,166,2,0,27,240,255,15,0,10,1,5,125 ],
[155, 248,5,0,155,2,0,27,240,255,15,0,0,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ],
[187, 248,5,0,187,2,0,27,240,255,15,0,32,0,5,125 ]]
for i in range(0,len(arrList3)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList3[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList3[i+len(arrList3)//2],[])
        time.sleep(2)
    
allOff()
for i in range(0,len(arrList3)//2):
    if i%2 == 0:
        ljh._writeToExodriver(arrList3[i],[])
        time.sleep(.01)
    else:
        ljh._writeToExodriver(arrList3[i+len(arrList3)//2],[])
        time.sleep(2)
    
allOff()
allOff()
