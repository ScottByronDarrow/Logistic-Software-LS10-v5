/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_mimaint.c,v 5.6 2002/07/08 05:30:25 scott Exp $
|  Program Name  : (pc_mimaint.c & pc_hwmaint.c & pc_idmaint.c)
|  Program Desc  : (Master Instructions, Hazard Warnings &)   
|                 (Detail Instructions)    
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 09/06/87          |
|---------------------------------------------------------------------|
| $Log: pc_mimaint.c,v $
| Revision 5.6  2002/07/08 05:30:25  scott
| S/C 004075 - After changing the text for one instruction, then save, the <Instruction Description> changes and reflect on all Instruction.
|
| Revision 5.5  2002/03/07 06:31:10  scott
| ..
|
| Revision 5.4  2002/03/07 05:02:29  scott
| Updated to always display instruction with last version no.
|
| Revision 5.3  2002/03/06 08:29:05  scott
| S/C 00832 - RTGMR7-Instruction Detail Maintainance; after changing the text for one instruction, then save, the changes does not reflect in the search window of field Instruction, but when selected, the previous
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_mimaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_mimaint/pc_mimaint.c,v 5.6 2002/07/08 05:30:25 scott Exp $";

#define	TXT_REQD
#ifdef MAXSCNS
#undef MAXSCNS
#endif
#define	MAXSCNS		6
#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>

#define	HEAD_SCN1	1
#define	HEAD_SCN2	2
#define	MAST_SCN	3
#define	INST_SCN	4
#define	WARN_SCN	5
#define	SHOW_SCN	6
int	head_scn = 1;

#include	"schema"

struct commRecord	comm_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct pchcRecord	pchc_rec;
struct pchwRecord	pchw_rec;
struct pcidRecord	pcid_rec;
struct pcmiRecord	pcmi_rec;
struct pcwcRecord	pcwc_rec;


	char	*pcwc2	= "pcwc2";

	int	run_prog = MAST_SCN;
	int	new_item = 0;
	int	abort_read;
	int	version;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	item_no [17];
	char	show_line [61];
	char	text_line [61];
	char	warn_line [81];
	char	desc [36];
	char	strength [6];
	int	instr_no;
	char	ins_name [9];
	char	inst_sel [9];
	char	wrk_cntr [9];
	long	hhwc_hash;
} local_rec;

