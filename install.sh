#!/bin/bash
apt-get install zip

OUTFILE=hygterm
g++ -o $OUTFILE hygterm.cpp -L/usr/local/lib -lwiringPi

RETVAL=$?
if [ $RETVAL -eq 0 ]
then
        # Make executable
	chmod 775 $OUTFILE
        # Copy to bin 
        cp $OUTFILE /usr/local/bin
        # Setup cron
        cp hygterm.cron /etc/cron.d/hygterm
        exit 0
else
	echo "Build failed."
        exit 1
fi
