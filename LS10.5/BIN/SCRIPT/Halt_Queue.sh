if [ $# -eq 0 ]
then
	echo "Usage : `basename $0` <queue_name(s)>"
	exit
fi

echo "Halting Queue(s) ... \n"

while [ $# -gt 0 ]
do
	echo "Queue : $1"
	lpm -H -d $1
	shift
done
