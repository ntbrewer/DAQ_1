################################################################################
#
# FILE:    laser_config.py
# AUTHOR:  DVM
# CREATED: 2018-06-13
# UPDATED: 2018-06-14
#
# COMPLIMENTARY FILES: laser_config.txt
#
########################################
# DESCRIPTION:
#
# Script for reading laser_config.txt and setting the laser to the given
# configuration. Any changes to the actual settings need to be made in
# laser_config.txt
#
################################################################################

# OPEN CONFIGURATION FILE
with open('./laser_config.txt') as cfile:
    cflines = cfile.read().splitlines()

# SET UP ARRAYS
commands = []

# EXCLUDE ALL COMMENTED LINES
for line in cflines:
    if line == '':
        continue
    elif line[0] == '#':
        continue
    else:
        commands.append(line)

# IMPORT NEEDED MODULES
import gpib

# OPENING GPIB DEVICE (LASER)
herring = gpib.find("laser")

# SENDING COMMANDS TO DEVICE
for comm in commands:
    gpib.write(herring,comm)

# DISPLAY SETTINGS
print('========================')
print('      NEW SETTINGS      ')
print('========================')
for comm in commands:
    gpib.write(herring,comm.split()[0] + ' ' + comm.split()[1])
    print(str(gpib.read(herring,1024)).replace('b\'','').replace('\\r\'',''))
print('========================')

################################################################################
