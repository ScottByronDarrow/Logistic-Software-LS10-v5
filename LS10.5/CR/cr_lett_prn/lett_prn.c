/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: lett_prn.c,v 5.4 2002/07/17 09:57:02 scott Exp $
|  Program Name  : (cr_lett_prn.c)                                    |
|  Program Desc  : (Letter Printing Program.                    )     |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel.     | Date Written  : 18/03/96       |
|---------------------------------------------------------------------|
| $Log: lett_prn.c,v $
| Revision 5.4  2002/07/17 09:57:02  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/26 09:05:20  robert
| Updated to avoid overlapping of descriptions
|
| Revision 5.2  2001/08/09 08:52:01  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:31  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:03:20  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:23:49  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/23 04:17:12  scott
| Updated to ensure program works with LS10-GUI without change
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lett_prn.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_lett_prn/lett_prn.c,v 5.4 2002/07/17 09:57:02 scott Exp $";

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
#include <ml_cr_mess.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		defaultMargin	= 5, 
			firstTime		= TRUE,
			partPrinted		= FALSE,
			argumentsPassed	= FALSE;  

	char	displayStringOne [201],
			displayStringTwo [201],
			workDescription [150];

	char	branchNumber [3];
	int		envCrCo = 0;

	FILE 	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct dblhRecord	dblh_rec;
struct dbldRecord	dbld_rec;
struct dbphRecord	dbph_rec;
struct dbpdRecord	dbpd_rec;
struct sumrRecord	sumr_rec;
struct comrRecord	comr_rec;


#include	<cr_commands.h>

	char	*data = "data";

	long	_dblh_hash [MAX_PAR];
	char	systemDate [11];
	long	lsystemDate;

/*===========================
| Local & Screen Structures |
===========================*/
struct {			  	 	 		/*-------------------------------------*/
	char	dummy [11];	  	  		/*| Dummy Used In Screen Generator.   |*/
	char	lett_code [9];    		/*| Holds Letter Code                 |*/
	char	lett_desc [41];    		/*| Holds Letter Description          |*/
	char	startSupplierNo [7];	/*| Holds Start Customer Number   	  |*/
	char	endSupplierNo [7];  	/*| Holds End Customer Number         |*/
	char	supplierName [2] [41];  /*| Holds Customer Name			      |*/
	char	output_to [9];     		/*|                                   |*/
	char	output_to_value [2];	/*|                                   |*/
	char	systemDate [11];    	/*|                                   |*/
	long	lsystemDate;       		/*|                                   |*/
	char	back [4];          		/*| Holds Background Flag             |*/
	char	onite [4];         		/*| Holds Overnight Flag              |*/
	char	back_value [2];    		/*|                                   |*/
	char	onite_value [2];   		/*|                                   |*/
	int		printerNumber;      	/*| Holds Printer Number              |*/
              		          		/*-------------------------------------*/
} local_rec;            

int	DISPLAY_IT;
		
