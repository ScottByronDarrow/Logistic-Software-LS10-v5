#!/bin/sh
#============================================================
#  Copyright (C) 1998 - 2001 LogisticSoftware
#============================================================
#  $Id: monthly.sh,v 5.5 2001/11/29 01:03:08 scott Exp $
# Standard Night Processing File. :                          
#                                                            
#     Shell names : night.sh  linked to                      
#                   day.sh    linked to                      
#                   weekly.sh linked to                      
#                   monthly.sh                               
#============================================================
#  $Log: monthly.sh,v $
#  Revision 5.5  2001/11/29 01:03:08  scott
#  Updated from testing
#
#  Revision 5.5  2001/11/14 04:26:36  scott
#  Updated to point to correct path for SANE.sh
#
#  Revision 5.4  2001/10/17 01:52:11  cha
#  Updated to fix conflicting status update with direct deliveries.
#  Changes made by Scott.
#
#============================================================

LPNO=$1
DUMMY_LPNO=$1

Clear ()
{
	$PROG_PATH/BIN/SCRIPT/SANE.sh
}
#====================================
#| Checking for Sleep/Delay Utility |
#====================================
CheckDelay ()
{
case ${SCRIPT} in

#       *** MODIFY IF REQUIRED START ***
	night.sh)
		s_time="5"
		echo "============================================================"
		echo "SYSTEM TO SLEEP FOR $s_time SECONDS "
		sleep $s_time
		cat /dev/null > $PROG_PATH/BIN/.Backup
		;;

	weekly.sh)
		s_time="10"
		echo "============================================================"
		echo "SYSTEM TO SLEEP FOR $s_time SECONDS "
		sleep $s_time
		cat /dev/null > $PROG_PATH/BIN/.Backup
		;;
#       *** MODIFY IF REQUIRED FINISH ***

	*)
		echo "============================================================"
		echo "NO SLEEP/DELAY INVOLVED"
		;;
esac
}

#=========================================
#| General Ledger Posting Fixup Utility. |
#=========================================
postfix ()
{
#      -------------------------------------------
#      | If last posting exited with a non zero. |
#      -------------------------------------------
        if [ $? -ne 0 ] 
        then
			gl_postfix      
#           ----------------------
#           | If Can't fix exit. |
#           ----------------------
            if [ $? -ne 0 ] #       If can't fix exit.
            then
            	exit 1
     		fi
        fi
}

run_backup ()
{
BACKUP="YES"
valid_reply=n
while [ "${valid_reply}" = "n" ]
do
	case ${SCRIPT} in
	monthly.sh)
		if [ "${month_bflag}" = "Y" ]
		then
			echo "============================================================"
			echo "THE PRE-MONTH END PROCESSING IS NOW COMPLETE "
			echo "THE SYSTEM WILL NOW UNDERTAKE (IF REQUIRED) ANOTHER "
			echo "BACKUP OF THE DATABASE FILES "
		fi
	;;
	esac
	Clear
	echo "============================================================"
	echo "PLEASE NOMINATE THE BACKUP DEVICE TO BE USED"
	echo "1 : BACKUP UNIT"
	echo "2 : NO BACKUP REQUIRED"
	echo "============================================================"
	echo "ANSWER (1-2)"
	read answer
	case ${answer} in
	1)
		TAPE_DEVICE=/dev/ios0/rstape006
		valid_reply=y
		;;
	2)
		BACKUP="NO"
		valid_reply=y
		;;
	*)
		sleep 1
		;;
	esac
done

