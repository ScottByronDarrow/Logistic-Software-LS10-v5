/*=====================================================================	
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: dd_print.i.c,v 5.5 2002/09/06 04:00:50 scott Exp $
|  Program Name  : (dd_print.i.c)
|  Program Desc  : (Direct Delivery Quotation and Order)
|                 (Confirmation Reprint)
|---------------------------------------------------------------------|
|  Author        :                 | Date Written  :   /  /           |
|---------------------------------------------------------------------|
| $Log: dd_print.i.c,v $
| Revision 5.5  2002/09/06 04:00:50  scott
| S/C SC4297 / LS01239 -- DDMR10 (Print Direct Delivery Supplier Confirmation)
|
| Revision 5.4  2002/07/18 06:31:55  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2001/11/06 03:04:30  scott
| Updated from testing.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dd_print.i.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DD/dd_print.i/dd_print.i.c,v 5.5 2002/09/06 04:00:50 scott Exp $";

#define	MAXWIDTH	140
#define	MAXLINES	100

#define		QUOTE		0
#define		ORD_CONF	1
#define		SUPP_CONF	2

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_dd_mess.h>
#include <ml_std_mess.h>

int		envDbCo 	= 0,
		envDbFind 	= 0,
		printerNo	= 0,
		autoPrint	= 0,
		typeFlag	= 0;

long	hashToPrint = 0L;

char	branchNo [3],
		runPrint [81];

char	*data  = "data";

FILE*   pout;

#define	TYPE	program [typeFlag]

#ifdef GVISION
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#endif	/* GVISION */

struct	{
	char	*_type;
	char	*_prmpt;
	char	*_desc;
	char	*_prg;
} program [] = {
	{"Q","Quotation No",	"Quotation",			"dd_qt_prt"},
	{"O","  Order No  ",	"Order Confirmation",	"dd_ord_conf"},
	{"S","P.O. Number ",	"Purchase Order",		"dd_sup_conf"},
	{"","","",""},
};

#include	"schema"

struct commRecord	comm_rec;
struct ddhrRecord	ddhr_rec;
struct ddlnRecord	ddln_rec;
struct cumrRecord	cumr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	ord_no [2][9];
	char	customer [2][21];
} local_rec;

