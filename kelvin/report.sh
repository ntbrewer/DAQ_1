#!/bin/bash
# Emails are getting blocked to UTK addresses. May need to use different addresses.
mutt -s "MTAS Daily Temperature Report" "darren.mckinnon.13@gmail.com" -a "report.png" < report.email.txt
