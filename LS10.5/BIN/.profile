
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | Standard UNIX flags required. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
umask	000
EXINIT='set shell=/bin/sh'

trap '' 1

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | PROG_PATH required to tell LSL where system is loaded into. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
###	=()<PROG_PATH=@<PROG_PATH>@>()=
PROG_PATH=/usr/ver9.10

# +=+=+=+=+=+=+=+=
# | System Path. |
# +=+=+=+=+=+=+=+=
PB=$PROG_PATH/BIN
LSL_PATH=$PB/AS:$PB/BM:$PB/CA:$PB/CM:$PB/CR:$PB/DB:$PB/DD:$PB/FA:$PB/FE:$PB/FF:$PB/GL:$PB/MAIL:$PB/MENU:$PB/MH:$PB/MISC:$PB/OL:$PB/PC:$PB/PM:$PB/PO:$PB/POS:$PB/PS:$PB/QC:$PB/QT:$PB/RG:$PB/SA:$PB/SJ:$PB/SK:$PB/SO:$PB/SQL:$PB/TM:$PB/TR:$PB/TS:$PB/UTILS:$PB/LRP:/usr/Ease/bin:$PB/SCRIPT

###	=()<PATH=$LSL_PATH:@<STDBIN>@:@<INFORMIXBIN>@>()=
PATH=$LSL_PATH:/bin:/usr/bin:/usr/informix7/bin

export	PATH PROG_PATH

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | Set up Clear screen, Bold on and off. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
BOLD=`tput smso`
OBOLD=`tput rmso`
CLEAR=`tput clear`

system_down ()
{
	echo $CLEAR
	echo "\t$BOLD+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=$OBOLD"
	echo "\t$BOLD|         System currently not available           |$OBOLD"
	echo "\t$BOLD|   as system maintenance is being carried out.    |$OBOLD"
	echo "\t$BOLD|     Please check with system administrator       |$OBOLD"
	echo "\t$BOLD|              or try again later                  |$OBOLD"
	echo "\t$BOLD+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=$OBOLD"

	sleep 2
	exit
}

#	system_down

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# Change directory to required place to be sure. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
cd $PROG_PATH/BIN


# +=+=+=+=+=+==+=+=+=+=+=+=+=+=+=+=
# | Set up LSL Console Port name. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
###	=()<CONSOLE_TTY=@<CONSOLE_TTY>@>()=
CONSOLE_TTY=/dev/tty01

# +=+=+=+=+=+=+=+=+=+=+=+=+=+
# | STANDARD LSL VARIABLES. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+
PSL_ENV_NAME=$PROG_PATH/BIN/LOGISTIC
export PSL_ENV_NAME

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | WP_USED when word processing package is required within LSL. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
###	=()<WP_USED=@<WP_USED>@>()=
#WP_USED=

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | COPYRIGHT LSL copyright. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=
COPYRIGHT="Logistic Software Limited."

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | MAIL_USED LSL mail = psl_mail, Unix mail = mail |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
###	=()<MAIL_USED=@<MAIL_USED>@>()=
MAIL_USED=psl_mail

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | FILE_PERM - Defines file permishion flags used by UNI. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
###	=()<FILE_PERM="@<OWNER>@ @<GROUP>@ 777">()=
FILE_PERM="sel develop 777"

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | ADJ_MONEY - Adjustment required for exclusion of one and two cent coins.  |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
ADJ_MONEY="1"

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | DEBUG - Turns on debug for development.     |
# | BUDGET - Used for debug within development. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
DEBUG=OFF
BUDGET=0

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | Spread sheet to General ledger interface variables. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
SPREAD=$PROG_PATH/SPREAD
###	=()<SPREAD_SHEET=@<SPREAD_SHEET>@>()=
SPREAD_SHEET=123
###	=()<SPREAD_EDIT=@<SPREAD_EDIT>@>()=
SPREAD_EDIT=vi

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | SQL environment variables. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
###	=()<DBDATE=@<DBDATE>@>()=
DBDATE=DMY4
###	=()<OUTRANGE_CHAR=@<OUTRANGE_CHAR>@>()=
SYS_LANG=0
OUTRANGE_CHAR=0
###	=()<INFORMIXDIR=@<INFORMIXDIR>@>()=
INFORMIXDIR=/usr/informix7
###	=()<DBTEMP=@<DBTEMP>@>()=
DBTEMP=/usr/tmp
export DBTEMP
###	=()<SQLEXEC=@<SQLEXEC>@>()=
SQLEXEC=sqlexec
export SQLEXEC
###	=()<if [ -n "@<INFORMIXTERM>@" ] >()=
if [ -n "" ] 
then
	###	=()<	INFORMIXTERM=@<INFORMIXTERM>@>()=
	INFORMIXTERM=
	export INFORMIXTERM
