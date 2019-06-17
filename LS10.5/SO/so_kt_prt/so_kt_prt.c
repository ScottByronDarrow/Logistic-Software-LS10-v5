/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_kt_prt.c,v 5.5 2002/07/17 09:58:09 scott Exp $
|  Program Name  : (so_kt_prt.c    )                                  |
|  Program Desc  : (Sales Order Kitting Print.                  )     |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written : 25/02/92          |
|---------------------------------------------------------------------|
| $Log: so_kt_prt.c,v $
| Revision 5.5  2002/07/17 09:58:09  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/07/05 06:24:46  kaarlo
| S/C 4033. Include delay in displaying error in Item Number field.
|
| Revision 5.3  2001/08/09 09:21:28  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:51:27  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:20:01  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_kt_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_kt_prt/so_kt_prt.c,v 5.5 2002/07/17 09:58:09 scott Exp $";

#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<ring_menu.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>
#include 	<Costing.h>

#define	PHANTOM	 (inmr_rec.inmr_class [0] == 'P')

char *UNDERLINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;
struct inexRecord	inex_rec;
struct soktRecord	sokt_rec;
struct sokdRecord	sokd_rec;

char	*inmr2	=	"inmr2";

char	item_prmpt [15];
int		prt_report;
int  	first_time = TRUE;
int		close_ok;

int  ProcessCompany 		(void);
int  ProcessPSDetails 		(void);
int  ProcessInvoiceDetails 	(void);

