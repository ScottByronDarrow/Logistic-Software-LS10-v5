/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (so_byitemprt.c)                                   |
|  Program Desc  : (Print Sales Orders by Item No.              )     |	
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, sohr, soln, inmr, inex,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 18/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : 23/01/89        | Modified  by : Fui Choo Yap.     |
|  Date Modified : 25/02/89        | Modified  by : Scott Darrow.     |
|  Date Modified : 23/03/89        | Modified  by : Fui Choo Yap.     |
|  Date Modified : (03/08/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (13/09/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (06/07/92)      | Modified  by : Simon Dubey.      |
|  Date Modified : (14/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (29/09/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (17/11/92)      | Modified  by : Simon Dubey.      |
|  Date Modified : (21/01/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (16/09/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (15/02/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (13/04/94)      | Modified  by : Roel Michels      |
|  Date Modified : (26/09/96)      | Modified  by : Elizabeth D. Paid |
|  Date Modified : (17/10/96)      | Modified  by : Elizabeth D. Paid |
|  Date Modified : (08/11/96)      | Modified  by : Bernard.M.delaVega|
|  Date Modified : (04/09/97)      | Modified  by : Rowena S Maandig  |
|  Date Modified : (23/10/1997)    | Modified  by  : Campbell Mander. |
|                                                                     |
|  Comments      : Modfied version of PSM.                            |
|                : If soln_qty_order+soln_qty_bord > 0 OR             |
|                : inmr_class = "Z" --> process order.                |
|                : Include an ALL orders option or BACKORDERS option. |
|                : Fix bug with  backgd/onite processing :            |
|		         : If (fork () != 0) then exit (0)                    | 
|                : Fix bug with  backgd processing :                  |
|                : If report printed in backgd, then don't set_tty () |
|                :                                                    |
|                : (25/02/89) - Added Tax Calcs etc.                  |
|                : (23/03/89) - Fix Bugs - $values weren't DOLLARS () |
|                :           - before they were printed.              |
|                : (03/08/90) - General Update for New Scrgen. S.B.D. |
|  (13/09/91)    : Added transfer print.                              |
|  (06/07/92)    : To include inex desc lines SC dfh 7287             |
|  (14/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (29/09/92)    : Fix found_soln definition for IBM compile.         |
|                : SC 7745 PSL.                                       |
|  (17/11/92)    : DPL8036 was doing "%-40-40s" rather than "%-40.40s"|
|  (21/01/93)    : PSL 6828 changes for multi-curr so.                |
|  (16/09/93)    : HGP 9503. Increase cus_ord_ref to 20 chars.        |
|  (15/02/94)    : PSL 10424.  Correct the printing of inex records.  |
|  (13/04/94)    : PSL 10673 - Online conversion                      |
|  (26/09/96)    : PSL 10673 - Added the base UOM in the report.      |
|  (17/10/96)    : PSL 10673 - Fixed the bug in computing the sales   |
|                :             values.                                |
|  (08/11/96)    : Modified the program to print the Ouantity         |
|                : Outstanding and Outstanding LCL Sales Value based  |
|                : on standard UOM                                    |
|  (04/09/97)    : Incorporate multilingual conversion.               |
|  (23/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: byitemprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_byitemprt/byitemprt.c,v 5.5 2002/07/17 09:58:06 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#ifndef	FALSE
#define	FALSE	0
#endif

#ifndef	TRUE
#define	TRUE	1
#endif

#define	FORWARD_ORDERS		 (status [0] == 'F')
#define	BK_ORDERS			 (status [0] == 'B')
#define	MANUAL_ORDER		 (status [0] == 'M')
#define	HELD_ORDERS			 (status [0] == 'H' || status [0] == 'O')
#define	ALL_ORDERS			 (status [0] == ' ')

#define	B_TFERS				 (local_rec.tfers [0] == 'B')

#define	FWD_OK				(soln_rec.status [0] == 'F')
#define	BACK_OK				(soln_rec.status [0] == 'B')
#define	MAN_OK				(soln_rec.status [0] == 'M')
#define	HELD_OK				(soln_rec.status [0] == 'H' || \
				 			soln_rec.status [0] == 'O' || \
				 			soln_rec.status [0] == 'C')

#define	LINE	0
#define	BLNK	1

char *head_prt = "| NO / NAME OF RECEIVING WAREHOUSE                            |  DEL NO  |  ISS DATE   | DUE DATE |   QTY B/O   |            |        |                |     |";

#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;
struct ccmrRecord	ccmr_rec;
struct inexRecord	inex_rec;

	int		line_num 	= 0,
			found_soln 	= FALSE,
			first_time 	= TRUE,
			part_det 	= 0;

	int		envVarRepTax 		= FALSE,
			envVarDbNettUsed 	= TRUE;
	

	char	status [2];
	char	rec_wh_name [41];

	int		first_flag 		= TRUE;

	float	qty 			= 0.0,
			tot_qty [4],
			tot_so_qty 		= 0.0,
			grand_qty 		= 0.0;

	double	extend			= 0.00,
			tot_so_shipmt 	= 0.00,
			grand_shipmt 	= 0.00;

	extern	int	TruePosition;
	FILE	*fout;

	int		envVarDbMcurr;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	s_item [17];
	char	e_item [17];
	char	item_desc [2] [41];
	int		printerNo;
	char	tfers [2];
	char	tfersDesc [21];
	char	lp_str [3];
	char 	back [2];
	char 	backDesc [21];
	char	onite [2];
	char	oniteDesc [21];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startItem",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " Start Item  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_item},
	{1, LIN, "startItemDesc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc [0]},
	{1, LIN, "endItem",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " End   Item  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_item},
	{1, LIN, "endItemDesc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc [1]},

	{1, LIN, "tfers", 7, 2, CHARTYPE,
		"U", "          ",
		" ", "N", " Transfers   ", " N(o) / C(ustomer) / S(tock) / B(oth)  ",
		NA, NO, JUSTLEFT, "NCSB", "", local_rec.tfers},
	{1, LIN, "tfersDesc", 7, 18, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "NCSB", "", local_rec.tfersDesc},
	{1, LIN, "printerNo",	 9, 2, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No  ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	 10, 2, CHARTYPE,
		"U", "          ",
		" ", "N", " Background  ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 10, 18, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onite",	 11, 2, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight   ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc",	 11, 18, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.oniteDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=======================
| Function Declarations |
=======================*/
int  	spec_valid 			(int);
int  	heading 			(int);
int  	ValidOrder 			(void);
int  	ProcessTrans 		(long, int, int);
int  	GetIthr 			(void);
int  	GetRecWarehouse 	(void);
void 	RunProgram 			(char *);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	HeadingOutput 		(void);
void 	PrintLine 			(int);
void 	ProcessFile 		(void);
void 	ProcessSoln 		(long);
void 	PrintSoln 			(long);
void 	PrintTotalPart 		(void);
void 	PrintInex 			(int, int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	char	order_desc [30];

	TruePosition	=	TRUE;

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (argc != 2 && argc != 6)
	{
		print_at (0,0,mlSoMess757,argv [0]);
		print_at (1,0,mlSoMess758);
		print_at (2,0,mlSoMess759);
		print_at (3,0,mlSoMess760);
		print_at (4,0,mlSoMess761);
		return (EXIT_SUCCESS);
	}

	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	SETUP_SCR (vars);

	OpenDB ();

	sprintf (status,"%-1.1s",argv [1]);

	if (argc == 6)
	{
		sprintf (local_rec.s_item,"%-16.16s",argv [2]);
		sprintf (local_rec.e_item,"%-16.16s",argv [3]);
		local_rec.printerNo = atoi (argv [4]);
		sprintf (local_rec.tfers, "%-8.8s", argv [5]);

		if (FORWARD_ORDERS)
			strcpy (order_desc,"(Forward Orders)");

		if (BK_ORDERS)
			strcpy (order_desc, "(Backorders)");

		if (MANUAL_ORDER)
			strcpy (order_desc, "(New/Unconsolidated Orders)");

		if (HELD_ORDERS)
			strcpy (order_desc,"(Held Orders)");

		if (ALL_ORDERS)
			strcpy (order_desc, "(ALL Orders)");

		sprintf (err_str,"Processing %s By Item Number.", order_desc);

		dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);

		HeadingOutput ();
		ProcessFile ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	strcpy (local_rec.tfers, "No      ");
	if (ALL_ORDERS || BK_ORDERS)
		FLD ("tfers") = NO;

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode			*/
	set_masks ();			/*  setup print using masks		*/
	init_vars (1);			/*  set default values			*/

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/

		/*-----------------------------
		| Entry screen 1 linear input |
		-----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
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
 char *prog_name)
{
	sprintf (local_rec.lp_str,"%2d",local_rec.printerNo);
	
	shutdown_prog ();

	if (!strncmp (local_rec.e_item, "~~~~~~~~~~~~~~~~", 16))
		memset ((char *)local_rec.e_item,0xff,sizeof (local_rec.e_item));
	
	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				status,
				local_rec.s_item,
				local_rec.e_item, 
				local_rec.lp_str,
				local_rec.tfers,
				ML (mlSoMess396), (char *)0);
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				status,
				local_rec.s_item,
				local_rec.e_item,
				local_rec.lp_str,
				local_rec.tfers, (char *)0);
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			status,
			local_rec.s_item,
			local_rec.e_item,
			local_rec.lp_str,
			local_rec.tfers, (char *)0);
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
	/*------------------
	| Validate Item No |
	-------------------*/
	if (LCHECK ("startItem")) 
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();
	
		if (!dflt_used)
		{
			cc = FindInmr (comm_rec.co_no, local_rec.s_item, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.s_item);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				errmess (ML (mlStdMess001));
	    		sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.item_desc [0],inmr_rec.description);
			SuperSynonymError ();
		}
		else
		{
			strcpy (local_rec.s_item,"                ");
			sprintf (local_rec.item_desc [0],"%-40.40s",ML ("START ITEM"));
		}

		if (prog_status != ENTRY && 
		    strcmp (local_rec.s_item,local_rec.e_item) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("startItemDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endItem")) 
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (!dflt_used)
		{
			cc = FindInmr (comm_rec.co_no, local_rec.e_item, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.e_item);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				errmess (ML (mlStdMess001));
	    		sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.item_desc [1],inmr_rec.description);
		}
		else
		{
			strcpy (local_rec.e_item,"~~~~~~~~~~~~~~~~");
			sprintf (local_rec.item_desc [1],"%-40.40s",ML ("END ITEM"));
		}

		if (strcmp (local_rec.s_item,local_rec.e_item) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("endItemDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("tfers"))
	{
		switch (local_rec.tfers [0])
		{
		case 'C':
			strcpy (local_rec.tfersDesc, ML ("Customer"));
			break;

		case 'S':
			strcpy (local_rec.tfersDesc, ML ("Stock"));
			break;

		case 'B':
			strcpy (local_rec.tfersDesc, ML ("Both"));
			break;

		case 'N':
		default:
			strcpy (local_rec.tfersDesc, ML ("No"));
			break;
		}
		DSP_FLD ("tfersDesc");
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
		strcpy (local_rec.backDesc, 
				(local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.oniteDesc, 
					(local_rec.onite [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
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

		if (FORWARD_ORDERS)
			strcpy (err_str,ML (mlSoMess152));

		if (BK_ORDERS)
			strcpy (err_str,ML (mlSoMess153));

		if (MANUAL_ORDER)
			strcpy (err_str,ML (mlSoMess154));

		if (HELD_ORDERS)
			strcpy (err_str,ML (mlSoMess155));

		if (ALL_ORDERS)
			strcpy (err_str,ML (mlSoMess156));

		rv_pr (err_str, (80 - strlen (err_str)) / 2,0,1);
		line_at (1,0,80);

		move (1,input_row);

		box (0,3,80,8);

		line_at (6,1,79);
		line_at (8,1,79);
		line_at (20,0,80);
		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (ithr);
	abc_fclose (itln);
	abc_fclose (ccmr);
	abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{

	if ((fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", PNAME);

	fprintf (fout,".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNo);
	fprintf (fout,".13\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L160\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".B1\n");

	if (FORWARD_ORDERS)
		fprintf (fout,".EFORWARD ORDERS BY ITEM NUMBER\n");

	if (BK_ORDERS)
		fprintf (fout,".EBACKORDER BY ITEM NUMBER\n");

	if (MANUAL_ORDER)
		fprintf (fout,".ENEW/UNCONSOLIDATED ORDERS BY ITEM NUMBER\n");

	if (HELD_ORDERS)
		fprintf (fout,".EHELD ORDERS BY ITEM NUMBER\n");

	if (ALL_ORDERS)
		fprintf (fout,".EALL ORDERS BY ITEM NUMBER\n");

	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %s\n",SystemTime ());

	fprintf (fout,".R====================================");
	fprintf (fout,"=======");
	fprintf (fout,"====================================================");
	fprintf (fout,"=================");
	fprintf (fout,"==================================================\n");

	fprintf (fout,"====================================");
	fprintf (fout,"=======");
	fprintf (fout,"====================================================");
	fprintf (fout,"=================");
	fprintf (fout,"==================================================\n");

	fprintf (fout,"|       ITEM       ");
	fprintf (fout,"|            DESCRIPTION             ");
	fprintf (fout,"|   DATE   ");
	fprintf (fout,"|  CATEGORY   ");
	fprintf (fout,"| UOM ");
	fprintf (fout,"|  QUANTITY  ");
	fprintf (fout,"| OUTSTDNG LCL");
	fprintf (fout,"|  CUSTOMER  ");
	fprintf (fout,"|  S.O.  ");
	fprintf (fout,"|  CUST. ORDER   |");
	fprintf (fout,"STAT.|");
	fprintf (fout,"\n");

	fprintf (fout,"|      NUMBER      ");
	fprintf (fout,"|                                    ");
	fprintf (fout,"|   DUE    ");
	fprintf (fout,"|   NUMBER    ");
	fprintf (fout,"|     ");
	fprintf (fout,"|  OUTSTDNG  ");
	fprintf (fout,"| SALES VALUE ");
	fprintf (fout,"|  ACRONYM   ");
	fprintf (fout,"| NUMBER ");
	fprintf (fout,"|  NUMBER        |");
	fprintf (fout,"     |");
	fprintf (fout,"\n");
	PrintLine (LINE);
	fflush (fout);
}

void
PrintLine (
 int line_type)
{

	switch (line_type)
	{
	case LINE:
		fprintf (fout,"|------------------");
		fprintf (fout,"|------------------------------------");
		fprintf (fout,"|----------");
		fprintf (fout,"|-------------");
		fprintf (fout,"|-----");
		fprintf (fout,"|------------");
		fprintf (fout,"|-------------");
		fprintf (fout,"|------------");
		fprintf (fout,"|--------");
		fprintf (fout,"|----------------|");
		fprintf (fout,"-----|\n");
		break;

	case BLNK:
		fprintf (fout,"|                  ");
		fprintf (fout,"|                                    ");
		fprintf (fout,"|          ");
		fprintf (fout,"|             ");
		fprintf (fout,"|     ");
		fprintf (fout,"|            ");
		fprintf (fout,"|             ");
		fprintf (fout,"|            ");
		fprintf (fout,"|        ");
		fprintf (fout,"|                |");
		fprintf (fout,"     |\n");
		break;
	}

	fflush (fout);
}

void
ProcessFile (
 void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.item_no,local_rec.s_item);
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && 
		       strcmp (inmr_rec.item_no,local_rec.s_item) >= 0 && 
		       strcmp (inmr_rec.item_no,local_rec.e_item) <= 0)
	{
		ProcessSoln (inmr_rec.hhbr_hash);
		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}

	/*-----------------------------------------------
	| On last record,print the total backlog.	|
	-----------------------------------------------*/
	if (found_soln)
	{
		fprintf (fout,"| ***** TOTAL SALES ORDER *****    ");
		fprintf (fout,"                     ");
		fprintf (fout,"|          ");
		fprintf (fout,"|             ");
		fprintf (fout,"|     ");
		fprintf (fout,"| %10.2f ",grand_qty);
		fprintf (fout,"|%12.2f ",DOLLARS (grand_shipmt));
		fprintf (fout,"|            ");
		fprintf (fout,"|        ");
		fprintf (fout,"|                |");
		fprintf (fout,"     |\n");
	}
	fflush (fout);
}

void
ProcessSoln (
	long	hhbrHash)
{
	int		validFlag = FALSE;

	tot_so_qty = 0.0;
	tot_so_shipmt = 0.0;

	first_time = TRUE;

	soln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (soln,&soln_rec,GTEQ,"r");
	while (!cc && soln_rec.hhbr_hash == hhbrHash) 
	{
		if (!ValidOrder ())
		{
			cc = find_rec (soln,&soln_rec,NEXT,"r");
			continue;
		}

		if ((soln_rec.qty_order + soln_rec.qty_bord > 0.00) || 
		      inmr_rec.inmr_class [0] == 'Z')
		{
			sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
			cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
			if (cc)
				file_err (cc, sohr, "DBFIND");

			dsp_process ("Item No. : ",inmr_rec.item_no);
			PrintSoln (hhbrHash);
			validFlag = TRUE;
		}	
		cc = find_rec (soln,&soln_rec,NEXT,"r");
	}
	if (validFlag)
		PrintInex (line_num, FALSE);
	if (part_det > 1)
	{
		if ((ALL_ORDERS || BK_ORDERS) && strncmp (local_rec.tfers,"N",1))
		{
			PrintLine (BLNK);
			ProcessTrans (hhbrHash, TRUE, TRUE);
		}

		PrintTotalPart ();
	}
	else
	if (part_det == 1)
		PrintLine (LINE);
	part_det = 0;
}

/*=======================
| Validate order types. |
=======================*/
int
ValidOrder (
 void)
{
	if (ALL_ORDERS)
		return (TRUE);

	if (FORWARD_ORDERS && FWD_OK)
		return (TRUE);

	if (BK_ORDERS && BACK_OK)
		return (TRUE);

	if (MANUAL_ORDER && MAN_OK)
		return (TRUE);

	if (HELD_ORDERS && HELD_OK)
		return (TRUE);

	return (FALSE);
}

void
PrintSoln (
 long hhbrHash)
{
	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00,
			lcl_ex_rate	= 0.00;

	cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
	cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, cumr, "DBFIND");

	found_soln = TRUE;


	lcl_ex_rate = (envVarDbMcurr && sohr_rec.exch_rate != 0.00) ? sohr_rec.exch_rate : 1.00;

	qty = soln_rec.qty_order + soln_rec.qty_bord;

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	(double) qty;
		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);

		if (envVarRepTax)
		{
			l_tax	=	(double) soln_rec.tax_pc;
			if (sohr_rec.tax_code [0] == 'D')
				l_tax *= l_total;
			else
			{
				if (envVarDbNettUsed)
					l_tax	*=	(l_total + soln_rec.item_levy + l_disc);
				else
					l_tax	*=	(l_total + soln_rec.item_levy);
			}
			l_tax	=	DOLLARS (l_tax);
		}
		l_tax	=	no_dec (l_tax);

		l_gst	=	(double) soln_rec.gst_pc;
		if (envVarDbNettUsed)
			l_gst	*=	(l_total - l_disc) + l_tax + soln_rec.item_levy;
		else
			l_gst	*=	(l_total + l_tax + soln_rec.item_levy);

		l_gst	=	DOLLARS (l_gst);
			
		if (envVarDbNettUsed)
			extend	=	l_total - l_disc + l_tax + l_gst + soln_rec.item_levy;
		else
			extend	=	l_total + l_tax + l_gst + soln_rec.item_levy;

		extend	/=	lcl_ex_rate;
	}
	tot_so_qty += qty;
	tot_so_shipmt += extend;

	grand_qty += qty;
	grand_shipmt += extend;


	if (first_time)
	{
		fprintf (fout,"| %-16.16s ",inmr_rec.item_no);
		fprintf (fout,"| %-34.34s ",soln_rec.item_desc);
		first_time = FALSE;
		line_num = 0;
	}
	else
	{
		fprintf (fout,"|%-18.18s"," ");
		PrintInex (line_num, TRUE);
		line_num++;
	}

	if (sohr_rec.dt_required == 0L)
		fprintf (fout,"|          ");
	else
		fprintf (fout,"|%-10.10s",DateToString (sohr_rec.dt_required));

 	fprintf (fout,"| %-11.11s ",inmr_rec.category);
 	fprintf (fout,"| %-4.4s",inmr_rec.sale_unit);
	fprintf (fout,"| %10.2f ",qty);
	fprintf (fout,"| %11.2f ",DOLLARS (extend));
	fprintf (fout,"|  %-9.9s ",cumr_rec.dbt_acronym);
	fprintf (fout,"|%-8.8s",sohr_rec.order_no);
	fprintf (fout,"|%-16.16s|",sohr_rec.cus_ord_ref);
	fprintf (fout,"  %1.1s  |\n",soln_rec.status);
	fflush (fout);

	part_det++;
}

/*-------------------------
| print total for a part. |
-------------------------*/
void
PrintTotalPart (
 void)
{
	fprintf (fout,".LRP2\n");
	PrintLine (LINE);
	fprintf (fout,"|  TOTAL FOR ITEM    :  %-16.16s ",inmr_rec.item_no);
	fprintf (fout,"               ");
	fprintf (fout,"|          |             ");
	fprintf (fout,"|     ");
	fprintf (fout,"| %10.2f ",tot_so_qty);
	fprintf (fout,"| %11.2f ",DOLLARS (tot_so_shipmt));
	fprintf (fout,"|            ");
	fprintf (fout,"|        ");
	fprintf (fout,"|                |");
	fprintf (fout,"     |\n");
	PrintLine (LINE);
}

int
ProcessTrans (
 long hhbrHash, 
 int rule_off, 
 int print_item)
{
	int	lcl_cc;
	int	fst_time = TRUE;
	int	tfer_printed = FALSE;
	char	tmp_iss_date [11];
	char	tmp_due_date [11];

	itln_rec.hhbr_hash	=	hhbrHash;
	lcl_cc = find_rec ("itln", &itln_rec, GTEQ, "r");
	while (!lcl_cc && itln_rec.hhbr_hash == hhbrHash)
	{
		if ((!strncmp (itln_rec.stock, local_rec.tfers, 1) || 
		     B_TFERS) &&
	    	    ((BK_ORDERS && !strcmp (itln_rec.status, "B")) || 
	              ALL_ORDERS))
		{
			if (GetIthr ())
			{
				lcl_cc = find_rec ("itln", &itln_rec, NEXT, "r");
				continue;
			}
	
			if (fst_time)
			{
				tot_qty [3] = 0.00;
	
				if (rule_off)
		        		fprintf (fout, "%s\n", head_prt);
				else
				{
		        		PrintLine (BLNK);
		        		fprintf (fout, "%s\n", head_prt);
				}
				fst_time = FALSE;
			}
			GetRecWarehouse ();

			strcpy (tmp_iss_date, DateToString (ithr_rec.iss_date));

			strcpy (tmp_due_date, DateToString (itln_rec.due_date));

			fprintf (fout, "| %2.2s / ", ccmr_rec.cc_no);
			fprintf (fout, "%-40.40s         ", ccmr_rec.name);
	
			fprintf (fout, "      |  %06ld  ", ithr_rec.del_no);
			fprintf (fout, "| %10.10s  ", tmp_iss_date);
			fprintf (fout, "|%10.10s", tmp_due_date);
			fprintf (fout, 
				"| %10.2f  |            |        |                |     |\n", 
				itln_rec.qty_border);

			fflush (fout);

			tot_qty [3] += itln_rec.qty_border;
			tot_qty [1] += itln_rec.qty_border;
			tfer_printed = TRUE;
		}
		lcl_cc = find_rec ("itln", &itln_rec, NEXT, "r");
	}
	
    	if (tfer_printed)
    	{
	    	PrintLine (BLNK);
	    	fprintf (fout, 
			"|%-37.37s%-24.24s|          |             |          | %10.2f  |            |        |                |     |\n",
			"  **  TRANSFER TOTAL  **",
			" ",
			tot_qty [3]);
			fflush (fout);
    	}
	
    	return (tfer_printed);
}

int
GetIthr (
 void)
{
	int	lcl_cc;

	ithr_rec.hhit_hash	=	itln_rec.hhit_hash;
	lcl_cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
	return (lcl_cc);
}

int
GetRecWarehouse (
 void)
{
	int	lcl_cc;

	
	ccmr_rec.hhcc_hash	=	itln_rec.r_hhcc_hash;
	lcl_cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (lcl_cc)
		sprintf (rec_wh_name, "%-40.40s", "Not Found");
	else
		sprintf (rec_wh_name, "%-40.40s", ccmr_rec.name);

	return (EXIT_SUCCESS);
}

void
PrintInex (
 int line_num, 
 int FLAG)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = line_num;

	if (!FLAG)
	{
		cc = find_rec ("inex", &inex_rec, GTEQ, "r");

		if (cc)
		{
			return;
		}

		while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			fprintf (fout,"|%-18.18s"," ");
			fprintf (fout,"| %-34.34s ", inex_rec.desc);
			fprintf (fout,"|          ");
			fprintf (fout,"| %-11.11s "," ");
			fprintf (fout,"|     ");
			fprintf (fout,"| %10.10s "," ");
			fprintf (fout,"| %11.11s "," ");
			fprintf (fout,"|  %-9.9s "," ");
			fprintf (fout,"| %-6.6s "," ");
			fprintf (fout,"|%-16.16s|"," ");
			fprintf (fout,"  %1.1s  |\n"," ");
			fflush (fout);
			cc = find_rec ("inex", &inex_rec, NEXT, "r");
		}
	}

	if (FLAG)
	{
		cc = find_rec ("inex", &inex_rec, COMPARISON, "r");

		if (cc)
		{
			fprintf (fout,"| %-34.34s ", " ");
			return;
		}

		if (inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			fprintf (fout,"| %-34.34s ", inex_rec.desc);
		}
	}
}
