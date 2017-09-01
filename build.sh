#!/bin/bash

OUTFILE=hygterm
g++ -o $OUTFILE hygterm.cpp -L/usr/local/lib -lwiringPi

RETVAL=$?
if [ $RETVAL -eq 0 ]
then
	# Make executable
	chmod 775 $OUTFILE
  exit 0
else
	echo "Build failed."
  exit 1
fi
