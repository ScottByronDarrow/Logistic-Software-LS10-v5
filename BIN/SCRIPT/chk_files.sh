#!/bin/sh
#+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
#   Copyright (C) 1990 - 1992 Pinnacle Software limited.    |
#+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+|
#                                                           |
# @(#) - chk_files.sh                                       |
#                                                           |
# Date Last Modified (  /  /  ) By :                        |
#                                                           |
# Comments :                                                |
#                                                           |
#+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=

# -----------------------------------------
# | Set up Clear screen, Bold on and off. |
# -----------------------------------------
BOLD=`tput smso`
OBOLD=`tput rmso`
CLEAR=`tput clear`

# -------------------------
# | Set up user and owner |
# -------------------------

USER=lsl
GROUP=lsl

export USER GROUP

if [ "$PROG_PATH" = "" ]
then
        echo $CLEAR
        echo "${BOLD} PROG_PATH environment variable not set!! ${OBOLD}"
        exit
fi

if [ ! -d $PROG_PATH/BIN ] 
then    
        echo $CLEAR
        echo "${BOLD} $PROG_PATH/BIN Does not exist!! ${OBOLD}"
        exit
fi
if [ ! -d $PROG_PATH/DATA/data.dbs ] 
then    
        echo $CLEAR
        echo "${BOLD} $PROG_PATH/DATA/data.dbs Does not exist!! ${OBOLD}"
        exit
fi

cd $PROG_PATH/BIN

if [ ! -d $PROG_PATH/BIN/LOG ] 
then    
        mkdir $PROG_PATH/BIN/LOG
        chown $USER $PROG_PATH/BIN/LOG
        chgrp $GROUP $PROG_PATH/BIN/LOG
        chmod 666 $PROG_PATH/BIN/LOG
fi

LOG_DIR="$PROG_PATH/BIN/LOG"

mv $LOG_DIR/chk_files.log4 $LOG_DIR/chk_files.log5 2>/dev/null
mv $LOG_DIR/chk_files.log3 $LOG_DIR/chk_files.log4 2>/dev/null
mv $LOG_DIR/chk_files.log2 $LOG_DIR/chk_files.log3 2>/dev/null
mv $LOG_DIR/chk_files.log1 $LOG_DIR/chk_files.log2 2>/dev/null
mv $LOG_DIR/chk_files.log  $LOG_DIR/chk_files.log1 2>/dev/null

cd $PROG_PATH/DATA/data.dbs

echo "Check Of Database Files Started At `date`" > $LOG_DIR/chk_files.log

for data_file in *.dat
do
        echo "Running check on $data_file"
        echo "Check of $data_file started `date`" >> $LOG_DIR/chk_files.log
        
        bcheck -y $data_file 2>/dev/null

        echo "Check of $data_file finished `date`" >> $LOG_DIR/chk_files.log

done
        
echo "Reset File Permissions At `date`" >> $LOG_DIR/chk_files.log

#
# Set permissions on files

cd $PROG_PATH/DATA/data.dbs

for fl in `ls`
do
        chmod 666 $fl 2>/dev/null
        chown $USER $fl 2>/dev/null
        chgrp $GROUP $fl 2>/dev/null
        echo "Permissions Have Been Reset For $fl"
done

echo "Check Of Database Files Finished At `date`" >> $LOG_DIR/chk_files.log

cd $PROG_PATH/BIN

chmod 666 $LOG_DIR/chk_files.log 2>/dev/null

${DEF_QUEUE} $LOG_DIR/chk_files.log 2>/dev/null

