/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: rg_print.c,v 5.7 2002/07/17 09:57:44 scott Exp $
|  Program Name  : (rg_print.c )                                    |
|  Program Desc  : (Routing Print.                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (23/01/92)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: rg_print.c,v $
| Revision 5.7  2002/07/17 09:57:44  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.6  2002/07/16 08:10:11  scott
| S/C 004124 - Moved search.
|
| Revision 5.5  2002/07/08 06:50:54  scott
| S/C 004073 - Updated for small validation changes
|
| Revision 5.4  2002/03/07 07:51:28  scott
| ..
|
| Revision 5.3  2002/03/07 07:27:33  scott
| S/C 830
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: rg_print.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/RG/rg_print/rg_print.c,v 5.7 2002/07/17 09:57:44 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_rg_mess.h>

FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct pcidRecord	pcid_rec;
struct pcwcRecord	pcwc_rec;
struct rgbpRecord	rgbp_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct rgrsRecord	rgrs_rec;

	char	*data	= "data",
			*inmr2	= "inmr2";

int	first_time;

struct
{
	int		seq_no;
	long	hhwc_hash;
	double	rate;
	double	ovhd_var;
	double	ovhd_fix;
	long	time;
	int		qty_rsrc;
	char	type [2];
	int	instr_no;
} store [MAXLINES];

int	no_stored;

/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy 		[11];
	char	systemDate 	[11];
	long	lsystemDate;
	char	startItem 	[17];
	char	startItemDesc [41];
	char	endItem 	[17];
	char	endItemDesc [41];
	int		startAltNo;
	int		endAltNo;
	int		printerNo;
	char	back 		[2];
	char	backDesc 	[11];
	char	onight 		[2];
	char	onightDesc	[11];
	char	lp_str 		[3];
	char	typeDesc 	[9];
	char	workCenter 	[9];
	double	totalCost;
	long	totalTime;
} local_rec;