static struct	var vars [] =
{
	{1, LIN, "lett_code", 3, 22, CHARTYPE,
	   "UUUUUUUU", "          ",
	   " "," ", "Letter Code", " ",
       NE, NO, JUSTLEFT, "", "", local_rec.lett_code},
	{1, LIN, "lett_desc", 3, 50, CHARTYPE,
       "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
       " ", " ", " ", " ",
       NA, NO, JUSTLEFT, "", "", local_rec.lett_desc},
	{1, LIN, "output_to", 5, 22, CHARTYPE,
       "U", "          ", 
	   " ", "S", "Output To : ", " Enter P (rinter) / S (creen) ",
       YES, NO, JUSTLEFT, "SP", "", local_rec.output_to_value},
	{1, LIN, "output_to_desc", 5, 50, CHARTYPE,
       "UUUUUUUUUU", "          ", 
	   " ", "", "", "",
       NA, NO, JUSTLEFT, "", "", local_rec.output_to},
	{1, LIN, "prnt_no", 6, 22, INTTYPE,
       "NN", "          ", 
       " ", " ", "Printer No. ", " Default is 1 ",
       YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back_gnd", 7, 22, CHARTYPE,
       "U", "          ", 
       " ", "N", "Background ", " Process in Background [Y/N] ",
       YES, NO, JUSTLEFT, "YN", "", local_rec.back_value},
	{1, LIN, "back_gnd_desc", 7, 27, CHARTYPE,
       "UUU", "          ", 
       " ", "", "", "",
       NA, NO, JUSTLEFT, "", "", local_rec.back},
	{1, LIN, "over_nite", 7, 67, CHARTYPE,
       "U", "          ", 
       " ", "N", "Overnight ", " Process Overnight [Y/N] ",
       YES, NO, JUSTLEFT, "YN", "", local_rec.onite_value},
	{1, LIN, "over_nite_desc", 7, 72, CHARTYPE,
       "UUU", "          ", 
       " ", "", "", "",
       NA, NO, JUSTLEFT, "", "", local_rec.onite},
    {1, LIN, "startSupplierNo", 9, 22, CHARTYPE,
       "UUUUUU", "          ",
       " ", " ", "Start Supplier ", " ",
       YES, NO, JUSTLEFT, "", "", local_rec.startSupplierNo},
    {1, LIN, "startSupplierName", 9, 50, CHARTYPE,
   	   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
       " ", " ", " ", " ",
       NA, NO, JUSTLEFT, "", "", local_rec.supplierName [0]},
    {1, LIN, "endSupplierNo", 10, 22, CHARTYPE,
       "UUUUUU", "          ",
       " ", "~ ", "End Supplier ", " ",
       YES, NO, JUSTLEFT, "", "", local_rec.endSupplierNo},
    {1, LIN, "endSupplierName", 10, 50, CHARTYPE,
	   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
       " ", " ", " ", " ",
       NA, NO, JUSTLEFT, "", "", local_rec.supplierName [1]},
	{0, LIN, "", 0, 0, INTTYPE,
       "A", "          ", 
       " ", "", "dummy", " ", 
       YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

#include <FindSumr.h> 

/*===========================
| Local function prototypes |
===========================*/
void	RunProgram			 (char *);
void	shutdown_prog		 (void);
void	OpenDB				 (void);
void	CloseDB				 (void);
int		spec_valid			 (int);
void	SrchDbph			 (char *);
void	DisplayLetter		 (void);
void	ParseRoutine		 (char *);
int		ValidateCommand		 (char *);
int		SubstitudeCommend	 (int);
void	PrintCoDetails		 (void);
int		heading				 (int);
char 	*GetFormatedDate 	 (long);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 1 && argc != 5)
	{
		print_at (0, 0, mlCrMess005, argv [0]);
		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	envCrCo = atoi (get_env ("CR_CO"));

	/*===========================
	| Open main database files. |
	===========================*/
	OpenDB ();

	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	if (argc == 5)
	{
		sprintf (local_rec.lett_code, "%-8.8s", argv [1]);
		sprintf (local_rec.startSupplierNo, "%-6.6s", argv [2]);
		sprintf (local_rec.endSupplierNo, "%-6.6s", argv [3]);
		local_rec.printerNumber = atoi (argv [4]);
		if (!argumentsPassed)
		{
			if (!strcmp (local_rec.endSupplierNo, "      ") &&
				 (local_rec.printerNumber == 0))
				DISPLAY_IT = TRUE;
			else
				DISPLAY_IT = FALSE;
		}
		else
		{
			DISPLAY_IT = FALSE;
			if (local_rec.printerNumber == 0)
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
		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		lcount [2]	= 0;
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
 char *	prog_name)
{
	char	printerString [3];

	sprintf (printerString,"%d",local_rec.printerNumber);

	snorm ();
	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);

	CloseDB (); 
	FinishProgram ();

	if (!strncmp (local_rec.onite, "Y", 1))
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.lett_code,
					local_rec.startSupplierNo,
					local_rec.endSupplierNo,
					printerString,
					"Print Letters Paragraphs", (char *)0);
		}
		else
			return;
	}

	else if (!strncmp (local_rec.back, "Y", 1))
	{
		if (fork () == 0)
		{
			execlp (prog_name,
					prog_name,
					local_rec.lett_code,
					local_rec.startSupplierNo,
					local_rec.endSupplierNo,
					printerString, (char *)0);
		}
		else
			return;
	}
	else 
	{
		execlp (prog_name,
				prog_name,
				local_rec.lett_code,
				local_rec.startSupplierNo,
				local_rec.endSupplierNo,
				printerString, (char *)0);
	}
}

