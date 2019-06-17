/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( prt_quote.c ) 	                                  |
|  Program Desc  : ( Quotaion Inquiry Display                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  qthr, exsf,     ,     ,     ,     ,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Elena Cuaresma  | Date Written  : 20/10/1995       |
|---------------------------------------------------------------------|
|  Date Modified : 08/05/1996      | Modified  by  : Jiggs A. Veloz   |
|  Date Modified : 12/09/1997      | Modified  by  : Marnie I. Organo |
|  Date Modified : 23/08/1999      | Modified  by  : Alvin Misalucha  |
|                                                                     |
|  Comments      : Updated to not allow printing of quotes ON HOLD    |
|  (12/09/1997)  : Updated for Multilingual Conversion.               |
|  (23/08/1999)  : Ported to ANSI convention.						  |
|                :                                                    |
| $Log: prt_quote.c,v $
| Revision 5.2  2001/08/09 08:44:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:20  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:04  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:31  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:08:59  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/11/16 03:29:20  scott
| Updated for warning due to usage of -Wall flags on compiler.
|
| Revision 1.11  1999/09/29 10:12:32  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/22 05:18:28  scott
| Updated from Ansi project.
|
| Revision 1.9  1999/09/14 04:05:38  scott
| Updated for Ansi.
|
| Revision 1.8  1999/09/10 12:37:28  ana
| (10/09/1999) SC1962 Corrected find_rec of qthr.
|
| Revision 1.7  1999/06/18 06:12:27  scott
| Updated to add log for cvs and remove old style read_comm()
|$Log: prt_quote.c,v $
|Revision 5.2  2001/08/09 08:44:44  scott
|Updated to add FinishProgram () function
|
|Revision 5.1  2001/08/06 23:38:20  scott
|RELEASE 5.0
|
|Revision 5.0  2001/06/19 08:12:50  robert
|LS10-5.0 New Release as of 19 JUNE 2001
|
|Revision 4.0  2001/03/09 02:34:04  scott
|LS10-4.0 New Release as at 10th March 2001
|
|Revision 3.0  2000/10/10 12:18:31  gerry
|Revision No. 3 Start
|<after Rel-10102000>
|
|Revision 2.0  2000/07/15 09:08:59  gerry
|Forced Revision No Start 2.0 Rel-15072000
|
|Revision 1.12  1999/11/16 03:29:20  scott
|Updated for warning due to usage of -Wall flags on compiler.
|
|Revision 1.11  1999/09/29 10:12:32  scott
|Updated to be consistant on function names.
|
|Revision 1.10  1999/09/22 05:18:28  scott
|Updated from Ansi project.
|
|Revision 1.9  1999/09/14 04:05:38  scott
|Updated for Ansi.
|
|Revision 1.8  1999/09/10 12:37:28  ana
|(10/09/1999) SC1962 Corrected find_rec of qthr.
|                                                             |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: prt_quote.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_prt_quote/prt_quote.c,v 5.2 2001/08/09 08:44:44 scott Exp $";

#define		MAXLINES	2000

#include	<std_decs.h>
#include	<pslscr.h>		
#include	<get_lpno.h>
#include 	<qt_status.h>
#include 	<hot_keys.h>
#include 	<tabdisp.h>
#include 	<dsp_screen.h>
#include 	<assert.h>
#include 	<ml_std_mess.h>
#include 	<ml_qt_mess.h>

typedef int	BOOL;

	char	*data = "data",
			*comm = "comm",	
			*ccmr = "ccmr",	
			*comr = "comr",	
	    	*qthr = "qthr",	
	    	*exsf = "exsf";	

	char	head_str[200],
			save_key[21];

	int		save_indx;
	int		lpno = 0;

	char	branchNo[3];
	int		envDbCo = 0,
			envDbFind	 = 0;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_cc_no"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 7;
	
	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	cc_no[3];
		long	t_dbt_date;
	} comm_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_stat_flag"}
	};

	int ccmr_no_fields = 5;

	struct {
		char	ccmr_co_no[3];
		char	ccmr_est_no[3];
		char	ccmr_cc_no[3];
		long	ccmr_hhcc_hash;
		char	ccmr_stat_flag[2];
	} ccmr_rec;

	/*==================================
	| Company Master File Base Record. |
	==================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_co_adr1"},
		{"comr_co_adr2"},
		{"comr_co_adr3"},
		{"comr_stat_flag"},
		{"comr_frt_mweight"},
		{"comr_frt_min_amt"}
	};

	int comr_no_fields = 8;

	struct {
		char	co_no[3];
		char	co_name[41];
		char	co_adr[3][41];
		char	stat_flag[2];
		float	comr_frt_mweight;
		double	comr_frt_min_amt;
	} comr_rec;

	/*========================+
	 | Quotation header file. |
	 +========================*/
