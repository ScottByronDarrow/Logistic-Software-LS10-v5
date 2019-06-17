/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_ser_age.c,v 5.2 2001/08/09 09:19:54 scott Exp $
|  Program Name  : (sk_ser_age.c )                                 |
|  Program Desc  : (Print ageing of stock (SERIAL)             )   |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 07/04/87         |
|---------------------------------------------------------------------|
| $Log: sk_ser_age.c,v $
| Revision 5.2  2001/08/09 09:19:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:48  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_ser_age.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ser_age/sk_ser_age.c,v 5.2 2001/08/09 09:19:54 scott Exp $";

#include 	<pslscr.h>	
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
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

	long	_year_end [6];

/*=======================
| Function Declarations |
=======================*/
void 	HeadingOutput 		(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessData 		(void);
void 	PrintValues 		(int, double);
void 	SetPeriod 			(void);
int  	PeriodIn 			(long);
void 	PrintCategory 		(int, double *);

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
		print_at (0,0, mlSkMess111, argv [0]); 
		return (EXIT_FAILURE);
	}
	
	lpno = atoi (argv [1]);
	sprintf (lower,"%-12.12s",argv [2]);
	sprintf (upper,"%-12.12s",argv [3]);

	/*======================
	| Open database files. |
	======================*/
	OpenDB ();

	_year_end [0] = get_eoy (comm_rec.inv_date, comm_rec.fiscal);

	dsp_screen ("Processing : Stock Ageing (SERIAL)", comm_rec.co_no, comm_rec.co_name);

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
	fprintf (ftmp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (ftmp, ".LPNO%d\n",lpno);
	fprintf (ftmp, ".12\n");
	fprintf (ftmp, ".L158\n");
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ESTOCK AGEING REPORT (SERIAL)\n");
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".EWarehouse: %s \n",clip (comm_rec.cc_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R=================");
	fprintf (ftmp, "==============================");
	fprintf (ftmp, "==========================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "=================");
	fprintf (ftmp, "==============================");
	fprintf (ftmp, "==========================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "|   ITEM NUMBER  ");
	fprintf (ftmp, "|      ITEM  DESCRIPTION      ");
	fprintf (ftmp, "|   SERIAL   NUMBER       ");
	fprintf (ftmp, "| Current Year");
	fprintf (ftmp, "| 1 year old  ");
	fprintf (ftmp, "| 2 years old ");
	fprintf (ftmp, "| 3 years old ");
	fprintf (ftmp, "| 4 years old ");
	fprintf (ftmp, "| 5+ years old|\n");
	fprintf (ftmp, ".PI12\n");

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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)", cc, PNAME);

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
    abc_fclose (ccmr);
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_dbclose ("data");
}

void
ProcessData (
 void)
{
	int	i;
	int	per_no;
	char	old_group [13];
	char	new_group [13];
	int	first_time = TRUE;
	int	ser_found;
	double	cost_used;
	double	value [7];
	double	gr_value [7];
	double	tot_value [7];

	SetPeriod ();

	for (i = 0;i < 7;i++)
	{
		value [i] = 0.00;
		gr_value [i] = 0.00;
		tot_value [i] = 0.00;
	}

	/*-----------------------
	|	read first incc	|
	-----------------------*/
	fflush (ftmp);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-12.12s%-16.16s",lower, " ");
	cc = find_rec (incc, &incc_rec, GTEQ, "r");

	sprintf (old_group,"%12.12s",lower);

	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash && 
		      strncmp (incc_rec.sort,upper,12) <= 0)
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
			file_err (cc,inmr, "DBFIND");

		dsp_process (" Item: ", inmr_rec.item_no);

		sprintf (new_group,"%1.1s%-11.11s",
			            inmr_rec.inmr_class,inmr_rec.category);

		/*-------------------------------
		| no insf records found yet	|
		-------------------------------*/
		ser_found = FALSE;

		/*------------------------------------
		| Find Serial Records that are free. |
		------------------------------------*/
		cc = FindInsf (incc_rec.hhwh_hash,0L, "","F","r");
		while (!cc && insfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			cost_used = SerialValue (insfRec.est_cost, insfRec.act_cost);
			per_no = PeriodIn (insfRec.date_in);

			/*---------------------------------------
			| there is something to be printed	|
			---------------------------------------*/
			if (!ser_found && cost_used != 0.00 && per_no >= 0)
			{
				/*---------------
				| new category	|
				---------------*/
				if (strcmp (new_group,old_group))
				{
					strcpy (old_group,new_group);
					PrintCategory (!first_time,gr_value);
					first_time = FALSE;
					for (i = 0;i < 7;i++)
						gr_value [i] = 0.00;
				}

				ser_found = TRUE;
			}

			/*-----------------------
			| record to be printed	|
			-----------------------*/
			if (per_no >= 0 && cost_used != 0.00)
			{
				value [per_no] += cost_used;
				gr_value [per_no] += cost_used;
				tot_value [per_no] += cost_used;
				fprintf (ftmp, ".LRP4\n");
				fprintf (ftmp, "|----------------");
				fprintf (ftmp, "+-----------------------------");
				fprintf (ftmp, "+-------------------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------|\n");
				fprintf (ftmp, "|%-16.16s",inmr_rec.item_no);
				fprintf (ftmp, "|%-29.29s",inmr_rec.description);
				fprintf (ftmp, "|%-25.25s",insfRec.serial_no);
				fflush (ftmp);
				PrintValues (per_no,cost_used);
			}
			cc = FindInsf (0L, 0L, "","F","r");
		}

		/*-----------------------------------------
		| Find Serial Records that are Committed. |
		-----------------------------------------*/
		cc = FindInsf (incc_rec.hhwh_hash, 0L, "","C","r");
		while (!cc && insfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			cost_used = SerialValue (insfRec.est_cost, insfRec.act_cost);

			per_no = PeriodIn (insfRec.date_in);

			/*---------------------------------------
			| there is something to be printed	|
			---------------------------------------*/
			if (!ser_found && cost_used != 0.00 && per_no >= 0)
			{
				/*---------------
				| new category	|
				---------------*/
				if (strcmp (new_group,old_group))
				{
					strcpy (old_group,new_group);
					PrintCategory (!first_time,gr_value);
					first_time = FALSE;
					for (i = 0;i < 7;i++)
						gr_value [i] = 0.00;
				}

				ser_found = TRUE;
			}

			/*-----------------------
			| record to be printed	|
			-----------------------*/
			if (per_no >= 0 && cost_used != 0.00)
			{
				value [per_no] += cost_used;
				gr_value [per_no] += cost_used;
				tot_value [per_no] += cost_used;
				fprintf (ftmp, ".LRP4\n");
				fprintf (ftmp, "|----------------");
				fprintf (ftmp, "+-----------------------------");
				fprintf (ftmp, "+-------------------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------");
				fprintf (ftmp, "+-------------|\n");
				fprintf (ftmp, "|%-16.16s",inmr_rec.item_no);
				fprintf (ftmp, "|%-29.29s",inmr_rec.description);
				fprintf (ftmp, "|%-25.25s",insfRec.serial_no);
				fflush (ftmp);
				PrintValues (per_no,cost_used);
			}
			cc = FindInsf (0L,0L,"","C","r");
		}

		/*-------------------------------
		| serial records found for item	|
		-------------------------------*/
		if (ser_found)
		{
			fprintf (ftmp, ".LRP3\n");
			fprintf (ftmp, "|%-16.16s"," ");
			fprintf (ftmp, "|%-29.29s"," ");
			fprintf (ftmp, "|%25.25s","*** ITEM TOTAL ***");

			for (i = 0;i < 7;i++)
			{
				if (value [i] == 0.00)
					fprintf (ftmp, "| %11.11s "," ");
				else
					fprintf (ftmp, "| %11.2f ",value [i]);
				fflush (ftmp);
				value [i] = 0.00;
			}
			fprintf (ftmp, "|\n");
		}

		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
	fprintf (ftmp, "|================");
	fprintf (ftmp, "|=============================");
	fprintf (ftmp, "|=========================");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============|\n");

	fprintf (ftmp, "|%-16.16s"," ");
	fprintf (ftmp, "|%29.29s","*** GROUP TOTAL ***");
	fprintf (ftmp, "| %23.2f ",gr_value [0]+gr_value [1]+gr_value [2]+gr_value [3]+gr_value [4]);

	for (i = 0;i < 7;i++)
	{
		if (gr_value [i] == 0.00)
			fprintf (ftmp, "| %11.11s "," ");
		else
			fprintf (ftmp, "| %11.2f ",gr_value [i]);
	}
	fprintf (ftmp, "|\n");

	fprintf (ftmp, "|================");
	fprintf (ftmp, "|=============================");
	fprintf (ftmp, "|=========================");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============|\n");
	fprintf (ftmp, "|%-16.16s"," ");
	fprintf (ftmp, "|%29.29s","*** SERIAL TOTAL ***");
	fprintf (ftmp, "| %23.2f ",tot_value [0]+tot_value [1]+tot_value [2]+tot_value [3]+tot_value [4]);

	for (i = 0;i < 7;i++)
	{
		if (tot_value [i] == 0.00)
			fprintf (ftmp, "| %11.11s "," ");
		else
			fprintf (ftmp, "| %11.2f ",tot_value [i]);
	}
	fprintf (ftmp, "|\n");
}

/*=======================================================
|	print serial value in appropriate period [0-5]	|
=======================================================*/
void
PrintValues (
 int period, 
 double value)
{
	int	i;

	for (i = 0;i < period;i++)
		fprintf (ftmp, "| %11.11s "," ");

	fflush (ftmp);
	fprintf (ftmp, "| %11.2f ",value);

	for (i = period;i < 7;i++)
		fprintf (ftmp, "| %11.11s "," ");

	fprintf (ftmp, "|\n");
	fflush (ftmp);
}

/*=======================================
|	initialise the period cutoffs	|
=======================================*/
void
SetPeriod (void)
{
	int	i;

	for (i = 1; i < 6; i++)
		_year_end [i] = AddYears (_year_end [i - 1], -1);
}

/*===============================================
|	return the period that the date is in	|
|	-1 : more recent than date input	|
|	0-4 : period that the date is in	|
===============================================*/
int
PeriodIn (
 long date_in)
{
	int	i;

	if (date_in > _year_end [0])
		return (-1);

	for (i = 0;i < 6;i++)
		if (_year_end [i] >= date_in && date_in > _year_end [i + 1])
			return (i);

	return (5);
}

void
PrintCategory (
 int prt_value, 
 double *gr_value)
{
	int	i;

	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	      strcpy (excf_rec.cat_desc, "No Category description found.");

	expand (err_str,excf_rec.cat_desc);

	fprintf (ftmp, ".PD| %-154.154s |\n",err_str);

	if (prt_value)
	{
		fprintf (ftmp, "|%-16.16s"," ");
		fprintf (ftmp, "|%29.29s","*** GROUP TOTAL ***");
		fprintf (ftmp, "| %23.2f ",gr_value [0]+gr_value [1]+gr_value [2]+gr_value [3]+gr_value [4]);
		for (i = 0;i < 7;i++)
		{
			if (gr_value [i] == 0.00)
				fprintf (ftmp, "| %11.11s "," ");
			else
				fprintf (ftmp, "| %11.2f ",gr_value [i]);
		}

		fprintf (ftmp, "|\n.PA\n");
	}
}
