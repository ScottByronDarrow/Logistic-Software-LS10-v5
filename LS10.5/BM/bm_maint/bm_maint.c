/*=====================================================================
|  Copyright (C) 1999 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: bm_maint.c,v 5.7 2002/11/28 01:36:16 scott Exp $
|  Program Name  : (bm_maint.c   )                                    |
|  Program Desc  : (BOM Product Specification Maintenance.      )     |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written : 19/06/87          |
|---------------------------------------------------------------------|
| $Log: bm_maint.c,v $
| Revision 5.7  2002/11/28 01:36:16  scott
| .
|
| Revision 5.6  2002/10/17 04:22:42  scott
| Updated to clean code on standard before modifications.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bm_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_maint/bm_maint.c,v 5.7 2002/11/28 01:36:16 scott Exp $";

#define	TOTSCNS		2
#define MAXLINES	100
#define MAXWIDTH	190
#define	TXT_REQD
#include <pslscr.h>
#include <twodec.h>
#include <number.h>
#include <ml_std_mess.h>
#include <ml_bm_mess.h>

extern	int	_win_func;

#include	"schema"

struct commRecord	comm_rec;
struct bminRecord	bmin_rec;
struct bmmsRecord	bmms_rec;
struct bmmsRecord	bmms2_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;

	char	*bmms2	= "bmms2",
			*data	= "data",
			*inum2	= "inum2";

   	int  	newItem = 0;

	struct 	storeRec {
		long	hhbrHash;
		char	source [3];
	} store [MAXLINES];

struct	INS_PTR
{
	char	text [41];
	struct	INS_PTR	*next;
} *ins_ptr [MAXLINES];

#define	INS_NUL	 ((struct INS_PTR *) NULL)
struct	INS_PTR	*free_head = INS_NUL;

/*
 * Local & Screen Structures. 
 */
