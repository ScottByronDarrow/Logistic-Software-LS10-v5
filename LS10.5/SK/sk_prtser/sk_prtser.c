/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_prtser.c,v 5.2 2001/08/09 09:19:42 scott Exp $
|  Program Name  : (sk_prtser.c  )                                 |
|  Program Desc  : (Print Serial records                       )   |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 01/04/87         |
|---------------------------------------------------------------------|
| $Log: sk_prtser.c,v $
| Revision 5.2  2001/08/09 09:19:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:38  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_prtser.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_prtser/sk_prtser.c,v 5.2 2001/08/09 09:19:42 scott Exp $";

#include 	<ml_sk_mess.h>	
#include 	<ml_std_mess.h>	
#include 	<pslscr.h>	
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<Costing.h>	

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct excfRecord	excf_rec;


	int	lpno = 1;

	FILE	*ftmp;

	char	lower [13], 
			upper [13];

/*=======================
| Function Declarations |
=======================*/
void 	HeadingOutput 	 (void);
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	ProcessData 	 (void);
void 	PrintCategory 	 (int, double);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc < 4)
	{
		print_at (0,0,mlSkMess111,argv [0]);
		return (EXIT_FAILURE);
	}
	lpno = atoi (argv [1]);
	sprintf (lower,"%-12.12s",argv [2]);
	sprintf (upper,"%-12.12s",argv [3]);

	OpenDB ();

	init_scr ();

	dsp_screen ("Processing : Detailed Stock Valuation (Serial)", comm_rec.co_no, comm_rec.co_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

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
	fprintf (ftmp, ".LP%d\n",lpno);
	fprintf (ftmp, ".PI12\n");

	fprintf (ftmp, ".12\n");
	fprintf (ftmp, ".L109\n");

	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".EDETAILED STOCK VALUATION (SERIAL)\n");
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (ftmp, ".EWarehouse: %s \n",clip (comm_rec.cc_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "============================");
	fprintf (ftmp, "=================\n");

	fprintf (ftmp, "===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "============================");
	fprintf (ftmp, "=================\n");

	fprintf (ftmp, "|   ITEM NUMBER    ");
	fprintf (ftmp, "|   I T E M    D E S C R I P T I O N       ");
	fprintf (ftmp, "| S E R I A L   N U M B E R ");
	fprintf (ftmp, "|    C O S T    |\n");

	fprintf (ftmp, "===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "============================");
	fprintf (ftmp, "=================\n");
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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);

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

void
ProcessData (
 void)
{
	char	old_group [13];
	char	new_group [13];
	int		firstTime = TRUE;
	int		ser_printed;

	double	value 		= 0.00,
			groupValue 	= 0.00,
			totalValue 	= 0.00,
			workValue	= 0.00;

	/*-----------------------
	|	read first incc	|
	-----------------------*/
	fflush (ftmp);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-28.28s",lower);
	cc = find_rec (incc,&incc_rec,GTEQ,"r");

	sprintf (old_group,"%12.12s",lower);

	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash && strncmp (incc_rec.sort,upper,12) <= 0)
	{
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",incc_rec.sort + 12);

		cc = find_rec (inmr, &inmr_rec, COMPARISON,"r");

		if (cc || inmr_rec.costing_flag [0] != 'S')
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		if (cc)
			file_err (cc, inmr, "DBFIND");

		dsp_process (" Item: ", inmr_rec.item_no);

		sprintf (new_group,"%1.1s%-11.11s",inmr_rec.inmr_class,inmr_rec.category);

		/*=====================================
		| Process Serial Items That are free. | 
		=====================================*/
		cc = FindInsf (incc_rec.hhwh_hash,0L, "","F","r");

		value = 0.00;
		ser_printed = FALSE;

		while (!cc && insfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			if (!ser_printed && strcmp (new_group,old_group))
			{
				strcpy (old_group,new_group);
				PrintCategory (!firstTime,groupValue);
				firstTime = FALSE;
				groupValue = 0.00;
			}
			workValue	=	SerialValue (insfRec.est_cost, insfRec.act_cost);

			ser_printed = TRUE;
			fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
			fprintf (ftmp, "| %-40.40s ",inmr_rec.description);
			fprintf (ftmp, "| %-25.25s ",insfRec.serial_no);

			fprintf (ftmp, "| %13.2f |", DPP (workValue));
			 
			value 		+= DPP (workValue);
			groupValue 	+= DPP (workValue);
			totalValue 	+= DPP (workValue);
			if (!strcmp (inmr_rec.supercession,"                "))
				fprintf (ftmp, "\n");
			else
				fprintf (ftmp, "*\n");

			cc = FindInsf (0L, 0L, "","F","r");
		}
		/*==========================================
		| Process Serial Items That are Committed. | 
		==========================================*/
		cc = FindInsf (incc_rec.hhwh_hash,0L, "","C","r");
		while (!cc && insfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			if (!ser_printed && strcmp (new_group,old_group))
			{
				strcpy (old_group,new_group);
				PrintCategory (!firstTime,groupValue);
				firstTime = FALSE;
				groupValue = 0.00;
			}
			
			workValue	=	SerialValue (insfRec.est_cost, insfRec.act_cost);

			ser_printed = TRUE;
			fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
			fprintf (ftmp, "| %-40.40s ",inmr_rec.description);
			fprintf (ftmp, "| %-25.25s ",insfRec.serial_no);

			fprintf (ftmp, "| %13.2f |", DPP (workValue));
			 
			value 		+= DPP (workValue);
			groupValue 	+= DPP (workValue);
			totalValue 	+= DPP (workValue);
			if (!strcmp (inmr_rec.supercession,"                "))
				fprintf (ftmp, "\n");
			else
				fprintf (ftmp, "*\n");

			cc = FindInsf (0L, 0L, "","C","r");
		}

		if (ser_printed)
		{
			fprintf (ftmp, "| %-16.16s "," ");
			fprintf (ftmp, "| %-40.40s "," ");
			fprintf (ftmp, "| %25.25s ","** ITEM TOTAL **");
			fprintf (ftmp, "| %13.2f |\n",value);
		}

		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
	fprintf (ftmp, "| %-16.16s "," ");
	fprintf (ftmp, "| %-40.40s "," ");
	fprintf (ftmp, "| %25.25s ","** GROUP TOTAL **");
	fprintf (ftmp, "| %13.2f |\n",groupValue);

	fprintf (ftmp, "| %-16.16s "," ");
	fprintf (ftmp, "| %-40.40s "," ");
	fprintf (ftmp, "| %25.25s ","** GRAND TOTAL **");
	fprintf (ftmp, "| %13.2f |\n",totalValue);
}

void
PrintCategory (
 int printValue, 
 double groupValue)
{
	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	      strcpy (excf_rec.cat_desc, "No Category description found.");

	expand (err_str,excf_rec.cat_desc);

	fprintf (ftmp, ".PD| %-103.103s |\n",err_str);

	if (printValue)
	{
		fprintf (ftmp, "| %-16.16s "," ");
		fprintf (ftmp, "| %-40.40s "," ");
		fprintf (ftmp, "| %25.25s ","** GROUP TOTAL **");
		fprintf (ftmp, "| %13.2f |\n",groupValue);
		fprintf (ftmp, ".PA\n");
	}
	else
		fprintf (ftmp, "| %-103.103s |\n",err_str);
}
