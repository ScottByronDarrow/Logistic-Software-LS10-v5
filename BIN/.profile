#====================================================================
#  Copyright (C) 1999 - 2000 LogisticSoftware                        
#====================================================================
# $Id: profile,v 4.3 2002/01/21 04:49:01 scott Exp $
# $Source: /usr/LS10/REPOSITORY/LS10.5/INSTALL/info/profile,v $
#---------------------------------------------------------------
# $Log: profile,v $
# Revision 4.3  2002/01/21 04:49:01  scott
# Updated to ensure spooler path before standard path.
#
# Revision 4.2  2001/09/12 03:21:32  scott
# Updated from Scott machine - 12th Sep 2001
#
# Revision 4.1  2001/08/07 00:08:30  scott
# RELEASE 5.0
#
# Revision 4.0  2001/03/09 02:49:26  scott
# LS10-4.0 New Release as at 10th March 2001
#
# Revision 1.3  2001/01/17 07:45:52  scott
# Updated from testing
#
# Revision 1.2  2001/01/15 08:16:59  scott
# New linked Scripts
#
# Revision 1.1  2001/01/12 07:20:35  scott
# New install system
#
# profile is a multi-purpose script that sets correct environment for 
# normal users as well as special scripts.
#		night)				-	Login for night processing.
#		weekly|friday)		-	Login for weekly and friday processing
#		month|monthly)		-	Login for weekly and friday processing
#		support)			- 	Login for Logistic Support Staff.
#		support)			- 	Login for Logistic Support Staff.
#		LS10-GUI-start)		-	Login and start LS10-GUI processes
#		LS10-GUI-stop)		-	Login and stop LS10-GUI process
#		LS10-GUI-status)	-	Login and run status for LS10 processes
#		LS10-start)			-	Login and start LS10 processes
#		LS10-stop)			-	Login and stop LS10 processes
#---------------------------------------------------------------

#---------------------------------------------------------------
# Standard UNIX flags required. 
#---------------------------------------------------------------
umask	000
EXINIT='set shell=/bin/sh'

trap '' 1

#---------------------------------------------------------------
# PROG_PATH required to tell LS10 where system is loaded into.
#---------------------------------------------------------------
###	=()<PROG_PATH=@<PROG_PATH>@>()=
PROG_PATH=/usr/LS10.5

#---------------------------------------------------------------
# PROG_PATH required to tell LS10 where system is loaded into.
#---------------------------------------------------------------
###	=()<PROG_PATH=@<PROG_PATH>@>()=
PROG_PATH=/usr/LS10.5
#---------------------------------------------------------------
# Setup normal LS10 complete path.
#---------------------------------------------------------------
PB=$PROG_PATH/BIN
LSL_PATH=$PB/LS10-GUI:$PB/AS:$PB/BM:$PB/CA:$PB/CM:$PB/CR:$PB/DB:$PB/DD:$PB/FA:$PB/FE:$PB/FF:$PB/GL:$PB/MAIL:$PB/MENU:$PB/MH:$PB/MISC:$PB/OL:$PB/PC:$PB/PM:$PB/PO:$PB/POS:$PB/PS:$PB/QC:$PB/QT:$PB/RG:$PB/SA:$PB/SJ:$PB/SK:$PB/SO:$PB/SQL:$PB/TM:$PB/TR:$PB/TS:$PB/UTILS:$PB/LRP:$PB/SCRIPT

LS10_SERVER_DIR=$PB/LS10-GUI
export	LS10_SERVER_DIR

#---------------------------------------------------------------
# System Path. 
#---------------------------------------------------------------
###	=()<PATH=$LSL_PATH:@<LPDIR>@:@<STDBIN>@:@<DATABASEBIN>@>()=
PATH=$LSL_PATH:/usr/Ease/EaseSpool:/usr/local/bin:/bin:/etc:/usr/bin:/tcb/bin:/usr/local/samba/bin:/usr/informix7/bin

export	PATH PROG_PATH

#---------------------------------------------------------------
# Set up Clear screen, Bold on and off.
#---------------------------------------------------------------
CLEAR=`tput clear`

#---------------------------------------------------------------
# System down for maintenance. 
#---------------------------------------------------------------
SystemDown ()
{
	echo $CLEAR
	echo "+--------------------------------------------------+"
	echo "|         System currently not available           |"
	echo "|   as system maintenance is being carried out.    |"
	echo "|     Please check with system administrator       |"
	echo "|              or try again later                  |"
	echo "+--------------------------------------------------+"
	sleep 2
	exit
}

#	SystemDown

#---------------------------------------------------------------
# Change directory to required place to be sure. 
#---------------------------------------------------------------
cd $PROG_PATH/BIN

