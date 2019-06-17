case $# in
	
	0)	echo "usage whereiz program name(s)"
	;;

	*)
	  while test $1
	    do
	       find / -name $1 -print
	       shift
	     done
esac
