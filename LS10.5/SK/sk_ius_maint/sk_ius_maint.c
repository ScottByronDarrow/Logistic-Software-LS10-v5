/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_ius_maint.c,v 5.6 2002/11/25 09:04:58 kaarlo Exp $
|  Program Name  : (sk_ius_maint.c)                                  
|  Program Desc  : (Maintain Inventory User Secifications)
|---------------------------------------------------------------------|
|  Date Written  : (04/06/1998)    | Author      : Scitt B Darrow     |
|---------------------------------------------------------------------|
| $Log: sk_ius_maint.c,v $
| Revision 5.6  2002/11/25 09:04:58  kaarlo
| .
|
| Revision 5.5  2002/11/25 07:52:41  kaarlo
| SC0036. Updated to fix display error and F4 error on "uspec_no".
|
| Revision 5.4  2002/07/25 11:17:36  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.3  2002/07/24 08:39:14  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.2  2002/06/26 05:48:49  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.1  2002/06/25 06:16:28  scott
| Updated for problems on GUI with two screens on one.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_ius_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ius_maint/sk_ius_maint.c,v 5.6 2002/11/25 09:04:58 kaarlo Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	TYPE	 	 (inputType [0] == 'T')
#define	CAT_ITEM	 (inputType [0] == 'I')
#define	CODE		 (inputType [0] == 'C')

#define	UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	DEFAULT	99

	/*
	 * Special fields and flags.
	 */
	char	inputType [2];

	char	badFileName [5];

   	int		newCode		= FALSE,
			newCatItem	= FALSE, 
			NEXT_SPEC	= TRUE;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;
