/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lett_prn.c,v 5.4 2002/07/18 06:24:14 scott Exp $
|  Program Name  : (db_lett_prn.c)
|  Program Desc  : (Letter Printing Program)
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel.  | Date Written : 15/02/96           |
|---------------------------------------------------------------------|
| $Log: db_lett_prn.c,v $
| Revision 5.4  2002/07/18 06:24:14  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/07/17 09:57:07  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2002/07/16 09:25:52  scott
| Updated from service calls and general maintenance.
|
| Revision 5.1  2001/12/07 03:53:54  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lett_prn.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lett_prn/db_lett_prn.c,v 5.4 2002/07/18 06:24:14 scott Exp $";

#define MAXSCNS 	2
#define MAXLINES	500
#define MAX_PAR		30
#define TABLINES	10
#define	P_PAGELEN	17

#define	X_OFF		0
#define	Y_OFF		0

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		marginLength = 5, 
			firstTime = TRUE,
			partPrinted = FALSE;

	int		pass	=	FALSE;    

	char	dspString [201],
			dspString2 [201],
			desc [150];

	FILE 	*fout;

static	char	*mth [] = {
	"January", "February", "March"    , "April"  , "May"     , "June",
	"July"   , "August"  , "September", "October", "November", "December"
};

#include	"schema"

struct commRecord	comm_rec;
struct dblhRecord	dblh_rec;
struct dbldRecord	dbld_rec;
struct dbphRecord	dbph_rec;
struct dbpdRecord	dbpd_rec;
struct cumrRecord	cumr_rec;
struct exafRecord	exaf_rec;
struct exsfRecord	exsf_rec;
struct exclRecord	excl_rec;
struct tspmRecord	tspm_rec;
struct comrRecord	comr_rec;

#include	<db_commands.h>

	char	*data = "data";
	long	_dblh_hash [MAX_PAR];
	char	systemDate [11];
	long	lsystemDate;

/*===========================
| Local & Screen Structures |
===========================*/
struct {			  	 	 	/*-------------------------------------*/
	char	dummy [11];	  	  	/*| Dummy Used In Screen Generator.   |*/
	char	lett_code [9];    	/*| Holds Letter Code                 |*/
	char	lett_desc [41];    	/*| Holds Letter Description          |*/
	char	s_dbt_no [7];      	/*| Holds Start Customer Number   	  |*/
	char	e_dbt_no [7];      	/*| Holds End Customer Number         |*/
	char	dbt_name [2] [41];  /*| Holds Customer Name			      |*/
	char	output_to [2];    	/*|                                   |*/
	char	outputDesc [11];   	/*|                                   |*/
	char	systemDate [11];   	/*|                                   |*/
	long	lsystemDate;       	/*|                                   |*/
	char	back [2];          	/*| Holds Background Flag             |*/
	char	backDesc [11];      /*| Holds Background Flag             |*/
	char	onight [2];         /*| Holds Overnight Flag              |*/
	char	onightDesc [11];    /*| Holds Overnight Flag              |*/
	int	lpno;                 	/*| Holds Printer Number              |*/
              		          	/*-------------------------------------*/
} local_rec;            

int	DISPLAY_IT;
		
