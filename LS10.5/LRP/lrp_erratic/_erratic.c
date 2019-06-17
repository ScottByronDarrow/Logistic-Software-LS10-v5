/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: _erratic.c,v 5.2 2001/08/09 09:29:44 scott Exp $
|  Program Name  : (lrp_erratic.c)                                    |
|  Program Desc  : (Erratic demand report.        )                   |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: _erratic.c,v $
| Revision 5.2  2001/08/09 09:29:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:28  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:19  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/01/30 05:48:49  scott
| Updated to add app.schema
|
| Revision 3.1  2001/01/23 22:30:32  scott
| Updated to add app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _erratic.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_erratic/_erratic.c,v 5.2 2001/08/09 09:29:44 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_lrp_mess.h>

#define		BY_PERCENT		 (local_rec.type [0]	==	'P')

#define	MAX_CCMR	100

FILE*   fout;

	struct
	{
		long	hhcc_hash;
		char	br_no [3];
		char	wh_no [3];
		char	wh_name [10];
	} whouse [MAX_CCMR];

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct ccmrRecord	ccmr_rec;

	int		firstItem;
	int		bestMethod;

	char	type [2];
	
	extern	int	TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startItem [17];
	char	endItem [17];
	char	item_desc [2] [41];
	char	type [2];
	float	percent;
	int		printerNumber;
	char	back [4];
	char	onight [4];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "from_item",	 4, 3, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "ALL", "Start Item             ", "Default is ALL ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "desc1",	 5, 3, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description            ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc [0]},
	{1, LIN, "to_item",	 7, 3, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "ALL", "End   Item             ", "Default is ALL ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "desc2",	 8, 3, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",   "Description            ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc [1]},
	{1, LIN, "flag",	 10, 3, CHARTYPE,
		"U", "          ",
		" ", "E", "E(rratic / P(ercentage ", "Produce report by Erratic demand or percentage difference ",
		 YES, NO,  JUSTLEFT, "EP", "", local_rec.type},
	{1, LIN, "percent",	 11, 3, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "20.00", "Percentage Difference. ", " ",
		 YES, NO,  JUSTLEFT, "", "",(char *)&local_rec.percent},
	{1, LIN, "printerNumber",	 13, 3, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No             ", " ",
		 YES, NO,  JUSTRIGHT, "", "",(char *)&local_rec.printerNumber},
	{1, LIN, "back",	 14, 3, CHARTYPE,
		"U", "          ",
		" ", "N", "Background             ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.back},
	{1, LIN, "back",	 15, 3, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight              ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.onight},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessItem 		(void);
void 	LoadHhcc 			(void);
int 	HeadingOutput 		(void);
int 	runProgram 			(char *, char *);
int 	spec_valid 			(int);
int 	heading 			(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	if (argc != 2 && argc != 6)
	{
		print_at (0,0, "Usage %s <DESCRIPTION>" ,argv [0]);
		print_at (1,0, "OR %s <LPNO> <START ITEM> <END ITEM> <P/E> <%%>",argv [0]);
        return (EXIT_FAILURE);
	}

	OpenDB ();

	if (argc == 6)
	{
		local_rec.printerNumber	=	atoi (argv [1]);
		sprintf (local_rec.startItem, "%-16.16s", 	argv [2]);
		sprintf (local_rec.endItem, "%-16.16s", 	argv [3]);
		sprintf (local_rec.type, 	 "%-1.1s", 		argv [4]);
		local_rec.percent	=	atof (argv [5]);

		HeadingOutput ();

		dsp_screen ("Processing Items ", comm_rec.co_no, comm_rec.co_name);

		firstItem = TRUE;
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",local_rec.startItem);
		cc = find_rec ("inmr",&inmr_rec,GTEQ,"r");
		while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no))
		{
			if (strcmp (inmr_rec.item_no, local_rec.endItem) > 0)
				break;

			dsp_process ("Item ",inmr_rec.item_no);

			ProcessItem ();

			cc = find_rec ("inmr",&inmr_rec,NEXT,"r");
		}

		fprintf (fout, ".EOF\n");
		pclose (fout);

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();	
	set_tty (); 
	set_masks ();
	init_vars (1);

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		/*-----------------------------
		| Entry screen 1 linear input |
		-----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

        prog_exit = runProgram (argv [0], argv [1]);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
runProgram (
 char*  programName,
 char*  programDescription)
{
	char	printerString [4];
	char	percent [10];

	sprintf (printerString,"%d",local_rec.printerNumber);
	sprintf (percent,"%6.2f",local_rec.percent);
	
	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
			        "ONIGHT",
			        programName,
			        printerString,
			        local_rec.startItem,
			        local_rec.endItem, 
			        local_rec.type, 
			        percent,
			        programDescription,
                    (char *)0);
            
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (programName,
				    programName,
				    printerString,
				    local_rec.startItem,
				    local_rec.endItem, 
				    local_rec.type, 
				    percent,
				    (char *)0);
	}
	else 
	{
		execlp (programName,
			    programName,
			    printerString,
			    local_rec.startItem,
			    local_rec.endItem, 
			    local_rec.type, 
			    percent,
			    (char *) 0);
        return (EXIT_FAILURE);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int    field)
{
	/*-------------------
	| Validate Category |
	-------------------*/
	if (LCHECK ("from_item"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.startItem, "%-16.16s", " ");
			sprintf (local_rec.item_desc [0],"%-40.40s","First Item");
			DSP_FLD ("from_item");
			DSP_FLD ("desc1");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.startItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.startItem);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startItem, inmr_rec.item_no);
		strcpy (local_rec.item_desc [0],inmr_rec.description);

		SuperSynonymError ();

		DSP_FLD ("from_item");
		DSP_FLD ("desc1");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("to_item"))
	{
		skip_entry = 1;
		if (dflt_used)
		{
			strcpy (local_rec.endItem, "~~~~~~~~~~~~~~~~");
			sprintf (local_rec.item_desc [1],"%-40.40s",ML ("Last Item"));
			DSP_FLD ("to_item");
			DSP_FLD ("desc2");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.endItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItem);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.item_desc [1],inmr_rec.description);

		SuperSynonymError ();

		DSP_FLD ("to_item");
		DSP_FLD ("desc2");

		return (EXIT_SUCCESS);
	}
	/*---------------
	| Validate type |
	---------------*/
	if (LCHECK ("flag"))
	{
		FLD ("percent")	=	 (local_rec.type [0] == 'P') ? YES : NA;

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
heading (
 int    scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlLrpMess027),25,0,1);

		move (0,1);
		line (80);

		move (1,input_row);

		box (0,3,80,12);
		move (1,6);
		line (79);
		move (1,9);
		line (79);
		move (1,12);
		line (79);

		move (0,20);
		line (80);

		print_at (21,0,  ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (21,45, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);

		move (0,22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	LoadHhcc ();
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose  (inmr);
	abc_fclose  (incc);
	abc_fclose  (ccmr);
	SearchFindClose ();
	abc_dbclose ("data");
}

void
ProcessItem (void)
{
	int		i;
	float	percentDiff	=	0.00;

	for (i = 0; i < MAX_CCMR && whouse [i].hhcc_hash != 0L; i++)
	{
		incc_rec.hhcc_hash = whouse [i].hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;

		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			continue;
	
		/*------------------------------
		| Erratic items are those with |
		| method D                     |
		------------------------------*/
		if (!BY_PERCENT && incc_rec.ff_method [0] != 'D')
			continue;
		else
		{
			if (incc_rec.pwks_demand == 0.00 &&
				incc_rec.wks_demand == 0.00)
					continue;

			if (incc_rec.pwks_demand <= 0.00)
				percentDiff = 999;
			else
			{
				percentDiff = incc_rec.wks_demand - incc_rec.pwks_demand;
				percentDiff /= incc_rec.pwks_demand;
				percentDiff *= 100;
				percentDiff	=	fabs (percentDiff);
				if (percentDiff < local_rec.percent)
					continue;
			}
		}
			
		fprintf (fout, "| %-16.16s ",  	inmr_rec.item_no);
		fprintf (fout, "| %-40.40s ",   inmr_rec.description);
		fprintf (fout, "|%4.4s ", 		inmr_rec.sale_unit);
		fprintf (fout, "| %2.2s ", 	whouse [i].br_no);
		fprintf (fout, "| %2.2s ", 	whouse [i].wh_no);
		fprintf (fout, "| %9.9s ", 	whouse [i].wh_name);

		if (!BY_PERCENT)
		{
			fprintf (fout, "|%15.2f ", 	incc_rec.closing_stock);
			fprintf (fout, "|%15.2f |\n", 	incc_rec.wks_demand);
		}
		else
		{

			fprintf (fout, "|%15.2f ",		incc_rec.pwks_demand);
			fprintf (fout, "|%15.2f ",		incc_rec.wks_demand);
			fprintf (fout, "|  %6.2f%%  |\n", fabs (percentDiff));
		}
	}

	fflush (fout);
}

int
HeadingOutput (void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat during (POPEN)",errno,PNAME);

	fprintf (fout,".START%s<%s>\n",DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNumber);
	fprintf (fout,".PI12\n");
	fprintf (fout,".8\n");
	fprintf (fout,".L152\n");
	if (!BY_PERCENT)
		fprintf (fout,".E ERRATIC DEMAND (Items using Focus Forecasting - Method 'D')\n");
	else
		fprintf (fout,".E ERRATIC DEMAND (Previous Demand different by %6.2f%%)\n",
						local_rec.percent);
	fprintf (fout,".B1\n");
	fprintf (fout,".ECOMPANY : %s - %s\n",comm_rec.co_no,comm_rec.co_name);

	fprintf (fout, ".R===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "======");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	if (!BY_PERCENT)
	{
		fprintf (fout, "=================");
		fprintf (fout, "==================\n");
	}
	else
	{
		fprintf (fout, "=================");
		fprintf (fout, "=================");
		fprintf (fout, "=============\n");
	}

	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "======");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	if (!BY_PERCENT)
	{
		fprintf (fout, "=================");
		fprintf (fout, "==================\n");
	}
	else
	{
		fprintf (fout, "=================");
		fprintf (fout, "=================");
		fprintf (fout, "=============\n");
	}

	fprintf (fout, "|   ITEM  NUMBER   ");
	fprintf (fout, "|        D E S C R I P T I O N             ");
	fprintf (fout, "| UOM ");
	fprintf (fout, "| BR ");
	fprintf (fout, "| WH ");
	fprintf (fout, "| WAREHOUSE ");
	if (!BY_PERCENT)
	{
		fprintf (fout, "| STOCK ON HAND  ");
		fprintf (fout, "|  WEEKS DEMAND  |\n");
	}
	else
	{
		fprintf (fout, "|PREV WKS DEMEND ");
		fprintf (fout, "| NEW WKS DEMAND ");
		fprintf (fout, "|PERCENT ERR|\n");
	}

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|-----");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|-----------");
	if (!BY_PERCENT)
	{
		fprintf (fout, "|----------------");
		fprintf (fout, "|----------------|\n");
	}
	else
	{
		fprintf (fout, "|----------------");
		fprintf (fout, "|----------------");
		fprintf (fout, "|-----------|\n");
	}

	return (EXIT_SUCCESS);
}

/*===============================================
| Load hhcc_hash table with valid warehouses	|
===============================================*/
void
LoadHhcc (void)
{
	int		indx;

	for (indx = 0; indx < MAX_CCMR; indx++)
	{
		whouse [indx].hhcc_hash = 0L;
		sprintf (whouse [indx].br_no, "%-2.2s", " ");
		sprintf (whouse [indx].wh_no, "%-2.2s", " ");
		sprintf (whouse [indx].wh_name, "%-10.10s", " ");
	}

	indx = 0;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no,  "  ");
	cc = find_rec ("ccmr",&ccmr_rec, GTEQ, "r");
	while (!cc && indx < MAX_CCMR && 
		   !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec ("ccmr", &ccmr_rec, NEXT, "r");
			continue;
		}
		whouse [indx].hhcc_hash = ccmr_rec.hhcc_hash;
		sprintf (whouse [indx].br_no, "%-2.2s", ccmr_rec.est_no);
		sprintf (whouse [indx].wh_no, "%-2.2s", ccmr_rec.cc_no);
		sprintf (whouse [indx++].wh_name, "%-10.10s", ccmr_rec.acronym);
		cc = find_rec ("ccmr", &ccmr_rec, NEXT, "r");
	}
}
