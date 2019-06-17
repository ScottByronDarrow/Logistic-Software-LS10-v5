/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: sk_prtfifo.c,v 5.4 2002/06/04 10:23:34 cha Exp $
|  Program Name  : (sk_prtfifo.c )                                  |
|  Program Desc  : (Print FIFO records                         )    |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 24/03/87         |
|---------------------------------------------------------------------|
| $Log: sk_prtfifo.c,v $
| Revision 5.4  2002/06/04 10:23:34  cha
| revert to previous version
|
| Revision 5.2  2001/08/09 09:19:41  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:19:23  scott
| Update - LS10.5
|
| Revision 4.1  2001/04/23 10:41:15  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to remove usage of old include files.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_prtfifo.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_prtfifo/sk_prtfifo.c,v 5.4 2002/06/04 10:23:34 cha Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

#define	BY_MODE 	 (mode [0] != ' ')
#define	MODE_OK		 (incc_rec.stat_flag [0] == mode [0])
#define	MAX_BR		5
#define	CHECK_LEN	 (groupLength < 12)
#define	FIFO_COST	 (inmr_rec.costing_flag [0] == 'F')
#define	LIFO_COST	 (inmr_rec.costing_flag [0] == 'I')

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct excfRecord	excf_rec;
struct sttfRecord	sttf_rec;

	int		printerNumber = 1,
			groupLength;

	FILE	*ftmp;

	char	lower [13], 
			upper [13],
			mode [2];

#include	<Costing.h>

/*=======================
| Function Declarations |
=======================*/
void 	HeadingOutput 		(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessData 		(void);
void 	PrintCategory 		(int, double);
float 	GetClosingStock 	(float, long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	/*---------------------------------------
	|	parameters							|
	|	1:	program name					|
	|	2:	printer number					|
	|	3:	lower bound   					|
	|	4:	upper bound   					|
	|	5:	Stock Take Mode   				|
	|	  	' ' - no modes expected			|
	---------------------------------------*/
	if (argc != 5)
	{
		print_at (0,0,mlSkMess018, argv [ 0 ]);
		return (EXIT_FAILURE);
	}
	
	printerNumber = atoi (argv [1]);
	sprintf (lower, "%-12.12s", argv [2]);
	sprintf (upper, "%-12.12s", argv [3]);
	sprintf (mode,  "%-1.1s",   argv [4]);

	if (strlen (clip (lower)) > strlen (clip (upper)))
		groupLength = strlen (clip (lower));
	else
		groupLength = strlen (clip (upper));

	OpenDB ();

	init_scr ();

	dsp_screen ("Processing : Detail Stock Valuation (FIFO).",
				comm_rec.co_no,comm_rec.co_name);

	IN_STAKE = (BY_MODE) ? TRUE : FALSE;
	
	/*---------------------------------------------
	| Find stock take record for date validation. |
	---------------------------------------------*/
	FindInsc (ccmr_rec.hhcc_hash, mode, BY_MODE);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen ("pformat","w")) == 0)
		file_err (errno, "pformat","POPEN");

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
	int	i;

	i = (BY_MODE) ? 1 : 0;

	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp,".PI12\n");

	fprintf (ftmp,".LP%d\n",printerNumber);
	fprintf (ftmp,".%d\n",12+i);
	fprintf (ftmp,".L130\n");

	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".EDETAIL STOCK VALUATION (FIFO)\n");
	if (BY_MODE)
		fprintf (ftmp,".ESTOCK TAKE SELECTION  [%s]\n",mode);

	fprintf (ftmp,".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp,".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (ftmp,".EWarehouse: %s \n",clip (comm_rec.cc_name));
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp,".R===================");
	fprintf (ftmp,"===========================================");
	fprintf (ftmp,"===========");
	fprintf (ftmp,"==============");
	fprintf (ftmp,"==============");
	fprintf (ftmp,"=================\n");

	fprintf (ftmp,"===================");
	fprintf (ftmp,"===========================================");
	fprintf (ftmp,"===========");
	fprintf (ftmp,"==============");
	fprintf (ftmp,"==============");
	fprintf (ftmp,"=================\n");

	fprintf (ftmp,"|   ITEM NUMBER    ");
	fprintf (ftmp,"|   I T E M    D E S C R I P T I O N       ");
	fprintf (ftmp,"|   DATE   ");
	fprintf (ftmp,"|  QUANTITY   ");
	fprintf (ftmp,"|     COST    ");
	fprintf (ftmp,"|  EXTENDED     |\n");

	fprintf (ftmp,"|------------------");
	fprintf (ftmp,"|------------------------------------------");
	fprintf (ftmp,"|----------");
	fprintf (ftmp,"|-------------");
	fprintf (ftmp,"|-------------");
	fprintf (ftmp,"|---------------|\n");
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

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);

	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no");
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
	abc_fclose (sttf);
	CloseCosting ();
	abc_dbclose ("data");
}

