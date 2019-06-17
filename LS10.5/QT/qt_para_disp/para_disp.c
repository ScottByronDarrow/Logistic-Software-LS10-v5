/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: para_disp.c,v 5.3 2002/07/17 09:57:43 scott Exp $
|  Program Name  : (qt_para_disp.c)
|  Program Desc  : (Inquiry Note Pad Display Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 01/01/91         |
|---------------------------------------------------------------------|
| $Log: para_disp.c,v $
| Revision 5.3  2002/07/17 09:57:43  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:44:43  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:19  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:48  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/05 04:38:20  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: para_disp.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_para_disp/para_disp.c,v 5.3 2002/07/17 09:57:43 scott Exp $";

#define MAXSCNS 	2
#define MAXLINES	500
#define TABLINES	10
#define	P_PAGELEN	17

#define	X_OFF		0
#define	Y_OFF		0

#include <std_decs.h>
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_qt_mess.h>


#define	BACK 		(local_rec.back  [0] == 'Y')
#define	ONITE 		(local_rec.onite [0] == 'Y')

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int	new_note = 0;

	extern	int	TruePosition;
	extern	int	EnvScreenOK;

#include	"schema"

struct commRecord	comm_rec;
struct qtlhRecord	qtlh_rec;
struct qtldRecord	qtld_rec;

#include	<qt_commands.h>

/*===========================
| Local & Screen Structures |
===========================*/
struct {			  			/*---------------------------------------*/
	char	dummy [11];	  		/*| Dummy Used In Screen Generator.		|*/
	char	startParagraph [11];/*| Holds Start Paragraph            	|*/
	char	endParagraph [11];  /*| Holds End Paragraph               	|*/
	char	outputTo [2];     	/*|                                   	|*/
	char	outputToDesc [11];  /*|                                   	|*/
	int		printerNo;			/*| Holds Printer Number              	|*/
	char	back [2];          	/*| Holds Background Flag             	|*/
	char	backDesc [11];		/*| Holds Background Flag             	|*/
	char	onite [2];         	/*| Holds Overnight Flag              	|*/
	char	oniteDesc [11];    	/*| Holds Overnight Flag              	|*/
              		          	/*---------------------------------------*/
} local_rec;            

int	DISPLAY_IT;
		
static	struct	var	vars []	={	

	{1, LIN, "startParagraph", 3, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "          ", "Start Paragraph   ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startParagraph}, 
	{1, LIN, "endParagraph", 3, 60, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "~~~~~~~~~~", "End Paragraph     ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endParagraph}, 
	{1, LIN, "outputTo", 5, 2, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Output To         ", " Enter Printer / Screen ", 
		YES, NO, JUSTLEFT, "SP", "", local_rec.outputTo}, 
	{1, LIN, "outputToDesc", 5, 26, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.outputToDesc}, 
	{1, LIN, "printerNo", 6, 2, INTTYPE, 
		"NN", "          ", 
		" ", "",  "Printer Number    ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "back", 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background        ", " Process in Background [Y/N] ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 7, 26, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.backDesc}, 
	{1, LIN, "onite", 8, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight         ", " Process Overnight [Y/N] ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onite}, 
	{1, LIN, "oniteDesc", 8, 26, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.oniteDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*==========================
| Function prototypes.     |
==========================*/
int		heading 		 	(int);
void	RunProgram		 	(char *);
void	shutdown_prog	 	(void);
void	OpenDB			 	(void);
void	CloseDB			 	(void);
void	SrchQtlh		 	(char *);
void	DisplayParagraph	(void);
void	SetOptions 			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	if (argc != 1 && argc != 4)
	{
		print_at (0,0,ML (mlQtMess701),argv [0]);
		return (argc);
	}

	SETUP_SCR (vars);

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (argc == 4)
	{
		sprintf (local_rec.startParagraph, 	"%-10.10s", argv [1]);
		sprintf (local_rec.endParagraph, 	"%-10.10s", argv [2]);
		local_rec.printerNo = atoi (argv [3]);
		DSP_UTILS_lpno	=	local_rec.printerNo;
		DISPLAY_IT = FALSE;
		if (local_rec.printerNo == 0)
			DISPLAY_IT = TRUE;

		DisplayParagraph ();

		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	set_tty ();
	init_scr ();
	set_masks ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		lcount [2] = 0;
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
	char	lp_str [3];

	sprintf (lp_str,"%d",local_rec.printerNo);

	shutdown_prog ();

	if (!strncmp (local_rec.onite, "Y", 1))
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.startParagraph,
				local_rec.endParagraph,
				lp_str,
				ML (mlQtMess008), (char *)0);
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
				local_rec.startParagraph,
				local_rec.endParagraph,
				lp_str, (char *)0);
		}
		else
			return;
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			local_rec.startParagraph,
			local_rec.endParagraph,
			lp_str, (char *)0);
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

	open_rec (qtld, qtld_list, QTLD_NO_FIELDS, "qtld_id_no");
	open_rec (qtlh, qtlh_list, QTLH_NO_FIELDS, "qtlh_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (void)
{
	abc_fclose (qtld);
	abc_fclose (qtlh);
	abc_dbclose ("data");
}

int
spec_valid (
 int	field)
{
	/*-----------------------
	| Validate Para Code    |
	-----------------------*/
	if (LCHECK ("startParagraph"))
	{
	   	if (SRCH_KEY)
	   	{
			SrchQtlh (temp_str);
			strcpy (local_rec.startParagraph, qtlh_rec.par_code);
			return (EXIT_SUCCESS);
	   	}

		if (dflt_used)
			return (EXIT_SUCCESS);

		strcpy (qtlh_rec.co_no,comm_rec.co_no);
		strcpy (qtlh_rec.par_code, local_rec.startParagraph);
		cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess047));
			return (EXIT_FAILURE);
		}

	   	return (EXIT_SUCCESS);
	}

	if (LCHECK ("endParagraph"))
	{
	   	if (SRCH_KEY)
	   	{
			SrchQtlh (temp_str);
			strcpy (local_rec.endParagraph, qtlh_rec.par_code);
			return (EXIT_SUCCESS);
	   	}

		if (dflt_used)
			return (EXIT_SUCCESS);

		if (strcmp (local_rec.startParagraph, local_rec.endParagraph) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (qtlh_rec.co_no,comm_rec.co_no);
		strcpy (qtlh_rec.par_code, local_rec.endParagraph);
		cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess047));
			return (EXIT_FAILURE);
		}

	   	return (EXIT_SUCCESS);
	}

	if (LCHECK ("outputTo"))
	{
		if (local_rec.outputTo [0] == 'S')
		{
			FLD ("printerNo")	= NA;
			FLD ("back")		= NA;
			FLD ("onite")		= NA;

			strcpy (local_rec.back,  "N");
			strcpy (local_rec.onite, "N");

			SetOptions ();
			local_rec.printerNo = 0;
			strcpy (local_rec.outputToDesc, ML ("Screen "));
			strcpy (local_rec.outputTo, "S");
			DSP_FLD ("outputToDesc");

			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
		else
		{
			FLD ("printerNo")	= YES;
			FLD ("back")		= YES;
			FLD ("onite")		= YES;
			
			strcpy (local_rec.outputToDesc, ML ("Printer "));
			DSP_FLD ("outputToDesc");
			return (EXIT_SUCCESS);
		}
	}

	/*----------------------
	| Validate Printer No. |
	----------------------*/
	if (LCHECK ("printerNo"))
    {
	   	if (SRCH_KEY)
			get_lpno (0);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
		{
			strcpy (local_rec.onite, "N");
			entry_exit = TRUE;
		}

		SetOptions ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		if (local_rec.onite [0] == 'Y')
			strcpy (local_rec.back, "N");

		SetOptions ();
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=============================================
| Search routine for Paragraph master file. |
=============================================*/
void
SrchQtlh (
 char *	key_val)
{
	_work_open (10,0,40);
	save_rec ("#Code","#Description");
	strcpy (qtlh_rec.co_no,comm_rec.co_no);
	sprintf (qtlh_rec.par_code, "%-10.10s", key_val);
	cc = find_rec (qtlh, &qtlh_rec, GTEQ, "r");
	while (!cc && !strcmp (qtlh_rec.co_no,comm_rec.co_no) &&
	          !strncmp (qtlh_rec.par_code,key_val,strlen (key_val)))
	{
		cc = save_rec (qtlh_rec.par_code,qtlh_rec.par_desc);
		if (cc)
			break;

		cc = find_rec (qtlh, &qtlh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qtlh_rec.co_no,comm_rec.co_no);
	sprintf (qtlh_rec.par_code, "%-10.10s", key_val);
	cc = find_rec (qtlh, &qtlh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, qtlh, "DBFIND");
}

void
DisplayParagraph (void)
{
	char	err_str1 [200];
	char	disp_str [200];
	char	last_para [11];
	int		srec_lcount = 0;
	int		i;

	if (DISPLAY_IT)
	{
		init_scr ();
		set_tty ();
		swide ();
		clear ();
		Dsp_prn_open (0,0,P_PAGELEN, "P a r a g r a p h   P r i n t",
			comm_rec.co_no, comm_rec.co_name,
			 (char *) 0, (char *) 0,
			 (char *) 0, (char *) 0);
	}
	else
	{
		Dsp_nd_prn_open (0,0,P_PAGELEN, "P a r a g r a p h   P r i n t",
			comm_rec.co_no, comm_rec.co_name,
			 (char *) 0, (char *) 0,
			 (char *) 0, (char *) 0);

		dsp_screen ("Paragraph Print", comm_rec.co_no, comm_rec.co_name);
	}
	Dsp_saverec ("                                      P A R A G R A P H    D E S C R I P T I O N .                                         ");	
	Dsp_saverec ("");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END]");

	strcpy (qtlh_rec.co_no, comm_rec.co_no);
	strcpy (qtlh_rec.par_code, local_rec.startParagraph);
	cc = find_rec (qtlh, &qtlh_rec, GTEQ, "r");
	while (!cc && !strcmp (qtlh_rec.co_no, comm_rec.co_no)
		&& strncmp (qtlh_rec.par_code, local_rec.endParagraph, 12) <= 0)
	{
		strcpy (last_para, qtlh_rec.par_code);

		sprintf (err_str, "%s", ML ("Paragraph : "));

		sprintf (disp_str,"^1%s%10.10s - (%40.40s) %52.52s^6", err_str, qtlh_rec.par_code, qtlh_rec.par_desc, " "); 
		Dsp_saverec (disp_str);

		if (!DISPLAY_IT)
			dsp_process ("Paragraph : ", qtlh_rec.par_code);
	
		srec_lcount = 1;
		qtld_rec.hhlh_hash 	= qtlh_rec.hhlh_hash;
		qtld_rec.line_no 	= 0;
		cc = find_rec (qtld, &qtld_rec, GTEQ, "r");
		while (!cc && qtld_rec.hhlh_hash == qtlh_rec.hhlh_hash)
		{
			srec_lcount++;
			if (srec_lcount > P_PAGELEN)
			{
				sprintf (err_str,  "%s", ML ("Paragraph : "));
				sprintf (err_str1, "%s", ML ("Continued"));

				sprintf (disp_str,"^1%s%10.10s - (%40.40s) %s...........%32.32s^6", err_str, qtlh_rec.par_code, qtlh_rec.par_desc, err_str1, " ");
				Dsp_saverec (disp_str);
	
				srec_lcount = 2;
			}
			sprintf (disp_str,"%-120.120s", qtld_rec.desc);
			Dsp_saverec (disp_str);
	    		cc = find_rec (qtld, &qtld_rec, NEXT, "r");
		}
		if (DISPLAY_IT)
		{
			if (srec_lcount + 2 < P_PAGELEN)
			{
				sprintf (err_str, "%s", ML ("END OF PARAGRAPH DETAILS FOR"));

				sprintf (disp_str,"^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^^1 %s %-10.10s ^6^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",err_str,last_para);
				Dsp_saverec (disp_str);
				srec_lcount++;
			}

			for (i = srec_lcount; i < P_PAGELEN; i++)
				Dsp_saverec (" ");
		}
		else
		{
			sprintf (disp_str,"                                                                                                                                   ");
			Dsp_saverec (disp_str);
		}

		cc = find_rec (qtlh, &qtlh_rec, NEXT, "r");
	}
	if (DISPLAY_IT)
		Dsp_srch ();
	else
	{
		snorm ();
		Dsp_print ();
		swide ();
	}

	Dsp_close ();

	if (DISPLAY_IT)
		rset_tty ();
}

void
SetOptions (void)
{
	strcpy (local_rec.back, 	(BACK) ? "Y" : "N");
	strcpy (local_rec.backDesc,	(BACK) ? ML ("Yes ") : ML ("No "));
	strcpy (local_rec.onite,	(ONITE) ? "Y" : "N");
	strcpy (local_rec.oniteDesc,(ONITE) ? ML ("Yes ") : ML ("No "));
	DSP_FLD ("back");
	DSP_FLD ("backDesc");
	DSP_FLD ("onite");
	DSP_FLD ("oniteDesc");
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
		
		rv_pr (ML (mlQtMess047),48,0,1);
		
		box (0,2,130,6);
		line_at (1,1,130);
		line_at (4,1,129);

		line_at (20,0,130);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		line_at (22,0,130);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
