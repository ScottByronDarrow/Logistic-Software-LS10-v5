/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: rc_alloc.c,v 5.3 2001/08/09 09:13:53 scott Exp $
|=====================================================================|
|  Program Name  : (gl_rc_alloc.c)
|  Program Desc  : (General Ledger Recovery Code Allocation)
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
| $Log: rc_alloc.c,v $
| Revision 5.3  2001/08/09 09:13:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:29  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:56  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: rc_alloc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_rc_alloc/rc_alloc.c,v 5.3 2001/08/09 09:13:53 scott Exp $";

#define SLEEPTIME 3

#include <pslscr.h>
#include <hot_keys.h>
#include <tabdisp.h>
#include <ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct glraRecord	glra_rec;
struct glrcRecord	glrc_rec;
struct ingpRecord	ingp_rec;
struct inmrRecord	inmr_rec;

	char	*data = "data";


struct	{
	char	dummy 		[11];
	char	startItem 	[sizeof inmr_rec.item_no], 
			endItem 	[sizeof inmr_rec.item_no],
			startGroup 	[sizeof ingp_rec.code], 
			endGroup 	[sizeof ingp_rec.code],
			startCat 	[sizeof excf_rec.cat_no], 
			endCat 		[sizeof excf_rec.cat_no];
} local_rec;

static	struct	var	vars	 []	= 
{
	{1, LIN, "code", 3, 19, CHARTYPE, 
		"UUUUU", "          ", 
		" ", "", "Recovery Code     :", " ", 
		YES, NO, JUSTLEFT, "", "", glrc_rec.code}, 
	{1, LIN, "desc", 3, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", glrc_rec.desc}, 
	{1, LIN, "st_item", 5, 19, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Start Item        :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startItem}, 
	{1, LIN, "en_item", 5, 59, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "End Item          :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endItem}, 
	{1, LIN, "st_group", 6, 19, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Sell Group  :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startGroup}, 
	{1, LIN, "en_group", 6, 59, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End Sell Group    :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endGroup}, 
	{1, LIN, "st_cat", 7, 19, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", " ", "Start Category    :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startCat}, 
	{1, LIN, "en_cat", 7, 59, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", " ", "End Category      :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endCat}, 
	{0, LIN, "dummy", 0, 0, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.dummy}, 

};

static	int	tag_func (int c, KEY_TAB *psUnused);
static	int	exit_func (int c, KEY_TAB *psUnused);

static	KEY_TAB listKeys [] =
{
   { " [T]OGGLE ",		'T',		tag_func,
	"Tag Line.",					"A" },
   { " [^T]OGGLE ALL",		CTRL ('T'), 	tag_func,
	"Tag All Lines.",				"A" },
   { NULL,			FN1, 		exit_func,
	"Exit without update.",						"A" },
   { NULL,			FN16, 		exit_func,
	"Exit and update the database.",				"A" },
   END_KEYS
};

int		tableSize;


/*
 * Local Function Prototypes.
 */
void 	OpenDB 		(void);
void 	CloseDB 	(void);
int 	heading 	(int);
int 	spec_valid 	(int);
void 	SrchGlrc 	(char *);
void 	SrchInmr 	(char *, char *, char *);
void 	SrchIngp 	(char *, char *, char *);
void 	SrchExcf 	(char *, char *, char *);
int 	Process 	(void);
int 	TagLine 	(int);
void 	Update 		(void);


int
main (
 int                argc,
 char*              argv [])
{
	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

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
		{
			abc_unlock (glrc);
			continue;
		}

		heading (1);
		scn_display (1);
		cc = Process ();
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (glra, glra_list, GLRA_NO_FIELDS, "glra_id_no");
	open_rec (glrc, glrc_list, GLRC_NO_FIELDS, "glrc_id_no");
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (ingp);
	abc_fclose (glrc);
	abc_fclose (glra);
	abc_fclose (excf);

	abc_dbclose (data);
}

int
heading (
 int                screen)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (screen != cur_screen)
		scn_set (screen);

	clear ();
	strcpy (err_str, ML (" General Ledger Recovery Code Allocation "));
	rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);

