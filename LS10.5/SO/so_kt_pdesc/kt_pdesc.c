/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: kt_pdesc.c,v 5.6 2002/07/23 07:47:49 scott Exp $
|  Program Name  : (so_kt_pdesc.c & so_kt_idesc.c)   
|  Program Desc  : (Special Instructions for P/S and Invoives)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 15/02/92          |
|---------------------------------------------------------------------|
| $Log: kt_pdesc.c,v $
| Revision 5.6  2002/07/23 07:47:49  scott
| S/C 004208
|
| Revision 5.5  2002/07/18 03:15:34  scott
| .
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: kt_pdesc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_kt_pdesc/kt_pdesc.c,v 5.6 2002/07/23 07:47:49 scott Exp $";

#define	TXT_REQD
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define	PACK_SLIP	 (programRunType [0] == 'P')
#define	INVOICE		 (programRunType [0] == 'I')

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct sokdRecord	sokd_rec;

	char	programRunType [2];
   	int  	newItem = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	item_no [17];
	char	text_line [61];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "item_no",	 3, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "item_desc",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{2,TXT,"text_line",9,8,0,"","          "," "," ",
	   "                       T E X T                            "," ",10,60,50,"","",local_rec.text_line},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
void 	LoadSpec 		(void);
void 	UpdateSpec 		(void);
int  	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "so_kt_pdesc"))
		strcpy (programRunType, "P");

	if (!strcmp (sptr, "so_kt_idesc"))
		strcpy (programRunType, "I");

	SETUP_SCR (vars);

	tab_row = 9;
	tab_col = 8;
	init_scr 	();
	set_tty 	();
	set_masks 	();

	OpenDB ();

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
		init_ok 	= TRUE;
		lcount [2] 	= 0;
		init_vars (1);
		init_vars (2);

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		scn_write (1);
		scn_display (1);
		scn_display (2);

		if (newItem)
			entry (2);
		else
			edit (2);
		
		if (!restart)
			UpdateSpec ();
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

	open_rec (sokd, sokd_list, SOKD_NO_FIELDS, "sokd_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (sokd);
	abc_fclose (inmr);
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

		strcpy (sokd_rec.co_no,comm_rec.co_no);
		strcpy (sokd_rec.type, (INVOICE) ? "I" : "P");
		sokd_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sokd_rec.line_no = 0;
		cc = find_rec (sokd, &sokd_rec, GTEQ, "r");	
		if (cc || 	strcmp (sokd_rec.co_no,comm_rec.co_no) || 
		         	sokd_rec.hhbr_hash != inmr_rec.hhbr_hash ||
			 		sokd_rec.type [0] != programRunType [0])
			newItem = TRUE;
		else
		{
			newItem = FALSE;
			LoadSpec ();
		}

		DSP_FLD ("item_no");
		DSP_FLD ("item_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

void
LoadSpec (void)
{
	init_vars (2);

	lcount [2] = 0;

	strcpy (sokd_rec.co_no,comm_rec.co_no);
	strcpy (sokd_rec.type, (INVOICE) ? "I" : "P");
	sokd_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sokd_rec.line_no = 0;
	cc = find_rec (sokd, &sokd_rec, GTEQ, "r");	
	while (!cc && !strcmp (sokd_rec.co_no,comm_rec.co_no) && 
		     sokd_rec.hhbr_hash == inmr_rec.hhbr_hash &&
		     sokd_rec.type [0] == programRunType [0])
	{
		strcpy (local_rec.text_line, sokd_rec.text);
		putval (lcount [2]++);
		cc = find_rec (sokd, &sokd_rec, NEXT, "r");	
	}
	scn_set (1);
}

void
UpdateSpec (void)
{
	int	i;

	scn_set (2);

	for (i = 0;i < lcount [2];i++)
	{
		getval (i);

		strcpy (sokd_rec.co_no,comm_rec.co_no);
		strcpy (sokd_rec.type, (INVOICE) ? "I" : "P");
		sokd_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sokd_rec.line_no = i;
		cc = find_rec (sokd,&sokd_rec,COMPARISON,"u");	
		/*-------------------------------
		| Didn't find record so add one	|
		-------------------------------*/
		if (cc)
		{
			strcpy (sokd_rec.co_no,comm_rec.co_no);
			strcpy (sokd_rec.type, (INVOICE) ? "I" : "P");
			sokd_rec.hhbr_hash = inmr_rec.hhbr_hash;
			sokd_rec.line_no = i;
			strcpy (sokd_rec.text,local_rec.text_line);
			cc = abc_add (sokd,&sokd_rec);
			if (cc)
			       file_err (cc, sokd, "DBADD");
		}
		else
		{
			strcpy (sokd_rec.text,local_rec.text_line);
			cc = abc_update (sokd,&sokd_rec);
			if (cc)
			       file_err (cc, sokd, "DBUPDATE");
		}
		abc_unlock (sokd);
	}

	for (i = lcount [2];i < MAXLINES;i++)
	{
		strcpy (sokd_rec.co_no,comm_rec.co_no);
		strcpy (sokd_rec.type, (INVOICE) ? "I" : "P");
		sokd_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sokd_rec.line_no = i;
		cc = find_rec (sokd,&sokd_rec,COMPARISON,"r");	
		if (!cc)
			abc_delete (sokd);
	}
}

int
heading (
 int scn)
{
	int	page_len;

	if (restart) 
		return (EXIT_SUCCESS);

	page_len = 80;

	clear ();
	if (INVOICE)
		strcpy (err_str, ML (mlSoMess209));
	else
		strcpy (err_str, ML (mlSoMess210));
	
	rv_pr (err_str, (page_len - strlen (err_str)) / 2, 0, 1);
	
	line_at (1,0, page_len);

	box (0,2,page_len, 4);

	if (scn == 1)
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
	scn_set (scn);
	line_at (21,0, page_len);
	print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
