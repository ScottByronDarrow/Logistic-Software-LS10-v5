/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_rtype_inp.c,v 5.1 2001/11/26 06:35:14 scott Exp $
|  Program Name  : (db_rtype_inp.c)
|  Program Desc  : (Customer Royalty Class Maintenance - Input/Maint)
|---------------------------------------------------------------------|
|  Author        : B.C. Lim.       | Date Written  : 29/08/88         |
|---------------------------------------------------------------------|
| $Log: db_rtype_inp.c,v $
| Revision 5.1  2001/11/26 06:35:14  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_rtype_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_rtype_inp/db_rtype_inp.c,v 5.1 2001/11/26 06:35:14 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	newCode = 0;

#include	"schema"

struct commRecord	comm_rec;
struct dbryRecord	dbry_rec;

extern	int		TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	prev_code [4];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "class", 4, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Customer Royalty Class      ", " ", 
		YES, NO, JUSTLEFT, " ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", dbry_rec.cr_type}, 
	{1, LIN, "desc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Customer Royalty Desc      ", " ", 
		YES, NO, JUSTLEFT, "", "", dbry_rec.desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	SrchDbry 		(char *);
void 	Update 			(void);
int 	heading 		(int);

/*
 * Main Processing Routine.
 */
int
main (
 int                argc,
 char*              argv [])
{

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	OpenDB (); 

	strcpy (local_rec.prev_code,"   ");

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		/*-----------------
		| Update records. |
		-----------------*/
		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}


/*=========================
| Program exit sequence	. |
=========================*/
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

	open_rec (dbry, dbry_list, DBRY_NO_FIELDS, "dbry_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (dbry);
	abc_dbclose ("data");
}

int
spec_valid (
	int		field)
{
	/*------------------------------------------
	| Validate Customer Royalty Class  Search. |
	------------------------------------------*/
	if (LCHECK ("class"))
	{
		if (SRCH_KEY)
		{
			SrchDbry (temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (dbry_rec.co_no,comm_rec.co_no);
		cc = find_rec (dbry, &dbry_rec, COMPARISON, "u");
		if (cc)
			newCode = 1;
		else
		{
			newCode = 0;
			entry_exit = TRUE;
		}
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchDbry (
	char	*key_val)
{
	_work_open (3,0,40);
	save_rec ("#No","#Royalty Class Description ");
	strcpy (dbry_rec.co_no,comm_rec.co_no);
	strcpy (dbry_rec.cr_type,key_val);
	cc = find_rec (dbry, &dbry_rec, GTEQ, "r");
	while (!cc && !strcmp (dbry_rec.co_no,comm_rec.co_no) && 
				  !strncmp (dbry_rec.cr_type,key_val,strlen (key_val)))
	{
		cc = save_rec (dbry_rec.cr_type,dbry_rec.desc);
		if (cc)
			break;
		cc = find_rec (dbry, &dbry_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (dbry_rec.co_no,comm_rec.co_no);
	sprintf (dbry_rec.cr_type,"%-3.3s",temp_str);
	cc = find_rec (dbry, &dbry_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in dbry During (DBFIND)",cc,PNAME);
}

/*
 * Update dbry records
 */
void
Update (void)
{
	if (newCode)
	{
		strcpy (dbry_rec.stat_flag,"0");
		cc = abc_add (dbry,&dbry_rec);
		if (cc)
			file_err (cc, dbry, "DBADD");
	}
	else
	{
		cc = abc_update (dbry,&dbry_rec);
		if (cc)
			file_err (cc, dbry, "DBUPDATE");
	}
	strcpy (local_rec.prev_code,dbry_rec.cr_type);
	abc_unlock (dbry);
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
		/*----------------------------
		| Royalty Class Maintenance. |
		----------------------------*/
		sprintf (err_str, " %s ", ML (mlDbMess064));
		rv_pr (err_str, 28,0,1);

		sprintf (err_str, ML (mlDbMess072), local_rec.prev_code);
		print_at (0,60, "%s", err_str);

		line_at (1,0,80);
		
		box (0,3,80,2);

		line_at (20,0,80);
		line_at (22,0,80);

		sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (21,0, err_str);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
