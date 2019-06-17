/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_act_rep.c,v 5.4 2002/07/17 09:57:52 scott Exp $
|  Program Name  : (so_act_rep.c) 
|  Program Desc  : (Active Status Report Selection)
|---------------------------------------------------------------------|
|  Author        : Irfan Gohir     | Date Written  : 05/10/93         |
|---------------------------------------------------------------------|
|  Date Modified : (17/09/97)      | Modified  by  :Roanna Marcelino  |
|---------------------------------------------------------------------|
| $Log: sk_act_rep.c,v $
| Revision 5.4  2002/07/17 09:57:52  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/09 09:17:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:34  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:43  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_act_rep.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_act_rep/sk_act_rep.c,v 5.4 2002/07/17 09:57:52 scott Exp $";

#include <pslscr.h>	
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	SLEEP_TIME	2

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inexRecord	inex_rec;
struct inasRecord	inas_rec;

	FILE	*fout;
	FILE	*fsort;

	char	*data  = "data";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	sysDate [11];
	char	activeStatus [2];
	char	activeDesc [41];
	int		printerNo;
	char 	back [6];
	char 	backDesc [6];
	char	onite [6];
	char	oniteDesc [6];
} local_rec;

struct {
	char	itemNo [17];
	char	itemDesc [41];
	float	stkAvailable;
	char	extraDesc [88];
} data_rec;