fi

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | Easespool Specific Environment Variables. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
###	=()<if [ -n "@<LPDIR>@" ] >()=
if [ -n "/usr/Ease/EaseSpool" ] 
then
	###	=()<	LPDIR=@<LPDIR>@>()=
	LPDIR=/usr/Ease/EaseSpool
	export LPDIR
fi

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | R U N   C H E C K  F O R  L I C E N C E |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
check_licence ()
{
	ttyno=`tty`
	if [ "$ttyno" = "$CONSOLE_TTY" ]
	then
		chk_brand
	fi
}

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | R U N   C H E C K  F O R  B A C K U P |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
check_backup ()
{
    if [ -f $PROG_PATH/BIN/.Backup ]
    then
	echo $CLEAR
	echo "\t$BOLD+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=$OBOLD"
	echo "\t$BOLD| Backup or night processing is currently running. |$OBOLD"
	echo "\t$BOLD+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=$OBOLD"

	sleep 2
	exit
    fi
}

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | R U N   C O M P A N Y / B R A N C H / W A R E H O U S E  S E L E C T |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
run_allselect ()
{
#
# all_select returns -1 if no company / branch / warehouse selected
#                     0 otherwise.
#
 	all_select C
	if [ ! $? -eq 0 ]
	then
		echo " No Company / Branch / Warehouse was selected "
		exit
		exit
	fi
}

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
# | C H E C K  I F  S H U T D O W N   S C R I P T   E X I S T S |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
s_down ()
{
    	if [ -f $PROG_PATH/BIN/SCRIPT/shutdown.sh ]
	then
		echo "Shutdown Initiated At `date`" >> $PROG_PATH/BIN/LOG/shutdown.log
		$PROG_PATH/BIN/SCRIPT/shutdown.sh
	fi
}

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | E X P O R T    R E L E V E N T   S Y S T E M    V A R I A B L E S    |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
export_all ()
{
	export DATA_PATH FILE_PERM COPYRIGHT EXINIT PATH DBPATH OUTRANGE_CHAR
	export WP_USED ADJ_MONEY BUDGET DEBUG SPREAD DBTEMP SYS_LANG
	export SPREAD_SHEET SPREAD_EDIT DBDATE INFORMIXDIR MAIL_USED

}

echo $CLEAR
echo "\t  ${BOLD}+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+${OBOLD}"
echo "\t           Initialising System for $LOGNAME "
echo "\t  ${BOLD}+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+${OBOLD}\n"

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | Export database path for LSL and isql. |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
DBPATH=":$PROG_PATH/DATA:$PROG_PATH/DATA/FORM:$PROG_PATH/DATA/SPEC_FORM:$PROG_PATH/BIN/SQL"
DATA_PATH="$PROG_PATH/DATA"

check_user=`basename $LOGNAME "\.sh"`

case $check_user in
night)
	export_all
	cat /dev/null > $PROG_PATH/BIN/.Backup
	night.sh
	rm $PROG_PATH/BIN/.Backup
	#s_down
	exit
	;;

weekly|friday)
	export_all
	cat /dev/null > $PROG_PATH/BIN/.Backup
	weekly.sh
	rm $PROG_PATH/BIN/.Backup
	s_down
	exit
	;;

premend|month|monthly)
	export_all
	cat /dev/null > $PROG_PATH/BIN/.Backup
	monthly.sh
	rm $PROG_PATH/BIN/.Backup
	#s_down
	exit
	;;
support)
	export_all
	check_licence 
	#check_backup
#	run_allselect
	;;
*)
	export_all
	check_licence 
	check_backup
#	run_allselect
	;;
esac

# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
# | R U N    S Y S T E M   M E N U . |
# +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=

# +=+=+=+=+=+=+=+=+=+=+=+=+
# | Set up Terminal Slot. |
# +=+=+=+=+=+=+=+=+=+=+=+=+
TERM_SLOT=
export TERM_SLOT
TERM_SLOT=`set_tslot`
export	TERM_SLOT
exec menu "all_select C"