if [ "YES" = "$BACKUP" ]
then
	Clear
	echo "============================================================"
	echo "TAPE DEVICE $TAPE_DEVICE IS BEING USED"
	echo "PLEASE REMEMBER TO PUT TAPE INTO DRIVE"
	echo "============================================================"
	case ${SCRIPT} in
		monthly.sh)
			if [ "${month_bflag}" = "Y" ]
			then
				echo "========================================================="
				echo "BACKUP TAPES SHOULD BE FOR : ( MONTH_END )"
				echo "========================================================="
			fi
			;;
        *)
			echo "========================================================="
			echo "BACKUP TAPES SHOULD BE FOR : ( ${SCRIPT} )"
			echo "========================================================="
			;;
	esac
		echo "============================================================"
        echo "PRESS RETURN WHEN READY"
		echo "============================================================"
        read answer

	case ${SCRIPT} in
        night.sh|day.sh)
			CheckDelay
			cd /
			echo "Daily Backup Started At `date`" >> $BL
			find $PROG_PATH/DATA -depth -print | cpio -ocvB > $TAPE_DEVICE
			echo "Daily Finished At `date`" >> $BL
			cd $PROG_PATH/BIN
			;;

        weekly.sh)
        bin/sh $PROG_PATH/BIN/SCRIPT/SANE.sh
        Clear
		echo "========================================================="
        echo "PLEASE CHECK THAT PROGRAM BACKUP TAPE IS IN DRIVE."
        echo "BACKUP TAPES SHOULD BE FOR : (${SCRIPT}) "
		echo "========================================================="
        echo "             PRESS RETURN WHEN READY"
        read answer
		CheckDelay
		cd /
		echo "Weekly Program Backup Started At `date`" >> $BL
		find $PROG_PATH etc -depth -print | grep -v $PROG_PATH/DATA | cpio -ocvB > $TAPE_DEVICE
		echo "Weekly Program Backup Finished At `date`" >> $BL

        Clear
		echo "========================================================="
        echo "   PLEASE CHECK THAT DATA BACKUP TAPE IS IN DRIVE."
        echo "     BACKUP TAPES SHOULD BE FOR : ${SCRIPT}"
		echo "========================================================="
        echo "           PRESS RETURN WHEN READY."
        read answer
		CheckDelay
		cd /
		echo "Weekly Data Backup Started At `date`" >> $BL
		find $PROG_PATH/DATA -depth -print | cpio -ocvB > $TAPE_DEVICE
		echo "Weekly Data Backup Finished At `date`" >> $BL
		cd $PROG_PATH/BIN       
	;;

        monthly.sh)
		CheckDelay
		cd /
		echo "Monthly Backup Started At `date`" >> $BL
		find $PROG_PATH/DATA  -depth -print | cpio -ocvB > $TAPE_DEVICE
		echo "Monthly Backup Finished At `date`" >> $BL
		cd $PROG_PATH/BIN
		;;
#       *** MODIFY IF REQUIRED END ***

         *)
		echo "No Backup Done" >> $BL
        ;;
        esac
fi
}

run_purge ()
{
	# -------------------
	# | Run Purge files |
	# -------------------
	case $SCRIPT in
        *)      
			echo "Miscellaneous Delete Started at `date`." >> $BL
			#-------------------------------------------------------
			#| Delete completed transactions from pogh/pogl files. |
			#-------------------------------------------------------
			po_grin_del  2>/dev/null

			#-------------------------------------------------------
			#| Delete completed transactions from ithr/itln files. |
			#-------------------------------------------------------
			sk_it_del 1  2>/dev/null

			#----------------------------------------------
			#| Delete completed sohr / soln transactions. |
			#----------------------------------------------
			so_rt_del   2>/dev/null

			#-----------------------------------------------------------
			#| Delete misc files : cfhs, bach, extr, cucc, cuph, inaf. |
			#-----------------------------------------------------------
			psl_misc_del  2>/dev/null

			#-------------------------------
			#| Delete Real time committed. |
			#-------------------------------
			so_cmmt_del $LPNO  2>/dev/null

			#-------------------------------
			#| Delete GL batch records.    |
			#-------------------------------
			gl_batch_del  2>/dev/null

			echo "Miscellaneous Delete's Ended at `date`" >> $BL
			;;
	esac
}

