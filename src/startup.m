#!/bin/sh
#
# /bin/sh replacement for startup, by Spectrum
# Update by Stephen Comoletti - 1/10/98.

# Make sure to edit following two lines for your correct path to area dir 
# and port number for the mud
cd ../area
port=6000
if [ "$1" != "" ]; then port=$1; fi
if [ -f ../data/shutdown.txt ]; then rm -f ../data/shutdown.txt; fi
ulimit -c 50000
ulimit -s 1500

# don't do this any more.
# renice +5 -p $$

DATE=`date`

# loop indefinately
while ( : ) do
  
  # make a backup copy of the original binary, since it might change, and we
  # might want to use it for gdb.
  cp ../src/merc ../src/merc.save

  # run the mud
  ../src/merc $port

  # shutdown?
  if [ -e ../data/shutdown.txt ]; then
#    echo "Main Port Shutdown on $DATE" | /usr/sbin/sendmail muffin@cs2.betterbox.net
    exit 1
  fi
  
  # sleep, so if we fail on boot we don't get massive looping
#  echo "Main Port Crashed on $DATE" | /usr/sbin/sendmail muffin@cs2.betterbox.net
  sleep 10
done
