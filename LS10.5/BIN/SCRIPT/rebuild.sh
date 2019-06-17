#!/bin/sh
#
# @(#) - rebuild.sh Version 1.1 Copyright (C) 1999 Logistic Software Limited.
#
USER=lsl
GROUP=lsl

export USER GROUP

#
# Step 1:
#
# Make Copies Of Rebuild Logs

LOG_DIR=$PROG_PATH/BIN/LOG

mv -f $LOG_DIR/rebuild.log4 $LOG_DIR/rebuild.log5 2>/dev/null
mv -f $LOG_DIR/rebuild.log3 $LOG_DIR/rebuild.log4 2>/dev/null
mv -f $LOG_DIR/rebuild.log2 $LOG_DIR/rebuild.log3 2>/dev/null
mv -f $LOG_DIR/rebuild.log1 $LOG_DIR/rebuild.log2 2>/dev/null
mv -f $LOG_DIR/rebuild.log  $LOG_DIR/rebuild.log1 2>/dev/null

# Step 2:
#
# Run Logistic Rebuild Program

echo "Rebuilds started at `date`" > $LOG_DIR/rebuild.log
echo "Disc Usage at start is " >> $LOG_DIR/rebuild.log
echo "`df` " >> $LOG_DIR/rebuild.log

$PROG_PATH/BIN/UTILS/rebuild -a -c -p

# Step 3
#
# Reset permissions on files

cd $PROG_PATH/DATA/data.dbs

for fl in `ls`
do
        chmod 666 $fl 2> /dev/null
        chown $OWNER $fl 2> /dev/null
        chgrp $GROUP $fl 2> /dev/null

        echo "Permissions Have Been Reset For $fl"
done

echo "Rebuilds finished at `date`" >> $LOG_DIR/rebuild.log

echo "Disc Usage at end is" >> $LOG_DIR/rebuild.log
echo "`df`" >> $LOG_DIR/rebuild.log

chmod 666 $LOG_DIR/rebuild.log

#
# Step 4
#
# Print Rebuild Log

${DEF_QUEUE} $LOG_DIR/rebuild.log
