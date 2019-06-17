/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_dp_lp.i.c   )                                 |
|  Program Desc  : ( Department Selection  Lp no select           )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cudp,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :   N/A,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 12/06/87         |
|---------------------------------------------------------------------|
|  Date Modified : (12/06/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (01/12/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (29/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (05/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (03/09/1999)    | Modified  by  : edge cabalfin    |
|                                                                     |
|  Comments      : Tidy up program to use new screen generator.       |
|                :                                                    |
|     (29/11/89) : Correct spelling in error message.                 |
|  (05/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (03/09/1999)  :  ANSIfication of the code                          |
|                :      - potential problems marked with QUERY        |
|                :                                                    |
|                                                                     |
| $Log: sj_dp_lp.i.c,v $
| Revision 5.3  2002/07/17 09:57:48  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:21  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:03  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:07  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:13  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:42  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/11/17 06:40:46  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/11/16 05:58:29  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.7  1999/09/29 10:12:55  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/24 05:06:29  scott
| Updated from Ansi
|
| Revision 1.5  1999/06/20 02:30:25  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_dp_lp.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_dp_lp.i/sj_dp_lp.i.c,v 5.3 2002/07/17 09:57:48 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#include <pslscr.h>
#include <get_lpno.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/
/*  NOTES
    these should be declared as const char*
    to minimize potential problems.
*/
char *cudp = "cudp";
char *data = "data";

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = 
    {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
	};

	const int comm_no_fields = 6;

	struct 
    {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
	} comm_rec;

	/*=========================
	| Department Master File. |
	=========================*/
	struct dbview cudp_list[] =
    {
		{"cudp_co_no"},
		{"cudp_br_no"},
		{"cudp_dp_no"},
		{"cudp_dp_name"},
	};

	const int cudp_no_fields = 4;

	struct 
    {
		char	dp_co_no[3];
		char	dp_br_no[3];
		char	dp_dp_no[3];
		char	dp_dp_name[41];
	} cudp_rec;

	int     lpno;
	char	lp_no[3];

/*============================
| Local & Screen Structures. |
============================*/
struct 
{
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	=	
{
	{1, LIN, "dp_no", 4, 25, CHARTYPE, 
		"UU", "          ", 
		" ", comm_rec.tdp_no, "Department No ", " ", 
		YES, NO, JUSTRIGHT, "", "", cudp_rec.dp_dp_no}, 
	{1, LIN, "dp_name", 5, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Name ", " ", 
		NA, NO, JUSTLEFT, "", "", cudp_rec.dp_dp_name}, 
	{1, LIN, "lpno", 6, 25, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&lpno}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*===============================
|   Local function prototypes   |
===============================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void load_dflt (void);
int spec_valid (int field);
void save_page (char *key_val);
int  heading (int scn);

/*===========================
| Main Processing Routine . |
===========================*/
int 
main (
 int argc, 
 char *argv[])
{
	if (argc != 2)
	{
		print_at(0,0, mlSjMess711,argv[0]);
        return (EXIT_FAILURE);
	}
	/*---------------------------
	| Stup required parameters. |
	---------------------------*/
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*-----------------------
	| Reset control flags . |
	-----------------------*/
	entry_exit = 0;
	edit_exit = 0;
	prog_exit = 0;
	restart = 0;
	search_ok = 1;

	load_dflt ();
	/*------------------------------
	| Edit screen 1 linear input . |	
	------------------------------*/
	heading (1);
	scn_display (1);
	edit (1);

	sprintf (lp_no,"%d",lpno);

	shutdown_prog ();

	if (!restart)
    {
		execlp (argv[1], 
                argv[1], 
                cudp_rec.dp_dp_no,
                lp_no,
                (char *)0); /* QUERY is this a null pointer? */
    }

    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
/*  QUERY
    this function is not called anywhere!
    renamed it anyway...
*/
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
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec (cudp, cudp_list, cudp_no_fields, "cudp_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (cudp);
	abc_dbclose (data);
}

void
load_dflt (
 void)
{
	lpno = 1;
	strcpy (cudp_rec.dp_co_no,comm_rec.tco_no);
	strcpy (cudp_rec.dp_br_no,comm_rec.test_no);
	strcpy (cudp_rec.dp_dp_no,comm_rec.tdp_no);

	cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
	if (cc)
    {
		sys_err ("Error in cudp during (DBFIND)",cc,PNAME);
    }
}

/*  QUERY
    assumed return values of 0 mean EXIT_SUCCESS and
    assumed return values of 1 mean EXIT_FAILURE.
    this is based on the program flow.
    NOTES
    comments would increase readability of the code..
*/
int
spec_valid (
 int field)
{
	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (lpno))
		{
			/*------------------
			| Invalid printer. |
			------------------*/
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
        } 
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dp_no")) 
	{
		if (SRCH_KEY)
		{
			save_page (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cudp_rec.dp_co_no,comm_rec.tco_no);
		strcpy (cudp_rec.dp_br_no,comm_rec.test_no);

		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			/*-----------------------
			| Department not found. |
			-----------------------*/
			print_mess (ML (mlStdMess084));
			return (EXIT_FAILURE);
		}
		scn_display (1);
		return (EXIT_SUCCESS);
	}
			
	return (EXIT_SUCCESS);
}

void
save_page(
 char *key_val)
{
	work_open ();
	save_rec ("#Dp","#Department Name");
	strcpy (cudp_rec.dp_co_no,comm_rec.tco_no);
	strcpy (cudp_rec.dp_br_no,comm_rec.test_no);
	sprintf (cudp_rec.dp_dp_no,"%2.2s",key_val);

	cc = find_rec (cudp, &cudp_rec, GTEQ, "r");
	while (!cc && 
           !strncmp (cudp_rec.dp_dp_no, key_val, strlen (key_val)) && 
           !strcmp (cudp_rec.dp_co_no, comm_rec.tco_no) && 
           !strcmp (cudp_rec.dp_br_no,comm_rec.test_no))
	{
		cc = save_rec (cudp_rec.dp_dp_no,cudp_rec.dp_dp_name);
		if (cc)
        {
			break;
        }
		cc = find_rec (cudp, &cudp_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (cc)
    {		
        return;
    }
	strcpy (cudp_rec.dp_co_no,comm_rec.tco_no);
	strcpy (cudp_rec.dp_br_no,comm_rec.test_no);
	sprintf (cudp_rec.dp_dp_no,"%2.2s",temp_str);

	cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
	if (cc)
    {
		sys_err ("Error in cudp During (DBFIND)", cc, PNAME);
    }
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
        {
			scn_set (scn);
        }
		clear ();
		/*-----------------------
		| Department Selection. |
		-----------------------*/
		sprintf (err_str, " %s ", ML (mlSjMess043));
		rv_pr (err_str, 29, 0, 1);
		move (0,1);
		line (80);

		box (0,3,80,3);

		move (0,20);
		line (80);
		sprintf (err_str, 
                 ML (mlStdMess038),
                 comm_rec.tco_no,
                 comm_rec.tco_name);
		print_at (21,0, "%s", err_str);
		print_at (22,0, 
                  ML (mlStdMess039),
                  comm_rec.test_no,
                  comm_rec.test_name);
		move (0,23);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}


/* [ end of file ] */