run_delete ()
{
	# ----------------------------------------
	# | Run Invoice/Stock Transaction Delete |
	# ----------------------------------------
	case $SCRIPT in
		weekly.sh)

		echo "Invoice Audit Delete Started at `date`" >> $BL
	
		#--------------------------------------------------------------
		#| so_auddel <find-stat> <No of days tr held> <check on cuin> |
		#--------------------------------------------------------------
		so_auddel  2>/dev/null
	
		echo "Invoice Audit Delete Finished." >> $BL
			
		echo "Stock Transaction Delete Started at `date`." >> $BL

		sk_trandel $LPNO  2>/dev/null
#       sk_mbaldel $LPNO 90 

		echo "Stock Transaction Delete Finished at `date`." >> $BL

		echo "Fifo Record Delete Started at `date`." >> $BL

		sk_fifo_del  2>/dev/null

		echo "Fifo Record Delete Ended at `date`." >> $BL

		echo "Fifo Record Check  Started at `date`." >> $BL

		sk_fifo_chk 1 U  2>/dev/null

		echo "Fifo Record Check  Ended at `date`." >> $BL

		echo "Stock Balance Check Started at `date`." >> $BL

		sk_bal_fix 1 U 2>/dev/null

		echo "Stock Balance Check Ended at `date`." >> $BL
		;;
	*)
		echo "No Invoice/Stock Transaction/Fifo Record Delete" >> $BL
		;;
	esac
}

cobrwh_select ()
{
	echo $cobrwh
	if [ "$cobrwh" = "END" ] 
	then
		Clear
		echo "============================================================"
		echo "RUNNING COMPANY UPDATE "
		sleep 2
		co_update
	else
		Clear
		echo "============================================================"
		echo "RUNNING BRANCH UPDATE FOR $cobrwh "
		re_select $cobrwh
		sleep 2
		br_update
	fi
}