#define	QTHR_NO_FIELDS	11

	struct dbview	qthr_list [QTHR_NO_FIELDS] =
	{
		{"qthr_co_no"},
		{"qthr_br_no"},
		{"qthr_quote_no"},
		{"qthr_hhcu_hash"},
		{"qthr_hhqt_hash"},
		{"qthr_enq_ref"},
		{"qthr_dt_quote"},
		{"qthr_sman_code"},
		{"qthr_status"},
		{"qthr_dbt_name"},
		{"qthr_prt_name"}
	};

	struct tag_qthrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	quote_no [9];
		long	hhcu_hash;
		long	hhqt_hash;
		char	enq_ref [21];
		long	dt_quote;
		char	sman_code [3];
		char	status [3];
		char	dbt_name [41];
		char	prt_name [16];
	}	qthr_rec;

	/*=========================
	| External Salesman File. |
	=========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
		{"exsf_stat_flag"}
	};

	int exsf_no_fields = 4;

	struct {
		char	co_no[3];
		char	no[3];
		char	desc[41];
		char	stat_flag[2];
	} exsf_rec;

static int 		PrintFunc (int	key, KEY_TAB *psUnused);
static int 		RestartFunc (int key, KEY_TAB *psUnused);
static int 		ExitFunc (int key, KEY_TAB *psUnused);
static long		table_hash [MAXLINES];
static int		no_in_tab;				
static int 		positionStatusField;	

#define PRINT_KEY		'P'

	static  KEY_TAB list_keys [] =
	{
	   { "[P]rint ",		PRINT_KEY,			PrintFunc,
		"Print Line",								"A" },
	   { NULL,				FN1,				RestartFunc,
		"Exit without update.",						"A" },
	   { NULL,				FN16,				ExitFunc,
		"Exit and update the database.",			"A" },
	   END_KEYS
	};

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	quote_no[9];
	char	qt_no[9];
	char	stat_desc[21];
} local_rec;

static struct	var vars[] ={

	{1, LIN,"n_quote",3,22, CHARTYPE,
		"UUUUUUUU","          ",
		" ","", "Quote Number    :","Enter Quotation Number.",
		NO,NO, JUSTLEFT,"","",(char *)&local_rec.quote_no},

	{0,LIN,"",0,0,INTTYPE,
		"A","          ",
		" ","", "dummy"," ",
		YES,NO, JUSTRIGHT," "," ",local_rec.dummy}
};

	static char		*tableName = NULL;

/*=========================== 
| Function prototypes .     |
===========================*/
int		main			(int argc, char * argv []);
int		heading			(int);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
void	show_qthr		(char *);
void	process			(void);

static int	RestartFunc				(int key, KEY_TAB * psUnused);
static int	ExitFunc				(int key, KEY_TAB * psUnused);
static BOOL	IsFieldStatus			(char * table_buf, char * status);
static void	FieldStatusUpdate		(char * table_buf, char * status);
static void	TableLineStatusUpdate	(char * status);
static int	PrintFunc				(int key, KEY_TAB * psUnused);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 2)
	{
		print_at(0,0,ML(mlStdMess036), argv[0]);
		return (argc);
	}

	lpno = atoi(argv[1]);

	SETUP_SCR 	(vars);
	init_scr 	();
	set_tty 	();
	set_masks 	();
	
	OpenDB		();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	clear		();

	while (prog_exit == 0)
	{
		search_ok 	= 	1;
		entry_exit 	= 	0;
		edit_exit 	= 	0;
		prog_exit 	= 	0;
		restart 	= 	0;
		init_ok 	= 	1;
		init_vars 	(1);	
		heading 	(1);
		entry 		(1);   
		if (!restart && !prog_exit)
			process();
		else
		{
			if (restart)
				continue;
			else
				prog_exit = 1;
		}
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

void
OpenDB (void)
{
	abc_dbopen(data);

	open_rec(qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no2");
	open_rec(exsf, exsf_list, exsf_no_fields, "exsf_id_no");
}

void
CloseDB (void)
{
	abc_fclose(qthr);
	abc_fclose(exsf);
	abc_dbclose(data);
}

int
spec_valid (
 int	field)
{
	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK("n_quote"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.quote_no, "ALL     ");
			DSP_FLD("n_quote");
			return(0);
		}

		if (SRCH_KEY)
		{
			show_qthr(temp_str);
			strcpy (local_rec.quote_no, qthr_rec.quote_no);
			DSP_FLD ("n_quote");
			return(0);
		}

		strcpy (qthr_rec.quote_no, zero_pad(local_rec.quote_no,8));
		strcpy (local_rec.quote_no, qthr_rec.quote_no); 
		DSP_FLD ("n_quote");

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		abc_selfield(qthr,"qthr_id_no2");
		strcpy(qthr_rec.co_no, comm_rec.tco_no);
		strcpy(qthr_rec.br_no, comm_rec.test_no);
		if ( find_rec(qthr, &qthr_rec, EQUAL, "w"))
		{
			print_err(ML(mlStdMess210));
			sleep(2);
			clear_mess();
			return(1);
		}
		return(0);
	}
	return(0);
}

