/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( tr_vehilist.c    )                               |
|  Program Desc  : ( Transport Vehicle Report Listing.            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  trve, comm, esmr,     ,     ,     ,     , 		  |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  trve,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel    | Date Written  : 11/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (08/05/96)      | Modified by : Liza Santos        |
|  Date Modified : (11/09/97)      | Modified by : Roanna Marcelino   |
|                                                                     |
|  Comments		 : Add vehicle type.                                  |
|  (11/09/97)    : Modified for Multilingual Conversion.              |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: vehilist.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_vehilist/vehilist.c,v 5.3 2002/07/17 09:58:13 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>

int 	first_time	= 1;

char 	*trve = "trve",
     	*comm = "comm",
		*esmr = "esmr",
		*data = "data",
		*sptr;

char	br_no	[3],
		prev_br_no[3],
		vehi_no	[11],
		vehi_type[11],
		desc[41];

float	cap;

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
        {"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"}
	};

	int comm_no_fields = 9;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
        char    tco_short[16];
		char	test_no[3];
		char  	test_name[41];
		char	test_short[16];
		char	tcc_no[3];
		char	tcc_short[10];
	} comm_rec;

	/*=================================
	| Transport Vehicle File Record.  |
	==================================*/
	struct dbview trve_list[] ={
		{"trve_co_no"},
		{"trve_br_no"},
        {"trve_ref"},
		{"trve_desc"},
        {"trve_cap"},
        {"trve_vehi_type"},
		{"trve_hhve_hash"}
	};

	int trve_no_fields = 7;

	struct {
		char	co_no[3];
        char    br_no[3];
        char    vehi_no[11];
        char    desc[41];
		float	cap;
        char    vehi_type[10];
		long	hhve_hash;
	} trve_rec;
	
	/*==========================================+
	 | Establishment/Branch Master File Record. |
	 +==========================================*/
#define	ESMR_NO_FIELDS	3

	struct dbview	esmr_list [ESMR_NO_FIELDS] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
	};

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	br_name [41];
	}	esmr_rec;

	FILE	*fout,
			*fsort,
			*popen(const char *, const char *);

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char	br_no[3];
	char	br_desc[41];
    int     lpno;
	char 	back_gr[2];
	char	over_ni[2];
	char	co_desc[3];
	} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "br_no",	 3, 20, CHARTYPE,
		"NN", "          ",
		" ", " ", " Business Unit    ", "Enter Business Unit. [SEARCH]. Default is All",
		 YES, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.br_desc},
	{1, LIN, "lpno",	 5, 20, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer Number   ", " ",
		 YES, NO, JUSTRIGHT, "", "",(char *)&local_rec.lpno},
    {1,LIN, "back_gr",    7, 20, CHARTYPE,
		"A", "          ",
		" ", "N", " Background (Y/N) ", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.back_gr},
	{1,LIN, "over_ni",    9, 20, CHARTYPE,
		"A", "          ",
		" ", "N", " Overnight  (Y/N) ", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.over_ni}, 
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int);
void HeadingOutput (void);
void ProcessFile (void);
void StoreData (void);
void DisplayReport (void);
void SrchEsmr (char *);
int heading (int);
void HeadingDisplay (char *, char *);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	SETUP_SCR 	(vars);
	OpenDB		();

	/*=============================
	| Set up required parameters  |
	=============================*/
	init_scr();			
	set_tty();         
	set_masks();	
	init_vars(1);

	while (prog_exit == 0)
	{
		/*===========================
   		|    Reset control flags    |
		============================*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars(1);		

		/*================================
		|     Entry screen 1 linear input |
		=================================*/
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		/*============================	
		| Edit screen 1 linear input |
		============================*/
		heading(1);
		scn_display(1);
		edit(1);      
		if (restart)
			continue;
		if (!restart)
		{
			dsp_screen("Printing Vehicle Listing", 
						comm_rec.tco_no, comm_rec.tco_name);
            ProcessFile();
		}
		prog_exit = 1;
	}
	fprintf (fout, ".EOF\n");
	pclose  (fout);
	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================= 
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec( trve, trve_list, trve_no_fields, "trve_id_no" );
	open_rec( esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no" );
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose(trve);
	abc_fclose(esmr);

	abc_dbclose(data);
}

