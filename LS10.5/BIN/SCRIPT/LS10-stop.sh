:
#
# $Id: LS10-stop.sh,v 5.1 2001/11/29 01:03:08 scott Exp $
# $Program$
# Title:	Control Script for background processing start and stop.
# Author:	Scott B Darrow 
#	
#	Note:
#	This script uses PROG_PATH
#	These should have been defined in /etc/init.d/LS10Services
#	which is the script that calls stopLS10Services.sh

###	=()<PROG_PATH=@<PROG_PATH>@>()=
PROG_PATH=/usr/ver9.10
export PROG_PATH

# -----------------------------------------------------------------
# Read program name being run.
# -----------------------------------------------------------------
SCRIPT=`basename $0`

case ${SCRIPT} in

	# -----------------------------------------------------------------
	# Status - LS10 - GUI
	# -----------------------------------------------------------------
	LS10-GUI-status.sh)
		LOGNAME="LS10-GUI-status"
		export	LOGNAME
		/bin/sh $PROG_PATH/BIN/.profile
		;;

	# -----------------------------------------------------------------
	# Start - LS10 - GUI
	# -----------------------------------------------------------------
	LS10-GUI-start.sh)
		LOGNAME="LS10-GUI-start"
		export	LOGNAME
		/bin/sh $PROG_PATH/BIN/.profile
		;;

	LS10-GUI-stop.sh)
		LOGNAME="LS10-GUI-stop"
		export	LOGNAME
		/bin/sh $PROG_PATH/BIN/.profile
		;;

	LS10-stop.sh)
		LOGNAME="LS10-stop"
		export	LOGNAME
		/bin/sh $PROG_PATH/BIN/.profile
		;;

	LS10-start.sh)
		LOGNAME="LS10-start"
		export	LOGNAME
		/bin/sh $PROG_PATH/BIN/.profile
		;;

	*)
		;;

esac