extern	int		TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "startItem",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start Item       ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "startItemDesc",	 4, 37, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startItemDesc},
	{1, LIN, "endItem",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "End Item         ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "endItemDesc",	 5, 37, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endItemDesc},

	{1, LIN, "startAltNo",	 7, 2, INTTYPE,
		"NNNNN", "          ",
		" ", "1", "Start Alternate  ", "",
		 NO, NO, JUSTRIGHT, "1", "32767", (char *) &local_rec.startAltNo},
	{1, LIN, "endAltNo",	 8, 2, INTTYPE,
		"NNNNN", "          ",
		" ", "1", "End Alternate    ", "",
		 NO, NO, JUSTRIGHT, "1", "32767", (char *) &local_rec.endAltNo},
	{1, LIN, "printerNo",		10, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No       ", "",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",		11, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background       ", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",		11, 25, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	12, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight        ", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",		12, 25, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.onightDesc},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		"", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void    ProcessLines		(void);
void    CalcValues 			(void);
int 	RunProgram 			(char *, char *);
int 	heading 			(int);
int 	spec_valid 			(int);
int 	Process 			(void);
int 	HeadingOutput 		(void);
int 	ItemHead 			(void);
int 	ProcessByProduct 	(void);
int 	ProcessInstruction 	(int);
int 	GetInstVersion 		(int);

/*
 * Main Processing Routine. 
 */
int
main (
 int    argc,
 char*  argv [])
{
	TruePosition	=	TRUE;
	if (argc != 2 && argc != 6)
	{
		print_at (0,0, mlStdMess244, argv [0]);
		print_at (1,0, mlRgMess700, argv [0]);
		return (argc);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	OpenDB ();

	if (argc == 2)
	{
		/*----------------------------
		| Setup required parameters. |
		----------------------------*/
		init_scr ();
		set_tty ();
		set_masks ();
		init_vars (1);

		/*
		 * Beginning of input control loop . 
		 */
		while (prog_exit == 0)
		{
			/*-----------------------
			| Reset control flags . |
			-----------------------*/
			entry_exit	= FALSE;
			edit_exit	= FALSE;
			prog_exit	= FALSE;
			restart		= FALSE;
			init_ok		= TRUE;
			search_ok	= TRUE;

			/*-------------------------------
			| Enter screen 1 linear input . |
			-------------------------------*/
			heading (1);
			entry (1);
			if (prog_exit)
            {
				shutdown_prog ();
                return (EXIT_SUCCESS);
            }

			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;

			if (RunProgram (argv [0], argv [1]))
				return (EXIT_SUCCESS);
		}
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	sprintf (local_rec.startItem, "%-16.16s", argv [1]);
	sprintf (local_rec.endItem, "%-16.16s", argv [2]);
	local_rec.startAltNo = atoi (argv [3]);
	local_rec.endAltNo = atoi (argv [4]);
	local_rec.printerNo = atoi (argv [5]);

	Process ();
	fprintf (fout, ".EOF\n");
	pclose (fout);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2, inmr);
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcid,  pcid_list, PCID_NO_FIELDS, "pcid_id_no");
	open_rec (pcwc,  pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (rgbp,  rgbp_list, RGBP_NO_FIELDS, "rgbp_id_no");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (pcid);
	abc_fclose (pcwc);
	abc_fclose (rgbp);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (rgrs);
	SearchFindClose ();
	abc_dbclose (data);
}

int
RunProgram (
	char	*progName,
	char	*progDesc)
{
	CloseDB (); 
	FinishProgram ();
	rset_tty ();

	if (local_rec.onight [0] == 'Y')
	{
		sprintf 
		(
			err_str,
			"ONIGHT \"%s\" \"%s\" \"%s\" \"%d\" \"%d\" \"%d\" \"%s\"",
			progName,
			local_rec.startItem,
			local_rec.endItem,
			local_rec.startAltNo,
			local_rec.endAltNo,
			local_rec.printerNo,
			progDesc
		);
		cc = SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf 
		(
			err_str,
			"\"%s\" \"%s\" \"%s\" \"%d\" \"%d\" \"%d\"",
			progName,
			local_rec.startItem,
			local_rec.endItem,
			local_rec.startAltNo,
			local_rec.endAltNo,
			local_rec.printerNo
		);
		cc = SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
	return (cc);
}

int
heading (
 int    scn)
{
	if (!restart)
	{
		scn_set (1);
		clear ();
		rv_pr (ML (mlRgMess002), 29, 0, 1);


		box (0, 3, 80, 9);
		line_at (1, 0, 80);
		line_at (6, 1, 79);
		line_at (9, 1, 79);
		line_at (20, 0, 80);

		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("startItem"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (dflt_used || !strlen (clip (local_rec.startItem)))
		{
			strcpy (local_rec.startItem, "                ");
			strcpy (local_rec.startItemDesc, ML ("First Item"));
			DSP_FLD ("startItem");
			DSP_FLD ("startItemDesc");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.startItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.startItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.startItem, inmr_rec.item_no);
		strcpy (local_rec.startItemDesc, inmr_rec.description);
		DSP_FLD ("startItem");
		DSP_FLD ("startItemDesc");

		if (prog_status != ENTRY && 
				strcmp (local_rec.startItem,local_rec.endItem) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endItem"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (dflt_used || !strlen (clip (local_rec.endItem)))
		{
			strcpy (local_rec.endItem, "~~~~~~~~~~~~~~~~");
			strcpy (local_rec.endItemDesc, ML ("Last Item"));
			DSP_FLD ("endItem");
			DSP_FLD ("endItemDesc");
			return (EXIT_SUCCESS);
		}


		cc = FindInmr (comm_rec.co_no, local_rec.endItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.endItem, inmr_rec.item_no);
		sprintf (local_rec.endItemDesc,"%-40.40s", inmr_rec.description);
		DSP_FLD ("endItem");
		DSP_FLD ("endItemDesc");

		if (strcmp (local_rec.startItem,local_rec.endItem) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("startAltNo"))
	{
		if (prog_status != ENTRY && local_rec.startAltNo > local_rec.endAltNo)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endAltNo"))
	{
		if (local_rec.startAltNo > local_rec.endAltNo)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}
		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
			strcpy (local_rec.backDesc, ML ("(Yes)"));
		else
			strcpy (local_rec.backDesc, ML ("(No )"));

		DSP_FLD ("back");
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
			strcpy (local_rec.onightDesc, ML ("(Yes)"));
		else
			strcpy (local_rec.onightDesc, ML ("(No )"));

		DSP_FLD ("onight");
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
Process (void)
{
	dsp_screen ("Processing Routings", comm_rec.co_no, comm_rec.co_name);

	HeadingOutput ();
	first_time = TRUE;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", local_rec.startItem);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
		strcmp (inmr_rec.item_no, local_rec.endItem) <= 0)
	{
		strcpy (rghr_rec.co_no, comm_rec.co_no);
		strcpy (rghr_rec.br_no, comm_rec.est_no);
		rghr_rec.hhbr_hash = inmr_rec.hhbr_hash;
		rghr_rec.alt_no = local_rec.startAltNo;
		cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
		while
		 (
			!cc &&
			!strcmp (rghr_rec.co_no, comm_rec.co_no) &&
			!strcmp (rghr_rec.br_no, comm_rec.est_no) &&
			rghr_rec.hhbr_hash == inmr_rec.hhbr_hash &&
			rghr_rec.alt_no <= local_rec.endAltNo
		)
		{
			dsp_process ("Item : ", inmr_rec.item_no);

			ItemHead ();
			if (!first_time)
				fprintf (fout, ".PA\n");

			fprintf (fout, ".R==============================");
			fprintf (fout, "================================");
			fprintf (fout, "================================");
			fprintf (fout, "================================");
			fprintf (fout, "===============================\n");

			ProcessLines ();
			cc = ProcessByProduct ();
			ProcessInstruction (cc);
			first_time = FALSE;
			cc = find_rec (rghr, &rghr_rec, NEXT, "r");
		}

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

int
HeadingOutput (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);

	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EDETAILED ROUTING REPORT\n");
	fprintf (fout, ".ECOMPANY %s : %s\n",
		comm_rec.co_no, clip (comm_rec.co_name));

	fprintf (fout, ".EAS AT %-24.24s\n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, ".CStart Item : %s ", local_rec.startItem);
	fprintf (fout, "   End Item : %s\n", local_rec.endItem);

	fprintf (fout, ".CStart Alternate : %3d ", local_rec.startAltNo);
	fprintf (fout, "   End Alternate : %3d\n", local_rec.endAltNo);

	fprintf (fout, ".B1\n");

	return (EXIT_SUCCESS);
}

int
ItemHead (void)
{
	char	*ttoa (long int, char *);

	CalcValues ();

	fprintf (fout, ".DS5\n");
	fprintf (fout,
		"     Product Code: %-16.16s  %-35.35s  Alternate No : %d\n",
		inmr_rec.item_no,
		inmr_rec.description,
		rghr_rec.alt_no);
	fprintf (fout,
		"     Elapsed Time: %s          Running Costs :  %9.2f",
		ttoa (local_rec.totalTime, "NNNNNN:NN"),
		DOLLARS (local_rec.totalCost));
	fprintf (fout,
		"                                   Print %s on Picking List\n",
		 (rghr_rec.print_all [0] == 'N') ? "Next sequence" : "All sequences");

	fprintf (fout, "=====");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "=========================================");
	fprintf (fout, "===========");
	fprintf (fout, "====");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====\n");

	fprintf (fout, "|SEQ|");
	fprintf (fout, "WRK CNTR|");
	fprintf (fout, "RESOURCE|");
	fprintf (fout, "          RESOURCE DESCRIPTION          |");
	fprintf (fout, "   TYPE   |");
	fprintf (fout, "OPS|");
	fprintf (fout, " SETUP |");
	fprintf (fout, "  RUN  |");
	fprintf (fout, " CLEAN |");
	fprintf (fout, "RATE/HOUR|");
	fprintf (fout, "VAR. OVHD|");
	fprintf (fout, "FIX. OVHD|");
	fprintf (fout, "   COST  |");
	fprintf (fout, "SPLT|");
	fprintf (fout, "YLD.|");
	fprintf (fout, "INST|\n");

	fprintf (fout, "|---+");
	fprintf (fout, "--------+");
	fprintf (fout, "--------+");
	fprintf (fout, "----------------------------------------+");
	fprintf (fout, "----------+");
	fprintf (fout, "---+");
	fprintf (fout, "-------+");
	fprintf (fout, "-------+");
	fprintf (fout, "-------+");
	fprintf (fout, "---------+");
	fprintf (fout, "---------+");
	fprintf (fout, "---------+");
	fprintf (fout, "---------+");
	fprintf (fout, "----+");
	fprintf (fout, "----+");
	fprintf (fout, "----|\n");

	fflush (fout);

	return (EXIT_SUCCESS);
}

void
ProcessLines (void)
{
	char	*ttoa (long int, char *);
	double	line_cost;
	long	tmp_time;

	rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
	rgln_rec.seq_no = 0;
	cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
	while (!cc && rghr_rec.hhgr_hash == rgln_rec.hhgr_hash)
	{
		cc = find_hash (pcwc, &pcwc_rec, EQUAL, "r", rgln_rec.hhwc_hash);
		if (cc)
			strcpy (pcwc_rec.work_cntr, "NOT FND.");
		strcpy (local_rec.workCenter, pcwc_rec.work_cntr);
		cc = find_hash (rgrs, &rgrs_rec, EQUAL, "r", rgln_rec.hhrs_hash);
		if (cc)
		{
			strcpy (rgrs_rec.code, "NOT FND.");
			strcpy (rgrs_rec.desc, "NOT FND.");
			strcpy (rgrs_rec.type, " ");
		}

		switch (rgrs_rec.type [0])
		{
		case 'L':
			strcpy (local_rec.typeDesc, ML ("Labour  "));
			break;

		case 'M':
			strcpy (local_rec.typeDesc, ML ("Machine "));
			break;

		case 'Q':
			strcpy (local_rec.typeDesc, ML ("QC Check"));
			break;

		case 'S':
			strcpy (local_rec.typeDesc, ML ("Special "));
			break;

		case 'O':
			strcpy (local_rec.typeDesc, ML ("Other   "));
			break;

		default:
			strcpy (local_rec.typeDesc, " ");
			break;
		}

		tmp_time = rgln_rec.setup + rgln_rec.run + rgln_rec.clean;
		line_cost = (double) tmp_time * (rgln_rec.rate + rgln_rec.ovhd_var);
		line_cost /= 60.00;
		line_cost += rgln_rec.ovhd_fix;
		line_cost *= (double) rgln_rec.qty_rsrc;

		fprintf (fout, ".LRP2\n");

		fprintf (fout, "|%3d|",		rgln_rec.seq_no);
		fprintf (fout, "%-8.8s|",	pcwc_rec.work_cntr);
		fprintf (fout, "%-8.8s|",	rgrs_rec.code);
		fprintf (fout, "%-40.40s|",	rgrs_rec.desc);
		fprintf (fout, " %-8.8s |",	local_rec.typeDesc);
		fprintf (fout, "%3d|",		rgln_rec.qty_rsrc);
		fprintf (fout, "%s|",		ttoa (rgln_rec.setup, "NNNN:NN"));
		fprintf (fout, "%s|",		ttoa (rgln_rec.run, "NNNN:NN"));
		fprintf (fout, "%s|",		ttoa (rgln_rec.clean, "NNNN:NN"));
		fprintf (fout, "%9.2f|",	DOLLARS (rgln_rec.rate));
		fprintf (fout, "%9.2f|",	DOLLARS (rgln_rec.ovhd_var));
		fprintf (fout, "%9.2f|",	DOLLARS (rgln_rec.ovhd_fix));
		fprintf (fout, "%9.2f|",	DOLLARS (line_cost));
		if (rgln_rec.can_split [0] == 'Y')
			fprintf (fout, "Yes |");
		else
			if (rgln_rec.can_split [0] == 'S')
				fprintf (fout, "Same|");
			else
				fprintf (fout, "No  |");
		fprintf (fout, "%-4.4s|",	rgln_rec.yld_clc);
		fprintf (fout, " %2d |\n",	rgln_rec.instr_no);

		cc = find_rec (rgln, &rgln_rec, NEXT, "r");
	}
	fprintf (fout, "=====================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "====================================================\n");

	scn_set (1);
}

void
CalcValues (void)
{
	double	cur_cost;
	long	cur_time,
			tmp_time = 0L;
	int	i;

	no_stored = 0;
	rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
	rgln_rec.seq_no = 0;
	cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
	while (!cc && rghr_rec.hhgr_hash == rgln_rec.hhgr_hash)
	{
		store [no_stored].seq_no		= rgln_rec.seq_no;
		store [no_stored].hhwc_hash	= rgln_rec.hhwc_hash;
		store [no_stored].rate		= rgln_rec.rate;
		store [no_stored].ovhd_var	= rgln_rec.ovhd_var;
		store [no_stored].ovhd_fix	= rgln_rec.ovhd_fix;
		store [no_stored].time		= rgln_rec.setup +
						  rgln_rec.run +
						  rgln_rec.clean;
		store [no_stored].qty_rsrc	= rgln_rec.qty_rsrc;
		cc = find_hash (rgrs, &rgrs_rec, EQUAL, "r", rgln_rec.hhrs_hash);
		if (cc)
			strcpy (store [no_stored].type, "*");
		else
			strcpy (store [no_stored].type, rgrs_rec.type);
		store [no_stored++].instr_no = rgln_rec.instr_no;
		cc = find_rec (rgln, &rgln_rec, NEXT, "r");
	}

	local_rec.totalTime = 0L;
	local_rec.totalCost = 0.00;

	for (tmp_time = 0L, i = 0; i < no_stored; i++)
	{
		cur_time = store [i].time;
		if (cur_time > tmp_time)
			tmp_time = cur_time;

		if (i + 1 == no_stored || store [i].seq_no != store [i+1].seq_no)
		{
			local_rec.totalTime += tmp_time;
			tmp_time = 0L;
		}

		cur_cost = (store [i].ovhd_var + store [i].rate) * cur_time;
		cur_cost /= 60.00;
		cur_cost += store [i].ovhd_fix;
		cur_cost *= store [i].qty_rsrc;

		local_rec.totalCost += cur_cost;
	}
}

int
ProcessByProduct (void)
{
	int	i,
		curr_seq,
		new_seq,
		first_byprd;

	fprintf (fout, ".R \n");

	fprintf (fout, ".DS6\n");
	fprintf (fout,
		"     Product Code: %-16.16s  %-35.35s  Alternate No : %d\n",
		inmr_rec.item_no,
		inmr_rec.description,
		rghr_rec.alt_no);
	fprintf (fout,
		"     Elapsed Time: %s          Running Costs :  %9.2f\n",
		ttoa (local_rec.totalTime, "NNNNNN:NN"),
		local_rec.totalCost);
	fprintf (fout, ".B1\n");
	fprintf (fout, "%-32.32sBY PRODUCTS cont..\n", " ");
	fprintf (fout, "%-32.32s==================\n", " ");
	fprintf (fout, "%-22.22sQUANTITY  ITEM NUMBER     DESCRIPTION\n", " ");

	first_byprd = TRUE;
	curr_seq = -1;
	for (i = 0; i < no_stored; i++)
	{
		if (store [i].seq_no == curr_seq)
			continue;

		curr_seq = store [i].seq_no;

		new_seq = TRUE;

		rgbp_rec.hhgr_hash	= rghr_rec.hhgr_hash;
		rgbp_rec.seq_no	= store [i].seq_no;
		rgbp_rec.hhbr_hash	= 0L;
		cc = find_rec (rgbp, &rgbp_rec, GTEQ, "r");
		while
		 (
			!cc &&
			rgbp_rec.hhgr_hash	== rghr_rec.hhgr_hash &&
			rgbp_rec.seq_no	== store [i].seq_no
		)
		{
			if (first_byprd)
			{
				fprintf (fout, ".LRP7\n");
				fprintf (fout, ".B1\n");
				fprintf (fout, "%-32.32sBY PRODUCTS\n", " ");
				fprintf (fout, "%-32.32s===========\n", " ");
				fprintf (fout, "%-22.22sQUANTITY  ITEM NUMBER     DESCRIPTION\n", " ");
			}

			if (new_seq)
			{
				fprintf (fout, ".B1\n");
				fprintf (fout, " SEQ NO %6d", store [i].seq_no);
			}
			else
				fprintf (fout, "%-14.14s", " ");

			cc = find_hash (inmr2, &inmr2_rec, EQUAL, "r", rgbp_rec.hhbr_hash);
			if (cc)
			{
				sprintf (inmr2_rec.item_no, "%-16.16s", "Not On File");
				sprintf (inmr2_rec.description, "%-40.40s", " ");
			}
			fprintf (fout, "  %14.6f", rgbp_rec.qty);
			fprintf (fout, "  %-16.16s", inmr2_rec.item_no);
			fprintf (fout, "  %-40.40s\n", inmr2_rec.description);

			new_seq = FALSE;
			first_byprd = FALSE;
			cc = find_rec (rgbp, &rgbp_rec, NEXT, "r");
		}
	}

	return (first_byprd == FALSE);
}

int
ProcessInstruction (
 int    brk_rqd)
{
	int	i,
		new_seq,
		inst_ver,
		first_instr;

	fprintf (fout, ".R \n");

	if (brk_rqd)
	{
		fprintf (fout, "----------------------------------------");
		fprintf (fout, "----------------------------------------");
		fprintf (fout, "----------------------------------------\n");
	}

	fprintf (fout, ".DS5\n");
	fprintf (fout,
		"     Product Code: %-16.16s  %-35.35s  Alternate No : %d\n",
		inmr_rec.item_no,
		inmr_rec.description,
		rghr_rec.alt_no);
	fprintf (fout,
		"     Elapsed Time: %s          Running Costs :  %9.2f\n",
		ttoa (local_rec.totalTime, "NNNNNN:NN"),
		DOLLARS (local_rec.totalCost));
	fprintf (fout, ".B1\n");
	fprintf (fout, "%-16.16sINSTRUCTIONS cont..\n", " ");
	fprintf (fout, "%-16.16s===================\n", " ");

	first_instr = TRUE;
	for (i = 0; i < no_stored; i++)
	{
		new_seq = TRUE;

		inst_ver = GetInstVersion (i);
		strcpy (pcid_rec.co_no, comm_rec.co_no);
		pcid_rec.hhbr_hash = inmr_rec.hhbr_hash;
		pcid_rec.hhwc_hash = store [i].hhwc_hash;
		pcid_rec.instr_no = store [i].instr_no;
		pcid_rec.version = inst_ver;
		pcid_rec.line_no = 0;
		cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
		while
		 (
			!cc &&
			!strcmp (pcid_rec.co_no, comm_rec.co_no) &&
			pcid_rec.hhbr_hash == inmr_rec.hhbr_hash &&
			pcid_rec.hhwc_hash == store [i].hhwc_hash &&
			pcid_rec.instr_no == store [i].instr_no &&
			pcid_rec.version == inst_ver
		)
		{
			if (first_instr)
			{
				fprintf (fout, ".LRP5\n");
				fprintf (fout, ".B1\n");
				fprintf (fout, "%-16.16sINSTRUCTIONS\n", " ");
				fprintf (fout, "%-16.16s============\n", " ");
			}

			if (new_seq)
			{
				fprintf (fout, ".B1\n");
				fprintf (fout, " SEQ NO %6d", store [i].seq_no);
			}
			else
				fprintf (fout, "%-14.14s", " ");

			fprintf (fout, "  %-60.60s\n", pcid_rec.text);

			new_seq = FALSE;
			first_instr = FALSE;
			cc = find_rec (pcid, &pcid_rec, NEXT, "r");
		}
	}

	return (EXIT_SUCCESS);
}

int
GetInstVersion (
 int    store_line)
{
	int	tmp_ver;

	tmp_ver = 0;

	strcpy (pcid_rec.co_no, comm_rec.co_no);
	pcid_rec.hhbr_hash = inmr_rec.hhbr_hash;
	pcid_rec.hhwc_hash = store [store_line].hhwc_hash;
	pcid_rec.instr_no = store [store_line].instr_no;
	pcid_rec.version = 0;
	pcid_rec.line_no = 0;
	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (pcid_rec.co_no, comm_rec.co_no) &&
		pcid_rec.hhbr_hash == inmr_rec.hhbr_hash &&
		pcid_rec.hhwc_hash == store [store_line].hhwc_hash &&
		pcid_rec.instr_no == store [store_line].instr_no
	)
	{
		tmp_ver = pcid_rec.version;

		cc = find_rec (pcid, &pcid_rec, NEXT, "r");
	}

	return (tmp_ver);
}