	line_at (1,0,80);
	line_at (4,1,79);
	line_at (21,0,80);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	box (0, 2, 80, 5);		

	line_cnt = 0;
	scn_write (screen);

    return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
			SrchGlrc (temp_str);
			return (EXIT_FAILURE);
		}

		sprintf (glrc_rec.co_no, "%2.2s", comm_rec.co_no);
		sprintf (glrc_rec.code, "%-5.5s", temp_str);
		cc = find_rec (glrc, &glrc_rec, EQUAL, "u");
		if (cc)
		{
			print_mess (ML ("Recovery code not found."));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("desc"))
	{
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_item"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startItem, "                ");
			DSP_FLD ("st_item");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			if (prog_status == ENTRY)
			{
				SrchInmr ("                ", temp_str, "~~~~~~~~~~~~~~~~");
			}
			else
			{
				SrchInmr ("                ", temp_str, local_rec.endItem);
			}
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			print_mess (mlStdMess017);
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", local_rec.startItem);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (mlStdMess001);
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!strcmp (inmr_rec.inmr_class, "K") ||
			!strcmp (inmr_rec.inmr_class, "P") ||
			!strcmp (inmr_rec.inmr_class, "N") ||
			!strcmp (inmr_rec.inmr_class, "Z"))
		{
			print_mess (ML("Kits, phantoms, non stock items not allowed."));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("en_item"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endItem, "~~~~~~~~~~~~~~~~");
			DSP_FLD ("en_item");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchInmr (local_rec.startItem, temp_str, "~~~~~~~~~~~~~~~~");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.endItem, local_rec.startItem) < 0)
		{
			print_mess (mlStdMess018);
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", local_rec.endItem);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess009));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!strcmp (inmr_rec.inmr_class, "K") ||
			!strcmp (inmr_rec.inmr_class, "P") ||
			!strcmp (inmr_rec.inmr_class, "N") ||
			!strcmp (inmr_rec.inmr_class, "Z"))
		{
			print_mess (ML ("Kits, phantoms, non stock items not allowed."));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_group"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startGroup, "      ");
			DSP_FLD ("st_group");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			if (prog_status == ENTRY)
			{
				SrchIngp ("      ", temp_str, "~~~~~~");
			}
			else
			{
				SrchIngp ("      ", temp_str, local_rec.endGroup);
			}
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.startGroup, local_rec.endGroup) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (ingp_rec.co_no, comm_rec.co_no);
		strcpy (ingp_rec.type, "S");
		sprintf (ingp_rec.code, "%-6.6s", local_rec.startGroup);
		cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess009));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("en_group"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endGroup, "~~~~~~");
			DSP_FLD ("en_group");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchIngp (local_rec.startGroup, temp_str, "~~~~~~");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.endGroup, local_rec.startGroup) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (ingp_rec.co_no, comm_rec.co_no);
		strcpy (ingp_rec.type, "S");
		sprintf (ingp_rec.code, "%-6.6s", local_rec.endGroup);
		cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess009));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_cat"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCat, "           ");
			DSP_FLD ("st_cat");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			if (prog_status == ENTRY)
			{
				SrchExcf ("           ", temp_str, "~~~~~~~~~~~");
			}
			else
			{
				SrchExcf ("           ", temp_str, local_rec.endCat);
			}
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.startCat, local_rec.endCat) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", local_rec.startCat);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess009));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("en_cat"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCat, "~~~~~~~~~~~");
			DSP_FLD ("en_cat");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (local_rec.startCat, temp_str, "~~~~~~~~~~~");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.endCat, local_rec.startCat) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", local_rec.endCat);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess009));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}



