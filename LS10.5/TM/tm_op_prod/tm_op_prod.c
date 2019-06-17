/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tm_op_prod.c,v 5.3 2001/11/14 08:16:54 scott Exp $
|  Program Name  : (tm_op_prod.c)
|  Program Desc  : (Telemarketing Operator Productivity Display)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 06/08/91         |
|---------------------------------------------------------------------|
| $Log: tm_op_prod.c,v $
| Revision 5.3  2001/11/14 08:16:54  scott
| Updated to convert to app.schema
| Clean code
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tm_op_prod.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_op_prod/tm_op_prod.c,v 5.3 2001/11/14 08:16:54 scott Exp $";

#define	X_OFF	0
#define	Y_OFF	3
#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_tm_mess.h>

FILE	*fsort;
char	*sptr;

char	*UNDER_LINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

#include	"schema"

struct commRecord	comm_rec;
struct tmopRecord	tmop_rec;
struct tmohRecord	tmoh_rec;
struct tmcfRecord	tmcf_rec;
struct tmchRecord	tmch_rec;

int	no_of_calls [2];
float 	camp_time [2];
float 	avg_call_time [2];
double	actual_sales [2];

extern	int		TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	campgn [5];
	char	campgn_name [61];
	char	st_op [15];
	char	st_op_name [41];
	char	end_op [15];
	char	end_op_name [41];
	int	lpno;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "campgn", 4, 2, CHARTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign Number  ", " Enter Campaign. Default for ALL ", 
		NO, NO, JUSTRIGHT, "0123456789", "", local_rec.campgn}, 
	{1, LIN, "campgn_name", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.campgn_name}, 
	{1, LIN, "st_op", 6, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", " ","Start Operator   ", "Enter Start Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.st_op}, 
	{1, LIN, "st_op_name", 6, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_op_name}, 
	{1, LIN, "end_op", 7, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", " ", "End Operator     ", "Enter End Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.end_op}, 
	{1, LIN, "end_op_name", 7, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_op_name}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	SrchTmpf 		(char *);
void 	SrchTmop 		(char *);
void 	Process 		(void);
void 	ProcessSorted 	(void);
void 	CalculateStats 	(void);
int 	heading 		(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
	int argc, 
	char *argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/

	OpenDB ();

	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok 	= TRUE;
		entry_exit 	= TRUE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);	

		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (!restart) 
			Process ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);   
}

void
OpenDB (
	void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmch, tmch_list, TMCH_NO_FIELDS, "tmch_hhop_hash");
	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (tmoh, tmoh_list, TMOH_NO_FIELDS, "tmoh_id_no");
}

