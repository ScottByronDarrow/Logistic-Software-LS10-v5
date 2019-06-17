/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sts_disp.c,v 5.3 2002/07/17 09:58:11 scott Exp $
|  Program Name  : (so_sts_disp.c)
|  Program Desc  : (Display/Print Sales Order Status)
|---------------------------------------------------------------------|
|  Date Written  : (12/10/88)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: sts_disp.c,v $
| Revision 5.3  2002/07/17 09:58:11  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:22:08  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:52:07  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:46  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/23 06:04:04  scott
| Updated to display status as "B" when qty-order = 0.00 and qty-backorder > 0.0 and current status = "M"
|
| Revision 4.0  2001/03/09 02:42:02  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/06 01:13:56  scott
| Updated to allow works order backorder line
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sts_disp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_sts_disp/sts_disp.c,v 5.3 2002/07/17 09:58:11 scott Exp $";

#define	X_OFF	2
#define	Y_OFF	2

#define	SLEEP_TIME	2

#include <pslscr.h>
#include <pr_format3.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define LSL_PSIZE		12
#define PRINTER		 (local_rec.dp_flag [0] == 'P')
#define DSP_SCN		 (local_rec.dp_flag [0] == 'D')
#define	MULT_CUST	 (!strcmp (local_rec.cust_no,"      ") )

char	*mcg_line	=	"^^GGGGGGGGGJGGJGGJGGGGGGJGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGJGGGGGGGGGJGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGJGGGGGGGG"; 

