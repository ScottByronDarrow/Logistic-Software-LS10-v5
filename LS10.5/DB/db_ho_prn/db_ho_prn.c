/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_ho_prn.c,v 5.3 2001/11/22 08:22:22 scott Exp $
|  Program Name  : (db_ho_prn.c)
|  Program Desc  : (Head Office Customer Breakdown Report)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 22/04/87         |
|---------------------------------------------------------------------|
| $Log: db_ho_prn.c,v $
| Revision 5.3  2001/11/22 08:22:22  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_ho_prn.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_ho_prn/db_ho_prn.c,v 5.3 2001/11/22 08:22:22 scott Exp $";

#include <ml_std_mess.h>	
#include <ml_db_mess.h>	
#include <pslscr.h>	
#include <dsp_screen.h>
#include <dsp_process.h>

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;

	char	*data  = "data",
			*cumr2 = "cumr2";

	/*============================ 
	| Local & Screen Structures. |
	============================*/
	int		printerNo 	= 1,
			envDbMcurr	= 0;
	
	FILE	*fout;

void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Process 		(void);
void 	ProcessCumr 	(void);
int 	OpenAudit 		(void);
void 	CloseAudit 		(void);
	
/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	init_scr ();

	if (argc > 1)
		printerNo = atoi (argv [1]);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Open main database files.
	 */
	OpenDB ();

	dsp_screen (" Print Customers to Head Office Breakdown ",comm_rec.co_no,comm_rec.co_name);
	
	Process ();

	shutdown_prog ();
    return (EXIT_SUCCESS);

}


/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);

	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);

	abc_dbclose (data);
}

void
Process (void)
{
	OpenAudit ();

	strcpy (cumr_rec.co_no,comm_rec.co_no);
	strcpy (cumr_rec.est_no,"  ");
	strcpy (cumr_rec.dbt_no,"      ");

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		/*
		 * master customer
		 */
		if (cumr_rec.ho_dbt_hash == 0L)
		{
			cumr2_rec.ho_dbt_hash = cumr_rec.hhcu_hash;
			ProcessCumr ();
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	CloseAudit ();
}

void
ProcessCumr (void)
{
	int		firstCumr = TRUE;

	cc = find_rec (cumr2, &cumr2_rec, COMPARISON, "r");
	while (!cc && cumr2_rec.ho_dbt_hash == cumr_rec.hhcu_hash)
	{
		if (firstCumr)
		{
			dsp_process (" Master Customer: ",cumr_rec.dbt_name);

			fprintf (fout,".LRP5\n");
			fprintf (fout,"          |--------");
			fprintf (fout,"|--------");
			fprintf (fout,"-------------------------------------------|");
			if (envDbMcurr)
				fprintf (fout,"---------|\n");
			else
				fprintf (fout,"\n");

			fprintf (fout, "          | %-6.6s ",cumr_rec.dbt_no);
			fprintf (fout, "| %-49.49s |",cumr_rec.dbt_name);
			if (envDbMcurr)
				fprintf (fout, "   %-3.3s   |\n",cumr_rec.curr_code);
			else
				fprintf (fout,"\n");

			fprintf (fout,"          |--------");
			fprintf (fout,"|--------");
			fprintf (fout,"-------------------------------------------|");
			if (envDbMcurr)
				fprintf (fout,"---------|\n");
			else
				fprintf (fout,"\n");

			firstCumr = FALSE;
		}
		fprintf (fout, "          | %-6.6s "," ");
		fprintf (fout, "| %-6.6s ",cumr2_rec.dbt_no);
		fprintf (fout, "| %-40.40s |",cumr2_rec.dbt_name);
		if (envDbMcurr)
			fprintf (fout,"         |\n");
		else
			fprintf (fout,"\n");
		fflush (fout);

		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
}

int
OpenAudit (void)
{
	if ((fout = popen ("pformat","w")) == NULL) 
	{
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);
		return (EXIT_FAILURE);
	}

	fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout,".LP%d\n",printerNo);
	fprintf (fout,".9\n");
	fprintf (fout,".L82\n");
	fprintf (fout,".ECUSTOMER TO HEAD OFFICE BREAKDOWN REPORT\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_short));
	fprintf (fout,".Eas at %s\n",SystemTime ());
	fprintf (fout,".B2\n");
	fprintf (fout,".EBranch: %s\n",clip (comm_rec.est_name));

	fprintf (fout, ".R          =========");
	fprintf (fout, "=========");
	fprintf (fout, "============================================");
	if (envDbMcurr)
		fprintf (fout, "==========\n");
	else
		fprintf (fout, "\n");

	fprintf (fout, "          =========");
	fprintf (fout, "=========");
	fprintf (fout, "============================================");
	if (envDbMcurr)
		fprintf (fout, "==========\n");
	else
		fprintf (fout, "\n");

	fprintf (fout, "          | DBT NO ");
	fprintf (fout, "|        ");
	fprintf (fout, "    D E B T O R         N A M E            |");
	if (envDbMcurr)
		fprintf (fout, "CURR CODE|\n");
	else
		fprintf (fout, "\n");

	fflush (fout);
	return (EXIT_SUCCESS);
}

/*
 *	Routine to close the audit trail output file.
 */
void
CloseAudit (void)
{
	fprintf (fout,".EOF\n");
	pclose (fout);
}

