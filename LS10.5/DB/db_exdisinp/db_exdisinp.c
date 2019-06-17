/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_exdisinp.c,v 5.2 2002/02/01 06:12:02 robert Exp $
|  Program Name  : (db_exdisinp.c) 
|  Program Desc  : (Customer External Discount Maintenance)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 12/02/88         |
|---------------------------------------------------------------------|
| $Log: db_exdisinp.c,v $
| Revision 5.2  2002/02/01 06:12:02  robert
| SC 00748 - updated to fix cancel key
|
| Revision 5.1  2001/12/07 03:39:34  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_exdisinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_exdisinp/db_exdisinp.c,v 5.2 2002/02/01 06:12:02 robert Exp $";

#define TOTSCNS		1
#define	MAXLINES	26
#define	TABLINES	13
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct exdfRecord	exdf_rec;

	char	*codes = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int		newItem = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
} local_rec;

static	struct	var	vars []	={	

	{1, TAB, "discountCode", MAXLINES, 3, CHARTYPE, 
		"U", "          ", 
		" ", "", " Code. ", " ", 
		NA, NO, JUSTLEFT, "", "", exdf_rec.code}, 
	{1, TAB, "discountPC", 0, 6, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", " Discount Percent ", " ", 
		YES, NO, JUSTRIGHT, "0.00", "100.00", (char *)&exdf_rec.disc_pc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
void 	LoadDiscount 		(void);
void 	Update 				(void);
int 	heading 			(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv [])
{
	SETUP_SCR (vars);

	tab_row = 3;
	tab_col = 24;
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();
    /*============================================
    | Get common info from common database file. |
    ============================================*/
	read_comm ( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );

	/*-----------------------
	| Reset control flags . |
	-----------------------*/
	newItem 		= FALSE;
	entry_exit 		= FALSE;
	edit_exit 		= FALSE;
	prog_exit 		= FALSE;
	restart 		= FALSE;
	init_ok 		= FALSE;

	/*-------------------------------
	| Enter screen 1 linear input . |	
	-------------------------------*/
	LoadDiscount ();

	if (newItem)
	{
		heading (1);
		scn_display (1);
		entry (1);
		if (restart || prog_exit)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }
	}

	/*------------------------------
	| Edit screen 1 linear input . |	
	------------------------------*/
	heading (1);
	scn_display (1);
	edit (1);

	if (!restart)
		Update ();
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

	open_rec (exdf, exdf_list, EXDF_NO_FIELDS, "exdf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (exdf);
	abc_dbclose ("data");
}

int
spec_valid (
	 int	field)  
{
	if (LCHECK ("discountCode"))
	{
		if (prog_status == ENTRY)
			getval (line_cnt);
	}
	return (EXIT_SUCCESS);
}

void
LoadDiscount (void)
{
	char	*sptr = codes;

	init_ok = 1;
	scn_set (1);
	init_ok = 0;

	lcount [1] = MAXLINES;
	newItem = 0;

	for (line_cnt = 0;line_cnt < MAXLINES;line_cnt++)
	{
		strcpy (exdf_rec.co_no,comm_rec.co_no);
		sprintf (exdf_rec.code,"%-1.1s",sptr);
		cc = find_rec (exdf,&exdf_rec,COMPARISON,"u");
		if (cc)
		{
			newItem = 1;
			sprintf (exdf_rec.code,"%-1.1s",sptr);
			exdf_rec.disc_pc = 0.00;
		}
		putval (line_cnt);
		sptr++;
	}
}

void
Update (void)
{
	print_at (2,0, ML (mlStdMess035));

	fflush (stdout);
	for (line_cnt = 0;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt);

		strcpy (exdf_rec.co_no,comm_rec.co_no);
		cc = find_rec (exdf,&exdf_rec,COMPARISON,"u");
		if (cc)
		{
			getval (line_cnt);
			cc = abc_add (exdf,&exdf_rec);
			if (cc)
			   file_err (cc, exdf, "DBADD");
		}
		else
		{
			getval (line_cnt);
			cc = abc_update (exdf,&exdf_rec);
			if (cc)
			   file_err (cc, exdf, "DBUPDATE");
		}
		abc_unlock (exdf);
	}
}

int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlDbMess127),22,0,1);

		line_at (1,0,80);
		line_at (20,0,80);

		print_at (21,0, ML (mlStdMess038) ,comm_rec.co_no,comm_rec.co_name);

		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