int
spec_valid (
 int field)
{
	/*------------------------------------------
	| Validate Branch Number and Allow Search. |
	------------------------------------------*/
	if (LCHECK("br_no"))
	{
		if (dflt_used)
		{ 
			strcpy (local_rec.br_no, "~~");
 			sprintf(local_rec.br_desc, "%-40.40s", "All Business Units.");
			DSP_FLD("br_no");
			DSP_FLD("br_desc");
   			return(0);
		}
		if (SRCH_KEY)
        {
			SrchEsmr(temp_str);
			return(0);
		}
		strcpy(esmr_rec.co_no, comm_rec.tco_no);
		strcpy(esmr_rec.br_no, local_rec.br_no);
		cc = find_rec(esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			/*Business Unit %s is not on file*/
			print_mess(ML(mlStdMess164));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.br_desc, esmr_rec.br_name);
		DSP_FLD("br_no");
		DSP_FLD("br_desc");
	}

    /*-------------------------
    | Validate Printer Number |
    -------------------------*/
	if ( LCHECK("lpno") )
	{
		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			/*Invalid Printer Number.*/
			print_mess(ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		return(0);
	}
	return(0);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	/*------------------
	| Open format file |
	------------------*/  
	if ((fout = popen("pformat","w"))==0)
		file_err(errno,"pformat","POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.lpno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L160\n");
	fprintf (fout, ".7\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EVEHICLE LISTING\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECOMPANY %s\n", clip(comm_rec.tco_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fprintf (fout, ".B1\n");

	fflush (fout);
}

void
ProcessFile (
 void)
{
	char	blank_br_no[3];

	fsort = sort_open("tr_list");

	if (!strcmp(local_rec.br_no, "~~"))
		strcpy (local_rec.br_no, "  ");

	sprintf (blank_br_no ,"%-2.2s",local_rec.br_no);

	memset (&trve_rec, 0, sizeof (trve_rec));
	strcpy (trve_rec.co_no, comm_rec.tco_no);
	strcpy (trve_rec.br_no, local_rec.br_no);
	strcpy (trve_rec.vehi_no, "          ");
	cc = find_rec (trve,&trve_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp(trve_rec.co_no, comm_rec.tco_no) &&
		   (!strcmp(trve_rec.br_no, local_rec.br_no) ||
		   (!strcmp (blank_br_no,"  "))))
	{
		StoreData();
		cc = find_rec (trve,&trve_rec, NEXT, "r");
	}
	DisplayReport();
	sort_delete(fsort, "tr_list");
}

void			
StoreData (
 void)
{
 	char data_string [100];
    
    sprintf(data_string,"%-2.2s %-10.10s %-40.40s %-15.2f %-10.10s\n",
				trve_rec.br_no,
				trve_rec.vehi_no,
				trve_rec.desc,       
				trve_rec.cap,
				trve_rec.vehi_type),
	sort_save (fsort,data_string);
}

void
DisplayReport (
 void)
{
    int 	first_time = TRUE;

	strcpy (prev_br_no, "  ");

	fsort = sort_sort(fsort, "tr_list");
	sptr =  sort_read(fsort);
	while (sptr != (char *)0)
	{
		sprintf (br_no,		"%-2.2s",	sptr);
		sprintf (vehi_no, 	"%-10.10s", sptr + 3);
		sprintf (desc, 		"%-40.40s", sptr + 14);
		cap = atof( sptr + 55);
		sprintf (vehi_type,	"%-10.10s", sptr + 71);

		strcpy (esmr_rec.co_no, comm_rec.tco_no);
		strcpy (esmr_rec.br_no, br_no);
		cc = find_rec(esmr, &esmr_rec, EQUAL, "r");
		if ((!cc) && !strcmp (esmr_rec.co_no, comm_rec.tco_no) &&
					 !strcmp (esmr_rec.co_no, br_no))
					 
		{	

			if (strcmp (prev_br_no, br_no))
			{
				if (first_time)
				{
					first_time = FALSE;
					HeadingOutput();
					HeadingDisplay(br_no, esmr_rec.br_name);
				}
				else
				{
					HeadingDisplay(br_no, esmr_rec.br_name);
					fprintf (fout, ".PA\n");
				}
				if (strcmp (prev_br_no, br_no))
					strcpy (prev_br_no, br_no);
			}
		}
		fprintf (fout, ".C| %-10.10s  ", vehi_no);
		fprintf (fout, "| %-40.40s  ", desc); 
		fprintf (fout, "| %15.2f   ",cap);
		fprintf (fout, "| %-10.10s   |\n", vehi_type);
		fflush (fout);
	sptr	=	sort_read (fsort);
	}
}

/*================================
| Search for Branch Master File. |
================================*/
void
SrchEsmr (
 char	*key_val)
{
	work_open();
	save_rec("#Branch #","#Description");
	strcpy(esmr_rec.co_no, comm_rec.tco_no);
	sprintf(esmr_rec.br_no,"%-2.2s",key_val);
	cc = find_rec(esmr,&esmr_rec,GTEQ,"r");
	while (!cc && !strcmp( esmr_rec.co_no, comm_rec.tco_no) &&
	      !strncmp(esmr_rec.br_no,key_val,strlen(key_val)))
	{
		cc = save_rec(esmr_rec.br_no,esmr_rec.br_name);
		if (cc)
			break;
		cc = find_rec(esmr,&esmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy( esmr_rec.co_no, comm_rec.tco_no);
	sprintf(esmr_rec.br_no,"%-2.2s", temp_str);
	cc = find_rec(esmr,&esmr_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, esmr, "DBFIND" );
}

int
heading (
 int scn)
{
	char string[50] = "";

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);       /* Set a screen ready for manipulation */
		/*Print Vehicle Listings */
		strcpy (string, ML(mlTrMess051));

		clear();
		rv_pr(string, (80 - strlen (string))/2,0,1);  /* Reverse video print */
        move(0,1);
        line(81);

		box(0,2,80,7);
		move(1,4);
		line(79);
		move(1,6);
		line(79);
		move(1,8);
		line(79);
 
        move(0,20);       
        line(80);

		print_at(21,1,ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_short);
		print_at(21,30,ML(mlStdMess039),comm_rec.test_no,comm_rec.test_short);
		print_at(21,60,ML(mlStdMess099),comm_rec.tcc_no,comm_rec.tcc_short);
        move(0,22);
        line(80);
		scn_write(scn);   /* Display all screen prompts */
	}

    return (EXIT_SUCCESS);
}

void
HeadingDisplay (
 char *br_no, 
 char *br_name)
{

	fprintf (fout, ".DS6\n");   

    fprintf (fout, ".EBRANCH : %s %s\n",
					br_no, clip(br_name)); 
	fprintf (fout, ".B1\n");
 
	fprintf (fout, ".R.C==============");
	fprintf (fout, "=============================================");
	fprintf (fout, "===================");
	fprintf (fout, "================\n");

	fprintf (fout, ".C--------------");
	fprintf (fout, "---------------------------------------------");
	fprintf (fout, "-------------------");
	fprintf (fout, "----------------\n");

	fprintf (fout, ".C| Vehicle No. ");
	fprintf (fout, "|               Description                 ");
	fprintf (fout, "|     Capacity     ");
	fprintf (fout, "|  Vehicle Type  |\n ");

	fprintf (fout, ".C--------------");
	fprintf (fout, "+--------------------------------------------");
	fprintf (fout, "+------------------");
	fprintf (fout, "+---------------\n");
	fflush(fout);  
}
