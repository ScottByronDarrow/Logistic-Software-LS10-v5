/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_ariface.c,v 5.11 2002/11/18 07:19:45 kaarlo Exp $
|  Program Name  : (gl_ariface.c)
|  Program Desc  : (General Ledger A/R Interface)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius.  | Date Written  : 29/05/95         |
|---------------------------------------------------------------------|
| $Log: gl_ariface.c,v $
| Revision 5.11  2002/11/18 07:19:45  kaarlo
| Updated to fix SC0030.
|
| Revision 5.10  2002/07/24 08:38:52  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.9  2002/06/26 05:08:34  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.8  2002/02/26 02:50:22  scott
| S/C 836 GLIF4 (character-based) / GLIF5 (GUI) - Maintain General Ledger Interrface / Maintain Control Accounts - The system allows the entry of Branch numbers upt to No. 99, however in the GL Interface Maintenance, it limits the use to up to No.9.  The user is given an error message
|
| Revision 5.7  2001/08/28 10:11:47  robert
| additional update for LS10.5-GUI
|
| Revision 5.6  2001/08/26 23:20:12  scott
| Updated from scotts machine - ongoing WIP release 10.5
|
| Revision 5.5  2001/08/20 23:12:45  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_ariface.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_ariface/gl_ariface.c,v 5.11 2002/11/18 07:19:45 kaarlo Exp $";
/*
 * Include file dependencies.
 */
#define MAXLINES    200 
#define MAXWIDTH    150
#define TABLINES    10

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>
#include <hot_keys.h>
#include <GlUtils.h>
#include <gl_idesc9.10.h>
#include <minimenu.h>

#define	INACTIVE(x)		(x == -1)
#define	IFOFF(x)		(x == 0)
#define	ACTIVE(x)		(x == 1)

/*
 * Local variables.
 */
int     NewCode 		= FALSE,
		tab_lns 		= 10,
		box_lns 		= 3,
		screenWidth 	= 130,
		envGlByClass 	= TRUE;

char    maintType 		[2],
		LastAccount 	[MAXLEVEL + 1],
		passedInterface [11];

/*
 * Constants, defines and stuff.
 */
#undef	TABLINES
#define	TABLINES    (tab_lns)

#define	SCN_MAIN		1
#define	SCN_DETAIL		2
#define	SCN_SUMMARY		3

#define	SUCCESS			0
#define	RETURN_ERR		1
#define	SLEEP_TIME		2

#define SEL_UPDATE	0
#define SEL_IGNORE	1
#define SEL_DELETE	2
#define SEL_DEFAULT	99

#define NO_KEY(x)	 (vars [x].required == NA || \
			  		  vars [x].required == NI || \
			  		  vars [x].required == ND)

#define HIDE(x)		 (vars [x].required == ND)

#include	"schema"

char	*data = "data";

const char	*glih2 = "glih2",
			*glic2 = "glic2";

GLIH_STRUCT		glih2Rec;

struct	commRecord		comm_rec;
struct	glicRecord		glic_rec;
struct	glicRecord		glic2_rec;
struct	esmrRecord		esmr_rec;
struct	ccmrRecord		ccmr_rec;
struct	excfRecord		excf_rec;
struct	exclRecord		excl_rec;
struct	exsfRecord		exsf_rec;

MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
	  "" },
	{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
	  "" },
	{ " 3. DELETE RECORD.                     ",
	  "" },
	{ ENDMENU }
};

	struct	storeRec	{
		int		ifActive;
		int		ifBrNoOK;
		int		ifWhNoOK;
		int		ifCategoryOK;
		int		ifClassSaleOK;
	} store [MAXLINES];
/*
 * Local & Screen Structures.
 */
struct 
{
	char	dummy 			[11],
			int_code 		[11],
			accDesc 		[26],
			currCode 		[4],
			GlMask 			[MAXLEVEL + 1],
			ifActive 		[2],
			ifBrNoOK 		[2],
			ifWhNoOK 		[2],
			ifCategoryOK 	[2],
			ifClassSaleOK 	[2];
} local_rec;

