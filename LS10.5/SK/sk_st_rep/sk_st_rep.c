/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: sk_st_rep.c,v 5.3 2001/08/09 09:20:03 scott Exp $
|  Program Name  : (sk_st_rep.c)
|  Program Desc  : (Print Stock Take Report)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 08/06/89         |
|---------------------------------------------------------------------|
| $Log: sk_st_rep.c,v $
| Revision 5.3  2001/08/09 09:20:03  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:56  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:33  scott
| Update - LS10.5
|
| Revision 4.1  2001/04/23 10:41:18  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to remove usage of old include files.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_st_rep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_st_rep/sk_st_rep.c,v 5.3 2001/08/09 09:20:03 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<Costing.h>

#define	GROUP		0

#define	SERIAL		 (inmr_rec.serial_item [0] == 'Y')
#define	MODE_OK		 (incc_rec.stat_flag [0] == mode [0])


#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct sttfRecord	sttf_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;
struct inexRecord	inex_rec;
struct excfRecord	excf_rec;

	double	totalCost [2];

	double	value 		= 0.00,
			averageCost = 0.00;

	int		printerNo		=	1,
			printType		=	0,
			envVarMultLoc 	=	0,
			envVarStExpDate =	0,
			envVarStUpZero	=	0;

	char	*inmr2 = "inmr2", 
			*data = "data";

	char	mode [2], 
			lower [13], 
			upper [13], 
			category [12], 
			*envVarSkIvalClass, 
 			*result, 
			envVarSkQtyMask [15],
			reportQty [30];

	FILE	*fout;
	FILE	*fsort;

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadCcmr 		(void);
void 	heading 		(void);
void 	Process 		(void);
void 	SaveLine 		(void);
void 	PrintSheet 		(char *);
void 	PrintTotal 		(int);
void 	PrintLine 		(char *);
void 	ProcessOther 	(void);
void 	PrintInex 		(void);
double 	DoubleValue 	(char *);
int  	ProcessTrans 	(long);
int  	ValidGroup 		(void);


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
		print_at (0,0,mlSkMess054, argv [0]);
		print_at (1,0,mlSkMess573);
		return (EXIT_FAILURE);
	}

	printerNo = atoi (argv [1]);

	sprintf (lower, "%-12.12s", argv [2]);
	sprintf (upper, "%-12.12s", argv [3]);

	if (argv [4] [0] != 'G' && argv [4] [0] != 'g')
	{
		print_at (0,0,mlSkMess573);
		return (EXIT_FAILURE);
	}

	printType = GROUP;
	sprintf (mode, "%-1.1s", argv [5]);

	sptr = chk_env ("MULT_LOC");
	if (sptr != (char *)0)
		envVarMultLoc = atoi (sptr);

	sptr = get_env ("ST_EXP_DATE");
	if (sptr != (char *)0)
		envVarStExpDate = atoi (sptr);

	sptr = get_env ("ST_UPZERO");
	if (sptr != (char *)0)
		envVarStUpZero = atoi (sptr);

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
	{	
		envVarSkIvalClass = strdup (sptr);
	}
	else
		envVarSkIvalClass = "ZKPN";
	upshift (envVarSkIvalClass); 

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (envVarSkQtyMask, "NNNNNNN.NNNNNN");
	else
		strcpy (envVarSkQtyMask, sptr);
	before = strlen (envVarSkQtyMask);
	sptr = strrchr (envVarSkQtyMask, '.');
	if (sptr)
		after = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (reportQty, "%%%df", before);
	else
		sprintf (reportQty, "%%%d.%df", before, after);

	OpenDB ();

	for (i = 0; i < 2; i++)
		totalCost [i] = 0.00;

	Process ();

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

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inmr2,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, (envVarStExpDate) ? "sttf_id_no2" : "sttf_id_no");

}

void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (incc);
	abc_fclose (inex);
	abc_fclose (excf);
	abc_fclose (sttf);
	CloseCosting ();
	abc_dbclose (data);
}