#ifndef GVISION
menu_type	dsp_menu [] = {
    {"                                  ", " ",/* Padding from left of screen */
	_no_option, "", 0, SHOW },
    {" <Components> ",            " Display Components . [C]",
	ProcessCompany,   "Cc", 0, VALID },
    {" <Packing Slip Details> ",  " Display Packing Slip Details. [P]",
	ProcessPSDetails ,   "Pp", 0, VALID },
    {" <Invoice> ",  		" Display Invoice Details. [I]",
	ProcessInvoiceDetails ,   "Ii", 0, VALID },
    {" <FN16> ",        " Exit From Menu.  [FN16]",
	_no_option, "", FN16, ALL },
    {"",}
};
#else
menu_type	dsp_menu [] = {
    {0, " <Components> ",            "Components",
	ProcessCompany, 0, VALID },
    {0, " <Packing Slip Details> ",  "Packing Slip Details",
	ProcessPSDetails , 0, VALID },
    {0, " <Invoice> ",  		"Invoice Details",
	ProcessInvoiceDetails , 0, VALID },
    {0, "", }
};
#endif

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];

	char	item_no [17];
	char	item_desc [41];
	char	type [8];
	char	end_item [17];
	char	end_desc [41];
	char	end_type [8];

	char	text_line [61];

	int	lpno;
	char	back [4];
	char	onight [4];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "item_no",	 3, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", item_prmpt, " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "item_desc",	 3, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{1, LIN, "item_type",	 3, 115, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "Type : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.type},

	{1, LIN, "end_item",	 4, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "End Item    : ", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_item},
	{1, LIN, "end_desc",	 4, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description : ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.end_desc},
	{1, LIN, "end_type",	 4, 115, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "Type : ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.end_type},

	{1, LIN, "lpno",	 6, 15, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No : ", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	 7, 15, CHARTYPE,
		"U", "          ",
		" ", "N", "Background : ", " ",
		 ND, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	 8, 15, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight : ", " ",
		 ND, NO,  JUSTLEFT, "YN", "", local_rec.onight},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	ClearMenu 			(void);
void 	CloseDB 			(void);
void 	HeadingOutput 		(void);
void 	ItemHeader 			(void);
void 	OpenDB 				(void);
void 	PrintInex 			(void);
void 	PrintKits 			(void);
void 	ReadMisc 			(void);
void 	RunProgram 			(char *, char *);
double 	GetCost 			(void);
int  	spec_valid 			(int);
int  	heading 			(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	prt_report = FALSE;
	strcpy (item_prmpt, "Item Number : ");
	if (!strncmp (sptr, "so_kt_prt", 9))
	{
		if (argc != 2 && argc != 4)
		{
			print_at (0,0, mlSoMess704, argv [0]);
			print_at (1,0, mlSoMess762, argv [0]);
			return (EXIT_FAILURE);
		}

		prt_report = TRUE;
		strcpy (item_prmpt, "Start Item  : ");
		FLD ("end_item") = YES;
		FLD ("end_desc") = NA;
		FLD ("end_type") = NA;
		FLD ("lpno") = YES;
		FLD ("back") = YES;
		FLD ("onight") = YES;
	}
	
	OpenDB ();

	if (prt_report && argc == 4)
	{
		local_rec.lpno = atoi (argv [1]);
		sprintf (local_rec.item_no, "%-16.16s", argv [2]);
		sprintf (local_rec.end_item, "%-16.16s", argv [3]);

		first_time = TRUE;
		PrintKits ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	tab_row = 9;
	tab_col = 8;
	init_scr ();
	set_tty ();
	set_masks ();

	input_row = 19;

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_ok		= TRUE;
		lcount [2] = 0;
		init_vars (1);
		init_vars (2);

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		if (prt_report)
		{
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;

			RunProgram (argv [0], argv [1]);
			shutdown_prog ();
            return (EXIT_SUCCESS);
		}

		heading (1);
		scn_display (1);

		first_time = TRUE;
		ProcessCompany ();
		first_time = FALSE;
#ifndef GVISION
		run_menu (dsp_menu, "", input_row);
#else
        run_menu (NULL, dsp_menu);
#endif
		if (close_ok)
		{
			Dsp_close ();
			close_ok = FALSE;
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");
	ReadMisc ();

	abc_alias ("inmr2", inmr);

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_id_no");
	open_rec (sokd, sokd_list, SOKD_NO_FIELDS, "sokd_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (sokt);
	abc_fclose (sokd);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (incc);
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	abc_fclose (ccmr);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("item_no"))
	{
		if (prt_report && dflt_used)
		{
			strcpy (local_rec.item_no, "                ");
			sprintf (local_rec.item_desc, "%-40.40s", ML ("First Item"));
			strcpy (local_rec.type, "       ");
			DSP_FLD ("item_no");
			DSP_FLD ("item_desc");
			DSP_FLD ("item_type");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (sokt_rec.co_no,comm_rec.co_no);
		sokt_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sokt_rec.line_no = 0;
		cc = find_rec (sokt, &sokt_rec, GTEQ, "r");	
		if (cc || 
		    strcmp (sokt_rec.co_no,comm_rec.co_no) || 
		    sokt_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			print_mess (ML (mlStdMess160));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (inmr_rec.inmr_class [0] == 'P')
			strcpy (local_rec.type, ML ("Phantom"));
		else
			strcpy (local_rec.type, ML ("Kitting"));

		sprintf (local_rec.item_desc,"%-40.40s",inmr_rec.description);

		DSP_FLD ("item_no");
		DSP_FLD ("item_desc");
		DSP_FLD ("item_type");
		return (EXIT_SUCCESS);
	}

        if (LCHECK ("end_item"))
        {
		if (F_HIDE (label ("end_item")))
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.end_item, "~~~~~~~~~~~~~~~~");
			sprintf (local_rec.end_desc, "%-40.40s", ML ("Last Item"));
			strcpy (local_rec.end_type, "       ");
			DSP_FLD ("end_item");
			DSP_FLD ("end_desc");
			DSP_FLD ("end_type");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.end_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.end_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (sokt_rec.co_no,comm_rec.co_no);
		sokt_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sokt_rec.line_no = 0;
		cc = find_rec (sokt, &sokt_rec, GTEQ, "r");	
		if (cc || 
		    strcmp (sokt_rec.co_no,comm_rec.co_no) || 
		    sokt_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			print_mess (ML (mlStdMess153));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.end_item, local_rec.item_no) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (inmr_rec.inmr_class [0] == 'P')
			strcpy (local_rec.end_type, ML ("Phantom"));
		else
			strcpy (local_rec.end_type, ML ("Kitting"));

		sprintf (local_rec.end_desc, "%-40.40s",inmr_rec.description);

		DSP_FLD ("end_item");
		DSP_FLD ("end_desc");
		DSP_FLD ("end_type");
                return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
			strcpy (local_rec.back, ML ("Yes"));
		else
			strcpy (local_rec.back, ML ("No "));

		DSP_FLD ("back");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
			strcpy (local_rec.onight, ML ("Yes"));
		else
			strcpy (local_rec.onight, ML ("No "));

		DSP_FLD ("onight");
		return (EXIT_SUCCESS);
	}

        return (EXIT_SUCCESS);             
}

void
RunProgram (
 char *prog_name, 
 char *prog_desc)
{
	char	lp_str [3];

	sprintf (lp_str, "%2d", local_rec.lpno);
	
	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					lp_str,
					local_rec.item_no,
					local_rec.end_item,
					prog_desc, (char *)0);
		}
	}

	if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				lp_str,
				local_rec.item_no,
				local_rec.end_item, (char *)0);
	}
	else 
	{
		execlp (prog_name,
				prog_name,
				lp_str,
				local_rec.item_no,
				local_rec.end_item, (char *)0);
	}
}

