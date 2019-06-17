/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ar_invprnt.i.c,v 5.3 2002/08/14 04:37:13 scott Exp $
|  Program Name  : (ar_invprnt.i.c)                                   |
|  Program Desc  : (Archive Invoice / Credit Note)    
|                  (For Print & Reprint incl Reflagging)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 31/10/88         |
|---------------------------------------------------------------------|
| $Log: ar_invprnt.i.c,v $
| Revision 5.3  2002/08/14 04:37:13  scott
| Updated for Linux error
|
| Revision 5.2  2002/07/18 07:18:25  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.1  2001/12/11 02:29:03  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ar_invprnt.i.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/ar_invprnt.i/ar_invprnt.i.c,v 5.3 2002/08/14 04:37:13 scott Exp $";

#define	MAXWIDTH	140
#define	MAXPRINT	10000
#define	MAXLINES	100
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>

#define	REPRINT		 (printFlag [0] == 'Y')
#define	INVOICE		0
#define	CREDIT_NOTE	1
#define	PACK_SLIP	2
#define	STD_ORDER	3
#define	STD_INVOICE	4
#define	CCN			5

#define	TYPE		program [typeFlag]
#define	STANDARD	 (typeFlag == STD_ORDER || typeFlag == STD_INVOICE)

#ifdef GVISION
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#define	fflush	Remote_fflush
#endif	/* GVISION */

struct	{
	char	*_type;
	char	*_prt;
	char	*_prmpt;
	char	*_desc;
	char	*_env;
	char	*_prg;
	char	*_alt_type;
} program [] = {
	{"I","I","Tax Invoice No  ","Tax Invoice", 	 "AR_CTR_INV","ctr_inv","I"},
	{"C","C","Credit Note No  ","Credit Note", 	 "AR_CTR_INV","ctr_inv","C"},
	{"", "", "", "", "", "", ""}
};

#include	"schema"

	struct	commRecord	comm_rec;
	struct	arhrRecord	arhr_rec;
	struct	arlnRecord	arln_rec;
	struct	cumrRecord	cumr_rec;

	int		envDbCo			= 0;
	int		envDbFind		= 0;
	int		printerNumber	= 0;
	int		typeFlag		= 0;

char	branchNumber [3];
char	printFlag [2];
char	runPrintProgram [81];

char	*data  = "data";

int		findStatus	=	FALSE;

FILE	*pout;

char	titleStr [31];
char	startStr [21];
char	endStr [18];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	inv_no [2] [9];
	char	customer [2] [31];
	char	find_codes [13];
} local_rec;

static	struct	var	vars []	={	

