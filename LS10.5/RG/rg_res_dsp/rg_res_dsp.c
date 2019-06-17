/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: rg_res_dsp.c,v 5.5 2002/07/17 09:57:44 scott Exp $
|  Program Name  : (rg_res_dsp.c)
|  Program Desc  : (Resource Detail Display)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written : 11/11/91          |
|---------------------------------------------------------------------|
| $Log: rg_res_dsp.c,v $
| Revision 5.5  2002/07/17 09:57:44  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/07/08 04:20:47  scott
| S/C 00845 - Windows Client
| (1) Default for Start Item Number is space.  After filling-up all fields, press F12, program cannot accept blank.
| (2) Error display has no delay in all fields.
| (3) When focus is at field Output To, and value is 'D', the displayed character is 'v', if value is 'P', the character is 'r'.
| CHAR-BASED / WINDOWS CLIENT
| (3) Accepts invalid ranges
|
| Revision 5.3  2002/06/25 07:12:52  scott
| S/C SC 3978.
| (1) When Resource Type is "Q" (Q/C Check), the character displayed is "k", otherwise, space.
| (2) When Output To is "D" (Display), the character displayed is "v", when "P" (Printer), the character is "r".
|
| Revision 5.2  2001/08/09 09:16:33  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:39:43  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: rg_res_dsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/RG/rg_res_dsp/rg_res_dsp.c,v 5.5 2002/07/17 09:57:44 scott Exp $";

#define	X_OFF		10
#define	Y_OFF		2
#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_rg_mess.h>

#include	<std_decs.h>

int 	heading 		 (int);
void 	exit_prog 		 (void);
void 	shutdownProg 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int 	spec_valid 		 (int);
void 	Process 		 (void);
void 	OpenOutput 		 (void);
void 	ReadRgrs 		 (void);
void 	SrchTypes 		 (char *);
void 	SrchRgrs 		 (char *);

#define	CODE	1
#define	TYPE	2

#define	DISP		 (local_rec.output [0] == 'D')

char	*UNDERLINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

#include	"schema"

struct commRecord	comm_rec;
struct rgrsRecord	rgrs_rec;

	char	*data	= "data";

	int	run_prog;

char	*val_types [] =
{	"Labour   ",
	"Machine  ",
	"Other    ",
	"Q/C Check",
	"Special  ",
	"",
};
int	no_types = 5;

int	restrict;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startCode [17];
	char	startCodeDesc [41];
	char	endCode [17];
	char	endCodeDesc [41];
	char	resourceType [2];
	char	resourceDesc [11];
	char	output [2];
	char	outputDesc [11];
	int	lpno;
} local_rec;

extern	int	TruePosition;

