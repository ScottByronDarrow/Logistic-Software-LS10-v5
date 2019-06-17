/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( sk_nst_varpr.c)                                  |
|  Program Desc  : ( Print Stock Take Variation Report.   	      )   |	
| $Id: nst_varpr.c,v 5.3 2001/08/09 09:19:31 scott Exp $
|---------------------------------------------------------------------|
|  Access files  :  comm, incc,     , inei, sttf,     ,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 07/04/89         |
|---------------------------------------------------------------------|
|  Date Modified : (30/06/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (03/07/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (11/02/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (20/02/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/06/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (22/09/92)      | Modified  by  : Anneliese Allen. |
|  Date Modified : (22/09/92)      | Modified  by  : Anneliese Allen. |
|  Date Modified : (18/06/93)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (09/02/94)      | Modified  by  : Aroha Merrilees. |
|  Date Modified : (31/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (11/12/98)      | Modified  by  : Ronnel Amanca.   |
|                :                                                    |
|                                                                     |
|  Comments      : Fix prog aborting with "Error in opening sortfile" |
|                : when no incc recs are found for current hhcc_hash &|
|                : selected stkmode.                                  |
|    (03/07/89)  : Fix core dump -caused by division by 0 ,in         |
|                :                                                    |
|                : (11/02/91) - Updated to fix value when not enough  |
|                :              fifo's.                               |
|                : (20/02/91) - Updated to fix diff in value when run |
|                :              by locations.                         |
|                :                                                    |
|                : (16/06/92) - exclude values of SK_IVAL_CLASS from  |
|                :              stock take  DFH 7096                  |
|                :                                                    |
|                : (22/09/92) - print group at the top of each group  |
|                :              rather than just the top of each page |
|                :                                                    |
|                : (22/09/92) - removed if (FIFO_ERR) stmt as it was  |
|                :              overwriting values just calculated by |
|                :              FindFifoCost function. Removed from    |
|                :              FIFO and LIFO cases. HMQ 7797         |
|                :                                                    |
| (22/09/92)     : Updated to use out_cost ().   DPL-8837             |
|                :                                                    |
| (09/02/94)     : PSL 10366 - Upgrade from ver7 to ver9, ability to  |
|                : show upto 6 decimal places.                        |
|                :                                                    |
| (31/03/94)     : HGP 10469. Removal of $ signs.                     |
| (11/12/98)     : Modified to expand the lenght of string in f_val   |
|                :                                                    |
|                                                                     |
| $Log: nst_varpr.c,v $
| Revision 5.3  2001/08/09 09:19:31  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:32  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:19  scott
| Update - LS10.5
|
| Revision 4.0  2001/03/09 02:38:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:30  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:51  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/09 02:28:29  scott
| General maintenance - added app.schema
|
| Revision 2.0  2000/07/15 09:11:34  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.19  1999/11/19 05:27:53  scott
| Updated for get_mend and get_mbeg
|
| Revision 1.18  1999/11/11 09:17:41  ronnel
| 11/11/1999 SC2089 Modified to print correct result on the variance report by item, by location and by group.
|
| Revision 1.17  1999/10/27 06:50:05  alvin
| Check-in Vij's ANSIfied code.
|
| Revision 1.13  1999/06/20 05:20:26  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: nst_varpr.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nst_varpr/nst_varpr.c,v 5.3 2001/08/09 09:19:31 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<Costing.h>
#include 	<ml_sk_mess.h>
#include 	<ml_std_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>

#define	GROUP		0
#define	ITEM 		1
#define	LOCN		2

#define	BY_ITEM		(reportPrintType == ITEM)
#define	BY_MLOC		(reportPrintType == LOCN && mult_loc)

#define	SERIAL		(inmr_rec.serial_item [0] == 'Y')
#define	MODE_OK		(incc_rec.stat_flag [0] == mode [0])

#define	FIFO 		(inmr_rec.costing_flag [0] == 'F')
#define	LIFO 		(inmr_rec.costing_flag [0] == 'I')
#define	LCST 		(inmr_rec.costing_flag [0] == 'L')
#define	AVGE 		(inmr_rec.costing_flag [0] == 'A')

#define	LCL_DESC	p_types [reportPrintType]._desc
#define	LCL_INMR	p_types [reportPrintType]._inmr


#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct sttfRecord	sttf_rec;
struct sttfRecord	sttf2_rec;
struct excfRecord	excf_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;
struct inloRecord	inlo_rec;

	float	totalStockTake 	= 0.00,
			oldQty     		= 0.00,
			newQty     		= 0.00,
			totalCount [2],
			totQvar [2];

	double	oldValue   = 0.00,
			newValue   = 0.00,
			totVar [2];

	int		printerNumber;
	int		reportPrintType;
	int		mult_loc = 0;

	char	*inmr2	= "inmr2";
	char	*sttf2	= "sttf2";
	char	*data	= "data";

	char	lower [17],
			upper [17],
			mode [2];

	int		firstTime = TRUE;
	char	*inval_cls,
			*result,
			dflt_qty  [15],
			rep_qty  [30];

	struct	{
		char	*_desc;
		char	*_inmr;
	} p_types [] = {
		{ "GROUP   ", "inmr_id_no_3" },
		{ "ITEM NUMBER", "inmr_id_no" },
		{ "LOCATION", "inmr_id_no" },
	};

	FILE	*fout;
	FILE	*fsort;

#include <LocHeader.h>

/*=======================
| Function Declarations |
=======================*/
double 	DoubleValue 		(char *);
float 	FloatValue 			(char *);
int  	ProcessStockFile 	(void);
static 	int DoubleZero 		(double);
void	shutdown_prog 		(void);
void 	CheckLocations 		(void);
void 	CloseDB 			(void);
void 	MainProcessRoutine 	(void);
void 	OpenDB 				(void);
void 	PrintEndOfReport 	(void);
void 	PrintLine 			(void);
void 	PrintReportTotals 	(int);
void 	PrintSaveLine 		(void);
void 	PrintSheet 			(char *);
void 	PrintSingleLine 	(char *);
void 	ProcByGroup 		(void);
void 	ProcByItem 			(void);
void 	ProcByLocation 		(void);
void 	ProcStdLocation 	(void);
void 	ProcessOtherInfo 	(void);
void 	ProcessTransactions (long);
void 	ReadCcmr 			(void);
void 	ReportHeading 		(void);
void 	SaveLine 			(void);
void 	ValidPrintLocations (long);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	int		i,
			after,
			before;

	if (argc != 6)
	{
		print_at (0,0,mlSkMess510,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);

	sptr = get_env ("MULT_LOC");
	if (sptr != (char *)0)
		mult_loc = atoi (sptr);

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (dflt_qty, sptr);
	before = strlen (dflt_qty);
	sptr = strrchr (dflt_qty, '.');
	if (sptr)
		after = (int) ( (sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (rep_qty, "%%%df", before);
	else
		sprintf (rep_qty, "%%%d.%df", before, after);

	switch (argv [4] [0])
	{
	case 'G':
	case 'g':
		sprintf (lower, "%-12.12s", argv [2]);
		sprintf (upper, "%-12.12s", argv [3]);
		reportPrintType = GROUP;
		break;

	case 'I':
	case 'i':
		sprintf (lower, "%-16.16s", argv [2]);
		sprintf (upper, "%-16.16s", argv [3]);
		reportPrintType = ITEM;
		break;

	case 'L':
	case 'l':
		sprintf (lower, "%-10.10s", argv [2]);
		sprintf (upper, "%-10.10s", argv [3]);
		reportPrintType = LOCN;
		break;

	default:
		print_at (0,0,mlSkMess510,argv [0]);
		return (EXIT_FAILURE);
	}
	sprintf (mode, "%-1.1s", argv [5]);

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
	{	
		inval_cls = strdup (sptr);
	}
	else
		inval_cls = strdup ("ZKPN");

	upshift (inval_cls); 

	OpenDB ();

	for (i = 0; i < 2; i++)
	{
		totalCount	[i] = 0.00;
		totVar 		[i] = 0.00;
		totQvar 	[i] = 0.00;
	}
	MainProcessRoutine ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB ();
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	ReadCcmr ();

	abc_alias (inmr2, inmr);
	abc_alias (sttf2, sttf);

	open_rec (inmr, inmr_list,  INMR_NO_FIELDS, LCL_INMR);
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list,  INCC_NO_FIELDS, "incc_id_no");
	open_rec (excf, excf_list,  EXCF_NO_FIELDS, "excf_id_no");
	open_rec (sttf, sttf_list,  STTF_NO_FIELDS, "sttf_id_no");
	open_rec (sttf2,sttf_list,  STTF_NO_FIELDS, "sttf_id_no");
	open_rec (inum, inum_list,  INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inlo, inlo_list,  INLO_NO_FIELDS, "inlo_mst_id");

	OpenInei ();
	OpenLocation (ccmr_rec.hhcc_hash);
}

void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (incc);
	abc_fclose (excf);
	abc_fclose (sttf);
	abc_fclose (sttf2);
	abc_fclose (inum);
	abc_fclose (inlo);
	CloseLocation ();
	CloseCosting ();
	abc_dbclose (data);
}

void
ReadCcmr (
 void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose (ccmr);
}

void
ReportHeading (
 void)
{
	if ( (fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	FindInsc (ccmr_rec.hhcc_hash, mode, TRUE);
	
	strcpy (err_str, inscRec.description);

	fprintf (fout, ".START%s <%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".11\n");
	fprintf (fout, ".L154\n");
	fprintf (fout, ".EStock Take Variation Report by %s\n", LCL_DESC);
	fprintf (fout, ".EStockTake Selection  [%s] %s\n", mode, clip (err_str));
	fprintf (fout, ".E%s %s %s %s\n",
			comm_rec.co_no, clip (comm_rec.co_name),
			comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".EW/H%s %s\n", comm_rec.cc_no, clip (comm_rec.cc_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fprintf (fout, ".EFrom %s To %s\n", lower, upper);

	fprintf (fout, ".R===================================================");
	fprintf (fout, "===================================================");
	fprintf (fout, "==================================================\n");

	fprintf (fout, "===================================================");
	fprintf (fout, "===================================================");
	fprintf (fout, "==================================================\n");

	switch (reportPrintType)
	{
	case GROUP:
		fprintf (fout, "!    GROUP   ");
		fprintf (fout, "!  ITEM  NUMBER  ");
		break;

	case ITEM:
		fprintf (fout, "!  ITEM  NUMBER  ");
		fprintf (fout, "!    GROUP   ");
		break;

	case LOCN:
		fprintf (fout, "!  LOCATION  ");
		fprintf (fout, "!  ITEM  NUMBER  ");
		break;

	}

	fprintf (fout, "!         D E S C R I P T I O N          ");
	fprintf (fout, "! QTY ON HAND  ");
	fprintf (fout, "!FIFO COST ");
	fprintf (fout, "!LAST COST ");
	fprintf (fout, "! QTY  COUNTED ");
	fprintf (fout, "!   QTY  VAR   ");
	fprintf (fout, "!  VARIATION !\n");

	switch (reportPrintType)
	{
	case GROUP:
	case LOCN:
		fprintf (fout, "!------------");
		fprintf (fout, "!----------------");
		break;

	case ITEM:
		fprintf (fout, "!----------------");
		fprintf (fout, "!------------");
		break;
	}

	fprintf (fout, "!----------------------------------------");
	fprintf (fout, "!--------------");
	fprintf (fout, "!----------");
	fprintf (fout, "!----------");
	fprintf (fout, "!--------------");
	fprintf (fout, "!--------------");
	fprintf (fout, "!------------!\n");
	fflush (fout);
}

void
MainProcessRoutine (
 void)
{
	sprintf (err_str, "Printing Stock Take Variation by %s", LCL_DESC);
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	ReportHeading ();

	switch (reportPrintType)
	{
	case	GROUP:
		ProcByGroup ();
		break;

	case	ITEM:
		ProcByItem ();
		break;

	case	LOCN:
		if (mult_loc)
			ProcByLocation ();
		else
			ProcStdLocation ();
		break;
	}
}

/*=================================
| Process stock by product group. |
=================================*/
void
ProcByGroup (
 void)
{
	char	group [13];
	int		no_rec = 1;

	abc_selfield (incc, "incc_id_no_2");
	abc_selfield (inmr, "inmr_hhbr_hash");

	sprintf (group, "%-12.12s", " ");

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort, "%-12.12s%-16.16s", lower, " ");
	cc = find_rec ("incc", &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (strncmp (incc_rec.sort, upper, 12) > 0)
			break;

		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		result = strstr (inval_cls, inmr_rec.inmr_class);
		if (result)
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}

		if (!cc && !SERIAL && MODE_OK)
		{
			if (ProcessStockFile ())
			{
				dsp_process ("Item : ", inmr_rec.item_no);

				if (strncmp (group, incc_rec.sort, 12))
				{
					if (strcmp (group, "            "))
						PrintReportTotals (0);

					PrintSheet (incc_rec.sort);

					sprintf (group, "%-12.12s", incc_rec.sort);
				}
				no_rec = 0;
				PrintLine ();
			}
		}
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	if (!no_rec)
		PrintReportTotals (0);

	PrintEndOfReport ();
}

/*=========================
| Process items by Items. |
=========================*/
void
ProcByItem (
 void)
{
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", lower);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) && 
		strcmp (inmr_rec.item_no, upper) <= 0)
	{
		result = strstr (inval_cls, inmr_rec.inmr_class);
		if (result)
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		if (!SERIAL)
		{
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (!cc && MODE_OK)
			{
				if (ProcessStockFile ())
				{
					dsp_process ("Item : ", inmr_rec.item_no);
					PrintLine ();
				}
			}
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	PrintEndOfReport ();
}

/*======================================
| Process items by Standard Locations. |
======================================*/
void
ProcStdLocation (
 void)
{
	char	location [11];
	int		no_rec = 1;

	abc_selfield (incc, "incc_id_no_3");
	abc_selfield (inmr, "inmr_hhbr_hash");

	sprintf (location, "%-10.10s", " ");

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.location, "%-10.10s", lower);
	sprintf (incc_rec.sort, "%-28.28s", " ");
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (strncmp (incc_rec.location, upper, 10) > 0)
			break;

		result = strstr (inval_cls, inmr_rec.inmr_class);
		if (result)
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}

		if (!SERIAL && MODE_OK)
		{
			inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (ProcessStockFile ())
			{
				dsp_process ("Item : ", inmr_rec.item_no);

				if (strcmp (location, incc_rec.location))
				{
					if (strcmp (location, "          "))
						PrintReportTotals (0);

					strcpy (location, incc_rec.location);
				}
				no_rec = 0;
				PrintLine ();
			}
		}
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	if (!no_rec)
		PrintReportTotals (0);
	PrintEndOfReport ();
}

/*======================================
| Process items by Multiple Locations. |
======================================*/
void
ProcByLocation (
 void)
{
	fsort = sort_open ("nstvar");

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", " ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		result = strstr (inval_cls, inmr_rec.inmr_class);
		if (result)
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		if (!SERIAL)
		{
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (!cc && MODE_OK)
			{
				if (MULT_LOC && !SK_BATCH_CONT)
					CheckLocations ();

				ValidPrintLocations (incc_rec.hhwh_hash);
			}
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	PrintSaveLine ();
}

/*=================================
| Validate and process locations. |
=================================*/
void
ValidPrintLocations (
 long	hhwh_hash)
{
	char	new_str [16];
	char	old_str [16];

	memset(new_str, 0, sizeof (new_str));
	memset(old_str, 0, sizeof (old_str));

	inlo_rec.hhwh_hash = hhwh_hash;
	inlo_rec.hhum_hash = 0L;
	strcpy (inlo_rec.location, "         ");
	strcpy (inlo_rec.lot_no, "      ");
	cc = find_rec ("inlo", &inlo_rec, GTEQ, "r");
	while (!cc && inlo_rec.hhwh_hash == hhwh_hash)
	{
		if (strncmp (inlo_rec.location, upper,10) <= 0 &&
			strncmp (inlo_rec.location, lower,10) >= 0)
		{
			totalStockTake = 0.00;

			sttf_rec.hhwh_hash = inlo_rec.hhwh_hash;
			strcpy (sttf_rec.location, inlo_rec.location);
			cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
			while (!cc && sttf_rec.hhwh_hash == hhwh_hash)
			{
				if (sttf_rec.hhum_hash != inlo_rec.hhum_hash ||
					strcmp (sttf_rec.location, 	inlo_rec.location) ||
					strcmp (sttf_rec.lot_no, 	inlo_rec.lot_no))
				{
					cc = find_rec (sttf, &sttf_rec, NEXT, "r");
					continue;
				}
				totalStockTake += sttf_rec.qty;
	
				cc = find_rec (sttf, &sttf_rec, NEXT, "r");
			}

			sprintf (old_str,"%13.*f",inmr_rec.dec_pt,n_dec(inlo_rec.stake,inmr_rec.dec_pt));

			sprintf (new_str,"%13.*f",inmr_rec.dec_pt,n_dec(totalStockTake, inmr_rec.dec_pt));

			if (strcmp (old_str, new_str))
			{
				cc = FindInei (inmr_rec.hhbr_hash, comm_rec.est_no, "r");
				if (cc)
					ineiRec.last_cost	=	0.00;
					
				oldQty  = n_dec (inlo_rec.stake, inmr_rec.dec_pt);
				newQty  = n_dec (totalStockTake, inmr_rec.dec_pt);
				oldValue = ineiRec.last_cost * (double) oldQty;
				newValue = ineiRec.last_cost * (double) newQty;
				SaveLine ();
			}
			if (firstTime)
				dsp_process ("Item : ", inmr_rec.item_no);

			firstTime = 0;
		}
		cc = find_rec ("inlo", &inlo_rec, NEXT, "r");
	}
}

int
ProcessStockFile (
 void)
{

	char 	old_str [16];
	char	new_str [16];

	ProcessTransactions (incc_rec.hhwh_hash);

	/*------------------------------------------
	| Don't print if there's no stock variation|
	------------------------------------------*/
	sprintf (old_str,"%13.*f",inmr_rec.dec_pt,n_dec(incc_rec.stake, inmr_rec.dec_pt));
	sprintf (new_str,"%13.*f",inmr_rec.dec_pt,n_dec(totalStockTake,inmr_rec.dec_pt));

	if (!strcmp (old_str,new_str))
		return (EXIT_SUCCESS);

	ProcessOtherInfo ();
	return (EXIT_FAILURE);
}

/*=========================
| Process transaction file|
=========================*/
void
ProcessTransactions (
 long	hhwh_hash)
{
	totalStockTake = 0.00;

	if (BY_MLOC)
	{
		sttf_rec.hhwh_hash = hhwh_hash;
		strcpy (sttf_rec.location, inlo_rec.location);
		cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
		while (!cc && sttf_rec.hhwh_hash == hhwh_hash)
		{
			if (sttf_rec.hhum_hash != inlo_rec.hhum_hash ||
				strcmp (sttf_rec.location, inlo_rec.location) ||
				strcmp (sttf_rec.lot_no, inlo_rec.lot_no))
			{
				cc = find_rec (sttf, &sttf_rec, NEXT, "r");
				continue;
			}
			totalStockTake += n_dec (sttf_rec.qty, inmr_rec.dec_pt);

			cc = find_rec (sttf, &sttf_rec, NEXT, "r");
		}
	}
	else
	{
		sttf_rec.hhwh_hash = hhwh_hash;
		strcpy (sttf_rec.location, "          ");
		cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
		while (!cc && sttf_rec.hhwh_hash == hhwh_hash)
		{
			totalStockTake += n_dec (sttf_rec.qty, inmr_rec.dec_pt);

			cc = find_rec (sttf, &sttf_rec, NEXT, "r");
		}
	}
	return;
}

/*====================================
| Main Processing Routine for Stock. |
====================================*/
void
ProcessOtherInfo (void)
{
	double	workCost	= 0.00;
	float	diff 		= 0.00;

	oldQty  = n_dec (incc_rec.stake, inmr_rec.dec_pt);
	newQty  = n_dec (totalStockTake, inmr_rec.dec_pt);
	diff	 = n_dec (totalStockTake, inmr_rec.dec_pt) - 
			   n_dec (incc_rec.stake, inmr_rec.dec_pt);

	switch (inmr_rec.costing_flag [0])
	{
	case 	'A':
	case 	'L':
	case 	'P':
	case 	'T':
			workCost	= 	FindIneiCosts
							(
								inmr_rec.costing_flag,
								comm_rec.est_no,
								inmr_rec.hhbr_hash
							);
			oldValue = workCost * (double) oldQty;
			newValue = workCost	* (double) newQty;
		break;

	case	'F':
	case	'I':
			oldValue	=	CheckIncf 
							(
								incc_rec.hhwh_hash,
								TRUE,
								incc_rec.stake, 
								inmr_rec.dec_pt
							);
								
			newValue	=	CheckIncf 
							(
								incc_rec.hhwh_hash,
								TRUE,
								incc_rec.stake + diff, 
								inmr_rec.dec_pt
							);

			oldValue 	+= 	FindIncfValue 
							(
								incc_rec.hhwh_hash, 
								incc_rec.stake, 
								FALSE,
								(FIFO) ? TRUE : FALSE,
								inmr_rec.dec_pt
							);
			newValue 	+= 	FindIncfValue 
							(
								incc_rec.hhwh_hash, 
								incc_rec.stake + diff, 
								FALSE,
								(FIFO) ? TRUE : FALSE,
								inmr_rec.dec_pt
							);
		break;

	default:	
		break;
	}
	return;
}

void
PrintEndOfReport (void)
{
	PrintReportTotals (1);
	fprintf (fout, ".EOF\n");
	fclose (fout);
}

void
PrintLine (void)
{
	double	fifoCost  = 0.00;
	double	diffValue = 0.00;

	if (n_dec (incc_rec.stake, inmr_rec.dec_pt) != 0.00)
		fifoCost  = oldValue /
				(double) n_dec (incc_rec.stake, inmr_rec.dec_pt);

	fifoCost = out_cost (fifoCost, inmr_rec.outer_size);

	diffValue = newValue - oldValue;
	diffValue = out_cost (diffValue, inmr_rec.outer_size);
	if (DoubleZero (diffValue))
		diffValue = 0.00;

	dsp_process ("Item : ", inmr_rec.item_no);

	switch (reportPrintType)
	{
	case GROUP:
		fprintf (fout, "!%-11.11s ", " ");
		fprintf (fout, "!%-16.16s", inmr_rec.item_no);
		break;

	case ITEM:
		fprintf (fout, "!%-16.16s", inmr_rec.item_no);
		fprintf (fout, "!%-12.12s", incc_rec.sort);
		break;

	case LOCN:
		fprintf (fout, "! %-10.10s ", incc_rec.location);
		fprintf (fout, "!%-16.16s", inmr_rec.item_no);
		break;
	}

	fprintf (fout, "!%-40.40s", inmr_rec.description);
	sprintf (err_str, rep_qty, n_dec (incc_rec.stake, inmr_rec.dec_pt));
	fprintf (fout, "!%14s", err_str);
	fprintf (fout, "!%10.2f", fifoCost);
	fprintf (fout, "!%10.2f", ineiRec.last_cost);
	sprintf (err_str, rep_qty, totalStockTake);
	fprintf (fout, "!%14s", err_str);
	sprintf (err_str, rep_qty, (totalStockTake - n_dec (incc_rec.stake,
			inmr_rec.dec_pt)));
	fprintf (fout, "!%14s", err_str);
	fprintf (fout, "!%12.2f!\n", diffValue);
	fflush (fout);

	totalCount [0] += n_dec (totalStockTake, inmr_rec.dec_pt);
	totalCount [1] += n_dec (totalStockTake, inmr_rec.dec_pt);

	totVar [0] += diffValue;
	totVar [1] += diffValue;

	totQvar [0] += n_dec (totalStockTake, inmr_rec.dec_pt) -
			n_dec (incc_rec.stake, inmr_rec.dec_pt);
	totQvar [1] += n_dec (totalStockTake, inmr_rec.dec_pt) -
			n_dec (incc_rec.stake, inmr_rec.dec_pt);
}

void
PrintSheet (
 char	*category)
{
	char	*cptr;

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", category + 1);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		strcpy (excf_rec.cat_desc, " ");

	cptr = expand (err_str, excf_rec.cat_desc);

	fprintf (fout, "!%12.12s  %-136.136s!\n", category, cptr);
}

void
PrintReportTotals (
 int	j)
{
	if (j == 0 || BY_ITEM)
	{
		if (BY_ITEM)
		{
			fprintf (fout, "!----------------");
			fprintf (fout, "!------------");
		}
		else
		{
			fprintf (fout, "!------------");
			fprintf (fout, "!----------------");
		}
		fprintf (fout, "!----------------------------------------");
		fprintf (fout, "!--------------");
		fprintf (fout, "!----------");
		fprintf (fout, "!----------");
		fprintf (fout, "!--------------");
		fprintf (fout, "!--------------");
		fprintf (fout, "!------------!\n");
	}

	if (BY_ITEM)
	{
		fprintf (fout, "!                ");
		fprintf (fout, "!            ");
	}
	else
	{
		fprintf (fout, "!            ");
		fprintf (fout, "!                ");
	}
	fprintf (fout, "!                                        ");
	fprintf (fout, "!              ");
	fprintf (fout, "! TOTAL FOR %s  ", (j == 0) ? LCL_DESC : "COMPANY ");
	sprintf (err_str, rep_qty, n_dec (totalCount  [j], inmr_rec.dec_pt));
	fprintf (fout, "!%14s", err_str);
	sprintf (err_str, rep_qty, n_dec (totQvar  [j], inmr_rec.dec_pt));
	fprintf (fout, "!%14s", err_str);
	fprintf (fout, "!%12.2f!\n", totVar [j]);
	fflush (fout);

	if (j == 0)
	{
		fprintf (fout, "!------------");
		fprintf (fout, "!----------------");
		fprintf (fout, "!----------------------------------------");
		fprintf (fout, "!--------------");
		fprintf (fout, "!----------");
		fprintf (fout, "!----------");
		fprintf (fout, "!--------------");
		fprintf (fout, "!--------------");
		fprintf (fout, "!------------!\n");
	}
	fflush (fout);

	totalCount [j] = 0.00;
	totVar [j]   = 0.00;
	totQvar [j]  = 0.00;
}

float	
FloatValue (
 char	*str)
{
	char	val [15];
	
	sprintf (val, "%-14.14s", str);
	return ((float) (atof (val)));
}

double	
DoubleValue (
 char	*str)
{
	char	val [15];
	
	sprintf (val, "%-12.12s", str);
	return (atof (val));
}

void
SaveLine (
 void)
{
    char cBuffer  [256];
	double	fifoCost   = 0.00;
	double	diffValue	= 0.00;

	if (n_dec (inlo_rec.stake, inmr_rec.dec_pt) != 0.00)
		fifoCost = oldValue / (double) n_dec (inlo_rec.stake,inmr_rec.dec_pt);

	fifoCost = out_cost (fifoCost, inmr_rec.outer_size);

	newValue = totalStockTake * fifoCost;	
	oldValue = n_dec (inlo_rec.qty, inmr_rec.dec_pt) * fifoCost;

	diffValue = newValue - oldValue;
	diffValue = out_cost (diffValue, inmr_rec.outer_size);
	if (DoubleZero (diffValue))
		diffValue = 0.00;

	sprintf (cBuffer, "%-10.10s ", inlo_rec.location);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%-16.16s ", inmr_rec.item_no);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%07ld ", incc_rec.hhwh_hash);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%07ld ", inmr_rec.hhbr_hash);
	sort_save (fsort, cBuffer);

	sprintf (err_str, rep_qty, n_dec (inlo_rec.stake, inmr_rec.dec_pt));
	sprintf (cBuffer, "%14s ", err_str);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%10.2f ", fifoCost);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%10.2f ", ineiRec.last_cost);
	sort_save (fsort, cBuffer);

	sprintf (err_str, rep_qty, totalStockTake);
	sprintf (cBuffer, "%14s ", err_str);
	sort_save (fsort, cBuffer);

	sprintf (err_str, rep_qty, (totalStockTake -
			n_dec (inlo_rec.stake, inmr_rec.dec_pt)));
	sprintf (cBuffer, "%14s ", err_str);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%12.2f\n", diffValue);
	sort_save (fsort, cBuffer);
	fflush (fsort);
}

/*================================================
| Use sorted work file is by multiple locations. |
================================================*/
void
PrintSaveLine (
 void)
{
	char	*sptr;
	char	currentLocation [11];
	char	previousLocation [11];
	long	hhbrHash;

	fsort = sort_sort (fsort, "nstvar");

	sptr = sort_read (fsort);

	sprintf (previousLocation, "%-10.10s", sptr);

	while (sptr != (char *)0)
	{
		sprintf (currentLocation, "%-10.10s", sptr);
		if (strcmp (previousLocation, currentLocation))
		{
			PrintReportTotals (0);
			strcpy (previousLocation, currentLocation);
		}
		hhbrHash = atol (sptr + 36);
		
		inmr2_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (!cc)
			PrintSingleLine (sptr);

		sptr = sort_read (fsort);
	}
	PrintReportTotals (0);
	PrintEndOfReport ();
	sort_delete (fsort, "nstvar");
}

void
PrintSingleLine (
 char	*sptr)
{
	fprintf (fout, "! %-10.10s ", sptr);
	fprintf (fout, "!%-16.16s", sptr + 11);
	fprintf (fout, "!%-40.40s", inmr2_rec.description);
	fprintf (fout, "!%14.14s", sptr + 44);
	fprintf (fout, "!%10.10s", sptr + 59);
	fprintf (fout, "!%10.10s", sptr + 70);
	fprintf (fout, "!%14.14s", sptr + 81);
	fprintf (fout, "!%14.14s", sptr + 96);
	fprintf (fout, "!%12.12s!\n", sptr + 111);
	fflush (fout);

	totalCount [0] += FloatValue (sptr + 81);
	totalCount [1] += FloatValue (sptr + 81);

	totQvar [0] += FloatValue (sptr + 96);
	totQvar [1] += FloatValue (sptr + 96);

	totVar [0] += DoubleValue (sptr + 111);
	totVar [1] += DoubleValue (sptr + 111);
}

/*
 *	Minor support functions
 */
static int
DoubleZero (
 double	m)
{
	return (fabs (m) < 0.0001);
}

void
CheckLocations (
 void)
{
	long	workInloHash	=	0L;

	sttf2_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	sprintf (sttf2_rec.location, "%-10.10s", lower);
	cc = find_rec (sttf2, &sttf2_rec, GTEQ, "r");
	while (!cc && sttf2_rec.hhwh_hash == incc_rec.hhwh_hash &&
			strncmp (sttf2_rec.location, upper,10) <= 0)
	{
		cc = 	FindLocation 
				(
					sttf2_rec.hhwh_hash,
					sttf2_rec.hhum_hash,
					sttf2_rec.location,
					ValidLocations,
					&workInloHash
				);
		if (cc)
		{
			inum_rec.hhum_hash	=	sttf2_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (err_str, DateToString (0L));
				InLotLocation 
				(							
					sttf2_rec.hhwh_hash,	
					incc_rec.hhcc_hash,	
					sttf2_rec.hhum_hash,	
					inum_rec.uom,			
					"  N/A  ",
					"  N/A  ",				
					sttf2_rec.location,		
					" ",					
					err_str,
					(float) 0.00, 			
					inum_rec.cnv_fct,
					"A",
					0.00,
					0.00,
					0.00,
					0.00,
					0L
				);							
			}
		}
		cc = find_rec (sttf2, &sttf2_rec, NEXT, "r");
	}
}
