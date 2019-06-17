/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_mbaldel.c)                                    |
|  Program Desc  : ( Deletion of Monthly Balance (inmb) Records.  )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: sk_mbaldel.c,v $
| Revision 5.1  2001/08/09 09:19:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:16:30  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:46  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:30  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:12  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.5  1999/11/03 07:32:09  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.4  1999/10/13 02:42:02  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.3  1999/10/08 05:32:33  scott
| First Pass checkin by Scott.
|
| Revision 1.2  1999/06/20 05:20:16  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_mbaldel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_mbaldel/sk_mbaldel.c,v 5.1 2001/08/09 09:19:05 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <get_lpno.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_inv_date"}
	};

	int comm_no_fields = 9;

	struct {
		int  	termno;
		char 	co_no[3];
		char 	co_name[41];
		char 	co_short[16];
		char 	est_no[3];
		char 	est_name[41];
		char 	est_short[16];
		char 	cc_no[3];
		long	inv_date;
	} comm_rec;

	/*====================================+
	 | Inventory Movements Balance Record |
	 +====================================*/
#define	INMB_NO_FIELDS	10

	struct dbview	inmb_list [INMB_NO_FIELDS] =
	{
		{"inmb_co_no"},
		{"inmb_hhbr_hash"},
		{"inmb_hhcc_hash"},
		{"inmb_date"},
		{"inmb_opening_bal"},
		{"inmb_avge_cost"},
		{"inmb_prev_cost"},
		{"inmb_last_cost"},
		{"inmb_std_cost"},
		{"inmb_latest_fifo"},
	};

	struct tag_inmbRecord
	{
		char	co_no [3];
		long	hhbr_hash;
		long	hhcc_hash;
		Date	date;
		float	opening_bal;
		Money	avge_cost;
		Money	prev_cost;
		Money	last_cost;
		Money	std_cost;
		Money	latest_fifo;
	}	inmb_rec;

	/*====================================+
	 | Inventory Master File Base Record. |
	 +====================================*/
#define	INMR_NO_FIELDS	2

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
	};

	struct tag_inmrRecord
	{
		char	item_no [17];
		long	hhbr_hash;
	}	inmr_rec;

	/*===========================================+
	 | Cost Centre/Warehouse Master File Record. |
	 +===========================================*/
#define	CCMR_NO_FIELDS	2

	struct dbview	ccmr_list [CCMR_NO_FIELDS] =
	{
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
	};

	struct tag_ccmrRecord
	{
		char	cc_no [3];
		long	hhcc_hash;
	}	ccmr_rec;

/*===========
 Table Names
============*/
static char
	*data = "data",
	*inmb = "inmb",
	*inmr = "inmr",
	*ccmr = "ccmr";

FILE	*fout;
int		lpno,
		days;

/*=======================
| Function Declarations |
=======================*/
void	ProcFile (void);
void	OpenAudit (void);
void	CloseAudit (void);
void	OpenDB (void);
void	CloseDB (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	if (argc != 3)
	{
		printf ("Usage: %s <LPNO> <days> \n", argv[0]);
		return (EXIT_FAILURE);
	}

	lpno = atoi (argv[1]);
	days = atoi (argv[2]);

	OpenDB();

	ProcFile ();

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (inmb, inmb_list, INMB_NO_FIELDS, "inmb_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_dbclose (data);

	abc_fclose (inmb);
	abc_fclose (inmr);
	abc_fclose (ccmr);
}

void
ProcFile (
 void)
{
	int		cc2;

	dsp_screen (" Deleting Records from Monthly Balance Table ", comm_rec.co_no,
														comm_rec.co_name);
	OpenAudit ();

	strcpy (inmb_rec.co_no, comm_rec.co_no);
	inmb_rec.date = 0L;
	for (cc = find_rec (inmb, &inmb_rec, GTEQ, "u");
			!cc &&
			!strcmp (inmb_rec.co_no, comm_rec.co_no) &&
			inmb_rec.date <= (comm_rec.inv_date - days);
			cc = find_rec (inmb, &inmb_rec, GTEQ, "u"))
	{
		strcpy (err_str, DateToString (inmb_rec.date));
		dsp_process ("Date : ", err_str);

		cc2 = find_hash (inmr, &inmr_rec, EQUAL, "r", inmb_rec.hhbr_hash);
		if (cc2)
			strcpy (inmr_rec.item_no, "**UNKNOWN ITEM**");

		cc2 = find_hash (ccmr, &ccmr_rec, EQUAL, "r", inmb_rec.hhcc_hash);
		if (cc2)
			strcpy (ccmr_rec.cc_no, "??");

		fprintf (fout, "| %2.2s | %-16.16s |%10.10s|   %9.2f   ",
								ccmr_rec.cc_no,
								inmr_rec.item_no,
								DateToString (inmb_rec.date),
								inmb_rec.opening_bal);

		fprintf (fout, "| %10.2f |  %10.2f | %10.2f |  %10.2f | %10.2f |\n",
								DOLLARS (inmb_rec.avge_cost),
								DOLLARS (inmb_rec.prev_cost),
								DOLLARS (inmb_rec.last_cost),
								DOLLARS (inmb_rec.std_cost),
								DOLLARS (inmb_rec.latest_fifo));

		cc = abc_delete (inmb);
		if (cc)
			file_err (cc, inmb, "DBADD");

		strcpy (inmb_rec.co_no, comm_rec.co_no);
		inmb_rec.date = 0L;
	}

	CloseAudit ();
}

void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", lpno);
	fprintf (fout, ".9\n");
	fprintf (fout, ".L130\n");
	fprintf (fout, ".EINVENTORY MONTHLY BALANCE DELETION AUDIT REPORT\n");
	fprintf (fout, ".E For Company %s\n", comm_rec.co_name);
	fprintf (fout, ".B2\n");

	fprintf (fout, ".R-----------------------------------");
	fprintf (fout, "-------------------------------------------");
	fprintf (fout, "-------------------------------------\n");

	fprintf (fout, "-----------------------------------");
	fprintf (fout, "-------------------------------------------");
	fprintf (fout, "-----------------------------------------\n");

	fprintf (fout, "| WH |    ITEM NUMBER   |          ");
	fprintf (fout, "|OPENING BALANCE|AVERAGE COST|PREVIOUS COST");
	fprintf (fout, "|  LAST COST |STANDARD COST| LATEST FIFO|\n");

	fprintf (fout, "|----|------------------|----------");
	fprintf (fout, "|---------------|------------|-------------");
	fprintf (fout, "|------------|-------------|------------|\n");

	fprintf (fout, ".PI12\n");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

