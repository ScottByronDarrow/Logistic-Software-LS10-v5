/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_dpsel.i.c   )                                 |
|  Program Desc  : ( Department Selection Input.                  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cudp,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :   N/A,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 12/06/87         |
|---------------------------------------------------------------------|
|  Date Modified : (12/06/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (02/11/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (05/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (03/09/1999)    | Modified  by  : edge cabalfin    |
|                                                                     |
|  Comments      : Tidy up program to use new screen generator.       |
|  (05/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
|  (03/09/1999)  : ANSIfication of the code                           |
|                :      - potential problems are marked with QUERY    |
|                :                                                    |
| $Log: sj_dpsel.i.c,v $
| Revision 5.2  2001/08/09 09:17:21  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:22  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:05  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:09  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:13  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:43  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/11/16 05:58:29  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.6  1999/09/29 10:12:55  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/24 05:06:29  scott
| Updated from Ansi
|
| Revision 1.4  1999/06/20 02:30:25  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN 
char	*PNAME = "$RCSfile: sj_dpsel.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_dpsel.i/sj_dpsel.i.c,v 5.2 2001/08/09 09:17:21 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#include <pslscr.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/
/*  NOTES
    these should be declared as const char*
    to minimize potential problems.
*/
char *comm = "comm";
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
		{"comm_dp_name"},
		{"comm_dp_short"},
	};

	const int comm_no_fields = 8;

	struct 
    {
		int	    termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
		char	tdp_name[41];
		char	tdp_short[16];
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
		{"cudp_dp_short"},
	};

	const int cudp_no_fields = 5;

	struct 
    {
		char	dp_co_no[3];
		char	dp_br_no[3];
		char	dp_dp_no[3];
		char	dp_dp_name[41];
		char	dp_dp_short[16];
	} cudp_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct 
{
    char    dummy[11];
} local_rec;

static struct var vars[] =
{	
	{1, LIN, "dp_no", 4, 18, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Department No.", " ", 
		YES, NO, JUSTRIGHT, "", "", cudp_rec.dp_dp_no}, 
	{1, LIN, "dp_name", 5, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Name. ", " ", 
		NA, NO, JUSTLEFT, "", "", cudp_rec.dp_dp_name}, 
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
int  spec_valid (int field);
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
		print_at (0,0, mlSjMess711,argv[0]);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);
	/*---------------------------
	| Stup required parameters. |
	---------------------------*/
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

	heading (1);
	entry (1);
	if (restart || prog_exit)
    {
        /*  QUERY
            expected program flow unknown.
            assuming EXIT_SUCCESS for return value.
        */
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	heading (1);
	scn_display (1);
	edit (1);
	if (restart)
    {
        /*  QUERY
            expected program flow unknown.
            assuming EXIT_SUCCESS for return value.
        */
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	strcpy (comm_rec.tdp_no,cudp_rec.dp_dp_no);
	strcpy (comm_rec.tdp_name,cudp_rec.dp_dp_name);
	strcpy (comm_rec.tdp_short,cudp_rec.dp_dp_short);

	cc = abc_update (comm,&comm_rec);
	if (cc)
    {
		file_err (cc, comm, "DBFIND");
    }

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
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
	open_rec(cudp, cudp_list, cudp_no_fields, "cudp_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose(cudp);
	abc_fclose(comm);
	abc_dbclose(data);
}

int
spec_valid (
 int field)
{
	if (LCHECK("dp_no"))
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
save_page (
 char *key_val)
{
	work_open ();
	save_rec ("#Dp","#Department Name");
	strcpy (cudp_rec.dp_co_no,comm_rec.tco_no);
	strcpy (cudp_rec.dp_br_no,comm_rec.test_no);
	sprintf (cudp_rec.dp_dp_no,"%2.2s",key_val);

	cc = find_rec (cudp, &cudp_rec, GTEQ, "r");
	while (!cc && 
           !strncmp (cudp_rec.dp_dp_no,key_val,strlen(key_val)) && 		  
           !strcmp (cudp_rec.dp_co_no,comm_rec.tco_no) && 
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
		sys_err("Error in cudp During (DBFIND)", cc, PNAME);
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
		print_at (0,0, ML (mlStdMess110), scn);

		/*-----------------------
		| Department Selection. |
		-----------------------*/
		sprintf (err_str, " %s ", ML (mlSjMess043));
		rv_pr (err_str,29,0,1);
		move (0,1);
		line (80);

		box (0,3,80,2);

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
