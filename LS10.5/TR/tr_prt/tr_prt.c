/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( tr_prt.c)                                        |
|  Program Desc  : ( Printing trucker records )                   	  |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, extf                                        |
|                :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      :                                                    |
|---------------------------------------------------------------------|
|  Date Written  : 05/20/96        |  Author : Edz Monserrate         |
|---------------------------------------------------------------------|
|  Date Modified : 11/09/97        |  Modified By : Marnie Organo     |
|  Date Modified : 14/10/97        |  Modified By : Marnie Organo     |
|                :     												  |	 
|                :     												  |	 
|  Comments      :     												  |	 
|  (14/10/97)    : Fixed mldb DBSTRUCTVIEW error.                     |
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: tr_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_prt/tr_prt.c,v 5.3 2002/07/17 09:58:13 scott Exp $";

#include 	<pslscr.h>
#include	<get_lpno.h>
#include 	<dsp_process2.h>
#include 	<dsp_screen.h>
#include 	<ml_std_mess.h>
#include 	<ml_tr_mess.h>

static char	*data	= "data",
			*extf	= "extf";


	/*=====================+
	 | System Common File. |
	 +=====================*/
#define	COMM_NO_FIELDS	8

	struct dbview	comm_list [COMM_NO_FIELDS] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"},
	};

	struct tag_commRecord
	{
		int		term;
		char	co_no [3];
		char	co_name [41];
		char	co_short [16];
		char	est_no [3];
		char	est_short [16];
		char	cc_no [3];
		char	cc_short [10];
	}	comm_rec;

	/*========================+
	 | External Trucker file. |
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

/*============================
| Local & Screen Structures. |
============================*/