/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	/*==============================
	| Read common terminal record. |
	==============================*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (dblh, dblh_list, DBLH_NO_FIELDS, "dblh_id_no");
	open_rec (dbld, dbld_list, DBLD_NO_FIELDS, "dbld_id_no");
	open_rec (dbph, dbph_list, DBPH_NO_FIELDS, "dbph_id_no");
	open_rec (dbpd, dbpd_list, DBPD_NO_FIELDS, "dbpd_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (dblh);
	abc_fclose (dbld);
	abc_fclose (dbph);
	abc_fclose (dbpd);
	abc_fclose (comr);
	abc_fclose (sumr);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
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
		sprintf (dbph_rec.letter_code, "S-%-8.8s", local_rec.lett_code);
		cc = find_rec (dbph, &dbph_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (err_str, ML (mlCrMess004), local_rec.lett_code);

			errmess (err_str); 
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.lett_desc, dbph_rec.letter_desc);
		DSP_FLD ("lett_desc"); 
    	return (EXIT_SUCCESS);
	}

	/*------------------------+
	| Validate Start Supplier |
	+------------------------*/
	if (LCHECK ("startSupplierNo"))
	{
		if (dflt_used)
		{
			if (!strncmp (local_rec.output_to, "S", 1))
			{
				print_mess (ML (mlStdMess023));
				sleep (sleepTime); 
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (local_rec.supplierName [0],"%-40.40s","Start Supplier");
				DSP_FLD ("startSupplierName"); 
				return (EXIT_SUCCESS);
			}
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}  
		
        if (prog_status != ENTRY &&
		    strcmp (local_rec.startSupplierNo,local_rec.endSupplierNo) > 0 &&
			strcmp (local_rec.endSupplierNo, "      "))
	 	{ 
			print_mess (ML (mlStdMess017));
			return (EXIT_FAILURE);
		}

        strcpy (sumr_rec.co_no,comm_rec.co_no);
        strcpy (sumr_rec.est_no,branchNumber);
        strcpy (sumr_rec.crd_no,pad_num (local_rec.startSupplierNo));
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.supplierName [0],"%-40.40s",sumr_rec.crd_name);
		DSP_FLD ("startSupplierName");

		if (!strncmp (local_rec.output_to, "S", 1))
		{
			FLD ("endSupplierNo") = NA;
			sprintf (local_rec.endSupplierNo, "%-6.6s", " ");
			DSP_FLD ("endSupplierNo");
			entry_exit = TRUE;
			argumentsPassed = FALSE;
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------+
	| Validate End Supplier |
	+----------------------*/
	if (LCHECK ("endSupplierNo"))
	{
		if (dflt_used)
		{
			if (!strncmp (local_rec.output_to, "S", 1))
			{
				print_mess (ML (mlStdMess023));
				sleep (sleepTime); 
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (local_rec.supplierName [1],"%-40.40s","End   Supplier");
				DSP_FLD ("endSupplierName"); 
				return (EXIT_SUCCESS);
			}
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
	
        strcpy (sumr_rec.co_no,comm_rec.co_no);
        strcpy (sumr_rec.est_no,branchNumber);
        strcpy (sumr_rec.crd_no,pad_num (local_rec.endSupplierNo));
	    cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startSupplierNo,local_rec.endSupplierNo) > 0)
		{
			errmess (ML (mlStdMess018));
			return (EXIT_FAILURE);
		}
		
		sprintf (local_rec.supplierName [1],"%-40.40s",sumr_rec.crd_name);
		DSP_FLD ("endSupplierName");
		argumentsPassed = TRUE; 
		return (EXIT_SUCCESS);
	}

	/*-------------------+
	| Validate Output to |
	+-------------------*/
	if (LCHECK ("output_to"))
	{
		if (dflt_used)
			local_rec.printerNumber = 0;

		if (!strncmp (temp_str, "S", 1))
		{
			FLD ("prnt_no")		= NA;
			FLD ("back_gnd")	= NA;
			FLD ("over_nite")	= NA;

			local_rec.printerNumber = 0;
			
			strcpy (local_rec.back, "No ");
			strcpy (local_rec.onite, "No ");
			strcpy (local_rec.back_value, "N");
			strcpy (local_rec.onite_value, "N");

			sprintf (local_rec.output_to, "%-8.8s", "Screen ");
			DSP_FLD ("output_to_desc");
		}
		else
		{
			FLD ("prnt_no")		= YES;
			FLD ("back_gnd")	= YES;
			FLD ("over_nite")	= YES;
			
			local_rec.printerNumber = 1;

			strcpy (local_rec.output_to, "Printer");
			DSP_FLD ("output_to_desc");
		}
		DSP_FLD ("prnt_no"); 
		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate Printer No. |
	----------------------*/
	if (LCHECK ("prnt_no"))
    {
		if (FLD ("prnt_no") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used && !argumentsPassed)
		{
			local_rec.printerNumber = 1;
			DSP_FLD ("prnt_no");
			return (EXIT_SUCCESS);
		} 

		if (SRCH_KEY)
			get_lpno (0);

		if (local_rec.printerNumber == 0)
			return (EXIT_FAILURE);

       	return (EXIT_SUCCESS);
	}

	if (LCHECK ("back_gnd"))
	{
		if (!strncmp (temp_str, "Y", 1))
		{
			strcpy (local_rec.back, "Yes");
			strcpy (local_rec.onite, "No ");
			strcpy (local_rec.onite_value, "N");
			entry_exit = TRUE;
		}
		else
			strcpy (local_rec.back, "No ");

		DSP_FLD ("back_gnd_desc");
		DSP_FLD ("over_nite");
		DSP_FLD ("over_nite_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("over_nite"))
	{
		if (!strncmp (temp_str, "Y", 1))
		{
			strcpy (local_rec.onite, "Yes");
			strcpy (local_rec.back, "No ");
			strcpy (local_rec.back_value, "N");
		}
		else
			strcpy (local_rec.onite, "No ");

		DSP_FLD ("back_gnd");
		DSP_FLD ("back_gnd_desc");
		DSP_FLD ("over_nite_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=================================
| Search routine for Letter file. |
=================================*/
void
SrchDbph (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code.","#Letter Description");
	strcpy (dbph_rec.co_no, comm_rec.co_no);
	sprintf (dbph_rec.letter_code, "S-%-8.8s", key_val);
	cc = find_rec (dbph, &dbph_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (dbph_rec.co_no, comm_rec.co_no) && 
		   !strncmp (dbph_rec.letter_code + 2,key_val,strlen (key_val)))
	{
		if (!strncmp (dbph_rec.letter_code, "S-", 2))
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
	sprintf (dbph_rec.letter_code, "S-%-8.8s", temp_str);
	cc = find_rec (dbph, &dbph_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, dbph, "DBFIND");
}

/*============================
| Display/Print Letter file. |
============================*/
void
DisplayLetter (
 void)
{
	char	parseString [201];
	char	last_cust [11];
	int		srec_lcount = 0;
	int		i;

	if (DISPLAY_IT)
	{
		init_scr ();
		set_tty ();
		swide (); 
		clear ();
		Dsp_prn_open (0,0,P_PAGELEN, "Letter Print",
			comm_rec.co_no, comm_rec.co_name,
			 (char *) 0, (char *) 0,
			 (char *) 0, (char *) 0);
	}
	else
	{
		if ((fout = popen ("pformat","w")) == NULL)
			sys_err ("Error in pformat During (POPEN)",errno,PNAME);

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout,".LP%d\n",local_rec.printerNumber);
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

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, local_rec.startSupplierNo);
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no) &&
				!strcmp (sumr_rec.est_no, branchNumber))
	{
		if (sumr_rec.letter [0] != 'Y')
		{
			if (!DISPLAY_IT)
			{
				if (!strcmp (sumr_rec.crd_no, local_rec.endSupplierNo))  
					break;
			}
			else
			{
				if (!strcmp (sumr_rec.crd_no, local_rec.startSupplierNo))
					break;
			}

			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		} 

		strcpy (comr_rec.co_no, sumr_rec.co_no); 
		cc = find_rec ("comr", &comr_rec, EQUAL, "r");

		strcpy (dbph_rec.co_no, comm_rec.co_no);
		sprintf (dbph_rec.letter_code, "S-%-8.8s", local_rec.lett_code);
		cc = find_rec (dbph, &dbph_rec, EQUAL, "r");
		if (!cc && !strcmp (dbph_rec.co_no, comm_rec.co_no)
			&& !strncmp (dbph_rec.letter_code + 2, local_rec.lett_code, 8))
		{
			strcpy (last_cust, sumr_rec.crd_no); 

			if (DISPLAY_IT)
			{
				sprintf (displayStringOne,"^1%s : %6.6s - (%40.40s) %56.56s^6", ML ("Supplier"),sumr_rec.crd_no, sumr_rec.crd_name, " ");
				Dsp_saverec (displayStringOne);
			}

			if (!DISPLAY_IT)
				dsp_process ("Letter : ", dbph_rec.letter_code);
	
			srec_lcount = 1;

			dbpd_rec.dbph_hash = dbph_rec.dbph_hash;
			dbpd_rec.line_no = 0L;
			cc = find_rec (dbpd, &dbpd_rec, GTEQ, "r");
			while (!cc && dbpd_rec.dbph_hash == dbph_rec.dbph_hash) 
			{
				abc_selfield (dblh, "dblh_dblh_hash"); 
				dblh_rec.dblh_hash = dbpd_rec.dblh_hash;
				cc = find_rec (dblh, &dblh_rec, EQUAL, "r");
				if (!cc && dblh_rec.dblh_hash == dbpd_rec.dblh_hash)
				{
					dbld_rec.dblh_hash = dblh_rec.dblh_hash;
					dbld_rec.line_no = 0L; 
					cc = find_rec (dbld, &dbld_rec, GTEQ, "r");
					while (!cc && dbld_rec.dblh_hash == dblh_rec.dblh_hash)  
					{
						srec_lcount++;
						if (srec_lcount > P_PAGELEN)
						{
							if (DISPLAY_IT)
							{
								sprintf (displayStringOne,"^1%s : %6.6s - (%40.40s) ....................%36.36s^6", ML ("Supplier"),sumr_rec.crd_no, sumr_rec.crd_name, " ");
								Dsp_saverec (displayStringOne);
							}
				
							srec_lcount = 2;
						}
							
						sprintf (parseString,
								"%*.*s%s", 
								defaultMargin, defaultMargin, " ", 
								clip (dbld_rec.desc));
		
						ParseRoutine (parseString);

						if (DISPLAY_IT)
							Dsp_saverec (displayStringTwo);   
						
						memset (displayStringTwo,0,sizeof (displayStringTwo)); 
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
					sprintf (displayStringOne,"^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^^1 END OF LETTER DETAILS FOR %-10.10s ^6^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG", last_cust);
					Dsp_saverec (displayStringOne);
					srec_lcount++;
				}

				for (i = srec_lcount; i < P_PAGELEN; i++)
					Dsp_saverec (" ");
			}
			/*
			else
			{
				 sprintf (displayStringOne,"                                                                                                                                   ");
				Dsp_saverec (displayStringOne);  
			}
			*/
		}

		if (!DISPLAY_IT)
		{
			fprintf (fout, ".PA\n");
			fflush (fout);  
		}

		if (!DISPLAY_IT) 
		{
			if (!strcmp (sumr_rec.crd_no, local_rec.endSupplierNo)) 
				break;
		}
		else
		{
			if (!strcmp (sumr_rec.crd_no, local_rec.startSupplierNo)) 
				break;
		}

		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
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
 char *	wrk_prt)
{
	int		cmd;
	int		i;
	char 	*cptr;
	char 	*dptr;
	char 	*wk_prt = strdup (wrk_prt);
	
	partPrinted = TRUE;

	/*---------------------------
	|	look for caret command	|
	---------------------------*/
	cptr = strchr (wk_prt, '.');
	dptr = wk_prt;
	if (cptr == NULL)
		sprintf (displayStringTwo,"%-120.120s", dbld_rec.desc); 

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
		cmd = ValidateCommand (cptr + 1);

		memset (&workDescription,0,sizeof workDescription);
		if (cmd >= CUR_DAT)
		{
			for (i=0; i< (int)strlen (clip (dbld_rec.desc)); i++)
			{
				if (!strncmp (&dbld_rec.desc [i], ".",1))
					break;
				if (firstTime)
				{
					strncpy (workDescription, &dbld_rec.desc [i],1);
					firstTime = FALSE;
				}
				else
					strncat (workDescription, &dbld_rec.desc [i], 1);
			} 

			SubstitudeCommend (cmd);
			dptr = cptr + 8;
		}
		else
		{
			sprintf (displayStringTwo,"%-120.120s", dbld_rec.desc); 
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
ValidateCommand (
 char *	wk_str)
{
	int	i;

	/*----------------------------------------
	| Dot command is last character on line. |
	----------------------------------------*/
	if (!strlen (wk_str))
		return (-1);

	for (i = 0;i < N_CMDS;i++)
		if (!strncmp (wk_str,dot_cmds [i], 7))
			return (i);

	return (-1);
}

/*==============================================
| Substitute valid .commands with actual data. |
==============================================*/
int
SubstitudeCommend (
 int cmd)
{
	char *	pr_sptr = (char *) 0;
	char	tmp_amt [21];

	switch (cmd)
	{

	/*-------------------------------
	| System Date, format dd/mm/yy. |
	-------------------------------*/
	case	CUR_DAT:
		partPrinted = TRUE;
		strcpy (displayStringTwo, local_rec.systemDate);
		if (!DISPLAY_IT)
			fprintf (fout, "%-10.10s", pr_sptr);
		break;
		
	/*-----------------------------
	| Suppliers Full system date. |
	-----------------------------*/
	case FUL_DAT:
		pr_sptr = GetFormatedDate (comm_rec.crd_date);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-22.22s", pr_sptr);
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
			strcat (displayStringTwo, pr_sptr);
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
			strcat (displayStringTwo, pr_sptr);
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
			strcat (displayStringTwo, pr_sptr);
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
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;
	
	/*-------------------
	| Suppliers Number. |
	-------------------*/
	case	CR_NUMB:
		pr_sptr = clip (sumr_rec.crd_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-6.6s", pr_sptr);
		}
		break;

	/*--------------------
	| Suppliers Acronym. |
	--------------------*/
	case	CR_ACRO:
		pr_sptr = clip (sumr_rec.acronym);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-9.9s", pr_sptr);
		}
		break;


	/*-----------------
	| Suppliers Name. |
	-----------------*/
	case	CR_NAME:
		pr_sptr = clip (sumr_rec.crd_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*--------------------
	| Suppliers Address. |
	--------------------*/
	case	CR_ADR1:
		pr_sptr = clip (sumr_rec.adr1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*--------------------
	| Suppliers Address. |
	--------------------*/
	case	CR_ADR2:
		pr_sptr = clip (sumr_rec.adr2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*--------------------
	| Suppliers Address. |
	--------------------*/
	case	CR_ADR3:
		pr_sptr = clip (sumr_rec.adr3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-40.40s", pr_sptr);
		}
		break;

	/*---------------------------
	| Prospects Contact Name 1. |
	---------------------------*/
	case	CR_CNT1:
		pr_sptr = clip (sumr_rec.cont_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-30.30s", pr_sptr);
		}
		break;

	/*---------------
	| Account Type. |
	---------------*/
	case	ACT_TYP:
		pr_sptr = clip (sumr_rec.acc_type);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-1.1s", pr_sptr);
		}
		break;

	/*------------
	| Bank Code. |
	------------*/
	case	BNK_CDE:
		pr_sptr = clip (sumr_rec.bank_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-3.3s", pr_sptr);
		}
		break;

	/*--------------
	| Bank Branch. |
	--------------*/
	case	BNK_BRN:
		pr_sptr = clip (sumr_rec.bank_branch);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-20.20s", pr_sptr);
		}
		break;

	/*-------------------
	| Discount Percent. |
	-------------------*/
	case	DIS_PER:
		sprintf (tmp_amt,"%8.2f",sumr_rec.disc);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-1.1s", pr_sptr);
		}
		break;
	
	/*-----------
	| Tax Code. |
	-----------*/
	case	TAX_CDE:
		pr_sptr = clip (sumr_rec.tax_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-1.1s", pr_sptr);
		}
		break;

	/*-------------
	| Tax Number. |
	-------------*/
	case	TAX_NUM:
		pr_sptr = clip (sumr_rec.tax_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-15.15s", pr_sptr);
		}
		break;

	/*---------------
	| Phone Number. |
	---------------*/
	case	PHN_NUM:
		pr_sptr = clip (sumr_rec.cont_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-15.15s", pr_sptr);
		}
		break;

	/*-------------
	| Fax Number. |
	-------------*/
	case	FAX_NUM:
		pr_sptr = clip (sumr_rec.fax_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			strcat (displayStringTwo, pr_sptr);
			if (!DISPLAY_IT)
				fprintf (fout, "%-15.15s", pr_sptr);
		}
		break;

	default:
		break;
	}
	strcat (workDescription, displayStringTwo);
	strcpy (displayStringTwo, workDescription);
	memset (&workDescription,0,sizeof workDescription); 

	if (!DISPLAY_IT)
		fflush (fout); 

	return (EXIT_SUCCESS);
}

/*===============================================================
| GetFormatedDate (long-date) returns date in 23 January 1986 . |
===============================================================*/
char *
GetFormatedDate (
	long	currentDate)
{
	DateToFmtString (currentDate, "%e %B %Y", err_str);
	return (err_str);
}

/*========================
| Print Company Details. |
========================*/
void
PrintCoDetails (void)
{
	move (0, 20); line (130);
	print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	move (0, 22); line (130);
}

/*================
| Print Heading. |
================*/
int
heading (
	int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		swide (); 
		clear ();
		
		rv_pr (ML (mlCrMess156), 43,0,1);
		
		move (1,1);
		line (130);

		box (0,2,130,8);
		move (1,4);
		line (129);
		move (1,8);
		line (129);

		PrintCoDetails ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