void
SrchGlrc (
 char*              keyVal)
{	
	_work_open (5,0,40);
	sprintf (glrc_rec.co_no, "%2.2s", comm_rec.co_no);
	sprintf (glrc_rec.code, "%-5.5s", keyVal);
	save_rec ("#Code", "#Recovery Code Description");
	for (cc = find_rec (glrc, &glrc_rec, GTEQ, "r");
		 !cc && !strcmp (glrc_rec.co_no, comm_rec.co_no);
		 cc = find_rec (glrc, &glrc_rec, NEXT, "r"))
	{
		cc = save_rec (glrc_rec.code, glrc_rec.desc);
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	sprintf (glrc_rec.co_no, "%2.2s", comm_rec.co_no);
	sprintf (glrc_rec.code, "%-5.5s", temp_str);
	cc = find_rec (glrc, &glrc_rec, EQUAL, "r");
	if (cc)
		file_err (cc, glrc, "DBFIND");
}

void
SrchInmr (
	char	*firstVal,
	char	*keyVal,
	char	*lastVal)
{
	int keyValLen = strlen (keyVal);

	_work_open (16,0,40);
	cc = save_rec ("#Item Number", "#Description");

	if (strncmp (keyVal, firstVal, keyValLen) < 0)
	{
		cc = disp_srch ();
		work_close ();
		return;
	}

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, strcmp (keyVal, firstVal) > 0 ? keyVal 
														    : firstVal);
	for (cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
		!cc && 
	     !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
		 strcmp (inmr_rec.item_no, lastVal) <= 0 &&
	     !strncmp (inmr_rec.item_no, keyVal, keyValLen);
		cc = find_rec (inmr, &inmr_rec, NEXT, "r"))
	{
		if (!strcmp (inmr_rec.inmr_class, "K") ||
			!strcmp (inmr_rec.inmr_class, "P") ||
			!strcmp (inmr_rec.inmr_class, "N") ||
			!strcmp (inmr_rec.inmr_class, "Z"))
		{
			continue;
		}

		if (save_rec (inmr_rec.item_no, inmr_rec.description))
			break;
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", temp_str);
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");
}



void
SrchIngp (
	char	*firstVal,
	char	*keyVal,
	char	*lastVal)
{
	int keyValLen = strlen (keyVal);

	_work_open (6,0,40);
	save_rec ("#Code", "#Description ");

	if (strncmp (keyVal, firstVal, keyValLen) < 0)
	{
		cc = disp_srch ();
		work_close ();
		return;
	}

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "S");
	strcpy (ingp_rec.code, strcmp (keyVal, firstVal) > 0 ? keyVal 
													     : firstVal);
	for (cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
		!cc && 
	     !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
		 ingp_rec.type [0] == 'S' &&
		 strcmp (ingp_rec.code, lastVal) <= 0 &&
	     !strncmp (ingp_rec.code, keyVal, strlen (keyVal));
		cc = find_rec (ingp, &ingp_rec, NEXT, "r"))
	{
		if (save_rec (ingp_rec.code, ingp_rec.desc))
			break;
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "S");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}
void
SrchExcf (
	char	*firstVal,
	char	*keyVal,
	char	*lastVal)
{
	int keyValLen = strlen (keyVal);

	_work_open (6,0,40);
	save_rec ("#Category", "#Description ");

	if (strncmp (keyVal, firstVal, keyValLen) < 0)
	{
		cc = disp_srch ();
		work_close ();
		return;
	}
	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no, strcmp (keyVal, firstVal) > 0 ? keyVal 
													       : firstVal);
	for (cc = find_rec (excf, &excf_rec, GTEQ, "r");
		!cc && 
	     !strcmp (excf_rec.co_no, comm_rec.co_no) &&
		 strcmp (excf_rec.cat_no, lastVal) <= 0 &&
	     !strncmp (excf_rec.cat_no, keyVal, strlen (keyVal));
		cc = find_rec (excf, &excf_rec, NEXT, "r"))
	{
		if (save_rec (excf_rec.cat_no, excf_rec.cat_desc))
			break;
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}



