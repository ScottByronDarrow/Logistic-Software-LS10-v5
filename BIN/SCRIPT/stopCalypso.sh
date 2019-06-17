:
#
# Title:	stopCalypso.sh
# Author:	Campbell Mander
# Date:		20 January 1998
# Purpose:	Attempts to stop Logistic Calypso service processes
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
#
PROG_PATH=/usr/ver9
export PROG_PATH
CALYPSO_LOG=${PROG_PATH}/LOG/LogisticCalypso.log
export CALYPSO_LOG

cd $PROG_PATH/BIN

trap '' 1

# +=+=+=+=+=+=+=+=
# | System Path. |
# +=+=+=+=+=+=+=+=
PB=$PROG_PATH/BIN
SEL_PATH=$PB/AS:$PB/BM:$PB/CA:$PB/CM:$PB/CR:$PB/DB:$PB/DD:$PB/FA:$PB/FE:$PB/FF:$PB/GL:$PB/MAIL:$PB/MENU:$PB/MH:$PB/MISC:$PB/OL:$PB/PC:$PB/PM:$PB/PO:$PB/POS:$PB/PS:$PB/QC:$PB/QT:$PB/RG:$PB/SA:$PB/SJ:$PB/SK:$PB/SO:$PB/SQL:$PB/TM:$PB/TR:$PB/TS:$PB/UTILS:$PB/LRP:/usr/Ease/bin:$PB/SCRIPT

###	=()<PATH=$SEL_PATH:@<STDBIN>@:@<INFORMIXBIN>@>()=
PATH=$SEL_PATH:/bin:/usr/bin:/usr/informix/bin:/etc:/usr/lib:/usr/ucb:/usr/lbin:/usr/UAP/bin

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | Export database path for SEL and isql. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
DBPATH=":$PROG_PATH/DATA:$PROG_PATH/DATA/FORM:$PROG_PATH/DATA/SPEC_FORM:$PROG_PATH/BIN/SQL"
DATA_PATH="$PROG_PATH/DATA"
TERM=vt220
TERM_SLOT=1

#
# E X P O R T    R E L E V E N T   S Y S T E M    V A R I A B L E S    |
export COPYRIGHT PATH DBPATH TERM TERM_SLOT

echo "Stopping Logistic Calypso services."
echo "Stopping Logistic Calypso services at `date`." >> $CALYPSO_LOG

# First, lets kill the so_calorder processes so that no more orders
# can be called up by Calypso
#
echo "Stopping so_calorder."
echo "Stopping so_calorder at `date`." >> $CALYPSO_LOG

calOrder=`ps -e | grep so_calor | sed -e 's/^  *//' -e s'/ .*//'`
if [ ! ".${calOrder}." = ".." ]
then
	for job_pid in ${calOrder}
	do
		kill -15 ${job_pid}
	done
fi

# Second, lets kill the so_calpayment processes so that no more 
# transactions can be processed by Calypso
#
echo "Stopping so_calpayment."
echo "Stopping so_calpayment at `date`." >> $CALYPSO_LOG

calPayment=`ps -e | grep so_calpa | sed -e 's/^  *//' -e s'/ .*//'`
if [ ! ".${calPayment}." = ".." ]
then
	for job_pid in ${calPayment}
	do
		kill -15 ${job_pid}
	done
fi

# Lastly, lets kill the sk_calrange processes
#
echo "Stopping sk_calrange."
echo "Stopping sk_calrange at `date`." >> $CALYPSO_LOG

calRange=`ps -e | grep sk_calra | sed -e 's/^  *//' -e s'/ .*//'`
if [ ! ".${calRange}." = ".." ]
then
	for job_pid in ${calRange}
	do
		kill -15 ${job_pid}
	done
fi

#
# Check if kill -15 worked...
# If not, kill -9 them.
#
echo "Checking that services stopped correctly."
echo >> $CALYPSO_LOG
echo "Checking that services stopped correctly at `date`." >> $CALYPSO_LOG

#Check for 'hung' so_calorder processes
#
hungJobs=`ps -e | grep so_calor | sed -e 's/^  *//' -e s'/ .*//'`
if [ ! ".${hungJobs}." = ".." ]
then
	for pid in ${hungJob}
	do
		kill -9 ${pid}
	done
fi

#Check for 'hung' so_calpayment processes
#
hungJobs=`ps -e | grep so_calpa | sed -e 's/^  *//' -e s'/ .*//'`
if [ ! ".${hungJobs}." = ".." ]
then
	for pid in ${hungJob}
	do
		kill -9 ${pid}
	done
fi

#Check for 'hung' sk_calrange processes
#
hungJobs=`ps -e | grep sk_calra | sed -e 's/^  *//' -e s'/ .*//'`
if [ ! ".${hungJobs}." = ".." ]
then
	for pid in ${hungJob}
	do
		kill -9 ${pid}
	done
fi

echo "Logistic Calypso services stopped."
echo "Logistic Calypso services stopped at `date`." >> $CALYPSO_LOG
echo "-----------------------------------------------" >> $CALYPSO_LOG

sleep 5
