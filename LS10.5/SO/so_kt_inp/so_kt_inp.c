/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (so_kt_inp.c  )                                    |
|  Program Desc  : (Kitting Specification Maintenance.          )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, sokt,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sokt,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 04/07/91         |
|---------------------------------------------------------------------|
|  Date Modified : (04/07/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (17/12/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (18/11/93)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (11/09/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : (17/12/91) - Add bonus flag to sokt.               |
|                :                                                    |
|   (18/11/93)   : HGP 9501 - Fix bonus flag input.                   |
|   (11/09/97)   : Updated for Multilingual Conversion.               |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_kt_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_kt_inp/so_kt_inp.c,v 5.5 2002/07/24 08:39:28 scott Exp $";

#define		KITTING		 (inmr_rec.inmr_class [0] == 'K')
#define		PHANTOM		 (inmr_rec.inmr_class [0] == 'P')

#define		KIT		 (programRunType [0] == 'K')
#define		BOM		 (programRunType [0] == 'B')

#define		TOTSCNS		2
#define 	MAXLINES	100
#define 	MAXWIDTH	150
#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>
#include 	<minimenu.h>

#define		LSL_UPDATE		0
#define		LSL_IGNORE		1
#define		LSL_DELETE		2
#define		LSL_DEFAULT		99

#include	"schema"

struct commRecord		comm_rec;
struct inmrRecord		inmr_rec;
struct soktRecord		sokt_rec;
struct soktRecord		sokt2_rec;
struct posdtupRecord	posdtup_rec;

   	int  	newItem			= 0;
   	int  	PosInstalled	= 0;

	char	programRunType [2];

	struct	storeRec {
		long	hhbrHash;
	} store [MAXLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	long	hhbr_hash;
	char	item_no [17];
	char	desc [41];
	char	matl_no [17];
	char	matl_desc [37];
	float	matl_qty;
	long	due_date;
	long	defaultDate;
	char	bonus [2];
	char	item_n_desc [17];
	char	item_d_desc [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "item_no",	 3, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "date",	 5, 15, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Due Date", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.defaultDate},
	{2, TAB, "matl_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item Number.  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.matl_no},
	{2, TAB, "matl_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " I t e m     D e s c r i p t i o n. ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.matl_desc},
	{2, TAB, "matl_qty",	 0, 1, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "   Qty.   ", " ",
		YES, NO, JUSTRIGHT, "0.00", "99999.99", (char *)&local_rec.matl_qty},
	{2, TAB, "due_date",	 0, 1, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "  Due Date  ", " ",
		 NI, NO, JUSTRIGHT, "", "", (char *)&local_rec.due_date},
	{2, TAB, "bonus",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "N", "B", " Bonus Item ",
		 NO, NO, JUSTRIGHT, "YN", "", local_rec.bonus},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
int  	DeleteLine 		(void);
void 	SetHhbrHash 	(void);
void 	LoadDetails 	(void);
void 	update_menu 	(void);
void 	AddKitItem 		(void);
void 	UpdateKit 		(void);
void 	DeleteKit 		(void);
void 	UpdatePosdt 	(void);
void 	Update 			(void);
int  	heading 		(int);

extern	int	kitPhantomSrch;

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc < 2)
	{
		print_at (0,0,mlSoMess724, argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (programRunType, "%-1.1s", argv [1]);

	if (!KIT && !BOM)
	{
		print_at (0,0,mlSoMess724, argv [0]);
		return (EXIT_FAILURE);
	}
	/*------------------------------
	| Check for POS installation.  |
	------------------------------*/
	sptr = chk_env ("POS_INSTALLED");
	PosInstalled = (sptr == (char *)0) ? 0 : atoi (sptr);
		
	SETUP_SCR (vars);


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

	OpenDB ();

	FLD ("date") 	  = (BOM) ? ND : YES;
	FLD ("due_date")  = (BOM) ? ND : NI;
	FLD ("bonus")     = (BOM) ? ND : NO;

	tab_row = (BOM) ? 6 : 7;
	tab_col = (BOM) ? 8 : 0;

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		lcount [2] 	= 0;

		sprintf (local_rec.desc, "%40.40s", " ");
		local_rec.defaultDate = 0L;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		scn_display (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		scn_write (1);
		scn_display (1);
		scn_write (2);
		scn_display (2);

		if (newItem)
			entry (2);
		else
			edit (2);
	
		if (restart)
			continue;

		Update ();

		if (PosInstalled)
	        UpdatePosdt ();	

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
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
 	abc_alias ("sokt2","sokt");

	open_rec ("sokt2",sokt_list, SOKT_NO_FIELDS, "sokt_id_no_2");
	open_rec ("sokt", sokt_list, SOKT_NO_FIELDS, "sokt_id_no");
	open_rec ("inmr", inmr_list, INMR_NO_FIELDS, "inmr_id_no");

	if (PosInstalled)
		open_rec (posdtup, posdtup_list, POSDTUP_NO_FIELDS, "pos_no1");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose ("sokt2");
	abc_fclose ("sokt");
	abc_fclose ("inmr");
	if (PosInstalled)
		abc_fclose (posdtup);

	SearchFindClose ();
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			kitPhantomSrch	=	TRUE;
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();
		
		strcpy (local_rec.desc,inmr_rec.description);
		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		if (KIT && !KITTING)
		{
			print_mess (ML (mlStdMess159));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (BOM && !PHANTOM)
		{
			print_mess (ML (mlStdMess160));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (sokt_rec.co_no,comm_rec.co_no);
		local_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
		sokt_rec.hhbr_hash 		= inmr_rec.hhbr_hash;
		sokt_rec.line_no 		= 0;
		cc = find_rec ("sokt",&sokt_rec,GTEQ,"r");	
		if (cc || strcmp (sokt_rec.co_no,comm_rec.co_no) || 
			sokt_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			newItem = TRUE;
			SetHhbrHash ();
		}
		else
		{
			newItem = FALSE;
			LoadDetails ();
			entry_exit = 1;
		}
		DSP_FLD ("item_no");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("matl_no"))
	{
		if (dflt_used)
			return (DeleteLine ());

		if (dflt_used)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			kitPhantomSrch	=	FALSE;
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}


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
		
		sprintf (local_rec.matl_desc,"%-36.36s",inmr_rec.description);
		DSP_FLD ("matl_no");
		DSP_FLD ("matl_desc");

		if (KITTING || PHANTOM)
		{
			sprintf (err_str, ML (mlStdMess161),
					 (KITTING) ? ML ("Kit") : ML ("BOM/Phantom"),
					 (KIT) ? ML ("Kit") : ML ("BOM."));

			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (inmr_rec.hhbr_hash == local_rec.hhbr_hash)
		{
			print_mess (ML (mlSoMess013));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (sokt2_rec.co_no,comm_rec.co_no);
		sokt2_rec.mabr_hash = local_rec.hhbr_hash;
		sokt2_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec ("sokt2",&sokt2_rec,COMPARISON,"r");	
		if (!cc)
		{
			sprintf (err_str, ML (mlStdMess162),
					 (KIT) ? ML ("Kit") : ML ("BOM."));

			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		store [line_cnt].hhbrHash = inmr_rec.hhbr_hash;
		sprintf (local_rec.matl_desc,"%-36.36s",inmr_rec.description);
		sprintf (local_rec.matl_no, "%-16.16s",inmr_rec.item_no);
		local_rec.due_date = local_rec.defaultDate;
	
		DSP_FLD ("matl_no");
		DSP_FLD ("matl_desc");
		DSP_FLD ("matl_qty");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	/*-------
	| entry	|
	-------*/
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}
	/*-------------------------------
	| no lines to delete			|
	-------------------------------*/
	if (lcount [2] <= 0)
	{
		print_mess (ML (mlStdMess032));
		return (EXIT_FAILURE);
	}
	/*---------------------------
	| delete lines				|
	---------------------------*/
	lcount [2]--;
	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		store [line_cnt].hhbrHash = store [line_cnt + 1].hhbrHash;
		if (line_cnt / TABLINES == this_page)
			line_display ();
	}
	/*-----------------------------------
	| blank last line - if required		|
	-----------------------------------*/
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	/*-------------------------------
	| zap buffer if deleted all		|
	-------------------------------*/
	if (lcount [2] <= 0)
	{
		init_vars (2);
		putval (i);
	}
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
SetHhbrHash (void)
{
	int	i;
	for (i = 0;i < MAXLINES;i++)
		store [i].hhbrHash = 0L;
}

void
LoadDetails (void)
{
	init_vars (2);

	SetHhbrHash ();
	abc_selfield ("inmr","inmr_hhbr_hash");
	lcount [2] = 0;
	strcpy (sokt_rec.co_no,comm_rec.co_no);
	sokt_rec.hhbr_hash = local_rec.hhbr_hash;
	sokt_rec.line_no = 0;
	cc = find_rec ("sokt",&sokt_rec,GTEQ,"u");	
	while (!cc && !strcmp (sokt_rec.co_no,comm_rec.co_no) && 
			sokt_rec.hhbr_hash == local_rec.hhbr_hash)
	{
		cc = find_hash ("inmr",&inmr_rec,COMPARISON,"r",sokt_rec.mabr_hash);
		if (!cc)
		{
			store [lcount [2]].hhbrHash = sokt_rec.mabr_hash;
			strcpy (local_rec.matl_no,inmr_rec.item_no);
			sprintf (local_rec.matl_desc,"%-36.36s", inmr_rec.description);
			local_rec.matl_qty = sokt_rec.matl_qty;
			local_rec.due_date = sokt_rec.due_date;
			strcpy (local_rec.bonus, (sokt_rec.bonus [0] == 'Y') ? "Y" : "N");
			putval (lcount [2]++);
		}
		cc = find_rec ("sokt",&sokt_rec,NEXT,"u");	
	}
	abc_selfield ("inmr","inmr_id_no");
	scn_set (1);
}

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD WITH CHANGES.       ",
		  "" },
		{ " 2. IGNORE CHANGES MADE TO RECORD.    ",
		  "" },
		{ " 3. DELETE CURRENTLY MODIFIED RECORD. ",
		  "" },
		{ ENDMENU }
	};

/*===================
| Update mini menu. |
===================*/
void
update_menu (void)
{
	for (;;)
	{
	    mmenu_print ("    U P D A T E    S E L E C T I O N.   ",upd_menu,0);
	    switch (mmenu_select (upd_menu))
	    {
		case	LSL_DEFAULT:
		case 	LSL_UPDATE :
			UpdateKit ();
			return;

		case 	LSL_IGNORE :
			return;

		case 	LSL_DELETE :
			DeleteKit ();
			return;
			break;

		default :
			break;
	    }
	}
}

/*==============================
| Add sales order kit records. |
==============================*/
void
AddKitItem (void)
{
	scn_set (2);
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);

		strcpy (sokt_rec.co_no,comm_rec.co_no);
		sokt_rec.hhbr_hash = local_rec.hhbr_hash;
		sokt_rec.line_no = line_cnt;

		putchar ('A');
		fflush (stdout);
		strcpy (sokt_rec.co_no,comm_rec.co_no);
		sokt_rec.hhbr_hash = local_rec.hhbr_hash;
		sokt_rec.line_no = line_cnt;
		sokt_rec.mabr_hash = store [line_cnt].hhbrHash;
		sokt_rec.matl_qty = local_rec.matl_qty;
		sokt_rec.due_date = local_rec.due_date;
		strcpy (sokt_rec.bonus, local_rec.bonus);
		cc = abc_add ("sokt",&sokt_rec);
		if (cc)
		       sys_err ("Error in sokt During (DBADD)",cc,PNAME);
	}
}

/*=================================
| Update sales order kit records. |
=================================*/
void
UpdateKit (void)
{
	scn_set (2);
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);

		strcpy (sokt_rec.co_no,comm_rec.co_no);
		sokt_rec.hhbr_hash = local_rec.hhbr_hash;
		sokt_rec.line_no = line_cnt;
		cc = find_rec ("sokt",&sokt_rec,COMPARISON,"u");	
		if (cc)
		{
			putchar ('A');
			fflush (stdout);
			strcpy (sokt_rec.co_no,comm_rec.co_no);
			sokt_rec.hhbr_hash = local_rec.hhbr_hash;
			sokt_rec.line_no = line_cnt;
			sokt_rec.mabr_hash = store [line_cnt].hhbrHash;
			sokt_rec.matl_qty = local_rec.matl_qty;
			sokt_rec.due_date = local_rec.due_date;
			strcpy (sokt_rec.bonus, local_rec.bonus);
			cc = abc_add ("sokt",&sokt_rec);
			if (cc)
		       		file_err (cc, "sokt", "DBADD");

			abc_unlock ("sokt");
		}
		else
		{
			putchar ('U');
			fflush (stdout);
			sokt_rec.mabr_hash = store [line_cnt].hhbrHash;
			sokt_rec.matl_qty = local_rec.matl_qty;
			sokt_rec.due_date = local_rec.due_date;
			strcpy (sokt_rec.bonus, local_rec.bonus);
			cc = abc_update ("sokt",&sokt_rec);
			if (cc)
		       		file_err (cc, "sokt", "DBUPDATE");
		}
	}

	strcpy (sokt_rec.co_no,comm_rec.co_no);
	sokt_rec.hhbr_hash = local_rec.hhbr_hash;
	sokt_rec.line_no = lcount [2];
	cc = find_rec ("sokt",&sokt_rec,GTEQ,"r");	
	while (!cc && !strcmp (sokt_rec.co_no,comm_rec.co_no) && 
			sokt_rec.hhbr_hash == local_rec.hhbr_hash)
	{
		putchar ('D');
		fflush (stdout);
		cc = abc_delete ("sokt");
		if (cc)
		       	file_err (cc, "sokt", "DBDELETE");

		strcpy (sokt_rec.co_no,comm_rec.co_no);
		sokt_rec.hhbr_hash = local_rec.hhbr_hash;
		sokt_rec.line_no = lcount [2];
		cc = find_rec ("sokt", &sokt_rec, GTEQ, "r");	
	}
}

/*=================================
| Delete sales order kit records. |
=================================*/
void
DeleteKit (void)
{
	scn_set (2);

	strcpy (sokt_rec.co_no,comm_rec.co_no);
	sokt_rec.hhbr_hash = local_rec.hhbr_hash;
	sokt_rec.line_no = 0;
	cc = find_rec ("sokt",&sokt_rec,GTEQ,"r");	
	while (!cc && !strcmp (sokt_rec.co_no,comm_rec.co_no) && 
			sokt_rec.hhbr_hash == local_rec.hhbr_hash)
	{
		putchar ('D');
		fflush (stdout);
		cc = abc_delete ("sokt");
		if (cc)
		       	file_err (cc, "sokt", "DBDELETE");

		strcpy (sokt_rec.co_no,comm_rec.co_no);
		sokt_rec.hhbr_hash = local_rec.hhbr_hash;
		sokt_rec.line_no = 0;
		cc = find_rec ("sokt", &sokt_rec, GTEQ, "r");	
	}
}

/*=======================
| Updated pos record (s) |
=======================*/
void 
UpdatePosdt (void)
{
    posdtup_rec.pos_no = 0;
	strcpy (posdtup_rec.file_name, "sokt");
	posdtup_rec.record_hash = local_rec.hhbr_hash;

	abc_add (posdtup,&posdtup_rec);
}

/*=================================
| Update sales order kit records. |
=================================*/
void
Update (void)
{
	if (newItem)
	{
		clear ();
		/*print_at (0,0,"\n\r\n\rUpdating %s Specifications ... ",
						 (KIT) ? "Kit" : "BOM");*/
		print_at (0,0,ML (mlSoMess014));
		fflush (stdout);
	}
	if (newItem)
		AddKitItem ();
	else
		update_menu ();
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
	if (KIT)
		rv_pr (ML (mlSoMess015), 20,0,1);
	else
		rv_pr (ML (mlSoMess016), 20,0,1);

	move (0,1);
	line (80);

	switch (scn)
	{
	case 	1:
		box (0,2,80, (BOM) ? 2 : 3);
		scn_set (2);
		scn_write (2);
		scn_display (2);
		break;

	case	2:
		box (0,2,80, (BOM) ? 2 : 3);
		scn_set (1);
		scn_write (1);
		scn_display (1);
		break;
	}

	move (0,20);
	line (80);

	move (0,21);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	move (0,22);
	line (80);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