/*--------------
| Print Report |
--------------*/
void
PrintKits (void)
{
	HeadingOutput ();

	dsp_screen (" Printing Kitting Items ", comm_rec.co_no, comm_rec.co_name);

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", local_rec.item_no);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
	       strcmp (inmr_rec.item_no, local_rec.end_item) <= 0)
	{
		if (inmr_rec.inmr_class [0] != 'P' && inmr_rec.inmr_class [0] != 'K')
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		dsp_process ("Item:", inmr_rec.item_no);

		ItemHeader ();
		if (!first_time)
			fprintf (fout, ".PA\n");

		ProcessCompany ();
		if (PHANTOM)
		{
			ProcessPSDetails ();
			ProcessInvoiceDetails ();
		}

		first_time = FALSE;
	
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*---------------------
| Prepare Output Pipe |
---------------------*/
void
HeadingOutput (void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.lpno);

	fprintf (fout,".8\n");
	fprintf (fout,".PI10\n");
	fprintf (fout,".L112\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n", ML (" Detailed Kitting Print "));
	fprintf (fout,".B1\n");
	fprintf (fout,
		".ECOMPANY %s : %s\n",
		comm_rec.co_no,
		clip (comm_rec.co_name));
	fprintf (fout,".B1\n");

	fprintf (fout, ".R====================================");
	fprintf (fout, "======================================");
	fprintf (fout, "======================================\n");
}

/*---------------------
| Define Item Heading |
---------------------*/
void
ItemHeader (void)
{
	fprintf (fout, ".DS5\n");
	fprintf (fout, 
		".CITEM: %-16.16s  DESCRIPTION: %-40.40s  TYPE: %s\n",
		inmr_rec.item_no,
		inmr_rec.description,
		 (PHANTOM) ? ML ("Phantom") : ML ("Kitting"));

	fprintf (fout, ".B1\n");

	fprintf (fout, "=====================================");
	fprintf (fout, "=====================================");
	fprintf (fout, "======================================\n");

	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|             ITEM DESCRIPTION             ");
	fprintf (fout, "| QUANTITY USED  ");
	fprintf (fout, "|  COST EACH   ");
	fprintf (fout, "| EXTENDED COST  |\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "+------------------------------------------");
	fprintf (fout, "+----------------");
	fprintf (fout, "+--------------");
	fprintf (fout, "+----------------|\n");
}

/*--------------------
| Process components |
--------------------*/
int
ProcessCompany (void)
{
	double	matl_cost, xtd_cost, tot_cost;
	int	data_fnd;
	int	no_cost;

	if (!first_time && close_ok && !prt_report)
		Dsp_close ();

	if (!prt_report)
	{
		ClearMenu ();
		close_ok = TRUE;
		Dsp_open (10, 5, 8);
		Dsp_saverec ("                                      K I T T I N G   C O M P O N E N T S                                     ");
		Dsp_saverec ("  ITEM NUMBER     |   ITEM DESCRIPTION                       |  QUANTITY USED |  COST EACH   | EXTENDED COST  ");
		Dsp_saverec (" [REDRAW]  [NEXT]  [PREV]  [EDIT/END] ");
	}

	tot_cost = 0.00;
	data_fnd = FALSE;

	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sokt_rec.line_no = 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
	       sokt_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		cc = find_hash ("inmr2", &inmr2_rec, COMPARISON, "r", sokt_rec.mabr_hash);
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		incc_rec.hhbr_hash = inmr2_rec.hhbr_hash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		no_cost = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (no_cost)
		{
			matl_cost	=	FindIneiCosts
							(
								inmr2_rec.costing_flag,
								comm_rec.est_no,
								inmr2_rec.hhbr_hash
							);
		}
		else
			matl_cost = GetCost ();

		xtd_cost  = matl_cost * sokt_rec.matl_qty;
		tot_cost += xtd_cost;

		sprintf (err_str,
			" %-16.16s %s %-40.40s %s %14.2f %s %12.2f %s %14.2f ",
			inmr2_rec.item_no,     (prt_report) ? "|" : "^E",
			inmr2_rec.description, (prt_report) ? "|" : "^E",
			sokt_rec.matl_qty,     (prt_report) ? "|" : "^E",
	 		matl_cost,                (prt_report) ? "|" : "^E",
			xtd_cost);
	
		if (prt_report)
			fprintf (fout, "|%s|\n", err_str);
		else
			Dsp_saverec (err_str);

			PrintInex ();

		data_fnd = TRUE;

		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}

	if (data_fnd)
	{
		sprintf (err_str,
			" TOTAL COST                                                                                    %14.2f",
			tot_cost);

		if (prt_report)
		{
			fprintf (fout, "|------------------");
			fprintf (fout, "+------------------------------------------");
			fprintf (fout, "+----------------");
			fprintf (fout, "+--------------");
			fprintf (fout, "+----------------|\n");

			fprintf (fout, "|%s |\n", err_str);

			fprintf (fout,"=====================================");
			fprintf (fout,"======================================");
			fprintf (fout,"=====================================\n");
		}
		else
		{
			Dsp_saverec (UNDERLINE);
			Dsp_saverec (err_str);
			Dsp_saverec (UNDERLINE);
		}
	}

	if (!prt_report)
		Dsp_srch ();

    return (EXIT_SUCCESS);
}

