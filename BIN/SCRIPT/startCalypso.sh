#!/bin/sh
#
# Title:	startCalypso.sh
# Author:	Scott B Darrow 
# Date:		6 September 1999
# Purpose:	Stops Calypso using soft and hard modes that re-starts services
#
# The Logistic Calypso services are :
#	so_calpayment	Transaction processing
#	so_calorder		Order information
#	sk_calrange		PLU Update
#
# If the Logistic Calypso services are running then they will be in one of
# two states.  
#
# If the Calypso server is NOT up, then one process for each of the Logistic 
# Calypso services will be running.  These processes are the LISTENing 
# processes, waiting for a connection to their respective ports.
#
# If the Calypso server IS up and running, then the LISTENing processes will 
# still exist, but there will also be a CONNECTed process for each service
# which is currently either waiting for or processing commands from the 
# Calypso server.
#
# This script shuts down ALL Logistic Calypso services.  This includes the
# LISTENing processes and any CONNECTed processes.
#
#   ** NOTE ** ** NOTE **** NOTE **** NOTE **** NOTE **** NOTE **** NOTE ** 
#   Chaning this script without a clear understanding of what you are 
#   doing will result in you being put to death by the author.
#
#	This script uses PROG_PATH
#	These should have been defined in /etc/init.d/Logistic
#	which is the script that calls stopCalypso.sh
# =================================
# | Standard UNIX flags required. |
# =================================
umask	000
EXINIT='set shell=/bin/sh'

trap '' 1

# ====================================================================
# | PROG_PATH required to tell Logistic where system is loaded into. |
# ====================================================================
PROG_PATH=/usr/ver9

# ================
# | System Path. |
# ================
PB=$PROG_PATH/BIN
PINN_PATH=$PB/BM:$PB/CM:$PB/CA:$PB/CR:$PB/DB:$PB/DD:$PB/FA:$PB/FE:$PB/FF:$PB/GL:$PB/MAIL:$PB/MENU:$PB/MH:$PB/OL:$PB/PC:$PB/PS:$PB/PO:$PB/PM:$PB/QC:$PB/QT:$PB/RG:$PB/SA:$PB/FR:$PB/SCRIPT:$PB/SJ:$PB/SK:$PB/SO:$PB/TM:$PB/TR:$PB/TS:$PB/UTILS:$PB/XP:/usr/cosmos/cosprint_2.1/bin:/$PB/UTILS

PATH=$PINN_PATH:/bin:/usr/bin:/usr/informix/bin

export	PATH PROG_PATH
stty -istrip

# ================================================
# Change directory to required place to be sure. |
# ================================================
cd $PROG_PATH/BIN

# =========================
# | Set up Terminal Slot. |
# =========================
TERM_SLOT=`set_tslot`
export	TERM_SLOT

# =============
# | Set Term. |
# =============
TERM=vt220
export TERM

# =================================
# | Set up SEL Console Port name. |
# =================================
###	=()<CONSOLE_TTY=@<CONSOLE_TTY>@>()=
CONSOLE_TTY=/dev/tty01

# ===========================
# | STANDARD SEL VARIABLES. |
# ===========================
PSL_ENV_NAME=$PROG_PATH/BIN/PINNACLE
export PSL_ENV_NAME

# ============================
# | COPYRIGHT SEL copyright. |
# ============================
COPYRIGHT="Software Engineering Limited."

# ==========================================================
# | FILE_PERM - Defines file permishion flags used by UNI. |
# ==========================================================
###	=()<FILE_PERM="@<OWNER>@ @<GROUP>@ 777">()=
FILE_PERM="psl psl 777"

# ==============================
# | SQL environment variables. |
# ==============================
DBDATE=DMY4
SYS_LANG=1
OUTRANGE_CHAR=1
INFORMIXDIR=/usr/informix
DBTEMP=/usr/tmp
export DBTEMP
SQLEXEC=sqlexec
export SQLEXEC
###	=()<if [ -n "@<INFORMIXTERM>@" ] >()=
if [ -n "" ] 
then
	###	=()<	INFORMIXTERM=@<INFORMIXTERM>@>()=
	INFORMIXTERM=
	export INFORMIXTERM
fi

# =============================================
# | Cosmos    Specific Environment Variables. |
# =============================================
###	=()<if [ -n "@<LPDIR>@" ] >()=
if [ -n "/usr/cosmos/cosprint_2.1" ] 
then
	###	=()<	LPDIR=@<LPDIR>@>()=
	LPDIR=/usr/cosmos/cosprint_2.1
	export LPDIR
fi

CALYPSO_LOG=/usr/ver9/LOG/LogisticCalypso.log
export CALYPSO_LOG