	{1, TAB, "start_inv_no", MAXLINES, 7, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", "1234567890123456", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.inv_no [0]}, 
	{1, TAB, "start_customer", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "  C u s t o m e r   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.customer [0]}, 
	{1, TAB, "end_inv_no", 0, 5, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", "1234567890123456", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.inv_no [1]}, 
	{1, TAB, "end_customer", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "  C u s t o m e r   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.customer [1]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=======================
| Function Declarations |
=======================*/
void	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	Process 			(void);
int  	spec_valid 			(int);
void 	PrintAll 			(void);
int  	DeleteLine 			(void);
void 	SrchInvoice 		(char *, char *, char *);
void	PrintFunction 		(int, long, char *, char *);
int  	heading 			(int);
char 	*CheckVariable 		(char *, char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc < 3)
	{
		print_at (0, 0, "Usage : %s <LPNO> <TYPE I/C> ", argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	SETUP_SCR (vars);

	printerNumber = atoi (argv [1]);

	switch (argv [2] [0])
	{
	case	'I':
	case	'i':
		typeFlag = INVOICE;
		sprintf (titleStr," Archive %s ",(REPRINT)? ML (mlSoMess314):ML (mlSoMess319));
		sprintf (startStr," %s ", 	ML (mlSoMess324));
		sprintf (endStr, " %s ", 	ML (mlSoMess329));
		break;

	case	'C':
	case	'c':
		typeFlag = CREDIT_NOTE;
		sprintf (titleStr, " Archive %s ", (REPRINT)?ML (mlSoMess315):ML (mlSoMess320));
		sprintf (startStr, " %s ", ML (mlSoMess325));
		sprintf (endStr, " %s ", ML (mlSoMess330));
		break;

	default:
		print_at (0, 0, ML (mlSoMess752));
		return (EXIT_FAILURE);
	}

	findStatus = FALSE;
	sprintf (local_rec.find_codes, "%-12.12s", " ");

	vars [0].prmpt = TYPE._prmpt;
	vars [2].prmpt = TYPE._prmpt;

	envDbCo		= atoi (get_env ("DB_CO"));
	envDbFind	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (runPrintProgram, CheckVariable (TYPE._env, TYPE._prg));

	strcpy (branchNumber, (!envDbCo) ? " 0" : comm_rec.est_no);

	init_scr	();
	set_tty		(); 
	set_masks	();

	while (prog_exit == 0)
	{
		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;

		init_vars (1);
		lcount [1] = 0;

		heading (1);
		entry (1);

		if (prog_exit || restart)
			break;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			break;

		prog_exit	=	TRUE;

		if (lcount [1] != 0)
			Process ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	FinishProgram ();
}

/*
 * Open Database Files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (arhr, arhr_list, ARHR_NO_FIELDS, "arhr_id_no");
	open_rec (arln, arln_list, ARLN_NO_FIELDS, "arln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (arhr);
	abc_fclose (arln);
	abc_fclose (cumr);
	abc_dbclose (data);
}

void
Process (void)
{
	int	firstTime = 1;

	clear ();
	print_at (0, 0, ML (mlStdMess035)); 

	if ((pout = popen (runPrintProgram, "w")) == 0)
	{
		sprintf (err_str, "Error in %s during (POPEN)", runPrintProgram);
		sys_err (err_str, errno, PNAME);
	}

	for (line_cnt = 0;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt);

		strcpy (arhr_rec.co_no, comm_rec.co_no);
		strcpy (arhr_rec.br_no, comm_rec.est_no);
		strcpy (arhr_rec.type, TYPE._type);
		sprintf (arhr_rec.inv_no, "%-8.8s", local_rec.inv_no [0]);
		cc = find_rec (arhr, &arhr_rec, GTEQ, "r");

		while (!cc && 
			!strcmp (arhr_rec.co_no, comm_rec.co_no) && 
			!strcmp (arhr_rec.br_no, comm_rec.est_no) && 
			 arhr_rec.type [0] == TYPE._type [0] &&
			strcmp (arhr_rec.inv_no, local_rec.inv_no [1]) <= 0)
		{
			arln_rec.hhco_hash 	= arhr_rec.hhco_hash;
			arln_rec.line_no 	= 0;
			cc = find_rec (arln, &arln_rec, GTEQ, "r");
			if (cc || arhr_rec.hhco_hash != arln_rec.hhco_hash)
			{
				cc = find_rec (arhr, &arhr_rec, NEXT, "r");
				continue;
			}

			if (firstTime)
				PrintFunction (printerNumber, -1L, "M", TYPE._prt);

			PrintFunction (printerNumber, arhr_rec.hhco_hash, "M", TYPE._prt);

			if (firstTime)
			{
				sprintf (err_str, "Reprinting %ss", TYPE._desc);
				dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
			}

			firstTime = 0;
			dsp_process (TYPE._desc, arhr_rec.inv_no);
		
			cc = find_rec (arhr, &arhr_rec, NEXT, "r");
		}
	}
	if (!firstTime)
		PrintFunction (printerNumber, 0L, "M", TYPE._prt);

	pclose (pout);
}

int
spec_valid (
	int		field)
{
	char	lowInvoice [9];
	char	highInvoice [9];

	if (LCHECK ("start_inv_no")) 
	{
		if (dflt_used)
			return (DeleteLine ());

		if (!strcmp (local_rec.inv_no [0], "ALL     "))
		{
			PrintAll ();
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.inv_no [0], zero_pad (local_rec.inv_no [0], 8));

		strcpy (lowInvoice, "        ");
		strcpy (highInvoice, (prog_status == ENTRY) ? "~~~~~~~~" : local_rec.inv_no [1]);
		if (SRCH_KEY)
		{
			SrchInvoice (lowInvoice, temp_str, highInvoice);
			return (EXIT_SUCCESS);
		}

		strcpy (arhr_rec.co_no, comm_rec.co_no);
		strcpy (arhr_rec.br_no, comm_rec.est_no);
		strcpy (arhr_rec.type, TYPE._type);
		strcpy (arhr_rec.inv_no, local_rec.inv_no [0]);
		cc = find_rec (arhr, &arhr_rec, COMPARISON, "r");
		if (cc)
		{
			/*------------------------------------------------------
			| %s No %s not on file", TYPE._desc, local_rec.inv_no [0] |
			------------------------------------------------------*/
			switch (typeFlag)
			{
				case  INVOICE		:
					strcpy (err_str, ML (mlStdMess115));
					break;
				case CREDIT_NOTE	:
					strcpy (err_str, ML (mlStdMess116));
					break;

				default	: break;
			}
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (strcmp (arhr_rec.inv_no, highInvoice) > 0)
		{
			/*----------------
			| Invalid range. |
			----------------*/
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		cumr_rec.hhcu_hash = arhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------------
			| Cannot find Customer (%ld) |
			--------------------------*/
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.customer [0], "%-20.20s", cumr_rec.dbt_name);
		DSP_FLD ("start_customer");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_inv_no")) 
	{
		if (dflt_used)
			strcpy (local_rec.inv_no [1], local_rec.inv_no [0]);

		if (!strcmp (local_rec.inv_no [1], "ALL     "))
		{
			PrintAll ();
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.inv_no [1], zero_pad (local_rec.inv_no [1], 8));

		strcpy (lowInvoice, local_rec.inv_no [0]);
		strcpy (highInvoice, "~~~~~~~~");
		if (SRCH_KEY)
		{
			SrchInvoice (lowInvoice, temp_str, highInvoice);
			return (EXIT_SUCCESS);
		}

		strcpy (arhr_rec.co_no, comm_rec.co_no);
		strcpy (arhr_rec.br_no, comm_rec.est_no);
		strcpy (arhr_rec.type, TYPE._type);
		strcpy (arhr_rec.inv_no, local_rec.inv_no [1]);
		cc = find_rec (arhr, &arhr_rec, COMPARISON, "r");
		if (cc)
		{
			switch (typeFlag)
			{
				case  INVOICE		:
					strcpy (err_str, ML (mlStdMess115));
					break;
				case CREDIT_NOTE	:
					strcpy (err_str, ML (mlStdMess116));
					break;

				default	: break;
			}
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (strcmp (arhr_rec.inv_no, lowInvoice) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		cumr_rec.hhcu_hash = arhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.customer [1], "%-20.20s", cumr_rec.dbt_name);
		DSP_FLD ("end_customer");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
PrintAll (void)
{
	sprintf (local_rec.inv_no [0], "%-8.8s", "00000000");
	sprintf (local_rec.inv_no [1], "%-8.8s", "~~~~~~~~");
	sprintf (local_rec.customer [0], "%-30.30s", ML ("ALL Customers"));
	sprintf (local_rec.customer [1], "%-30.30s", ML ("ALL Customers"));
	DSP_FLD ("start_inv_no");
	DSP_FLD ("end_inv_no");
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
		/*
		 * Cannot Delete Lines on Entry
		 */
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

	strcpy (local_rec.inv_no [0], "        ");
	strcpy (local_rec.inv_no [1], "        ");
	strcpy (local_rec.customer [0], "                    ");
	strcpy (local_rec.customer [1], "                    ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*
 * Search for order number.
 */
void
SrchInvoice (
	 char	*lowInvoice, 
	 char	*keyValue, 
	 char	*highInvoice)
{
	work_open ();
	sprintf (err_str, "#%s", TYPE._prmpt);
	save_rec (err_str, "#Customer");

	strcpy (arhr_rec.co_no, comm_rec.co_no);
	strcpy (arhr_rec.br_no, comm_rec.est_no);
	strcpy (arhr_rec.type, TYPE._type);
	sprintf (arhr_rec.inv_no, "%-8.8s", keyValue);

	cc = find_rec (arhr, &arhr_rec, GTEQ, "r");

	while (!cc && !strcmp (arhr_rec.co_no, comm_rec.co_no) && 
				  !strcmp (arhr_rec.br_no, comm_rec.est_no) &&
				  arhr_rec.type [0] == TYPE._type [0])
	{
		arln_rec.hhco_hash 	= arhr_rec.hhco_hash;
		arln_rec.line_no 	= 0;

		cc = find_rec (arln, &arln_rec, GTEQ, "r");

		if (!cc && arln_rec.hhco_hash == arhr_rec.hhco_hash && 
		     strcmp (arhr_rec.inv_no, lowInvoice) >= 0 && 
		     strcmp (arhr_rec.inv_no, highInvoice) <= 0)
		{
			cumr_rec.hhcu_hash = arhr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (!cc)
			{
				cc = save_rec (arhr_rec.inv_no, cumr_rec.dbt_name);
				if (cc)
					break;
			}
		}
		cc = find_rec (arhr, &arhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (arhr_rec.co_no, comm_rec.co_no);
	strcpy (arhr_rec.br_no, comm_rec.est_no);
	strcpy (arhr_rec.type, 	TYPE._type);
	sprintf (arhr_rec.inv_no, "%-8.8s", temp_str);
	cc = find_rec (arhr, &arhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, arhr, "DBFIND");
}

void
PrintFunction (
	int		printerNumber,
	long	hhcoHash,
	char 	*mode,
	char 	*typeFlag)
{
	int		c	=	0;
	static int	printAll = 0;
	char	*sptr;

	if (!printAll)
	{
		sptr = chk_env ("LINE_UP");
		if (sptr != (char *)0)
		{
			c = atoi (sptr);
			printAll = !(c);
		}

		fprintf (pout,"%d\n",printerNumber);
		fprintf (pout,"%s\n",mode);
		fflush (pout);

		if (printAll)
		{
			fprintf (pout,"0\n");
			fflush (pout);
			return;
		}
	}

	do
	{
		fprintf (pout,"%ld\n",hhcoHash);
		fflush (pout);
		if (!printAll)
		{
			switch (typeFlag [0])
			{
			case	'I':
			case	'i':
			strcpy (err_str,ML ("Reprint Invoice (for lineup) <Y/N> ? "));
			break;

			case	'C':
			case	'c':
			strcpy (err_str,ML ("Reprint Credit Note (for lineup) <Y/N> ? "));
			break;

			default:
				return;
			}

			sleep (sleepTime);
			clear ();
			c = prmptmsg (err_str,"YyNn",26,1);
			if (mode[0] == 'M' && (c == 'N' || c == 'n'))
			{
				fprintf (pout,"0\n");
				fflush (pout);
			}
			clear ();
		}
	} while (!printAll && (c == 'Y' || c == 'y'));
	printAll = 1;
}

/*
 * Check for environment for run program name. 
 */
char *   
CheckVariable (
	char *environmentName, 
	char *programName)
{
	char	*sptr;
	char	runPrint [41];

	/*
	 * Check Company & Branch
	 */
	sprintf (runPrint,"%s%s%s",programName, comm_rec.co_no,comm_rec.est_no);
	sptr = chk_env (runPrint);
	if (sptr == (char *)0)
	{
		/*
		 * Check Company
		 */
		sprintf (runPrint,"%s%s",environmentName,comm_rec.co_no);
		sptr = chk_env (runPrint);
		if (sptr == (char *)0)
		{
			sprintf (runPrint,"%s",environmentName);
			sptr = chk_env (runPrint);
			return ((sptr == (char *)0) ? programName : sptr);
		}
		else
			return (sptr);
	}
	else
		return (sptr);
}
int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	clear ();

	rv_pr (titleStr, (80 - strlen (titleStr)) / 2, 0, 1);

	line_at (1,0,80);

	rv_pr (startStr, (40 - strlen (startStr)) / 2, tab_row - 2, 1);
	rv_pr (endStr, 38 + (40 - strlen (err_str)) / 2, tab_row - 2, 1);

	line_at (20,0,80);
	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21, 0, "%s", err_str);

	sprintf (err_str, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 0, "%s", err_str);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
