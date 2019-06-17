/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_int_add.c,v 5.3 2001/11/22 08:28:20 scott Exp $
|  Program Name  : (db_int_add.c)
|  Program Desc  : (Interest Addition)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 20/01/87         |
|---------------------------------------------------------------------|
| $Log: db_int_add.c,v $
| Revision 5.3  2001/11/22 08:28:20  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_int_add.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_int_add/db_int_add.c,v 5.3 2001/11/22 08:28:20 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cumrRecord	cumr_rec;
struct cuinRecord	cuin_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	double	tot_int = 0.00,
			grand_tot = 0.00;

	int		envDbCo = 0,
			lp_no = 1;

	FILE	*pp;
	char	branchNo [3];

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	shutdown_prog 	(void);
void 	HeadingOutput	(int);
void 	ReadMisc 		(void);
void 	ProcessFile		(void);
void 	EndReport 		(void);
	
int
main (
	int		argc,
	char	*argv [])
{
	envDbCo = atoi (get_env ("DB_CO"));

	OpenDB ();
	ReadMisc ();

	dsp_screen ("Interest Update To Customers Ledger.",
				comm_rec.co_no,comm_rec.co_name);

	HeadingOutput (lp_no);

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, "  ");
	strcpy (cumr_rec.dbt_no, "      ");

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		if (cumr_rec.int_flag [0] == 'Y')
			ProcessFile ();

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
		dsp_process ("Customer : ",cumr_rec.dbt_no);
	
	}
	EndReport ();
	pclose (pp);
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_inv_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
HeadingOutput (
	int		printerNo)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.dbt_date),PNAME);
	fprintf (pp,".LP%d\n",printerNo);
	fprintf (pp,".11\n");
	fprintf (pp,".L90\n");
	fprintf (pp,".E%s\n",comm_rec.co_name);
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n","INTEREST UPDATE AUDIT");
	fprintf (pp,".B1\n");
	fprintf (pp,".EAS AT : %s\n", SystemTime ());

	fprintf (pp, ".R===================================================");
	fprintf (pp, "======================\n");

	fprintf (pp, "===================================================");
	fprintf (pp, "======================\n");

	fprintf (pp, "|  CODE  |         D E B T O R   N A M E          |");
	fprintf (pp, "INVOICE |INTEREST AMT|\n");

	fprintf (pp, "|--------|----------------------------------------|");
	fprintf (pp, "--------|------------|\n");

	fprintf (pp,".PI12\n");
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no); 
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);

}

/*===========================
| Validate and print lines. |
===========================*/
void
ProcessFile (void)
{
	int		inv_count = 0;
	char	inv_no [9];

	double	tot_bal = 0.00;

	tot_bal = 	cumr_balance [1] + cumr_balance [2] + 
		  		cumr_balance [3] + cumr_balance [4] + cumr_balance [5];

	if (tot_bal == 0.00)
		return;
	
	tot_int = (tot_bal / 100) * (double) comr_rec.int_rate;
	tot_int /= 12;

	if (tot_int <= 0.00)
		return;

	while (cc == 0)
	{
		inv_count++;
		sprintf (inv_no, "IN%06d", inv_count);
		strcpy (cuin_rec.inv_no, inv_no);
		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
	}
	sprintf (inv_no, "IN%06d", inv_count);

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.pay_terms, cumr_rec.crd_prd);
	strcpy (cuin_rec.type, "1");
	strcpy (cuin_rec.co_no, comm_rec.co_no); 
	strcpy (cuin_rec.est, (!envDbCo) ? comm_rec.est_no 
					    : cumr_rec.est_no);

	strcpy (cuin_rec.dp, " 1");
	strcpy (cuin_rec.inv_no, inv_no);
	strcpy (cuin_rec.narrative, "INTEREST.           ");
	cuin_rec.date_of_inv = comm_rec.dbt_date;
	cuin_rec.date_posted = comm_rec.dbt_date;
	cuin_rec.disc = 0.00;
	cuin_rec.amt = tot_int;
	strcpy (cuin_rec.stat_flag, "0");
	
	cc = abc_add (cuin, &cuin_rec);
	if (cc)
		file_err (cc, cuin, "DBADD");
	
	fprintf (pp, "| %6.6s ",cumr_rec.dbt_no);
	fprintf (pp, "|%-40.40s",mid (cumr_rec.dbt_name,1,30));
	fprintf (pp, "|%-8.8s",cuin_rec.inv_no);
	fprintf (pp, "|%12.2f|\n",DOLLARS (tot_int));
	fflush (pp);

	cumr_balance [0] += tot_int;

	cc = abc_update (cumr, &cumr_rec);
	if (cc)
		file_err (cc, cumr, "DBUPDATE");
	
	grand_tot += tot_int;
	tot_int = 0.00;
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
EndReport (void)
{
    fprintf (pp, "|--------|----------------------------------------|");
    fprintf (pp, "--------|------------|\n");
    fprintf (pp, "|        |          **** GRAND TOTAL ****         |");
    fprintf (pp, "        |%12.2f|\n", DOLLARS (grand_tot));
	fprintf (pp,".EOF\n");
}
