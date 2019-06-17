:
#
# Title:	stopease.sh
# Author:	Trevor van Bremen
# Date:		06-Jul-90
# Purpose:	Attempts to stop EaseSpool even if printers are "hung".
#
#	to cater for version 1 and 2 EaseSpool

#	Note:
#	This script uses LPDIR Ease_sched
#	These should have been defined in /etc/init.d/EaseSpool
#	which is the script that calls stopease.sh
#
# Firstly, run lpstop in background.
# Then wait a little while.
$LPDIR/lpstop &
sleep 5
#
# Next, check if any jobs haven't terminated yet (jobs = lpprint)
# If so, try a kill -15 (Theoretically, a gracefull termination)
#
hung_jobs=`ps -e | grep lpprint | sed -e 's/^  *//' -e s'/ .*//'`
if [ ! ".${hung_jobs}." = ".." ]
then
	for job_pid in ${hung_jobs}
	do
		echo "kill -15 ${job_pid}"
	done
fi
#
# Check if kill -15 worked...
# If not, assume that port is hung so warn user to cold-boot machine.
# Also, remove the named pipes associated with the print job.
# Then force EaseSpool to die by kill -9 of:
# 1. lpstop	(Also, remove it's associated named pipe)
# 2. lpschctl
# 3. lpsch
#
hung_jobs=`ps -e | grep lpprint | sed -e 's/^  *//' -e s'/ .*//'`
if [ ! ".${hung_jobs}." = ".." ]
then
	echo "Warning... The following print job PIDs appear to be hung"
	echo "Please cold-boot this machine before returning to multiuser-mode" 
	for job_pid in ${hung_jobs}
	do
		echo " ${job_pid}\c"
		if [ -p $LPDIR/${job_pid} ]
		then
			rm $LPDIR/${job_pid}
		fi
	done
	echo
	lpstop=`ps -e | grep lpstop | sed -e 's/^  *//' -e s'/ .*//'`
	for pid in ${lpstop}
	do
		kill -9 ${pid}
		if [ -p $LPDIR/${pid} ]
		then
			rm $LPDIR/${pid}
		fi
	done
	sleep 5
	lpschctl=`ps -e | grep lpschctl | sed -e 's/^  *//' -e s'/ .*//'`
	for pid in ${lpschctl}
	do
		kill -9 ${pid}
	done
	sleep 5
	lpsch=`ps -e | grep lpsch | sed -e 's/^  *//' -e s'/ .*//'`
	for pid in ${lpsch}
	do
		kill -9 ${pid}
	done
	sleep 5
	rm -f $Ease_Sched/schedcomm
fi