/*-----------------------------------
| Process Packing Slip Instructions |
-----------------------------------*/
int 
ProcessPSDetails (void)
{
	int	fst_ps;

	if (close_ok && !prt_report)
	{
		Dsp_close ();
		close_ok = FALSE;
	}
	
	if (prt_report)
		fprintf (fout, ".R \n");
	else
	{
		ClearMenu ();

		close_ok = TRUE;
		Dsp_open (10, 5, 9);
		Dsp_saverec ("                              P A C K I N G   S L I P   I T E M   D E T A I L S                               ");
		Dsp_saverec ("");
		Dsp_saverec (" [REDRAW]  [NEXT]  [PREV]  [EDIT/END] ");
	}

	fst_ps = TRUE;
	strcpy (sokd_rec.co_no, comm_rec.co_no);
	strcpy (sokd_rec.type, "P");
	sokd_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sokd_rec.line_no = 0;
	cc = find_rec (sokd, &sokd_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (sokd_rec.co_no, comm_rec.co_no) &&
	       sokd_rec.type [0] == 'P' &&
	       sokd_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (fst_ps && prt_report)
		{
			fprintf (fout, ".DS2\n");
			fprintf (fout, "PACKING SLIP ITEM DETAILS (cont)\n");
			fprintf (fout, "================================\n");

			fprintf (fout, ".B1\n");
			fprintf (fout, "PACKING SLIP ITEM DETAILS\n");
			fprintf (fout, "=========================\n");
		}

		sprintf (err_str, "%-30.30s %-60.60s ", " ", sokd_rec.text);

		if (prt_report)
			fprintf (fout, "%s\n", err_str);
		else
			Dsp_saverec (err_str);

		fst_ps = FALSE;
		cc = find_rec (sokd, &sokd_rec, NEXT, "r");
	}

	if (!prt_report)
		Dsp_srch ();

    return (EXIT_SUCCESS);
}