static struct var vars [] =
{
	{1, LIN, "intCode",	 2, 22, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Interface Code   : ", " ",
		NE, NO,  JUSTLEFT, "", "", glihRec.int_code},
	{1, LIN, "intDesc",	 2, 58, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", glic_rec.desc},
	{1, LIN, "classType",	 3, 22, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Customer Type    : ", "Enter Customer Type Code. [SEARCH].",
		ND, NO,  JUSTLEFT, "", "", glihRec.class_type},
	{1, LIN, "classDesc",	 3, 58, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		ND, NO,  JUSTLEFT, "", "", excl_rec.class_desc},
	{1, LIN, "salesmanCode",	 3, 22, CHARTYPE,
		"UU", "          ",
		" ", " ", "Salesman No.     : ", "Enter Salesman Code. [SEARCH].",
		ND, NO,  JUSTLEFT, "", "", glihRec.class_type},
	{1, LIN, "salesmanDesc",	 3, 58, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		ND, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{1, LIN, "categoryCode",	 4, 22, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Category Code    : ", "Enter Product Category. [SEARCH].",
		 ND, NO, JUSTLEFT, "", "", glihRec.cat_no},
	{1, LIN, "categoryDesc",	 4, 58, CHARTYPE,
	"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		ND, NO,  JUSTLEFT, "", "", excf_rec.cat_desc},
	{2, TAB, "brNo",	MAXLINES, 0, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.est_no, "Br. No", "Enter Branch number. [SEARCH]",
		 NA, NO, JUSTRIGHT, "", "", esmr_rec.est_no},
	{2, TAB, "brName",	 0, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "", " Branch Short Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", esmr_rec.short_name},
	{2, TAB, "whNo",	 0, 2, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.cc_no, "Wh. No", "Enter Warehouse number. [SEARCH]",
		 NA, NO, JUSTRIGHT, "", "", ccmr_rec.cc_no},
	{2, TAB, "whName",	 0, 5, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", " Warehouse Acronym ", " ",
		 NA, NO,  JUSTLEFT, "", "", ccmr_rec.acronym},
	{2, TAB, "acctNo",	 0, 2, CHARTYPE,
		local_rec.GlMask, "          ",
		"0", "0", "General Ledger Account" , "Enter Cash Account Number for Interface Code. [SEARCH]",
		 YES, NO, JUSTLEFT, "", "", glidRec.acct_no},
	{2, TAB, "acctCurr",	 0, 0, CHARTYPE,
		"AAA", "          ",
		" ", "", "Curr", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.currCode},
	{2, TAB, "acctDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " G/L Account Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc},
	{3, TAB, "intCode2",	 MAXLINES, 1, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "  Interface  ", " ",
		NA, NO,  JUSTLEFT, "", "", glic_rec.code},
	{3, TAB, "intDesc2",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "          Interface Description         ", " ",
		 NA, NO,  JUSTLEFT, "", "", glic_rec.desc},
	{3, TAB, "acct2_no",	 0, 1, CHARTYPE,
		local_rec.GlMask, "          ",
		"0", "0", "General Ledger Acct", "Enter General Ledger Account number for Interface Code. [SEARCH]",
		 YES, NO, JUSTLEFT, "", "", glic_rec.acct_no},
	{3, TAB, "acct2_curr",	 0, 0, CHARTYPE,
		"AAA", "          ",
		" ", "", "Cur", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.currCode},
	{3, TAB, "acct2_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " G/L Account Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc},
	{3, TAB, "ifActive",	 0, 7, CHARTYPE,
		"U", "          ",
		" ", "", "Interface Active", "Enter Y(es) if interface is active. N(ot) active ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ifActive},
	{3, TAB, "ifBrNoOK",	 0, 4, CHARTYPE,
		"U", "          ",
		" ", "", "By Branch", "Enter Y(es) if interface by Branch is active. N(ot) active ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ifBrNoOK},
	{3, TAB, "ifWhNoOK",	 0, 6, CHARTYPE,
		"U", "          ",
		" ", "", "By Warehouse", "Enter Y(es) if interface by Warehouse is active. N(ot) active ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ifWhNoOK},
	{3, TAB, "ifCategoryOK",	 0, 5, CHARTYPE,
		"U", "          ",
		" ", "", "By Category", "Enter Y(es) if interface by category is active. N(ot) active ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ifCategoryOK},
	{3, TAB, "ifClassSaleOK",	 0, 3, CHARTYPE,
		"U", "          ",
		" ", "", "By Type", "Enter Y(es) if interface by type/salesman is active. N(ot) active ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ifClassSaleOK},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function declarations.
 */
char	*IntToChar 		(int);
int		CharToInt		(char *);
int	 	heading 		(int);
int  	spec_valid 		(int);
int 	FindGLAcct 		(char *, int);
void 	tab_other 		(int);
void	shutdown_prog 	(void);
void 	CloseDB 		(void);
void 	DelIface 		(long);
void 	DisplayIface 	(void);
void 	LoadGlic 		(void);
void 	LoadWhTable 	(void);
void 	OpenDB 			(void);
void 	SrchCcmr 		(char *);
void 	SrchEsmr 		(char *);
void 	SrchExcf 		(char *);
void 	SrchExcl 		(char *);
void 	SrchExsf 		(char *);
void 	SrchGlic 		(char *);
void 	Update 			(void);
void 	UpdateGlic 		(void);

/*
 * Main Processing Routine.
 */
int
main (
 int  argc, 
 char *argv[])
{
	char	*sptr;

	if (argc < 2)
	{
		clear ();
		print_at (0,0,mlGlMess151, argv [0]);
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("GL_BYCLASS");
	envGlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sprintf (local_rec.GlMask,"%-*.*s", MAXLEVEL, MAXLEVEL, "NNNNNNNNNNNNNNNN");

	sprintf (maintType, "%-1.1s", argv [1]);

	SETUP_SCR (vars);

	_set_masks ("gl_ariface.s");
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (3, store, sizeof (struct storeRec));
#endif

	if (argc > 2)
	{
		strcpy (maintType, "I");
		sprintf (passedInterface, "%-10.10s", argv [2]);
		FLD ("intCode")	=	NI;
	}
	tab_row = 4;
	tab_col = 0;

	if (maintType [0] == 'I')
	{
		tab_row = 6;
		tab_col = 10;
		tab_lns = 12;
	}
	else
	{
		tab_row = 2;
		tab_col = 0;
		tab_lns = 16;
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	init_vars (SCN_MAIN);

	clear ();
	
	swide ();
	screenWidth = 132;

	OpenDB ();

	GL_SetAccWidth (comm_rec.co_no, FALSE);

	if (maintType [0] == 'I')
	{
		init_vars (SCN_DETAIL);
		no_edit (SCN_SUMMARY);
	}
	else
	{
		if (maintType [0] == 'S')
		{
			FLD ("acct2_no")		=	ND;
			FLD ("acct2_desc")		=	ND;
			FLD ("acct2_curr")		=	ND;
			FLD ("ifActive")		=	YES;
			FLD ("ifCategoryOK")	=	YES;
			FLD ("ifClassSaleOK")	=	YES;
			FLD ("ifBrNoOK")		=	YES;
			FLD ("ifWhNoOK")		=	YES;
		}
		memset (&glic_rec, 0, sizeof glic_rec);
		no_edit (SCN_MAIN);
		no_edit (SCN_DETAIL);
		init_vars (SCN_SUMMARY);
		LoadGlic ();
	}

	/*
	 * Beginning of input control loop.
	 */
	while (!prog_exit)
	{
		/*
		 * Reset control flags.
		 */
		entry_exit = FALSE;
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;

		if (maintType [0] == 'I')
		{
			lcount [SCN_DETAIL] = 0;
			init_vars (SCN_MAIN);
			init_vars (SCN_DETAIL);
	
			/*
			 * Enter screen 1 linear input.
			 */
			heading (SCN_MAIN);
			scn_display (SCN_MAIN);
			entry (SCN_MAIN);
			if (restart || prog_exit)
				continue;
	
			/*
			 * Load Warehouse table.
			 */
			LoadWhTable ();
	
			heading (SCN_DETAIL);
			scn_display (SCN_DETAIL);
			edit (SCN_DETAIL);
	
			if (restart)
			{
				abc_unlock (glih);
				continue;
			}
	
			Update ();
			abc_unlock (glih);
		}
		else
		{
			/*
			 * Enter screen 3 linear input.
			 */
			heading (SCN_SUMMARY);

			if (restart || prog_exit)
				continue;
	
			scn_display (SCN_SUMMARY);
			edit (SCN_SUMMARY);

			if (!restart)
                UpdateGlic ();
            
			prog_exit = TRUE;
		}
		if (F_NOKEY (label ("intCode")))
			break;
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
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (glih2, glih);

	open_rec (glic,  glic_list, GLIC_NO_FIELDS, "glic_id_no");
	open_rec (glih2, glih_list, GLIH_NO_FIELDS, "glih_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (excl,  excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (exsf,  exsf_list, EXSF_NO_FIELDS, "exsf_id_no");

	OpenGlmr ();
	OpenGlid ();
	OpenGlih ();
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (glic);
	abc_fclose (glih2);
	abc_fclose (glmr);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_fclose (excf);
	abc_fclose (excl);
	abc_fclose (exsf);

	GL_Close ();

	abc_dbclose (data);
}

int
spec_valid (
	int	field)
{
	/*
	 * Validate Interface Code.
	 */
	if (LCHECK ("intCode"))
	{
		if (F_NOKEY (label ("intCode")))
		{
			strcpy (glihRec.int_code, passedInterface);
			DSP_FLD ("intCode");
		}

		if (SRCH_KEY)
		{
			SrchGlic (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (glic_rec.co_no, comm_rec.co_no);
		sprintf (glic_rec.code, "%-10.10s", glihRec.int_code);

        cc = find_rec (glic, &glic_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlGlMess164));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (!glic_rec.int_active && maintType [0] != 'S')
		{
			errmess (ML ("Interface exists but is not active."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.ifActive,     IntToChar (glic_rec.int_active));
		strcpy (local_rec.ifCategoryOK, IntToChar (glic_rec.cat_ok));
		strcpy (local_rec.ifClassSaleOK,IntToChar (glic_rec.class_ok));
		strcpy (local_rec.ifBrNoOK, 	IntToChar (glic_rec.br_no_ok));
		strcpy (local_rec.ifWhNoOK, 	IntToChar (glic_rec.wh_no_ok));
		/*
		 * Interface includes category.
		 */
		if (ACTIVE (glic_rec.cat_ok))
		{
			if (!ACTIVE (glic_rec.class_ok))
			{
				vars [label ("categoryCode")].row	=	3;
				vars [label ("categoryDesc")].row	=	3;
			}
			else
			{
				vars [label ("categoryCode")].row	=	4;
				vars [label ("categoryDesc")].row	=	4;
			}
			FLD ("categoryCode")	=	YES;
			FLD ("categoryDesc")	=	NA;
			display_prmpt (label ("categoryCode"));
			display_prmpt (label ("categoryDesc"));
		}
		else
		{
			FLD ("categoryCode")	=	ND;
			FLD ("categoryDesc")	=	ND;
		}

		/*
		 * Interface includes class.
		 */
		if (ACTIVE (glic_rec.class_ok))
		{
			if (envGlByClass)
			{
				FLD ("classType")	=	YES;
				FLD ("classDesc")	=	NA;
				display_prmpt (label ("classType"));
				display_prmpt (label ("classDesc"));
			}
			else
			{
				FLD ("salesmanCode")	= YES;
				FLD ("salesmanDesc")	= NA;
				display_prmpt (label ("salesmanCode"));
				display_prmpt (label ("salesmanDesc"));
			}
		}
		else
		{
			FLD ("classType")		= ND;
			FLD ("classDesc")		= ND;
			FLD ("salesmanCode")	= ND;
			FLD ("salesmanDesc")	= ND;
		}
		
		FLD ("brNo")		=	(ACTIVE (glic_rec.br_no_ok)) ? YES : NA;
		FLD ("brName")		=	(ACTIVE (glic_rec.br_no_ok)) ? NA  : NA;
		FLD ("whNo")		=	(ACTIVE (glic_rec.wh_no_ok)) ? YES : NA;
		FLD ("whName")		=	(ACTIVE (glic_rec.wh_no_ok)) ? NA  : NA;

		DSP_FLD ("intDesc");

		DisplayIface ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Interface Code.
	 */
	if (LCHECK ("intCode2"))
	{
		if (last_char == EOI)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchGlic (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (glic_rec.co_no, comm_rec.co_no);
		sprintf (glic_rec.code, "%-10.10s", glic_rec.code);

        cc = find_rec (glic, &glic_rec, EQUAL, "u");
		if (cc) 
			NewCode = TRUE;
		else
		{
			NewCode = FALSE;
			entry_exit = TRUE;
			DSP_FLD ("intDesc2");

            cc = FindGLAcct (glic_rec.acct_no, TRUE);
			if (!cc)
			{
				sprintf (local_rec.accDesc, "%-25.25s",glmrRec.desc);
				sprintf (local_rec.currCode, "%-3.3s",glmrRec.curr_code);
			}
			else
			{
				sprintf (local_rec.accDesc, "%-25.25s", " ");
				sprintf (local_rec.currCode, "%-3.3s", " ");
			}
			DSP_FLD ("acct2_no");
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Category Code.
	 */
	if (LCHECK ("categoryCode"))
	{
		if (NO_KEY (label ("categoryCode")))
		{
			sprintf (glihRec.cat_no, "%-11.11s", " ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (excf_rec.cat_desc, " ");
			DSP_FLD ("categoryDesc");
			return (EXIT_SUCCESS);	
		}

		strcpy (excf_rec.co_no, comm_rec.co_no);
		strcpy (excf_rec.cat_no, glihRec.cat_no);

        cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("categoryDesc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Sell Group Code.
	 */
	if (LCHECK ("classType"))
	{
		if (NO_KEY (field))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (excl_rec.class_desc, " ");
			DSP_FLD ("classDesc");
			return (EXIT_SUCCESS);	
		}

		strcpy (excl_rec.co_no, comm_rec.co_no);
		strcpy (excl_rec.class_type, glihRec.class_type);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("classDesc");
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Salesman as header Level.
	 */
	if (LCHECK ("salesmanCode"))
	{
		if (NO_KEY (field))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
			
		if (dflt_used)
		{
			strcpy (exsf_rec.salesman, " ");
			DSP_FLD ("salesmanDesc");
			return (EXIT_SUCCESS);	
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%-2.2s", glihRec.class_type);

        cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("salesmanDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Debit G/L Account.
	 */
	if (LCHECK ("acctNo") || LCHECK ("acct2_no"))
	{
		if (SRCH_KEY)
		{
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			if (maintType [0] == 'I')
				strcpy (glidRec.acct_no, LastAccount);
			else
				strcpy (glic_rec.acct_no, LastAccount);
		}

		if (maintType [0] == 'I')
			cc = FindGLAcct (glidRec.acct_no, FALSE);
		else
			cc = FindGLAcct (glic_rec.acct_no, FALSE);

		if (!cc)
		{
			sprintf (local_rec.accDesc, "%-25.25s", glmrRec.desc);
			sprintf (local_rec.currCode, "%-3.3s", glmrRec.curr_code);
		}
		else
		{
			sprintf (local_rec.accDesc, "%-25.25s", " ");
			sprintf (local_rec.currCode, "%-3.3s", " ");
		}

		if (maintType [0] == 'I')
		{
			DSP_FLD ("acctDesc");
			DSP_FLD ("acctCurr");
		}
		else
		{
			DSP_FLD ("acct2_desc");
			DSP_FLD ("acct2_curr");
		}

		if (maintType [0] == 'I')
			strcpy (LastAccount, glidRec.acct_no);
		else
			strcpy (LastAccount, glic_rec.acct_no);

		return (cc);
	}
	return (EXIT_SUCCESS);
}

int	
CharToInt (
	char	*interfaceCode)
{
	if (interfaceCode [0] == '-')
		return (-1);
	else if (interfaceCode [0] == 'N')
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}
char	*
IntToChar (
	int		interfaceCode)
{
	if (ACTIVE (interfaceCode))
		return ("Y");
	else if (IFOFF (interfaceCode))
		return ("N");

	return ("-");
}
void
LoadWhTable (void)
{
	strcpy (glihRec.co_no, comm_rec.co_no);

    cc = find_rec (glih, &glihRec, COMPARISON, "w");
	if (cc) 
		NewCode = TRUE;
	else    
	{
		NewCode = FALSE;
		entry_exit 	= 1;

		if (FLD ("categoryCode") != ND)
		{
			strcpy (excf_rec.co_no,  comm_rec.co_no);
			strcpy (excf_rec.cat_no, glihRec.cat_no);
			if (find_rec (excf, &excf_rec, COMPARISON, "r"))
				strcpy (excf_rec.cat_desc, " ");
		}

		if (FLD ("classType") != ND)
		{
			strcpy (excl_rec.co_no, comm_rec.co_no);
			strcpy (excl_rec.class_type,  glihRec.class_type);
			if (find_rec (excl, &excl_rec, COMPARISON, "r"))
				strcpy (excl_rec.class_desc, " ");
		}
		if (FLD ("salesmanCode") != ND)
		{
			strcpy (exsf_rec.co_no, comm_rec.co_no);
			sprintf (exsf_rec.salesman_no, "%-2.2s",  glihRec.class_type);
			if (find_rec (exsf, &exsf_rec, COMPARISON, "r"))
				strcpy (exsf_rec.salesman, " ");
		}

		DSP_FLD ("intCode");
		DSP_FLD ("intDesc");
		DSP_FLD ("categoryCode");
		DSP_FLD ("categoryDesc");
		DSP_FLD ("classType");
		DSP_FLD ("classDesc");
		DSP_FLD ("salesmanCode");
		DSP_FLD ("salesmanDesc");
	}

	scn_set (SCN_DETAIL);
	init_vars (SCN_DETAIL);
	lcount [SCN_DETAIL] = 0;

	if (ACTIVE (glic_rec.br_no_ok))
	{
		/*
		 * Find branch master file record.
		 */
		strcpy (esmr_rec.co_no,  comm_rec.co_no);
		strcpy (esmr_rec.est_no, "  ");
		cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
		while (!cc && !strcmp (esmr_rec.co_no,  comm_rec.co_no))
		{
			if (!NewCode)
			{
				glidRec.hhih_hash = glihRec.hhih_hash;
				strcpy (glidRec.br_no, esmr_rec.est_no);
				strcpy (glidRec.wh_no, "  ");
	
				cc = find_rec (glid, &glidRec, EQUAL, "r");
				if (cc)
					strcpy (glidRec.acct_no, glic_rec.acct_no);
			}
			else
				strcpy (glidRec.acct_no, glic_rec.acct_no);

			cc = FindGLAcct (glidRec.acct_no, TRUE);
			strcpy (local_rec.accDesc, 	(cc) ? " " : glmrRec.desc);
			strcpy (local_rec.currCode, (cc) ? " " : glmrRec.curr_code);
		 	strcpy (ccmr_rec.acronym, ML ("N/A"));

			putval (lcount [SCN_DETAIL]++);
			cc = find_rec (esmr, &esmr_rec, NEXT, "r");
		}
	}
	if (ACTIVE (glic_rec.wh_no_ok))
	{
    	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
		strcpy (ccmr_rec.est_no, "  ");
		strcpy (ccmr_rec.cc_no , "  ");
		cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
		while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
		{
			strcpy (esmr_rec.co_no,  ccmr_rec.co_no);
			strcpy (esmr_rec.est_no, ccmr_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
				continue;
			}
			if (!NewCode)
			{
				glidRec.hhih_hash = glihRec.hhih_hash;
				strcpy (glidRec.br_no, ccmr_rec.est_no);
				strcpy (glidRec.wh_no, ccmr_rec.cc_no);
	
            	cc = find_rec (glid, &glidRec, EQUAL, "r");
				if (cc)
					strcpy (glidRec.acct_no, glic_rec.acct_no);
			}
			else
				strcpy (glidRec.acct_no, glic_rec.acct_no);
	
        	cc = FindGLAcct (glidRec.acct_no, TRUE);
			strcpy (local_rec.accDesc, 	(cc) ? " " : glmrRec.desc);
			strcpy (local_rec.currCode, (cc) ? " " : glmrRec.curr_code);

			putval (lcount [SCN_DETAIL]++);
			cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
		}
	}
	if (!ACTIVE (glic_rec.br_no_ok) && !ACTIVE (glic_rec.wh_no_ok))
	{
		if (!NewCode)
		{
			glidRec.hhih_hash = glihRec.hhih_hash;
			strcpy (glidRec.br_no, "  ");
			strcpy (glidRec.wh_no, "  ");
	
			cc = find_rec (glid, &glidRec, EQUAL, "r");
			if (cc)
				strcpy (glidRec.acct_no, glic_rec.acct_no);
		}
		else
			strcpy (glidRec.acct_no, glic_rec.acct_no);
	
		cc = FindGLAcct (glidRec.acct_no, TRUE);
		if (!cc)
		{
			sprintf (local_rec.accDesc, "%-25.25s",glmrRec.desc);
			sprintf (local_rec.currCode, "%-3.3s",glmrRec.curr_code);
		}
		else
		{
			sprintf (local_rec.accDesc, "%-25.25s", " ");
			sprintf (local_rec.currCode, "%-3.3s", " ");
		}
		strcpy (esmr_rec.short_name, ML ("ALL BRANCHES"));
		strcpy (ccmr_rec.acronym, ML ("N/A"));
		putval (lcount [SCN_DETAIL]++);
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	vars [scn_start].row = lcount  [SCN_DETAIL];

	scn_set (SCN_MAIN);

	return;
}

/*
 * Update competitor codes.
 */
void
Update (void)
{
	int		NewGlid;
	int		i;
	int		exitLoop;

	if ((maintType [0] == 'C') || (maintType [0] == 'S'))
	{
		glic_rec.int_active	= CharToInt (local_rec.ifActive);
		glic_rec.cat_ok		= CharToInt (local_rec.ifCategoryOK);
		glic_rec.class_ok	= CharToInt (local_rec.ifClassSaleOK);
		glic_rec.br_no_ok	= CharToInt (local_rec.ifBrNoOK);
		glic_rec.wh_no_ok	= CharToInt (local_rec.ifWhNoOK);

		if (NewCode)
		{
			cc = abc_add (glic, &glic_rec);
			if (cc)
				file_err (cc, glic, "DBADD");
	
			cc = find_rec (glic, &glic_rec, EQUAL, "u");
			if (cc)
				file_err (cc, glic, "DBFIND");
		}
		else
		{
			cc = abc_update (glic, &glic_rec);
			if (cc)
                file_err (cc, glic, "DBUPDATE");
		}
        return;
	}

	strcpy (glihRec.co_no, comm_rec.co_no);
	NewCode = find_rec (glih, &glihRec, EQUAL, "u");
	if (NewCode)
	{
		strcpy (glihRec.int_desc, glic_rec.desc);
		cc = abc_add (glih, &glihRec);
		if (cc)
            file_err (cc, glih, "DBADD");

		cc = find_rec (glih, &glihRec, EQUAL, "u");
		if (cc)
            file_err (cc, glih, "DBFIND");

		/*
		 * Delete out old records.
		 */
		DelIface (glihRec.hhih_hash);

		/*
		 * Add glid records from TAB screen.
		 */
		scn_set (SCN_DETAIL);
		for (i = 0; i < lcount [SCN_DETAIL]; i++)
		{
			getval (i);
			glidRec.hhih_hash = glihRec.hhih_hash;
			strcpy (glidRec.br_no, esmr_rec.est_no);
			strcpy (glidRec.wh_no, ccmr_rec.cc_no);
			NewGlid = find_rec (glid, &glidRec, EQUAL, "u");

			getval (i); 

			if (NewGlid)
			{
				if (strcmp (glidRec.acct_no, "0000000000000000"))
				{
					glidRec.hhih_hash = glihRec.hhih_hash;
					strcpy (glidRec.br_no, esmr_rec.est_no);
					strcpy (glidRec.wh_no, ccmr_rec.cc_no);
	
                	cc = abc_add (glid, &glidRec);
					if (cc)
						file_err (cc, glid, "DBADD");
				}
			}
			else
			{
				if (!strcmp (glidRec.acct_no, "0000000000000000"))
				{
					cc = abc_delete (glid);
					if (cc)
						file_err (cc, glid, "DBDELETE");
				}
				else
				{
					cc = abc_update (glid, &glidRec);
					if (cc)
						file_err (cc, glid, "DBUPDATE");
				}
			}
		}
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print (" UPDATE SELECTION.                     ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				strcpy (glihRec.int_desc, glic_rec.desc);

                cc = abc_update (glih, &glihRec);
				if (cc)
					file_err (cc, glih, "DBUPDATE");

				/*
				 * Add glid records from TAB screen.
				 */
				scn_set (SCN_DETAIL);
				for (i = 0; i < lcount [SCN_DETAIL]; i++)
				{
					getval (i);
					glidRec.hhih_hash = glihRec.hhih_hash;
					strcpy (glidRec.br_no, esmr_rec.est_no);
					strcpy (glidRec.wh_no, ccmr_rec.cc_no);
					NewGlid = find_rec (glid, &glidRec, EQUAL, "u");

					getval (i); /* getval again to overwrite read accounts */

					if (NewGlid)
					{
						if (strcmp (glidRec.acct_no, "0000000000000000"))
						{
							glidRec.hhih_hash = glihRec.hhih_hash;
							strcpy (glidRec.br_no, esmr_rec.est_no);
							strcpy (glidRec.wh_no, ccmr_rec.cc_no);
	
                        	cc = abc_add (glid, &glidRec);
							if (cc)
								file_err (cc, glid, "DBADD");
						}
					}
					else
					{
						if (strcmp (glidRec.acct_no, "0000000000000000"))
						{
							cc = abc_update (glid, &glidRec);
							if (cc)
                            	file_err (cc, glid, "DBUPDATE");
						}
						else
						{
							cc = abc_delete (glid);
							if (cc)
                            	file_err (cc, glid, "DBDELETE");
						}
					}
				}
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (glih);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				DelIface (glihRec.hhih_hash);

				cc = abc_delete (glih);
				if (cc)
					file_err (cc, glih, "DBDELETE");
				
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
}

void
DelIface (
	long	hhihHash)
{
	glidRec.hhih_hash = hhihHash;
	strcpy (glidRec.br_no, "  ");
	strcpy (glidRec.wh_no, "  ");

    cc = find_rec (glid, &glidRec, GTEQ, "u");
	while (!cc && (glidRec.hhih_hash == hhihHash))
	{
		cc = abc_delete (glid);
		if (cc)
			file_err (cc, glid, "DBDELETE");

		glidRec.hhih_hash = hhihHash;
		strcpy (glidRec.br_no, "  ");
		strcpy (glidRec.wh_no, "  ");
		cc = find_rec (glid, &glidRec, GTEQ, "u");
	}
	abc_unlock (glid);
}

/*
 * Search on Interface Code.
 */
void
SrchGlic (
	char	*keyValue)
{
	_work_open (11,0,40);

	save_rec ("#Interface.", "#Interface Code Description");

	strcpy  (glic_rec.co_no,    comm_rec.co_no);
	sprintf (glic_rec.code, "%-10.10s", keyValue);

    cc = find_rec (glic, &glic_rec, GTEQ, "r");
	while (!cc && !strcmp  (glic_rec.co_no,    comm_rec.co_no) &&     
           		  !strncmp (glic_rec.code, keyValue, strlen (keyValue)))
	{
		if (glic_rec.int_active || maintType [0] == 'S')
		{
			cc = save_rec (glic_rec.code, glic_rec.desc);
			if (cc)
				break;
		}
		cc = find_rec (glic, &glic_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
        return;

	strcpy  (glic_rec.co_no,    comm_rec.co_no);
	sprintf (glic_rec.code, "%-10.10s", temp_str);

    cc = find_rec (glic, &glic_rec, EQUAL, "r");
	if (cc)
		file_err (cc, glic, "DBFIND");
}

/*
 * Search on Category Code.
 */
void
SrchExcf ( 
	char	*keyValue)
{
	_work_open (11,0,40);
	save_rec ("#Code", "#Category Description");
	strcpy  (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no,  "%-11.11s", keyValue);

    cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc 	&& 
           !strcmp  (excf_rec.co_no, comm_rec.co_no) && 
           !strncmp (excf_rec.cat_no,  keyValue, strlen (keyValue)))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

        cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy  (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no,  "%-11.11s", temp_str);

    cc = find_rec (excf, &excf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}
/*
 * Search on Customer Class.
 */
void
SrchExcl (
	char	*keyValue)
{
	_work_open (3,0,40);
	save_rec ("#No ", "#Customer Class Description");
	strcpy  (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type,  "%-3.3s", keyValue);

    cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && 
           !strcmp  (excl_rec.co_no, comm_rec.co_no) && 
           !strncmp (excl_rec.class_type,  keyValue, strlen (keyValue)))
	{
		cc = save_rec (excl_rec.class_type, excl_rec.class_desc);
		if (cc)
			break;

		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}

    cc = disp_srch ();
    work_close ();
	if (cc)
        return;

	strcpy  (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type,  "%-3.3s", temp_str);

    cc = find_rec (excl, &excl_rec, EQUAL, "r");
	if (cc)
		file_err (cc, excl, "DBFIND");
}

/*
 * Search for salesman.
 */
void
SrchExsf (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No","#Salesman.");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",keyValue);

    cc = find_rec (exsf,&exsf_rec,GTEQ,"r");
	while (!cc && 
           !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
           !strncmp (exsf_rec.salesman_no,keyValue,strlen (keyValue)))
	{
		cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;
        
		cc = find_rec (exsf,&exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
    
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);

    cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");
}

void
SrchEsmr (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No","#Branch number description");
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", keyValue);

    cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, comm_rec.co_no) && 
	       !strncmp (esmr_rec.est_no, keyValue, strlen (keyValue)))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", temp_str);

    cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchCcmr (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Warehouse no description");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, esmr_rec.est_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", keyValue);

    cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc &&  !strcmp (comm_rec.co_no, ccmr_rec.co_no) && 
	       		   !strcmp (esmr_rec.est_no, ccmr_rec.est_no)  &&
	               !strncmp (ccmr_rec.cc_no, keyValue, strlen (keyValue)))
	{
		cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
		if (cc)
			break;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (ccmr_rec.co_no,  esmr_rec.co_no);
	sprintf (ccmr_rec.est_no, esmr_rec.est_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", temp_str);

    cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

int
heading (
 int scn)
{
	int		boxSize	=	1;
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	if (maintType [0] == 'I')
		rv_pr (ML (mlGlMess105), (screenWidth - strlen (ML (mlGlMess105))) / 2, 0, 1);
	if (maintType [0] == 'C')
		rv_pr (ML (mlGlMess150), (screenWidth - strlen (ML (mlGlMess150))) / 2, 0, 1);
	if (maintType [0] == 'S')
		rv_pr (ML (mlGlMess184), (screenWidth - strlen (ML (mlGlMess184))) / 2, 0, 1);

	
	switch (scn)
	{
	case 	1:
		box (0, 1, screenWidth, 3);
		break;

	case	2:
		if (ACTIVE (glic_rec.class_ok))
			boxSize++;
		if (ACTIVE (glic_rec.cat_ok))
			boxSize++;
		box (0, 1, screenWidth, boxSize);
		scn_set (SCN_MAIN);
		scn_write (SCN_MAIN);
		scn_display (SCN_MAIN);
		scn_set (SCN_DETAIL);
		break;

	case	3:
		line_at (1,0, screenWidth -2);
		scn_set (SCN_SUMMARY);
		scn_write (SCN_SUMMARY);
		break;
	}
	line_at (21,0, screenWidth -2);

	print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}


/*
 * Find account number, account description etc.
 */
int
FindGLAcct (
	char 	*accountNo, 
	int 	silent)
{
	GL_FormAccNo (accountNo, glmrRec.acc_no, 0);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL, accountNo);

	if (!strncmp (glmrRec.acc_no, "0000000000000000", MAXLEVEL))
	{
		if (!silent)
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
		}
		return (EXIT_FAILURE);
	}
	strcpy (glmrRec.co_no, comm_rec.co_no);

    cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
    if (cc)
	{
		if (!silent)
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
		}
		return (EXIT_FAILURE);
	}

	if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0] != 'P')
	{
		if (!silent)
		{
			errmess (ML (mlStdMess025));
			sleep (sleepTime);
			clear_mess ();
		}
		return (EXIT_FAILURE);
	}
	if (glmrRec.system_acc [0] == 'M')
	{
		if (!silent)
		{
			errmess (ML (mlGlMess183));
			sleep (sleepTime);
			clear_mess ();
		}
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Display Interface Code records. 
 */
void
DisplayIface (void)
{
	char	 disp_str [100];
	char	workString [2] [101];

	Dsp_open (75, 6, 9);

	strcpy (workString [0], "       Interface    ");
	strcpy (workString [1], "         Code       ");
	if (ACTIVE (glic_rec.class_ok))
	{
		if (envGlByClass)
		{
			strcat (workString [0], "|   Customer   ");
			strcat (workString [1], "|   Type       ");
		}
		else
		{
			strcat (workString [0], "|   Salesman   ");
			strcat (workString [1], "|     Code     ");
		}
	}
	if (ACTIVE (glic_rec.cat_ok))
	{
		strcat (workString [0], "|    Category       ");
		strcat (workString [1], "|      Code         ");
	}
	else
	{
		strcat (workString [0], "                    ");
		strcat (workString [1], "                    ");
	}
	if (!ACTIVE (glic_rec.class_ok))
	{
		strcat (workString [0], "               ");
		strcat (workString [1], "               ");
	}
	Dsp_saverec (workString [0]);
	Dsp_saverec (workString [1]);
	
	if (!ACTIVE (glic_rec.class_ok) && !ACTIVE (glic_rec.cat_ok))
		Dsp_saverec ("");
	else
		Dsp_saverec ("[NEXT] [PREV] [EDIT/END]");

	strcpy (glih2Rec.co_no, comm_rec.co_no);
	strcpy (glih2Rec.int_code, glihRec.int_code);
	strcpy (glih2Rec.cat_no, 		"            ");
	strcpy (glih2Rec.class_type, 		"   ");

	cc = find_rec (glih2, &glih2Rec, GTEQ, "r");
	while (!cc && !strcmp (glih2Rec.co_no, comm_rec.co_no) &&
           		  !strcmp (glih2Rec.int_code, glihRec.int_code))
	{
		if (ACTIVE (glic_rec.class_ok) && ACTIVE (glic_rec.cat_ok))
		{
			sprintf 
			(
				disp_str,
				"       %10.10s   |     %3.3s      |   %11.11s      ",
				glih2Rec.int_code,
				glih2Rec.class_type,
				glih2Rec.cat_no
			);
		}
		else if (ACTIVE (glic_rec.class_ok) && !ACTIVE (glic_rec.cat_ok))
		{
			sprintf 
			(
				disp_str,
				"       %10.10s   |     %3.3s      ",
				glih2Rec.int_code,
				glih2Rec.class_type
			);
		}
		else if (!ACTIVE (glic_rec.class_ok) && ACTIVE (glic_rec.cat_ok))
		{
			sprintf 
			(
				disp_str,
				"       %10.10s   |   %11.11s      ",
				glih2Rec.int_code,
				glih2Rec.cat_no
			);
		}
		else
		{
			sprintf 
			(
				disp_str,
				"                                  "
			);
		}
		
		Dsp_saverec (disp_str);

		cc = find_rec (glih2, &glih2Rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
}

/*
 * Load gljc records and add if required.
 */
void
LoadGlic (void)
{
	int	i;

	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (SCN_SUMMARY);
	lcount [SCN_SUMMARY] = 0;

	for (i = 0 ; strlen (gl_idesc [i]._ifcode); i++)
	{
		strcpy (glic_rec.co_no, comm_rec.co_no);
		sprintf (glic_rec.code, "%-10.10s", gl_idesc [i]._ifcode);

        cc = find_rec (glic, &glic_rec, EQUAL, "r");
		if (cc)
		{
			memset (&glic_rec, 0, sizeof glic_rec);
			strcpy (glic_rec.co_no, comm_rec.co_no);
			sprintf (glic_rec.code, "%-10.10s", gl_idesc [i]._ifcode);
			sprintf (glic_rec.desc, "%-40.40s", gl_idesc [i]._ifdesc);
			glic_rec.int_active	= gl_idesc [i]._active;
			glic_rec.cat_ok		= gl_idesc [i]._byCategory;
			glic_rec.class_ok	= gl_idesc [i]._byClassType;
			glic_rec.br_no_ok	= gl_idesc [i]._byBrNo;
			glic_rec.wh_no_ok	= gl_idesc [i]._byWhNo;
			strcpy (local_rec.accDesc," ");
			strcpy (local_rec.currCode," ");
			cc = abc_add (glic,&glic_rec);
		}
		else
		{
			cc = FindGLAcct (glic_rec.acct_no, TRUE);
			if (!cc)
			{
				strcpy (local_rec.accDesc, glmrRec.desc);
				strcpy (local_rec.currCode, glmrRec.curr_code);
			}
			else
			{
				strcpy (local_rec.accDesc," ");
				strcpy (local_rec.currCode," ");
			}
		}
		if (!glic_rec.int_active && maintType[0] != 'S')
			continue;
			
		store [lcount [SCN_SUMMARY]].ifActive		=	glic_rec.int_active;
		store [lcount [SCN_SUMMARY]].ifBrNoOK		=	glic_rec.cat_ok;
		store [lcount [SCN_SUMMARY]].ifWhNoOK		=	glic_rec.class_ok;
		store [lcount [SCN_SUMMARY]].ifCategoryOK	=	glic_rec.br_no_ok;
		store [lcount [SCN_SUMMARY]].ifClassSaleOK	=	glic_rec.wh_no_ok;
		strcpy (local_rec.ifActive,     IntToChar (glic_rec.int_active));
		strcpy (local_rec.ifCategoryOK, IntToChar (glic_rec.cat_ok));
		strcpy (local_rec.ifClassSaleOK,IntToChar (glic_rec.class_ok));
		strcpy (local_rec.ifBrNoOK, 	IntToChar (glic_rec.br_no_ok));
		strcpy (local_rec.ifWhNoOK, 	IntToChar (glic_rec.wh_no_ok));
		putval (lcount [SCN_SUMMARY]++);
		if (lcount [SCN_SUMMARY] > MAXLINES) 
            break;
	}
	vars [label ("intCode2")].row = lcount [SCN_SUMMARY];
	scn_set (SCN_SUMMARY);
	return;
}

/*
 * Update all files.
 */
void
UpdateGlic (void)
{
	int	i;

	clear ();
	print_at (0,0, ML (mlStdMess035));
	fflush (stdout);

	scn_set (SCN_SUMMARY);
	for (i = 0;i < lcount [SCN_SUMMARY];i++) 
	{
		getval (i);

		glic_rec.int_active	= CharToInt (local_rec.ifActive);
		glic_rec.cat_ok		= CharToInt (local_rec.ifCategoryOK);
		glic_rec.class_ok	= CharToInt (local_rec.ifClassSaleOK);
		glic_rec.br_no_ok	= CharToInt (local_rec.ifBrNoOK);
		glic_rec.wh_no_ok	= CharToInt (local_rec.ifWhNoOK);

		memcpy (&glic2_rec, &glic_rec, sizeof glic_rec);
		strcpy (glic_rec.co_no, comm_rec.co_no);
		sprintf (glic_rec.code, "%-10.10s", glic_rec.code);

		cc = find_rec (glic, &glic_rec, EQUAL, "u");
		if (cc)
		{
			cc = abc_add (glic,&glic_rec);
			if (cc)
				file_err (cc, glic, "DBADD");

			abc_unlock (glic);
		}
		else
		{
			memcpy (&glic_rec, &glic2_rec, sizeof glic2_rec);

            cc = abc_update (glic,&glic_rec);
			if (cc)
				file_err (cc, glic, "DBUPDATE");
		}
	}
}

void
tab_other (
	int		i)
{
	if (maintType [0] != 'S')
		return;

	FLD ("ifActive") 		= (INACTIVE (store [i].ifActive)) 		? NA : YES;
	FLD ("ifCategoryOK")	= (INACTIVE (store [i].ifCategoryOK))	? NA : YES;
	FLD ("ifClassSaleOK")	= (INACTIVE (store [i].ifClassSaleOK))	? NA : YES;
	FLD ("ifBrNoOK")		= (INACTIVE (store [i].ifBrNoOK))		? NA : YES;
	FLD ("ifWhNoOK")		= (INACTIVE (store [i].ifWhNoOK))		? NA : YES;
}
