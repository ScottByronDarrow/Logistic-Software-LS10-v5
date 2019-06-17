/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: bcode_inp.c,v 5.6 2002/07/25 11:17:32 scott Exp $
|  Program Name  : (sk_bcode_inp.c)
|  Program Desc  : (Stock bar code input)
|---------------------------------------------------------------------|
|  Date Written  : (06/10/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: bcode_inp.c,v $
| Revision 5.6  2002/07/25 11:17:32  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.5  2002/07/24 08:39:12  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/06/20 07:10:53  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/08/09 09:18:09  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:41  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:52  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bcode_inp.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_bcode_inp/bcode_inp.c,v 5.6 2002/07/25 11:17:32 scott Exp $";

/*===========================================
| These next two lines for 132 tabular only |
===========================================*/
#define MAXWIDTH 100
#define MAXLINES 20

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#define		SCN_BARCODE		2

	int	clear_ok	=	TRUE;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inbmRecord	inbm_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct posdtupRecord	posdtup_rec;

	struct	storeRec {
		char	barCodes [17];
	} store [MAXLINES];

/*=========
 File Names
===========*/
static char
	*data	= "data",
	*inum2	= "inum2";

/*==========
  Globals
==========*/
	int	newCode = 0;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	item_no [17];
	char	sup_part [17];
	char	desc [41];
	char	UOM [5];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "item_no", 4, 18, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number : ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "desc", 5, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Item Desc   : ", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description},
	{SCN_BARCODE, TAB, "barcode",	 MAXLINES, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Bar code Number   ", " ",
		 YES, NO,  JUSTLEFT, "", "", inbm_rec.barcode},
	{SCN_BARCODE, TAB, "UOM",	 0, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "  UOM  ", "Enter UOM. Default = standard UOM ",
		YES, NO,  JUSTLEFT, "", "", local_rec.UOM},
	{SCN_BARCODE, TAB, "DATE",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "", "",
		ND, NO,  JUSTLEFT, "", "", (char *)&inbm_rec.last_mod},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};


int	PosInstalled	=	FALSE;

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
int  	CheckDupBarcode (char *, int);
int  	DeleteLine 		(void);
void 	LoadBarCodes 	(void);
void 	Update 			(void);
void 	SrchInum 		(char *);
int  	heading 		(int);
void 	PosUpdate 		(void);



