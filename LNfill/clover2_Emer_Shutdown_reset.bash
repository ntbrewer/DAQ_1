#!/bin/bash
# CLOVER 1 CARDS
# Script for read and set channels in an MPOD crate
# modified from Wiener Manual Example 
# IGD 18.Apr.08
# IGD Modified for LeRIBSS commissioning 25.Jun.08
# Clover 1 is at the top of the cross and is detector D4
# option -Oqv suppresses "WIENER-CRATE-MIB::outputIndex.u0 = INTEGER:" eg for outputIndex
# options -q "quick print for easier parsing", -v "print values only" -U "dont print units"

ip=192.168.13.239
path=/usr/share/snmp/mibs
setVoltage=0
#setCurrent is in Amp!*!*!*!*! *** IF RAMPING DOWN SET STATUS TO OFF ***
#setCurrent=0.001
#setRamp=20
#Status 0 off 1 on DO NOT TURN ON IF ALREADY ON
setStatus=0
setClearShutdown=2
setClearEvents=10

channelCount=$(snmpget  -Oqv -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputNumber.0)
indices=$(snmpwalk -Oqv -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputIndex)
x=(`echo $indices \ tr''''`)

COUNTER=0

while [ $COUNTER -lt $channelCount ] 
do
	index=$(echo ${x[${COUNTER}]})

	if [ $index == u4 -o $index == u5 -o $index == u6 -o $index == u7 ]
	    then

	    #set parameters
	    voltage=$(snmpset -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputVoltage.$index F $setVoltage)
#	    iLimit=$(snmpset -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputCurrent.$index F $setCurrent)
#	    rampspeed=$(snmpset -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputVoltageRiseRate.$index F $setRamp)
	    clearShutdown=$(snmpset -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputSwitch.$index i $setClearShutdown)
	    clearEvents=$(snmpset -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputSwitch.$index i $setClearEvents)

	    # ON OFF PROTECTION TEST
	    status=$(snmpget -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c public $ip outputSwitch.$index)
	    if [ $setStatus -eq 1 -a $status == Off ]
		then
		status=$(snmpset -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputSwitch.$index i $setStatus)
		elif [ $setStatus -eq 0 -a $status == On ]
		then
		status=$(snmpset -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputSwitch.$index i $setStatus)
	    fi


            #after ramping down set channel  status back to off
	    #status=$(snmpset -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c guru $ip outputSwitch.$index i 0)

	
	#review settings
	voltage=$(snmpget -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c public $ip outputVoltage.$index)
	iLimit=$(snmpget -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c public $ip outputCurrent.$index)
	sense=$(snmpget -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c public $ip outputMeasurementSenseVoltage.$index)
	current=$(snmpget -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c public $ip outputMeasurementCurrent.$index)
	rampspeed=$(snmpget -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c public $ip outputVoltageRiseRate.$index)
	status=$(snmpget -OqvU -v 2c -M $path -m +WIENER-CRATE-MIB -c public $ip outputSwitch.$index)

	echo "$index $voltage $iLimit $sense $current $rampspeed $status"

	fi
	let COUNTER=COUNTER+1
	
done

