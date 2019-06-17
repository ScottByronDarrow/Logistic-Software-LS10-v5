:
#	Author	Robert Hale	07/08/93

#	This script is designed to shutdown and reboot client's system.
#	The script is called from .profile and is normally executed when
#	night.sh weekly.sh processing is completed

#	Send First Warning To Logout

/etc/wall<<-!
	
	Please Logout As System To Be Shutdown In 30 Seconds
	
	!
sleep 30

#	Send Second And Final Warning

/etc/wall<<-!
	
	Logout Now Or Risk Your Files Being Damaged
	
	!
sleep 30

#	Initiate Shutdown After Ascertaining The Appropriate Method

TYPE=SCO				#	*** MODIFIY IF REQUIRED ***
case $TYPE in
	AIX)	
		cd /
		/etc/shutdown +1 -r
		kill -1 0
		;;
	ALT)
		cd /
		/etc/reboot
		/etc/shutdown -y -g0
		;;
	ICL)
		echo "\n\n\n\t\t Shutdown For $TYPE Not Yet Established"
		sleep 2
		;;
	MIPS)
		cd /
		/etc/shutdown -y -g5 -i6
		;;
	NCR)
		echo "\n\n\n\t\t Shutdown For $TYPE Not Yet Established"
		sleep 2
		;;
	SCO)
		cd /
		/etc/shutdown -y -g0 -i6
		;;
	*)
		echo "Shutdown Type Not Yet Established"
		sleep 2
		;;
esac

