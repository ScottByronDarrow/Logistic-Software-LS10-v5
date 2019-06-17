/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: prtffzero.c,v 5.2 2001/08/09 09:19:40 scott Exp $
|  Program Name  : (sk_prtffzero.c)                                 |
|  Program Desc  : (Print FIFO records where cost price == 0.00 )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 24/03/87         |
|---------------------------------------------------------------------|
| $Log: prtffzero.c,v $
| Revision 5.2  2001/08/09 09:19:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:19:21  scott
| Update - LS10.5
|
| Revision 4.1  2001/04/23 10:41:12  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to remove usage of old include files.
|
=====================================================================*/
#define	CCMAIN 
char	*PNAME = "$RCSfile: prtffzero.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_prtffzero/prtffzero.c,v 5.2 2001/08/09 09:19:40 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<Costing.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct excfRecord	excf_rec;


	int	printerNo = 1;

	FILE	*ftmp;

/*=======================
| Function Declarations |
=======================*/
void 	HeadingOutput 		(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	ProcessData 		(void);
void 	PrintCategory 		(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	OpenDB ();


	if (argc > 1)
		printerNo = atoi (argv [1]);

	init_scr ();

	dsp_screen ("Processing : Stock Report (FIFO)", comm_rec.co_no, comm_rec.co_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "popen");

	HeadingOutput ();

	ProcessData ();

    fprintf (ftmp,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose (ftmp);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp, ".PI12\n");
	fprintf (ftmp, ".LP%d\n",printerNo);

	fprintf (ftmp, ".12\n");
	fprintf (ftmp, ".L112\n");

	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ESTOCK REPORT ZERO COST (FIFO)\n");
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".EBRANCH: %s \n",clip (comm_rec.est_name));
	fprintf (ftmp, ".EWAREHOUSE: %s \n",clip (comm_rec.cc_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "===========");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "===========");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "|   ITEM NUMBER    ");
	fprintf (ftmp, "|   I T E M    D E S C R I P T I O N       ");
	fprintf (ftmp, "|    DATE  ");
	fprintf (ftmp, "|  QUANTITY   ");
	fprintf (ftmp, "|     COST    |\n");

	fprintf (ftmp, "|------------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|----------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------|\n");
	fflush (ftmp);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	ReadMisc ();

	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (incc);
	CloseCosting ();
	abc_dbclose ("data");
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

void
ProcessData (
 void)
{
	char	oldGroup [13];
	char	newGroup [13];
	int		firstTime = TRUE,
			firstFifo;
	float	onHand	=	0.00;

	/*-----------------------
	|	read first incc	|
	-----------------------*/
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-28.28s"," ");
	cc = find_rec (incc,&incc_rec,GTEQ,"r");

	sprintf (oldGroup,"%12.12s"," ");

	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",incc_rec.sort + 12);

		cc = find_rec (inmr, &inmr_rec, COMPARISON,"r");

		if (cc || (inmr_rec.costing_flag [0] != 'F' && inmr_rec.costing_flag [0] != 'I'))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		if (strcmp (inmr_rec.supercession,"                "))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
			
		dsp_process (" Item: ", inmr_rec.item_no);
		fflush (stdout);

		sprintf (newGroup,"%1.1s%-11.11s",inmr_rec.inmr_class,inmr_rec.category);

		onHand = incc_rec.closing_stock;
		firstFifo = 1;
		cc = FindIncf (incc_rec.hhwh_hash,FALSE,"r");

		while (onHand > 0.00 && !cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			if (onHand < incfRec.fifo_qty)
			{
				incfRec.fifo_qty = onHand;
				onHand = 0.00;
			}
			else
				onHand -= incfRec.fifo_qty;

			if (incfRec.fifo_cost != 0.00)
			{
				cc = FindIncf (0L,FALSE,"r");
				continue;
			}

			if (firstFifo)
			{
				if (strcmp (newGroup,oldGroup))
				{
					strcpy (oldGroup,newGroup);
					PrintCategory (!firstTime);
					firstTime = FALSE;
				}

				fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
				fprintf (ftmp, "| %-40.40s ",inmr_rec.description);
				firstFifo = 0;
			}
			else
			{
				fprintf (ftmp, "| %-16.16s "," ");
				fprintf (ftmp, "| %-40.40s "," ");
			}
			fprintf (ftmp, "|%-10.10s",DateToString (incfRec.fifo_date));
			fprintf (ftmp, "|%12.2f ",incfRec.fifo_qty);
			fprintf (ftmp, "|%12.2f |\n",incfRec.fifo_cost);
			fflush (ftmp);
			cc = FindIncf (0L,FALSE,"r");
		}

		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
}

void
PrintCategory (
 int prt_extend)
{
	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	      strcpy (excf_rec.cat_desc, "No Category description found.");

	expand (err_str,excf_rec.cat_desc);

	fprintf (ftmp, ".PD| %-98.98s |\n",err_str);

	if (prt_extend)
		fprintf (ftmp, ".PA\n");
}