struct iudsRecord	iuds_rec;
struct iudcRecord	iudc_rec;
struct iudiRecord	iudi_rec;

	struct	storeRec	{
		long	hhcf_hash;
		long	hhbr_hash;
		int		spec_type;
		char	code [3];
	} store [MAXLINES];	
		
	char	*data = "data";

	int		MaxUserCodes	=	10;

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
		  "" },
		{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
		  "" },
		{ ENDMENU }
	};

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	/*--------------------
	| Specification Type |
	--------------------*/
	{3, LIN, "tspec_no",	 3, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Specification No   ", "Enter <return> for next new specification No or [SEARCH] for existing.",
		 NE, NO, JUSTRIGHT, "", "", (char *)&iuds_rec.spec_no},
	{3, LIN, "tspec_desc",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description.       ", " ",
		YES, NO,  JUSTLEFT, "", "", iuds_rec.spec_desc},
	/*--------------------
	| Specification Type |
	--------------------*/
	{3, LIN, "cspec_no",	 3, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Specification No   ", "Enter Specification No [SEARCH]",
		 NE, NO, JUSTRIGHT, "", "", (char *)&iudc_rec.spec_no},
	{3, LIN, "cspec_desc",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.       ", " ",
		NA, NO,  JUSTLEFT, "", "", iuds_rec.spec_desc},
	{3, LIN, "cspec_code",	 6, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Specification Code ", "Enter Specification Code [SEARCH]",
		 NO, NO, JUSTLEFT, "", "", iudc_rec.code},
	{3, LIN, "cspec_cdesc",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Code Description.  ", " ",
		YES, NO,  JUSTLEFT, "", "", iudc_rec.desc},
	/*------------------------
	| Category / Item Input. |
	------------------------*/
	{4, LIN, "cat_no",	4, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Category         ", "Use Search key for valid categories.",
		NO, NO,  JUSTLEFT, "", "", excf_rec.cat_no},
	{4, LIN, "cat_desc",	4, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", excf_rec.cat_desc},
	{4, LIN, "item_no", 5, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number      ", " ", 
		YES, NO, JUSTLEFT, "", "", inmr_rec.item_no}, 
	{4, LIN, "item_desc", 5, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	/*-----------------------------
	| Category / Item Allocation. |
	-----------------------------*/
	{5, TAB, "uspec_no",	MAXLINES, 0, INTTYPE,
		"NN", "          ",
		" ", " ", "No", "Enter User defined prompt number.",
		YES, NO, JUSTLEFT, "", "", (char *)&iuds_rec.spec_no},
	{5, TAB, "utype",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "User Defined Prompt", "",
		NA, NO, JUSTLEFT, "", "", iuds_rec.spec_desc},
	{5, TAB, "ucode",	0, 1, CHARTYPE,
		"UU", "          ",
		" ", " ", "Code", "Enter user defined code [SEARCH]",
		NO, NO, JUSTLEFT, "", "", iudc_rec.code},
	{5, TAB, "udesc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " User Defined Code Description.         ", "",
		 NA, NO,  JUSTLEFT, "", "", iudc_rec.desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

extern	int	TruePosition;

/*=======================
| Function Declarations |
=======================*/
int  	CheckUcode 		 (long, int, int);
int  	DeleteLine 		 (void);
int  	GetSpecs 		 (long, long);
int  	IudcDelOk 		 (void);
int  	IudsDelOk 		 (void);
int  	heading 		 (int);
int  	spec_valid 		 (int);
void 	CloseDB 		 (void);
void 	OpenDB 			 (void);
void 	SrchExcf 		 (char *);
void 	SrchInds 		 (char *);
void 	SrchIudc 		 (char *);
void 	Update 			 (void);
void 	Updateiudc 		 (void);
void 	Updateiudi 		 (void);
void 	Updateiuds 		 (void);
void 	shutdown_prog 	 (void);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	int		i;

	TruePosition	=	TRUE;

	tab_row	=	5;
	tab_col	=	6;

	if (argc != 2)
	{
		print_at (0,0, "Usage: %s < T (ype) C (ode) I (nput)>\007\n\r",argv [0]);
		return (EXIT_FAILURE);
	}

	/*----------------------------------------------
	| Check for advertising levy/ freight charges. |
	----------------------------------------------*/
	sptr = chk_env ("SK_MAX_UD_CODES");
	MaxUserCodes = (sptr == (char *)0) ? 10 : atoi (sptr);

	/*----------------
	| Printer Number |
	----------------*/
	sprintf (inputType,"%-1.1s", argv [1]);

	if (!TYPE && !CODE && !CAT_ITEM)
	{
		print_at (0,0, "Usage: %s < T (ype) C (ode) I (nput)>\007\n\r",argv [0]);
		return (EXIT_FAILURE);
	}

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	if (TYPE)
	{
		for (i = label ("tspec_no"); i <= label ("tspec_desc"); i++)
			vars [i].scn = 1;
	}

	if (CODE)
	{
		for (i = label ("cspec_no"); i <= label ("cspec_cdesc"); i++)
			vars [i].scn = 1;
	}

	if (CAT_ITEM)
	{
		for (i = label ("cat_no"); i <= label ("item_desc"); i++)
			vars [i].scn = 1;

		for (i = label ("uspec_no"); i <= label ("udesc"); i++)
			vars [i].scn = 2;
	}


	init_scr ();
	set_tty ();
	set_masks ();

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (5, store, sizeof (struct storeRec));
#endif
	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		if (CAT_ITEM)
		{
			lcount [2] 	= 0;
			init_vars (1);	
			init_vars (2);	
		}
		else
			init_vars (1);	

		heading (1);
		if (TYPE)
		{
			char	nextSpecStr [16];

			strcpy (iuds_rec.co_no,comm_rec.co_no);
			iuds_rec.spec_no = 0;
			NEXT_SPEC = 1;
			cc = find_rec ("iuds",&iuds_rec,GTEQ,"r");
		
			while (!cc && !strcmp (iuds_rec.co_no,comm_rec.co_no)
				&& iuds_rec.spec_no == NEXT_SPEC)
			{
				if (NEXT_SPEC < MaxUserCodes)
					NEXT_SPEC++;	
				cc = find_rec ("iuds",&iuds_rec,NEXT,"r");
			}

			sprintf (nextSpecStr, "%hd",NEXT_SPEC);
			vars [label ("tspec_no")].highval = p_strsave (nextSpecStr);
		}
		entry (1);
		if (prog_exit || restart)
			continue;

		if (CAT_ITEM)
		{
			heading (2);
			
			scn_display (2);

			if (newCatItem == TRUE) 
				entry (2);
			else
				edit (2);
		}
		else
		{
			/*------------------------------
			| Edit screen 1 linear input . |	
			------------------------------*/
			heading (1);
			scn_display (1);
			edit (1);
		}

		if (restart)
			continue;

		if (!restart)
			Update ();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (iudi, iudi_list, IUDI_NO_FIELDS, "iudi_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (iuds, iuds_list, IUDS_NO_FIELDS, "iuds_id_no");
	open_rec (iudc, iudc_list, IUDC_NO_FIELDS, "iudc_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (iuds);
	abc_fclose (iudc);
	abc_fclose (iudi);
	abc_fclose (inmr);
	abc_fclose (excf);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*----------------------------------
	| Validate Specification Type      |
	----------------------------------*/
	if (LCHECK ("tspec_no"))
	{
		if (iuds_rec.spec_no > MaxUserCodes)
		{
			print_mess (ML ("Specification number is greater than system defined value."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		 
		if (SRCH_KEY)
		{
			SrchInds (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used) 
		{
			if (NEXT_SPEC < MaxUserCodes)
			{
				char	nextSpecStr [16];

				iuds_rec.spec_no = NEXT_SPEC++;
				newCode = 1;

				sprintf (nextSpecStr, "%hd",NEXT_SPEC);
				vars [label ("tspec_no")].highval = p_strsave (nextSpecStr);

				strcpy (iuds_rec.spec_desc," ");
			}
		}
		else    
		{
			strcpy (iuds_rec.co_no,comm_rec.co_no);
			iuds_rec.spec_no = atoi (temp_str);
			cc = find_rec ("iuds", &iuds_rec, COMPARISON, (TYPE) ? "u" : "r");
			if (cc)
			{
				if (TYPE)
					abc_unlock ("iuds");
				print_mess (ML ("Specification number not found."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			newCode		= 0;
			entry_exit	= TRUE;
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate Code Specification Type |
	----------------------------------*/
	if (LCHECK ("cspec_no"))
	{
		if (iudc_rec.spec_no > MaxUserCodes)
		{
			print_mess (ML ("Specification number is greater than system defined value."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		 
		if (SRCH_KEY)
		{
			SrchInds (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (iuds_rec.co_no,comm_rec.co_no);
		iuds_rec.spec_no = atoi (temp_str);
		cc = find_rec ("iuds", &iuds_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess (ML ("Specification type not found."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("cspec_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| validate Specification Code |
	-----------------------------*/
	if (LCHECK ("cspec_code"))
	{
		if (SRCH_KEY)
		{
			SrchIudc (temp_str);
			return (EXIT_SUCCESS);
		}

		newCode = 0;
		if (!strncmp (iudc_rec.code, "  ", strlen (iudc_rec.code))) 
		{
			print_mess (ML ("Specification code must be entered."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (iudc_rec.co_no,comm_rec.co_no);
		iudc_rec.spec_no = iuds_rec.spec_no;
		cc = find_rec ("iudc", &iudc_rec, COMPARISON, "u");
		if (cc) 
		{
			newCode = 1;
			strcpy (iudc_rec.desc,"");
		}
		else    
			entry_exit = 1;

		DSP_FLD ("cspec_cdesc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("item_no") && (FLD ("item_no")!= NA))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();

		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML ("Item is not on file."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		DSP_FLD ("item_no");

		cc = GetSpecs (0L, inmr_rec.hhbr_hash);
		if (!cc)
		{
			entry_exit 	= TRUE;
			newCatItem	= FALSE;
		}
		else
			newCatItem	= TRUE;
			
		return (EXIT_SUCCESS);
	}
	
	/*--------------------
	| Validate Category. |
	--------------------*/
	if (LCHECK ("cat_no"))
	{
		if (dflt_used)
		{
			excf_rec.hhcf_hash	=	0L;
			FLD ("item_no")	=	YES;
			return (EXIT_SUCCESS);
		}
		
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no, comm_rec.co_no);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("cat_desc");

		cc = GetSpecs (excf_rec.hhcf_hash, 0L);
		if (!cc)
		{
			entry_exit 	= TRUE;
			newCatItem	= FALSE;
		}
		else
			newCatItem	= TRUE;

		FLD ("item_no")	=	NA;
		inmr_rec.hhbr_hash	=	0L;
		return (EXIT_SUCCESS);
	}
	
	/*----------------------------------
	| Validate Specification Type      |
	----------------------------------*/
	if (LCHECK ("uspec_no"))
	{
		if (last_char == DELLINE || dflt_used)
			return (DeleteLine ());

		if (SRCH_KEY)
		{
			SrchInds (temp_str);
			return (EXIT_SUCCESS);
		}

		if (iuds_rec.spec_no > MaxUserCodes)
		{
			print_mess (ML ("Specification number is greater than system defined value."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (CheckUcode (inmr_rec.hhbr_hash, iuds_rec.spec_no, line_cnt))
		{
			print_mess (ML ("Specification number already used for item."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		 
		strcpy (iuds_rec.co_no,comm_rec.co_no);
		cc = find_rec ("iuds", &iuds_rec, COMPARISON, "r");
		if (cc)
		{
			abc_unlock ("iuds");
			print_mess (ML ("Specification number not found."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

        if (prog_status != ENTRY)
		{
			DSP_FLD ("uspec_no");
			do
			{
				get_entry (label ("ucode"));
				cc = spec_valid (label ("ucode"));
			} while (cc && !restart);
		}

		DSP_FLD ("utype");

		return (EXIT_SUCCESS);
	}
	
	/*--------------------
	| Validate User Code |
	--------------------*/
	if (LCHECK ("ucode"))
	{
		if (SRCH_KEY)
		{
			SrchIudc (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (iudc_rec.co_no,comm_rec.co_no);
		iudc_rec.spec_no = iuds_rec.spec_no;
		cc = find_rec ("iudc", &iudc_rec, COMPARISON,"r");
		if (cc)
		{
			abc_unlock ("iudc");
			errmess (ML ("Code is not on file."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("udesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==============================================
| Read detail lines from note-pad detail file. |
==============================================*/
int
GetSpecs (
 long	hhcf_hash,
 long	hhbr_hash)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;

	iudi_rec.hhbr_hash	=	hhbr_hash;
	iudi_rec.hhcf_hash	=	hhcf_hash;
	iudi_rec.spec_no	=	0;
	strcpy (iudi_rec.code, "  ");

	cc = find_rec ("iudi", &iudi_rec, GTEQ, "r");
	while (!cc)
	{
		if (hhbr_hash > 0L && iudi_rec.hhbr_hash != hhbr_hash)
		{
			cc = find_rec ("iudi", &iudi_rec, NEXT, "r");
			continue;
		}
		if (hhcf_hash > 0L && iudi_rec.hhcf_hash != hhcf_hash)
		{
			cc = find_rec ("iudi", &iudi_rec, NEXT, "r");
			continue;
		}
		strcpy (iudc_rec.co_no,comm_rec.co_no);
		iudc_rec.spec_no = iudi_rec.spec_no;
		strcpy (iudc_rec.code, iudi_rec.code);
		cc = find_rec ("iudc", &iudc_rec, COMPARISON,"r");
		if (cc)
		{
			cc = find_rec ("iudi", &iudi_rec, NEXT, "r");
			continue;
		}
		strcpy (iuds_rec.co_no,comm_rec.co_no);
		iuds_rec.spec_no = iudi_rec.spec_no;
		cc = find_rec ("iuds", &iuds_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec ("iudi", &iudi_rec, NEXT, "r");
			continue;
		}

		store [lcount [2]].hhbr_hash = iudi_rec.hhbr_hash;
		store [lcount [2]].spec_type = iuds_rec.spec_no;

		putval (lcount [2]++);

		cc = find_rec ("iudi", &iudi_rec, NEXT, "r");
	}
	scn_set (1);

	/*---------------------
	| No entries to edit. |
	---------------------*/
	if (lcount [2] == 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*==================
| Updated records. |
==================*/
void
Update (
 void)
{
	if (TYPE)
		Updateiuds ();

	if (CODE)
		Updateiudc ();

	if (CAT_ITEM)
		Updateiudi ();
}

/*=======================================================
|
=======================================================*/
int
CheckUcode (
 long	hhbr_hash,
 int	ucode,
 int	line_no)
{
	int		i;
	int		no_items = (prog_status == ENTRY) ? line_cnt : lcount [1];

	for (i = 0;i < no_items;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*---------------------------------
		---------------------------------*/
		if (store [i].hhbr_hash == hhbr_hash &&
			store [i].spec_type == ucode)
				return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
/*=============================
| Add or Update iuds record . |
=============================*/
void
Updateiuds (
 void)
{
	int		exitLoop;

	strcpy (iuds_rec.co_no, comm_rec.co_no);
	if (newCode)
	{
		char	nextSpecStr [16];

		cc = abc_add (iuds, &iuds_rec);
		if (cc) 
			file_err (cc, iuds, "DBADD");

		if (NEXT_SPEC < MaxUserCodes)
			NEXT_SPEC++;	

		sprintf (nextSpecStr, "%hd",NEXT_SPEC);
		vars [label ("tspec_no")].highval = p_strsave (nextSpecStr);
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case DEFAULT :
			case UPDATE :
				cc = abc_update (iuds, &iuds_rec);
				if (cc) 
					file_err (cc, iuds, "DBUPDATE");

				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (iuds);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (IudsDelOk ())
				{
					clear_mess ();
					cc = abc_delete (iuds);
					if (cc)
						file_err (cc, iuds, "DBUPDATE");
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	abc_unlock (iuds);
}

/*=============================
| Add or update iudc record . |
=============================*/
void
Updateiudc (
 void)
{
	int		exitLoop;

	if (newCode)
	{
		cc = abc_add (iudc, &iudc_rec);
		if (cc) 
			file_err (cc, iudc, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case DEFAULT :
			case UPDATE :
				cc = abc_update (iudc, &iudc_rec);
				if (cc) 
					file_err (cc, iudc, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (iudc);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (IudcDelOk ())
				{
					clear_mess ();
					cc = abc_delete (iudc);
					if (cc)
						file_err (cc, iudc, "DBUPDATE");
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	abc_unlock (iudc);
}

/*===========================
| Check whether it is OK to |
| delete the iuds record.   |
| Files checked are :       |
===========================*/
int
IudsDelOk (
 void)
{
	/*-------------
	| Currently no checking
	-------------*/
    return (EXIT_FAILURE);
}
/*===========================
| Check whether it is OK to |
| delete the iudc record.   |
| Files checked are :       |
===========================*/
int
IudcDelOk (
 void)
{
	/*-------------
	| Currently no checking
	-------------*/
    return (EXIT_FAILURE);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char *key_val)
{
	_work_open (11,0,40);
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	save_rec ("#Cat No", "#Category Description");
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no, key_val, strlen (key_val)) &&
		      !strcmp (excf_rec.co_no, comm_rec.co_no))
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
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, excf, "DBFIND");
}

/*=======================================
| Item user defined specification code. |
=======================================*/
void
SrchIudc (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Code Description");
	strcpy (iudc_rec.co_no,comm_rec.co_no);
	iudc_rec.spec_no = iuds_rec.spec_no;
	strcpy (iudc_rec.code,key_val);
	cc = find_rec ("iudc", &iudc_rec, GTEQ, "r");

	while (!cc 
	&&     !strcmp (iudc_rec.co_no,comm_rec.co_no) 
	&&     iudc_rec.spec_no == iuds_rec.spec_no 
	&&     !strncmp (iudc_rec.code,key_val,strlen (key_val)))
	{
		cc = save_rec (iudc_rec.code,iudc_rec.desc);
		if (cc)
			break;

		cc = find_rec ("iudc",&iudc_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (iudc_rec.co_no,comm_rec.co_no);
	iudc_rec.spec_no = iuds_rec.spec_no;
	sprintf (iudc_rec.code,"%-2.2s",temp_str);
	cc = find_rec ("iudc",&iudc_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "iudc", "DBFIND");
}

void
Updateiudi (
 void)
{
	iudi_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	iudi_rec.hhcf_hash	=	excf_rec.hhcf_hash;
	iudi_rec.spec_no	=	0;
	strcpy (iudi_rec.code, "  ");
	cc = find_rec (iudi, &iudi_rec, GTEQ, "u");
	while (!cc && 
		iudi_rec.hhbr_hash	==	inmr_rec.hhbr_hash &&
		iudi_rec.hhcf_hash	==	excf_rec.hhcf_hash)
	{
		abc_delete ("iudi");
		iudi_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		iudi_rec.hhcf_hash	=	excf_rec.hhcf_hash;
		iudi_rec.spec_no	=	0;
		strcpy (iudi_rec.code, "  ");
		cc = find_rec (iudi, &iudi_rec, GTEQ, "u");
	}			
	abc_unlock ("iudi");

	/*------------------------
	| Set screen SCN_UDEFINE |
	------------------------*/
	scn_set (2);

	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);
		iudi_rec.spec_no	=	iuds_rec.spec_no;
		strcpy (iudi_rec.code, iudc_rec.code);
		iudi_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		iudi_rec.hhcf_hash	=	excf_rec.hhcf_hash;
		cc = abc_add (iudi, &iudi_rec);
		if (cc)
			file_err (cc, iudi, "DBADD");

	}
}

void
SrchInds (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Spec Type Description");
	strcpy (iuds_rec.co_no,comm_rec.co_no);
	iuds_rec.spec_no = atoi (key_val);
	cc = find_rec ("iuds",&iuds_rec,GTEQ,"r");

	while (!cc && !strcmp (iuds_rec.co_no,comm_rec.co_no))
	{
    	sprintf (err_str,"%2d",iuds_rec.spec_no);
		cc = save_rec (err_str,iuds_rec.spec_desc);
		if (cc)
			break;

		cc = find_rec ("iuds",&iuds_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (iuds_rec.co_no,comm_rec.co_no);
	iuds_rec.spec_no = atoi (temp_str);
	cc = find_rec ("iuds",&iuds_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "iuds", "DBFIND");
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	if (TYPE)
	{
		rv_pr (ML (" Specification Type Maintenance "), 20,0,1);
		move (0,1);
		line (80);
		box (0,2,80,2);

	}

	if (CODE)
	{
		rv_pr (ML (" Specification Code Maintenance "), 20,0,1);
		move (0,1);
		line (80);

		box (0,2,80,5);
		move (1,5);
		line (79);
	}
	if (CAT_ITEM)
	{
		rv_pr (ML (" Item Specification Allocation Maintenance "), 20,0,1);
		line_at (1,0,80);

		if (scn	== 1)
			box (0,3,80,2);
		else
		{
			sprintf (err_str, "Category : %-16.16s - %s", excf_rec.cat_no,
		 											excf_rec.cat_desc);
			rv_pr (err_str, 1,2,1);

			sprintf (err_str, "Item No  : %-16.16s - %s", inmr_rec.item_no,
		 											inmr_rec.description);
			rv_pr (err_str, 1,3,1);
		}
/*
		if (scn	== 1)
		{
			scn_set (2);
			scn_write (2);
			scn_display (2);
		}
		else
		{
			scn_set (1);
			scn_write (1);
			scn_display (1);
		}
*/
	}
	scn_set (scn);
	line_at (20,0,80);
	print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{                          
    int i;           
	int this_page = line_cnt / TABLINES;
	/*-------         
	| entry |        
	-------*/          
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);       
	}            
	/*--------------------
	| no lines to delete |
	--------------------*/
	if (lcount [2] <= 0)
	{
		print_mess (ML (mlStdMess032));
		return (EXIT_FAILURE);
	}
	/*--------------
	| delete lines |
	--------------*/
	lcount [2]--;

	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		if (line_cnt / TABLINES == this_page)
		line_display ();
	}

	/*-------------------------------
	| blank last line - if required |
	-------------------------------*/
	if (line_cnt / TABLINES == this_page)
		blank_display ();
	/*---------------------------
	| zap buffer if deleted all |
	---------------------------*/
	if (lcount [2] <= 0)
	{
		init_vars (2);
		putval (i);
	}
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}