#---------------------------------------------------------------
# Set up LSL Console Port name. 
#---------------------------------------------------------------
###	=()<CONSOLE_TTY=@<CONSOLE_TTY>@>()=
CONSOLE_TTY=/dev/tty0

#---------------------------------------------------------------
# LS10 environment variables file.
#---------------------------------------------------------------
###	=()<PSL_ENV_NAME=@<PROG_PATH>@/@<ENVIRON_NAME>@>()=
PSL_ENV_NAME=/usr/LS10.5/BIN/LOGISTIC
export PSL_ENV_NAME

#---------------------------------------------------------------
# WP_USED when word processing package is required within LS10. 
#---------------------------------------------------------------
###	=()<WP_USED=@<WP_USED>@>()=
WP_USED=vi

#---------------------------------------------------------------
# COPYRIGHT LSL copyright.
#---------------------------------------------------------------
COPYRIGHT="Logistic Software Limited."

#---------------------------------------------------------------
# MAIL_USED LSL mail = psl_mail, Unix mail = mail
#---------------------------------------------------------------
###	=()<MAIL_USED=@<MAIL_USED>@>()=
MAIL_USED=psl_mail

#---------------------------------------------------------------
# FILE_PERM - Defines file permishion flags used by UNI. 
#---------------------------------------------------------------
###	=()<FILE_PERM="@<OWNER>@ @<GROUP>@ 777">()=
FILE_PERM="lsl develop	 777"

#---------------------------------------------------------------
# ADJ_MONEY - Adjustment required for exclusion of currency.
#---------------------------------------------------------------
###	=()<ADJ_MONEY="@<ADJ_MONEY>@">()=
ADJ_MONEY="0"

#---------------------------------------------------------------
# DEBUG - Turns on debug for development.   
#---------------------------------------------------------------
DEBUG=OFF

#---------------------------------------------------------------
# BUDGET - Used for debug within development. 
#---------------------------------------------------------------
BUDGET=0

#---------------------------------------------------------------
# Spread sheet to General ledger interface variables.
#---------------------------------------------------------------
SPREAD=$PROG_PATH/SPREAD
###	=()<SPREAD_SHEET=@<SPREAD_SHEET>@>()=
SPREAD_SHEET=2020
###	=()<SPREAD_EDIT=@<SPREAD_EDIT>@>()=
SPREAD_EDIT=vi                                    

#---------------------------------------------------------------
# Date format required. 
#---------------------------------------------------------------
###	=()<DBDATE=@<DBDATE>@>()=
DBDATE=DMY4

#---------------------------------------------------------------
# Lock Directory.
#---------------------------------------------------------------
###	=()<LOCK_DIR=@<PROG_PATH>@:@<LOCK_DIR>@>()=
LOCK_DIR=/usr/LS10.5/BIN/LOCK/

#---------------------------------------------------------------
# LS10 database type.
#---------------------------------------------------------------
###	=()<LS10_DB=@<LS10_DB>@>()=
LS10_DB=ORACLE	
export	LS10_DB

#---------------------------------------------------------------
# Define as 0 of double byte not required, 1 of require.
#---------------------------------------------------------------
###	=()<OUTRANGE_CHAR=@<OUTRANGE_CHAR>@>()=
OUTRANGE_CHAR=0

#---------------------------------------------------------------
# Define as 0 if multiple languages not required.
#---------------------------------------------------------------
###	=()<SYS_LANG=@<SYS_LANG>@>()=
SYS_LANG=0

#---------------------------------------------------------------
# Define INFORMIX related stuff.
#---------------------------------------------------------------
if [ "$LS10_DB" = "INFORMIX" ]
then
	###	=()<INFORMIXDIR=@<INFORMIXDIR>@>()=
INFORMIXDIR=/usr/informix7
	export 	INFORMIXDIR

	###	=()<INFORMIXSERVER=@<INFORMIXSERVER>@>()=
INFORMIXSERVER=local
	export 	INFORMIXSERVER

	###	=()<if [ -n "@<INFORMIXTERM>@" ] >()=
if [ -n "terminfo" ] 
	then
		###	=()<	INFORMIXTERM=@<INFORMIXTERM>@>()=
	INFORMIXTERM=terminfo
		export INFORMIXTERM
	fi

	###	=()<SQLEXEC=@<SQLEXEC>@>()=
SQLEXEC=sqlexec
	export SQLEXEC
fi
#---------------------------------------------------------------
# Define ORACLE related stuff.
#---------------------------------------------------------------
if [ "$LS10_DB" = "ORACLE" ]
then
	###	=()<ORACLEDIR=@<ORACLEDIR>@>()=
ORACLEDIR=/usr/oracle/OraHome1
	export 	ORACLEDIR

	###	=()<ORACLE_HOME=@<ORACLE_HOME>@>()=