char	*g_line	=	"^^GGGGGGGGGGJGGJGGJGGGGGGGGGGJGGGGGGGGGGJGGGGJGGGGGGGGGGGJGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGJGGGGGGGGGGG"; 

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct cumrRecord	cumr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;

	FILE	*fin,
			*fout;

	int		envDbCo 		= 0,
			envDbFind		= 0,
			nothingPrinted	= 1;

	char	branchNumber [3],
			displayString [300];

	char	pslipNo [9];

	float	CnvFct		=	0.00,
			StdCnvFct	=	0.00,
			dspQtyOrd	= 	0.00,
			dspQtyBord	= 	0.00;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	systemDate [11];
	long 	lsystemDate;
	long	st_date;
	long	ed_date;
	char 	cust_no [7];
	char 	cust_name [41];
	long	hhcu_hash;
	char 	dp_flag [2];
	char 	dp_flag_desc [10];
	char 	stat_flag [2];
	char 	statusDesc [20];
	char 	inpt_flag [2];
	int		printerNumber;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startdate",	 4, 22, EDATETYPE,
		"DD/DD/DDDD", "           ",
		" ", "00/00/0000", "Start Date. ", "Default to 00/00/0000 ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.st_date},
	{1, LIN, "enddate",	 5, 22, EDATETYPE,
		"DD/DD/DDDD", "           ",
		" ", " ", "End Date. ", "Default to End of Month ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ed_date},
	{1, LIN, "debtor",	 7, 22, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", "Customer No ", "Default is ALL ",
		YES, NO,  JUSTLEFT, "", "", local_rec.cust_no},
	{1, LIN, "name",	 7, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "              Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cust_name},

	{1, LIN, "stat_flag",	 9, 22, CHARTYPE,
		"U", "          ",
		" ", " ", "Order Status.", "R(eleased) P(Slip) M(anual) F(orward) B(ackOrd) C(redit) O(ver Mgn) H(eld) S(elected) G(Sched) W(order) - Default ALL" ,
		YES, NO,  JUSTLEFT, "BCFHMPRSGOW", "", local_rec.stat_flag},
	{1, LIN, "statusDesc",	 9, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "" ,
		NA, NO,  JUSTLEFT, "", "", local_rec.statusDesc},

	{1, LIN, "dpflag",	11, 22, CHARTYPE,
		"U", "          ",
		" ", "D", "D(isplay or P(rint.", " ",
		YES, NO,  JUSTLEFT, "DP", "", local_rec.dp_flag},
	{1, LIN, "dpflag_desc",	11, 25, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.dp_flag_desc},

	{1, LIN, "printerNumber",	12, 22, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No.  ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>

/*=======================
| Function Declarations |
=======================*/
int  	FindPS 			(long);
int  	check_page 		(void);
int  	heading 		(int);
int  	spec_valid 		(int);
void 	CloseDB 		(void);
void 	DisplayHeading 	(void);
void 	InitOutput 		(void);
void 	LocalDisplay 	(void);
void 	OpenDB 			(void);
void 	PrintHeading 	(void);
void 	PrintShow 		(void);
void 	ProcessSohr 	(void);
void 	ProcessSoln		(void);
void 	SetDefaults 	(void);
void 	shutdown_prog 	(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	if ((fin = pr_open ("so_sts_disp.p")) == NULL)
		sys_err ("Error in opening so_sts_disp.p during (FOPEN)",errno,PNAME);

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		crsr_on ();

		SetDefaults ();	/* set defaults for static structure */

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		/*============================
		| Process Orders in Database.|
		============================*/
		clear ();
		crsr_off ();
		fflush (stdout);
		InitOutput ();

		ProcessSohr ();

		if (PRINTER)
		{
			fprintf (fout,".EOF\n");
			pclose (fout);
		}
		abc_selfield (cumr, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
	}
	shutdown_prog ();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" 
							       						    : "cumr_id_no3");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
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
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (ccmr);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("enddate"))
	{
		if (dflt_used)
			local_rec.ed_date = MonthEnd (local_rec.lsystemDate);
		
		if (prog_status != ENTRY && 
				local_rec.st_date > local_rec.ed_date)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("startdate"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (prog_status != ENTRY && 
				local_rec.st_date > local_rec.ed_date)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("debtor"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.cust_no,"      ");
			sprintf (local_rec.cust_name,"%-40.40s","All Customers ");
			local_rec.hhcu_hash = 0L;
			DSP_FLD ("debtor");
			DSP_FLD ("name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			strcpy (local_rec.cust_name,cumr_rec.dbt_name);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.cust_no));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (local_rec.cust_name,cumr_rec.dbt_name);
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dpflag"))
	{
		switch (local_rec.dp_flag [0])
		{
		case	'D':
			strcpy (local_rec.dp_flag_desc,"Display");
			FLD ("printerNumber") = NA;
			local_rec.printerNumber = 0;
			DSP_FLD ("printerNumber");
			break;

		case	'P':
			strcpy (local_rec.dp_flag_desc,"Print  ");
			FLD ("printerNumber") = YES;
			break;
		}
		DSP_FLD ("dpflag_desc");
	}

	if (LCHECK ("stat_flag"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.stat_flag," ");
			strcpy (local_rec.statusDesc,"ALL                ");
			strcpy (local_rec.inpt_flag, " ");
			DSP_FLD ("statusDesc");
			return (EXIT_SUCCESS);
		}

		switch (local_rec.stat_flag [0])
		{
		case	'R':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Released Order"));
			break;

		case	'T':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Transport Ord."));
			break;

		case	'D':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Complete Order"));
			break;

		case	'S':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Selected"));
			break;

		case	'P':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Packing Slip"));
			break;

		case	'M':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Manual Release Order"));
			break;

		case	'H':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Held Order"));
			break;

		case	'F':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Forward Order"));
			break;

		case	'B':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Back Order"));
			break;

		case	'C':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Credit Release"));
			break;

		case	'O':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Over Margin"));
			break;

		case	'G':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Scheduled Order"));
			break;

		case	'W':
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Works Order P/S."));
			break;

		default:
			sprintf (local_rec.statusDesc,"%-19.19s",ML ("Unknown Status"));
			break;
		}
		local_rec.inpt_flag [0] = local_rec.stat_flag [0];
		DSP_FLD ("statusDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (FLD ("printerNumber") == NA)
			return (EXIT_SUCCESS);

		if (last_char == SEARCH)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
ProcessSohr (
 void)
{
	nothingPrinted = 1;

	if (!MULT_CUST	)
		sprintf (err_str, "Customer %s (%s)",cumr_rec.dbt_no, cumr_rec.dbt_name);
	else
		strcpy (err_str, ML ("Customer Order Status"));

	if (DSP_SCN)
	{
		Dsp_nc_prn_open (0,2,LSL_PSIZE,err_str,
					    comm_rec.co_no,comm_rec.co_name,
					    comm_rec.est_no,comm_rec.est_name,
					    (char *)0, (char *)0);

		if (MULT_CUST)
		{
			Dsp_saverec ("  ORDER  |BR|WH|CUST. | CUSTOMER  |  ORDER   |   DATE   | UOM|  ORDER  |   ITEM NUMBER   |       ITEM DESCRIPTION      |S| P/SLIP "); 
			Dsp_saverec (" NUMBER  |NO|NO|NUMBER| ACRONYM   |   DATE   |   DUE    |    |   QTY   |                 |                             | | NUMBER "); 
		}
		else
		{
			Dsp_saverec (" ORDER    |BR|WH|  ORDER   |   DATE   |UOM.| SUPPLY    | BACKORDER |  ITEM NUMBER   |       ITEM DESCRIPTION       |S|  PACKING  "); 
			Dsp_saverec (" NUMBER   |NO|NO|  DATE    |   DUE    |    | QUANTITY  | QUANTITY  |                |                              | |  SLIP NO  "); 
		}
		Dsp_saverec (" [REDRAW]  [PRINT]  [NEXT SCN] [PREV SCN]  [END/INPUT] ");
		PrintShow ();
	}
	else
		PrintHeading ();

	/*-----------------------------------------------
	| Change index on cumr to cumr_hhcu_hash	|
	-----------------------------------------------*/
	abc_selfield (cumr,"cumr_hhcu_hash");

	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	if (!strcmp (local_rec.cust_no,"      "))
		sohr_rec.hhcu_hash = 0L;
	else
		sohr_rec.hhcu_hash = local_rec.hhcu_hash;

	sprintf (sohr_rec.order_no,"%-8.8s"," ");
	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");

	while (!cc && !strcmp (sohr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (sohr_rec.br_no,comm_rec.est_no) && 
		      (sohr_rec.hhcu_hash == local_rec.hhcu_hash || 
		        local_rec.hhcu_hash == 0L) )
	{
		ProcessSoln ();

		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}

	if (!nothingPrinted && !PRINTER)
		Dsp_saverec ((MULT_CUST) ? mcg_line : g_line);

	if (DSP_SCN)
	{
		Dsp_srch ();
		Dsp_close ();
	}
}

void
ProcessSoln (
 void)
{
	char	star [2];
	char	str_ord [11];

	int		nstock 	= FALSE;
	int		pslip 	= FALSE;

	if (sohr_rec.status [0] == 'P' || sohr_rec.status [0] == 'S' ||
		sohr_rec.status [0] == 'D' || sohr_rec.status [0] == 'T')
		pslip = TRUE;

	strcpy (star, (sohr_rec.full_supply [0] == 'Y') ? "*" : " ");

	soln_rec.hhso_hash 	= sohr_rec.hhso_hash;
	inmr_rec.hhbr_hash 	= soln_rec.hhbr_hash;

	soln_rec.line_no = 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"r");

	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		inmr_rec.hhbr_hash 	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON,"r");
		if (cc || (inmr_rec.inmr_class [0] != 'Z' && 
			soln_rec.qty_order + soln_rec.qty_bord <= 0.00))
		{
			cc = find_rec (soln,&soln_rec,NEXT,"r");
			nstock = TRUE;
			continue;
		}
		if (inmr_rec.inmr_class [0] == 'Z' && nstock)
		{
			cc = find_rec (soln,&soln_rec,NEXT,"r");
			nstock = FALSE;
			continue;
		}
		nstock = FALSE;

		if (soln_rec.qty_order == 0.0 && soln_rec.qty_bord > 0.0 && soln_rec.status [0] == 'M')
			strcpy (soln_rec.status, "B");

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		StdCnvFct = inum_rec.cnv_fct;

		if (inum_rec.hhum_hash != soln_rec.hhum_hash)
		{
			inum_rec.hhum_hash = soln_rec.hhum_hash;  
			cc = find_rec (inum, &inum_rec, EQUAL, "r"); 
		}

		if (cc)
		{
			strcpy (inum_rec.uom, inmr_rec.sale_unit);
			inum_rec.cnv_fct = 1;
		}

		CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
		dspQtyOrd	= 	 (soln_rec.qty_order) / CnvFct;
		dspQtyBord	= 	 (soln_rec.qty_bord) / CnvFct;

		if (local_rec.st_date <= soln_rec.due_date && 
			soln_rec.due_date <= local_rec.ed_date && 
		        (soln_rec.status [0] == local_rec.inpt_flag [0] || 
			  local_rec.inpt_flag [0] == ' '))
		{
			if (nothingPrinted)
				nothingPrinted = 0;

			if (PRINTER)
				dsp_process ("Order No :",sohr_rec.order_no);
				
			cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;
			cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
			if (!cc)
			{
				inmr_rec.hhbr_hash = soln_rec.hhbr_hash;
				cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
			}
			ccmr_rec.hhcc_hash	=	soln_rec.hhcc_hash;
			cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");

			if (!cc)
			{
				if (soln_rec.status [0] == 'P' || 
					 soln_rec.status [0] == 'D' || 
					 soln_rec.status [0] == 'T' || 
					 soln_rec.status [0] == 'S' || pslip)
				{
					if (FindPS (soln_rec.hhsl_hash))
						sprintf (pslipNo, "%-8.8s", ML (mlStdMess272));
					else
						strcpy (pslipNo, cohr_rec.inv_no);
				}
				else
					sprintf (pslipNo, "%-8.8s", ML (mlStdMess272));


				if (PRINTER)
				{
					pr_format (fin,fout,"ORDER_LINE",1, star);
					pr_format (fin,fout,"ORDER_LINE",2, sohr_rec.order_no);
					pr_format (fin,fout,"ORDER_LINE",3, sohr_rec.br_no);
					pr_format (fin,fout,"ORDER_LINE",4, cumr_rec.dbt_no);
					pr_format (fin,fout,"ORDER_LINE",5, cumr_rec.dbt_name);
					pr_format (fin,fout,"ORDER_LINE",6, DateToString (sohr_rec.dt_raised));
					pr_format (fin,fout,"ORDER_LINE",7, DateToString (soln_rec.due_date));
					pr_format (fin,fout,"ORDER_LINE",8, inum_rec.uom);
					sprintf (str_ord, "%-10.10s", comma_fmt ((double)dspQtyOrd + dspQtyBord,"NNNNNN.NN"));
					pr_format (fin,fout,"ORDER_LINE",9, str_ord);
					pr_format (fin,fout,"ORDER_LINE",10, inmr_rec.item_no);
					pr_format (fin,fout,"ORDER_LINE",11, soln_rec.item_desc);
					pr_format (fin,fout,"ORDER_LINE",12, soln_rec.status);
					pr_format (fin,fout,"ORDER_LINE",13, pslipNo);

				}
				else
					LocalDisplay ();
			}
		}
		cc = find_rec (soln,&soln_rec,NEXT,"r");
	}
}

void
LocalDisplay (
 void)
{
	char	Date1 [11];
	char	Date2 [11];
	char	star [2];

	sprintf (Date1, "%-10.10s",DateToString (sohr_rec.dt_raised));
	sprintf (Date2, "%-10.10s",DateToString (soln_rec.due_date));

	strcpy (star, (sohr_rec.full_supply [0] == 'Y') ? "*" : " ");

	if (MULT_CUST)
	{
		sprintf (displayString,"%s%-8.8s^E%-2.2s^E%-2.2s^E%-6.6s^E %-9.9s ^E%-10.10s^E%-10.10s^E%s^E%9.2f^E %-16.16s^E%-29.29s^E%1.1s^E%-8.8s",
				star,
				sohr_rec.order_no , 
				ccmr_rec.est_no ,
				ccmr_rec.cc_no ,
				cumr_rec.dbt_no , 
				cumr_rec.dbt_acronym ,
				Date1, Date2,
				inum_rec.uom,
				dspQtyOrd + dspQtyBord,
				inmr_rec.item_no , 
				soln_rec.item_desc ,
				soln_rec.status, 
				pslipNo);
	}
	else
	{
		sprintf (displayString,"%s%-8.8s ^E%-2.2s^E%-2.2s^E%-10.10s^E%-10.10s^E%s^E %10.2f^E %10.2f^E%-16.16s^E%-30.30s^E%1.1s^E%-8.8s",
				star,
				sohr_rec.order_no , 
				ccmr_rec.est_no ,
				ccmr_rec.cc_no ,
				Date1,Date2,
				inum_rec.uom,
				dspQtyOrd,
				dspQtyBord,
				inmr_rec.item_no , 
				soln_rec.item_desc ,
				soln_rec.status, 
				pslipNo);
	}
	Dsp_saverec (displayString);
}

void
SetDefaults (
 void)
{
	local_rec.st_date = 0L;
	local_rec.ed_date = StringToDate ("31/12/2099");
	local_rec.hhcu_hash = 0L;
	strcpy (local_rec.cust_no,"      ");
	sprintf (local_rec.cust_name,"%-40.40s",ML ("All Customers"));
	strcpy (local_rec.dp_flag,"D");
	strcpy (local_rec.dp_flag_desc,"Display");
	local_rec.printerNumber = 1;
	strcpy (local_rec.stat_flag," ");
	strcpy (local_rec.statusDesc,"   ");
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();
		
		rv_pr (ML (mlSoMess257),42,0,1);
		line_at (1,0,132);

		box (0,3,132,9);

		line_at (6,1,131);
		line_at (8,1,131);
		line_at (10,1,131);
		line_at (20,0,132);

		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		line_at (22,0,132);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
{
	if (PRINTER)
	{
		dsp_screen (" Printing Order Status ",
					comm_rec.co_no,comm_rec.co_name);
		/*----------------------
		| Open pipe to pformat | 
 		----------------------*/
		if ((fout = popen ("pformat","w")) == NULL)
			sys_err ("Error in pformat During (POPEN)", errno,PNAME);
	
		/*---------------------------------
		| Initialize printer for output.  |
		---------------------------------*/
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout,".LP%d\n",local_rec.printerNumber);
		fprintf (fout,".11\n");	
		fprintf (fout,".PI16\n");
		fprintf (fout,".L180\n"); /*158*/
	}
	else  /*  DISPLAY  */
		DisplayHeading ();
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (
 void)
{
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
  	pr_format (fin,fout,"HEAD1",0,0);
	pr_format (fin,fout,"HEAD2",1,DateToString (local_rec.st_date));
	pr_format (fin,fout,"HEAD2",2,DateToString (local_rec.ed_date));
  	pr_format (fin,fout,"HEAD3",0,0);
  	pr_format (fin,fout,"LINE1",0,0);
  	pr_format (fin,fout,"HEAD4",0,0);
  	pr_format (fin,fout,"HEAD5",0,0);	
  	pr_format (fin,fout,"LINE2",0,0);
  	pr_format (fin,fout,"RULER",0,0);
}

/*======================================================
| DISPLAY SCREEN.                                      |
| Display Heading at screen after clearing the screen. |
======================================================*/
void
DisplayHeading (
 void)
{
	clear ();

 	rv_pr (ML (mlSoMess164),50,0,1); 
	if (MULT_CUST)
	{
		print_at (1,0, ML (mlStdMess112),DateToString (local_rec.st_date));
		print_at (1,105, ML (mlStdMess113),DateToString (local_rec.ed_date));
	}
	else
	{
 		print_at (1,0,ML (mlSoMess259),
				cumr_rec.dbt_no,
				clip (cumr_rec.dbt_name),
				DateToString (local_rec.st_date));

		print_at (1,105, ML (mlStdMess113),DateToString (local_rec.ed_date));
	}
}

/*=====================================================
| check if a new page is needed on screen or printer. |
=====================================================*/
int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

void
PrintShow (
 void)
{
	us_pr (ML (mlSoMess258),0,22,1);
}

int
FindPS (
 long hhsl_hash)
{
	/*-----------------------------------------
	| Find coln record for current hhsl_hash. |
	-----------------------------------------*/
	coln_rec.hhsl_hash = hhsl_hash;
 	cc = find_rec (coln,&coln_rec,COMPARISON,"r");
	if (cc)
		return (EXIT_FAILURE);

	/*-------------------------------------------------------------
	| Cohr record is already found so don't bother to find again. |
	-------------------------------------------------------------*/
	if (coln_rec.hhco_hash == cohr_rec.hhco_hash)
		return (EXIT_SUCCESS);

	cohr_rec.hhco_hash = coln_rec.hhco_hash;
 	return (find_rec (cohr,&cohr_rec,COMPARISON,"r"));
}