extern	int	TruePosition;
static struct	var vars [] =
{
	{1, LIN, "lett_code", 3, 2, CHARTYPE,
	   "UUUUUUUU", "          ",
	   " "," ",  "Letter Code      ", " ",
       NE, NO, JUSTLEFT, "", "", local_rec.lett_code},
	{1, LIN, "lett_desc", 3, 35, CHARTYPE,
       "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
       " ", " ", " ", " ",
       NA, NO, JUSTLEFT, "", "", local_rec.lett_desc},
	{1, LIN, "output_to", 5, 2, CHARTYPE,
       "U", "          ", 
	   " ", "S", "Output To        ", " Enter P(rinter) / S(creen) ",
       YES, NO, JUSTLEFT, "SP", "", local_rec.output_to},
	{1, LIN, "outputDesc", 5, 35, CHARTYPE,
       "AAAAAAAAAA", "          ", 
	   " ", "", "", "",
       NA, NO, JUSTLEFT, "", "", local_rec.outputDesc},
	{1, LIN, "prnt_no", 6, 2, INTTYPE,
       "NN", "          ", 
       " ", " ", "Printer No       ", "",
       YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back", 7, 2, CHARTYPE,
       "U", "          ", 
       " ", "N", "Background       ", " Process in Background [Y/N] ",
       YES, NO, JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc", 7, 35, CHARTYPE,
       "AAAAAAAAA", "          ", 
       " ", "", "", "",
       NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight", 8, 2, CHARTYPE,
       "U", "          ", 
       " ", "No", "Overnight        ", " Process Overnight [Y/N] ",
       YES, NO, JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc", 8, 35, CHARTYPE,
       "AAAAAAAAA", "          ", 
       " ", "", "", "",
       NA, NO, JUSTLEFT, "", "", local_rec.onightDesc},
    {1, LIN, "s_dbt_no", 10, 2, CHARTYPE,
       "UUUUUU", "          ",
       " ", " ", "Start Customer   ", " ",
       YES, NO, JUSTLEFT, "", "", local_rec.s_dbt_no},
    {1, LIN, "s_dbt_name", 10, 35, CHARTYPE,
   	   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
       " ", " ", " ", " ",
       NA, NO, JUSTLEFT, "", "", local_rec.dbt_name [0]},
    {1, LIN, "e_dbt_no", 11, 2, CHARTYPE,
       "UUUUUU", "          ",
       " ", "~", "End Customer     ", " ",
       YES, NO, JUSTLEFT, "", "", local_rec.e_dbt_no},
    {1, LIN, "e_dbt_name", 11, 35, CHARTYPE,
	   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
       " ", " ", " ", " ",
       NA, NO, JUSTLEFT, "", "", local_rec.dbt_name [1]},
	{0, LIN, "", 0, 0, INTTYPE,
       "A", "          ", 
       " ", "", "dummy", " ", 
       YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

#include <FindCumr.h>

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	RunProgram 		(char *);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	SrchDbph 		(char *);
void 	DisplayLetter 	(void);
void 	ParseRoutine 	(char *);
int 	ValidCommand	(char *);
void 	SubCommand 		(int);
char 	*GenDate		(long);
void 	PrintCoStuff	(void);
int 	heading 		(int);

	int		envVarDbCo		=	0;
	char	branchNumber [3];

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;
	if (argc != 1 && argc != 5)
	{
		print_at (0, 0, mlDbMess160,argv [0]);
        return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	TruePosition	=	TRUE;

	/*===========================
	| Open main database files. |
	===========================*/
	OpenDB ();

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();


	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	/*==============================
	| Read common terminal record. |
	==============================*/
	if (argc == 5)
	{
		sprintf (local_rec.lett_code, "%-8.8s", argv [1]);
		sprintf (local_rec.s_dbt_no, "%-6.6s", argv [2]);
		sprintf (local_rec.e_dbt_no, "%-6.6s", argv [3]);
		local_rec.lpno = atoi (argv [4]);
		if (!pass)
		{
			if (!strcmp (local_rec.e_dbt_no, "      "))
				DISPLAY_IT = TRUE;
			else
				DISPLAY_IT = FALSE; 
		}
		else
		{
			DISPLAY_IT = FALSE;
			if (local_rec.lpno == 0)
				DISPLAY_IT = TRUE; 
		}
	
		DisplayLetter (); 
		
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	set_tty ();
	init_scr ();
	set_masks ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		lcount [2] 	= 0;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
RunProgram (
	char	*programName)
 {
	char	printerString [3];

	sprintf (printerString,"%d",local_rec.lpno);

	snorm ();
	clear ();
	print_at (0,0, ML (mlStdMess035)); 
	fflush (stdout);

	CloseDB (); 
	FinishProgram ();

	if (!strncmp (local_rec.onight, "Y", 1))
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				programName,
				local_rec.lett_code,
				local_rec.s_dbt_no,
				local_rec.e_dbt_no,
				printerString,
				"Print Letters Paragraphs", (char *)0);
		}
	}
    else if (!strncmp (local_rec.back, "Y", 1))
	{
		if (fork () == 0)
		{
			execlp (programName,
				programName,
				local_rec.lett_code,
				local_rec.s_dbt_no,
				local_rec.e_dbt_no,
				printerString, (char *)0);
		}
	}
	else 
	{
		execlp (programName,
			programName,
			local_rec.lett_code,
			local_rec.s_dbt_no,
			local_rec.e_dbt_no,
			printerString, (char *)0);
	}
}


/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec ("dblh", dblh_list, DBLH_NO_FIELDS, "dblh_id_no");
	open_rec ("dbld", dbld_list, DBLD_NO_FIELDS, "dbld_id_no");
	open_rec ("dbph", dbph_list, DBPH_NO_FIELDS, "dbph_id_no");
	open_rec ("dbpd", dbpd_list, DBPD_NO_FIELDS, "dbpd_id_no");
	open_rec ("exaf", exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec ("exsf", exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec ("excl", excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec ("tspm", tspm_list, TSPM_NO_FIELDS, "tspm_hhcu_hash"); 
	open_rec ("comr", comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec ("cumr", cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (void)
{
	abc_fclose ("dblh");
	abc_fclose ("dbld");
	abc_fclose ("dbph");
	abc_fclose ("dbpd");
	abc_fclose ("exaf");
	abc_fclose ("exsf");
	abc_fclose ("excl");
	abc_fclose ("tspm");
	abc_fclose ("comr");
	abc_fclose ("cumr");
	abc_dbclose ("data");
}

int
spec_valid (
 int                field)
{
	/*-----------------------
	| Validate Letter Code. |
	-----------------------*/
	if (LCHECK ("lett_code"))
	{
		if (SRCH_KEY)
		{
			SrchDbph (temp_str);
			sprintf (local_rec.lett_code, "%-8.8s", dbph_rec.letter_code + 2);
			return (EXIT_SUCCESS);
		}

		if (dflt_used && !strcmp (local_rec.lett_code, "        "))
			return (EXIT_FAILURE);

		strcpy (dbph_rec.co_no,comm_rec.co_no);
		sprintf (dbph_rec.letter_code, "C-%-8.8s", local_rec.lett_code);
		cc = find_rec ("dbph", &dbph_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess109));
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.lett_desc, dbph_rec.letter_desc);
		DSP_FLD ("lett_desc"); 
    	return (EXIT_SUCCESS);
	}

	/*------------------------+
	| Validate Start Customer |
	+------------------------*/
	if (LCHECK ("s_dbt_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}  
		if (dflt_used)
		{
			if (!strncmp (local_rec.output_to, "S", 1))
			{
				print_mess (ML (mlStdMess021));
				sleep (sleepTime); 
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				strcpy (local_rec.dbt_name [0], ML ("Start Customer"));
				DSP_FLD ("s_dbt_name"); 
				return (EXIT_SUCCESS);
			}
		}

		
        if (prog_status != ENTRY &&
		    strcmp (local_rec.s_dbt_no,local_rec.e_dbt_no) > 0 &&
			strcmp (local_rec.e_dbt_no, "      "))
	 	{ 
			print_mess (ML (mlStdMess017));
			return (EXIT_FAILURE);
		}

        strcpy (cumr_rec.co_no,comm_rec.co_no);
        strcpy (cumr_rec.est_no,branchNumber);
        strcpy (cumr_rec.dbt_no,pad_num (local_rec.s_dbt_no));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.dbt_name [0],"%-40.40s",cumr_rec.dbt_name);
		DSP_FLD ("s_dbt_name");
		
		if (!strncmp (local_rec.output_to, "S", 1))
		{
			FLD ("e_dbt_no")	= NA;
			sprintf (local_rec.e_dbt_no, "%-6.6s", " ");
			DSP_FLD ("e_dbt_no"); 
			entry_exit = TRUE;
			pass = FALSE;
		}
		
		return (EXIT_SUCCESS);
	}

	/*----------------------+
	| Validate End Customer |
	+----------------------*/
	if (LCHECK ("e_dbt_no"))
	{
		if (dflt_used)
		{
			if (!strncmp (local_rec.output_to, "S", 1))
			{
				print_mess (ML (mlStdMess021));
				sleep (sleepTime); 
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				strcpy (local_rec.dbt_name [1], ML ("End   Customer"));
				DSP_FLD ("e_dbt_name"); 
				return (EXIT_SUCCESS);
			}
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		
        strcpy (cumr_rec.co_no,comm_rec.co_no);
        strcpy (cumr_rec.est_no,branchNumber);
        strcpy (cumr_rec.dbt_no,pad_num (local_rec.e_dbt_no));
	    cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.s_dbt_no,local_rec.e_dbt_no) > 0)
		{
			errmess (ML (mlStdMess018));
			return (EXIT_FAILURE);
		}
		
		sprintf (local_rec.dbt_name [1],"%-40.40s",cumr_rec.dbt_name);
		DSP_FLD ("e_dbt_name");
		pass = TRUE;
		return (EXIT_SUCCESS);
	}

	/*-------------------+
	| Validate Output to |
	+-------------------*/
	if (LCHECK ("output_to"))
	{
		if (!strncmp (temp_str, "S", 1))
		{
			FLD ("prnt_no")	= NA;
			FLD ("back")	= NA;
			FLD ("onight")	= NA;

			local_rec.lpno = 0;
			DSP_FLD ("prnt_no");
			strcpy (local_rec.backDesc, 	ML ("No "));
			strcpy (local_rec.onightDesc, 	ML ("No "));
			strcpy (local_rec.back, 	"N");
			strcpy (local_rec.onight, 	"N");

			sprintf (local_rec.outputDesc, "%-8.8s", ML ("Screen "));
			DSP_FLD ("output_to");
		}
		else
		{
			FLD ("prnt_no")	= YES;
			FLD ("back")	= YES;
			FLD ("onight")	= YES;

			strcpy (local_rec.outputDesc, ML ("Printer"));
			DSP_FLD ("outputDesc");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS); 
	}

	/*----------------------
	| Validate Printer No. |
	----------------------*/
	if (LCHECK ("prnt_no"))
    {
		if (FLD ("prnt_no") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			local_rec.lpno = 1;
			DSP_FLD ("prnt_no");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
			get_lpno (0);

		if (local_rec.lpno == 0)
			return (EXIT_FAILURE);

       	return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (!strncmp (temp_str, "Y", 1))
		{
			strcpy (local_rec.backDesc,   ML ("Yes"));
			strcpy (local_rec.onightDesc, ML ("No "));
			strcpy (local_rec.back,   "Y");
			strcpy (local_rec.onight, "N");
			entry_exit = TRUE;
		}
		else
		{
			strcpy (local_rec.backDesc, ML ("No "));
			strcpy (local_rec.back, "N");
		}

		DSP_FLD ("backDesc");
		DSP_FLD ("onightDesc");
		DSP_FLD ("back");
		DSP_FLD ("onight");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (!strncmp (temp_str, "Y", 1))
		{
			strcpy (local_rec.onightDesc,ML ("Yes"));
			strcpy (local_rec.backDesc,  ML ("No "));
			strcpy (local_rec.onight, 	 "Y");
			strcpy (local_rec.back,  	 "N");
		}
		else
		{
			strcpy (local_rec.onightDesc, ML ("No "));
			strcpy (local_rec.onight, 	  "N");
		}

		DSP_FLD ("backDesc");
		DSP_FLD ("onightDesc");
		DSP_FLD ("back");
		DSP_FLD ("onight");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=================================
| Search routine for Letter file. |
=================================*/
void
SrchDbph (
	char	*key_val)
{
	_work_open (10,0,40);
	save_rec ("#Code.","#Letter Description");
	strcpy (dbph_rec.co_no, comm_rec.co_no);
	sprintf (dbph_rec.letter_code, "C-%-8.8s", key_val);
	cc = find_rec (dbph, &dbph_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (dbph_rec.co_no, comm_rec.co_no) && 
		   !strncmp (dbph_rec.letter_code + 2,key_val,strlen (key_val)))
	{
		if (!strncmp (dbph_rec.letter_code, "C-", 2))
		{
			cc = save_rec (dbph_rec.letter_code + 2, dbph_rec.letter_desc);
			if (cc)
				break;
		}
		cc = find_rec (dbph, &dbph_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (dbph_rec.co_no, comm_rec.co_no);
	sprintf (dbph_rec.letter_code, "C-%-8.8s", temp_str);
	cc = find_rec (dbph, &dbph_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, dbph, "DBFIND");
}

/*============================
| Display/Print Letter file. |
============================*/
void
DisplayLetter (void)
{
	char	parseString [301];
	char	last_cust [11];
	int		srec_lcount = 0;
	int		i;

	if (DISPLAY_IT)
	{
		init_scr ();
		set_tty ();
		swide (); 
		clear ();
		Dsp_prn_open (0,0,P_PAGELEN, "L e t t e r   P r i n t",
			comm_rec.co_no, comm_rec.co_name,
			 (char *) 0, (char *) 0,
			 (char *) 0, (char *) 0);
	}
	else
	{
		if ((fout = popen ("pformat","w")) == NULL)
			sys_err ("Error in pformat During (POPEN)",errno,PNAME);

		fprintf (fout,".START00/00/00\n");
		fprintf (fout,".LP%d\n",local_rec.lpno);
		fprintf (fout,".PI16\n");
		fprintf (fout,".OP\n");
		fprintf (fout,".PL200\n");
		fprintf (fout,".1\n"); 
		fprintf (fout,".L80\n"); 
		fflush (fout);  

		dsp_screen ("Letter Print", comm_rec.co_no, comm_rec.co_name); 
	}
	
	if (DISPLAY_IT)
	{
		Dsp_saverec ("                                          L E T T E R    D E S C R I P T I O N .                                           ");	
		Dsp_saverec ("");
		Dsp_saverec (" [REDRAW]  [PRINT]  [NEXT SCN]  [PREV SCN]  [INPUT/END]");
	}

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNumber);
	strcpy (cumr_rec.dbt_no, local_rec.s_dbt_no);
	cc = find_rec ("cumr", &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no) &&
				!strcmp (cumr_rec.est_no, branchNumber))
	{
		if (cumr_rec.letter [0] != 'Y')
		{
			if (!DISPLAY_IT)
			{
				if (!strcmp (cumr_rec.dbt_no, local_rec.e_dbt_no))  
					break;
			}
			else
			{
				if (!strcmp (cumr_rec.dbt_no, local_rec.s_dbt_no))
					break;
			}

			cc = find_rec ("cumr", &cumr_rec, NEXT, "r");
			continue;
		} 

		strcpy (exaf_rec.co_no, cumr_rec.co_no); 
		strcpy (exaf_rec.area_code, cumr_rec.area_code); 
		cc = find_rec ("exaf", &exaf_rec, EQUAL, "r");

		strcpy (exsf_rec.co_no, cumr_rec.co_no); 
		strcpy (exsf_rec.salesman_no, cumr_rec.sman_code); 
		cc = find_rec ("exsf", &exsf_rec, EQUAL, "r");

		strcpy (excl_rec.co_no, cumr_rec.co_no); 
		strcpy (excl_rec.class_type, cumr_rec.class_type); 
		cc = find_rec ("excl", &excl_rec, EQUAL, "r");

		tspm_rec.hhcu_hash = cumr_rec.hhcu_hash; 
		cc = find_rec ("tspm", &tspm_rec, EQUAL, "r");

		strcpy (comr_rec.co_no, cumr_rec.co_no); 
		cc = find_rec ("comr", &comr_rec, EQUAL, "r");

		strcpy (dbph_rec.co_no, comm_rec.co_no);
		sprintf (dbph_rec.letter_code, "C-%-8.8s", local_rec.lett_code);
		cc = find_rec ("dbph", &dbph_rec, EQUAL, "r");
		if (!cc && !strcmp (dbph_rec.co_no, comm_rec.co_no)
			&& !strncmp (dbph_rec.letter_code + 2, local_rec.lett_code,8))
		{
			strcpy (last_cust, cumr_rec.dbt_no); 

			if (DISPLAY_IT)
			{
				sprintf (dspString,"^1Customer : %6.6s - (%40.40s) %56.56s^6", cumr_rec.dbt_no, cumr_rec.dbt_name, " ");
				Dsp_saverec (dspString);
			}

			if (!DISPLAY_IT)
				dsp_process ("Letter : ", dbph_rec.letter_code);
	
			srec_lcount = 1;

			dbpd_rec.dbph_hash = dbph_rec.dbph_hash;
			dbpd_rec.line_no = 0L;
			cc = find_rec ("dbpd", &dbpd_rec, GTEQ, "r");
			while (!cc && dbpd_rec.dbph_hash == dbph_rec.dbph_hash) 
			{
				abc_selfield (dblh, "dblh_dblh_hash"); 
				dblh_rec.dblh_hash = dbpd_rec.dblh_hash;
				cc = find_rec ("dblh", &dblh_rec, EQUAL, "r");
				if (!cc && dblh_rec.dblh_hash == dbpd_rec.dblh_hash)
				{
					dbld_rec.dblh_hash = dblh_rec.dblh_hash;
					dbld_rec.line_no = 0L; 
					cc = find_rec ("dbld", &dbld_rec, GTEQ, "r");
					while (!cc && dbld_rec.dblh_hash == dblh_rec.dblh_hash)  
					{
						srec_lcount++;
						if (srec_lcount > P_PAGELEN)
						{
							if (DISPLAY_IT)
							{
								sprintf (dspString,"^1Customer : %6.6s - (%40.40s) Continued...........%36.36s^6", cumr_rec.dbt_no, cumr_rec.dbt_name, " ");
								Dsp_saverec (dspString);
							}
				
							srec_lcount = 2;
						}
							
						sprintf (parseString,
								"%*.*s%s", 
								marginLength, marginLength, " ", 
								clip (dbld_rec.desc));
		
						ParseRoutine (parseString);

						if (DISPLAY_IT)
							Dsp_saverec (dspString2);   
						
						memset (dspString2, 0, sizeof (dspString2)); 
						cc = find_rec ("dbld", &dbld_rec, NEXT, "r");
					}
				}
				cc = find_rec ("dbpd", &dbpd_rec, NEXT, "r");
				abc_selfield (dblh, "dblh_id_no");
			} 	

			if (DISPLAY_IT)
			{
				if (srec_lcount + 2 < P_PAGELEN)
				{
					sprintf (dspString,"^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^^1 END OF LETTER DETAILS FOR %-10.10s ^6^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG", last_cust);
					Dsp_saverec (dspString);
					srec_lcount++;
				}

				for (i = srec_lcount; i < P_PAGELEN; i++)
					Dsp_saverec (" ");
			}
		}

		if (!DISPLAY_IT)
		{
			fprintf (fout, ".PA\n");
			fflush (fout);  
		}

		if (!DISPLAY_IT) 
		{
			if (!strcmp (cumr_rec.dbt_no, local_rec.e_dbt_no)) 
				break;
		}
		else
		{
			if (!strcmp (cumr_rec.dbt_no, local_rec.s_dbt_no)) 
				break;
		}

		cc = find_rec ("cumr", &cumr_rec, NEXT, "r");
	}
	
	if (DISPLAY_IT)
	{
		Dsp_srch ();
		Dsp_close ();
		rset_tty ();
	}
}

void
ParseRoutine (
 char*              wrk_prt)
{
	int		cmd;
	int		i;
	char	*cptr;
	char	*dptr;
	char	*wk_prt = strdup (wrk_prt);
	
	partPrinted = TRUE;

	/*---------------------------
	|	look for caret command	|
	---------------------------*/
	cptr = strchr (wk_prt, '.');
	dptr = wk_prt;
	if (cptr == NULL)
		sprintf (dspString2,"%-120.120s", dbld_rec.desc); 

	while (cptr)
	{
		partPrinted = FALSE;
		/*----------------------
		| print line up to now |
		----------------------*/
		*cptr = (char) NULL;

		if (cptr != wk_prt)
		{
			partPrinted = TRUE;
			if (!DISPLAY_IT)
				fprintf (fout, "%s", dptr);
		}

		/*---------------------------
		|	check if valid .command	|
		---------------------------*/
		cmd = ValidCommand (cptr + 1);

		memset (&desc,0,sizeof desc);
		if (cmd >= CUR_DAT)
		{

			for (i=0; (unsigned int) i < strlen (clip (dbld_rec.desc)); i++)
			{
				if (!strncmp (&dbld_rec.desc [i], ".",1))
					break;
				if (firstTime)
				{
					strncpy (desc, &dbld_rec.desc [i],1);
					firstTime = FALSE;
				}
				else
					strncat (desc, &dbld_rec.desc [i], 1);
			} 

			SubCommand (cmd);
			dptr = cptr + 8;
		}
		else
		{
			sprintf (dspString2,"%-120.120s", dbld_rec.desc); 
			partPrinted = TRUE;
			dptr = cptr + 1;
		}
		cptr = strchr (dptr,'.');
	}

	/*-----------------------
	|	print rest of line	|
	-----------------------*/
	if (partPrinted && !DISPLAY_IT)
	{
		if (dptr)
			fprintf (fout, "%s\n", dptr);
		else
			fprintf (fout, "\n");
	}
	free (wk_prt);
}

/*=====================
| Validate .commands. |
=====================*/
int
ValidCommand (
 char*              wk_str)
{
	int	i;

	/*----------------------------------------
	| Dot command is last character on line. |
	----------------------------------------*/
	if (!strlen (wk_str))
		return (-1);

	for (i = 0;i < N_CMDS;i++)
		if (!strncmp (wk_str,dot_cmds [i],7))
			return (i);

	return (-1);
}

/*==============================================
| Substitute valid .commands with actual data. |
==============================================*/
void
SubCommand (
 int                cmd)
{
	char	*pr_sptr;
	char	tmp_amt [21];

	switch (cmd)
	{

	/*-------------------------------
	| System Date, format dd/mm/yy. |
	-------------------------------*/
	case	CUR_DAT:
		partPrinted = TRUE;
		strcpy (dspString2, local_rec.systemDate);
		if (!DISPLAY_IT)
        {
		/*	fprintf (fout, "%-8.8s", pr_sptr); */
        	fprintf (fout, "%-8.8s", dspString2);
        }
		break;
		
	/*---------------------------
	| Customer Full system date. |
	---------------------------*/
	case FUL_DAT:
		pr_sptr = GenDate (comm_rec.dbt_date);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-20.20s", pr_sptr);
		}
		break;
	
	/*---------------
	| Company Name. |
	---------------*/
	case	CO_NAME:
		pr_sptr = clip (comr_rec.co_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;
		
	/*--------------------
	| Company Address 1. |
	--------------------*/
	case	CO_ADR1:
		pr_sptr = clip (comr_rec.co_adr1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;
		
	/*--------------------
	| Company Address 2. |
	--------------------*/
	case	CO_ADR2:
		pr_sptr = clip (comr_rec.co_adr2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;
		
	/*--------------------
	| Company Address 3. |
	--------------------*/
	case	CO_ADR3:
		pr_sptr = clip (comr_rec.co_adr3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;
	
	/*-----------------
	| Customer Number. |
	-----------------*/
	case	DB_NUMB:
		pr_sptr = clip (cumr_rec.dbt_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-6.6s", pr_sptr);
		}
		break;

	/*------------------
	| Customer Acronym. |
	------------------*/
	case	DB_ACRO:
		pr_sptr = clip (cumr_rec.dbt_acronym);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-9.9s", pr_sptr);
		}
		break;


	/*---------------
	| Customer Name. |
	---------------*/
	case	DB_NAME:
		pr_sptr = clip (cumr_rec.dbt_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*------------------
	| Customer Address. |
	------------------*/
	case	DB_ADR1:
		pr_sptr = clip (cumr_rec.ch_adr1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*------------------
	| Customer Address. |
	------------------*/
	case	DB_ADR2:
		pr_sptr = clip (cumr_rec.ch_adr2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*------------------
	| Customer Address. |
	------------------*/
	case	DB_ADR3:
		pr_sptr = clip (cumr_rec.ch_adr3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*---------------------------
	| Prospects Contact Name 1. |
	---------------------------*/
	case	DB_CNT1:
		pr_sptr = clip (tspm_rec.cont_name1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-30.30s", pr_sptr);
		}
		break;

	/*---------------------------
	| Prospects Contact Code 1. |
	---------------------------*/
	case	CNT_CD1:
		pr_sptr = clip (tspm_rec.cont_code1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-3.3s", pr_sptr);
		}
		break;
	
	/*---------------------------
	| Prospects Contact Name 2. |
	---------------------------*/
	case	DB_CNT2:
		pr_sptr = clip (tspm_rec.cont_name2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-30.30s", pr_sptr);
		}
		break;

	/*---------------------------
	| Prospects Contact Code 2. |
	---------------------------*/
	case	CNT_CD2:
		pr_sptr = clip (tspm_rec.cont_code2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-3.3s", pr_sptr);
		}
		break;

	/*--------------
	| Salesman No. |
	--------------*/
	case	SM_NUMB:
		pr_sptr = clip (cumr_rec.sman_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-2.2s", pr_sptr);
		}
		break;

	/*----------------
	| Salesman Name. |
	----------------*/
	case	SM_NAME:
		pr_sptr = clip (exsf_rec.salesman);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;
	
	/*----------
	| Area No. |
	----------*/
	case	AR_NUMB:
		pr_sptr = clip (exaf_rec.area_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-2.2s", pr_sptr);
		}
		break;
	
	/*------------
	| Area Name. |
	------------*/
	case	AR_NAME:
		pr_sptr = clip (exaf_rec.area);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;
	
	/*----------------
	| Contract Type. |
	----------------*/
	case	CNT_TYP:
		pr_sptr = clip (cumr_rec.cont_type);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-3.3s", pr_sptr);
		}
		break;

	/*-------------
	| Price Type. |
	-------------*/
	case	PRI_TYP:
		pr_sptr = clip (cumr_rec.price_type);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-1.1s", pr_sptr);
		}
		break;

	/*------------
	| Bank Code. |
	------------*/
	case	BNK_CDE:
		pr_sptr = clip (cumr_rec.bank_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-3.3s", pr_sptr);
		}
		break;

	/*--------------
	| Bank Branch. |
	--------------*/
	case	BNK_BRN:
		pr_sptr = clip (cumr_rec.branch_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-20.20s", pr_sptr);
		}
		break;

	/*----------------
	| Discount Code. |
	----------------*/
	case	DIS_CDE:
		pr_sptr = clip (cumr_rec.disc_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-1.1s", pr_sptr);
		}
		break;

	/*-----------
	| Tax Code. |
	-----------*/
	case	TAX_CDE:
		pr_sptr = clip (cumr_rec.tax_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-1.1s", pr_sptr);
		}
		break;

	/*-------------
	| Tax Number. |
	-------------*/
	case	TAX_NUM:
		pr_sptr = clip (cumr_rec.tax_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-15.15s", pr_sptr);
		}
		break;

	/*-----------------------
	| Date Of Last Invoice. |
	-----------------------*/
	case	DT_LINV:
		strcpy (err_str, DateToString (cumr_rec.date_lastinv));
		pr_sptr = clip (err_str);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-8.8s", pr_sptr);
		}
		break;

	/*-----------------------
	| Date Of Last Payment. |
	-----------------------*/
	case	DT_LPAY:
		strcpy (err_str, DateToString (cumr_rec.date_lastpay));
		pr_sptr = clip (err_str);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-8.8s", pr_sptr);
		}
		break;

	/*-------------------------
	| Amount Of Last Payment. |
	-------------------------*/
	case	AMT_LPY:
		sprintf (tmp_amt,"%8.2f",cumr_rec.amt_lastpay);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-8.8s", pr_sptr);
		}
		break;

	/*-----------------------
	| Month To Date Sales . |
	-----------------------*/
	case	MTD_SAL:
		sprintf (tmp_amt,"%8.2f",cumr_rec.mtd_sales);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-8.8s", pr_sptr);
		}
		break;

	/*----------------------
	| Year To Date Sales . |
	----------------------*/
	case	YTD_SAL:
		sprintf (tmp_amt,"%8.2f",cumr_rec.ytd_sales);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-8.8s", pr_sptr);
		}
		break;

	/*---------------------------
	| Value Of Current Orders . |
	---------------------------*/
	case	ORD_VAL:
		sprintf (tmp_amt,"%8.2f",cumr_rec.ord_value);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-8.8s", pr_sptr);
		}
		break;

	/*------------
	| Post Code. |
	------------*/
	case	DB_POST:
		pr_sptr = clip (cumr_rec.post_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-10.10s", pr_sptr);
		}
		break;

	/*------------------
	| Business Sector. |
	------------------*/
	case	DB_BSEC:
		pr_sptr = clip (excl_rec.class_desc);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;
		
	/*---------------
	| Phone Number. |
	---------------*/
	case	PHN_NUM:
		pr_sptr = clip (cumr_rec.phone_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-15.15s", pr_sptr);
		}
		break;

	/*-------------
	| Fax Number. |
	-------------*/
	case	FAX_NUM:
		pr_sptr = clip (cumr_rec.fax_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (dspString2, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-15.15s", pr_sptr);
		}
		break;

	default:
		break;
	}
	strcat (desc, dspString2);
	strcpy (dspString2, desc);
	memset (&desc,0,sizeof desc); 

	if (!DISPLAY_IT)
		fflush (fout); 
}

/*====================================================
| GenDate (long-date) returns date in 23 January 1986 . |
====================================================*/
char*
GenDate (
	long	currentDate)
{
	int		day,
			mon,
			year;

	DateToDMY (currentDate, &day, &mon, &year);

	sprintf (err_str, "%d %s %04d", day, mth [mon -1], year);
	return (err_str);
}

/*========================
| Print Company Details. |
========================*/
void
PrintCoStuff (void)
{
	line_at (20,0,130);
	line_at (22,0,130);
	print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
}

/*================
| Print Heading. |
================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		swide (); 
		clear ();
		
		rv_pr (ML (mlDbMess128),43,0,1);
		
		box (0,2,130,9);
		line_at (1,1,130);
		line_at (4,1,129);
		line_at (9,1,129);

		PrintCoStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

