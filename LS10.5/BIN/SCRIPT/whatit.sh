:
# script to print listing of program version on a machine.
#  Modified 4/11/91 - to use PROG_PATH
#  and remove path from sysfiles.

# script to strip result of what into values
whatsp()
{ 
# setup defualts
PTYPE=" "
PVER=" "
PDATE="00/00/00"
PSCN=" N/A  "
# Save program name
PMOD="$1"
shift
# Save program name
PROG=`echo $1 |sed "s/://g"`
shift
# save off version info
for k in $*
do
    case $1 in
     PSLSCR) PSCN=$2
             ;;
     Logistic) PTYPE=$1
               PVER=$2
               PDATE=$3
			   ;;
     Pinnacle) PTYPE=$1
               PVER=$2
               PDATE=$3
               ;;
     PSL.???)  PTYPE=$1
               PVER=$2
               PDATE=$3
               ;;
     SEL.???)  PTYPE=$1
               PVER=$2
               PDATE=$3
               ;;
   esac
   shift
done
echo $PCLIENT"|"$PROG"|"$PMOD"|"$PTYPE"|"$PVER"|"$PDATE"|"$PSCN"|" >> /usr/tmp/vers.out
}

#SETUP

HOME="$PROG_PATH/BIN"
DIRS="GL OL RG TM AS PC TR BM SA TS CA UTILS CM CR PO DB DD MAIL SJ MENU PS SK FA SO FE MH QC FF MISC QT"
COMPARE=`file $HOME/MENU/menu |awk -F" " '{print $2}'`

cd $HOME
echo "List of program versions numbers" > /usr/tmp/what.out
echo "" > /usr/tmp/vers.out
echo "" > /usr/tmp/other.out

#PROGRAM

echo "Enter 3 Digit client code :\c"
read PCLIENT

for i in $DIRS
do
    if [ -d $i ]
    then
        echo "Directory = $i" 
        echo "Directory = $i" >> /usr/tmp/what.out
        cd $HOME/$i
        for j in `ls`
        do
            echo "Filename $j" >> /usr/tmp/what.out
            what $j |grep P >> /usr/tmp/what.out

            echo "Filename $j"
            if [ "$COMPARE" = `file $j |awk -F" " '{print $2}'` ]
            then
                whatsp $i $j `what $j |sed 's/PSL-/PSL./g'|sed 's/V-/ /g' |sed 's/-/ /g' |sed 's/(/ /g' |sed 's/)/ /g'`
            else
                echo "$i/$j" >> /usr/tmp/other.out
            fi

        done
     fi

cd $HOME
done

#	Create copy of database sys files
cd $PROG_PATH/DATA/data.dbs
tar cvf /usr/tmp/sys.out  sys*

#	Create copy of Screen Files
cd $PROG_PATH/BIN/SCN
tar cvf /usr/tmp/scn.out *.s

#	Create copy of Print Files
cd $PROG_PATH/BIN/PR_FILE
tar cvf /usr/tmp/pr_file.out *.p

#	Create copy of Environment Variables
unload_env > /usr/tmp/$PCLIENT.out

exit