br_update ()
{
	echo "============================================================"
	echo "PROCESSING INVOICE TRANSACTIONS"
	# Start Branch Processing

	echo "Invoice Processing For $cobrwh Started at `date`" >> $BL

	#---------------------------------------------------------------
	#| so_stkup "6" "4" "I"     ==> Posts all invoices to stock.   |
	#---------------------------------------------------------------
	so_stkup "X" "4" "I" 2>/dev/null
	so_stkup "7" "4" "I" 2>/dev/null
	so_stkup "6" "4" "I" 2>/dev/null

	#---------------------------------------------------------------
	#| so_overide LPNO <status> ==> Prints invoice overide report. |
	#---------------------------------------------------------------
	so_overide $LPNO 4  2>/dev/null

	#--------------------------------------------------------------
	#| so_invledup "4" "3"  ==> Posts Invoices to Debtors Ledger. |
	#--------------------------------------------------------------
	if [ "$AUTO_CASH" = "N" ]
	then
		so_invledup "4" "3" "I" $DUMMY_LPNO 
	else
		#---------------------------------------------------------
		#| auto cash receipts on cash sales account is required.  |
		#---------------------------------------------------------
		so_gl_ledup "4" "3" "I" $DUMMY_LPNO 
	fi
	#-------------------------------------------------------------
	#| so_invsaup "3" "2"  ==> Posts Invoices to Sales Analysis. |
	#-------------------------------------------------------------
	so_invsaup "3" "2" "I"  2>/dev/null

	#-------------------------------------------------------------
	#| so_invglup "2" "1"  ==> Posts Invoices to G/L work Files. |
	#-------------------------------------------------------------
	so_invglup "2" "1" "I" 

	#-------------------------------------------------------------
	#| so_comm_upd "1" "0"  ==> Posts Invoices to Machine History |
	#-------------------------------------------------------------
	so_comm_upd "1" "0" "I" $LPNO  2>/dev/null

	#-------------------------------------------------------------
	#| so_invmhup "0" "9"  ==> Posts Invoices to Machine History |
	#-------------------------------------------------------------
	so_invmhup "0" "9" "I"  2>/dev/null

	#---------------------------------------------------------
	#| Creat debtors sales journal and post records from GL. |
	#---------------------------------------------------------
	db_winvtogl I $LPNO 
	gl_postbat $PID $LPNO " 4" 2>/dev/null

	echo "Credit Note Processing For $cobrwh Started at `date`." >> $BL

	echo "============================================================"
	echo "PROCESSING CREDIT NOTE TRANSACTIONS"
	#--------------------------------------------------------------
	#| so_stkup "6" "4" "C"     ==> Posts all credits to stock.   |
	#--------------------------------------------------------------
	so_stkup "X" "4" "C" 2>/dev/null
	so_stkup "7" "4" "C" 2>/dev/null
	so_stkup "6" "4" "C" 2>/dev/null

	#-------------------------------------------------------------
	#| so_crdledup "4" "3"  ==> Posts Credits to Debtors Ledger. |
	#-------------------------------------------------------------
	if [ "$AUTO_CASH" = "N" ]
	then
		so_crdledup "4" "3" "C" $DUMMY_LPNO 
	else
		#---------------------------------------------------------
		#| auto cash receipts on cash sales account is required.  |
		#---------------------------------------------------------
		so_gl_ledup "4" "3" "C" $DUMMY_LPNO
	fi

	#------------------------------------------------------------
	#| so_crdsaup "3" "2"  ==> Posts Credits to Sales Analysis. |
	#------------------------------------------------------------
	so_crdsaup "3" "2" "C" 2>/dev/null

	#------------------------------------------------------------
	#| so_crdglup "2" "1"  ==> Posts Credits to G/L work Files. |
	#------------------------------------------------------------
	so_invglup "2" "1" "C" 

	#-------------------------------------------------------------
	#| so_comm_upd "1" "9"  ==> Posts Invoices to Machine History |
	#-------------------------------------------------------------
	so_comm_upd "1" "9" "C" $LPNO 2>/dev/null

	#-----------------------------------------------------------------
	#| Creat debtors sales returns journal and post records from GL. |
	#-----------------------------------------------------------------
	db_winvtogl C $LPNO 
	gl_postbat 1 $LPNO " 5" 2>/dev/null

	echo "Credit Posting to G/L Done." >> $BL

	echo "Inv/Cr Note Processing For $cobrwh Finished at `date`" >> $BL

	# -------------------------------------------
	# | Post Stock Transfers to General Ledger. |
	# -------------------------------------------
	sk_ir_glup 1 B

	#-------------------------------------------
	#| Post Stock Purchases to General Ledger. |
	#-------------------------------------------
	po_gl_up 2>/dev/null
	gl_format 11 $LPNO 2>/dev/null
	gl_postbat $PID $LPNO "11" 2>/dev/null

	#-------------------------------------
	#| Post Contracts to General Ledger. |
	#-------------------------------------
	echo "Contract Posting to G/L Done. "     >> $BL

	echo "Branch Processing For $cobrwh Finished at `date`" >> $BL
	# End Branch Processing
}