/*------------------------------
| Process Invoice Instructions |
------------------------------*/
int
ProcessInvoiceDetails (void)
{
	int	fst_inv;

	if (close_ok && !prt_report)
	{
		Dsp_close ();
		close_ok = FALSE;
	}
	
	if (!prt_report)
	{
		ClearMenu ();
		close_ok = TRUE;
		Dsp_open (10, 5, 9);
		Dsp_saverec ("                                   I N V O I C E   I T E M   D E T A I L S                                    ");
		Dsp_saverec ("");
		Dsp_saverec (" [REDRAW]  [NEXT]  [PREV]  [EDIT/END] ");
	}

	fst_inv = TRUE;
	strcpy (sokd_rec.co_no, comm_rec.co_no);
	strcpy (sokd_rec.type, "I");
	sokd_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sokd_rec.line_no = 0;
	cc = find_rec (sokd, &sokd_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (sokd_rec.co_no, comm_rec.co_no) &&
	       sokd_rec.type [0] == 'I' &&
	       sokd_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (fst_inv && prt_report)
		{
			fprintf (fout, ".DS2\n");
			fprintf (fout, "INVOICE ITEM DETAILS (cont)\n");
			fprintf (fout, "===========================\n");

			fprintf (fout, ".B1\n");
			fprintf (fout, "INVOICE ITEM DETAILS\n");
			fprintf (fout, "====================\n");
		}

		sprintf (err_str, "%-30.30s %-60.60s ", " ", sokd_rec.text);
	
		if (prt_report)
			fprintf (fout, "%s\n", err_str);
		else
			Dsp_saverec (err_str);

		fst_inv = FALSE;
		cc = find_rec (sokd, &sokd_rec, NEXT, "r");
	}

	if (!prt_report)
		Dsp_srch ();

    return (EXIT_SUCCESS);
}

/*------------------
| Clear menu lines |
------------------*/
void
ClearMenu (void)
{
	move (0, input_row);
	cl_line ();
	move (0, input_row + 1);
	cl_line ();
}

/*------------------------
| Calculate cost of item |
------------------------*/
double	
GetCost (void)
{
	double	cost;

	switch (inmr2_rec.costing_flag [0])
	{
	case 'A':
	case 'L':
	case 'P':
	case 'T':
		cost	=	FindIneiCosts
					(
						inmr2_rec.costing_flag,
						comm_rec.est_no,
						inmr2_rec.hhbr_hash
					);
	case 'S':
		cost	=	FindInsfValue (incc_rec.hhwh_hash, TRUE);
		break;

	case 'F':
		cost	=	FindIncfValue 
					(
						incc_rec.hhwh_hash, 
				   		incc_rec.closing_stock, 
				   		sokt_rec.matl_qty, 
						TRUE,
						inmr_rec.dec_pt
					);
		break;

	case 'I':
		cost	=	FindIncfValue	
					(
						incc_rec.hhwh_hash,
						incc_rec.closing_stock,
						sokt_rec.matl_qty, 
						FALSE,
						inmr_rec.dec_pt
					);
		break;
   	 
	default:
		cost	=	FindIneiCosts
					(
						"L",
						comm_rec.est_no,
						inmr2_rec.hhbr_hash
					);
		break;
	}

	if (cost < 0.00)
	{
		cost	=	FindIneiCosts
					(
						"L",
						comm_rec.est_no,
						inmr2_rec.hhbr_hash
					);
	}
	return (cost);
}

int
heading (
 int scn)
{
	int	page_len;

	if (restart) 
		return (EXIT_SUCCESS);

	page_len = 132;

	swide ();
	clear ();
	if (prt_report)
		strcpy (err_str, ML (mlSoMess176));
	else
		strcpy (err_str, ML (mlSoMess177));
	
	rv_pr (err_str, (page_len - strlen (err_str)) / 2, 0, 1);
	
	line_at (1,0,page_len);

	if (prt_report)
	{
		box (0, 2, page_len, 6);
		line_at (5,1,page_len - 2);
	}
	else
		box (0, 2, page_len, 1);
	

	scn_set (scn);
	line_at (21,0,page_len);
	print_at (22,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
PrintInex (void)
{
	inex_rec.hhbr_hash = inmr2_rec.hhbr_hash;
	inex_rec.line_no   = 0;
	cc = find_rec (inex, &inex_rec, GTEQ, "r");
	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr2_rec.hhbr_hash)
	{
		sprintf (err_str,
			" %-16.16s %s %-40.40s %s %14.2s %s %12.2s %s %14.2s ",
			" ",                      (prt_report) ? "|" : "^E",
			inex_rec.desc,         (prt_report) ? "|" : "^E",
			" ",                      (prt_report) ? "|" : "^E",
	 		" ",                      (prt_report) ? "|" : "^E",
			" ");

		if (prt_report)
			fprintf (fout, "|%s|\n", err_str);
		else
			Dsp_saverec (err_str);

		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}
		