ORACLE_HOME=/usr/oracle/OraHome1
	export 	ORACLE_HOME

	###	=()<LOCK_DIR=@<LOCK_DIR>@>()=
#LOCK_DIR=/BIN/LOCK
	export 	LOCK_DIR
fi
#---------------------------------------------------------------
# Define DB2 related stuff.
#---------------------------------------------------------------
if [ "$LS10_DB" = "DB2" ]
then
	###	=()<DB2DIR=@<DB2DIR>@>()=
DB2DIR=/usr/DB2
	export 	DB2DIR
fi

#---------------------------------------------------------------
# Spooler Specific Environment Variables.
#---------------------------------------------------------------
###	=()<if [ -n "@<LPDIR>@" ] >()=
if [ -n "/usr/Ease/EaseSpool" ] 
then
	###	=()<	LPDIR=@<LPDIR>@>()=
	LPDIR=/usr/Ease/EaseSpool
	export LPDIR
fi

#---------------------------------------------------------------
# Run Check For Licence 
#---------------------------------------------------------------
check_licence ()
{
	ttyno=`tty`
	if [ "$ttyno" = "$CONSOLE_TTY" ]
	then
		chk_brand
	fi
}

#---------------------------------------------------------------
# Run check for backup
#---------------------------------------------------------------
check_backup ()
{
    if [ -f $PROG_PATH/BIN/.Backup ]
    then
	echo $CLEAR
	echo "+--------------------------------------------------+"
	echo "| Backup or night processing is currently running. |"
	echo "+--------------------------------------------------+"
	sleep 2
	exit
    fi
}

#---------------------------------------------------------------
# Run company/branch/warehouse Select    
#---------------------------------------------------------------
run_allselect ()
{
	#---------------------------------------------------------------
	# all_select returns -1 if no company / branch / warehouse selected
	#                     0 otherwise.
	#---------------------------------------------------------------
 	all_select C
	if [ ! $? -eq 0 ]
	then
		echo " No Company / Branch / Warehouse was selected "
		exit
		exit
	fi
}

#---------------------------------------------------------------
# Check if shutdown script exists
#---------------------------------------------------------------
CheckForShutdown ()
{
	if [ -f $PROG_PATH/BIN/SCRIPT/shutdown.sh ]
	then
		echo "Shutdown Initiated At `date`" >> $PROG_PATH/BIN/LOG/shutdown.log
		$PROG_PATH/BIN/SCRIPT/shutdown.sh
	fi
}

#---------------------------------------------------------------
# Export relevent system variables
#---------------------------------------------------------------
export_all ()
{
	export DATA_PATH FILE_PERM COPYRIGHT EXINIT PATH DBPATH OUTRANGE_CHAR
	export WP_USED ADJ_MONEY BUDGET DEBUG SPREAD DBTEMP SYS_LANG
	export SPREAD_SHEET SPREAD_EDIT DBDATE MAIL_USED

}

echo $CLEAR
echo "Initialising System for $LOGNAME"

#---------------------------------------------------------------
# Export database path for LS10 and Database
#---------------------------------------------------------------
DBPATH=":$PROG_PATH/DATA:$PROG_PATH/DATA/FORM:$PROG_PATH/DATA/SPEC_FORM:$PROG_PATH/BIN/SQL"
DATA_PATH="$PROG_PATH/DATA"

check_user=`basename $LOGNAME "\.sh"`

case $check_user in
#---------------------------------------------------------------
# Night processing user.
#---------------------------------------------------------------
night)
	export_all
	cat /dev/null > $PROG_PATH/BIN/.Backup
	night.sh
	rm $PROG_PATH/BIN/.Backup
	CheckForShutdown
	exit
	;;

#---------------------------------------------------------------
# Weekly and Friday user.
#---------------------------------------------------------------
weekly|friday)
	export_all
	cat /dev/null > $PROG_PATH/BIN/.Backup
	weekly.sh
	rm $PROG_PATH/BIN/.Backup
	CheckForShutdown
	exit
	;;

#---------------------------------------------------------------
# Month end processing user.
#---------------------------------------------------------------
premend|month|monthly)
	export_all
	cat /dev/null > $PROG_PATH/BIN/.Backup
	monthly.sh
	rm $PROG_PATH/BIN/.Backup
	CheckForShutdown
	exit
	;;

#---------------------------------------------------------------
# Logistic Support user.
#---------------------------------------------------------------
support)
	export_all
	check_licence 
	check_backup
	;;

