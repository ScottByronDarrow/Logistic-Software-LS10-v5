/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_rc_dsp.c,v 5.6 2002/09/06 02:15:30 scott Exp $
|  Program Name  : (gl_rc_dsp.c)
|  Program Desc  : (General Ledger Recovery Code Display/Print)
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
| $Log: gl_rc_dsp.c,v $
| Revision 5.6  2002/09/06 02:15:30  scott
| S/C 4288 Updated for looping
|
| Revision 5.5  2002/07/17 09:57:13  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/03/08 01:12:54  scott
| Updated to remove disk based sort routines.
|
| Revision 5.3  2001/08/09 09:13:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:30  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:57  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_rc_dsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_rc_dsp/gl_rc_dsp.c,v 5.6 2002/09/06 02:15:30 scott Exp $";

#define SCREEN_SIZE 14
#define SCREEN_X 5
#define SCREEN_Y 2
#define LSL_PSIZE 50

#include <pslscr.h>
#include <hot_keys.h>
#include <pr_format3.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <arralloc.h>

#include	"schema"

struct commRecord	comm_rec;
struct glraRecord	glra_rec;
struct glrcRecord	glrc_rec;
struct glriRecord	glri_rec;
struct inmrRecord	inmr_rec;

	char	*data = "data",
			*inmr1 = "inmr1",
			*inmr2 = "inmr2";

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [43];
	char	recCode		[sizeof glra_rec.code];
	char	catCode		[sizeof inmr_rec.category];
	char	sellGrp		[sizeof inmr_rec.sellgrp];
	char	itemNo		[sizeof inmr_rec.item_no];
	char	brNo		[sizeof glri_rec.br_no];
	char	whNo		[sizeof glri_rec.wh_no];
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

struct	{
	char	dummy [11];
	char	startCode [6], endCode [6];
	char	print [8];
	char	print_desc [8];
	int		printer;
} local_rec;
extern	int		TruePosition;
extern	int		EnvScreenOK;

static struct var vars [] = 
{
	{1, LIN, "st_code",	 3, 2, CHARTYPE,
	 "UUUUU", "          ",
	 " ", " ", "Start Code    ", " ",
	 YES, NO,  JUSTLEFT, "", "", local_rec.startCode},
	{1, LIN, "en_code",	 4, 2, CHARTYPE,
	 "UUUUU", "          ",
	 " ", " ", "End Code      ", " ",
	 YES, NO,  JUSTLEFT, "", "", local_rec.endCode},
	{1, LIN, "print",	 6, 2, CHARTYPE,
	 "U", "          ",
	 " ", " ", "Display/Print  ", " ",
	 YES, NO,  JUSTLEFT, "DP", "", local_rec.print},
	{1, LIN, "print_desc",	 6, 22, CHARTYPE,
	 "AAAAAAA", "          ",
	 " ", "", "", "",
	 NA, NO,  JUSTLEFT, "", "", local_rec.print_desc},
	{1, LIN, "printer",	 7, 2, INTTYPE,
	 "NN", "          ",
	 " ", " ", "Printer No    ", " ",
	 YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.printer},

	{0, LIN, "dummy",	 0, 0, CHARTYPE,
	 "U", "          ",
	 " ", "", "", " ",
	 ND, NO,  JUSTLEFT, "", "", local_rec.dummy},
};

extern int  _wide;
/*
 * Local Function Prototypes
 */
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Title 			(void);
int 	heading 		(int);
int 	spec_valid 		(int);
void 	SrchGlrc 		(char *, char *, char *);
void 	Process 		(void);
void 	Sort 			(void);
void 	Print 			(void);
void 	Display 		(void);
void 	GetLine 		(int);

int
main (
 int                argc,
 char*              argv [])
{
	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;
	OpenDB ();

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	prog_exit = FALSE;
	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = FALSE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;
		init_vars (1);

		heading (1);
		entry (1);

		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Process ();
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}



void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (inmr1, inmr);
	abc_alias (inmr2, inmr);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (glra, glra_list, GLRA_NO_FIELDS, "glra_code_id");
	open_rec (glrc, glrc_list, GLRC_NO_FIELDS, "glrc_id_no");
	open_rec (glri, glri_list, GLRI_NO_FIELDS, "glri_full_id");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr1, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

void
CloseDB (void)
{
	abc_fclose (inmr2);
	abc_fclose (inmr1);
	abc_fclose (inmr);
	abc_fclose (glri);
	abc_fclose (glrc);
	abc_fclose (glra);

	abc_dbclose (data);
}

