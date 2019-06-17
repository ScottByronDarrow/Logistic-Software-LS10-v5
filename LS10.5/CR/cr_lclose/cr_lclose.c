/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_lclose.c,v 5.4 2001/12/06 23:51:38 scott Exp $
|  Program Name  : (cr_lclose.c   ) 
|  Program Desc  : (Close Suppliers Master File At Month End.   ) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cr_lclose.c,v $
| Revision 5.4  2001/12/06 23:51:38  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/06 09:36:32  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_lclose.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_lclose/cr_lclose.c,v 5.4 2001/12/06 23:51:38 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include    "schema"

struct commRecord   comm_rec;
struct sumrRecord   sumr_rec;

	int		envCrCo = 0;
	int		yearEnd = FALSE;
	char	branchNumber [3];

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB		 (void);
void	CloseDB		 (void);
void	Update		 (void);

int
main (
 int	argc,
 char *	argv [])
{
	int	mth = 0;

	envCrCo = atoi (get_env ("CR_CO"));

	OpenDB ();

	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	dsp_screen ("Closing Suppliers Ledger", comm_rec.co_no, comm_rec.co_name);

	DateToDMY (comm_rec.crd_date, NULL, &mth, NULL);

	if (mth == comm_rec.fiscal)
		yearEnd = TRUE;
	else
		yearEnd = FALSE;
	
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, "      ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (sumr_rec.est_no, branchNumber))
	{
		Update ();
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_dbclose ("data");
}

/*=========================
| Update Customer Periods. |
=========================*/
void
Update (void)
{
	sumr_rec.bo_per3 += sumr_rec.bo_per2;
	sumr_rec.bo_per2 = sumr_rec.bo_per1;
	sumr_rec.bo_per1 = sumr_rec.bo_curr;
	sumr_rec.bo_curr = 0.0;
	sumr_rec.mtd_exp = 0.0;
	if (yearEnd)
		sumr_rec.ytd_exp = 0.0;

	cc = abc_update (sumr, &sumr_rec);
	if (cc)
		file_err (cc, sumr, "DBUPDATE");

	dsp_process ("Supplier : ", sumr_rec.crd_no);
}