/*==========================
| Search for order number. |
==========================*/
void
show_qthr (
 char *	key_val)
{
	char	wk_mask[9];

	work_open();
	save_rec("#Qt No.","#Quote Date| Contact");
	abc_selfield(qthr,"qthr_id_no2");
	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	sprintf(qthr_rec.quote_no,"%-8.8s",key_val);
	cc = find_rec(qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strncmp(qthr_rec.quote_no,key_val,strlen(key_val)) && 
		   !strcmp(qthr_rec.co_no, comm_rec.tco_no) && 
		   !strcmp(qthr_rec.br_no, comm_rec.test_no))
	{
		strcpy(err_str, DateToString(qthr_rec.dt_quote));
						
		sprintf(wk_mask, "%-8.8s", qthr_rec.quote_no);
		cc = save_rec(wk_mask,err_str);
		if (cc)
			break;

		cc = find_rec(qthr, &qthr_rec, NEXT, "r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	sprintf(qthr_rec.quote_no, "%-8.8s", temp_str);
	cc = find_rec(qthr, &qthr_rec, COMPARISON, "r");
	if (cc)
	{
		file_err(cc, qthr, "DBFIND");
	}

	abc_selfield(qthr,"qthr_id_no");
}

void
process (void)
{
	int	i;
 	char	*table_fmt = "  %-8.8s  %-15.15s   %-10.10s   %-40.40s   %-20.20s   %-20.20s "; 
 	char	table_buf [133];

	assert (tableName == NULL);
	tableName = "prt_quote";
	tab_open (tableName, list_keys, 5, 0, 14, FALSE);
	sprintf (table_buf, table_fmt,
				"QT.NO.",
				"   REFERENCE   ",
				" QT. DATE ",
				"                 CUSTOMER                 ",
				"     SALESMAN         ",
				"      STATUS          "); 

	positionStatusField = (int) (strstr (table_buf, "      STATUS          ")
						  - table_buf);
	assert (strlen (table_buf)  < sizeof table_buf); 
	tab_add (tableName, "#%s", table_buf);

	if (!strcmp (local_rec.quote_no, "ALL     "))
		strcpy (local_rec.quote_no, "        ");

	memset (&qthr_rec, 0, sizeof (qthr_rec));
	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, comm_rec.test_no);
	strcpy(qthr_rec.quote_no, local_rec.quote_no);
	cc = find_rec(qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strcmp(qthr_rec.co_no, comm_rec.tco_no)
				&& !strcmp (qthr_rec.br_no, comm_rec.test_no))
	{
		if (strcmp(local_rec.quote_no,"        ") != 0 && strcmp (qthr_rec.quote_no, local_rec.quote_no) != 0)
		{
			cc = find_rec(qthr, &qthr_rec, NEXT, "r");
			continue;
		}
														
		strcpy (exsf_rec.co_no, qthr_rec.co_no);
		strcpy (exsf_rec.no, qthr_rec.sman_code);
		cc = find_rec(exsf, &exsf_rec, COMPARISON, "r");
			
		for (i = 0;strlen(q_status[i]._stat);i++)
		{
			if (!strncmp(qthr_rec.status, q_status[i]._stat,strlen(q_status[i]._stat)))
			{
				
				sprintf(local_rec.stat_desc,"%-20.20s",q_status[i]._desc);
				break;
			}
		}

		sprintf (table_buf, table_fmt,
				qthr_rec.quote_no,
				qthr_rec.enq_ref,
				DateToString(qthr_rec.dt_quote),
				qthr_rec.dbt_name,
				exsf_rec.desc,
			 	local_rec.stat_desc);

		tab_add (tableName, "%s", table_buf);
		table_hash [no_in_tab++] = qthr_rec.hhqt_hash;

		cc = find_rec(qthr, &qthr_rec, NEXT, "r");
	}

	if (no_in_tab)
		tab_scan (tableName); 

	if (restart)
	{
		tab_close (tableName, TRUE);
		tableName = NULL;
		FinishProgram (); 
		no_in_tab = 0;
		restart	= TRUE; 
	}
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (!strcmp (local_rec.quote_no, "        "))
			strcpy (local_rec.quote_no, "ALL     ");

		if (scn != cur_screen)
			scn_set(scn);

		clear();
		swide();
	
		move(0,1);
		line(132);

		rv_pr(ML(mlQtMess008),57,0,1);

		box(0,2,132,1);

		line_cnt = 0;
		scn_set(scn);
		scn_write(scn);
		scn_display(scn);
	}
	return (EXIT_SUCCESS);
}