#---------------------------------------------------------------
# LS10 - GUI startup script.
#---------------------------------------------------------------
LS10-GUI-start)
	export_all
	TERM_SLOT=0
	export TERM_SLOT
	PSL_MENU_PATH=$PROG_PATH/BIN/MENUSYS/GUI
	export	PSL_MENU_PATH

	echo "Starting $LS10_SERVER_DIR/SelSrvD"
	$LS10_SERVER_DIR/SelSrvD &
	sleep 5
	echo "Starting $LS10_SERVER_DIR/PwdSrvD"
	$LS10_SERVER_DIR/PwdSrvD &
	sleep 5
	exit
	;;

#---------------------------------------------------------------
# LS10 - GUI status script.
#---------------------------------------------------------------
LS10-GUI-status)

	echo "Logistic LS10 services - STATUS."

	#---------------------------------------------------------------
	# SelSrvD processes
	#---------------------------------------------------------------
	selSrvD=`ps -e | grep SelSrvD | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${selSrvD}." = ".." ]
	then
		for job_pid in ${selSrvD}
		do
			echo "${job_pid} SelSrvD"
		done
	fi

	#---------------------------------------------------------------
	# PwdSrvD processes
	#---------------------------------------------------------------
	pwdSrvD=`ps -e | grep PwdSrvD | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${pwdSrvD}." = ".." ]
	then
		for job_pid in ${pwdSrvD}
		do
			echo "${job_pid} PwdSrvD"
		done
	fi
	sleep 5
	exit
	;;

#---------------------------------------------------------------
# LS10 - GUI stop script.
#---------------------------------------------------------------
LS10-GUI-stop)
	ServerLogFile=${PROG_PATH}/BIN/LOG/LS10Server.log
	echo "Stopping Logistic LS10 services."
	echo "Stopping Logistic LS10 services at `date`." >> $ServerLogFile

	#---------------------------------------------------------------
	# First, lets kill the SelSrvD processes
	#---------------------------------------------------------------
	echo "Stopping SelSrvD."
	echo "Stopping SelSrvD at `date`." >> $ServerLogFile

	selSrvD=`ps -e | grep SelSrvD | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${selSrvD}." = ".." ]
	then
		for job_pid in ${selSrvD}
		do
			echo "kill -15 ${job_pid}"
			kill -15 ${job_pid}
		done
	fi

	#---------------------------------------------------------------
	# Second, lets kill the PwdSrvD processes
	#---------------------------------------------------------------
	echo "Stopping PwdSrvD."
	echo "Stopping PwdSrvD at `date`." >> $ServerLogFile

	pwdSrvD=`ps -e | grep PwdSrvD | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${pwdSrvD}." = ".." ]
	then
		for job_pid in ${pwdSrvD}
		do
			echo "kill -15 ${job_pid}"
			kill -15 ${job_pid}
		done
	fi

	#---------------------------------------------------------------
	# Check if kill -15 worked...  # If not, kill -9 them.
	#---------------------------------------------------------------
	echo "Checking that services stopped correctly."
	echo >> $ServerLogFile
	echo "Checking that services stopped correctly `date`." >> $ServerLogFile

	#---------------------------------------------------------------
	# Check for 'hung' SelSrvD processes
	#---------------------------------------------------------------
	hungJobs=`ps -e | grep SelSrvD | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${hungJobs}." = ".." ]
	then
		for job_pid in ${hungJobs}
		do
			kill -9 ${job_pid}
		done
	fi

	#---------------------------------------------------------------
	# Check for 'hung' PwdSrvD processes
	#---------------------------------------------------------------
	hungJobs=`ps -e | grep PwdSrvD | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${hungJobs}." = ".." ]
	then
		for job_pid in ${hungJobs}
		do
			kill -9 ${job_pid}
		done
	fi

	echo "Logistic LS10 services stopped."
	echo "Logistic LS10 services stopped at `date`." >> $ServerLogFile
	echo "#-----------------------------------------------" >> $ServerLogFile

	sleep 5
	exit
	;;

#---------------------------------------------------------------
# Start LS10 processes. 
#---------------------------------------------------------------
LS10-start)
	export_all
	TERM_SLOT=0
	export TERM_SLOT

	echo "Starting LS10 Background Process"
	exec bg_ctrl 1
	sleep 5
	exit
	;;

#---------------------------------------------------------------
# Stop LS10 processes. 
#---------------------------------------------------------------
LS10-stop)
	export_all
	TERM_SLOT=0
	export TERM_SLOT

	echo "Stopping LS10 Background Process"
	exec bg_ctrl 0
	sleep 5
	exit
	;;
#---------------------------------------------------------------
# All other LS10 users. 
#---------------------------------------------------------------
*)
	export_all
	check_licence 
	check_backup
	;;
esac

#---------------------------------------------------------------
# Run system menu and setup terminal slot.
#---------------------------------------------------------------
TERM_SLOT=
export TERM_SLOT
TERM_SLOT=`set_tslot`
export	TERM_SLOT
exec menu "all_select C"