/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	/*----------------------------------------------
	| Check for advertising levy/ freight charges. |
	----------------------------------------------*/
	sptr = chk_env ("POS_INSTALLED");
	PosInstalled = (sptr == (char *)0) ? FALSE : atoi (sptr);

	SETUP_SCR (vars);


	init_scr ();	
	set_tty ();
	set_masks ();		
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (SCN_BARCODE, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	OpenDB ();

	tab_row	=	7;
	tab_col	=	18;
	while (prog_exit == 0) 
	{
		int		i;

		for (i = 0; i < MAXLINES; i++)
			strcpy (store [i].barCodes, " ");

		lcount [2] = 0;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);
		clear_ok = TRUE,

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		scn_write (1);
		scn_display (1);
		scn_write (SCN_BARCODE);
		scn_display (SCN_BARCODE);

		/*------------------------------
		| Enter screen 2 tabular input.|
		------------------------------*/
		if (newCode)
			entry (SCN_BARCODE);
		else
			edit (SCN_BARCODE);

		if (prog_exit || restart) 
			continue;

		edit_all ();
		if (restart) 
			continue;

		clear_ok = TRUE,

		/*------------------------------
		| Update selection status.     |
		------------------------------*/
		Update ();

		/*--------------------------------------
		| POS system installed so write audit. |
		--------------------------------------*/
		if (PosInstalled)
			PosUpdate ();
	}	
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (inum2, inum);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inbm, inbm_list, INBM_NO_FIELDS, "inbm_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2,inum_list, INUM_NO_FIELDS, "inum_id_no2");

	if (PosInstalled)
		open_rec (posdtup, posdtup_list, POSDTUP_NO_FIELDS, "pos_no1");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (inbm);
	abc_fclose (inum);
	abc_fclose (inum2);
    
	if (PosInstalled)
		abc_fclose (posdtup);

	SearchFindClose ();
    abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();
	

		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		SuperSynonymError ();

		/*------------------------
		| Discontinued Product ? |
		------------------------*/
		if (inmr_rec.active_status [0] == 'D')
		{
			print_mess (ML (mlSkMess558));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.item_no, inmr_rec.item_no);
		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		LoadBarCodes ();

		return (EXIT_SUCCESS);
	}

	/*==========================
	| Validate Unit of Measure |
	==========================*/
	if (LCHECK ("UOM"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.UOM, inum_rec.uom);
		}

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		strcpy (inum2_rec.uom, local_rec.UOM);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (inmr_rec.hhbr_hash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		strcpy (local_rec.UOM, inum2_rec.uom);

		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Bar Code Number . |
	----------------------------*/
	if (LCHECK ("barcode"))
	{
		if (dflt_used || last_char == DELLINE)
			return (DeleteLine ());

		abc_selfield (inbm, "inbm_id_no");
		strcpy (inbm_rec.co_no, comm_rec.co_no);
		cc = find_rec (inbm, &inbm_rec, COMPARISON, "r");
		if (!cc)
		{
			errmess (ML ("Barcode number already on file."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (CheckDupBarcode (inbm_rec.barcode, line_cnt))
		{
			print_mess (ML ("Barcode number already keyed."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].barCodes, inbm_rec.barcode);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=======================================
| Check for duplicate bar code numbers. |
=======================================*/
int
CheckDupBarcode (
 char	*BarCodeNo,
 int	line_no)
{
	int		i;
	int		no_items = (prog_status == ENTRY) ? line_cnt : lcount [SCN_BARCODE];

	for (i = 0;i < no_items;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		if (!strcmp (store [i].barCodes, "                "))
			continue;

		if (!strcmp (store [i].barCodes, BarCodeNo))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Delete bar code lines. |
========================*/
int
DeleteLine (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;
	/*-----------------------
	| entry					|
	-----------------------*/
	if (prog_status == ENTRY)
	{
		print_mess (ML ("Cannot insert/delete lines on ENTRY."));
		return (EXIT_FAILURE);
	}
	/*-------------------------------
	| no lines to delete			|
	-------------------------------*/
	if (lcount [SCN_BARCODE] <= 0)
	{
		print_mess (ML ("Cannot delete line - no lines to delete."));
		return (EXIT_FAILURE);
	}
	/*---------------------------
	| delete lines				|
	---------------------------*/
	lcount [SCN_BARCODE]--;
	for (i = line_cnt;line_cnt < lcount [SCN_BARCODE];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
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
	if (lcount [SCN_BARCODE] <= 0)
	{
		init_vars (SCN_BARCODE);
		putval (i);
	}
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*======================
| Load bar code lines. |
======================*/
void
LoadBarCodes (
 void)
{

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (SCN_BARCODE);
	lcount [SCN_BARCODE] = 0;

	abc_selfield (inbm, "inbm_id_no2");
	strcpy (inbm_rec.co_no, comm_rec.co_no);
	strcpy (inbm_rec.item_no, inmr_rec.item_no);
	strcpy (inbm_rec.barcode, " ");
	cc = find_rec (inbm, &inbm_rec, GTEQ, "r");
	while (!cc && !strcmp (inbm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (inbm_rec.item_no, inmr_rec.item_no))
	{
		strcpy (store [lcount [SCN_BARCODE]].barCodes, inbm_rec.barcode);
		strcpy (local_rec.UOM, inbm_rec.uom);
		putval (lcount [SCN_BARCODE]++);

		if (lcount [SCN_BARCODE] >= MAXLINES)
			break;

		cc = find_rec (inbm, &inbm_rec, NEXT, "r");
	}
	newCode = (lcount [SCN_BARCODE] == 0);
	scn_set (1);
}

/*========================
| Update bar code lines. |
========================*/
void
Update (
 void)
{
	/*---------------
	| Maintain inmr  |
	----------------*/	
	scn_set (SCN_BARCODE);

	abc_selfield (inbm, "inbm_id_no2");

	strcpy (inbm_rec.co_no, comm_rec.co_no);
	strcpy (inbm_rec.item_no, inmr_rec.item_no);
	strcpy (inbm_rec.barcode, " ");
	cc = find_rec (inbm, &inbm_rec, GTEQ, "r");
	while (!cc && !strcmp (inbm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (inbm_rec.item_no, inmr_rec.item_no))
	{
		abc_delete (inbm);

		strcpy (inbm_rec.co_no, comm_rec.co_no);
		strcpy (inbm_rec.item_no, inmr_rec.item_no);
		strcpy (inbm_rec.barcode, " ");
		cc = find_rec (inbm, &inbm_rec, GTEQ, "r");
	}
	scn_set (SCN_BARCODE);
	for (line_cnt = 0; line_cnt < lcount [SCN_BARCODE]; line_cnt++)
	{
		getval (line_cnt);

		strcpy (inbm_rec.co_no, comm_rec.co_no);
		strcpy (inbm_rec.item_no, local_rec.item_no);
		strcpy (inbm_rec.uom, local_rec.UOM);
		if (inbm_rec.last_mod <= 0L)
			inbm_rec.last_mod = comm_rec.inv_date;
		cc = abc_add (inbm, &inbm_rec);
		if (cc)
			file_err (cc, inbm, "DBADD");
	}
}

/*===============
| Search on UOM |
===============*/
void
SrchInum (
 char *key_val)
{
	work_open ();
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
	{
		if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (inmr_rec.hhbr_hash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom, inum2_rec.desc);
			if (cc)
				break;
		}

		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum2, "DBFIND");
}

/*-----------------
| Screen Heading. |
-----------------*/		
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		fflush (stdout);

		print_at (0,25,ML ("%R Bar Code Maintenance."));
		fflush (stdout);

		line_at (1,0,79);

		switch (scn)
		{
		case  1 :
			box (0,3,79,2);
			scn_set (2);
			scn_write (2);
			scn_display (2);
			break;
		
		case  2 :
			box (0,3,79,2);
			scn_set (1);
			scn_write (1);
			scn_display (1);
			fflush (stdout);
			break;

		}
		line_at (20,1,79);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void 
PosUpdate (void)
{
	/* inmr also updates inbm */
	posdtup_rec.pos_no = 0;
	strcpy (posdtup_rec.file_name,inmr);
	posdtup_rec.record_hash = inmr_rec.hhbr_hash; 
   
	abc_add (posdtup, &posdtup_rec);
}