co_update ()
{
	# Start Company Processing

	echo "============================================================"
	echo "PROCESSING ALL OTHER TRANSACTIONS"
	echo "Company Processing Started at `date`" >> $BL

	# ------------------------------------------------------------
	# | Post Stock Misc Stock issues/receipts to General Ledger. |
	# ------------------------------------------------------------
	gl_format 10 $LPNO 2>/dev/null
	gl_postbat $PID $LPNO "10"  2>/dev/null
	echo "Misc stock issues/receipts Posting to G/L Done. "  >> $BL

	#---------------------------------------------
	#| Post Stock Adjustments to General Ledger. |
	#---------------------------------------------
	gl_format 12 $LPNO 2>/dev/null
	gl_postbat $PID $LPNO "12" 2>/dev/null
	echo "Stock Adjustments Posting to G/L Done. "  >> $BL
	

	#-----------------------------------------------
	#| Post Stock Cost of Sales to General Ledger. |
	#-----------------------------------------------
	gl_format 13 $LPNO 2>/dev/null
	gl_postbat $PID $LPNO "13" 2>/dev/null
	echo "Stock Cost of Sales Posting to G/L Done. " >> $BL

	#-------------------------------------------------------
	#| Post Production Issues & Receipts to General Ledger. |
	#-------------------------------------------------------
	pc_glcons $LPNO > /dev/null
	gl_format 19 $LPNO 2>/dev/null
	gl_postbat $PID $LPNO "19" 2>/dev/null
	echo "Production Issues & Receipts Posting to G/L Done." >> $BL

	#------------------------
	# Run Daily File Purge  |
	#------------------------
	run_purge

	#-----------------------------------------
	# Run Invoice/Stock Transaction Deletion |
	#-----------------------------------------
	run_delete

	echo "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+" >> $BL
	echo "=+ Company Processing Finished at `date`   =+" >> $BL
	echo "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+" >> $BL
	# End Company Processing
}

#===================================
# The Main Processing Starts Here  |
#===================================

# ------------------------------------------------------------
# | Get Process Id to make files unique to night processing. |
# ------------------------------------------------------------
PID=$$

# ----------------------------------------------------------------------
# | Set Variable AUTO_CASH To Yes for automatic Cash Receipt Journals. |
# ----------------------------------------------------------------------
AUTO_CASH="Y"

# --------------------------------------------------
# | Get script name of script currently being run. |
# --------------------------------------------------
SCRIPT=`basename $0`

# ----------------------------------------------
# | Set up default printer to print night log. |
# ----------------------------------------------
#       *** MODIFY IF REQUIRED START ***
DEF_QUEUE="lp -d SYSTEM"       # used for later versions of easespool
export DEF_QUEUE
#       *** MODIFY IF REQUIRED END ***

# ---------------------------
# | Get base log file name. |
# ---------------------------
BL=$PROG_PATH/BIN/LOG/base.log
LOG_DIR=$PROG_PATH/BIN/LOG

