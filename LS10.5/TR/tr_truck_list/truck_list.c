/*==============================================================================
|  Copyright (C) 1988 - 1995 Logistic Software Limited.                        |
|==============================================================================|
|  Program Name  : (tr_truck_hist.c)                                           |
|  Program Desc  : (Driver' List)                                            |
|	                                                                           |
|                  (                                           )               |
|------------------------------------------------------------------------------|
|  Access files  : comm, extf                                                  |
|  Database      : (data)                                                      |
|------------------------------------------------------------------------------|
|  Updates Files :                                                             |
|  Database      : ( N/A)                                                      |
|------------------------------------------------------------------------------|
|  Author        : Bernard M. dela Vega | Date Written  : 05/20/96             |
|------------------------------------------------------------------------------|
|  Date Modified : (00/00/00)           | Modified  by  :                      |
|                :                                                             |
|  Comments      :                                                             |
|                :                                                             |
|$Log: truck_list.c,v $
|Revision 5.4  2002/07/17 09:58:13  scott
|Updated to change argument to get_lpno from (1) to (0)
|
|Revision 5.3  2002/03/01 02:53:19  scott
|Updated description from Trucker to Driver
|
|Revision 5.2  2001/08/09 09:23:11  scott
|Updated to add FinishProgram () function
|
|Revision 5.1  2001/08/06 23:53:54  scott
|RELEASE 5.0
|
|Revision 5.0  2001/06/19 08:21:51  robert
|LS10-5.0 New Release as of 19 JUNE 2001
|
|Revision 4.0  2001/03/09 02:43:02  scott
|LS10-4.0 New Release as at 10th March 2001
|
|Revision 3.0  2000/10/10 12:23:41  gerry
|Revision No. 3 Start
|<after Rel-10102000>
|
|Revision 2.0  2000/07/15 09:14:36  gerry
|Forced Revision No Start 2.0 Rel-15072000
|
|Revision 1.11  2000/02/22 08:17:40  ana
|(22/02/2000) SC2017 Corrected saving of extf.
|                                                             |
==============================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: truck_list.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_truck_list/truck_list.c,v 5.4 2002/07/17 09:58:13 scott Exp $";

#include <pslscr.h>
#include <ml_tr_mess.h>
#include <ml_std_mess.h>
#include <dsp_process.h>
#include <dsp_screen.h>
#include <get_lpno.h>
#include <twodec.h>
#include <pr_format3.h>

char	*data	= "data",
		*comm	= "comm",
		*extf	= "extf",
		*DBFIND	= "DBFIND";

FILE	*fout,
		*fsort;

/*====================+
| System Common File. |
+====================*/
#define	COMM_NO_FIELDS 6	

	struct dbview	comm_list [COMM_NO_FIELDS] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"}
	};

	struct tag_commRecord
	{
		int		term;
		char	tco_no [3];
		char	co_name [41];
		char	co_short [16];
		char	test_no [3];
		char	test_name [41];
	}	comm_rec;


/*========================+
 | External Driver file. |
 +========================*/
#define	EXTF_NO_FIELDS	3

	struct dbview	extf_list [EXTF_NO_FIELDS] =
	{
		{"extf_co_no"},
		{"extf_code"},
		{"extf_name"}
	};

	struct tag_extfRecord
	{
		char	co_no [3];
		char	code [7];
		char	name [41];
	}	extf_rec;


/*============================+
| Local and Screen Structure  |
+============================*/
	struct
	{
		char	dummy[11];
		char	truck_code[7];
		char    truck_desc[41]; 
		char	onight[2];
		int		lpno;
		char	lp_str[3];
		char	back[2];
	}	local_rec;
	
static struct var  vars[] =
{
	{1, LIN, "tr_code", 3, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Driver Code        ", "Enter Driver's Code. [Search]. Default is All",
		YES, NO,  JUSTLEFT, "", "", local_rec.truck_code},