void
Title (void)
{
	strcpy (err_str, ML (" General Ledger Recovery Code Display/Print "));

	clear ();
	rv_pr (err_str, ( (_wide ? 132 : 80) - strlen (err_str)) / 2, 0, 1);

	line_at (5,1, _wide ? 131 : 79);
	line_at (1,0, _wide ? 132 : 80);
	line_at (21,0, _wide ? 132 : 80);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
}

int
heading (
 int                screen)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (screen != cur_screen)
		scn_set (screen);

	Title ();

	box (0, 2, 80, 5);		

	line_cnt = 0;
	scn_write (screen);
    return (EXIT_SUCCESS);
}


int
spec_valid (
 int                field)
{
	if (LCHECK ("st_code"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCode, "     ");
			DSP_FLD ("st_code");
			return (EXIT_SUCCESS);

		}

		if (SRCH_KEY)
		{
			if (prog_status == ENTRY)
				SrchGlrc ("     ", temp_str, "~~~~~");
			else
				SrchGlrc ("     ", temp_str, local_rec.endCode);
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.startCode, local_rec.endCode) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (glrc_rec.co_no, comm_rec.co_no);
		sprintf (glrc_rec.code, "%-5.5s", local_rec.startCode);
		cc = find_rec (glrc, &glrc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("en_code"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCode, "~~~~~");
			DSP_FLD ("en_code");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchGlrc (local_rec.startCode, temp_str, "~~~~~");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.endCode, local_rec.startCode) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (glrc_rec.co_no, comm_rec.co_no);
		sprintf (glrc_rec.code, "%-5.5s", local_rec.endCode);
		cc = find_rec (glrc, &glrc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("print"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.print, "D");
			strcpy (local_rec.print_desc, ML ("Display"));
			DSP_FLD ("print");
			DSP_FLD ("print_desc");
			FLD ("printer") = NA;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			print_mess (ML ("Search not available on this field."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!strcmp ("D", local_rec.print))
		{
			strcpy (local_rec.print_desc, ML ("Display"));
			DSP_FLD ("print_desc");
			FLD ("printer") = NA;
			return (EXIT_SUCCESS);
		}

		if (!strcmp ("P", local_rec.print))
		{
			strcpy (local_rec.print_desc, ML ("Print  "));
			DSP_FLD ("print_desc");
			FLD ("printer") = YES;

			if (prog_status != ENTRY)
			{
				get_entry (label ("printer"));
				while (TRUE)
				{
					if (spec_valid (label ("printer")))
						get_entry (label ("printer"));
					else
						break;
				}
				DSP_FLD ("printer");
			}

			return (EXIT_SUCCESS);
		}

		return (EXIT_FAILURE);
	}

	if (LCHECK ("printer"))
	{
		if (FLD ("printer") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			local_rec.printer = 1;
			DSP_FLD ("printer");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			local_rec.printer = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printer))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchGlrc (
 char*              firstVal,
 char*              keyVal,
 char*              lastVal)
{
	int keyValLen = strlen (keyVal);

	work_open ();
	cc = save_rec ("#Code", "#Description");

	if (strncmp (keyVal, firstVal, keyValLen) < 0)
	{
		cc = disp_srch ();
		work_close ();
		return;
	}

	strcpy (glrc_rec.co_no, comm_rec.co_no);
	strcpy (glrc_rec.code, strcmp (keyVal, firstVal) > 0 ? keyVal 
													     : firstVal);
	for (cc = find_rec (glrc, &glrc_rec, GTEQ, "r");
		!cc && 
	     !strcmp (glrc_rec.co_no, comm_rec.co_no) &&
		 strcmp (glrc_rec.code, lastVal) <= 0 &&
	     !strncmp (glrc_rec.code, keyVal, keyValLen);
		cc = find_rec (glrc, &glrc_rec, NEXT, "r"))
	{
		if (save_rec (glrc_rec.code, glrc_rec.desc))
			break;
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (glrc_rec.co_no, comm_rec.co_no);
	sprintf (glrc_rec.code, "%-5.5s", temp_str);
	cc = find_rec (glrc, &glrc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, glrc, "DBFIND");
}

void
Process (void)
{
	Sort ();
	if (local_rec.print [0] == 'P')
		Print ();
	else
		Display ();
}

void
Sort (void)
{
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	print_mess (ML ("Reading GL Recovery Code Allocations"));

	strcpy (glra_rec.co_no, comm_rec.co_no);
	strcpy (glra_rec.code, local_rec.startCode);
	for (cc = find_rec (glra, &glra_rec, GTEQ, "r");
		 !cc &&
		  !strcmp (glra_rec.co_no, comm_rec.co_no) &&
		  strcmp (glra_rec.code, local_rec.endCode) <= 0;
		 cc = find_rec (glra, &glra_rec, NEXT, "r"))
	{
		cc = find_hash (inmr2, &inmr_rec, EQUAL, "r", glra_rec.hhbr_hash);
		if (cc)
			file_err (cc, inmr, "DBFIND");

		strcpy (glri_rec.co_no, glra_rec.co_no);
		strcpy (glri_rec.code, glra_rec.code);
		strcpy (glri_rec.br_no, "  ");
		strcpy (glri_rec.wh_no, "  ");
		for (cc = find_rec (glri, &glri_rec, GTEQ, "r");
			 !cc &&
			  !strcmp (glri_rec.co_no, comm_rec.co_no) &&
			  !strcmp (glri_rec.code, glra_rec.code);
			 cc = find_rec (glri, &glri_rec, NEXT, "r"))
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
				sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

			/*
			 * Load values into array element sortCnt.
			 */
			sprintf 
			(
				sortRec [sortCnt].sortCode, 
				"%-5.5s%-11.11s%-6.6s%-16.16s%2.2s%2.2s",
				glra_rec.code,
				inmr_rec.category,
				inmr_rec.sellgrp,
				inmr_rec.item_no,
				glri_rec.br_no,
				glri_rec.wh_no
			);
			strcpy (sortRec [sortCnt].recCode, glra_rec.code);
			strcpy (sortRec [sortCnt].catCode, inmr_rec.category);
			strcpy (sortRec [sortCnt].sellGrp, inmr_rec.sellgrp);
			strcpy (sortRec [sortCnt].itemNo, inmr_rec.item_no);
			strcpy (sortRec [sortCnt].brNo, glri_rec.br_no);
			strcpy (sortRec [sortCnt].whNo, glri_rec.wh_no);
			/*
			 * Increment array counter.
			 */
			sortCnt++;
		}
	}
	print_mess (ML ("Sorting GL Recovery Code Allocations"));
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
}

void
Print (void)
{
	FILE *input, *output;

	int		currentLine = 1,
			firstCode = TRUE,
			newCode,
			newCat,
			newSell,
			i;
	char	lastCode [6] = "",
			lastCat [12] = "",
			lastSell [7] = "";

	if (! (input = pr_open ("gl_rc_dsp.p")))
		sys_err ("Error in gl_rc_dsp.p during pr_open ()", 0, PNAME);
	if (! (output = popen ("pformat", "w")))
		sys_err ("Error in pformat during popen ()", 0, PNAME);
	
	print_mess (ML ("Printing GL Recovery Code Allocations"));

	fprintf (output, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (output, ".OP\n");
	fprintf (output, ".PL0\n");
	fprintf (output, ".LP%d\n", local_rec.printer);
	fprintf (output, ".1\n");
	fprintf (output, ".PI12\n");
	fprintf (output, ".L130\n");

	newCode = FALSE;
	newCat = FALSE;
	newSell = FALSE;
	for (i = 0; i < sortCnt; i++)
	{
		GetLine (i);

		if (strcmp (lastCode, glrc_rec.code))
		{
			newCode = TRUE;
			strcpy (lastCode, glrc_rec.code);
		}
		if (strcmp (lastCat, inmr_rec.category))
		{
			newCat = TRUE;
			strcpy (lastCat, inmr_rec.category);
		}
		if (strcmp (lastSell, inmr_rec.sellgrp))
		{
			newSell = TRUE;
			strcpy (lastSell, inmr_rec.sellgrp);
		}

		if (newCode)
		{
			if (!firstCode)
			{
				pr_format (input, output, "LINE", 0, 0);
				currentLine++;
				pr_format (input, output, "VBLEBLANK", 1,
						   LSL_PSIZE - currentLine + 1);
				pr_format (input, output, "NEXTPAGE", 0, 0);
			}
			else
				firstCode = FALSE;

			pr_format (input, output, "LINE", 0, 0);
			pr_format (input, output, "MAINHEAD1", 0, 0);
			pr_format (input, output, "MAINHEAD2", 0, 0);
			pr_format (input, output, "LINE", 0, 0);
			pr_format (input, output, "CDETAIL", 1, glrc_rec.code);
			pr_format (input, output, "CDETAIL", 2, glrc_rec.desc);
			pr_format (input, output, "CDETAIL", 3, DOLLARS (glrc_rec.value));
			pr_format (input, output, "LINE", 0, 0);

			currentLine = 7;
		}

		pr_format (input, output, "MDETAIL", 1,
				   newCode || newCat ? inmr_rec.category : "");
		pr_format (input, output, "MDETAIL", 2, 
				   newCode || newCat || newSell ? inmr_rec.sellgrp : "");
		pr_format (input, output, "MDETAIL", 3, inmr_rec.item_no);
		pr_format (input, output, "MDETAIL", 4, inmr_rec.description);
		pr_format (input, output, "MDETAIL", 5, comm_rec.co_no);
		pr_format (input, output, "MDETAIL", 6, glri_rec.br_no);
		pr_format (input, output, "MDETAIL", 7, glri_rec.wh_no);
		pr_format (input, output, "MDETAIL", 8, glri_rec.acc_no);

		currentLine++;
		if (currentLine > LSL_PSIZE)
		{
			pr_format (input, output, "NEXTPAGE", 0, 0);
			currentLine = 1;
		}
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);

	if (!firstCode)
	{
		pr_format (input, output, "LINE", 0, 0);
		currentLine++;
		pr_format (input, output, "VBLEBLANK", 1,
				   LSL_PSIZE - currentLine + 1);
		pr_format (input, output, "NEXTPAGE", 0, 0);
	}

	pclose (output);
	fclose (input);
}



void
Display (void)
{
	int		newCode,
			newCat,
			newSell,
			i;
	char	lastCode [6] = "",
			lastCat [12] = "",
			lastSell [7] = "";
	char	displayString [160];

	clear ();
	swide ();

	Title ();

	Dsp_open (SCREEN_X, SCREEN_Y, SCREEN_SIZE);

	Dsp_saverec (" Category    | Sell   | Item             |"
				 " Description                              |"
				 " Co | BR | Wh |  G.L.  Account   ");
	Dsp_saverec ("");
	Dsp_saverec (" [REDRAW]  [NEXT SCN]  [PREV SCN]  [EDIT/END]");

	newCode = FALSE;
	newCat = FALSE;
	newSell = FALSE;
	for (i = 0; i < sortCnt; i++)
	{
		GetLine (i);

		if (strcmp (lastCode, glrc_rec.code))
		{
			newCode = TRUE;
			strcpy (lastCode, glrc_rec.code);
		}
		if (strcmp (lastCat, inmr_rec.category))
		{
			newCat = TRUE;
			strcpy (lastCat, inmr_rec.category);
		}
		if (strcmp (lastSell, inmr_rec.sellgrp))
		{
			newSell = TRUE;
			strcpy (lastSell, inmr_rec.sellgrp);
		}

		if (newCode)
		{
			sprintf (displayString,
					 " Code :  %-5.5s (%-20.20s)    Value :  %8.2f",
					 glrc_rec.code, glrc_rec.desc, DOLLARS (glrc_rec.value));
			Dsp_saverec (displayString);
		}
		sprintf (displayString, " %-11.11s ^E"
		                        " %-6.6s ^E %-16.16s ^E %-40.40s ^E %2.2s ^E"
								" %2.2s ^E %2.2s ^E %-16.16s ",
				 newCode || newCat ? inmr_rec.category : " ",
				 newCode || newCat || newSell ? inmr_rec.sellgrp : " ",
				 inmr_rec.item_no,
				 inmr_rec.description,
				 comm_rec.co_no,
				 glri_rec.br_no,
				 glri_rec.wh_no,
				 glri_rec.acc_no);

		Dsp_saverec (displayString);
	}

	Dsp_srch ();
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);

	clear ();
	snorm ();
}

void
GetLine (
	int		i)
{
	strcpy (glra_rec.code, 		sortRec [i].recCode);
	strcpy (inmr_rec.item_no,	sortRec [i].itemNo);
	strcpy (glri_rec.br_no, 	sortRec [i].brNo);
	strcpy (glri_rec.wh_no, 	sortRec [i].whNo);
	
	strcpy (glrc_rec.co_no, comm_rec.co_no);
	strcpy (glrc_rec.code, glra_rec.code);
	cc = find_rec (glrc, &glrc_rec, EQUAL, "r");
	if (cc)
		file_err (cc, glrc, "DBFIND");

	strcpy (glri_rec.co_no, comm_rec.co_no);
	strcpy (glri_rec.code, glra_rec.code);
	cc = find_rec (glri, &glri_rec, EQUAL, "r");
	if (cc)
		file_err (cc, glri, "DBFIND");

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	cc = find_rec (inmr1, &inmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");
}

int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}
