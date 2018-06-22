#!/bin/bash
# Emails are getting blocked to UTK addresses. May need to use different addresses.
#mutt -s "MTAS is warming over 31C" brewer.nathant@gmail.com -c brasco.utk.edu -c rykaczewskik@ornl.gov -c dmckinno@vols.utk.edu < KelvinEmail.txt
#mutt -s "MTAS is warming over 31C" brewer.nathant@gmail.com < KelvinEmail.txt
#mutt -s "MTAS is warming over 31C" dmckinno@vols.utk.edu -c darren.mckinnon.13@gmail.com < KelvinEmail.txt
mutt -s "MTAS ALARM TESTING" darren.mckinnon.13@gmail.com < kelvin.email.txt