void
CloseDB (
	void)
{
	abc_fclose (tmcf);
	abc_fclose (tmch);
	abc_fclose (tmop);
	abc_fclose (tmoh);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	/*--------------------------
	| Validate Campaign Number |
	--------------------------*/
	if (LCHECK ("campgn"))
	{
		if (SRCH_KEY)
		{
			SrchTmpf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.campgn, "ALL ");
			strcpy (local_rec.campgn_name, ML (" LL CAMPAIGNS"));
			
			DSP_FLD ("campgn");
			DSP_FLD ("campgn_name");
			return (EXIT_SUCCESS);
		}

		strcpy (tmcf_rec.co_no, comm_rec.co_no);
		tmcf_rec.campaign_no = atoi (local_rec.campgn);
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			/*No Such Campaign */
			print_mess (ML (mlTmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (err_str, "%s, ", clip (tmcf_rec.c_name1));
		strcat (err_str, clip (tmcf_rec.c_name2));
		sprintf (local_rec.campgn_name, "%-60.60s",  err_str);
		
		DSP_FLD ("campgn_name");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_op"))
	{
		if (last_char == FN16)
		{
			restart = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.st_op, "              ");
			sprintf (local_rec.st_op_name, 
				"%-14.14s", 
				"First Operator");
			DSP_FLD ("st_op");
			DSP_FLD ("st_op_name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.st_op);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			/* Operator Not Found On File */
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.st_op_name, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("st_op_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_op"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.end_op, "~~~~~~~~~~~~~~");
			sprintf (local_rec.end_op_name, 
				"%-14.14s", 
				"Last Operator");
			DSP_FLD ("end_op");
			DSP_FLD ("end_op_name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.end_op);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			/* Operator Not Found On File */
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.end_op_name, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("end_op_name");
		return (EXIT_SUCCESS);
	}
					
	return (EXIT_SUCCESS);
}

/*==========================================
| Search routine for Campaign Header File. |
==========================================*/
void
SrchTmpf (
	char *key_val)
{
	work_open ();
	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#Campaign No.", "#Campaign Description.");
	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
		tmcf_rec.campaign_no >= atoi (key_val))
	{
		sprintf (err_str, "%4d", tmcf_rec.campaign_no);
		cc = save_rec (err_str, tmcf_rec.c_name1);
		if (cc)
			break;

		cc = find_rec (tmcf, &tmcf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (temp_str);
	cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmcf, "DBFIND");
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmop (
	char *key_val)
{
	work_open ();
	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", key_val);
	save_rec ("#Operator I.D.", "#Operator Full Name.");
	cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
	while (!cc && !strcmp (tmop_rec.co_no, comm_rec.co_no) &&
		      !strncmp (tmop_rec.op_id, key_val, strlen (key_val)))
	{
		cc = save_rec (tmop_rec.op_id, tmop_rec.op_name);
		if (cc)
			break;

		cc = find_rec (tmop, &tmop_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", temp_str);
	cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmop, "DBFIND");
}

void
Process (
	void)
{
	char	sort_str [256];

 	fsort = sort_open ("op_prod");

	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	if (!strcmp (local_rec.campgn, "ALL ")) 
		tmcf_rec.campaign_no = 0;
	else
		tmcf_rec.campaign_no = atoi (local_rec.campgn);
	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmcf_rec.co_no, comm_rec.co_no))
	{
		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.st_op);
		cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
		while (!cc && 
	       	!strcmp (tmop_rec.co_no, comm_rec.co_no) &&
	       	strcmp (tmop_rec.op_id, local_rec.end_op) <= 0)
		{
	
			if (strcmp (local_rec.campgn, "ALL ")) 
			{
				if (atoi (local_rec.campgn) != tmcf_rec.campaign_no)
				{
					cc = find_rec (tmop, &tmop_rec, NEXT, "r");
					continue;
				}
			}
	
			CalculateStats ();

			tmoh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
			tmoh_rec.hhop_hash = tmop_rec.hhop_hash;
			cc = find_rec (tmoh, &tmoh_rec, COMPARISON, "r");
			if (cc || tmoh_rec.campgn_time <= 0L)
				camp_time [0] = 0.00;
			else
				camp_time [0] = (float) (tmoh_rec.campgn_time / 60.00);

			if (camp_time [0] > 0.00 || no_of_calls [0] > 0)
			{
				sprintf (sort_str, 
					"%05d %-14.14s %8ld %8ld %-40.40s %7.2f %4d %7.2f \n", 
					tmop_rec.campaign_no, 
					tmop_rec.op_id, 
					tmcf_rec.hhcf_hash, 
					tmop_rec.hhop_hash, 
					tmop_rec.op_name, 
					camp_time [0], 
					no_of_calls [0], 
					actual_sales [0]);
	
				sort_save (fsort, sort_str);
			}
			
			cc = find_rec (tmop, &tmop_rec, NEXT, "r");
		}
		if (strcmp (local_rec.campgn, "ALL ")) 
			break;
	
		cc = find_rec (tmcf, &tmcf_rec, NEXT, "r");
	}
	ProcessSorted ();
	return;
}

void
ProcessSorted (
	void)
{
	char	*sptr;
	char	data_str [256];
	int	first_time = TRUE;
	int	data_found = FALSE;
	long	curr_camp;
	long	prev_camp = 0L;

	abc_selfield (tmcf, "tmcf_hhcf_hash");
	abc_selfield (tmop, "tmop_hhop_hash");
	
	no_of_calls [1] = 0;
	camp_time [1] = 0.00;
	avg_call_time [1] = 0.00;
	actual_sales [1] = 0.00;
	
	/*-----------------
	| Open Dsp window |
	-----------------*/
	Dsp_prn_open (0, 3, 15, 
		     "                        Telemarketing Call Back Dsiplay                         ", (char *) 0, 
	            (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);

	Dsp_saverec ("  Operator   |Operator Name           |Calls|Cmpgn Tme|Avg Tme/Call| Sales    ");
	Dsp_saverec ("");
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]  ");
	
	fsort = sort_sort (fsort, "op_prod");

	sptr = sort_read (fsort);

	while (sptr)
	{
		curr_camp = atol (sptr + 21);

		if (first_time)
			prev_camp = curr_camp;

		cc = find_hash (tmop, &tmop_rec, COMPARISON, "r", atol (sptr + 30));
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		cc = find_hash (tmcf, &tmcf_rec, COMPARISON, "r", atol (sptr + 21));
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		camp_time [0] = (float) atof (sptr + 81);
		no_of_calls [0] = atoi (sptr + 89);
		actual_sales [0] = atof (sptr + 94);
		no_of_calls [1] += no_of_calls [0];

		if (curr_camp != prev_camp || first_time)
		{
			if (!first_time)
			{
				if (no_of_calls [1] == 0 || camp_time [1] == 0.00)
					avg_call_time [1] = 0.00;
				else
					avg_call_time [1] = camp_time [1] / (float)no_of_calls [1];

				sprintf (data_str, 
					" %-37.37s^E%5d^E %7.2f ^E %10.2f ^E %8.2f ", 
					" TOTAL FOR CAMPAIGN ", 
					no_of_calls [1], 
					camp_time [1], 
					avg_call_time [1], 
					actual_sales [1]);
		
				Dsp_saverec (UNDER_LINE);
				Dsp_saverec (data_str);
				Dsp_saverec (UNDER_LINE);

				no_of_calls [1] = 0;
				camp_time [1] = 0.00;
				avg_call_time [1] = 0.00;
				actual_sales [1] = 0.00;
			}

			data_found = TRUE;
			sprintf (data_str, 
				"^1 %4d  %-40.40s ^6", tmcf_rec.campaign_no, tmcf_rec.c_name1);

			Dsp_saverec (data_str);

			prev_camp = curr_camp;
		}

		camp_time [0] = (float) atof (sptr + 81);
		no_of_calls [0] = atoi (sptr + 89);
		actual_sales [0] = atof (sptr + 94);
		first_time = FALSE;

		if (no_of_calls [0] == 0 || camp_time [0] == 0.00)
			avg_call_time [0] = 0.00;
		else
			avg_call_time [0] = camp_time [0] / (float)no_of_calls [0];

		sprintf (data_str, 
			" %-12.12s^E %-23.23s^E%5d^E %7.2f ^E %10.2f ^E %8.2f ", 
			sptr + 6, 
			sptr + 39, 
			no_of_calls [0], 
			camp_time [0], 
			avg_call_time [0], 
			actual_sales [0]);

		camp_time [1] += camp_time [0];

		Dsp_saverec (data_str);

		sptr = sort_read (fsort);
	}

	if (data_found)
	{
		if (no_of_calls [1] == 0 || camp_time [1] == 0.00)
			avg_call_time [1] = 0.00;
		else
			avg_call_time [1] = camp_time [1] / (float)no_of_calls [1];

		sprintf (data_str, 
			" %-37.37s^E%5d^E %7.2f ^E %10.2f ^E %8.2f ", 
			" TOTAL FOR CAMPAIGN ", 
			no_of_calls [1], 
			camp_time [1], 
			avg_call_time [1], 
			actual_sales [1]);
		
		Dsp_saverec (UNDER_LINE);
		Dsp_saverec (data_str);
		Dsp_saverec (UNDER_LINE);
	}

	sort_delete (fsort, "op_prod");

	Dsp_srch ();

	Dsp_close ();

	abc_selfield (tmcf, "tmcf_id_no");
	abc_selfield (tmop, "tmop_id_no");
}

void
CalculateStats (
	void)
{

	no_of_calls [0] = 0;
	actual_sales [0] = 0.00;

	cc = find_hash (tmch, &tmch_rec, GTEQ, "r", tmop_rec.hhop_hash);
	while (!cc && tmch_rec.hhop_hash == tmop_rec.hhop_hash)
	{
		if (tmch_rec.hhcf_hash != tmcf_rec.hhcf_hash)
		{
			cc = find_hash (tmch, &tmch_rec, NEXT, "r", tmop_rec.hhop_hash);
			continue;
		}
		no_of_calls [0]++;
		actual_sales [0] += tmch_rec.ord_value;

		cc = find_hash (tmch, &tmch_rec, NEXT, "r", tmop_rec.hhop_hash);
	}

	no_of_calls [1]   += no_of_calls [0];
	actual_sales [1]  += actual_sales [0];

	return;
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
	int scn)
{
	char	hdng_date [11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
	
		strcpy (hdng_date, DateToString (TodaysDate ()));
		rv_pr (hdng_date, 69, 0, 0);

		line_at (1,0,80);

		/* Telemarketing Operator Productivity Display. */
		rv_pr (ML (mlTmMess070), 20, 0, 1);

		box (0, 3, 80, 4);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