static int
RestartFunc (
 int 		key,
 KEY_TAB *	psUnused)
{
	assert (key == FN1);
	restart	= TRUE;
	return key;
}

static int
ExitFunc (
 int 		key,
 KEY_TAB *	psUnused)
{
	assert (key == FN16);
	restart	= TRUE;
	return key;
}

static BOOL
IsFieldStatus (
 char * table_buf,
 char *	status)
{
	assert (strlen (table_buf) > positionStatusField + strlen (status));

	return !strncmp (&table_buf [positionStatusField], status, strlen (status));
}

static void
FieldStatusUpdate (
 char *	table_buf,
 char *	status)
{
	if (!IsFieldStatus (table_buf, status))
		strncpy (&table_buf [positionStatusField], status, strlen (status));
}

static void
TableLineStatusUpdate (
 char *	status)
{
	int 	st_line;
 	char	table_buf [133];

	assert (tableName != NULL);

	st_line = tab_tline (tableName);
	tab_get (tableName, table_buf , EQUAL, st_line);

	assert (strlen (table_buf ) < sizeof table_buf );

	FieldStatusUpdate (table_buf, status);
	tab_update (tableName, "%s", table_buf);
}

static int
PrintFunc (
 int 		key,
 KEY_TAB *	psUnused)
{
	int		ln_no;
	char	status_desc[21];
	char	*sptr;
	char	run_string[100];

	assert (key == PRINT_KEY);
	ln_no = tab_tline (tableName);
	abc_selfield(qthr, "qthr_hhqt_hash");
	qthr_rec.hhqt_hash = table_hash[ln_no];
	cc = find_rec(qthr, &qthr_rec, EQUAL, "u");
	if (!cc)
	{
		if (!strcmp(qthr_rec.status, "20") || !strcmp(qthr_rec.status, "30"))
		{
			sptr = getenv ("PROG_PATH");
			sprintf( run_string, "qt_prt_save %s/BIN/PRT %s %d", (sptr == (char *)0) ? "/usr/ver9" : sptr, qthr_rec.prt_name, lpno);

			cc = sys_exec (run_string);
			if (!cc)
			{
				strcpy(qthr_rec.status, "30");
				cc = abc_update (qthr, &qthr_rec);
				if (cc)
					file_err (cc, qthr, "DBUPDATE");

				abc_unlock (qthr);
				abc_selfield(qthr, "qthr_id_no2");
				strcpy (status_desc, ML("Quote Printed       "));
				TableLineStatusUpdate (status_desc);
			}
		}
		else
		{
			print_err (ML(mlQtMess007));
			sleep(2);
			clear_mess();
			abc_unlock (qthr);
			abc_selfield(qthr, "qthr_id_no2");
			return(1);
		} 
	}
	abc_selfield(qthr, "qthr_id_no2");
	heading(1);
	redraw_table(tableName);
	restart = FALSE;
	return (key);
}
