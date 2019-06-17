/*========================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                            |
|========================================================================|
|                                                                        |
|       Program:        lc_check.c                                       |
|       Author:         Trevor van Bremen                                |
|       Description:    This program reads the LICENSE file that is      |
|                       located in $PROG_PATH, decodes it, and returns   |
|                       an int value.                                    |
|       Parameters:     des_rec  - Structure pointer for psl_decrypt()   |
|                       lic_rec  - Structure pointer to hold name etc    |
|------------------------------------------------------------------------|
|  Date Modified : (21.09.94)      | Modified by : Jonathan Chen         |
|                                                                        |
|  (21.09.94) : Replaced return codes with symbolic constants. Moved     |
|             : license writes to lc_io.c                                |
|                                                                        |
========================================================================*/
#include	<std_decs.h>

int
lc_check (
 struct DES_REC	*des_rec,
 struct LIC_REC	*lic_rec)
{
	long	today;

	lc_read (lic_rec);		/* read in license */

	strcpy (des_rec->passwd, lic_rec->passwd);
	psl_decrypt (des_rec);

	lic_rec->max_usr = des_rec->max_usr;
	lic_rec->max_trm = des_rec->max_trm;
	lic_rec->expiry  = des_rec->expiry;

	if (des_rec -> user_key != lc_i_no ())
		return (LICENSE_BAD);		/* inode-mismatch */

	today = TodaysDate ();
	if (today <= lic_rec -> expiry || !lic_rec -> expiry)
	{
		if ((today + 14) <= lic_rec -> expiry || !lic_rec -> expiry)
			return (LICENSE_OK);
		return (LICENSE_DYING);		/* Almost expired!! */
	}
	return (LICENSE_DEAD);			/* Already expired!! */
}