struct
{
	char	alt_group 		[21];
	char	alt_uom 		[5];
	char	desc2 			[41];
	char	desc 			[36];
	char	dflt_qty 		[15];
	char	dummy 			[11];
	char	ins_text 		[41];
	char	instr 			[4];
	char	itemNumber 		[17];
	char	matl_cons 		[2];
	char	matl_desc 		[41];
	char	matl_no 		[17];
	char	matl_src 		[3];
	char	matl_std_uom 	[5];
	char	matl_uom 		[5];
	char	matl_work_cntr 	[9];
	char	scrap_pc 		[10];
	char	std_group 		[21];
	char	std_uom 		[5];
	char	strength 		[6];
	double	matl_qty;
	float	cnv_fct;				/* material item conversion factor */
	float	matl_wst_pc;
	float	max_batch;
	float	min_batch;
	float	std_batch;
	float	uom_cfactor;			/* final item conversion factor */
	int		alt_no;
	int		dec_pt;					/* material item decimal places */
	int		instr_no;
	int		iss_seq;
	long	hhbrHash;
	long	malt_uom;				/* material item alt uom hand */
	long	mstd_uom;				/* material item std uom hand */
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "itemNumber",	 2, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{1, LIN, "alt_no",	 3, 18, INTTYPE,
		"NNNNN", "          ",
		" ", "0", "Alternate No : ", " ",
		YES, NO,  JUSTRIGHT, "0", "32767", (char *)&local_rec.alt_no},
	{1, LIN, "strength",	 4, 18, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "Strength", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.strength},
	{1, LIN, "desc",		 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "desc2",	 6, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc2},
	{1, LIN, "std_uom",	 3, 88, CHARTYPE,
		"AAAA", "          ",
		"", "", "Standard", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.std_uom},
	{1, LIN, "alt_uom",	 3, 110, CHARTYPE,
		"AAAA", "          ",
		"", "", "  Alternate ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.alt_uom},

	{2, TAB, "matl_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "    Material.   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.matl_no},
	{2, TAB, "matl_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "          Material Description.         ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.matl_desc},
	{2, TAB, "std_uom_grp",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.std_group},
	{2, TAB, "alt_uom_grp",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.alt_group},
	{2, TAB, "std_uom",	 0, 0, LONGTYPE,
		"NNNNNNNNNNN", "          ",
		"", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.mstd_uom},
	{2, TAB, "alt_uom",	 0, 0, LONGTYPE,
		"NNNNNNNNNNN", "          ",
		"", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.malt_uom},
	{2, TAB, "cnv_fct",	 0, 0, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.cnv_fct},
	{2, TAB, "dec_pt",	 0, 0, INTTYPE,
		"N", "          ",
		"", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.dec_pt},
	{2, TAB, "uom_cfactor",	 0, 0, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.uom_cfactor},
	{2, TAB, "matl_uom",	 0, 1, CHARTYPE,
		"AAAA", "          ",
		" ", "", " UOM ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.matl_uom},
	{2, TAB, "matl_qty",	 0, 0, DOUBLETYPE,
		local_rec.dflt_qty, "          ",
		" ", "1", "   Quantity   ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.matl_qty},
	{2, TAB, "matl_wst_pc",	 0, 3, FLOATTYPE,
		"NN.NN", "          ",
		" ", local_rec.scrap_pc, " % Waste ", " Percentage of Waste ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.matl_wst_pc},
	{2, TAB, "instr_no",	 0, 7, INTTYPE,
		"NN", "          ",
		" ", "0", " Instruction No ", " Instruction Number ",
		 ND, NO, JUSTRIGHT, "0", "99", (char *)&local_rec.instr_no},
	{2, TAB, "matl_src",	 0, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Source", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.matl_src},
	{2, TAB, "iss_seq",	 0, 1, INTTYPE,
		"NNNNN", "          ",
		" ", "", "Iss Seq", "Routing Sequence No. before which stock MUST be issued (0 = Pre-release)",
		 NO, NO, JUSTRIGHT, "0", "32767", (char *) &local_rec.iss_seq},
	{2, TAB, "matl_cons",	 0, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Cons.", "",
		 NA, NO, JUSTLEFT, "YN", "", local_rec.matl_cons},
	{2, TAB, "instr",	 0, 1, CHARTYPE,
		"AAA", "          ",
		" ", "", "Inst.", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.instr},
	{3, TXT, "instr_text",	 8, 60, 0,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "   INDIVIDUAL MATERIAL INSTRUCTION (S)  ", "",
		10, 40, 100, "", "", local_rec.ins_text},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
/*
 * Local Function Prototypes.
 */
struct 	INS_PTR *ins_alloc 	 (void);
void 	shutdown_prog 		 (void);
void 	tab_other 			 (int);
void 	AlternateSearch 	 (char *);
void 	CloseDB 			 (void);
void 	LoadSpec 			 (void);
void 	InitPointers 		 (void);
void 	OpenDB 				 (void);
void 	SetupHhbrHash 		 (void);
void 	SrchInum 			 (char *);
void 	Update 				 (void);
int 	win_function 		 (int, int, int, int);
int 	spec_valid 			 (int);
int 	DeleteLine 			 (void);
int 	CalcConv 			 (void);
int 	ValidUOM 			 (void);
int 	heading 			 (int);
int 	ValidQuantity 		 (double, int);

extern	int	manufacturingSrch;

/*
 * Main Processing Routine 
 */
int
main (
	int		argc,
	char	*argv [])
{
	long	hhbrHash;
	char 	*sptr;

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dflt_qty, sptr);

	SETUP_SCR (vars);


	_win_func = TRUE;
	for (cc = 0; cc < MAXLINES; cc++)
		ins_ptr [cc] = INS_NUL;

	init_scr 	();
	set_tty 	();
	set_masks 	();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars 	(1);

	OpenDB 		();
	read_comm 	(comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	tab_row = 8;
	tab_col = 7;

	swide ();
	/*
	 * Beginning of input control loop 
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags 
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		InitPointers ();

		init_vars (1);
		init_vars (2);
		init_vars (3);
		lcount [2] = 0;
		lcount [3] = 0;

		inei_rec.std_batch = 0.0;
		inei_rec.min_batch = 0.0;
		inei_rec.max_batch = 0.0;

		abc_unlock ("bmms");

		/*
		 * Enter screen 1 linear input 
		 */
		if (argc == 1)
		{
			heading (1);
			entry (1);
		}
		else
		{
			last_char = 0;
			abc_selfield (inmr, "inmr_hhbr_hash");
			hhbrHash 			= atol (argv [1]);
			local_rec.alt_no 	= atoi (argv [2]);
			inmr_rec.hhbr_hash	=	hhbrHash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (cc)
				break;

			heading (2);
			scn_set (1);
			strcpy (local_rec.itemNumber, inmr_rec.item_no);
			spec_valid (label ("itemNumber"));
			spec_valid (label ("alt_no"));
			scn_set (2);
		}

		if (restart || prog_exit)
			continue;

		if (newItem)
		{
			if (argc == 1)
				heading (2);
			entry (2);
			if (restart)
				continue;
		}
		if (argc == 1 || newItem)
			heading (2);

		scn_display (2);
		edit (2);
		if (restart)
			continue;

		Update ();
		if (argc != 1)
			prog_exit = TRUE;
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
	abc_dbopen (data);

 	abc_alias (bmms2, bmms);
 	abc_alias (inum2, inum);

	open_rec (bmin,  bmin_list, BMIN_NO_FIELDS, "bmin_id_no");
	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (bmms2, bmms_list, BMMS_NO_FIELDS, "bmms_id_no_2");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_uom");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (bmin);
	abc_fclose (bmms);
	abc_fclose (bmms2);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	SearchFindClose ();
	abc_dbclose (data);
}

void
InitPointers (void)
{
	struct	INS_PTR	*tempPointer = INS_NUL;
	int	i;

	for (i = 0; i < MAXLINES; i++)
	{
		while (ins_ptr [i] != INS_NUL)
		{
			tempPointer 		= ins_ptr [i]->next;
			ins_ptr [i]->next 	= free_head;
			free_head 			= ins_ptr [i];
			ins_ptr [i] 		= tempPointer;
		}
	}
}

int
win_function (
	int		fld,
	int		lin,
	int		scn,
	int		mode)
{
	struct	INS_PTR	*currPointer;
	int		i,
			tmp_line;

	if (mode == ENTRY)
	{
		print_mess (ML (mlBmMess039));
		sleep (sleepTime);
		clear_mess ();
		return (PSLW);
	}

	_win_func = FALSE;
	tmp_line = line_cnt;

	init_vars (3);
	lcount [3] = 0;

	currPointer = ins_ptr [lin];
	while (currPointer != INS_NUL)
	{
		strcpy (local_rec.ins_text, currPointer->text);
		putval (lcount [3]++);
		currPointer = currPointer->next;
	}

	heading (3);
	scn_display (3);
	edit (3);

	if (!restart)
	{
		/*
		 * Free up the prior contents of	the specific lines instr. 
		 */
		while (ins_ptr [lin] != INS_NUL)
		{
			currPointer = ins_ptr [lin]->next;
			ins_ptr [lin]->next = free_head;
			free_head = ins_ptr [lin];
			ins_ptr [lin] = currPointer;
		}

		/*
		 * Remove trailing blank lines.	
		 */
		while (lcount [3] > 0)
		{
			getval (lcount [3] - 1);
			if (strlen (clip (local_rec.ins_text)) == 0)
				lcount [3]--;
			else
				break;
		}

		/*
		 * Build linked list 
		 */
		for (i = lcount [3]; i > 0; i--)
		{
			getval (i - 1);

			currPointer = ins_alloc ();

			currPointer->next = ins_ptr [lin];
			strcpy (currPointer->text, local_rec.ins_text);
			ins_ptr [lin] = currPointer;
		}
	}

	line_cnt = tmp_line;
	tab_row = 8;
	tab_col = 7;
	scn_set (2);
	move (0, 22);
	cl_line ();
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,45, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

	/*
	 * Update by_prod flag on scn 2 
	 */
	getval (lin);
	strcpy (local_rec.instr, (lcount [3] == 0) ? "   " : "Yes");
	putval (lin);

	restart = FALSE;
	_win_func = TRUE;
	return (PSLW);
}

struct INS_PTR *
ins_alloc (void)
{
	struct INS_PTR *currPointer;

	/*
	 * Return pointer to a node in the free list if possible   
	 */
	if (free_head != INS_NUL)
	{
		currPointer = free_head;
		free_head = free_head->next;

		currPointer->next = INS_NUL;
		return (currPointer);
	}

	currPointer = (struct INS_PTR *) malloc (sizeof (struct INS_PTR));

	if (currPointer == INS_NUL)
		sys_err ("Error allocating memory for instruction list During (MALLOC)", errno, PNAME);

	currPointer->next = INS_NUL;

	return (currPointer);
}

int
spec_valid (
	int		field)
{
	int		ans;
        
	if (LCHECK ("itemNumber"))
	{
		if (SRCH_KEY)
		{
			manufacturingSrch 	=	TRUE;
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			manufacturingSrch 	=	FALSE;
			return (EXIT_SUCCESS);
		}
		strcpy (inmr_rec.co_no,comm_rec.co_no);

		abc_selfield (inmr, "inmr_id_no");

		cc = FindInmr (comm_rec.co_no, local_rec.itemNumber, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNumber);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if
		 (
			strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP")
		)
		{
			print_mess (ML (mlBmMess008));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		/*
		 * check item has a non-zero standard unit cost 
		 */
		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc || inei_rec.std_cost == 0.00)
		{
			sprintf (err_str,ML (mlBmMess040), clip (inmr_rec.item_no));
			ans = prmptmsg (err_str, "YyNn", 1, 23);
			move (1, 23);
			cl_line ();
			if (ans == 'N' || ans == 'n')
				return (EXIT_FAILURE);
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum,&inum_rec,EQUAL, "r");
		strcpy (local_rec.std_uom, (cc) ? "    " : inum_rec.uom);
		inum_rec.hhum_hash	=	inmr_rec.alt_uom;
		cc = find_rec (inum,&inum_rec,EQUAL, "r");
		strcpy (local_rec.alt_uom, (cc) ? "    " : inum_rec.uom);

		sprintf (local_rec.itemNumber,"%-16.16s",inmr_rec.item_no);
		DSP_FLD ("itemNumber");
		sprintf (local_rec.desc, "%-35.35s", inmr_rec.description);
		sprintf (local_rec.strength, "%-5.5s", inmr_rec.description + 35);
		sprintf (local_rec.desc2, "%-40.40s", inmr_rec.description2);
		DSP_FLD ("itemNumber");
		DSP_FLD ("strength");
		DSP_FLD ("desc");
		DSP_FLD ("desc2");
		DSP_FLD ("std_uom");
		DSP_FLD ("alt_uom");

		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc)
		{
			inei_rec.std_batch = 1;
			inei_rec.min_batch = 1;
			inei_rec.max_batch = 1;
		}
		local_rec.std_batch = inei_rec.std_batch;
		local_rec.min_batch = inei_rec.min_batch;
		local_rec.max_batch = inei_rec.max_batch;

		print_at (6,  69, ML (mlBmMess028), inei_rec.std_batch);
		print_at (6,  90, ML (mlBmMess029), inei_rec.min_batch);
		print_at (6, 111, ML (mlBmMess030), inei_rec.max_batch);

		local_rec.hhbrHash = inmr_rec.hhbr_hash;

        return (EXIT_SUCCESS);
	}

	if (LCHECK ("alt_no"))
	{
		if (SRCH_KEY)
		{
			AlternateSearch (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/*
			 * Read branch record for default bom no. 
			 */
			strcpy (inei_rec.est_no, comm_rec.est_no);
			inei_rec.hhbr_hash = local_rec.hhbrHash;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");

			/*
			 * If branch default is 0, read company default bom no. 
			 */
			if (cc || inei_rec.dflt_bom <= 0)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				inmr_rec.hhbr_hash	=	local_rec.hhbrHash;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, "inmr", "DBFIND");
				if (inmr_rec.dflt_bom <= 0)
				{
					print_mess (ML (mlStdMess007));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				local_rec.alt_no = inmr_rec.dflt_bom; 
				DSP_FLD ("alt_no");
				abc_selfield (inmr, "inmr_id_no");
			}
			else
			{
				local_rec.alt_no = inei_rec.dflt_bom;
				DSP_FLD ("alt_no");
			}
		}

		strcpy (bmms_rec.co_no, comm_rec.co_no);
		bmms_rec.hhbr_hash = inmr_rec.hhbr_hash;
		bmms_rec.alt_no = local_rec.alt_no;
		bmms_rec.line_no = 0;
		cc = find_rec (bmms, &bmms_rec, GTEQ, "r");	
		if (cc || 
		    strcmp (bmms_rec.co_no, comm_rec.co_no) || 
		    bmms_rec.hhbr_hash != inmr_rec.hhbr_hash ||
		    bmms_rec.alt_no != local_rec.alt_no)
		{
			newItem = TRUE;
			SetupHhbrHash ();
		}
		else
		{
			newItem = FALSE;
			LoadSpec ();
		}

		return (EXIT_SUCCESS);
	}

    if (LCHECK ("matl_no"))
    {
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
			return (DeleteLine ());

		abc_selfield (inmr, "inmr_id_no");


		cc = FindInmr (comm_rec.co_no, local_rec.matl_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.matl_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (inmr_rec.hhbr_hash == local_rec.hhbrHash)
		{
			print_mess (ML (mlBmMess036));
			return (EXIT_FAILURE);
		}

		strcpy (bmms2_rec.co_no, comm_rec.co_no);
		bmms2_rec.mabr_hash = local_rec.hhbrHash;
		bmms2_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (bmms2, &bmms2_rec, COMPARISON, "r");	
		if (!cc)
		{
			print_mess (ML (mlBmMess037));
			return (EXIT_FAILURE);
		}

		strcpy (bmms_rec.co_no, comm_rec.co_no);
		bmms_rec.hhbr_hash = inmr_rec.hhbr_hash;
		bmms_rec.alt_no = 1;
		bmms_rec.line_no = 0;
		if (strcmp (inmr_rec.source, "BP") &&
		     strcmp (inmr_rec.source, "BM") &&
		     strcmp (inmr_rec.source, "MP") &&
		     strcmp (inmr_rec.source, "MC"))
		{
			cc = 0;
		}
		else
			cc = find_rec (bmms, &bmms_rec, GTEQ, "r");

		if (cc ||
		     strcmp (bmms_rec.co_no, comm_rec.co_no) ||
		     bmms_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			print_mess (ML (mlBmMess006));
			sleep (sleepTime);
			clear_mess ();
		}

		store [line_cnt].hhbrHash = inmr_rec.hhbr_hash;
		strcpy (store [line_cnt].source, inmr_rec.source);
		strcpy (local_rec.matl_desc, inmr_rec.description);

		tab_other (line_cnt);

		/*
		 * check item has a non-zero standard unit cost 
		 */
		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc || inei_rec.std_cost == 0.00)
		{
			if (!strcmp (inmr_rec.source, "BP") &&
		     	!strcmp (inmr_rec.source, "BM") &&
		     	!strcmp (inmr_rec.source, "MP") &&
		     	!strcmp (inmr_rec.source, "MC"))
			{
				sprintf (err_str,ML (mlBmMess040), clip (inmr_rec.item_no));
				ans = prmptmsg (err_str, "YyNn", 1, 23);
				move (1, 23);
				cl_line ();
				if (ans == 'N' || ans == 'n')
					return (EXIT_FAILURE);
			}
			else
			{
				print_mess (ML (mlBmMess035));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum,&inum_rec,EQUAL, "r");
		strcpy (local_rec.matl_uom, (cc) ? "    " : inum_rec.uom);
		strcpy (local_rec.matl_std_uom, (cc) ? "    " : inum_rec.uom);
		sprintf (local_rec.std_group, "%-20.20s",
		    (cc) ? " " : inum_rec.uom_group);
		local_rec.mstd_uom = cc ? 0L : inum_rec.hhum_hash;
		inum_rec.hhum_hash	=	inmr_rec.alt_uom;
		cc = find_rec (inum,&inum_rec,EQUAL, "r");
		sprintf (local_rec.alt_group, "%-20.20s",(cc) ? " " : inum_rec.uom_group);
		local_rec.malt_uom = cc ? 0L : inum_rec.hhum_hash;

		local_rec.dec_pt = inmr_rec.dec_pt;
		local_rec.uom_cfactor = inmr_rec.uom_cfactor;
		sprintf (local_rec.matl_no, "%-16.16s", inmr_rec.item_no);
		sprintf (local_rec.scrap_pc, "%4.1f", inmr_rec.scrap_pc);
		sprintf (local_rec.matl_src, "%-2.2s", inmr_rec.source);
		sprintf (local_rec.matl_cons, "N");
		DSP_FLD ("matl_no");
		DSP_FLD ("matl_desc");
		DSP_FLD ("matl_uom");
		DSP_FLD ("matl_wst_pc");
		DSP_FLD ("matl_src");
		DSP_FLD ("matl_cons");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("matl_uom"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			abc_selfield (inum, "inum_hhum_hash");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.matl_uom, local_rec.matl_std_uom);
			DSP_FLD ("matl_uom");
			return (EXIT_SUCCESS);
		}

		sprintf (inum_rec.uom, "%-4.4s", temp_str);
		cc = find_rec (inum2, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			return (EXIT_FAILURE);
		}
		if
		 (
		    strcmp (inum_rec.uom_group, local_rec.std_group) &&
		    strcmp (inum_rec.uom_group, local_rec.alt_group)
		)
		{
			print_mess (ML (mlStdMess028));
			return (EXIT_FAILURE);
		}

		CalcConv ();
		if (!ValidUOM ())
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("matl_qty"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		local_rec.matl_qty = n_dec (local_rec.matl_qty, local_rec.dec_pt);
		if (!ValidQuantity (local_rec.matl_qty, local_rec.dec_pt))
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("iss_seq"))
	{
	/*
		if (dflt_used || local_rec.iss_seq == 0)
		{
			print_mess (ML (mlBmMess038));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	*/

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	int		i;
	int		this_page = line_cnt / TABLINES;
	int		delta = 1;

	/*
	 * Entry
	 */
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}
	/*
	 * No lines to delete			
	 */
	if (lcount [2] <= 0)
	{
		print_mess (ML (mlStdMess032));
		return (EXIT_FAILURE);
	}
	/*
	 * Delete lines			
	 */
	for (i = line_cnt; line_cnt < lcount [2] - delta; line_cnt++)
	{
		getval (line_cnt + delta);
		putval (line_cnt);

		store [line_cnt].hhbrHash = store [line_cnt + delta].hhbrHash;
		strcpy (store [line_cnt].source, store [line_cnt + delta].source);
		store [line_cnt + delta].hhbrHash = 0L;
		strcpy (store [line_cnt + delta].source, "  ");

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	while (line_cnt < lcount [2])
	{
		sprintf (local_rec.matl_no, "%-16.16s", " ");
		sprintf (local_rec.matl_desc, "%-40.40s", " ");
		sprintf (local_rec.std_group, "%-20.20s", " ");
		sprintf (local_rec.alt_group, "%-20.20s", " ");
		local_rec.mstd_uom = 0;
		local_rec.malt_uom = 0;
		local_rec.cnv_fct = 0;
		local_rec.dec_pt = 0;
		local_rec.uom_cfactor = 0;
		sprintf (local_rec.matl_uom, "%-4.4s", " ");
		local_rec.matl_qty = 0;
		local_rec.matl_wst_pc = 0;
		local_rec.instr_no = 0;
		sprintf (local_rec.matl_src, "%-2.2s", " ");
		local_rec.iss_seq = 0;
		sprintf (local_rec.matl_cons, "%-1.1s", " ");

		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			blank_display ();

		line_cnt++;
	}

	/*
	 * blank last line - if required		
	 */
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	lcount [2] -= delta;

	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
SetupHhbrHash (void)
{
	int		i;
	for (i = 0;i < MAXLINES;i++)
		store [i].hhbrHash = 0L;
}


void
LoadSpec (void)
{
	struct	INS_PTR *currPointer	=	INS_NUL;
	struct	INS_PTR *tempPointer	=	INS_NUL;

	init_vars (2);

	SetupHhbrHash ();
	abc_selfield (inmr, "inmr_hhbr_hash");
	lcount [2] = 0;
	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = local_rec.hhbrHash;
	bmms_rec.alt_no = local_rec.alt_no;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "u");	
	while (!cc &&
	        !strcmp (bmms_rec.co_no, comm_rec.co_no) &&
	        bmms_rec.hhbr_hash == local_rec.hhbrHash &&
	        bmms_rec.alt_no == local_rec.alt_no)
	{
		inmr_rec.hhbr_hash	=	bmms_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	    if (!cc)
	    {
			store [lcount [2]].hhbrHash = bmms_rec.mabr_hash;
			strcpy (store [lcount [2]].source, inmr_rec.source);
			strcpy (local_rec.matl_src, inmr_rec.source);
			strcpy (local_rec.matl_cons, bmms_rec.cons);
			strcpy (local_rec.matl_no, inmr_rec.item_no);
			strcpy (local_rec.matl_desc, inmr_rec.description);
			local_rec.dec_pt = inmr_rec.dec_pt;

			inum_rec.hhum_hash	=	bmms_rec.uom;
			cc = find_rec (inum,&inum_rec,EQUAL, "r");
			strcpy (local_rec.matl_uom, (cc) ? "    " : inum_rec.uom);

			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum,&inum_rec,EQUAL, "r");
			strcpy (local_rec.matl_std_uom, (cc) ? "    " : inum_rec.uom);
			sprintf (local_rec.std_group, "%-20.20s",
		    								(cc) ? " " : inum_rec.uom_group);
			local_rec.mstd_uom = cc ? 0L : inum_rec.hhum_hash;

			inum_rec.hhum_hash	=	inmr_rec.alt_uom;
			cc = find_rec (inum,&inum_rec,EQUAL, "r");
			sprintf (local_rec.alt_group, "%-20.20s",
											(cc) ? " " : inum_rec.uom_group);
			local_rec.malt_uom = cc ? 0L : inum_rec.hhum_hash;

			local_rec.matl_qty = (double) n_dec (bmms_rec.matl_qty, inmr_rec.dec_pt);
			local_rec.matl_wst_pc = bmms_rec.matl_wst_pc;
			local_rec.instr_no = bmms_rec.instr_no;
			local_rec.iss_seq = bmms_rec.iss_seq;

			strcpy (local_rec.instr, "   ");
			strcpy (bmin_rec.co_no, bmms_rec.co_no);
			bmin_rec.hhbr_hash	= bmms_rec.hhbr_hash;
			bmin_rec.alt_no	= bmms_rec.alt_no;
			bmin_rec.line_no	= bmms_rec.line_no;
			bmin_rec.tline	= 0;
			cc = find_rec (bmin, &bmin_rec, GTEQ, "r");
			while
			 (
				!cc &&
				!strcmp (bmin_rec.co_no, bmms_rec.co_no) &&
				bmin_rec.hhbr_hash == bmms_rec.hhbr_hash &&
				bmin_rec.alt_no == bmms_rec.alt_no &&
				bmin_rec.line_no == bmms_rec.line_no
			)
			{
				tempPointer = ins_alloc ();
				strcpy (tempPointer->text, bmin_rec.text);
				if (ins_ptr [lcount [2]] == INS_NUL)
				{
					ins_ptr [lcount [2]] = tempPointer;
					currPointer = tempPointer;
				}
				else
				{
					currPointer->next = tempPointer;
					currPointer = tempPointer;
				}
				strcpy (local_rec.instr, "Yes");
				cc = find_rec (bmin, &bmin_rec, NEXT, "r");
			}

			putval (lcount [2]++);
	    }
	    cc = find_rec (bmms, &bmms_rec, NEXT, "u");	
	}
	scn_set (1);
}

int
CalcConv (void)
{
	char	std_group [21];
	char	alt_group [21];
	number	std_cnv_fct;
	number	alt_cnv_fct;
	number	cnv_fct;
	number	result;
	number	uom_cfactor;

	/*
	 * Get the UOM conversion factor 
	 */
	inum_rec.hhum_hash	=	local_rec.malt_uom;
	cc = find_rec (inum,&inum_rec,EQUAL, "r");
	if (cc)
		file_err (cc, "inum", "DBFIND");

	strcpy (alt_group, inum_rec.uom_group);
	/*
	 * Converts a float to arbitrary precision number defined as number
	 */
	NumFlt (&alt_cnv_fct, inum_rec.cnv_fct);
	inum_rec.hhum_hash	=	local_rec.mstd_uom;
	cc = find_rec (inum,&inum_rec,EQUAL, "r");
	if (cc)
		file_err (cc, "inum", "2DBFIND");
	strcpy (std_group, inum_rec.uom_group);

	/*
	 * Converts a float to arbitrary precision number defined as number 
	 */
	NumFlt (&std_cnv_fct, inum_rec.cnv_fct);

	strcpy (inum_rec.uom, local_rec.matl_uom);
	cc = find_rec (inum2, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inum2", "3DBFIND");

	/*
	 * converts a float to arbitrary precision number defined as number 
	 */
	NumFlt (&cnv_fct, inum_rec.cnv_fct);

	/*
	 * Conversion factor = std uom cnv_fct / iss uom cnv_fct    
	 *       OR                                                
	 * Conversion factor = (std uom cnv_fct / iss uom cnv_fct) 
	 *                     * item's conversion factor          
	 * Same calculation as in pc_recprt.                       
	 * a function that divides one number by another and places
	 * the result in another number defined variable           
	 */
	if (strcmp (alt_group, inum_rec.uom_group))
		NumDiv (&std_cnv_fct, &cnv_fct, &result);
	else
	{
		NumFlt (&uom_cfactor, local_rec.uom_cfactor);
		NumDiv (&alt_cnv_fct, &cnv_fct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*
	 * Converts a arbitrary precision number to a float
	 */
	local_rec.cnv_fct = NumToFlt (&result);

	return (EXIT_SUCCESS);
}

/*
 * Check whether uom is valid compared with 
 * the dec_pt and the conversion factor.    
 * eg. std uom = kg     iss uom = gm        
 *     conv.fact = 1000 dec_pt = 2          
 *     issue 5 gm, converts to 0.005 kg     
 *     round to 2 dec_pt, new qty = 0.01 kg 
 *     or 10 gm                             
 * This is incorrect and not allowed.       
 */
int
ValidUOM (void)
{
	long	numbers [7];
	int		ans;

	numbers [0] = 1;
	numbers [1] = 10;
	numbers [2] = 100;
	numbers [3] = 1000;
	numbers [4] = 10000;
	numbers [5] = 100000;
	numbers [6] = 1000000;

	if (local_rec.cnv_fct > numbers [local_rec.dec_pt])
	{
		sprintf (err_str,ML (mlBmMess050),
			clip (local_rec.matl_uom),
			local_rec.dec_pt);
		ans = prmptmsg (err_str, "YyNn", 1, 23);
		move (1, 23);
		cl_line ();
		if (ans == 'n' || ans == 'N')
			return (FALSE);
	}
	return (TRUE);
}

void
Update (void)
{
	struct	INS_PTR	*currPointer;
	long	hhum_hash;

	clear ();
	print_at (2,0,ML (mlStdMess035));
	fflush (stdout);

	abc_selfield (inmr, "inmr_hhbr_hash");

	/*
	 * Delete any trailing bmin records where line_no > EOF	
	 */
	strcpy (bmin_rec.co_no, comm_rec.co_no);
	bmin_rec.hhbr_hash = local_rec.hhbrHash;
	bmin_rec.alt_no = local_rec.alt_no;
	bmin_rec.line_no = 0;
	bmin_rec.tline = 0;
	cc = find_rec (bmin, &bmin_rec, GTEQ, "u");	
	while
	 (
		!cc &&
		!strcmp (bmin_rec.co_no, comm_rec.co_no) &&
		bmin_rec.hhbr_hash == local_rec.hhbrHash &&
		bmin_rec.alt_no == local_rec.alt_no
	)
	{
	    abc_unlock (bmin);
	    cc = abc_delete (bmin);
	    if (cc)
			file_err (cc, bmin, "DBDELETE");
	    strcpy (bmin_rec.co_no, comm_rec.co_no);
	    bmin_rec.hhbr_hash = local_rec.hhbrHash;
	    bmin_rec.line_no = 0;
	    bmin_rec.tline = 0;
	    cc = find_rec (bmin, &bmin_rec, GTEQ, "u");	
	}
	abc_unlock (bmin);

	scn_set (2);
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
	    getval (line_cnt);

		inmr_rec.hhbr_hash	=	store [line_cnt].hhbrHash;
	    cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	    if (cc)
			file_err (cc, "inmr", "DBFIND");

	    strcpy (inum2_rec.uom, local_rec.matl_uom);
	    cc = find_rec (inum2, &inum2_rec, EQUAL, "r");
	    if (cc)
		    file_err (cc, "inum2", "DBFIND");
	    hhum_hash = inum2_rec.hhum_hash;

	    strcpy (bmms_rec.co_no, comm_rec.co_no);
	    bmms_rec.hhbr_hash = local_rec.hhbrHash;
	    bmms_rec.alt_no = local_rec.alt_no;
	    bmms_rec.line_no = line_cnt;
	    cc = find_rec (bmms, &bmms_rec, COMPARISON, "u");	
	    if (cc)
	    {
			strcpy (bmms_rec.co_no, comm_rec.co_no);
			bmms_rec.hhbr_hash = local_rec.hhbrHash;
			bmms_rec.uom = hhum_hash;
			bmms_rec.alt_no = local_rec.alt_no;
			bmms_rec.line_no = line_cnt;
			strcpy (bmms_rec.cons, local_rec.matl_cons);
			bmms_rec.mabr_hash = store [line_cnt].hhbrHash;
			bmms_rec.matl_qty = (float) local_rec.matl_qty;
			bmms_rec.matl_wst_pc = local_rec.matl_wst_pc;
			bmms_rec.instr_no = local_rec.instr_no;
			bmms_rec.iss_seq = local_rec.iss_seq;
			cc = abc_add (bmms, &bmms_rec);
			if (cc)
				   file_err (cc, "bmms", "DBADD");
	    }
	    else
	    {
			strcpy (bmms_rec.cons, local_rec.matl_cons);
			bmms_rec.mabr_hash = store [line_cnt].hhbrHash;
			bmms_rec.uom = hhum_hash;
			bmms_rec.matl_qty = (float) local_rec.matl_qty;
			bmms_rec.matl_wst_pc = local_rec.matl_wst_pc;
			bmms_rec.instr_no = local_rec.instr_no;
			bmms_rec.iss_seq = local_rec.iss_seq;
			cc = abc_update (bmms, &bmms_rec);
			if (cc)
				file_err (cc, "bmms", "DBUPDATE");
	    }

	    currPointer = ins_ptr [line_cnt];
	    bmin_rec.tline = 0;
	    while (currPointer != INS_NUL)
	    {
			strcpy (bmin_rec.co_no, comm_rec.co_no);
			bmin_rec.hhbr_hash = local_rec.hhbrHash;
			bmin_rec.alt_no = local_rec.alt_no;
			bmin_rec.line_no = line_cnt;
			strcpy (bmin_rec.text, currPointer->text);
			cc = abc_add (bmin, &bmin_rec);
			if (cc)
				file_err (cc, bmin, "DBADD");
			bmin_rec.tline++;
			currPointer = currPointer->next;
	    }

	    abc_unlock (bmms);
	}

	/*
	 * Delete any trailing bmms records where line_no > EOF	
	 */
	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = local_rec.hhbrHash;
	bmms_rec.alt_no = local_rec.alt_no;
	bmms_rec.line_no = lcount [2];
	cc = find_rec (bmms, &bmms_rec, GTEQ, "u");	
	while
	 (
		!cc &&
		!strcmp (bmms_rec.co_no, comm_rec.co_no) &&
		bmms_rec.hhbr_hash == local_rec.hhbrHash &&
		bmms_rec.alt_no == local_rec.alt_no
	)
	{
	    abc_unlock (bmms);
	    cc = abc_delete (bmms);
	    if (cc)
			file_err (cc, "bmms", "DBDELETE");
	    strcpy (bmms_rec.co_no, comm_rec.co_no);
	    bmms_rec.hhbr_hash = local_rec.hhbrHash;
	    bmms_rec.line_no = lcount [2];

	    cc = find_rec (bmms, &bmms_rec, GTEQ, "u");	
	}
	abc_unlock (bmms);

	abc_selfield (inmr, "inmr_id_no");
}

int
heading (
	int	scn)
{

	if (scn == 3)
	{
		line_cnt = 0;
		scn_set (scn);
		scn_write (scn);
		return (EXIT_SUCCESS);
	}

	if (!restart)
	{
		clear ();
		swide ();
		scn_set (1);

		rv_pr (ML (mlBmMess042), 48, 0, 1);

		box (0, 1, 67, 5);
		box (67, 1, 65, 5);
		line_at (4,68,64);
		scn_write (1);
		scn_display (1);

		rv_pr (ML (mlBmMess043), 88, 2, FALSE);
		rv_pr (ML (mlBmMess044), 89, 5, FALSE);

		line_at (20,0,132);

		print_at (21,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

		print_at (6,  69, ML (mlBmMess028), inei_rec.std_batch);
		print_at (6,  90, ML (mlBmMess029), inei_rec.min_batch);
		print_at (6, 111, ML (mlBmMess030), inei_rec.max_batch);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		if (scn > 1)
		{
			scn_set (scn);
			scn_write (scn);
		}
	}
    return (EXIT_SUCCESS);
}

/*
 * Search for Alternates	
 */
void
AlternateSearch (
	char	*key_val)
{
	char	alt_str [6];
	int	curr_alt;

	_work_open (5,0,1);
	save_rec ("#Alt.", "#");

	curr_alt = 0;
	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = local_rec.hhbrHash;
	bmms_rec.alt_no = 1;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");	
	while (!cc &&
	       !strcmp (bmms_rec.co_no, comm_rec.co_no) &&
	       bmms_rec.hhbr_hash == local_rec.hhbrHash)
	{
		sprintf (alt_str, "%5d", bmms_rec.alt_no);

		if (curr_alt == bmms_rec.alt_no)
		{
			cc = find_rec (bmms, &bmms_rec, NEXT, "r");	
			continue;
		}

		curr_alt = bmms_rec.alt_no;
		cc = save_rec (alt_str, "");
		if (cc)
			break;
		cc = find_rec (bmms, &bmms_rec, NEXT, "r");	
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = inmr_rec.hhbr_hash;
	bmms_rec.alt_no = atoi (temp_str);
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
}

void
SrchInum (
	char	*key_val)
{
	_work_open (4,0,40);
	save_rec ("#UOM ", "#Unit Description");
	abc_selfield (inum, "inum_id_no");

	strcpy (inum_rec.uom_group, local_rec.std_group);
	inum_rec.hhum_hash = 0L;
	cc = find_rec (inum, &inum_rec, GTEQ, "r");
    while
	 (
	    !cc &&
	    !strcmp (inum_rec.uom_group, local_rec.std_group)
	)
    {
	    cc = save_rec (inum_rec.uom, inum_rec.desc);
	    if (cc)
			break;
	    cc = find_rec (inum, &inum_rec, NEXT, "r");
	}

	if (strcmp (local_rec.std_group, local_rec.alt_group))
	{
	    strcpy (inum_rec.uom_group, local_rec.alt_group);
	    inum_rec.hhum_hash = 0L;
	    cc = find_rec (inum, &inum_rec, GTEQ, "r");
        while
	    (
		!cc &&
		!strcmp (inum_rec.uom_group, local_rec.alt_group)
	   )
    	{
			cc = save_rec (inum_rec.uom, inum_rec.desc);
			if (cc)
		    	break;
			cc = find_rec (inum, &inum_rec, NEXT, "r");
	    }
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	abc_selfield (inum, "inum_uom");
	sprintf (inum_rec.uom, "%-4.4s", temp_str);
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inum", "4DBFIND");
}

void
tab_other (
 int                line_no)
{
	FLD ("matl_cons") = NA;
	if (store [line_no].hhbrHash != 0L && store [line_no].source [0] == 'B')
		FLD ("matl_cons") = YES;
}

/*
 * Checks if the quantity entered by the user 
 * valid quantity that can be saved to a     
 * float variable without any problems of   
 * losing figures after the decimal point. 
 * eg. if _dec_pt is 2 then the greatest  
 * quantity the user can enter is 99999.99
 */
int
ValidQuantity (
 double             _qty,
 int                _dec_pt)
{
	/*
	 * Quantities to be compared with with the user has entered.     
	 */
	double	compare [7];
	
	compare [0] = 9999999.00;
	compare [1] = 999999.90;
	compare [2] = 99999.99;
	compare [3] = 9999.999;
	compare [4] = 999.9999;
	compare [5] = 99.99999;
	compare [6] = 9.999999;

	if (_qty > compare [_dec_pt])
	{
		sprintf (err_str, ML (mlBmMess041), _qty, compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}
