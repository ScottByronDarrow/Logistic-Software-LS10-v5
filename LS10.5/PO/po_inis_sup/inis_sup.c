/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: inis_sup.c,v 5.3 2002/07/17 09:57:37 scott Exp $
|  Program Name  : (po_inis_sup.c)
|  Program Desc  : (Inventory Supplier XRef By Supplier)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 03/02/89         |
|---------------------------------------------------------------------|
| $Log: inis_sup.c,v $
| Revision 5.3  2002/07/17 09:57:37  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:15:41  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:30  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/13 05:32:11  scott
| Updated to add app.schema - removes code related to tables from program as it allows for better quality contol.
| Updated to add sleep delay - did not work with LS10-GUI
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: inis_sup.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_inis_sup/inis_sup.c,v 5.3 2002/07/17 09:57:37 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inisRecord	inis_rec;
struct inumRecord	inum_rec;

	int		envVarCrCo 		= 0,
			envVarCrFind  	= 0,
			programRun 		= FALSE,
			firstTime 		= 1;

	char	oldSupplierNo [7],
			newSupplierNo [7],
			branchNumber [2];

	extern	int		TruePosition;
	extern	int		EnvScreenOK;
	FILE	*fout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startSupplierNo [7];
	char	endSupplierNo [7];
	char	supplierName [2] [41];
	int		printerNo;
	char	printerString [3];
	char 	back [2];
	char 	backDesc [16];
	char	onite [2];
	char	oniteDesc [16];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "fromSupplier", 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Supplier     ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startSupplierNo}, 
	{1, LIN, "name1", 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.supplierName [0]}, 
	{1, LIN, "toSupplier", 5, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "~~~~~~", "End Supplier       ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endSupplierNo}, 
	{1, LIN, "name2", 5, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.supplierName [1]}, 
	{1, LIN, "printerNo", 7, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number     ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "back", 8, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background         ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 8, 26, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "N", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onite", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight          ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onite}, 
	{1, LIN, "oniteDesc", 9, 26, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "N", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.oniteDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};
#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/

void 	shutdown_prog 				 (void);
void 	OpenDB 						 (void);
void 	CloseDB 					 (void);
void 	RunProgram 					 (char *);
void 	exitBackground 				 (void);
void 	HeadingOutput 				 (void);
void 	ProcessFile 				 (void);
void 	GetInis  					 (long);
int 	spec_valid 					 (int);
void 	PrintSupplier 				 (int);
int 	heading 					 (int);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 1 && argc != 4)
	{
		print_at (0,0,mlPoMess724);
        return (EXIT_FAILURE);
	}

	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	SETUP_SCR (vars);

	envVarCrCo = atoi (get_env ("CR_CO"));
	envVarCrFind = atoi (get_env ("CR_FIND"));

	OpenDB ();

	if (envVarCrCo == 0)
		strcpy (branchNumber," 0");
	else
		strcpy (branchNumber,comm_rec.est_no);

	if (argc == 4)
	{
		sprintf (local_rec.startSupplierNo,"%-6.6s",argv [1]);
		sprintf (local_rec.endSupplierNo,"%-6.6s",argv [2]);
		local_rec.printerNo = atoi (argv [3]);

		dsp_screen ("Processing : Inventory Supplier X-Ref Report By Supplier", comm_rec.co_no, comm_rec.co_name);

		if ((fout = popen ("pformat","w")) == NULL)
			sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);
		HeadingOutput ();
		ProcessFile ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		exitBackground ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);		/*  set default values		*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("inmr", inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec ("inis", inis_list, INIS_NO_FIELDS, "inis_id_no4");
	open_rec ("sumr", sumr_list, SUMR_NO_FIELDS, (envVarCrFind == 0) ? "sumr_id_no" : "sumr_id_no3");
	open_rec ("inum", inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("inmr");
	abc_fclose ("inis");
	abc_fclose ("sumr");
	abc_fclose ("inum");
	abc_dbclose ("data");
}

void
RunProgram  (
 char *prog_name)
{
	programRun = TRUE;

	sprintf (local_rec.printerString,"%d",local_rec.printerNo);
	
	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	CloseDB (); FinishProgram ();;
	rset_tty ();

	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
			execlp
			 (
				"ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.startSupplierNo,
				local_rec.endSupplierNo,
				local_rec.printerString,
				"Print Inventory Xref By Supplier", (char *)0
			);
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp
			 (
				prog_name,
				prog_name,
				local_rec.startSupplierNo,
				local_rec.endSupplierNo,
				local_rec.printerString, (char *)0
			);
	}
	else 
	{
		execlp
		 (
			prog_name,
			prog_name,
			local_rec.startSupplierNo,
			local_rec.endSupplierNo,
			local_rec.printerString, (char *)0
		);
	}
}

void
exitBackground (
 void)
{
	if (!programRun)
		CloseDB (); 
		
	FinishProgram ();
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.printerNo);

	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".EINVENTORY SUPPLIER XREF BY SUPPLIER REPORT\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout, ".R==================");
	fprintf (fout, "==================");
	fprintf (fout, "==========================================");
	fprintf (fout, "====");
	fprintf (fout, "=====");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========\n");

	fprintf (fout, "==================");
	fprintf (fout, "==================");
	fprintf (fout, "==========================================");
	fprintf (fout, "====");
	fprintf (fout, "=====");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========\n");

	fprintf (fout, "| SUPPLIER ITEM NO");
	fprintf (fout, "|    ITEM NUMBER  ");
	fprintf (fout, "|       ITEM DESCRIPTION                  ");
	fprintf (fout, "|PRI");
	fprintf (fout, "|CURR");
	fprintf (fout, "| SUPPLIER ");
	fprintf (fout, "|LAST  COST");
	fprintf (fout, "|DUTY");
	fprintf (fout, "|LIC.");
	fprintf (fout, "|UOM ");
	fprintf (fout, "|MIN  ORDER");
	fprintf (fout, "|NORM ORDER");
	fprintf (fout, "|LEAD TIME|\n");

	fprintf (fout, "|                 ");
	fprintf (fout, "|                 ");
	fprintf (fout, "|                                         ");
	fprintf (fout, "|   ");
	fprintf (fout, "|    ");
	fprintf (fout, "|   PRICE  ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|CODE");
	fprintf (fout, "|CODE");
	fprintf (fout, "|    ");
	fprintf (fout, "|   QTY    ");
	fprintf (fout, "|   QTY    ");
	fprintf (fout, "|   DAYS  |\n");

	fprintf (fout, "|-----------------");
	fprintf (fout, "|-----------------");
	fprintf (fout, "|-----------------------------------------");
	fprintf (fout, "|---");
	fprintf (fout, "|----");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|---------|\n");
	fflush (fout);
}

void
ProcessFile (
 void)
{
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no,local_rec.startSupplierNo);

	cc = find_rec ("sumr",&sumr_rec,GTEQ,"r");

	strcpy (oldSupplierNo,sumr_rec.crd_no);
	strcpy (newSupplierNo,sumr_rec.crd_no);

	if (!strcmp (local_rec.endSupplierNo, "~~~~~~"))
		memset ((char *)local_rec.endSupplierNo, 0xff, sizeof (local_rec.endSupplierNo));

	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no) && 
				  !strcmp (sumr_rec.est_no,branchNumber) && 
				  strcmp (sumr_rec.crd_no,local_rec.startSupplierNo) >= 0 && 
				  strcmp (sumr_rec.crd_no,local_rec.endSupplierNo) <= 0) 
	{
		dsp_process ("Supplier : ",sumr_rec.crd_no);

		GetInis (sumr_rec.hhsu_hash);

		cc = find_rec ("sumr",&sumr_rec,NEXT,"r");
	}
}

void
GetInis  (
	long	hhsuHash)
{

	inis_rec.hhsu_hash = hhsuHash;
	strcpy (inis_rec.sup_part, "                ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec ("inis",&inis_rec,GTEQ,"r");
	while (!cc && inis_rec.hhsu_hash == hhsuHash)
	{
		inmr_rec.hhbr_hash	=	inis_rec.hhbr_hash;
		cc = find_rec ("inmr",&inmr_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy (inmr_rec.item_no,"Unknown Item");
			strcpy (inmr_rec.description,"No Description Found");
		}

		strcpy (newSupplierNo,sumr_rec.crd_no);

		if (firstTime || strcmp (newSupplierNo,oldSupplierNo))
		{
			PrintSupplier (firstTime);
			strcpy (oldSupplierNo,newSupplierNo);
			firstTime = 0;
		}
		inum_rec.hhum_hash	=	inis_rec.sup_uom;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		if (cc)
			strcpy (inum_rec.uom, "????");

		fprintf (fout,"| %16.16s",	inis_rec.sup_part);
		fprintf (fout,"| %16.16s",	inmr_rec.item_no);
		fprintf (fout,"|%40.40s ",	inmr_rec.description);
		fprintf (fout,"|%-2.2s ",	inis_rec.sup_priority);
		fprintf (fout,"|%-3.3s ",	sumr_rec.curr_code);
		fprintf (fout,"|%9.2f ",		inis_rec.fob_cost);
		fprintf (fout,"|%10.10s",	DateToString (inis_rec.lcost_date));
		fprintf (fout,"| %-2.2s ",	inis_rec.duty);
		fprintf (fout,"| %-2.2s ",	inis_rec.licence);
		fprintf (fout,"|%-4.4s",		inum_rec.uom);
		fprintf (fout,"|%9.2f ",		inis_rec.min_order);
		fprintf (fout,"|%9.2f ",		inis_rec.norm_order);
		fprintf (fout,"|%8.2f |\n",	inis_rec.lead_time);

		cc = find_rec ("inis",&inis_rec,NEXT,"r");
	}
}

int
spec_valid (
 int field)
{
	/*-------------------
	| Validate Supplier |
	-------------------*/
	if (LCHECK ("fromSupplier"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startSupplierNo,"      ");
			sprintf (local_rec.supplierName [0],"%-40.40s","Start Supplier");
			DSP_FLD ("fromSupplier");
			DSP_FLD ("name1");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && strcmp (local_rec.startSupplierNo,local_rec.endSupplierNo) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.startSupplierNo));
		cc = find_rec ("sumr", &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlPoMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.supplierName [0],sumr_rec.crd_name);
		DSP_FLD ("name1");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("toSupplier"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endSupplierNo,"~~~~~~");
			sprintf (local_rec.supplierName [1],"%-40.40s", ML ("End Supplier"));
			DSP_FLD ("toSupplier");
			DSP_FLD ("name2");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.endSupplierNo));
		cc = find_rec ("sumr", &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlPoMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startSupplierNo,local_rec.endSupplierNo) > 0)
		{
			errmess (ML (mlPoMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.supplierName [1],sumr_rec.crd_name);
		DSP_FLD ("name2");
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
		strcpy (local_rec.backDesc, (local_rec.back [0] == 'Y') ? ML ("YES")
																: ML ("NO "));
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.oniteDesc, (local_rec.onite [0] == 'Y') ? ML ("YES")
																  : ML ("NO "));
		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
PrintSupplier (
	int		firstTime)
{
	char	workSumrString [50];
	char	workString [300];
	sprintf (workSumrString,"%-6.6s - %40.40s",sumr_rec.crd_no,sumr_rec.crd_name);
	expand (workString,workSumrString);

	fprintf (fout, ".PD|%-155.155s|\n",workString);
	if (!firstTime)
		fprintf (fout, ".PA\n");
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
		rv_pr (ML (mlPoMess137),21,0,1);

		line_at (1,0,80);
		line_at (6,1,79);
		line_at (20,0,80);
		box (0,3,80,6);

		print_at (21,0,ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
