/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: miss_inv.c,v 5.6 2002/07/24 02:01:29 scott Exp $
|  Program Name  : ( so_miss_inv.c)
|  Program Desc  : ( Report on missing invoices)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 03/11/89         |
|---------------------------------------------------------------------|
| $Log: miss_inv.c,v $
| Revision 5.6  2002/07/24 02:01:29  scott
| S/C 004210
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: miss_inv.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_miss_inv/miss_inv.c,v 5.6 2002/07/24 02:01:29 scott Exp $";

#define	isdig(x)	(x >= '0' && x <= '9') 
#define	ACTIVE		(somc_rec.active [0] == 'Y')
#define	INV_LEN		8

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#include "schema"

struct commRecord   comm_rec;
struct esmrRecord   esmr_rec;
struct somcRecord   somc_rec;
struct somiRecord   somi_rec;

FILE	*fout;	

int		lpno = 1;
int		num_digit = 0;

char	nextInvoice [sizeof somc_rec.start_seq];

char	store [10][sizeof somc_rec.start_seq];

char	*somi2	=	"somi2";
/*
 * Local & Screen Structures. 
 */

struct {
	char	dummy [11];
	char	startInvoice [sizeof somc_rec.start_seq];
	char	endInvoice 	 [sizeof somc_rec.start_seq];
} local_rec;

static	struct	var	vars[]	={	

