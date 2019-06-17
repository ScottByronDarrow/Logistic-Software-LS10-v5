#include	<FindSerial.h>
/*===================================================
|	updates the status of the insf record			|
|	returns:										|
|	0	all ok										|
|													|
|	1,												|
|	2,												|
|	999	error in dbupdate							|
|													|
|	1001,											|
|	1002,											|
|	1999	error in dbfind							|
===================================================*/
int	UpdateInsf (long, char *, char *, char *);

int	
UpdateInsf 
 (
	long	hhwh_hash, 
	char 	*serial_no, 
	char 	*old_status, 
	char 	*new_status)
{
	cc = FindInsf (hhwh_hash,serial_no,old_status,"u");
	if (cc)
		return (cc + 1000);

	strcpy (insf_rec.status,new_status);
	return (abc_update ("insf",&insf_rec));
}