static	struct	var vars [] =
{
	{1, LIN, "startCode",	 4, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Start Code         ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCode},
	{1, LIN, "startCodeDesc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startCodeDesc},
	{1, LIN, "endCode",	 5, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "End Code           ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCode},
	{1, LIN, "endCodeDesc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endCodeDesc},
	{1, LIN, "c_output",	 7, 2, CHARTYPE,
		"U", "          ",
		" ", "D", "Output To          ", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.output},
	{1, LIN, "c_outputDesc",	 7, 35, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.outputDesc},
	{1, LIN, "c_lpno",	8, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No         ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	{2, LIN, "resourceType",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", " ", "Resource Type      ", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.resourceType},
	{2, LIN, "resourceDesc",	 4, 35, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.resourceDesc},
	{2, LIN, "t_output",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "D", "Output To          ", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.output},
	{2, LIN, "t_outputDesc",	 6, 35, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.outputDesc},
	{2, LIN, "t_lpno",	 7, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No         ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char *argv [])
{
	if (argc != 3)
	{
		print_at (0,0,mlRgMess701, argv [0]);
		return EXIT_FAILURE;
	}

	switch (argv [1][0])
	{
	case 'C':
		run_prog = CODE;
		break;

	case 'T':
		run_prog = TYPE;
		break;

	default:
		print_at (0,0,mlRgMess702, argv [0]);
		return EXIT_FAILURE;
		break;
	}

	if (argv [2][0] == 'N')
		restrict = FALSE;
	else
		restrict = TRUE;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		init_vars (run_prog);
		heading (run_prog);
		entry (run_prog);
		if (restart || prog_exit)
			continue;

		heading (run_prog);
		scn_display (run_prog);
		edit (run_prog);

		if (restart)
			continue;

		Process ();
	}

	shutdownProg ();

	return EXIT_SUCCESS;

}

/*========================
| Program exit sequence. |
========================*/
void 
shutdownProg (
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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (rgrs, rgrs_list, RGRS_NO_FIELDS, "rgrs_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB (
 void)
{
	abc_fclose (rgrs);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int	i;

	if (LCHECK ("startCode"))
	{
		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strlen (clip (local_rec.startCode)))
		{
			sprintf (local_rec.startCode, "%-8.8s", " ");
			strcpy (local_rec.startCodeDesc, ML ("Start Range"));

			DSP_FLD ("startCode");
			DSP_FLD ("startCodeDesc");
			return (EXIT_SUCCESS);
		}

		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		sprintf (rgrs_rec.code, "%-8.8s", local_rec.startCode);
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess233));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY && 
		    strcmp (local_rec.startCode,local_rec.endCode) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("startCode");
		sprintf (local_rec.startCodeDesc,"%-40.40s",rgrs_rec.desc);
		DSP_FLD ("startCodeDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endCode"))
	{
		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strlen (clip (local_rec.endCode)))
		{
			strcpy (local_rec.endCode, "~~~~~~~~");
			strcpy (local_rec.endCodeDesc, ML ("End Range"));

			DSP_FLD ("endCode");
			DSP_FLD ("endCodeDesc");
			return (EXIT_SUCCESS);
		}

		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		sprintf (rgrs_rec.code, "%-8.8s", local_rec.endCode);
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess233));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startCode,local_rec.endCode) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("endCode");
		sprintf (local_rec.endCodeDesc,"%-40.40s",rgrs_rec.desc);
		DSP_FLD ("endCodeDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("resourceType"))
	{
		if ((prog_status == ENTRY && last_char == FN16))
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTypes (temp_str);
			return (EXIT_SUCCESS);
		}

		for (i = 0; i < no_types; i++)
		{
			if (local_rec.resourceType [0] == val_types [i][0])
			{
				sprintf (local_rec.resourceDesc, "%-9.9s", val_types [i]);
				DSP_FLD ("resourceDesc");
				return (EXIT_SUCCESS);
			}
		}
		print_mess (ML (mlRgMess008));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (LCHECK ("c_output") || LCHECK ("t_output"))
	{
		if (DISP)
		{
			strcpy (local_rec.outputDesc, ML ("Display"));
			local_rec.lpno = 0;
			if (run_prog == CODE)
			{
				FLD ("c_lpno") = NA;
				DSP_FLD ("c_lpno");
			}
			else
			{
				FLD ("t_lpno") = NA;
				DSP_FLD ("t_lpno");
			}
		}
		else
		{
			strcpy (local_rec.outputDesc, ML ("Printer"));
			if (run_prog == CODE)
				FLD ("c_lpno") = YES;
			else
				FLD ("t_lpno") = YES;
		}

		display_field (field + 1);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("c_lpno") || LCHECK ("t_lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void 
Process (
 void)
{
	OpenOutput ();

	ReadRgrs ();

	if (DISP)
	{
		Dsp_srch ();
		Dsp_close ();
	}
	else
		Dsp_print ();
}

void
OpenOutput (
 void)
{
	if (DISP)
	{
		heading (0);
		Dsp_prn_open (15, 2, 14,
			"                 Display Resource Details                 ",
			comm_rec.co_no, comm_rec.co_name,
			comm_rec.est_no, comm_rec.est_name,
			comm_rec.cc_no, comm_rec.cc_name);
	}
	else
	{
		Dsp_nd_prn_open (15, 0, 15,
			"                 Display Resource Details                 ",
			comm_rec.co_no, comm_rec.co_name,
			comm_rec.est_no, comm_rec.est_name,
			comm_rec.cc_no, comm_rec.cc_name);
	}

	Dsp_saverec ("  CODE  |      CODE DESCRIPTION        | RESOURCE |COST|          |    OVERHEAD RATE    ");
	Dsp_saverec ("        |                              |   TYPE   |TYPE|   RATE   |  FIXED   | VARIABLE ");
	Dsp_saverec (" [REDRAW][PRINT][NEXT][PREV][EDIT/END] ");

}

/*--------------------------------------
| Read rgrs records in specified range |
--------------------------------------*/
void
ReadRgrs (
 void)
{
	int	data_found;
	int	first_time;
	char	data_str [250];
	char	data_str1 [250];
	char	tmp_rate [11];
	char	tmp_var [11];
	char	tmp_fix [11];

	data_found = FALSE;
	first_time = TRUE;

	if (!DISP)
	{
		dsp_screen ("Printing Instructions Details",
			comm_rec.co_no,
			comm_rec.co_name);
	}

	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	if (run_prog == CODE)
		sprintf (rgrs_rec.code, "%-8.8s", local_rec.startCode);
	else
		sprintf (rgrs_rec.code, "%-8.8s", " ");

	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (rgrs_rec.co_no, comm_rec.co_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.est_no))
	{
		if (run_prog == CODE)
		{
			if (strcmp (rgrs_rec.code, local_rec.endCode) > 0)
				break;
		}
		else
		{
			if (rgrs_rec.type [0] != local_rec.resourceType [0])
			{
				cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
				continue;
			}
		}

		if (!DISP)
			dsp_process ("Code", rgrs_rec.code);

		data_found = TRUE;
		first_time = FALSE;

		sprintf (tmp_var, "%10.3f", DOLLARS (rgrs_rec.ovhd_var));
		sprintf (tmp_fix, "%10.3f", DOLLARS (rgrs_rec.ovhd_fix));
		sprintf (tmp_rate,"%10.3f", DOLLARS (rgrs_rec.rate));

		switch (rgrs_rec.type [0])
		{
		case 'O':
			if (!strcmp (rgrs_rec.cost_type, "DVC") ||
			    !strcmp (rgrs_rec.cost_type, "IVC"))
			{
				sprintf (tmp_fix,"%-10.10s", " ");
			}

			if (!strcmp (rgrs_rec.cost_type, "DFC") ||
			    !strcmp (rgrs_rec.cost_type, "IFC"))
			{
				sprintf (tmp_var,"%-10.10s", " ");
			}

			sprintf (data_str,
				"%-8.8s^E%-30.30s^E%-10.10s^E%-3.3s ",
				rgrs_rec.code,
				rgrs_rec.desc,
				rgrs_rec.type_name,
				rgrs_rec.cost_type);

			if (restrict)
			{
				sprintf (data_str1,
					"^E%-14.14s^E%-14.14s^E%-14.14s",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ");
			}
			else
			{
				sprintf (data_str1,
					"^E%-10.10s^E%-10.10s^E%-10.10s",
					tmp_rate,
					tmp_fix,
					tmp_var);
			}
			strcat (data_str, data_str1);

			break;

		case 'Q':
			sprintf (data_str,
				"%-8.8s^E%-30.30s^E%-10.10s^E%-3.3s ",
				rgrs_rec.code,
				rgrs_rec.desc,
				rgrs_rec.type_name,
				"N/A");

			if (restrict)
			{
				sprintf (data_str1,
					"^E%-14.14s^E%-14.14s^E%-14.14s",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ");
			}
			else
			{
				sprintf (data_str1,
					"^E%-10.10s^E%-10.10s^E%-10.10s",
					tmp_rate,
					tmp_fix,
					tmp_var);
			}
			strcat (data_str, data_str1);

			break;

		case 'S':
			sprintf (data_str,
				"%-8.8s^E%-30.30s^E%-10.10s^E%-3.3s ",
				rgrs_rec.code,
				rgrs_rec.desc,
				rgrs_rec.type_name,
				"N/A");

			if (restrict)
			{
				sprintf (data_str1,
					"^E%-14.14s^E%-14.14s^E%-14.14s",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ");
			}
			else
			{
				sprintf (data_str1,
					"^E%-10.10s^E%-10.10s^E%-10.10s",
					tmp_rate,
					tmp_fix,
					tmp_var);
			}
			strcat (data_str, data_str1);

			break;

		case 'L':
			sprintf (data_str,
				"%-8.8s^E%-30.30s^E%-10.10s^E%-3.3s ",
				rgrs_rec.code,
				rgrs_rec.desc,
				rgrs_rec.type_name,
				"N/A");

			if (restrict)
			{
				sprintf (data_str1,
					"^E%-14.14s^E%-14.14s^E%-14.14s",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ");
			}
			else
			{
				sprintf (data_str1,
					"^E%-10.10s^E%-10.10s^E%-10.10s",
					tmp_rate,
					tmp_fix,
					tmp_var);
			}
			strcat (data_str, data_str1);

			break;

		case 'M':
			sprintf (data_str,
				"%-8.8s^E%-30.30s^E%-10.10s^E%-3.3s ",
				rgrs_rec.code,
				rgrs_rec.desc,
				rgrs_rec.type_name,
				"N/A");

			if (restrict)
			{
				sprintf (data_str1,
					"^E%-14.14s^E%-14.14s^E%-14.14s",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ",
					" ^^NNNNNNNN^^ ");
			}
			else
			{
				sprintf (data_str1,
					"^E%-10.10s^E%-10.10s^E%-10.10s",
					tmp_rate,
					tmp_fix,
					tmp_var);
			}
			strcat (data_str, data_str1);

			break;

		default:
			sprintf (data_str, "%-88.88s", " ");
			break;

		}

		Dsp_saverec (data_str);

		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}

	if (data_found)
		Dsp_saverec (UNDERLINE);
}

void
SrchTypes (
 char *key_val)
{
	char	type_letter [2];
	int	i;

	_work_open (2,0,40);
	save_rec ("#T", "#Resource Type Description");

	for (i = 0; i < no_types; i++)
	{
		sprintf (type_letter, "%-1.1s", val_types [i]);
		cc = save_rec (type_letter, val_types [i]);
		if (cc)
			break;
	}

	cc = disp_srch ();
	work_close ();
}

void
SrchRgrs (
 char *key_val)
{
	_work_open (8,0,40);
	save_rec ("#Type", "#Description");

	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", key_val);
	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while (!cc && !strcmp (rgrs_rec.co_no, comm_rec.co_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.est_no) &&
		!strncmp (rgrs_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (rgrs_rec.code, rgrs_rec.desc);
		if (cc)
			break;

		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", key_val);
	cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rgrs, "DBFIND");
}

int
heading (
 int scn)
{
	if (restart)
		return 0;

	swide ();
	clear ();

	if (restrict)
		sprintf (err_str, ML (mlRgMess006));
	else
		sprintf (err_str, ML (mlRgMess005));

	if (scn != 0)
	{
		switch (run_prog)
		{
		case	CODE:
			box (0, 3, 132, 5);
			line_at (6,1,131);

			break;

		case	TYPE:
			box (0, 3, 132, 4);
			line_at (5,1,131);

			break;

		default:
			break;
		}
	}
	rv_pr (err_str, (132 - strlen (err_str)) / 2, 0, 1);

	line_at (1,0,132);
	line_at (21,0,132);

	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	rv_pr (err_str, 0, 22, 0);

	if (scn == 0)
		return (EXIT_SUCCESS);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	return 0;

}
