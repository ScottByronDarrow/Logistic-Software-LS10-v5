PROG_PATH=/usr/LS10.5
export PROG_PATH
DBPATH=$PROG_PATH/DATA
PATH=$PATH:$PROG_PATH/BIN/SO:$PROG_PATH/BIN/SK:$PROG_PATH/BIN/UTILS:$PROG_PATH/BIN/MENU

LOG=/tmp/SelBgProcs.log

export DBPATH PATH

#----------------------------------------
# Check for the existence of so_bgcalc
#----------------------------------------
bg1=`ps -ef | grep so_bgcalc | grep -v grep | wc -l`
if [ $bg1 -lt 1 ]
then
    echo "Checking BG so_bgcalc NOT running, starting..." `date` >> $LOG
	bg_ctrl 1 so_bgcalc
fi
#----------------------------------------
# Check for the existence of so_bgstkup
#----------------------------------------
bg2=`ps -ef | grep so_bgstkup | grep -v grep | wc -l`
if [ $bg2 -lt 1 ]
then
    echo "Checking BG so_bgstkup NOT running, starting..." `date` >> $LOG
	bg_ctrl 1 so_bgstkup
fi

#----------------------------------------
# Check for the existence of so_pscreat
#----------------------------------------
bg3=`ps -ef | grep so_pscreat | grep -v grep | wc -l`
if [ $bg3 -lt 1 ]
then
    echo "Checking BG so_pscreat NOT running, starting..." `date` >> $LOG
	bg_ctrl 1 so_pscreat
fi

#----------------------------------------
# Check for the existence of sk_bgmove
#----------------------------------------
bg4=`ps -ef | grep sk_bgmove | grep -v grep | wc -l`
if [ $bg4 -lt 1 ]
then
    echo "Checking BG sk_bgmove NOT running, starting..." `date` >> $LOG
	bg_ctrl 1 sk_bgmove
fi

#----------------------------------------
# Check for the existence of sk_bgfutrcst
#----------------------------------------
#bg5=`ps -ef | grep sk_bgfutrcst | grep -v grep | wc -l`
#if [ $bg5 -lt 1 ]
#then
#    echo "Checking BG sk_bgfutrcst NOT running, starting..." `date` >> $LOG
#	bg_ctrl 1 sk_bgfutrcst
#fi