void
ProcessData (
 void)
{
	char	old_group [13];
	char	new_group [13];
	int		first_time = TRUE,
			first_fifo;

	float	on_hand = 0.00,
			needed = 0.00;

	double	addCost = 0.00,
			extend = 0.00,
			gr_extend = 0.00,
			tot_extend = 0.00;

	float	tot_qty = 0.00;
	double	tot_val = 0.00;

	int	no_printed = 0;

	/*---------------------------------------
	| read incc records			|
	---------------------------------------*/
	fflush (ftmp);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-28.28s",lower);
	cc = find_rec (incc, &incc_rec,GTEQ,"r");
	sprintf (old_group,"%12.12s",lower);
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		no_printed = 0;

		if (BY_MODE && !MODE_OK)
		{
			cc = find_rec (incc, &incc_rec,NEXT,"r");
			continue;
		}
		/*---------------------------------------
		| read inmr record			|
		---------------------------------------*/
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",incc_rec.sort + 12);
		cc = find_rec (inmr, &inmr_rec, COMPARISON,"r");
		if (cc || (!FIFO_COST && !LIFO_COST))
		{
			cc = find_rec (incc, &incc_rec,NEXT,"r");
			continue;
		}

		if (cc)
			file_err (cc, inmr, "DBFIND");

		dsp_process (" Item: ",inmr_rec.item_no);

		sprintf (new_group,"%1.1s%-11.11s", inmr_rec.inmr_class,
						   inmr_rec.category);

		if (strncmp (new_group,upper,groupLength) > 0)
			break;

		on_hand = (BY_MODE) ? GetClosingStock (incc_rec.closing_stock, 
						  incc_rec.hhwh_hash) 
				    : incc_rec.closing_stock;

		needed = on_hand;

		first_fifo = 1;

		tot_qty = 0.00;
		tot_val = 0.00;

		cc = FindIncf (incc_rec.hhwh_hash,FALSE,"r");
		while (on_hand > 0.00 && !cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			if (!ValidStDate (incfRec.fifo_date))
			{
				cc = FindIncf (0L,FALSE,"r");
				continue;
			}

			if (incfRec.fifo_qty <= 0.00)
			{
				cc = FindIncf (0L,FALSE,"r");
				continue;
			}
				
			if (on_hand < incfRec.fifo_qty)
			{
				incfRec.fifo_qty = on_hand;
				on_hand = 0.00;
			}
			else
				on_hand -= incfRec.fifo_qty;

			if (first_fifo)
			{
				if (strncmp (new_group, old_group, groupLength))
				{
					strcpy (old_group,new_group);
					PrintCategory (!first_time,gr_extend);
					first_time = FALSE;
					gr_extend = 0.00;
				}
				fprintf (ftmp,"| %-16.16s ",inmr_rec.item_no);
				fprintf (ftmp,"| %-40.40s ",inmr_rec.description);
				first_fifo = 0;
			}
			else
			{
				fprintf (ftmp,"| %-16.16s "," ");
				fprintf (ftmp,"| %-40.40s "," ");
			}
			fprintf (ftmp,"|%10.10s",DateToString (incfRec.fifo_date));
			fprintf (ftmp,"|%12.2f ",incfRec.fifo_qty);
			fprintf (ftmp,"|%12.2f ",incfRec.fifo_cost);
			extend = out_cost (incfRec.fifo_cost,inmr_rec.outer_size);
			extend *= (double) incfRec.fifo_qty;
			extend = twodec (extend);
			gr_extend += extend;
			tot_extend += extend;
			fprintf (ftmp,"|%14.2f |\n",extend);

			tot_qty += incfRec.fifo_qty;
			tot_val += extend;

			cc = FindIncf (0L,FALSE,"r");
			no_printed++;
		}
		if (on_hand > 0.00)
		{
			strcpy (ineiRec.est_no, ccmr_rec.est_no);
			ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
			if (FindInei (inmr_rec.hhbr_hash, ccmr_rec.est_no, "r"))
			{
				ineiRec.avge_cost = 0.00;
				ineiRec.last_cost = 0.00;
			}

			addCost = 	CheckIncf 
						(
							incc_rec.hhwh_hash, 
							FALSE, 
							needed,
							inmr_rec.dec_pt
						);
					       
			fprintf (ftmp,"| %-16.16s ", (first_fifo) 
					       ? inmr_rec.item_no : " ");
			fprintf (ftmp,"| %-40.40s ", (first_fifo) 
					       ? inmr_rec.description : " ");
			
			fprintf (ftmp,"|%10.10s",DateToString (comm_rec.inv_date));
			fprintf (ftmp,"|%12.2f ",on_hand);
			fprintf (ftmp,"|%12.2f ",addCost / on_hand);
			extend = out_cost (addCost / on_hand, 
					   inmr_rec.outer_size);

			extend *= (double) on_hand;
			extend = twodec (extend);
			gr_extend += extend;
			tot_extend += extend;

			tot_qty += on_hand;
			tot_val += extend;
			fprintf (ftmp,"|%14.2f |**\n",extend);
			no_printed++;
		}
		if (no_printed > 1)
		{
			fprintf (ftmp,"| %-16.16s "," ");
			fprintf (ftmp,"| %-40.40s "," ");
			fprintf (ftmp,"|ITEM TOTAL");
			fprintf (ftmp,"|%12.2f ", tot_qty);
			fprintf (ftmp,"|             ");
			fprintf (ftmp,"|%14.2f |\n",tot_val);
		}
		cc = find_rec (incc, &incc_rec,NEXT,"r");
	}
	fprintf (ftmp,"| %-16.16s "," ");
	fprintf (ftmp,"| %-40.40s "," ");
	fprintf (ftmp,"| %-8.8s "," ");
	fprintf (ftmp,"| %-11.11s "," ");
	fprintf (ftmp,"| %-11.11s ","** GROUP **");
	fprintf (ftmp,"|%14.2f |\n",gr_extend);

	fprintf (ftmp,"| %-16.16s "," ");
	fprintf (ftmp,"| %-40.40s "," ");
	fprintf (ftmp,"| %-8.8s "," ");
	fprintf (ftmp,"| %-11.11s "," ");
	fprintf (ftmp,"| %-11.11s ","** TOTAL **");
	fprintf (ftmp,"|%14.2f |\n",tot_extend);
}

