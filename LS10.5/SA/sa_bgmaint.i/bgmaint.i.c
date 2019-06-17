/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_bgmaint.i.c )                                 |
|  Program Desc  : ( Selection for Sales Budget Maintenance       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 30/05/87         |
|---------------------------------------------------------------------|
|  Date Modified : (30/05/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (29/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (16/09/97)      | Modified  by  : Rowena S Maandig |
|                                                                     |
|  Comments      :                                                    |
|  (29/03/94)    : INF 10647. Changes for ver9 compile on SCO.        |
|  (16/09/97)    : Updated to incorporate multilingual conversion.    |
|                :                                                    |
|                                                                     |
| $Log: bgmaint.i.c,v $
| Revision 5.2  2001/08/09 09:16:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:07  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:18  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:15  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/04/27 03:34:28  ramon
| Fixed the error in displaying the description fields.
|
| Revision 1.12  2000/04/27 00:47:25  cam
| Changed fields to not be AUTONEXT as this was causing problems under GVision.
|
| Revision 1.11  2000/04/25 03:47:14  ramon
| Added description fields.
|
| Revision 1.10  1999/12/06 01:35:22  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/16 04:55:30  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.8  1999/09/29 10:12:41  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:27:25  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 02:01:50  scott
| Updated from Ansi Project.
|
| Revision 1.5  1999/06/18 09:39:18  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bgmaint.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_bgmaint.i/bgmaint.i.c,v 5.2 2001/08/09 09:16:44 scott Exp $";

#include <ml_std_mess.h>		
#include <ml_sa_mess.h>		
#include <pslscr.h>		

#define TOTSCNS		1
#define	SALES	0
#define	CATG	1
#define	A_CODE	2
#define	C_TYPE	3

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};

	int comm_no_fields = 3;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
	} comm_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	by_who[4][8];
	char	by_who_desc[4][8];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "by_sman", 4, 18, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Salesman.", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "NY", "", local_rec.by_who[0]}, 
	{1, LIN, "by_sman_desc", 4, 21, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.by_who_desc[0]}, 
	{1, LIN, "by_catg", 5, 18, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Category.", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "NY", "", local_rec.by_who[1]}, 
	{1, LIN, "by_catg_desc", 5, 21, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.by_who_desc[1]}, 
	{1, LIN, "by_area", 6, 18, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Area.", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "NY", "", local_rec.by_who[2]}, 
	{1, LIN, "by_area_desc", 6, 21, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.by_who_desc[2]}, 
	{1, LIN, "by_ctype", 7, 18, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Class Type.", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "NY", "", local_rec.by_who[3]}, 
	{1, LIN, "by_ctype_desc", 7, 21, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.by_who_desc[3]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int redraw_rep (int field, int indx);
void set_dflt (void);
int run_prog (char *filename);
int find_rep (void);
void ReadMisc (void);
int heading (int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	if (argc != 2)
	{
		print_at(0,0,mlSaMess726,argv[0]);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();          /*  get into raw mode		*/
	set_masks();		/*  setup print using masks	*/
	init_vars(1);		/*  set default values		*/

    OpenDB ();
	/*==============================
	| Read common terminal record. |
	==============================*/
	ReadMisc(); 

	/*=====================
	| Reset control flags |
	=====================*/
   	search_ok = 1;
   	entry_exit = 1;
   	prog_exit = 0;
   	restart = 0;
	init_vars(1);	
	set_dflt();

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	heading(1);
	scn_display(1);
	edit(1);
	prog_exit = 1;
	rset_tty();
	if (restart) 
    {
		shutdown_prog();
        return (EXIT_SUCCESS);
    }
	
	if (run_prog (argv[1]) == 1)
	{
		return (EXIT_SUCCESS);
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

int
spec_valid (
 int    field)
{

	/*--------------------------------------------
	--------------------------------------------*/
	if (strcmp(FIELD.label,"by_sman") == 0) 
	{
		redraw_rep(field,0);
		return(0);
	}
	
	/*--------------------------------------------
	--------------------------------------------*/
	if (strcmp(FIELD.label,"by_catg") == 0) 
	{
		redraw_rep(field,1);
		return(0);
	}
	
	/*--------------------------------------------
	--------------------------------------------*/
	if (strcmp(FIELD.label,"by_area") == 0) 
	{
		redraw_rep(field,2);
		return(0);
	}
	
	/*--------------------------------------------
	--------------------------------------------*/
	if (strcmp(FIELD.label,"by_ctype") == 0) 
	{
		redraw_rep(field,3);
		return(0);
	}
	
	return(0);
}

int
redraw_rep (
 int    field,
 int    indx)
{
	int	i;
	int	option;

	if (local_rec.by_who[indx][0] == 'N')
		option = find_rep();
	else
		option = indx;

	for (i = 0;i < 4;i++)
	{
		if (i == option)
		{
			strcpy(local_rec.by_who[i], "Y");
			strcpy(local_rec.by_who_desc[i], "Yes");
		}
		else
		{
			strcpy(local_rec.by_who[i], "N");
			strcpy(local_rec.by_who_desc[i], "No ");
		}
		display_field(i * 2);
		display_field(i * 2 + 1);
	}
	return(0);
}

void
set_dflt (void)
{
	int	i;

	for (i = 0;i < 4;i++)
	{
		if (i == 0)
		{
			strcpy(local_rec.by_who[0],"Y");
			strcpy(local_rec.by_who_desc[0],"Yes");
		}
		else
		{
			strcpy(local_rec.by_who[i],"N");
			strcpy(local_rec.by_who_desc[i],"No ");
		}
	}
}

int
run_prog (
 char*  filename)
{
	char	rep_type[3];

	clear();
	/*print_at(0,0,"Busy ... ");*/
	print_at(0,0,ML(mlStdMess035));
	fflush(stdout);
	sprintf(rep_type,"%d",find_rep());
	execlp(filename,filename,rep_type,(char *)0);
	return (EXIT_FAILURE);
}

int
find_rep (void)
{
	int	i;

	for (i = 3;i > 0 && local_rec.by_who[i][0] == 'N';i--);

	return((local_rec.by_who[i][0] == 'Y') ? i : 0);
}

void
OpenDB (void)
{
    abc_dbopen ("data");
}

void
CloseDB (void)
{
    abc_dbclose("data");    
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		rv_pr(ML(mlSaMess019),22,0,1);

		box(0,3,80,4);

		move(0,20);
		line(80);
		
		print_at(21,0,ML(mlStdMess038),comm_rec.tco_no,clip(comm_rec.tco_name));
		move(0,22);
		line(80);
		move(1,input_row);
		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}

