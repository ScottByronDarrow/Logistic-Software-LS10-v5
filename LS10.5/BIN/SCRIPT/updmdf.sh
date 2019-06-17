:
#	This script is designed to update menu securities from <a>

if [ "$PROG_PATH" = "" ]
then
	echo "Variable PROG_PATH Not set"
	exit
fi

clear
echo "\n\n\n\n\t PROG_PATH has been set to $PROG_PATH"
echo "\n\n\t Press Any Key To Continue"
read ans

cd $PROG_PATH/BIN/MENUSYS

for fl in `ls *.mdf`
do
	echo "Processing $fl"
	sed -e 's/\<a\>/\<a|b\>/g' $fl > temp
	mv temp $fl
	echo "$fl complete"
done

#	Set File permissions

$PROG_PATH/BIN/UTILS/UNI *.mdf

cd $PROG_PATH/BIN/MENUSYS/SUB_MENU

for fl in `ls *.mdf`
do
	echo "Processing $fl"
	sed -e 's/\<a\>/\<a|b\>/g' $fl > temp
	mv temp $fl
	echo "$fl complete"
done

#	Set File permissions

$PROG_PATH/BIN/UTILS/UNI *.mdf