static	int	ShowFunc	(int, KEY_TAB *);
static	int	SelFunc	 	(int, KEY_TAB *);
static	int	ExitFunc	(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB inst_keys [] =
{
   { " SHOW LINES ",	'S', ShowFunc,
	"Show Instruction Lines.",					"A" },
   { NULL,		'\r', SelFunc,
	"Select Instruction To Be Read In.",				"A" },
   { NULL,		FN1, ExitFunc,
	"Selection of users complete.",					"A" },
   { NULL,		FN16, ExitFunc,
	"Selection of users complete.",					"A" },
   END_KEYS
};
#else
static	KEY_TAB inst_keys [] =
{
   { " [S]HOW LINES",	'S', ShowFunc,
	"Show Instruction Lines.",					"A" },
   { NULL,		'\r', SelFunc,
	"Select Instruction To Be Read In.",				"A" },
   { NULL,		FN1, ExitFunc,
	"Selection of users complete.",					"A" },
   { NULL,		FN16, ExitFunc,
	"Selection of users complete.",					"A" },
   END_KEYS
};
#endif

extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "itemNo",	 2, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number        ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc1",	 3, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description #1     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "strength",	 3, 60, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "Strength  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.strength},
	{1, LIN, "haz",		 2, 60, CHARTYPE,
		"AAAA", "          ",
		" ", "", "Hazard Class       ", " ",
		 ND, NO,  JUSTLEFT, "", "", pchc_rec.pchc_class},
	{1, LIN, "haz_desc",	 2, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", pchc_rec.desc},
	{1, LIN, "desc2",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description #2     ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description2},
	{1, LIN, "workCenter",	 6, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Work Centre        ", " ",
		 YES, NO, JUSTLEFT, "", "", local_rec.wrk_cntr},
	{1, LIN, "instr_no",	 6, 35, INTTYPE,
		"NN", "          ",
		" ", "", "Instruction #      ", " ",
		 ND, NO, JUSTRIGHT, "1", "99", (char *)&local_rec.instr_no},
	{2, LIN, "ins_nam",	 3, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Instruction Name   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.ins_name},
	{3, TXT, "text_line",	 6, 8, 0,
		"", "          ",
		" ", " ", "                       T E X T                            ", " ",
		 13, 60, 50, "", "", local_rec.text_line},
	{4, TXT, "inst_scn",	 9, 7, 0,
		"", "          ",
		" ", " ", "                       T E X T                            ", " ",
		 10, 60, 50, "", "", local_rec.text_line},
	{5, TXT, "text_line",	 9, 24, 0,
		"", "          ",
		" ", " ", "                                 T E X T                                      ", " ",
		 10, 80, 50, "", "", local_rec.warn_line},
	{6, TXT, "show_inst",	 9, 1, 0,
		"", "          ",
		" ", " ", "       I N S T R U C T I O N   L I N E S                  ", " ",
		 5, 60, 5, "", "", local_rec.show_line},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
#include    <tabdisp.h>
/*
 * function prototypes 
 */
int 	GetVersionNo 		(void);
int 	heading 			(int);
int 	PrintOther 			(void);
int 	ReadIneiRecord 		(void);
int 	ReadInstructions 	(void);
int 	spec_valid 			(int);
int 	UpdatePcid 			(void);
void 	CloseDB 			(void);
void 	LoadPchw 			(void);
void 	LoadPcid 			(void);
void 	LoadPcmi 			(void);
void 	OpenDB 				(void);
void 	shutdown_prog 		(void);
void 	SrchPcid 			(char *);
void 	SrchPcmi 			(char *);
void 	SrchPcwc 			(char *);
void 	UpdatePchw 			(void);
void 	UpdatePcmi 			(void);

/*
 * Main Processing Routine.
 */
int
main (
 int  argc, 
 char *argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	head_scn = HEAD_SCN1;

	if (!strcmp (sptr, "pc_mimaint"))
	{
		head_scn = HEAD_SCN2;
		run_prog = MAST_SCN;
	}

	if (!strcmp (sptr, "pc_idmaint"))
		run_prog = INST_SCN;

	if (!strcmp (sptr, "pc_hwmaint"))
		run_prog = WARN_SCN;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	if (run_prog == WARN_SCN)
	{
		FLD ("haz") 		= NA;
		FLD ("haz_desc") 	= NA;
		FLD ("workCenter") 	= ND;
	}

	tab_row = 9;
	tab_col = 8;
	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();
	
	if (run_prog == WARN_SCN)
		swide ();

	if (run_prog == INST_SCN)
		FLD ("instr_no") = YES;
	else
		FLD ("instr_no") = ND;

	/*
	 * Beginning of input control loop 
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		lcount [run_prog] = 0;
		init_vars (run_prog);

		switch (run_prog)
		{
			case	MAST_SCN:
				no_edit (WARN_SCN);
				no_edit (INST_SCN);
				break;

			case	WARN_SCN:
				no_edit (MAST_SCN);
				no_edit (INST_SCN);
				break;

			case	INST_SCN:
				no_edit (MAST_SCN);
				no_edit (WARN_SCN);
				break;
		}

		/*
		 * Enter screen 1 linear input 
		 */
		init_vars (head_scn);
		heading (head_scn);
		entry (head_scn);
		if (restart || prog_exit)
			continue;

		scn_write (head_scn);
		scn_display (head_scn);
		scn_display (run_prog);

		if (new_item)
			entry (run_prog);
		else
			edit (run_prog);

		if (restart)
			continue;

		switch (run_prog)
		{
			case	MAST_SCN:
				UpdatePcmi ();
				break;

			case	WARN_SCN:
				UpdatePchw ();
				break;

			case	INST_SCN:
				UpdatePcid ();
				break;
		}
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
	abc_alias (pcwc2, pcwc);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pchc, pchc_list, PCHC_NO_FIELDS, "pchc_id_no");
	open_rec (pchw, pchw_list, PCHW_NO_FIELDS, "pchw_id_no");
	open_rec (pcid, pcid_list, PCID_NO_FIELDS, "pcid_id_no");
	open_rec (pcmi, pcmi_list, PCMI_NO_FIELDS, "pcmi_id_no");

	open_rec (pcwc, pcwc_list, PCWC_NO_FIELDS, "pcwc_id_no");
	open_rec (pcwc2, pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (pchc);
	abc_fclose (pchw);
	abc_fclose (pcid);
	abc_fclose (pcmi);
	SearchFindClose ();
	abc_dbclose ("data");
}
int
spec_valid (
 int field)
{
	if (LCHECK ("inst_scn"))
	{
		if ((run_prog == INST_SCN) && SRCH_KEY)
		{
			return (ReadInstructions ());
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("itemNo"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		ReadIneiRecord ();

		if (run_prog == MAST_SCN || run_prog == WARN_SCN)
			PrintOther ();

		if (run_prog == WARN_SCN)
		{
			strcpy (pchw_rec.co_no, comm_rec.co_no);
			pchw_rec.hhbr_hash = inmr_rec.hhbr_hash;
			pchw_rec.line_no = 0;
			cc = find_rec (pchw, &pchw_rec, GTEQ, "r");
			if (cc || strcmp (pchw_rec.co_no, comm_rec.co_no) ||
				pchw_rec.hhbr_hash != inmr_rec.hhbr_hash)
				new_item = TRUE;
			else
			{
				new_item = FALSE;
				LoadPchw ();
			}
		}

		DSP_FLD ("itemNo");
		sprintf (local_rec.desc, "%-35.35s", inmr_rec.description);
		sprintf (local_rec.strength, 
			"%-5.5s", 
			&inmr_rec.description [35]);
		DSP_FLD ("desc1");
		DSP_FLD ("strength");
		DSP_FLD ("desc2");

		if (run_prog == WARN_SCN)
		{
			DSP_FLD ("haz");
			DSP_FLD ("haz_desc");
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("workCenter"))
	{
		if (run_prog == INST_SCN)
		{
			if (SRCH_KEY)
			{
				SrchPcwc (temp_str);
				return (EXIT_SUCCESS);
			}

			strcpy (pcwc_rec.co_no, comm_rec.co_no);
			strcpy (pcwc_rec.br_no, comm_rec.est_no);
			sprintf (pcwc_rec.work_cntr, "%-8.8s", local_rec.wrk_cntr);
			cc = find_rec (pcwc, &pcwc_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlPcMess105));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			local_rec.hhwc_hash = pcwc_rec.hhwc_hash;

			DSP_FLD ("workCenter");
		}

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("instr_no"))
	{
		if (SRCH_KEY)
		{
			SrchPcid (temp_str);
			return (EXIT_SUCCESS);
		}

		version = GetVersionNo ();
		if (version == 0)
			new_item = TRUE;
		else
		{
			new_item = FALSE;
			LoadPcid ();
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ins_nam"))
	{
		if (end_input && prog_status == ENTRY)
			prog_exit = TRUE;

		if (run_prog == MAST_SCN)
		{
			if (SRCH_KEY)
			{
				SrchPcmi (temp_str);
				return (EXIT_SUCCESS);
			}

			strcpy (pcmi_rec.co_no, comm_rec.co_no);
			strcpy (pcmi_rec.inst_name, local_rec.ins_name);
			pcmi_rec.line_no = 0;
			cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
			if (cc ||
			     strcmp (pcmi_rec.co_no, comm_rec.co_no) ||
			     strcmp (pcmi_rec.inst_name, local_rec.ins_name))
			{
				new_item = TRUE;
			}
			else
			{
				new_item = FALSE;
				LoadPcmi ();
			}
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 * Get a master instruction and 'suck' into TXT screen.
 */
int
ReadInstructions (void)
{
	int	mast_fnd;

	abort_read = FALSE;

	/*
	 * Show all master instructions
	 */
	mast_fnd = FALSE;
	tab_open ("mst_inst", inst_keys, 1, 0, 3, FALSE);
	tab_add ("mst_inst", "#   Instruction    ");

	strcpy (pcmi_rec.co_no, comm_rec.co_no);
	sprintf (pcmi_rec.inst_name, "%-8.8s", " ");
	pcmi_rec.line_no = 0;
	cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	while (!cc && !strcmp (pcmi_rec.co_no, comm_rec.co_no))
	{
		if (pcmi_rec.line_no == 0)
		{
			tab_add ("mst_inst", "     %-8.8s     ", pcmi_rec.inst_name);

			mast_fnd = TRUE;
		}

		cc = find_rec (pcmi, &pcmi_rec, NEXT, "r");
	}

	if (mast_fnd)
	{
		tab_scan ("mst_inst");
		tab_close ("mst_inst", TRUE);
	}
	else
	{
		tab_add ("mst_inst", "    No Master     ");
		tab_add ("mst_inst", "   Instructions   ");
		tab_display ("mst_inst", TRUE);
		sleep (sleepTime);
		tab_close ("mst_inst", TRUE);

		return (EXIT_FAILURE);
	}

	heading (HEAD_SCN1);
	scn_display (1);

	scn_set (INST_SCN);

	if (abort_read)
		return (EXIT_FAILURE);

	strcpy (pcmi_rec.co_no, comm_rec.co_no);
	sprintf (pcmi_rec.inst_name, "%-8.8s", local_rec.inst_sel);
	pcmi_rec.line_no = 0;
	cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcmi_rec.co_no, comm_rec.co_no) && 
	       !strcmp (pcmi_rec.inst_name, local_rec.inst_sel))
	{
		sprintf (local_rec.text_line, "%-60.60s", pcmi_rec.text);

		putval (-1);

		cc = find_rec (pcmi, &pcmi_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

static int	
ShowFunc (
 int     c, 
 KEY_TAB *psUnused)
{
	char	inst_buf [200];
	int	no_lines;

	tab_get ("mst_inst", inst_buf, CURRENT, 0);

	lcount [SHOW_SCN] = 0;
	init_vars (SHOW_SCN);

	no_lines = 0;

	strcpy (pcmi_rec.co_no, comm_rec.co_no);
	sprintf (pcmi_rec.inst_name, "%-8.8s", &inst_buf [5]);
	pcmi_rec.line_no = 0;
	cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcmi_rec.co_no, comm_rec.co_no) && 
	       !strncmp (pcmi_rec.inst_name, &inst_buf [5], 8))
	{
		sprintf (local_rec.show_line, "%-60.60s", pcmi_rec.text);
		putval (lcount [SHOW_SCN]++);

		if (++no_lines >= 5)
			break;

		cc = find_rec (pcmi, &pcmi_rec, NEXT, "r");
	}

	scn_display (SHOW_SCN);
	crsr_off ();

	return (c);
}

static int	
SelFunc (
 int     c, 
 KEY_TAB *psUnused)
{
	char	inst_buf [200];

	tab_get ("mst_inst", inst_buf, CURRENT, 0);
	sprintf (local_rec.inst_sel, "%-8.8s", &inst_buf [5]);

	return (FN16);
}

static int	
ExitFunc (
 int     c, 
 KEY_TAB *psUnused)
{
	abort_read = TRUE;
	return (FN16);
}

void
SrchPcwc (
 char *keyValue)
{
	_work_open (8,0,40);
	save_rec ("#No", "#Work Centre Description");
	strcpy (pcwc_rec.co_no, comm_rec.co_no);
	strcpy (pcwc_rec.br_no, comm_rec.est_no);
	sprintf (pcwc_rec.work_cntr, "%-8.8s", keyValue);
	cc = find_rec (pcwc, &pcwc_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcwc_rec.co_no, comm_rec.co_no) &&
	       !strcmp (pcwc_rec.br_no, comm_rec.est_no) &&
	       !strncmp (pcwc_rec.work_cntr, keyValue, strlen (keyValue)))
	{
		cc = save_rec (pcwc_rec.work_cntr, pcwc_rec.name);
		if (cc)
			break;

		cc = find_rec (pcwc, &pcwc_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pcwc_rec.co_no, comm_rec.co_no);
	strcpy (pcwc_rec.br_no, comm_rec.est_no);
	sprintf (pcwc_rec.work_cntr, "%-8.8s", temp_str);
	cc = find_rec (pcwc, &pcwc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pcwc, "DBFIND");
}

int
ReadIneiRecord (void)
{
	inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inei_rec.est_no, comm_rec.est_no);
	cc = find_rec (inei, &inei_rec, COMPARISON, "r");
	if (cc)
	{
		inei_rec.std_batch = 0.00;
		inei_rec.min_batch = 0.00;
		inei_rec.max_batch = 0.00;
		sprintf (pchc_rec.pchc_class, "%-4.4s", " ");
		sprintf (pchc_rec.desc, "%-40.40s", " ");
	}
	else
	{
		strcpy (pchc_rec.co_no, comm_rec.co_no);
		strcpy (pchc_rec.type, "Z");
		sprintf (pchc_rec.pchc_class, "%-4.4s", inei_rec.hzrd_class);
		cc = find_rec (pchc, &pchc_rec, COMPARISON, "r");
	}

	return (EXIT_SUCCESS);
}

void
LoadPcmi (void)
{
	init_vars (MAST_SCN);

	lcount [MAST_SCN] = 0;

	strcpy (pcmi_rec.co_no, comm_rec.co_no);
	strcpy (pcmi_rec.inst_name, local_rec.ins_name);
	pcmi_rec.line_no = 0;
	cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (pcmi_rec.co_no, comm_rec.co_no) &&
		!strcmp (pcmi_rec.inst_name, local_rec.ins_name)
	)
	{
		strcpy (local_rec.text_line, pcmi_rec.text);
		putval (lcount [MAST_SCN]++);
		cc = find_rec (pcmi, &pcmi_rec, NEXT, "r");
	}
	scn_set (head_scn);
}

void
LoadPchw (void)
{
	init_vars (WARN_SCN);

	lcount [WARN_SCN] = 0;
	strcpy (pchw_rec.co_no, comm_rec.co_no);
	pchw_rec.hhbr_hash = inmr_rec.hhbr_hash;
	pchw_rec.line_no = 0;
	cc = find_rec (pchw, &pchw_rec, GTEQ, "r");
	while (!cc && !strcmp (pchw_rec.co_no, comm_rec.co_no) &&
			pchw_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		strcpy (local_rec.warn_line, pchw_rec.text);
		putval (lcount [WARN_SCN]++);
		cc = find_rec (pchw, &pchw_rec, NEXT, "r");
	}
	scn_set (head_scn);
}

void
LoadPcid (void)
{
	init_vars (INST_SCN);

	lcount [INST_SCN] = 0;
	strcpy (pcid_rec.co_no, comm_rec.co_no);
	pcid_rec.hhbr_hash = inmr_rec.hhbr_hash;
	pcid_rec.hhwc_hash = local_rec.hhwc_hash;
	pcid_rec.instr_no = local_rec.instr_no;
	pcid_rec.version = version;
	pcid_rec.line_no = 0;
	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	while (!cc && !strcmp (pcid_rec.co_no, comm_rec.co_no) &&
			pcid_rec.hhbr_hash == inmr_rec.hhbr_hash &&
			pcid_rec.hhwc_hash == local_rec.hhwc_hash &&
			pcid_rec.instr_no == local_rec.instr_no &&
			pcid_rec.version == version)
	{
		strcpy (local_rec.text_line, pcid_rec.text);
		putval (lcount [INST_SCN]++);
		cc = find_rec (pcid, &pcid_rec, NEXT, "r");
	}
	scn_set (head_scn);
}

int
UpdatePcid (void)
{
	int	i;
	int	nxt_version;
	
	scn_set (INST_SCN);

	/*
	 * Get new version number
	 */
	nxt_version = version + 1;

	for (i = 0;i < lcount [INST_SCN];i++)
	{
		getval (i);

		strcpy (pcid_rec.co_no, comm_rec.co_no);
		pcid_rec.hhbr_hash = inmr_rec.hhbr_hash;
		pcid_rec.hhwc_hash = local_rec.hhwc_hash;
		pcid_rec.instr_no = local_rec.instr_no;
		pcid_rec.version = nxt_version;
		pcid_rec.line_no = i;
		strcpy (pcid_rec.text, local_rec.text_line);
		cc = abc_add (pcid, &pcid_rec);
		if (cc)
			file_err (cc, pcid, "DBADD");
	}

	return (EXIT_SUCCESS);
}

/*
 * Get latest version number
 */
int
GetVersionNo (void)
{
	int	ver_no;

	ver_no = 0;

	strcpy (pcid_rec.co_no, comm_rec.co_no);
	pcid_rec.hhbr_hash = inmr_rec.hhbr_hash;
	pcid_rec.hhwc_hash = local_rec.hhwc_hash;
	pcid_rec.instr_no = local_rec.instr_no;
	pcid_rec.line_no = 0;
	pcid_rec.version = 1;
	cc = find_rec (pcid, &pcid_rec, GTEQ, "u");
	while (!cc && !strcmp (pcid_rec.co_no, comm_rec.co_no) &&
	       pcid_rec.hhbr_hash == inmr_rec.hhbr_hash &&
	       pcid_rec.hhwc_hash == local_rec.hhwc_hash &&
	       pcid_rec.instr_no == local_rec.instr_no)
	{
		ver_no = pcid_rec.version;
		cc = find_rec (pcid, &pcid_rec, NEXT, "u");
	}

	return (ver_no);
}

void
UpdatePcmi (void)
{
	clear ();

	print_at (0,0, ML (mlPcMess030));

	fflush (stdout);

	scn_set (MAST_SCN);

	for (line_cnt = 0;line_cnt < lcount [MAST_SCN];line_cnt++)
	{
		getval (line_cnt);

		strcpy (pcmi_rec.co_no, comm_rec.co_no);
		strcpy (pcmi_rec.inst_name, local_rec.ins_name);
		pcmi_rec.line_no = line_cnt;
		cc = find_rec (pcmi, &pcmi_rec, COMPARISON, "u");
		if (cc)
		{
			strcpy (pcmi_rec.co_no, comm_rec.co_no);
			strcpy (pcmi_rec.inst_name, local_rec.ins_name);
			pcmi_rec.line_no = line_cnt;
			strcpy (pcmi_rec.text, local_rec.text_line);
			cc = abc_add (pcmi, &pcmi_rec);
			if (cc)
				file_err (cc, pcmi, "DBADD");
		}
		else
		{
			strcpy (pcmi_rec.text, local_rec.text_line);
			cc = abc_update (pcmi, &pcmi_rec);
			if (cc)
				file_err (cc, pcmi, "DBUPDATE");
		}
		abc_unlock (pcmi);
	}

	strcpy (pcmi_rec.co_no, comm_rec.co_no);
	strcpy (pcmi_rec.inst_name, local_rec.ins_name);
	pcmi_rec.line_no = lcount [MAST_SCN];
	cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (pcmi_rec.co_no, comm_rec.co_no) &&
		!strcmp (pcmi_rec.inst_name, local_rec.ins_name)
	)
	{
		cc = abc_delete (pcmi);
		if (cc)
			file_err (cc, pcmi, "DBDELETE");

		strcpy (pcmi_rec.co_no, comm_rec.co_no);
		strcpy (pcmi_rec.inst_name, local_rec.ins_name);
		pcmi_rec.line_no = lcount [MAST_SCN];
		cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	}
}

void
UpdatePchw (void)
{
	clear ();

	print_at (0,0, ML (mlPcMess031));

	fflush (stdout);

	scn_set (WARN_SCN);

	for (line_cnt = 0;line_cnt < lcount [WARN_SCN];line_cnt++)
	{
		getval (line_cnt);

		strcpy (pchw_rec.co_no, comm_rec.co_no);
		pchw_rec.hhbr_hash = inmr_rec.hhbr_hash;
		pchw_rec.line_no = line_cnt;
		cc = find_rec (pchw, &pchw_rec, COMPARISON, "u");
		/*
		 * new hazard warning		
		 */
		if (cc)
		{
			strcpy (pchw_rec.co_no, comm_rec.co_no);
			pchw_rec.hhbr_hash = inmr_rec.hhbr_hash;
			pchw_rec.line_no = line_cnt;
			strcpy (pchw_rec.text, local_rec.warn_line);
			cc = abc_add (pchw, &pchw_rec);
			if (cc)
				file_err (cc, pchw, "DBADD");
		}
		else
		{
			strcpy (pchw_rec.text, local_rec.warn_line);
			cc = abc_update (pchw, &pchw_rec);
			if (cc)
				file_err (cc, pchw, "DBUPDATE");
		}
		abc_unlock (pchw);
	}

	strcpy (pchw_rec.co_no, comm_rec.co_no);
	pchw_rec.hhbr_hash = inmr_rec.hhbr_hash;
	pchw_rec.line_no = lcount [WARN_SCN];
	cc = find_rec (pchw, &pchw_rec, GTEQ, "r");
	while (!cc && !strcmp (pchw_rec.co_no, comm_rec.co_no) &&
			pchw_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		cc = abc_delete (pchw);
		if (cc)
			file_err (cc, pchw, "DBDELETE");

		strcpy (pchw_rec.co_no, comm_rec.co_no);
		pchw_rec.hhbr_hash = inmr_rec.hhbr_hash;
		pchw_rec.line_no = lcount [WARN_SCN];
		cc = find_rec (pchw, &pchw_rec, GTEQ, "r");
	}
}

int
PrintOther (void)
{
	cc = find_hash (inum, &inum_rec, COMPARISON, "r", inmr_rec.std_uom);
	if (cc)
		sprintf (inum_rec.uom, "%-4.4s", " ");

	sprintf (err_str, ML (mlPcMess032) , inum_rec.uom);
	rv_pr (err_str, 2, 6, 0);

	cc = find_hash (inum, &inum_rec, COMPARISON, "r", inmr_rec.alt_uom);
	if (cc)
		sprintf (inum_rec.uom, "%-4.4s", " ");

	sprintf (err_str, ML (mlPcMess032) , inum_rec.uom);
	rv_pr (err_str, 23, 6, 0);

	sprintf (err_str, ML (mlPcMess033) , inmr_rec.uom_cfactor);
	rv_pr (err_str, 43, 6, 0);

	sprintf (err_str, ML (mlPcMess034) , inei_rec.std_batch);
	rv_pr (err_str, 2, 7, 0);

	sprintf (err_str, ML (mlPcMess035) , inei_rec.min_batch);
	rv_pr (err_str, 23, 7, 0);

	sprintf (err_str, ML (mlPcMess036) , inei_rec.max_batch);
	rv_pr (err_str, 43, 7, 0);

	return (EXIT_SUCCESS);
}

void
SrchPcmi (
 char *keyValue)
{
	_work_open (8,0,60);
	save_rec ("#No", "#Description");
	strcpy (pcmi_rec.co_no, comm_rec.co_no);
	sprintf (pcmi_rec.inst_name, "%-8.8s", keyValue);
	pcmi_rec.line_no = 0;
	cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcmi_rec.co_no, comm_rec.co_no) &&
	       !strncmp (pcmi_rec.inst_name, keyValue, strlen (keyValue)))
	{
		if (pcmi_rec.line_no == 0)
		{
			cc = save_rec (pcmi_rec.inst_name, pcmi_rec.text);
			if (cc)
				break;
		}
		cc = find_rec (pcmi, &pcmi_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}

void
SrchPcid (
 char *keyValue)
{
	int		firstTime	=	TRUE;
	int		lastInstr	=	0;
	char	lastText	[61];

	_work_open (2,0,40);
	save_rec ("#No", "#Instruction Description");
	strcpy (pcid_rec.co_no, comm_rec.co_no);
	pcid_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	pcid_rec.hhwc_hash 	= pcwc_rec.hhwc_hash;
	pcid_rec.instr_no 	= 1;
	pcid_rec.version 	= 1;
	pcid_rec.line_no 	= 0;
	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcid_rec.co_no, comm_rec.co_no) &&
	       pcid_rec.hhbr_hash == inmr_rec.hhbr_hash &&
	       pcid_rec.hhwc_hash == pcwc_rec.hhwc_hash)
	{
		if (pcid_rec.line_no > 0)
		{
			cc = find_rec (pcid, &pcid_rec, NEXT, "r");
			continue;
		}
		if (firstTime)
		{
			lastInstr = pcid_rec.instr_no;
			strcpy (lastText, pcid_rec.text);
			firstTime = FALSE;
		}
		if (pcid_rec.instr_no != lastInstr)
		{
			sprintf (err_str, "%2d", lastInstr);
			cc = save_rec (err_str, lastText);
			if (cc)
				break;
		}
		lastInstr = pcid_rec.instr_no;
		strcpy (lastText, pcid_rec.text);
		cc = find_rec (pcid, &pcid_rec, NEXT, "r");
	}
	if (firstTime == FALSE)
	{
		sprintf (err_str, "%2d", lastInstr);
		save_rec (err_str, lastText);
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	pcid_rec.instr_no = atoi (temp_str);
}

int
heading (
 int scn)
{
	int	page_len;

	if (restart)
		return (EXIT_SUCCESS);

	page_len = (run_prog == WARN_SCN) ? 130 : 80;

	switch (run_prog)
	{
		case	MAST_SCN:
			clear ();
			strcpy (err_str, ML (mlPcMess037));
			box (0, 2, page_len, 1);
			line_at (1,0,80);
			break;

		case	WARN_SCN:
			swide ();
			clear ();
			strcpy (err_str, ML (mlPcMess038));
			box (0, 1, page_len, 6);
			break;

		case	INST_SCN:
			clear ();
			strcpy (err_str, ML (mlPcMess039));
			box (0, 1, page_len, 5);
			break;
	}

	rv_pr (err_str, (page_len - strlen (err_str)) / 2, 0, 1);


	if (scn == 1)
	{
		scn_set (run_prog);
		scn_write (run_prog);
		scn_display (run_prog);
	}
	else
	{
		scn_set (head_scn);
		scn_write (head_scn);
		scn_display (head_scn);
	}
	scn_set (scn);
	line_at (21,0,page_len);

	print_at (22,0, ML (mlStdMess038) , comm_rec.co_no, comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_FAILURE);
}