struct {
	char	co_no [3];
	char	code [7];
	char	name [41];
	int		lpno;
	char	lp_str [3];
	char	back [5];
	char	onite [5];
	char	dummy [11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "code",	 4, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", "Trucker Code:", "Enter Trucker Code (Default All) [SEARCH]",
		 YES, NO,  JUSTLEFT, "", "", local_rec.code},
	{1, LIN, "name",	 6, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Trucker Name:", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.name},
	{1, LIN, "lpno",	 8, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number   :", "Enter Printer Number ",
		 YES, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",  	 10, 18, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Background (Y/N) :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	 10, 54, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Overnight (Y/N)  :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

FILE 	*fout,
		*fsort;
char	curr_co[3],
		errs_str[9];

/*=====================
 Function declarations
======================*/
int spec_valid (int field);
int heading (int scn);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void end_prnt (void);
void run_prog (char *prog_name);
void head_output (void);
void process_file (void);
void store_data (void);
void display_report (void);
void head_display (void);
void SrchExtf (char *key_val);


/*==========================
| Main Processing Routine  |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 1 && argc != 3)
	{
		print_at(0,0,mlStdMess036, argv[0]);
		return (EXIT_FAILURE);
	}
	SETUP_SCR (vars);
	OpenDB();
	if (argc == 3)
	{
		sprintf(local_rec.code,"%-6.6s",argv[1]);
		local_rec.lpno = atoi(argv[2]);
		dsp_screen("Printing Trucker Records",
					comm_rec.co_no,comm_rec.co_name);
		head_output();
		process_file();	
		fprintf(fout,".EOF\n");
		end_prnt();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/* ============================
	| Set up required parameters  |
	=============================*/
	
	init_scr();			
	set_tty();         
	set_masks();	
	init_vars(1);

	while (prog_exit == 0)
	{
		/*=========================	
		|   Reset control flags   |
		=========================*/
	
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars(1);

		/*=============================	
		| Entry screen 1 linear input |
		=============================*/
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
		strcpy(err_str, ML(mlTrMess052));
		run_prog (argv[0]);	
		prog_exit = 1;
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
	CloseDB (); 
	FinishProgram ();
}


/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);
	open_rec( extf, extf_list, EXTF_NO_FIELDS, "extf_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose(extf);
	abc_dbclose(data);
}

int
spec_valid (
 int field)
{
	/*------------------------------------------
	| Validate Status Code And Allow Search. |
	------------------------------------------*/
	if (LCHECK("code"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.code, "      ");
			sprintf (local_rec.name, "%-40.40s", "All Trucker Records");
			DSP_FLD ("code");
			DSP_FLD ("name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExtf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (extf_rec.co_no, comm_rec.co_no);
		strcpy (extf_rec.code, local_rec.code);
		cc = find_rec (extf, &extf_rec, EQUAL, "r");
		if (cc)
		{
		/*	sprintf (err_str, "Trucker Code %s is not on file.", 
			local_rec.code);*/
			errmess (ML(mlTrMess005));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.name, extf_rec.name);
		DSP_FLD ("code");
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
	/*-------------
	| Printer No. | 
	-------------*/
	if (LCHECK("lpno"))
	{
		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp(local_rec.lpno))
		{
			/*print_mess("Invalid Printer Number");*/
			print_mess(ML(mlStdMess020));
			sleep(2);
			clear_mess();
			return(1);
		}
		return(0);
	}

	/*-----------------------------------------------
	| Validate for Background or foreground process |
	-----------------------------------------------*/
	if ( LCHECK("back") )
	{
		strcpy(local_rec.back,(local_rec.back[0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD( "back" );
		return(0);
	}

	if ( LCHECK("onight") )
	{
		strcpy(local_rec.onite,(local_rec.onite[0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD( "onight" );
		return(0);
	}
	return(0);
}

void
end_prnt (
 void)
{
	pclose(fout);
}

void
run_prog (
 char *prog_name)
{
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);

	shutdown_prog ();
	
	if (local_rec.onite[0] == 'Y')
	{
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.code,
				local_rec.lp_str,
				err_str,(char *)0);
				/*"Print Trucker Records",(char *)0);*/
		/*else
			exit(0);*/
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp(prog_name,
				prog_name,
				local_rec.code,
				local_rec.lp_str,(char *)0);
		/*else
			exit(0);*/
	}
	else 
	{
		execlp(prog_name,
			prog_name,
			local_rec.code,
			local_rec.lp_str,(char *)0);
	}
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	if ((fout = popen("pformat","w")) == (FILE *) NULL)
		file_err(errno, "pformat", "POPEN" );

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".PI12\n");
	fprintf(fout,".L80\n");
	fprintf(fout,".4\n");
	fprintf(fout,".B1\n");
	fprintf(fout,".E%s\n", clip(comm_rec.co_name));
	fprintf(fout,".ETrucker Record Listing\n\n");
	fprintf (fout,".R================="); 
	fprintf (fout,"==========================================\n");
	fflush (fout); 
}

void
process_file (
 void)
{
	fsort=sort_open("tr_prt");
	strcpy (extf_rec.co_no, comm_rec.co_no);
	strcpy (extf_rec.code, local_rec.code);
	cc = find_rec (extf, &extf_rec, GTEQ, "r");
	while ((!cc) && !strcmp (extf_rec.co_no, comm_rec.co_no))
	{
		store_data();
		if (!strcmp (extf_rec.code, local_rec.code))
			break;
		cc = find_rec (extf, &extf_rec,NEXT,"r");
	}
	display_report();
	fflush(fout);
	sort_delete (fsort, "tr_prt");
}

void
store_data (
 void)
{
	char data_string [300];
	sprintf(data_string,"%-6.6s %-40.40s\n",
	extf_rec.code, extf_rec.name);
	sort_save (fsort, data_string);
}

void
display_report (
 void)
{
	char 	*sptr;
    char	code    	[7],
		 	name        [41],
			co_no 		[3],
			prev_co_no	[3];
	int		first_time = TRUE;	
	fsort = sort_sort (fsort, "tr_prt");
	sptr =  sort_read (fsort);
	while (sptr != (char *)0)
	{
		sprintf (code, "%-6.6s", sptr);
		sprintf (name, "%-40.40s", sptr + 7);
		strcpy (co_no, extf_rec.co_no); 
			if (strcmp (prev_co_no, co_no) ) 
			{
				if (!first_time)
				{
					first_time = FALSE;	
					head_display();
					fprintf (fout,".PA\n");	
				}
				else 
				{
					head_display();
					first_time = FALSE;	
				}
				if (strcmp (prev_co_no, co_no)) 
					strcpy (prev_co_no, co_no);
			}
		fprintf (fout,"|      %-6.6s    ", code); 	
		fprintf (fout,"|%-40.40s|\n", name); 	
		sptr = sort_read (fsort);
	}
}

void
head_display (
 void)
{
	fprintf (fout, ".DS6\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".B1\n");
	fprintf (fout,"================="); 
	fprintf (fout,"==========================================\n");
	fprintf (fout,"|  Trucker Code  ");
	fprintf (fout,"|              Trucker Name              |\n"); 
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------------------------------|\n");
	fflush (fout);
}

int
heading (
 int scn)
{
	if ( restart ) 
		return (EXIT_SUCCESS);
	if (scn != cur_screen)
		scn_set(scn);    
	clear();
	rv_pr(ML(mlTrMess006),25,0,1);
	move(0,1);
	line(80);
	box(0,3,80,7);
	move(1,5);
	line(79);	
	move(1,7);
	line(79);
	move(1,9);  
	line(79);
	move(0,20); 
	line(80);
	print_at(21,0,ML(mlStdMess038), comm_rec.co_no, clip(comm_rec.co_short));
	print_at(21,29,ML(mlStdMess039), comm_rec.est_no, clip(comm_rec.est_short));
	print_at(21,57,ML(mlStdMess099), comm_rec.cc_no, clip(comm_rec.cc_short));
	move(0,22);
	line(80);
	line_cnt = 0;
	scn_write(scn);   
    return (EXIT_SUCCESS);
}

/*==========================
| Search for Trucker file. |
==========================*/
void
SrchExtf (
 char *key_val)
{
	work_open ();
	strcpy  (extf_rec.co_no,  comm_rec.co_no);
	sprintf (extf_rec.code,  "%-6.6s", key_val);
	save_rec ("#TC", "#Trucker Name");
	cc = find_rec (extf, &extf_rec, GTEQ, "r");
	while (!cc 
	&&     !strcmp  (extf_rec.co_no, comm_rec.co_no) 
	&&     !strncmp (extf_rec.code, key_val, strlen (key_val)) )
	{
		cc = save_rec (extf_rec.code, extf_rec.name);
		if (cc)
			break;

		cc = find_rec (extf, &extf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy  (extf_rec.co_no, comm_rec.co_no);
	sprintf (extf_rec.code, "%-6.6s", temp_str);
	cc = find_rec (extf, &extf_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, extf, "DBFIND");
}