export DATA_PATH FILE_PERM COPYRIGHT EXINIT PATH DBPATH 
export WP_USED SPREAD DBTEMP
export SPREAD_SHEET SPREAD_EDIT DBDATE INFORMIXDIR 

# ==========================================
# | Export database path for SEL and isql. |
# ==========================================
DBPATH=":$PROG_PATH/DATA:$PROG_PATH/DATA/FORM:$PROG_PATH/DATA/SPEC_FORM:$PROG_PATH/BIN/SQL"
DATA_PATH="$PROG_PATH/DATA"

# ---------------------------------------------------------------
# Ensure Calypso jobs are stopped before trying to start them.  |
# ---------------------------------------------------------------
StopCalypso ()
{
	echo "Seeing if Logistic Calypso service are running."
	echo "Stopping Logistic Calypso services `date`." >> $CALYPSO_LOG

	# -------------------------------------------------------------------
	# First, lets kill the so_calorder processes so that no more orders
	# can be called up by Calypso
	# -------------------------------------------------------------------
	echo "Stopping so_calorder at `date`." >> $CALYPSO_LOG

	calOrder=`ps -e | grep so_calor | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${calOrder}." = ".." ]
	then
		for job_pid in ${calOrder}
		do
			echo "Stopping softly so_calorder."
			kill -15 ${job_pid} >/dev/null
		done
	fi

	# -------------------------------------------------------------------
	# Second, lets kill the so_calpayment processes so that no more 
	# transactions can be processed by Calypso
	# -------------------------------------------------------------------
	echo "Stopping so_calpayment at `date`." >> $CALYPSO_LOG

	calPayment=`ps -e | grep so_calpa | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${calPayment}." = ".." ]
	then
		for job_pid in ${calPayment}
		do
			echo "Stopping softly so_calpayment."
			kill -15 ${job_pid} >/dev/null
		done
	fi
	# -------------------------------------------------------------------
	# Lastly, lets kill the sk_calrange processes
	# -------------------------------------------------------------------
	echo "Stopping sk_calrange at `date`." >> $CALYPSO_LOG

	calRange=`ps -e | grep sk_calra | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${calRange}." = ".." ]
	then
		for job_pid in ${calRange}
		do
			kill -15 ${job_pid} >/dev/null
			echo "Stopping softly sk_calrange."
		done
	fi

	# -------------------------------------------------------------------
	# Check if kill -15 worked...
	# If not, kill -9 them.
	# -------------------------------------------------------------------
	echo >> $CALYPSO_LOG
	echo "Checking that services stopped correctly at `date`." >> $CALYPSO_LOG

	# -------------------------------------------------------------------
	# Check for 'hung' so_calorder processes
	# -------------------------------------------------------------------

	hungJobs=`ps -e | grep so_calor | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${hungJobs}." = ".." ]
	then
		for pid in ${hungJobs}
		do
			echo "Stopping hard so_calorder."
			kill -9 ${pid} >/dev/null
		done
	fi

	# -------------------------------------------------------------------
	# Check for 'hung' so_calpayment processes
	# -------------------------------------------------------------------

	hungJobs=`ps -e | grep so_calpa | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${hungJobs}." = ".." ]
	then
		for pid in ${hungJobs}
		do
			echo "Stopping hard so_calpayment."
			kill -9 ${pid} >/dev/null
		done
	fi

	# -------------------------------------------------------------------
	# Check for 'hung' sk_calrange processes
	# -------------------------------------------------------------------
	hungJobs=`ps -e | grep sk_calra | sed -e 's/^  *//' -e s'/ .*//'`
	if [ ! ".${hungJobs}." = ".." ]
	then
		for pid in ${hungJobs}
		do
			echo "Stopping hard sk_calrange."
			kill -9 ${pid} >/dev/null
		done
	fi
	echo "Logistic Calypso services stopped at `date`." >> $CALYPSO_LOG
}
StopCalypso

echo "Starting Logistic Calypso."
echo "Starting Logistic Calypso services at `date`." >> $CALYPSO_LOG

echo "Starting so_calpayment."
echo "Starting so_calpayment at `date`." >> $CALYPSO_LOG
nohup so_calpayment >/dev/null &

echo "Starting so_calorder."
echo "Starting so_calorder at `date`." >> $CALYPSO_LOG
nohup so_calorder >/dev/null &

echo "Starting sk_calrange."
echo "Starting sk_calrange at `date`." >> $CALYPSO_LOG
nohup sk_calrange >/dev/null &

echo "Logistic Calypso services started."
echo "Logistic Calypso services started." >> $CALYPSO_LOG