int
Process (void)
{
	/*
	 * Open table
	 */
	tab_open ("ItemList", listKeys, 8, 8, 9, FALSE);
	tab_add ("ItemList",
			 "# %-16.16s  %-40.40s  %-3.3s ",
		     "Item Number", "Item Description", "TAG");

	tableSize = 0;
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.startItem);
	for (cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
		!cc && 
	     !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
		 strcmp (inmr_rec.item_no, local_rec.endItem) <= 0;
		cc = find_rec (inmr, &inmr_rec, NEXT, "r"))
	{
		/*
		 * Check that item falls within correct
		 * selling group and category range
		 */

		if (strcmp (inmr_rec.sellgrp, local_rec.startGroup) < 0 ||
			strcmp (inmr_rec.sellgrp, local_rec.endGroup) > 0 ||
		    strcmp (inmr_rec.category, local_rec.startCat) < 0 ||
			strcmp (inmr_rec.category, local_rec.endCat) > 0)
		{
			continue;
		}

		strcpy (glra_rec.co_no, comm_rec.co_no);
		strcpy (glra_rec.code, glrc_rec.code);
		glra_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (glra, &glra_rec, EQUAL, "r");

		tab_add ("ItemList", 
				 " %-16.16s  %-40.40s   %1.1s  ",
				 inmr_rec.item_no, inmr_rec.description, (cc) ? " " : "A");
		tableSize++;
	}

	if (tableSize == 0)
	{
		tab_add ("ItemList",
				 "No valid items.");
		tab_display ("ItemList", TRUE);
		sleep (SLEEPTIME);
		tab_close ("ItemList", TRUE);
		return (EXIT_SUCCESS);
	}
	else
	{
		tab_scan ("ItemList");
	}
	
	if (!restart)
		Update ();

	tab_close ("ItemList", TRUE);
	return (EXIT_FAILURE);
}

static int
tag_func (
	int		c,
	KEY_TAB	*psUnused)
{
	int		i, st_line = tab_tline ("ItemList");
	char	buffer [100];

	if (c == 'T')
		TagLine (st_line);
	else
	{
		for (i = st_line; i < tableSize; i++)
			TagLine (i);

		tab_display ("ItemList", TRUE);
		tab_get ("ItemList", buffer, EQUAL, st_line);
		redraw_keys ("ItemList");
	}

	return (c);
}

int
TagLine (
	int		line_no)
{
	char	buffer [100];
	char	status;

	tab_get ("ItemList", buffer, EQUAL, line_no);
	status = buffer [62];

	status = status == 'A' ? ' ' : 'A';

	tab_update ("ItemList", "%-62.62s%c ", buffer, status);

	return (EXIT_SUCCESS);
}

static int
exit_func (
	int		c,
	KEY_TAB	*psUnused)
{
	if (c == FN1)
		restart = TRUE;
	
	return (c);
}

void
Update (void)
{
	int		i;
	char	buffer [100];
	char	status;

	/*
	 * If this ends up taking too long the program  could always store the 
	 * initial states for each item and only add/delete for those items that 
	 * have changed.                           
	 */

	print_mess (ML ("Updating General Ledger Recovery Code Allocations."));

	for (i = 0; i < tableSize; i++)
	{
		tab_get ("ItemList", buffer, EQUAL, i);

		status = buffer [62];

		sprintf (inmr_rec.co_no, "%2.2s", comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", buffer + 1);
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		strcpy (glra_rec.co_no, comm_rec.co_no);
		strcpy (glra_rec.code, glrc_rec.code);
		glra_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (glra, &glra_rec, EQUAL, "r");

		if (!cc && status != 'A')
		{
			cc = abc_delete (glra);
			if (cc)
				file_err (cc, glra, "DELETE");
		}
		else if (cc && status == 'A')
		{
			sprintf (glra_rec.co_no, "%2.2s", comm_rec.co_no);
			sprintf (glra_rec.code, "%-5.5s", glrc_rec.code);
			glra_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = abc_add (glra, &glra_rec);
			if (cc)
				file_err (cc, glra, "DBADD");
		}
	}
}