if [ $# -eq 0 ]
then
	echo "========================================================="
	echo "USAGE : `basename $0` <LPNO>"
	echo "========================================================="
	exit
fi

Clear
echo "====================================================================="
echo "($SCRIPT)  PROCESSING STARTED"

#-------------------------
#| Check for lock files. |
#-------------------------
while [ -f $PROG_PATH/WORK/${SCRIPT}.LOCK ]
do
	Clear
	echo "====================================================================="
	echo "${SCRIPT} processing locked."
	echo "Remove file ($PROG_PATH/WORK/${SCRIPT}.LOCK) to Continue."
	echo "====================================================================="
	sleep 10
done

# ----------------------
# | Create lock files. |
# ----------------------
cat /dev/null > $PROG_PATH/WORK/${SCRIPT}.LOCK

# ------------------------------------------------
# | Make copies of previous night processing log |
# ------------------------------------------------
echo "====================================================================="
echo "C R E A T I N G  L O G  F I L E S."

mv -f $LOG_DIR/${SCRIPT}.log4 $LOG_DIR/${SCRIPT}.log5 2> /dev/null
mv -f $LOG_DIR/${SCRIPT}.log3 $LOG_DIR/${SCRIPT}.log4 2> /dev/null
mv -f $LOG_DIR/${SCRIPT}.log2 $LOG_DIR/${SCRIPT}.log3 2> /dev/null
mv -f $LOG_DIR/${SCRIPT}.log1 $LOG_DIR/${SCRIPT}.log2 2> /dev/null
mv -f $LOG_DIR/${SCRIPT}.log  $LOG_DIR/${SCRIPT}.log1 2> /dev/null

cd $PROG_PATH/BIN

echo "THE ${SCRIPT} PROCESSING STREAM STARTED AT `date`" > $BL

#-------------
# Run Backup |
#-------------

run_backup

Clear
echo "====================================================================="
echo "($SCRIPT) PROCESSING RUNNING "

# ----------------------------------
# | Delete save files from system. |
# ----------------------------------
echo "====================================================================="
echo "CLEARING PRINTER FILES > SEVEN DAYS OLD"
#       *** MODIFY IF REQUIRED START ***
find $PROG_PATH/SYSTEM \( -type f -name 'save*' \) -mtime +7 -exec rm -f {} \; 2>/dev/null
find $PROG_PATH/SYSTEM \( -type f -name '*.spl' \) -mtime +7 -exec rm -f {} \; 2>/dev/null
#       *** MODIFY IF REQUIRED FINISH ***

# ----------------------------------
# | Delete Work files from system. |
# ----------------------------------
echo "====================================================================="
echo "CLEARING WORK FILES"

#       *** MODIFY IF REQUIRED START ***
find "$PROG_PATH/WORK" \( -type f -name '*' \) -mtime +30 -exec rm -f {} \; 2>/dev/null
find $PROG_PATH/WORK \( -type f -name 'dsp*' \) -mtime +1 -exec rm -f {} \; 2>/dev/null
find $PROG_PATH/TAB \( -type f -name '*' \) -mtime +1 -exec rm -f {} \; 2>/dev/null
#       *** MODIFY IF REQUIRED FINISH ***

# -------------------------------------------------------------------
# | Update misc stock transactions to update from current terminal. |
# -------------------------------------------------------------------
case ${SCRIPT} in
	*)
		echo "============================================================"
		echo "UPDATE TRANSFERS G/L TRANSACTIONS TO CURRENT TERMINAL "
		TERM_SLOT=`set_tslot`
		gl_upd_glwk $TERM_SLOT 2>/dev/null
	;;
esac

# --------------------------------------------
# | Setup Companies/Branches To Be Processed |
# --------------------------------------------
case ${SCRIPT} in

day.sh)
		echo "============================================================"
        echo "PROCESSING CURRENT BRANCH ONLY "
        sleep 2
        br_update
        co_update
        ;;

*) 
        for fl in `cat $PROG_PATH/BIN/SCRIPT/cobrwh_list`
        do
			cobrwh=`echo $fl | sed 's/#/ /g'`
			echo $cobrwh
			sleep 2
			cobrwh_select
        done

		#-----------------------------------------------------------------
		# The following will post all up-posted gl trans from work file. |
		#-----------------------------------------------------------------
		gl_postbat $PID $LPNO " 1"  2>/dev/null
		gl_postbat $PID $LPNO " 4"  2>/dev/null
		gl_postbat $PID $LPNO " 5"  2>/dev/null
		gl_postbat $PID $LPNO " 6"  2>/dev/null
		gl_postbat $PID $LPNO " 7"  2>/dev/null
		gl_postbat $PID $LPNO " 8"  2>/dev/null
		gl_postbat $PID $LPNO " 9"  2>/dev/null
		gl_postbat $PID $LPNO "10"  2>/dev/null
		gl_postbat $PID $LPNO "11"  2>/dev/null
		gl_postbat $PID $LPNO "12"  2>/dev/null
		gl_postbat $PID $LPNO "13"  2>/dev/null
		gl_postbat $PID $LPNO "14"  2>/dev/null
		gl_postbat $PID $LPNO "15"  2>/dev/null
		gl_postbat $PID $LPNO "16"  2>/dev/null
		gl_postbat $PID $LPNO "17"  2>/dev/null
		gl_postbat $PID $LPNO "18"  2>/dev/null
		gl_postbat $PID $LPNO "19"  2>/dev/null
		gl_postbat $PID $LPNO "20"  2>/dev/null
		gl_postbat $PID $LPNO "21"  2>/dev/null
		gl_postbat $PID $LPNO "22"  2>/dev/null
		gl_postbat $PID $LPNO "23"  2>/dev/null
		gl_postbat $PID $LPNO "24"  2>/dev/null
		gl_postbat $PID $LPNO "25"  2>/dev/null
        ;;
