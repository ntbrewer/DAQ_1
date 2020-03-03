#!/bin/bash
# Emails are getting blocked to UTK addresses. May need to use different addresses.
mutt -s "MTAS Daily Temperature Report" "brewer.nathant@gmail.com" -a "report.png" < report.email.txt