static	struct	var	vars [] =
{
	{1, LIN, "act_status",	 4, 15, CHARTYPE,
		"U", "          ",
		" ", " ", "Active Status:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.activeStatus},
	{1, LIN, "act_desc",	 4, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "-", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.activeDesc},
	{1, LIN, "printerNo",	6, 15, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No.  :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	7, 15, CHARTYPE,
		"U", "          ",
		" ", "N", "Background   :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	7, 18, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onite",8, 15, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight    :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc",	8, 18, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.oniteDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/* 
 * Global variables declared here 
 */
int	    nextPage = TRUE;
char	repTitle [61];

/*
 * Function Declarations.
 */
void 	BuildSortFile 		(void);
void 	CloseDB 			(void);
void 	GetExtraDesc 		(void);
void 	HeaderOutput 		(void);
void 	OpenDB 				(void);
void 	PrintDetails 		(void);
void 	RunProgram 			(char *, char *);
void 	ProcessReport 		(void);
void 	SetDefaults 		(void);
void 	SrchInas 			(char *);
void 	StkAvailable 		(void);
void 	shutdown_prog 		(void);
int  	heading 			(int);
int  	spec_valid 			(int);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 2 && argc != 3)
	{
		print_at (0,0,ML (mlSkMess210), argv [0]);
		print_at (1,0,ML (mlSkMess240), argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	/*
	 * Setup required parameters.
	 */
	init_scr ();		/*  sets terminal from termcap	*/
	set_tty ();		/*  get into raw mode		*/
	set_masks ();	/*  setup print using masks	*/
	swide ();

	OpenDB ();

	if (argc == 3)
	{
		sprintf (local_rec.activeStatus, "%-1.1s", argv [1]);
		local_rec.printerNo = atoi (argv [2]);
		ProcessReport ();
		HeaderOutput ();
		PrintDetails ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------
	| Reset control flags |
	---------------------*/
	entry_exit 	= FALSE;
	edit_exit 	= FALSE;
	prog_exit 	= FALSE;
	search_ok 	= TRUE;
	restart 	= FALSE;
	init_vars (1);		/*  set default values		*/

	SetDefaults ();

	/*----------------------------
	| Edit screen 1 linear input |
	----------------------------*/
	heading (1);
		
	scn_display (1);
	edit (1);

	if (!restart)
		RunProgram (argv [0], argv [1]);
	
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS,	"inmr_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inas, inas_list, INAS_NO_FIELDS, "inas_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (inas);
	abc_dbclose (data);
}

void
SetDefaults (
 void)
{
	strcpy (local_rec.sysDate, DateToString (TodaysDate ()));
	sprintf (local_rec.activeDesc, "%-40.40s", "Status Description");
	local_rec.printerNo = 1;
	strcpy (local_rec.back, "N");
	strcpy (local_rec.backDesc, "No ");
	strcpy (local_rec.onite, "N");
	strcpy (local_rec.oniteDesc, "No ");
}

void
RunProgram (
 char *programName, 
 char *programDesc)
{
	
	shutdown_prog ();

	if (local_rec.onite [0] == 'Y')
	{
		sprintf 
		(
			err_str, 
			"ONIGHT %s %s %d %s", 
			programName, 
			local_rec.activeStatus,
			local_rec.printerNo,
			programDesc
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf 
		(
			err_str, 
			"%s %s %d", 
			programName, 
			local_rec.activeStatus,
			local_rec.printerNo
		);
		SystemExec (err_str,(local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
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

int
spec_valid (
 int field)
{
	/*-------------------------
	| Validate active status. |
	-------------------------*/
	if (LCHECK ("act_status"))
	{
		if (SRCH_KEY)
		{
			SrchInas (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inas_rec.co_no, comm_rec.co_no);
		sprintf (inas_rec.act_code, "%-1.1s", local_rec.activeStatus);
		cc = find_rec (inas, &inas_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlSkMess312));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
		{
			strcpy (local_rec.activeDesc, inas_rec.description);
			DSP_FLD ("act_desc");
		}
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc,
			 (local_rec.back [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.oniteDesc,
			 (local_rec.onite [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}
void
ProcessReport (
 void)
{
	sprintf (repTitle, ML ("Item List For Active Status %s"), local_rec.activeStatus);
	fsort = sort_open ("datfile");
	
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, "                ");

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	dsp_screen ("Processing : Active Status Report Selection.",
				comm_rec.co_no,comm_rec.co_name);

	while (!cc)
	{
		if (!strncmp (inmr_rec.active_status, local_rec.activeStatus, 1))
		{
			dsp_process ("Item No. :",inmr_rec.item_no);
			strcpy (data_rec.itemNo, inmr_rec.item_no);
			strcpy (data_rec.itemDesc, inmr_rec.description);
			StkAvailable ();
			GetExtraDesc ();
			BuildSortFile ();
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
StkAvailable (void)
{
	char *cptr;

	cptr = chk_env ("SO_FWD_AVL");
	if (cptr == (char *)0)
	{
		data_rec.stkAvailable = inmr_rec.on_hand -
				      			inmr_rec.committed -
				      			inmr_rec.backorder -
				      			inmr_rec.forward;
	}
	else
	{
		data_rec.stkAvailable = inmr_rec.on_hand -
				      			inmr_rec.committed -
				      			inmr_rec.backorder;
	}
}

void
GetExtraDesc (void)
{
	char	exDesc [4] [41];
	char	descStr [165];
	int		count = 0;

	exDesc [0] [0] = '\0';
	exDesc [1] [0] = '\0';
	exDesc [2] [0] = '\0';
	exDesc [3] [0] = '\0';

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;
	cc = find_rec (inex, &inex_rec, GTEQ, "r");
	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash && count < 4)
	{
		sprintf (exDesc [count], "%-40.40s", inex_rec.desc);
		cc = find_rec (inex, &inex_rec, NEXT, "r");
		count++;
		
	}
	
	sprintf (descStr,"%s %s %s %s", clip (exDesc [0]),
			 clip (exDesc [1]), clip (exDesc [2]), clip (exDesc [3]));

	strncpy (data_rec.extraDesc, descStr, 87);
}

void
BuildSortFile (void)
{
	char	dataStr [250];

	sprintf (dataStr, 
			"%-16.16s%-40.40s%10.2f%-87.87s\n", 
			data_rec.itemNo, 
			data_rec.itemDesc, 
			data_rec.stkAvailable,
			data_rec.extraDesc);

    sort_save (fsort, dataStr);
}

void
HeaderOutput (void)
{
	if ((fout = popen ("pformat","w")) == (FILE *)NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNo);

	fprintf (fout,".13\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",repTitle);
	fprintf (fout,".B1\n");
	fprintf (fout,".ECOMPANY %s : %s\n",comm_rec.co_no,clip (comm_rec.co_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %s\n",SystemTime ());
	fprintf (fout,".B1\n");

	fprintf (fout,"=====================================================");
	fprintf (fout,"=====================================================");
	fprintf (fout,"====================================================\n");
	fprintf (fout,"!  ITEM NUMBER   !            ITEM DESCRIPTION       ");
	fprintf (fout,"     !AVAILABLE !                                    ");
	fprintf (fout," EXTRA DESCRIPTION                                 !\n");
	fprintf (fout,"!----------------------------------------------------");
	fprintf (fout,"-----------------------------------------------------");
	fprintf (fout,"---------------------------------------------------!\n");

	fprintf (fout,".R=====================================================");
	fprintf (fout,"=====================================================");
	fprintf (fout,"====================================================\n");
}

void
PrintDetails (void)
{
	char 	*sptr;

	fsort = sort_sort (fsort, "datfile");
	sptr = sort_read (fsort);

	while (sptr)
	{
		fprintf (fout,"!%-16.16s", sptr);          	/* Get the item no       */
		fprintf (fout,"!%-40.40s", sptr + 16);      /* Get the item desc     */
		fprintf (fout,"!%10.2f", atof (sptr + 56)); /* Get the Stk Available */
		fprintf (fout,"!%-87.87s!\n",sptr + 66);    /* Get the Extra Desc    */

		sptr = sort_read (fsort);
	}

	fprintf (fout,".EOF\n");
	pclose (fout);
	sort_delete (fsort, "datfile");
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlSkMess667),45,0,1);
		line_at (1,0,132);

		box (0,3,132,5);

		line_at (5,1,131);
		line_at (19,0,132);
		print_at (20,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (21,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		print_at (22,0,ML (mlStdMess099), comm_rec.cc_no,comm_rec.cc_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
SrchInas (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#Cd","#Active Status Description");
	strcpy (inas_rec.co_no, comm_rec.co_no);
	strcpy (inas_rec.act_code, " ");
	cc = find_rec (inas, &inas_rec, GTEQ, "r");
	while (!cc)
	{
		cc = save_rec (inas_rec.act_code, inas_rec.description);
		if (cc)
			break;

		cc = find_rec (inas, &inas_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();

	strcpy (inas_rec.co_no, comm_rec.co_no);
	strcpy (inas_rec.act_code, temp_str);
	cc = find_rec (inas, &inas_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inas, "DBFIND");
}