esac

#-------------------------------------------------------
#| Clear out any System Accounting for Logistic Users. |
#-------------------------------------------------------
echo "============================================================"
echo "CLEARING SYSTEM ACCOUNTING FILES "
if [ -d $PROG_PATH/BIN/ACCOUNT ]
then
	find $PROG_PATH/BIN/ACCOUNT \( -name '*' \) -mtime +7 -exec rm -f {} \; 2>/dev/null
fi
if [ -d $PROG_PATH/BIN/ACCOUNT/.b ]
then
	find $PROG_PATH/BIN/ACCOUNT/.b \( -name '*' \) -mtime +7 -exec rm -f {} \;  2>/dev/null
fi

#-------------------------------------------------
#| Run any permanent user defined night reports. |
#-------------------------------------------------
echo "=+ Permanent User Defined Reports Started At `date`   =+" >> $BL

base_perm=`basename ${SCRIPT} .sh`
if [ -f SCRIPT/perm_${base_perm}.sh ]
then
	SCRIPT/perm_${base_perm}.sh 
fi

echo "=+ Permanent User Defined Reports Finished At `date`  =+" >> $BL

#--------------------------------
#| Run and clear Night Reports. |
#--------------------------------
echo "=+ ${SCRIPT} Reports Started At `date`                    =+" >> $BL
 
base_night=`basename ${SCRIPT} .sh`
if [ -f SCRIPT/${base_night}_repts.sh ]
then
	SCRIPT/${base_night}_repts.sh 2>/dev/null
	echo > SCRIPT/${base_night}_repts.sh
fi

echo "=+ ${SCRIPT} Reports Finished At `date`                   =+" >> $BL

# ---------------------
# | Running File Check|
# ---------------------
case ${SCRIPT} in

	weekly.sh)
		echo "============================================================"
		echo "RUNNING FILE CHECKS "
		echo "=+  Running File Check Started At `date`  +=" >> $BL
		chk_files.sh
		echo "=+  Running File Check Finished At `date`  +=" >> $BL
	;;

	*) 
		echo "============================================================"
		echo "FILE CHECKS NOT RUN. "
		echo "File Checks not run" >> $BL
        ;;
esac
# ---------------------
# | Running Rebuilds. |
# ---------------------
case ${SCRIPT} in

	weekly.sh)
		echo "============================================================"
		echo "RUNNING REBUILDS. "
		echo "=+  Rebuild shell Started At `date`  +=" >> $BL
		rebuild.sh
		echo "=+  Rebuild shell Finished At `date`  +=" >> $BL
	;;

	*) 
		echo "============================================================"
		echo "REBUILDS NOT RUN."
		echo "Rebuild.sh not run" >> $BL
        ;;
esac

echo "=+  The ${SCRIPT} Processing Stream Finished At `date`    =+" >> $BL

case ${SCRIPT} in

monthly.sh)
        #       Set flag to indicate that first backup prior to final update
        #       has been completed
        month_bflag=Y

        #       Initiate A Second Data Backup Prior To Month End Close
        echo "=+ A 2nd Data Backup Prior To MonthEnd Started at`date`=+" >> $BL
        run_backup
        echo "=+ A 2nd Data Backup Prior To MonthEnd Finished at`date`=+" >> $BL
        ;;
esac

${DEF_QUEUE} $BL

mv $BL $LOG_DIR/${SCRIPT}.log 2>/dev/null
 
rm -f $PROG_PATH/WORK/${SCRIPT}.LOCK 2>/dev/null

# ==================================================
# | E N D   O F  N I G H T   P R O C E S S I N G . |
# ==================================================
