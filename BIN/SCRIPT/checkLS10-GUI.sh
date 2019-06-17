#!/bin/sh
# ===========================================================================
# |	Startup/Shutdown Script For GVision Background Processes                |
# |                                                                         |
# | The script expects the environment variable PROG_PATH to be have been   |
# |set up correctly by /etc/init.d/GVision                                  |
# |                                                                         |
# ===========================================================================

###	=()<PROG_PATH=@<PROG_PATH>@>()=
PROG_PATH=/usr/LS10.5
export PROG_PATH

#------------------------------------------------------------------------
# Check for the existence of so_bgcalc
#------------------------------------------------------------------------
bg1=`ps -ef | grep SelSrvD | grep -v grep | wc -l`
if [ $bg1 -lt 1 ]
then
    echo "Checking SelSrvD NOT running, starting..." `date` >> $LOG
	LOGNAME="LS10-GUI-start"
	export	LOGNAME
	/bin/sh $PROG_PATH/BIN/.profile
fi

#------------------------------------------------------------------------
# Check for the existence of so_bgstkup
#------------------------------------------------------------------------
bg2=`ps -ef | grep PwdSrvD | grep -v grep | wc -l`
if [ $bg2 -lt 1 ]
then
    echo "Checking PwdSrvD NOT running, starting..." `date` >> $LOG
	LOGNAME="LS10-GUI-start"
	export	LOGNAME
	/bin/sh $PROG_PATH/BIN/.profile
fi
