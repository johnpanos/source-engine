# Runs a given process with a specified timeout.
# BAT? No. Perl? No. Python? Yes. Good god, FINALLY!!!
#
# %1  - timeout (run the app for AT MOST this many seconds)
# %2  - app to launch
# %3+ - command-line args for the app, if any

import subprocess
import string
import time
import sys
import os


timeout = float( sys.argv[1] )
command = string.join( sys.argv[2:], " " )
# print str( timeout ) + ", " + command
p = subprocess.Popen( command )

fin_time = time.time() + timeout
while p.poll() == None and fin_time > time.time():
	time.sleep(1)
if fin_time < time.time():
	# ask nicely first (give the app 3 seconds to close normally)
	os.popen( 'TASKKILL /PID ' + str(p.pid) )
	time.sleep(3)
	# now make absolutely sure it's dead
	os.popen( 'TASKKILL /PID ' + str(p.pid) + ' /F' )
# else:
#	print p.poll()
