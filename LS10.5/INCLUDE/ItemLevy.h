/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ItemLevy.h,v 5.0 2001/06/19 06:51:17 cha Exp $
|  Program Name  : (itemLevy.h) 
|  Program Desc  : (Item Levy Include)
|---------------------------------------------------------------------|
|  Date Written  : 20th May 2001   |  Author     : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: ItemLevy.h,v $
| Revision 5.0  2001/06/19 06:51:17  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/05/22 12:32:40  scott
| Updated to add a memset
|
| Revision 4.1  2001/05/21 00:51:54  scott
| New function for item levy
|
=====================================================================*/

/*======================
| Function prototypes. |
======================*/
static	int	ItemLevy	(long, char *, char *, Date);

/*==========================================================
| Find relevent inal record for item/branch/currency/date. |
==========================================================*/
static	int	
ItemLevy (
	long	hhbrHash,
	char	*brNo,
	char	*currCode,
	long	dbtDate)
{

	memset (&inal_rec, 0, sizeof (inal_rec));

	/*-------------------
	| Find inal record. |
	-------------------*/
	inal_rec.hhbr_hash = hhbrHash;
	strcpy (inal_rec.br_no, brNo);
	strcpy (inal_rec.curr_code, currCode);
	inal_rec.date_from = dbtDate;

	cc = find_rec (inal, &inal_rec, LTEQ, "r");
	while (!cc && 
	       !strcmp (inal_rec.br_no, brNo) &&
	       !strcmp (inal_rec.curr_code, currCode) &&
	       inal_rec.hhbr_hash == hhbrHash)
	{
		/*------------------------------------------
		| Date in sub range for promotional price. |
		------------------------------------------*/
		if (dbtDate >= inal_rec.date_from && 
		    dbtDate <= inal_rec.date_to)
			return (0);

		cc = find_rec (inal, &inal_rec, PREVIOUS, "r");
	}
	if (strcmp (brNo, "  "))
		return (ItemLevy (hhbrHash, "  ", currCode, dbtDate));

	memset (&inal_rec, 0, sizeof (inal_rec));
	return (1);
}