void
ReadCcmr (
 void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

void
heading (
 void)
{
	if ((fout = popen ("pformat", "w")) == 0)
		file_err (errno, "pformat", "popen");

	FindInsc (ccmr_rec.hhcc_hash, mode, FALSE);
	strcpy (err_str, (!strlen (inscRec.description)) ? " " : inscRec.description);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", printerNo);
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L150\n");
	fprintf (fout, ".ESTOCK TAKE REPORT EXPIRY DATE REPORT\n");
	fprintf (fout, ".EStockTake Selection [%s] %s\n", mode, clip (err_str));
	fprintf (fout, ".E%s %s %s %s\n", comm_rec.co_no, clip (comm_rec.co_name), comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".EW/H%s %s\n", comm_rec.cc_no, clip (comm_rec.cc_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fprintf (fout, ".EFrom %s To %s\n", lower, upper);

	fprintf (fout, ".R===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, "==================");
	fprintf (fout, "=================");
	if (envVarStExpDate)
		fprintf (fout, "============");
	fprintf (fout, "\n");

	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, "==================");
	fprintf (fout, "=================");
	if (envVarStExpDate)
		fprintf (fout, "============");
	fprintf (fout, "\n");

	fprintf (fout, "|   ITEM  NUMBER   ");
	fprintf (fout, "|  D E S C R I P T I O N                   ");
	fprintf (fout, "| BIN LOCATION ");
	fprintf (fout, "|   QUANTITY   ");
	fprintf (fout, "|     COST       ");
	fprintf (fout, "| EXTENDED  COST ");
	if (envVarStExpDate)
		fprintf (fout, "|EXPIRY DATE");
	fprintf (fout, "|\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------");
	if (envVarStExpDate)
		fprintf (fout, "|-----------");
	fprintf (fout, "|\n");
	fflush (fout);
}

void
Process (
 void)
{
	int	nx_item  = 1;

	dsp_screen (" Printing Stock Take Expiry Date Audit.", comm_rec.co_no, comm_rec.co_name);
	heading ();

	sprintf (category, "%-11.11s", " ");

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort, "%-12.12s%-16.16s", lower, " ");
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash && ValidGroup ())
	{
		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if ((result = strstr (envVarSkIvalClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}
		if (cc || strcmp (inmr_rec.co_no, comm_rec.co_no))
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}
		nx_item = 1;
		if (!SERIAL && MODE_OK)
		{
			if (nx_item)
			{
				ProcessOther ();
				nx_item = 0;
			}

			if (!ProcessTrans (incc_rec.hhwh_hash) && envVarStUpZero == 0)
			{
				cc = find_rec (incc, &incc_rec, NEXT, "r");
				continue;
			}
		}
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	if (fsort != (FILE *) 0)
		PrintSheet (category);
	PrintTotal (1);

	fprintf (fout, ".EOF\n");
	fclose (fout);
}

void
SaveLine (
 void)
{
	double	extend  = 0.00;
    char cBuffer [256];

	extend = (double) n_dec (sttf_rec.qty, inmr_rec.dec_pt);
	extend *= value;
	extend = twodec (extend);

	sprintf (cBuffer, "%-10.10s ", DateToString (sttf_rec.exp_date));
    sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%-16.16s ", inmr_rec.item_no);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%-10.10s ", sttf_rec.location);
	sort_save (fsort, cBuffer);

	sprintf (err_str, reportQty, n_dec (sttf_rec.qty, inmr_rec.dec_pt));
	sprintf (cBuffer, "%14s ", err_str);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%14.2f ", value);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%14.2f ", extend);
	sort_save (fsort, cBuffer);

	sprintf (cBuffer, "%07ld\n", inmr_rec.hhbr_hash);
	sort_save (fsort, cBuffer);

	dsp_process ("Item : ", inmr_rec.item_no);
}

int
ValidGroup (
 void)
{
	if (strncmp (incc_rec.sort, lower, 12) < 0)
		return (EXIT_SUCCESS);

	if (strncmp (incc_rec.sort, upper, 12) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

void
PrintSheet (
 char *cat_no)
{
	char	*sptr;
	char	*cptr;
	long	hhbrHash;

	fsort = sort_sort (fsort, "stockTakeReport");

	sptr = sort_read (fsort);

	if (sptr != (char *)0)
	{
		hhbrHash = atol (sptr + 82);
		
		inmr2_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (excf_rec.co_no, comm_rec.co_no);
			sprintf (excf_rec.cat_no, "%-11.11s", cat_no);
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc)
				strcpy (excf_rec.cat_desc, " ");

			cptr = expand (err_str, excf_rec.cat_desc);

			fprintf (fout, "|%1.1s%11.11s  %-123.123s|\n", inmr2_rec.inmr_class, cat_no, cptr);
		}
	}

	while (sptr != (char *)0)
	{
		hhbrHash = atol (sptr + 82);
		
		inmr2_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (!cc)
			PrintLine (sptr);

		sptr = sort_read (fsort);
	}

	PrintTotal (0);
	sort_delete (fsort, "stockTakeReport");
}

void
PrintTotal (
 int j)
{
	if (j == 0)
	{
		fprintf (fout, "|------------------");
		fprintf (fout, "|------------------------------------------");
		fprintf (fout, "|--------------");
		fprintf (fout, "|--------------");
		fprintf (fout, "|----------------");
		fprintf (fout, "|----------------");
		if (envVarStExpDate)
			fprintf (fout, "|-----------");
		fprintf (fout, "|\n");
	}

	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|              ");
	fprintf (fout, "|         Total for %s     ", (j == 0) ? "Group  " : "Company");
	fprintf (fout, "| %14.2f ", totalCost [j]);
	if (envVarStExpDate)
		fprintf (fout, "|           ");
	fprintf (fout, "|\n");
	fflush (fout);

	if (j == 0)
	{
		fprintf (fout, "|------------------");
		fprintf (fout, "|------------------------------------------");
		fprintf (fout, "|--------------");
		fprintf (fout, "|--------------");
		fprintf (fout, "|----------------");
		fprintf (fout, "|----------------");
		if (envVarStExpDate)
			fprintf (fout, "|-----------");
		fprintf (fout, "|\n");
	}
	fflush (fout);

	totalCost [j]   = 0.00;
}

void
PrintLine (
 char *sptr)
{
	fprintf (fout, "| %-16.16s ", inmr2_rec.item_no);
	fprintf (fout, "| %-40.40s ", inmr2_rec.description);
	fprintf (fout, "|  %-10.10s  ", sptr + 26);
	fprintf (fout, "|%14.14s", sptr + 37);
	fprintf (fout, "| %14.14s ", sptr + 52);
	fprintf (fout, "| %14.14s ", sptr + 67);
	if (envVarStExpDate)
		fprintf (fout, "| %-8.8s  ", sptr);
	fprintf (fout, "|\n");

	PrintInex ();
	fflush (fout);

	totalCost [0] += DoubleValue (sptr + 67);
	totalCost [1] += DoubleValue (sptr + 67);
}

/*=========================
| Process transaction file|
=========================*/
int
ProcessTrans (
	long	hhwhHash)
{
	int		transactionFound = FALSE;

	sttf_rec.hhwh_hash = hhwhHash;
	strcpy (sttf_rec.location, "          ");
	if (envVarStExpDate)
		sttf_rec.exp_date = 0L;
	cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
	while (!cc && sttf_rec.hhwh_hash == hhwhHash)
	{
		transactionFound = TRUE;

		/*----------------------------------------
		| Print Categ header on change of categ. |
		----------------------------------------*/
		if (strcmp (category, inmr_rec.category))
		{
			if (strcmp (category, "           "))
				PrintSheet (category);

			fsort = sort_open ("stockTakeReport");
			strcpy (category, inmr_rec.category);
		}

		SaveLine ();

		cc = find_rec (sttf, &sttf_rec, NEXT, "r");
	}
	return (transactionFound);
}

/*====================================
| Main Processing Routine for Stock. |
====================================*/
void
ProcessOther (
 void)
{
	value = 0.00;
	averageCost = 0.00;
	switch (inmr_rec.costing_flag [0])
	{
	case 'S':
		averageCost = FindInsfValue (incc_rec.hhwh_hash, TRUE);
		value = out_cost (averageCost, inmr_rec.outer_size);
		break;

	case 'F':
		averageCost = FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						n_dec (incc_rec.closing_stock, inmr_rec.dec_pt),
						TRUE, 
						TRUE,
						inmr_rec.dec_pt
					);
		value = out_cost (averageCost, inmr_rec.outer_size);
		break;

	case 'I':
		averageCost = FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						n_dec (incc_rec.closing_stock, inmr_rec.dec_pt),
						TRUE, 
						FALSE,
						inmr_rec.dec_pt
					);
		value = out_cost (averageCost, inmr_rec.outer_size);
		break;

	case 'L':
	case 'A':
	case 'P':
	case 'T':
		averageCost = FindIneiCosts 
					(
						inmr_rec.costing_flag,
						comm_rec.est_no,
						inmr_rec.hhbr_hash
					);
		value = out_cost (averageCost, inmr_rec.outer_size);
		break;

	default:
		break;
	}
	value = twodec (value);
}

double	
DoubleValue (
 char *str)
{
	char	val [15];
	
	sprintf (val, "%-14.14s", str);
	return (atof (val));
}

void
PrintInex (
 void)
{

	inex_rec.hhbr_hash = inmr2_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr2_rec.hhbr_hash )
	{
		fprintf (fout, "| %-16.16s ", " ");
		fprintf (fout, "| %-40.40s ", inex_rec.desc);
		fprintf (fout, "|  %-10.10s  ", " " );
		fprintf (fout, "|%14.14s", " ");
		fprintf (fout, "| %14.14s ", " ");
		fprintf (fout, "| %14.14s ", " ");
		fprintf (fout, "| %-8.8s  ", " ");
		fprintf (fout, "|\n");
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}