static	struct	var	vars [] =
{
	{1, TAB, "start_ord_no",	MAXLINES, 1, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "0", "123456789012", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.ord_no [0]},
	{1, TAB, "start_customer",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  C u s t o m e r   ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.customer [0]},
	{1, TAB, "end_ord_no",	 0, 1, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "0", "123456789012", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.ord_no [1]},
	{1, TAB, "end_customer",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  C u s t o m e r   ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.customer [1]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

void    shutdown_prog 	(void);
void    OpenDB 			(void);
void    CloseDB 		(void);
void    Process 		(void);
int     spec_valid 		(int);
void    PrintAll 		(void);
int     DeleteLine 		(void);
void    SrchDdhr 		(char *, char *, char *);
int     heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	if (argc < 3)
	{
		/*-----------------------------------------------------
		| Usage : %s <printerNo> <type [Q/O/S]> [hhdd_hash]\007\n\r |
		-----------------------------------------------------*/
		print_at (0,0, ML (mlDdMess037), argv [0]);
		return (argc);
	}

	autoPrint = FALSE;
	if (argc == 4)
	{
		autoPrint = TRUE;
		hashToPrint = atol (argv [3]);
	}

	if (!autoPrint)
		SETUP_SCR (vars);

	printerNo = atoi (argv [1]);

	switch (argv [2][0])
	{
	case 'Q' :
	case 'q' :
			typeFlag = QUOTE;
			break;

	case 'O' :
	case 'o' :
			typeFlag = ORD_CONF;
			break;

	case 'S' :
	case 's' :
			typeFlag = SUPP_CONF;
			if (argc < 4)	
			{
				/*
				 * Use purchase order report program to reprint purchase orders
				 */
				print_at (0,0, ML (mlDdMess092));
			}
			break;
	
	default :
			/*
			 * <type> must be Q(uotataion, O(rder Confirmation, 
			 * or S(upplier Confirmation
			 */
			print_at (0,0, ML (mlDdMess093));
			return (EXIT_FAILURE);
	}

    OpenDB ();


	vars [0].prmpt = strdup (TYPE._prmpt);
	vars [2].prmpt = strdup (TYPE._prmpt);

	strcpy (runPrint,TYPE._prg);

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	if (!autoPrint)
	{
		init_scr ();
		set_tty (); 
		set_masks ();
	}

	if (autoPrint)
	{
		if (typeFlag == QUOTE)
			strcpy (err_str, "dd_qt_prt");

		else if (typeFlag == ORD_CONF)
			strcpy (err_str, "dd_ord_conf");

		else if (typeFlag == SUPP_CONF)
			strcpy (err_str, "dd_sup_conf");
		else
		{
			shutdown_prog ();
			return (EXIT_SUCCESS);
		}
		if ((pout = popen (err_str, "w")) == 0)
		{
			shutdown_prog ();
			return (EXIT_SUCCESS);
		}
		fprintf (pout, "%d\n", printerNo);
		fprintf (pout, "S\n");
		fprintf (pout, "%ld\n",hashToPrint);
#ifdef GVISION
		Remote_fflush (pout);
#else
		fflush (pout);
#endif
		pclose(pout);
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		lcount [1] 	= 0;

		heading (1);
		entry (1);

		if (prog_exit || restart)
			break;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			break;

		prog_exit = TRUE;

		if (lcount [1] != 0)
			Process ();
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

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ddhr, ddhr_list, DDHR_NO_FIELDS, "ddhr_id_no2");
	open_rec (ddln, ddln_list, DDLN_NO_FIELDS, "ddln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (ddhr);
	abc_fclose (ddln);
	abc_fclose (cumr);
	abc_dbclose (data);
}

void
Process (void)
{
	int	firstTime = TRUE;

	clear ();
	print_at (0,0, ML (mlStdMess035));
	fflush (stdout);

	if ((pout = popen (runPrint,"w")) == 0)
	{
		sprintf (err_str,"Error in %s during (POPEN)",runPrint);
		sys_err (err_str,errno,PNAME);
	}

	for (line_cnt = 0;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt);

		strcpy (ddhr_rec.co_no, comm_rec.co_no);
		strcpy (ddhr_rec.br_no, comm_rec.est_no);
		sprintf (ddhr_rec.order_no, "%-8.8s", local_rec.ord_no [0]);
		cc = find_rec (ddhr, &ddhr_rec, GTEQ, "r");
		while (!cc && 
			!strcmp (ddhr_rec.co_no, comm_rec.co_no) && 
			!strcmp (ddhr_rec.br_no, comm_rec.est_no) && 
			strcmp (ddhr_rec.order_no, local_rec.ord_no [1]) <= 0)
		{
			if (firstTime == TRUE)
			{
				sprintf (err_str, "Printing %ss", TYPE._desc);
				dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

				fprintf (pout, "%d\n", printerNo);
				fprintf (pout, "M\n");
				fprintf (pout, "%ld\n",0L);
			}
			fprintf (pout, "%ld\n",ddhr_rec.hhdd_hash);
#ifdef GVISION
			Remote_fflush (pout);
#else
			fflush (pout);
#endif
			firstTime = FALSE;
			dsp_process (TYPE._desc, ddhr_rec.order_no);
		
			cc = find_rec (ddhr, &ddhr_rec, NEXT, "r");
		}
	}

	if (firstTime == FALSE)
		fprintf (pout, "%ld\n",0L);

	pclose (pout);
}

int
spec_valid (
 int    field)
{
	char	lowOrderNo [9];
	char	highOrderNo [9];

	if (LCHECK ("start_ord_no")) 
	{
		if (dflt_used)
			return (DeleteLine ());

		if (!strncmp (local_rec.ord_no [0], "ALL     ", 8))
		{
			PrintAll ();
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.ord_no [0], zero_pad (local_rec.ord_no [0], 8));

		strcpy (lowOrderNo, "        ");
		strcpy (highOrderNo, (prog_status == ENTRY) ? "~~~~~~~~" : 
													local_rec.ord_no [1]);
/*
		if (typeFlag != SUPP_CONF)
		{
*/
			if (SRCH_KEY)
			{
				SrchDdhr (lowOrderNo, temp_str, highOrderNo);
				return (EXIT_SUCCESS);
			}

			strcpy (ddhr_rec.co_no, comm_rec.co_no);
			strcpy (ddhr_rec.br_no, comm_rec.est_no);
			sprintf (ddhr_rec.order_no, "%-8.8s", local_rec.ord_no [0]);
			cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
			if (cc)
			{
				if (typeFlag == QUOTE)
					sprintf (err_str, ML (mlStdMess152));
				if (typeFlag == ORD_CONF)
					sprintf (err_str, ML (mlStdMess122));
				if (typeFlag == SUPP_CONF)  
					sprintf (err_str, ML (mlStdMess048));
	
				print_err (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	
			if (strncmp (ddhr_rec.order_no, highOrderNo, 8) > 0)
			{
				print_err (ML (mlStdMess017));
				return (EXIT_FAILURE);
			}
	
			cumr_rec.hhcu_hash	=	ddhr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cc)
			{
				print_err (ML (mlStdMess021));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			/*
		}
		*/

		sprintf (local_rec.customer [0], "%-20.20s", cumr_rec.dbt_name);
		DSP_FLD ("start_customer");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_ord_no")) 
	{
		if (dflt_used)
			strcpy (local_rec.ord_no [1], local_rec.ord_no [0]);

		if (!strncmp (local_rec.ord_no [1], "ALL     ", 8))
		{
			PrintAll ();
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.ord_no [1], zero_pad (local_rec.ord_no [1], 8));

		strcpy (lowOrderNo, local_rec.ord_no [0]);
		strcpy (highOrderNo, "~~~~~~~~");
		/*
		if (typeFlag != SUPP_CONF)
		{
		*/
			if (SRCH_KEY)
			{
				SrchDdhr (lowOrderNo, temp_str, highOrderNo);
				return (EXIT_SUCCESS);
			}
	
			strcpy (ddhr_rec.co_no, comm_rec.co_no);
			strcpy (ddhr_rec.br_no, comm_rec.est_no);
			sprintf (ddhr_rec.order_no, "%-8.8s", local_rec.ord_no [1]);
			cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
			if (cc)
			{
				if (typeFlag == QUOTE)
					sprintf (err_str, ML (mlStdMess152));
				if (typeFlag == ORD_CONF)
					sprintf (err_str, ML (mlStdMess122));
				if (typeFlag == SUPP_CONF)  
					sprintf (err_str, ML (mlStdMess048));

				print_err (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			if (strncmp (ddhr_rec.order_no, lowOrderNo, 8) < 0)
			{
				print_err (ML (mlStdMess018));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	
			cumr_rec.hhcu_hash	=	ddhr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cc)
			{
				print_err (ML (mlStdMess021));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			/*
		}
		*/
		sprintf (local_rec.customer [1], "%-20.20s", cumr_rec.dbt_name);
		DSP_FLD ("end_customer");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
PrintAll (void)
{
	sprintf (local_rec.ord_no [0], "%-8.8s", "00000000");
	sprintf (local_rec.ord_no [1], "%-8.8s", "~~~~~~~~");
	sprintf (local_rec.customer [0], "%-20.20s", "ALL Customers       ");
	sprintf (local_rec.customer [1], "%-20.20s", "ALL Customers       ");
	DSP_FLD ("start_ord_no");
	DSP_FLD ("end_ord_no");
	DSP_FLD ("start_customer");
	DSP_FLD ("end_customer");

	entry_exit = 1;
	edit_exit = 1;

	if (prog_status == ENTRY)
		putval (line_cnt++);
}

int
DeleteLine (void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		/*------------------------------
		| Cannot Delete Lines on Entry |
		------------------------------*/
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [1]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	strcpy (local_rec.ord_no [0], "        ");
	strcpy (local_rec.ord_no [1], "        ");
	strcpy (local_rec.customer [0], "                    ");
	strcpy (local_rec.customer [1], "                    ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*==========================
| Search for order number. |
==========================*/
void
SrchDdhr (
	char	*lowOrderNo,
	char	*keyedValue,
	char	*highOrderNo)
{
	_work_open (8,0,40);
	sprintf (err_str, "#%s", TYPE._prmpt);
	save_rec (err_str, "#Customer");

	strcpy (ddhr_rec.co_no, comm_rec.co_no);
	strcpy (ddhr_rec.br_no, comm_rec.est_no);
	sprintf (ddhr_rec.order_no, "%-8.8s", keyedValue);

	cc = find_rec (ddhr,&ddhr_rec,GTEQ,"r");

	while (!cc && !strcmp (ddhr_rec.co_no, comm_rec.co_no) && 
		!strcmp (ddhr_rec.br_no, comm_rec.est_no))
	{
		ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		ddln_rec.line_no = 0;
		cc = find_rec (ddln, &ddln_rec, GTEQ, "r");

		if (!cc && ddln_rec.hhdd_hash == ddhr_rec.hhdd_hash && 
		     strncmp (ddhr_rec.order_no, lowOrderNo, 8) >= 0 && 
		     strncmp (ddhr_rec.order_no, highOrderNo, 8) <= 0)
		{
			cumr_rec.hhcu_hash	=	ddhr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (!cc)
			{
				cc = save_rec (ddhr_rec.order_no, cumr_rec.dbt_name);
				if (cc)
					break;
			}
		}
		cc = find_rec (ddhr, &ddhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ddhr_rec.co_no, comm_rec.co_no);
	strcpy (ddhr_rec.br_no, comm_rec.est_no);
	sprintf (ddhr_rec.order_no, "%-8.8s", temp_str);
	cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ddhr, "DBFIND");
}

int
heading (
 int    scn)
{
	char	startString [26];
	char	endString [26];

	if (restart) 
        return (EXIT_SUCCESS);
	
	clear ();

	switch (typeFlag)
	{
		case	QUOTE	:
			sprintf (err_str,  " %s ", ML (mlDdMess094));	
			sprintf (startString," %s ", ML (mlDdMess090));	
			sprintf (endString,  " %s ", ML (mlDdMess091));	
			break;

		case	ORD_CONF:
			sprintf (err_str,  " %s ", ML (mlDdMess095));	
			sprintf (startString," %s ", ML (mlDdMess097));	
			sprintf (endString,  " %s ", ML (mlDdMess098));	
			break;

		case	SUPP_CONF:
			sprintf (err_str,  " %s ", ML (mlDdMess096));	
			sprintf (startString," %s ", ML (mlDdMess099));	
			sprintf (endString,  " %s ", ML (mlDdMess100));	
			break;

		default	:	
			break;
	}

	rv_pr (err_str, (80 - strlen (err_str)) / 2,0,1);

	line_at (1,0,80);

	rv_pr (startString, (40 - strlen (startString)) / 2, tab_row - 2,1);
	rv_pr (endString,38 + (40 - strlen (endString)) / 2, tab_row - 2,1);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

