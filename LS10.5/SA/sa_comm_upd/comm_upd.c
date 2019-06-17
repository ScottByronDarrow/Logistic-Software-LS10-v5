/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_comm_upd.c )                                  |
|  Program Desc  : ( Sales commission update.                       ) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (07/07/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|  Comments      :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: comm_upd.c,v $
| Revision 5.5  2002/07/17 09:57:46  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/02/21 07:04:16  kaarlo
| S/C 00789. Updated to use "update" i/o "read" for find_rec of sacl and sach.
|
| Revision 5.3  2002/02/21 06:58:59  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.2  2001/08/09 09:16:52  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:21  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:32  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:55  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:24  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  2000/02/07 05:16:12  scott
| Updated to add sort_delete that did not exist. Updated from Trevor.
|
| Revision 1.10  1999/12/06 01:35:24  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/11 07:12:56  scott
| Updated for warning errors.
|
| Revision 1.8  1999/11/11 07:10:53  scott
| Updated to remove suin as not used.
|
| Revision 1.7  1999/10/16 01:11:21  nz
| Updated for pjulmdy routines
|
| Revision 1.6  1999/10/12 05:18:31  scott
| Updated for get_mend and get_mbeg
|
| Revision 1.5  1999/09/29 10:12:47  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/17 07:27:32  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.3  1999/09/16 02:01:51  scott
| Updated from Ansi Project.
|
| Revision 1.2  1999/06/18 09:39:20  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: comm_upd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_comm_upd/comm_upd.c,v 5.5 2002/07/17 09:57:46 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sa_mess.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <get_lpno.h>

#include "schema"

#define	CF			comma_fmt

char 	*data = "data",
		*exsf2 = "exsf2";

struct commRecord   comm_rec;
struct esmrRecord   esmr_rec;
struct cumrRecord   cumr_rec;
struct cuinRecord   cuin_rec;
struct cuhdRecord   cuhd_rec;
struct exsfRecord   exsf_rec;
struct saclRecord   sacl_rec;
struct sachRecord   sach_rec;

char	*_sort_read(FILE *srt_fil);
char	*srt_offset[256];

char	*sptr; 

char	prev_sman [3], 
		curr_sman [3],
		prev_desc[41],
		prev_inv [9], 	
		curr_inv [9];
		
FILE	*fsort, *fout;

int 	envDbCo = 0,
		envDbFind = 0;

char    branchNo[3];

int		SA_COMM_PAYMENT = 1;
int		SA_COMMISSION 	= 0;

/*====================================
|	Screen Input Variables			 |
====================================*/

struct {
	   char 	inp_start_sman [3];
	   char 	inp_end_sman [3];
	   char		sman_desc [41];
	   char		eman_desc [41];
	   char		det_sum_desc [41];
	   long 	e_hash;
	   long     s_hash;
	   long 	e_date;
	   long     s_date;
       char     disppr[2];
       char     systemDate[11];
       int      lpno;
	   char		dummy [2];
	   } local_rec;

int	lpno;

static	struct	var	vars[] =
{
	{1, LIN, "start_sman",	 4, 24, CHARTYPE,
		"UU", "        ",
		" ", " ", "Start Sales Person ", "Default is ALL",
		YES, NO, JUSTLEFT, "", "", local_rec.inp_start_sman}, 
	{1, LIN, "start_sman_desc",	 4, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Salesman Name", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sman_desc},
	{1, LIN, "end_sman",	 5, 24, CHARTYPE,
		"UU", "        ",
		" ", "0", "End Sales Person   ", "Default is ALL",
		YES, NO, JUSTLEFT, "", "", local_rec.inp_end_sman}, 
	{1, LIN, "end_sman_desc",	 5, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "","Salesman Name", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.eman_desc},
	{1, LIN, "start_date", 	7, 24, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Start Date         ", " Default is Start of month. ", 
		 NO, NO,  JUSTLEFT, " ", "", (char *)&local_rec.s_date}, 
	{1, LIN, "end_date", 	7, 80, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "End Date           ", " Default is Date today.  ", 
		 NO, NO,  JUSTLEFT, " ", "", (char *)&local_rec.e_date}, 
	{1, LIN, "lpno",	8, 24, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer ", "Printer Number     ",
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <std_decs.h>

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/

void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void InitOutput (void);
int heading (int scn);
void SetDefaults (void);
void ProcessData (void);
void PrintData (void);
void search_ssman (char *key_val);
char *_sort_read (FILE *srt_fil);

/*============================
|	Main Processing Routine  |
==============================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	SETUP_SCR ( vars );
	init_scr ();
	set_tty ();
	set_masks ();

	envDbCo = atoi( get_env("DB_CO") );
	envDbFind  = atoi( get_env("DB_FIND") );

	sptr = chk_env( "SA_COMM_PAYMENT" );
	SA_COMM_PAYMENT = ( sptr == ( char *)0 ) ? TRUE : atoi( sptr );

	sptr = chk_env( "SA_COMMISSION" );
	SA_COMMISSION = ( sptr == ( char *)0 ) ? FALSE : atoi( sptr );

	OpenDB();

	strcpy (branchNo, ( envDbCo ) ? comm_rec.est_no : " 0" );

	swide();
	clear();

	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		prog_status = FALSE;
		init_vars( 1 );	

		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;
	
		/*============================
		| Process Orders in Database.|
		============================*/
		InitOutput();

		ProcessData();

		PrintData();

		fprintf(fout,".EOF\n");
		pclose(fout);
	}
	shutdown_prog();
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

/*======================
| Open Database Files. |
======================*/

void
OpenDB (void)
{
	abc_dbopen(data);
	abc_alias ( exsf2, exsf );
	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec ( exsf, exsf_list, EXSF_NO_FIELDS, "exsf_hhsf_hash" );
	open_rec ( sacl, sacl_list, SACL_NO_FIELDS, "sacl_sach_hash" );
	open_rec ( sach, sach_list, SACH_NO_FIELDS, "sach_id_no" );
	open_rec ( cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash" );
	open_rec ( cuin, cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash" );
	open_rec ( cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcp_hash" );
	open_rec ( esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no" );
	open_rec ( exsf2, exsf_list, EXSF_NO_FIELDS, "exsf_id_no" );

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
}

/*=======================
| Close Database Files. |
=======================*/

void
CloseDB (void)
{
	abc_fclose ( exsf );
	abc_fclose ( sacl );
	abc_fclose ( sach );
	abc_fclose ( cumr );
	abc_fclose ( cuin );
	abc_fclose ( cuhd );
	abc_fclose ( esmr );
	abc_fclose ( exsf2 );
	abc_dbclose(data);
}

int
spec_valid (
 int    field)
{
 	/*============================= 
  	| Validate Starting Salesman  | 
  	=============================*/
	if (LCHECK("start_sman")) 
	{
		if (dflt_used)
		{
			sprintf (local_rec.inp_start_sman,"%-2.2s","  ");
			sprintf (local_rec.inp_end_sman,  "%-2.2s","~~");

			sprintf (local_rec.sman_desc, "%-40.40s", ML ("All salesmen have been selected."));
			sprintf (local_rec.eman_desc, "%-40.40s", ML ("All salesmen have been selected."));

			FLD("end_sman") = NA;

			DSP_FLD ("start_sman");
			DSP_FLD ("end_sman");
			DSP_FLD ("start_sman_desc");
			DSP_FLD ("end_sman_desc");
			return(0);
		}
		FLD("end_sman") = YES;
		
		if (prog_status != ENTRY && 
		    strncmp(local_rec.inp_end_sman,"~~",2) && 
		    strcmp(local_rec.inp_start_sman,local_rec.inp_end_sman) > 0)
		{
			print_mess (ML ("Start must be less than end."));
			sleep(2);
			return(1);
		}

		if (SRCH_KEY)
		{
			search_ssman(temp_str);
			return(0);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,local_rec.inp_start_sman);

		cc = find_rec ("exsf2",&exsf_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess (ML("Salesman not found."));
			sleep(2);
			return(1);
		}
		sprintf (local_rec.inp_start_sman, "%2.2s", exsf_rec.salesman_no);
		strcpy (local_rec.sman_desc,exsf_rec.salesman);
		local_rec.s_hash = exsf_rec.hhsf_hash;
		
		DSP_FLD ("start_sman");
		DSP_FLD ("start_sman_desc");

		return(0);
	}

	/*======================== 
	| Validate End Salesman  | 
	========================*/
	if (LCHECK("end_sman")) 
	{
		if (dflt_used)
		{
			sprintf (local_rec.inp_end_sman,"%-2.2s","~~");
			sprintf (local_rec.sman_desc, "%-40.40s",ML ("All salesmen have been selected."));
			cc = find_rec ( "exsf2",&exsf_rec,LAST,"r" );
			DSP_FLD ("end_sman");
			DSP_FLD ("end_sman_desc");
			cc = find_rec ("exsf2",&exsf_rec,LAST,"r" );
			local_rec.e_hash = exsf_rec.hhsf_hash;
			return(0);
		}

		if (SRCH_KEY)
		{
			search_ssman(temp_str);
			return(0);
		}

		if (strncmp(local_rec.inp_start_sman,"  ",2) == 0 && 
		    strncmp(local_rec.inp_end_sman,"~~",2)) 
		{
			print_mess (ML ("All salesmen have been selected."));
			sleep(2);
			return(1);
		}

		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,local_rec.inp_end_sman);
		cc = find_rec ("exsf2",&exsf_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess (ML("Salesman not found."));
			sleep(2);
			return(1);
		}
		sprintf (local_rec.inp_end_sman,"%2.2s",exsf_rec.salesman_no);
		strcpy (local_rec.eman_desc,exsf_rec.salesman);

		if (strcmp(local_rec.inp_start_sman,local_rec.inp_end_sman) > 0)
		{
			errmess(ML ("End must be greater than start."));
			sleep(2);
			return(1);
		}
		local_rec.e_hash = exsf_rec.hhsf_hash;
		DSP_FLD ("end_sman");
		DSP_FLD ("end_sman_desc");
		return(0);
	}

 	/*-------------------------------
  	| Validate Date Entered 		| 
  	-------------------------------*/
	if (LCHECK ("start_date"))
	{
		if (dflt_used) 
			local_rec.s_date = MonthStart (StringToDate (local_rec.systemDate));

		if ( local_rec.s_date > StringToDate( local_rec.systemDate ))
		{
			print_mess( ML("Start must be less than end."));
			sleep( 2 );
			return ( 1 );
		}

		return ( 0 );
	}

 	/*-------------------------------
  	| Validate Date Entered 		| 
  	-------------------------------*/
	if (LCHECK ("end_date"))
	{
		if (dflt_used) 
			local_rec.e_date = StringToDate (local_rec.systemDate);

		if ( local_rec.e_date < local_rec.s_date )
		{
			print_mess(ML ("End date must not be later than today's date and not earlier than start date"));
			sleep ( 2 );
			return ( 1 );
		}
		
		if ( local_rec.e_date > StringToDate( local_rec.systemDate ))
		{
			print_mess( ML("End must be greater than start."));
			sleep( 2 );
			return ( 1 );
		}
		DSP_FLD ("end_date");
		return ( 0 );
	}

	/*-------------------------
	| Validate Printer Number |
	-------------------------*/
	if ( LCHECK("lpno") )
	{
		if(dflt_used)
		{
			local_rec.lpno = 1;
			return(0);
		}	

		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess(ML("Invalid printer."));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		return(0);
	}
	return (EXIT_SUCCESS);
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/

void
InitOutput (void)
{
	char	wk_date[2][11];

	strcpy (wk_date[0], DateToString (local_rec.s_date));
	strcpy (wk_date[1], DateToString (local_rec.e_date));

	dsp_screen(" Printing Sales Commissions",
					comm_rec.co_no,comm_rec.co_name);

	/*------------------
	| Open format file |
	------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.lpno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L%d\n", 100);
	fprintf (fout, ".12\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ESALESMAN COMMISSION UPDATE AUDIT\n");
	fprintf (fout, ".CNOTE : This report should be used to post commissions to accounts payable manually\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime());
	strcpy (err_str, ".C Start Date : %10.10s  /  End Date : %10.10s  /  Start Salesperson : %2.2s  /  End Salesperson : %2.2s\n");
	fprintf (fout, err_str, 
			wk_date[0], wk_date[1],
			local_rec.inp_start_sman, local_rec.inp_end_sman);
	
	fprintf (fout, ".R=====");
	fprintf (fout, "===========================================");
	fprintf (fout, "=================================================\n");

	fprintf (fout, "=====");
	fprintf (fout, "===========================================");
	fprintf (fout, "=================================================\n");

	fprintf (fout, "| SM ");
	fprintf (fout, "|      SALES PERSON DESCRIPTION.           ");
	fprintf (fout, "|     INVOICE   ");
	fprintf (fout, "|     AMOUNT    ");
	fprintf (fout, "|   COMMISSION  |\n");
	
	fprintf (fout, "| NO ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|      TOTAL    ");
	fprintf (fout, "|    COLLECTED  ");
	fprintf (fout, "|    TO PAY     |\n");

	fprintf (fout, "|----");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------|\n");
}

int
heading (
 int    scn)
{
	if (restart) 
		return (EXIT_FAILURE);
	
	swide();
	clear();

	strcpy (err_str,ML(" Sales Commission Update "));

	rv_pr( err_str, (130 - strlen( err_str )) / 2, 0, 1);

	move (0,1);
	line(132);

	switch (scn)
	{
	case	1:
		box(0,3,132,5);
		move (1,6);
		line(131);
		move (0,21);
		line(132);
		break;

	default:
		break;
	}

	line_cnt	=	0;
	scn_write (scn);

	move (0,23);
	print_at(22,0,ML ("Co : %s - %s "),comm_rec.co_no,comm_rec.co_name);

    return (EXIT_FAILURE);
}

void
SetDefaults (void)
{
	strcpy ( local_rec.inp_start_sman,"  ");
	strcpy ( local_rec.inp_end_sman,"~~");
	strcpy ( local_rec.sman_desc,"All Salesmen");
	strcpy ( local_rec.eman_desc,"All Salesmen");
	local_rec.s_date = StringToDate (local_rec.systemDate);
	local_rec.e_date = StringToDate (local_rec.systemDate);
	local_rec.lpno = 1;
	DSP_FLD ("start_sman");
	DSP_FLD ("start_date");
	DSP_FLD ("end_date");
	DSP_FLD ("start_sman_desc");
	DSP_FLD ("end_sman");
	DSP_FLD ("end_sman_desc");
	FLD("end_sman") 	= NA;
	FLD("lpno") 		= NA;

}

void
ProcessData (void)
{
    char    sort_buffer[125];

	if ((fsort = sort_open ("commission")) == NULL)
		sys_err ("Error in opening commission file",cc,PNAME);

	if ( !strcmp (local_rec.inp_start_sman, "  "))
	{
		strcpy ( local_rec.inp_start_sman, "  ");
		strcpy ( local_rec.inp_end_sman,   "~~");
	}

	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no, local_rec.inp_start_sman);
	cc = find_rec (exsf2, &exsf_rec, GTEQ, "r");	

	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.co_no))
	{
		if (strcmp (exsf_rec.salesman_no, local_rec.inp_end_sman) > 0)
			break;
	
		if (exsf_rec.com_status [0] != 'C')
		{
			cc = find_rec (exsf2, &exsf_rec, NEXT, "r");	
			continue;
		}
 
		dsp_process ("Salesman", exsf_rec.salesman_no);

		sach_rec.hhsf_hash = exsf_rec.hhsf_hash;
		sach_rec.hhcu_hash	=	0L;
		sach_rec.hhci_hash	=	0L;
		cc = find_rec (sach, &sach_rec, GTEQ, "u");
		while (!cc && sach_rec.hhsf_hash == exsf_rec.hhsf_hash)	
		{
			cumr_rec.hhcu_hash = sach_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock ("sach");
				cc = find_rec (sach, &sach_rec, NEXT, "u");
				continue;
			}

			cuin_rec.hhci_hash = sach_rec.hhci_hash;
			cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock ("sach");
				cc = find_rec (sach, &sach_rec, NEXT, "u");
				continue;
			}

			if (SA_COMM_PAYMENT)
			{
				sacl_rec.sach_hash =  sach_rec.sach_hash; 
				cc = find_rec (sacl, &sacl_rec, GTEQ, "u");
				while (!cc	&& sacl_rec.sach_hash ==  sach_rec.sach_hash)
				{
					if ((sacl_rec.rec_date > local_rec.e_date ||
						sacl_rec.rec_date < local_rec.s_date) ||
						sacl_rec.rec_amt == 0.00 || sacl_rec.status[0] != '0')
					{
						abc_unlock ("sacl");
						cc = find_rec (sacl, &sacl_rec, NEXT, "u");
						continue;
					}
	
					cuhd_rec.hhcp_hash = sacl_rec.hhcp_hash;
					cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "r");
					if (cc)
					{
						abc_unlock (sacl);
						cc = find_rec (sacl, &sacl_rec, NEXT, "u");
						continue;
					}

					sprintf (sort_buffer,
                             "%s%c%s%c%s%c%ld%c%f%c%s%c%f%c%ld%c%f%c%f%c%s%c%ld%c%s\n",
							 exsf_rec.salesman_no,	1, /* spt_offset = 0 	*/ /*          3 chars */
							 cumr_rec.dbt_acronym,	1, /* spt_offset = 1 	*/ /*         10 chars */
							 cuin_rec.inv_no,		1, /* spt_offset = 2 	*/ /*          9 chars */
							 cuin_rec.date_of_inv,	1, /* spt_offset = 3 	*/ /* long:   11 chars */
							 sach_rec.inv_amt,		1, /* spt_offset = 4 	*/ /* double: 16 chars */
							 cuhd_rec.receipt_no,	1, /* spt_offset = 5 	*/ /*          9 chars */
							 sacl_rec.rec_amt,		1, /* spt_offset = 6 	*/ /* double: 16 chars */
							 sacl_rec.rec_date,		1, /* spt_offset = 7 	*/ /* long:   11 chars */
							 sach_rec.com_rate,		1, /* spt_offset = 8 	*/ /* float:   8 chars */
							 sacl_rec.com_amt,		1, /* spt_offset = 9 	*/ /* double: 16 chars */
							 sach_rec.sale_flag,    1, /* spt_offset = 10 	*/ /*          2 chars */
							 exsf_rec.hhsf_hash,    1, /* spt_offset = 11 	*/ /* long:   11 chars */
							 " ");                                             /*          1 char  */
                                                                               /* '\n'     1 char  */
                                                                               /* term chr 1 char  */
                                                                               /* total  125 chars */
                    sort_save (fsort, sort_buffer);
					strcpy (sacl_rec.status, "1");
					cc = abc_update ("sacl", &sacl_rec);
					if (cc)
						file_err (cc, "sacl", "DBUPDATE");

					cc = find_rec (sacl, &sacl_rec, NEXT, "u");
				}
			}
			else
			{
				if (cuin_rec.date_of_inv > local_rec.e_date ||
					cuin_rec.date_of_inv < local_rec.s_date ||
					sach_rec.status[0] != '0') 
				{
					abc_unlock ("sach");
					cc = find_rec (sach, &sach_rec, NEXT, "u");
					continue;
				}
                sprintf (sort_buffer,
				         "%s%c%s%c%s%c%ld%c%f%c%s%c%f%c%ld%c%f%c%f%c%s%c%ld%c%s\n",
                         exsf_rec.salesman_no,	1, /* spt_offset = 0 	*/ /*          3 chars */
                         cumr_rec.dbt_acronym,	1, /* spt_offset = 1 	*/ /*         10 chars */
                         cuin_rec.inv_no,		1, /* spt_offset = 2 	*/ /*          9 chars */
                         cuin_rec.date_of_inv,	1, /* spt_offset = 3 	*/ /* long:   11 chars */
                         sach_rec.inv_amt,		1, /* spt_offset = 4 	*/ /* double: 16 chars */
                         "PAY SALE",			1, /* spt_offset = 5 	*/ /*          8 chars */
                         sach_rec.com_val,		1, /* spt_offset = 6 	*/ /* double: 16 chars */
                         cuin_rec.date_of_inv,	1, /* spt_offset = 7 	*/ /* long:   11 chars */
                         sach_rec.com_rate,		1, /* spt_offset = 8 	*/ /* float:   8 chars */
                         sach_rec.com_val,		1, /* spt_offset = 9 	*/ /* double: 16 chars */
                         sach_rec.sale_flag,	1, /* spt_offset = 10 	*/ /*          2 chars */
                         exsf_rec.hhsf_hash,	1, /* spt_offset = 11 	*/ /* long:   11 chars */
                         " ");                                             /*          1 char  */
                                                                           /* '\n'     1 char  */
                                                                           /* term chr 1 char */
                                                                           /* total  124 chars */ 
                sort_save (fsort, sort_buffer);
    		}
			strcpy (sach_rec.status, "1");
			cc = abc_update ("sach", &sach_rec);
			if (cc)
				file_err (cc, "sach", "DBUPDATE");

			cc = find_rec (sach, &sach_rec, NEXT, "u");
		}
		cc = find_rec (exsf2, &exsf_rec, NEXT, "r");		
	}
}

void
PrintData (void)
{
	int		i;

	char	wk_amt[4][15];

	double	TotalSman [3];
	double	TotalGrand [3];

	int		FirstTime	=	TRUE;

	strcpy (prev_sman, "");
	strcpy (prev_desc, "");

	for (i = 0; i < 3; i++)
	{
		TotalSman [i]	=	0.00;
		TotalGrand [i]	=	0.00;
	}
	fsort = sort_sort(fsort,"commission");
	sptr = _sort_read(fsort);
	while (sptr != (char *)0)
	{
		exsf_rec.hhsf_hash	=	atol (srt_offset[11]);
		cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "exsf", "DBFIND");

		sprintf (curr_sman, "%-2.2s", srt_offset[0]);		
		
		if (strcmp(prev_sman,curr_sman))
		{
			if (!FirstTime)
			{
				strcpy (wk_amt[0],CF (TotalSman [0],"NNN,NNN,NNN.NN"));
				strcpy (wk_amt[1],CF (TotalSman [1],"NNN,NNN,NNN.NN"));
				strcpy (wk_amt[2],CF (TotalSman [2],"NNN,NNN,NNN.NN"));

				fprintf (fout, "| %2.2s ", prev_sman);
				fprintf (fout, "| %40.40s ",prev_desc);
				fprintf (fout, "|%14.14s ",	wk_amt[0]);
				fprintf (fout, "|%14.14s ",	wk_amt[1]);
				fprintf (fout, "|%14.14s |\n",	wk_amt[2]);
				TotalSman [0]	=	0.00;
				TotalSman [1]	=	0.00;
				TotalSman [2]	=	0.00;
			}
			FirstTime = FALSE;
		} 

		strcpy (wk_amt[0],CF (DOLLARS (atof (srt_offset[4])),"NNN,NNN,NNN.NN"));
		strcpy (wk_amt[1],CF (DOLLARS (atof (srt_offset[6])),"NNN,NNN,NNN.NN"));
		strcpy (wk_amt[2],CF (atof (srt_offset[8]),"NNN,NNN.NN"));
		strcpy (wk_amt[3],CF (DOLLARS (atof (srt_offset[9])),"NNN,NNN,NNN.NN"));

		TotalSman [0]	+=	DOLLARS (atof (srt_offset[4]));
		TotalSman [1]	+=	DOLLARS (atof (srt_offset[6]));
		TotalSman [2]	+=	DOLLARS (atof (srt_offset[9]));
		TotalGrand [0]	+=	DOLLARS (atof (srt_offset[4]));
		TotalGrand [1]	+=	DOLLARS (atof (srt_offset[6]));
		TotalGrand [2]	+=	DOLLARS (atof (srt_offset[9]));

		strcpy (prev_sman, curr_sman);	
		strcpy (prev_desc, exsf_rec.salesman);	
	
		sptr = _sort_read(fsort);
	}
	sort_delete (fsort, "commission");
	strcpy (wk_amt[0],CF (TotalSman [0],"NNN,NNN,NNN.NN"));
	strcpy (wk_amt[1],CF (TotalSman [1],"NNN,NNN,NNN.NN"));
	strcpy (wk_amt[2],CF (TotalSman [2],"NNN,NNN,NNN.NN"));

	fprintf (fout, "| %2.2s ", prev_sman);
	fprintf (fout, "| %40.40s ",prev_desc);
	fprintf (fout, "|%14.14s ",	wk_amt[0]);
	fprintf (fout, "|%14.14s ",	wk_amt[1]);
	fprintf (fout, "|%14.14s |\n",	wk_amt[2]);
	strcpy (wk_amt[0],CF (TotalGrand [0],"NNN,NNN,NNN.NN"));
	strcpy (wk_amt[1],CF (TotalGrand [1],"NNN,NNN,NNN.NN"));
	strcpy (wk_amt[2],CF (TotalGrand [2],"NNN,NNN,NNN.NN"));

	fprintf (fout, "|----");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------|\n");

	fprintf (fout, "| *** GRAND TOTAL ***                           ");
	fprintf (fout, "|%14.14s ",	wk_amt[0]);
	fprintf (fout, "|%14.14s ",	wk_amt[1]);
	fprintf (fout, "|%14.14s |\n",	wk_amt[2]);
}

void
search_ssman (
 char*  key_val)
{
    work_open();
	save_rec("#Sm Code","#Salesman Name");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec ("exsf2",&exsf_rec,GTEQ,"r");
    while (!cc && !strcmp(exsf_rec.co_no,comm_rec.co_no) 
			&& !strncmp(exsf_rec.salesman_no,key_val,strlen(key_val)))
   	{
		cc = save_rec(exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec ("exsf2",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec ("exsf2",&exsf_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in exsf During (DBFIND)",cc,PNAME);
}

/*========================================
| Save offsets for each numerical field. |
========================================*/
/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char*
_sort_read (
 FILE*  srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset[0] = sptr;

	tptr = sptr;
	while (fld_no < 12)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		srt_offset[fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}