	{1, TAB, "startInvoice", MAXLINES, 4, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", "Start Invoice.", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startInvoice}, 
	{1, TAB, "endInvoice", 0, 4, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", " End Invoice. ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.endInvoice}, 

	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*
 * Function Declarations 
 */
void 	ProcessData 		(void);
void 	HeadOutput 			(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	GetBranchNo 		(void);
int  	spec_valid 			(int);
void 	DisectInvoice 		(char *);
void 	ProcessSomc 		(void);
void 	ProcessSomi 		(void);
int  	StoreMissing 		(char *, char *, int);
void 	CheckSequence 		(char *);
int  	ValidCoBr 			(void);
int  	heading 			(int);
int  	DeleteLine 			(void);
void 	SrchSomc 			(char *);

/*
 * Main Processing Routine. 
 */
int
main (
	int		argc,
	char	*argv [])
{
	if (argc != 2)
	{
		print_at (0, 0, mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}

	lpno = atoi (argv [1]);

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	GetBranchNo ();

	/*
	 * Reset control flags 
	 */
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		init_vars (1);
		lcount [1] = 0;
		heading (1);
		entry (1);

		if (prog_exit)
            break; 

		heading (1);
		scn_display (1);
		edit (1);

		if (!restart)
			ProcessData ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
ProcessData (void)
{
	rset_tty ();
	clear ();

	dsp_screen ("Processing : Missing Invoices Report", 
				comm_rec.co_no, comm_rec.co_name);

	/*
	 * Open pipe work file to pformat. 
 	 */
	if ((fout = popen ("pformat", "w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);
        prog_exit = 1;
	}

	HeadOutput ();

	ProcessSomc ();

	fprintf (fout, ".EOF\n");

	pclose (fout);
	set_tty ();
}

/*
 * Start Out Put To Standard Print 
 */
void
HeadOutput (void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", lpno);
	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L64\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany : %s - %s \n", comm_rec.co_no, comm_rec.co_name);
	fprintf (fout, ".EAS AT : %s\n", SystemTime());

	fprintf (fout, ".EBranch : %s - %s\n", comm_rec.est_no, esmr_rec.est_name);
	fprintf (fout, "Start Invoice No: %s", local_rec.startInvoice);
	fprintf (fout, "                  ");
	getval (lcount [1]-1);
	fprintf (fout, "End Invoice No: %s\n", local_rec.endInvoice);

	fprintf (fout, ".EMISSING INVOICES REPORT\n");

	fprintf (fout, "==========================================\n");
	fprintf (fout, ".R==========================================\n");

	fprintf (fout, "| INVOICE NUMBER                         |\n");

	fprintf (fout, "|----------------------------------------|\n");

	fflush (fout);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */

void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (somi2, somi);
	open_rec (somc, somc_list, SOMC_NO_FIELDS,"somc_id_no");
	open_rec (somi, somi_list, SOMI_NO_FIELDS, "somi_id_no");
	open_rec (somi2, somi_list, SOMI_NO_FIELDS, "somi_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/

void
CloseDB (void)
{
	abc_fclose (somc);
	abc_fclose (somi);
	abc_fclose (somi2);
	abc_fclose (esmr);
	abc_dbclose ("data");
}

void
GetBranchNo (void)
{
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, comm_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

int
spec_valid (
	int 	field)
{

	if (LCHECK  ("startInvoice"))
	{
		if (dflt_used)
			return (DeleteLine ());

		strcpy (local_rec.startInvoice, zero_pad (local_rec.startInvoice, 8));

		if (last_char == SEARCH)
		{
			SrchSomc (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (somc_rec.co_no, comm_rec.co_no);
		strcpy (somc_rec.br_no, comm_rec.est_no);
		strcpy (somc_rec.start_seq, local_rec.startInvoice);
		strcpy (somc_rec.end_seq, "        ");
		cc = find_rec (somc, &somc_rec, GTEQ, "r");
		if (cc || strcmp (somc_rec.start_seq, local_rec.startInvoice))
		{
			sprintf (err_str, ML(mlSoMess204), local_rec.startInvoice);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		else
		{
			if (!ACTIVE)
			{
				sprintf (err_str, ML (mlSoMess205), 
						 local_rec.startInvoice, local_rec.endInvoice);
				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			strcpy (local_rec.endInvoice, somc_rec.end_seq);
			DSP_FLD ("endInvoice");
		}
		return (cc);
	}
	return (EXIT_SUCCESS);
}

void
DisectInvoice (
	char 	*invoiceNo)
{
	int	i;

	num_digit = 0;
	for (i = 0; i < INV_LEN; i++)
	{
		if (isdig (invoiceNo [i]))
			num_digit++;
	}
}

void
ProcessSomc (void)
{
	int	tmp_lcount = lcount [1];

	for (line_cnt = 0; line_cnt < tmp_lcount; line_cnt++)
	{
		getval (line_cnt);

		strcpy (somc_rec.co_no, comm_rec.co_no);
		strcpy (somc_rec.br_no, comm_rec.est_no);
		strcpy (somc_rec.start_seq, local_rec.startInvoice);
		strcpy (somc_rec.end_seq, local_rec.endInvoice);
		
		cc = find_rec (somc, &somc_rec, GTEQ, "r");
		
		while (!cc && !strcmp (somc_rec.co_no, comm_rec.co_no) && 
			   !strcmp (somc_rec.br_no, comm_rec.est_no) && 
			   strcmp (somc_rec.end_seq, local_rec.endInvoice) <= 0)
		{
			if (ACTIVE)
			{
				DisectInvoice (somc_rec.start_seq);
				ProcessSomi ();
			}

			cc = find_rec (somc, &somc_rec, NEXT, "r");
		}
	}
}

void
ProcessSomi (void)
{
	int		firstTime = 1;
	int		first_flag = 0;
	int		i = 0;
	char	prevInvoice [sizeof somc_rec.start_seq];

	/*
	 * Clear missing invoice table	
	 */
	for (i = 0; i < 10; i++)
		strcpy (store [i], "        ");

	strcpy (somi_rec.co_no, somc_rec.co_no);
	strcpy (somi_rec.br_no, somc_rec.br_no);
	strcpy (somi_rec.inv_no, somc_rec.start_seq);
	
	cc = find_rec (somi, &somi_rec, GTEQ, "r");
	
	strcpy(nextInvoice, somc_rec.start_seq);
	strcpy(prevInvoice, somc_rec.start_seq);

	while (!cc && !strcmp (somi_rec.co_no, comm_rec.co_no) && 
		   !strcmp (somi_rec.br_no, comm_rec.est_no) && 
		   strcmp (somi_rec.inv_no, somc_rec.end_seq) <= 0)
	{
		if (!firstTime)
			CheckSequence (prevInvoice);

		if (strcmp (nextInvoice, somi_rec.inv_no) != 0)
			first_flag = StoreMissing (nextInvoice, somi_rec.inv_no, firstTime);

		/*
		 * Print iff table not empty ie. not spaces		
		 */
		if (strcmp (store [0], "        "))
		{
			if (first_flag <= 10)
			{
				for (i = 0; i < first_flag; i++)
				{
					fprintf (fout, "| %-38.38s |\n", store [i]);
					strcpy (store [i], "        ");
				}
			}
			else
			{
				fprintf (fout, "|%-8.8s TO %-8.8s%20.20s|\n", 
						 store [0], store [1], " ");
				strcpy (store [0], "        ");
				strcpy (store [1], "        ");
			}
			first_flag = 0;
		}
		strcpy (prevInvoice, somi_rec.inv_no);
		firstTime = 0;

		cc = find_rec (somi, &somi_rec, NEXT, "r");
	}
	if (!firstTime)
	{
		if (first_flag <= 10)
		{
			for (i = 0; i < first_flag; i++)
				fprintf(fout, "| %-38.38s |\n", store [i]);
		}
		else
			fprintf (fout, "|%-8.8s TO %-8.8s%20.20s|\n", 
					 store [0], store [1], " ");
			
		/*
		 * Print missing invoices at end of range		
		 */
		
		CheckSequence (prevInvoice);
		
		if (strcmp (nextInvoice, somc_rec.end_seq) <= 0)
		{
			first_flag = StoreMissing (nextInvoice, somc_rec.end_seq,firstTime);
			if (first_flag <= 10)
			{
				for (i = 0; i <= first_flag; i++)
					fprintf (fout, "| %-38.38s |\n", store[i]);
			}
			else
			{
				fprintf (fout, "|%-8.8s TO %-8.8s%20.20s|\n", 
						 store[0], store[1], " ");
			}
		}
	}
}

int
StoreMissing (
	char 	*nextInvoice, 
	char 	*currInvoice, 
	int 	firstTime)
{
	long	numer1;
	long	numer2;
	long	numer3;
	int		alpha_len;
	int		num_item = 0;

	alpha_len = INV_LEN - num_digit;

	numer1 = atol (nextInvoice + alpha_len);
	numer2 = atol (currInvoice + alpha_len);

	if (numer2 - numer1 > 10)
	{
		sprintf (store [num_item++], "%*.*s%0*ld", alpha_len, alpha_len, 
				 nextInvoice, num_digit, numer1);

		/*
		 * Find the end  invoice of the current range	
		 */
		strcpy (somi_rec.co_no, somc_rec.co_no);
		strcpy (somi_rec.br_no, somc_rec.br_no);
		sprintf (somi_rec.inv_no, "%-8.8s", currInvoice);

		cc = find_rec (somi, &somi_rec, COMPARISON, "r");

		if (cc)
			numer3 = numer2;
		else
			numer3 = numer2 - 1;

		sprintf (store [num_item], "%*.*s%0*ld", alpha_len, alpha_len, nextInvoice,
				 num_digit, numer3);
				 
		return(numer2 - numer1);
	}

	while (numer1 != numer2 && num_item <= 10)
	{
		sprintf (store [num_item++], "%*.*s%0*ld", alpha_len, alpha_len, 
				 nextInvoice, num_digit, numer1);
		numer1++;
	}
	if (numer1 == numer2 && num_item < 10)
	{
		sprintf (store [num_item], "%*.*s%0*ld", alpha_len, alpha_len, 
				 nextInvoice, num_digit, numer1);
	}

	return(num_item);
}

void
CheckSequence (
	char 	*invoiceNo)
{
	long	numer;
	int		alpha_len;

	alpha_len = INV_LEN - num_digit;

	numer = atol (invoiceNo + alpha_len);
	numer++;

	sprintf (nextInvoice, "%*.*s%0*ld", alpha_len, alpha_len, invoiceNo,
			 num_digit, numer);
}

int
ValidCoBr (void)
{
	if (!strcmp (somi_rec.co_no, comm_rec.co_no) && 
		!strcmp (somi_rec.br_no, comm_rec.est_no) && 
		strcmp (somi_rec.inv_no, somc_rec.end_seq) <= 0)
		return(1);
		
	return(0);
}

/*
 * Heading concerns itself with clearing the screen, painting the  
 * screen overlay in preparation for input                        
 */
int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
			
		clear();

		rv_pr (ML (mlSoMess206), 23, 0, 1);
		line_at (1,0,80);
		line_at (20,0,80);

		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	int		i;
	int		this_page;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return(1);
	}

	lcount [1]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount[1];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	strcpy (local_rec.startInvoice, "        ");
	strcpy (local_rec.endInvoice, "        ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (0);
}

void
SrchSomc (
	char 	*keyValue)
{
    _work_open(17,0,10);
	save_rec ("#START    END     ", "#Completed");
	strcpy (somc_rec.co_no, comm_rec.co_no);
	strcpy (somc_rec.br_no, comm_rec.est_no);
	sprintf (somc_rec.start_seq, "%-8.8s", keyValue);
	strcpy (somc_rec.end_seq, "        ");
	
	cc = find_rec(somc, &somc_rec, GTEQ, "r");
    while (!cc && !strcmp (somc_rec.co_no, comm_rec.co_no) && 
    	   !strcmp (somc_rec.br_no, comm_rec.est_no))
    {
		if (ACTIVE)
		{
			sprintf (err_str, "%-8.8s-%-8.8s", 
					 somc_rec.start_seq, somc_rec.end_seq);
			cc = save_rec (err_str,"Yes");
			if (cc)
				break;
		}

		cc = find_rec(somc, &somc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	
	if (cc)
		return;
	
	strcpy (somc_rec.co_no, comm_rec.co_no);
	strcpy (somc_rec.br_no, comm_rec.est_no);
	sprintf (somc_rec.start_seq, "%-8.8s", temp_str);
	sprintf (somc_rec.end_seq, "%-8.8s", temp_str + 9);
	
	strcpy (temp_str, somc_rec.start_seq);

	cc = find_rec (somc, &somc_rec, COMPARISON, "r");
	
	if (cc)
		file_err (cc, somc, "DBFIND");
}
