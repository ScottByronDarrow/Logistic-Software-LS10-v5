/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_purge.i.c  )                                  |
|  Program Desc  : ( Input Program for Service job Purges         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  data,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 25/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (22/11/88)      | Modified  by  : B.C.LIM          |
|  Date Modified : (13/09/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (15/10/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : 													  |
|  (22/11/88)    : Tidy up program to use new screen generator.       |
|  (13/09/97)    : Updated for Multilingual Conversion and	     	  |
|  (15/10/97)    : Fixed MLDB error.						     	  |
|                :                                                    |
|                                                                     |
| $Log: sj_purge.i.c,v $
| Revision 5.2  2001/08/09 09:17:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:44  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:37  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:03  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/11/17 06:40:49  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.9  1999/11/16 05:58:35  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.8  1999/10/20 02:07:05  nz
| Updated for final changes on date routines.
|
| Revision 1.7  1999/09/29 10:13:07  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/24 05:06:41  scott
| Updated from Ansi
|
| Revision 1.5  1999/06/20 02:30:36  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_purge.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_purge.i/sj_purge.i.c,v 5.2 2001/08/09 09:17:45 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>
				    
	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
	} comm_rec;

	char	rep_desc[31];
	char	systemDate[11];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	back[5];
	char	onight[5];
	char	s_fr_date[10];
	long	fr_date;
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "back", 4, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", "Background ", " ", 
		NO, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 5, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", "Overnight ", " ", 
		NO, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "from_date", 7, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Purge Cutoff Date", "All invoiced items before this date will be purged", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.fr_date}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int heading (int scn);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{

	SETUP_SCR (vars);

	/*---------------------------------------
	|	parameters			|
	|	1:	program name		|
	|	2:	program description	|
	---------------------------------------*/

	if (argc != 3) 
	{
		print_at(0,0, mlStdMess037 ,argv[0]);
		return (EXIT_FAILURE);
	}

	sprintf(rep_desc,"%-30.30s",argv[2]);

	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	strcpy (systemDate, DateToString (TodaysDate()));

	OpenDB();

	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit = 0;
   	prog_exit = 0;
   	restart = 0;
   	search_ok = TRUE;
	init_vars(1);

	strcpy(local_rec.back,"N(o ");
	strcpy(local_rec.onight,"N(o ");
	local_rec.fr_date = StringToDate(systemDate);

	heading(1);
	scn_display(1);
	edit(1);
    if (restart || restart) {
		shutdown_prog();
        return (EXIT_SUCCESS);
    }
	
	rset_tty();

	strcpy(local_rec.s_fr_date,DateToString(local_rec.fr_date));

	clear();
	/*print_at(0,0,"Busy,....");*/
	print_at(0,0, ML(mlStdMess035));
	fflush(stdout);

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y')
	{ 
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				argv[1],
				local_rec.s_fr_date,
				ML(mlSjMess061),(char *)0);
				/*"Purging Service Job",(char *)0);*/
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
			 execlp(argv[1],
				argv[1],
				local_rec.s_fr_date,(char *)0);
	}
	else 
	{
		execlp(argv[1],
			argv[1],
			local_rec.s_fr_date,(char *)0);
	}

    shutdown_prog();
    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen("data");
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	CloseDB ();
}

void
CloseDB (
 void)
{
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if (strcmp(FIELD.label,"back") == 0)
	{
		strcpy(local_rec.back,(local_rec.back[0] == 'Y') ? "Y(es" : "N(o ");
		display_field(label("back"));
		return(0);
	}

	if (strcmp(FIELD.label,"onight") == 0)
	{
		strcpy(local_rec.onight,(local_rec.onight[0] == 'Y') ? "Y(es" : "N(o ");
		display_field(label("onight"));
		return(0);
	}
	return(0);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();

		rv_pr(rep_desc,(80 - strlen(clip(rep_desc))) / 2,0,1);
		move(0,1);
		line(80);

		box(0,3,80,4);
		move(1,6);
		line(79);

		move(0,19);
		line(80);
		move(0,20);
		print_at(20,0, ML(mlStdMess038) ,comm_rec.tco_no,comm_rec.tco_name);

		print_at(21,0, ML(mlStdMess039) ,comm_rec.tes_no,comm_rec.tes_name);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