	{1, LIN, "tr_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.truck_desc},

	{1, LIN, "ptr_no",	 5, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No         ", "Enter printer no.",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	{1, LIN, "back",	 7, 18, CHARTYPE,
		"U", "          ",
		" ", "N", "Background (Y/N)   ", "Print Report In The Background",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},

	{1, LIN, "onight",	 7, 50, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight (Y/N)   ", "Print Report In Overnight Batch",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
	};


/*======================+
| Function Declarations |
+======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	EndPrinting 	(void);
void 	shutdown_prog 	(void);
int 	spec_valid 		(int);
void 	SrchExtf 		(char *);
int 	heading 		(int);
void 	HeadOutput 		(void);
void 	ProcessFile 	(void);
void 	StoreSummary 	(void);
void 	DisplaySummary 	(void);
void 	RunReport 		(char *);
/*==========================+
| Main Processing Routine . |
+==========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 1 && argc != 3)
	{
		print_at(0,0,mlTrMess703,argv[0]);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB();

	if (argc == 3)
	{
		sprintf(local_rec.truck_code,"%-6.6s",argv[1]);
		local_rec.lpno = atoi (argv[2]);
		dsp_screen("Processing : Driver List ",
					comm_rec.tco_no, comm_rec.co_name);

		ProcessFile();

		fprintf(fout,".EOF\n");
		EndPrinting();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*--------------------------+
	| Setup required parameters |
	+--------------------------*/
	init_scr();			/*  sets terminal from termcap	*/
	set_tty();          /*  get into raw mode		*/
	set_masks();		/*  setup print using masks	*/
	init_vars(1);		/*  set default values		*/

	while (prog_exit == 0)
	{
		/*--------------------+
		| Reset control flags |
		+--------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars(1);	

		/*----------------------------+
		| Entry screen 1 linear input |
		+----------------------------*/
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		/*---------------------------+
		| Edit screen 1 linear input |
		+---------------------------*/
		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		RunReport(argv[0]); 
		prog_exit = TRUE;
	}
	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*=======================+
| Open data base files . |
+=======================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");
	read_comm( comm_list, COMM_NO_FIELDS, (char *)&comm_rec );

	open_rec (extf, extf_list, EXTF_NO_FIELDS, "extf_id_no");
}


/*========================+
| Close data base files . |
+========================*/
void
CloseDB (
 void)
{
	abc_fclose(extf);
}


/*========================+
|   Program File Close    |
+========================*/
void
EndPrinting (
 void)
{
	pclose(fout);
}

/*=======================+
| Program exit sequence. |
+=======================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
	abc_dbclose("data");
}


/*===========+
| Validation |
+===========*/
int
spec_valid (
 int field)
{

	/*-+---------------------------------------+
	| Validate Driver's Code And Allow Search |
	+--+--------------------------------------*/
	if (LCHECK("tr_code"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.truck_code, "      ");
			sprintf (local_rec.truck_desc, "%-40.40s", "All Drivers.");
			DSP_FLD ("tr_code");
			DSP_FLD ("tr_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
	 		SrchExtf (temp_str);
  			return (EXIT_SUCCESS);
		}

        strcpy (extf_rec.co_no, comm_rec.tco_no);		
		strcpy (extf_rec.code, local_rec.truck_code);
		cc = find_rec (extf, &extf_rec, EQUAL, "r");
		if (cc)
		{
			errmess(ML(mlTrMess005));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.truck_code, extf_rec.code);
		strcpy (local_rec.truck_desc, extf_rec.name);
		DSP_FLD ("tr_code");
		DSP_FLD ("tr_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------------+
	| Validate Printer No |
	+--------------------*/
	if (LCHECK("ptr_no"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}
	
		if (!valid_lp(local_rec.lpno))
		{
			print_mess(ML(mlStdMess020));
			sleep(3);
			clear_mess();
			return(1);
		}
		return(0);
	}
	return(0);
}


/*==================================+
| Search for Drivers's master file. |
+==================================*/
void
SrchExtf (
 char *key_val)
{
	_work_open (6,0,40);
	strcpy  (extf_rec.co_no,  comm_rec.tco_no);
	sprintf (extf_rec.code,  "%-6.6s", key_val);
	save_rec ("#Driver", "#Drivers Code Description");
	cc = find_rec (extf, &extf_rec, GTEQ, "r");
	while (!cc && (strlen (comm_rec.tco_no) ?
			!strcmp  (extf_rec.co_no, comm_rec.tco_no) :
			!strcmp  (extf_rec.co_no, comm_rec.tco_no))
			&& !strncmp (extf_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (extf_rec.code, extf_rec.name);
		if (cc)
			break;

		cc = find_rec (extf, &extf_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();
	if (cc)
		return;

	strcpy  (extf_rec.co_no,  comm_rec.tco_no);
	sprintf (extf_rec.code, "%-6.6s", temp_str);
	cc = find_rec (extf, &extf_rec, EQUAL, "r");
	if (cc)
	 	file_err (cc, extf, "DBFIND");
}


/*===============+
| Screen Display |
+===============*/
int
heading (
 int scn)
{
	if ( restart ) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set(scn);
	clear();
	rv_pr(ML(mlTrMess031),30,0,1);
	move(0,1);
	line(80);

	box(0,2,80,5);
	move(1,4);
	line(79);

	move(1,6);
	line(79);

	move(0,19);
	line(80);
	move(0,20);
	strcpy(err_str,ML(mlStdMess038));
	print_at(20,0,err_str, comm_rec.tco_no,clip(comm_rec.co_name));

	move(0,21);
	line(80);
	line_cnt = 0;
	scn_write(scn);

    return (EXIT_SUCCESS);
}

/*==================================+
| Start Out Put To Standard Print . |
+==================================*/
void
HeadOutput (
 void)
{

	/*-----------------+
	| Open format file |
	+-----------------*/
	if ((fout = popen("pformat","w")) == NULL)
		file_err(errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout, ".LP%d\n",local_rec.lpno);
	fprintf(fout, ".PI10\n");
	fprintf(fout, ".L80\n");
	fprintf(fout, ".9\n");
	fprintf(fout, ".B1\n");
    fprintf(fout, ".E%s\n",clip(comm_rec.co_name));
	fprintf(fout, ".EDriver List Report\n");
	fprintf(fout, ".B1\n");
	fprintf(fout, ".B1\n");
	fprintf(fout, "====================");
	fprintf(fout, "====================");
	fprintf(fout, "====================");
	fprintf(fout, "====================");
	fprintf(fout, "====================\n");
    fprintf(fout, "|         T R U C K E R  C O D E       ");
	fprintf(fout, "|        T R U C K E R  N A M E         |\n");
	fprintf(fout, "--------------------");
	fprintf(fout, "--------------------");
	fprintf(fout, "--------------------");
	fprintf(fout, "--------------------\n");
	fprintf(fout, ".R====================");
	fprintf(fout, "====================");
	fprintf(fout, "====================");
	fprintf(fout, "====================");
	fprintf(fout, "====================\n");
	fflush (fout);
}

void
ProcessFile (
 void)
{
	fsort = sort_open ("truck_list");

	strcpy (extf_rec.co_no, comm_rec.tco_no);
	strcpy (extf_rec.code, local_rec.truck_code);
	cc = find_rec (extf,&extf_rec,GTEQ,"r");

	while (!cc && !strcmp(extf_rec.co_no, comm_rec.tco_no))
	{
		StoreSummary();
		if (!strcmp(extf_rec.code, local_rec.truck_code))
			break;
		cc = find_rec (extf, &extf_rec, NEXT,"r");
	}
	DisplaySummary();
	fflush (fout);
	sort_delete(fsort, "truck_list");
}

void
StoreSummary (
 void)
{
	char	data_string[50];

	sprintf (data_string,"%-6.6s%-40.40s\n",
							extf_rec.code,
							extf_rec.name);
	sort_save (fsort, data_string);
}

void
DisplaySummary (
 void)
{
	char	*sptr;
	char	tr_code[7],
			prev_tr_code[7],
			tr_desc[41];

	int		first_time = TRUE;

	memset (tr_code, 0, sizeof(tr_code));
	memset (prev_tr_code, 0, sizeof(prev_tr_code));
	memset (tr_desc, 0, sizeof(tr_desc));

	fsort = sort_sort(fsort, "truck_list");

	sptr = sort_read(fsort);
	strcpy (prev_tr_code, "");

	while (sptr != (char *)0)
	{
		sprintf (tr_code, "%-6.6s", sptr);
		sprintf (tr_desc, "%-40.40s", sptr + 6);

		strcpy (extf_rec.co_no, comm_rec.tco_no);
		strcpy (extf_rec.code, tr_code);
		cc = find_rec(extf, &extf_rec, EQUAL, "r");
		if (cc)
			file_err (cc, extf,"DBFIND");

		if (strcmp(prev_tr_code,tr_code))
		{
			if (!first_time)
			{
				fprintf(fout, "--------------------");
				fprintf(fout, "--------------------");
				fprintf(fout, "--------------------");
				fprintf(fout, "--------------------\n");
				first_time = FALSE;
			}
			else
			{
				HeadOutput();
				first_time = FALSE;
			}
			if (strcmp(prev_tr_code, tr_code))
			{
				strcpy (prev_tr_code, tr_code);
			}
		}
		fprintf(fout, "|                  %-6.6s              ", tr_code);
		fprintf(fout, "|  %-40.40s|\n", tr_desc);
		sptr = sort_read(fsort);
	}
}

void
RunReport (
 char *prog_name)
{
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);

	shutdown_prog ();

	if (local_rec.onight[0] == 'Y')
	{
		/*"Print Driver List Report", (char *)0);*/
		if (fork() == 0)
		{
			execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.truck_code, 
					local_rec.lp_str, 
					err_str, (char *)0);
		}
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
		{
			execlp (prog_name,
					prog_name,
					local_rec.truck_code, 
					local_rec.lp_str, (char *)0);
		}
	}
	else
	{
		execlp (prog_name,
				prog_name,
				local_rec.truck_code, 
				local_rec.lp_str, (char *)0);
	}
}