void
PrintCategory (
 int prt_extend, 
 double gr_extend)
{
	int	len = groupLength - 1;
	/*------------------------------------------------------------
	| If the selected group length < 12,then get the desc.of the|
	| higher (shorter) level category.                           |
	------------------------------------------------------------*/
	strcpy (excf_rec.co_no,comm_rec.co_no);
	if (CHECK_LEN)
		sprintf (excf_rec.cat_no,"%-*.*s%-*.*s",len,len,
				inmr_rec.category, (11 - len), (11 - len)," ");
	else
		strcpy (excf_rec.cat_no,inmr_rec.category);

	cc = find_rec (excf, &excf_rec, COMPARISON,"r");
	if (cc)
	      sprintf (excf_rec.cat_desc,"%40.40s",
					"No Category description found.");

	expand (err_str,excf_rec.cat_desc);

	fprintf (ftmp,".PD| %s%35.35s|\n",err_str," ");

	if (prt_extend)
	{
		fprintf (ftmp,"| %-16.16s "," ");
		fprintf (ftmp,"| %-40.40s "," ");
		fprintf (ftmp,"| %-8.8s "," ");
		fprintf (ftmp,"| %-11.11s "," ");
		fprintf (ftmp,"| %-11.11s ","** GROUP **");
		fprintf (ftmp,"|%14.2f |\n",gr_extend);
		fprintf (ftmp,".PA\n");
	}
}

/*===========================
| Get closing stock actual. |
===========================*/
float	
GetClosingStock (
 float on_hand, 
 long hhwh_hash)
{
	float	counted = 0.00;

	sttf_rec.hhwh_hash = hhwh_hash;
	sprintf (sttf_rec.location,"%-10.10s"," ");
	cc = find_rec (sttf, &sttf_rec,GTEQ,"r");
	while (!cc && sttf_rec.hhwh_hash == hhwh_hash)
	{
		counted += sttf_rec.qty;
		cc = find_rec (sttf, &sttf_rec,NEXT,"r");
	}

	return (on_hand - (on_hand - counted));
}
