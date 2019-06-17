/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( qc_recalc.c    )                                 |
|  Program Desc  : ( Recalculation of the current QC qty as       )   |
|                  ( reflected by qchr/qcln records.              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ccmr, qchr, inmr, incc,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  inmr, incc,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees.| Date Written  : 12/09/1994       |
|---------------------------------------------------------------------|
|  Date Modified : (01/11/1994)    | Modified  by  : Aroha Merrilees. |
|  Date Modified : (03/11/1994)    | Modified  by  : Aroha Merrilees. |
|                                                                     |
|  Comments      :                                                    |
|  (01/11/94)    : PSL 11299 - upgrade to version 9 - mfg cutover -   |
|                : no code changes                                    |
|  (03/11/94)    : PSL 11299 - check QC_APPLY                         |
|  (18/08/99)    : 1722 - check QC_APPLY                         |
|                                                                     |
| $Log: qc_recalc.c,v $
| Revision 5.2  2001/08/09 09:16:27  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:38  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:51  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:08:50  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/11/04 08:44:16  scott
| Updated to fix warnings due to -Wall flag.
|
| Revision 1.8  1999/09/29 10:12:26  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/21 05:26:37  scott
| Updated from Ansi Project
|
| Revision 1.6  1999/09/11 06:10:45  alvin
| Removed std_decs.h
|
| Revision 1.5  1999/09/09 06:16:02  alvin
| Converted to ANSI format.
|
| Revision 1.4  1999/06/18 05:55:47  scott
| Updated to add log file for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qc_recalc.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QC/qc_recalc/qc_recalc.c,v 5.2 2001/08/09 09:16:27 scott Exp $";

#define		NO_SCRGEN
#define		MOD	10

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};

	int	comm_no_fields = 3;

	struct tag_commRecord
	{
		int		term;
		char	tco_no [3];
		char	tco_name [41];
	} comm_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list [] =
	{
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
	};

	int	ccmr_no_fields = 4;

	struct tag_ccmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		long	hhcc_hash;
	} ccmr_rec;

	/*============================================
	| Inventory QC Purchased Items Reveival File |
	============================================*/
	struct dbview qchr_list [] =
	{
		{"qchr_co_no"},
		{"qchr_br_no"},
		{"qchr_wh_no"},
		{"qchr_qc_centre"},
		{"qchr_hhbr_hash"},
		{"qchr_exp_rel_dt"},
		{"qchr_origin_qty"},
		{"qchr_rel_qty"},
		{"qchr_rej_qty"},
	};

	int	qchr_no_fields = 9;

	struct tag_qchrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		char	qc_centre [5];
		long	hhbr_hash;
		long	exp_rel_dt;
		float	origin_qty;
		float	rel_qty;
		float	rej_qty;
	} qchr_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_qc_qty"},
		{"inmr_qc_reqd"},
	};

	int	inmr_no_fields = 5;

	struct tag_inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		float	qc_qty;
		char	qc_reqd [3];
	} inmr_rec;

	/*======================================
	| Inventory warehouse/cost centre file |
	======================================*/
	struct dbview incc_list [] =
	{
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_hhwh_hash"},
		{"incc_qc_qty"},
	};

	int	incc_no_fields = 4;

	struct tag_inccRecord
	{
		long	hhcc_hash;
		long	hhbr_hash;
		long	hhwh_hash;
		float	qc_qty;
	} incc_rec;

	char	*data = "data",
			*comm = "comm",
			*ccmr = "ccmr",
			*qchr = "qchr",
			*inmr = "inmr",
			*incc = "incc";

/*==========================
| Function prototypes      |
==========================*/
int		main		(int argc, char * argv []);
void	OpenDB		(void);
void	CloseDB	(void);
int		ProcFile	(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		QC_APPLY;
	char	*sptr;

	QC_APPLY = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	if (!QC_APPLY)
		return (EXIT_SUCCESS);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	set_tty ();

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	dsp_screen (" Recalculating QC Quantity ",
			comm_rec.tco_no, comm_rec.tco_name);

	ProcFile ();

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_id_no");
	open_rec (qchr, qchr_list, qchr_no_fields, "qchr_id_no2");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_id_no");
	open_rec (incc, incc_list, incc_no_fields, "incc_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (qchr);
	abc_fclose (inmr);
	abc_fclose (incc);

	abc_dbclose (data);
}

/*=====================================================================
| Process whole inmr file for company. Calculate the QC quantity from |
| the qchr records.                                                   |
=====================================================================*/
int
ProcFile (void)
{
	float	qcQty,
			qcTmp;

	/*-----------------------
	| Read whole inmr file. |
	-----------------------*/
	strcpy (inmr_rec.co_no,		comm_rec.tco_no);
	strcpy (inmr_rec.item_no,	" ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "u");
	while (!cc &&
		!strcmp (inmr_rec.co_no, comm_rec.tco_no)) 
	{
		qcQty = 0.00;

		dsp_process ("Item No : ", inmr_rec.item_no);

		if (inmr_rec.qc_reqd [0] != 'Y')
		{
			abc_unlock (inmr);
			cc = find_rec (inmr, &inmr_rec, NEXT, "u");
			continue;
		}

		/*-------------------------------------------------------
		| Read ccmr records to find QC records.                 |
		-------------------------------------------------------*/
		strcpy (ccmr_rec.co_no,		comm_rec.tco_no);
		strcpy (ccmr_rec.est_no,	" ");
		strcpy (ccmr_rec.cc_no,		" ");
		cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
		while (!cc &&
			!strcmp (ccmr_rec.co_no,	comm_rec.tco_no))
		{
			qcTmp = 0.00;

			/*-------------------------------------------------------
			| Read qchr records to calculate QC quantity.           |
			-------------------------------------------------------*/
			strcpy (qchr_rec.co_no, ccmr_rec.co_no);
			strcpy (qchr_rec.br_no, ccmr_rec.est_no);
			strcpy (qchr_rec.wh_no, ccmr_rec.cc_no);
			qchr_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (qchr_rec.qc_centre, " ");
			qchr_rec.exp_rel_dt = 0L;
			cc = find_rec (qchr, &qchr_rec, GTEQ, "r");
			while (!cc &&
				!strcmp (qchr_rec.co_no, ccmr_rec.co_no) &&
				!strcmp (qchr_rec.br_no, ccmr_rec.est_no) &&
				!strcmp (qchr_rec.wh_no, ccmr_rec.cc_no) &&
				qchr_rec.hhbr_hash == inmr_rec.hhbr_hash)
			{
				qcTmp += qchr_rec.origin_qty;
				qcTmp -= qchr_rec.rel_qty;
				qcTmp -= qchr_rec.rej_qty;

				cc = find_rec (qchr, &qchr_rec, NEXT, "r");
			}

			/*-------------------------------------------------------
			| Update incc record with correct QC quantity.          |
			-------------------------------------------------------*/
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			if (!find_rec (incc, &incc_rec, EQUAL, "r"))
			{
				incc_rec.qc_qty = qcTmp;
				if ((cc = abc_update (incc, &incc_rec)))
					file_err (cc, incc, "DBUPDATE");
			}
			qcQty += qcTmp;

			cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
		}

		inmr_rec.qc_qty = qcQty;
		if ((cc = abc_update (inmr, &inmr_rec)))
			file_err (cc, inmr, "DBUPDATE");

		cc = find_rec (inmr, &inmr_rec, NEXT, "u");
	}
	abc_unlock (inmr);

	return (EXIT_SUCCESS);
}
