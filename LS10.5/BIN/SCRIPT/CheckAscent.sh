:
# script to print listing of program version on a machine.
#  Modified 4/11/91 - to use PROG_PATH
#  and remove path from sysfiles.

HOME="$PROG_PATH/BIN"
DIRS="GL OL RG TM AS PC TR BM SA TS CA UTILS CM CR PO DB DD MAIL SJ MENU PS SK FA SO FE MH QC LRP MISC QT"

cd $HOME
echo "List of program versions numbers" > /tmp/what.out
echo "" > /tmp/vers.out
echo "" > /tmp/other.out

#PROGRAM

#echo "Enter 3 Digit client code :\c"
#read PCLIENT
PCLIENT=ASC

for i in $DIRS
do
    if [ -d $i ]
    then
        echo "Directory = $i" 
        echo "Directory = $i" >> /tmp/what.out
        cd $HOME/$i
        for j in `ls`
        do
            echo "Filename $j" >> /tmp/what.out
            psl_what $j >> /tmp/what.out

            echo "Filename $j"
        done
     fi

cd $HOME
done

#	Create copy of database sys files
#cd $PROG_PATH/DATA/data.dbs
#tar cvf /tmp/sys.out  sys*

#	Create copy of Screen Files
#cd $PROG_PATH/BIN/SCN
#tar cvf /tmp/scn.out *.s

#	Create copy of Print Files
#cd $PROG_PATH/BIN/PR_FILE
#tar cvf /tmp/pr_file.out *.p

#	Create copy of Environment Variables
unload_env > /tmp/$PCLIENT.out

tar cvf /tmp/Include.tar $PROG_PATH/INCLUDE/
tar cvf /tmp/Schema.tar $PROG_PATH/SCHEMA/
tar cvf /tmp/LIB.tar $PROG_PATH/LIB/BASE/

exit
