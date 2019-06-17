/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Log: sk_st_sup.c,v $
| Revision 5.5  2002/07/17 09:58:01  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/01/09 01:16:58  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.3  2001/08/09 09:20:09  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:58  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:34  scott
| Update - LS10.5
|
|=====================================================================|
| $Id: sk_st_sup.c,v 5.5 2002/07/17 09:58:01 scott Exp $
|---------------------------------------------------------------------|
|  Date Written  : 28/03/89        | Author      : Roger Gibbison     |
|---------------------------------------------------------------------|
| $Log: sk_st_sup.c,v $
| Revision 5.5  2002/07/17 09:58:01  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/01/09 01:16:58  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.3  2001/08/09 09:20:09  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:58  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:34  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_st_sup.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_st_sup/sk_st_sup.c,v 5.5 2002/07/17 09:58:01 scott Exp $";

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<Costing.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>

#define		SERIAL		 (inmr_rec.serial_item [0] == 'Y')
#define		COST_SALES	 (costSales [0] == 'Y')
#define		COUNTED	 	 (stts_rec.counted [0] == 'Y')
#define		NEW_SER	 	 (stts_rec.status [0] == 'N')

#define		NO_CHANGE 	0.00
#define		ADDITION 	1.00
#define		SUBTRACTION -1.00

#define		LEAVE		0
#define		ZERO_COUNT	1
#define		ZERO_ALL	2

#define		ERR_SOLD	1
#define		ERR_DEL		2
#define		ADD_SER		3

	FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct sttsRecord	stts_rec;
struct intrRecord	intr_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;

static char	*data = "data";

extern	int	TruePosition;

	char	costSales [2],
			*serialSpace = "                         ",
			*envSkIvalClass,
 			*result,
			debitAccNo [sizeof glmrRec.acc_no],
			creditAccNo [sizeof glmrRec.acc_no];

	long	debitHhmrHash 	= 0L,
			creditHhmrHash 	= 0L;

	double	oldValue 		= 0.00,
			newValue 		= 0.00,
			totalVariance	= 0.00;

	float	totalOld 		= 0.00,
			totalNew 		= 0.00;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	locCurr [4];
	char	audit [6];
	int		printerNo;
	float	qty;
	char	location [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "stake_code",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Stock Selection Code         ", "",
		 NE, NO,  JUSTLEFT, "", "", inscRec.stake_code},
	{1, LIN, "stake_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Stock Selection Description  ", "",
		 NA, NO,  JUSTLEFT, "", "", inscRec.description},
	{1, LIN, "stake_date",	 6, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Start Date                   ", "",
		 NA, NO,  JUSTLEFT, "", "", (char *)&inscRec.start_date},
	{1, LIN, "stake_time",	 7, 2, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "Start Time                   ", "",
		 NA, NO,  JUSTLEFT, "", "", inscRec.start_time},
	{1, LIN, "printerNo",	 9, 2, INTTYPE,
		"NN", "          ",
		" ", "1","Printer Number               ", "",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{0, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
int  	spec_valid 			(int);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	GlDefaults 			(void);
void 	SrchInsc 			(char *);
void 	Update 				(void);
void 	ProcessStts 		(long);
void 	ProcessOther 		(void);
void 	OpenAudit 			(void);
void 	CloseAudit 			(void);
void 	AddGlwk 			(double);
int  	GetAccounts 		(char *);
void 	AddIntr 			(float, double);
void 	DeleteInsc 			(void);
void 	CancelSerial 		(long, long);
void 	LogError 			(int);
int  	heading 			(int);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	sprintf (costSales, "%-1.1s",get_env ("COST_SALES"));

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
	{	
		envSkIvalClass = p_strsave (sptr);
	}
	else
		envSkIvalClass = "ZKPN";
	upshift (envSkIvalClass); 

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();
	GlDefaults ();

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		edit_all ();
		if (restart)
			continue;

		Update ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| Validate Stock Take code. |
	---------------------------*/
	if (LCHECK ("stake_code"))
	{
		if (SRCH_KEY)
		{
			SrchInsc (temp_str);
			return (EXIT_SUCCESS);
		}

		inscRec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (insc,&inscRec,COMPARISON,"r");
		if (cc)
		{
			sprintf (err_str,ML (mlSkMess047),inscRec.stake_code);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (inscRec.serial_take [0] != 'Y')
		{
			sprintf (err_str, ML (mlSkMess048), inscRec.stake_code);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("stake_desc");
		DSP_FLD ("stake_date");
		DSP_FLD ("stake_time");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Printer Number. |
	--------------------------*/
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
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
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
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (stts, stts_list, STTS_NO_FIELDS, "stts_id_no");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhbr_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	OpenInsc ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (local_rec.locCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (local_rec.locCurr, "%-3.3s", comr_rec.base_curr);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (comr);
	abc_fclose (intr);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (stts);
	abc_fclose (coln);
	abc_fclose (soln);
	GL_CloseBatch (local_rec.printerNo);
	GL_Close ();
	abc_dbclose (data);
}

void
GlDefaults (void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	if (costSales [0] == 'Y')
	{
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			"STAKE VAR.",
			" ",
			" "
		);
	}
	abc_fclose (ccmr);
}

/*=============================================
| Search Routine for Stock take Control File. |
=============================================*/
void
SrchInsc (
 char *key_val)
{
	work_open ();
	save_rec ("# ","#Stock Take Selection Desc");
	inscRec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (inscRec.stake_code," ");
	cc = find_rec (insc,&inscRec,GTEQ,"r");
	while (!cc && inscRec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (inscRec.serial_take [0] == 'Y')
		{
			cc = save_rec (inscRec.stake_code, inscRec.description);
			if (cc)
				break;
		}

		cc = find_rec (insc,&inscRec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	inscRec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (inscRec.stake_code,temp_str);
	cc = find_rec (insc,&inscRec,COMPARISON,"r");
	if (cc)
		file_err (cc, insc, "DBFIND");
}

/*=======================
|	Update inmr / incc	|
=======================*/
void
Update (void)
{
	dsp_screen ("Updating Stock Take Figures for Serial items.",comm_rec.co_no,comm_rec.co_name);
	OpenAudit ();

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (incc_rec.sort, "                            ");
	
	cc = find_rec (incc,&incc_rec,GTEQ, "u");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash) 
	{
		if (incc_rec.stat_flag [0] != inscRec.stake_code [0])
		{
			abc_unlock (incc);
			cc = find_rec (incc,&incc_rec,NEXT, "u");
			continue;
		}
		cc = find_hash ("inmr",&inmr_rec,COMPARISON,"u",incc_rec.hhbr_hash);

		if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		{
			abc_unlock (inmr);
			abc_unlock (incc);
			cc = find_rec (incc,&incc_rec,NEXT, "u");
			continue;
		}

		if (!SERIAL || cc)
		{
			abc_unlock (inmr);
			abc_unlock (incc);
			cc = find_rec (incc,&incc_rec,NEXT, "u");
			continue;
		}
		dsp_process ("Item : ", inmr_rec.item_no);

		/*--------------------------------------
		| Process and Update Location records. |
		--------------------------------------*/
		ProcessStts (incc_rec.hhwh_hash);

		abc_unlock (incc);
		abc_unlock (inmr);
		cc = find_rec (incc,&incc_rec,NEXT, "u");
	}
	abc_unlock (incc);

	CloseAudit ();
	DeleteInsc ();

}

/*======================================================================
| Process transaction file and process into existing location records. |
======================================================================*/
void
ProcessStts (
 long hhwh_hash)
{
	/*--------------------------------------- 
	| Process new transaction (s) to update. |
	---------------------------------------*/
	stts_rec.hhwh_hash = hhwh_hash;
	strcpy (stts_rec.serial_no, serialSpace);
	cc = find_rec (stts, &stts_rec, GTEQ, "r");
	while (!cc && stts_rec.hhwh_hash == hhwh_hash)
	{
		/*--------------------------------------------------
		| Existing Serial counted , all OK so delete stts. |
		--------------------------------------------------*/
		if (!NEW_SER && COUNTED)
		{
			incc_rec.stake = 0.0;
			strcpy (incc_rec.stat_flag, "0");

			cc = abc_update ("incc", &incc_rec);
			if (cc)
				file_err (cc, "incc", "DBUPDATE");
				
			abc_delete (stts);

			stts_rec.hhwh_hash = hhwh_hash;
			strcpy (stts_rec.serial_no, serialSpace);
			cc = find_rec (stts, &stts_rec, GTEQ, "r");
			continue;
		}
		/*----------------------------------------------------
		| ProcessOther ()                                       | 
		|   i) Update inmr, ii) Update incc, iii) Add intr,  |
		|  iv) Add glwk,     v) Print Audit.                 |
		----------------------------------------------------*/
		ProcessOther ();

		abc_delete (stts);
		stts_rec.hhwh_hash = hhwh_hash;
		strcpy (stts_rec.serial_no, serialSpace);
		cc = find_rec (stts, &stts_rec, GTEQ, "r");
	}
	return;
}

/*====================================
| Main Processing Routine for Stock. |
====================================*/
void
ProcessOther (
 void)
{
	float	diff = 0.00,
		new_qty = 0.00,
		old_qty = 0.00;

	cc	=	FindInsf 
			(
				stts_rec.hhwh_hash, 
				0L, 
				stts_rec.serial_no, 
				stts_rec.status, 
				"u"
			);
	if (cc)
	{
		/*-------------------------------------------------
		| Error, Serial item sold since stock take began. |
		-------------------------------------------------*/
		if (!NEW_SER)	
		{
			LogError (ERR_SOLD);
			abc_unlock (insf);
			incc_rec.stake = 0.0;
			strcpy (incc_rec.stat_flag, "0");

			cc = abc_update ("incc", &incc_rec);
			if (cc)
				file_err (cc, "incc", "DBUPDATE");
				
			return;
		}
		/*-----------------------------------------------
		| New Serial item Found that was not in system. |
		-----------------------------------------------*/
		else
		{
			abc_unlock (insf);
			insfRec.hhwh_hash = stts_rec.hhwh_hash;
			insfRec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (insfRec.status, "F");
			strcpy (insfRec.serial_no, stts_rec.serial_no);
			insfRec.date_in = inscRec.start_date;
			strcpy (insfRec.location, stts_rec.location);
			insfRec.est_cost = stts_rec.cost;
			insfRec.act_cost = stts_rec.cost;
			strcpy (insfRec.stock_take, "Y");
			strcpy (insfRec.stat_flag, "0");
			cc = abc_add (insf, &insfRec);
			if (cc)
				file_err (cc, "insf", "DBUPDATE");
				
			oldValue = 0.00;
		}
	}
	else
	{
		oldValue	= 	FindInsfCost 
						(
							stts_rec.hhwh_hash, 
							0L,
							stts_rec.serial_no, 
							stts_rec.status
						);
	}
	
	new_qty = (float) ((COUNTED)    ? 1.00 : 0.00);
	old_qty = (float) ((NEW_SER) ? 0.00 : 1.00);
	diff = new_qty - old_qty;

	/*---------------------------------
	| Update figures for inmr & incc. |
	---------------------------------*/
	inmr_rec.on_hand += diff;
	
	incc_rec.adj  += diff;
	incc_rec.ytd_adj += diff;
	incc_rec.closing_stock = (incc_rec.opening_stock + 
			      	      	  incc_rec.pur + 
							  incc_rec.receipts - 
							  incc_rec.issues - 
							  incc_rec.sales + 
							  incc_rec.adj);


	if (!NEW_SER && !COUNTED)
		newValue = 0.00;
	else
		newValue = stts_rec.cost;

	/*------------------------------------------------
	| Cost of sales is interfaced to general ledger. |
	------------------------------------------------*/
	if (COST_SALES)
		GetAccounts (inmr_rec.category);

	/*----------------------------
	| Add inventory transaction. |
	----------------------------*/
	AddIntr (diff, newValue - oldValue);

	/*---------------------------------
	| Add General Ledger Transaction. |
	---------------------------------*/
	AddGlwk (newValue - oldValue);

	cc = abc_update ("inmr", &inmr_rec);
	if (cc)
		file_err (cc, "inmr", "DBUPDATE");
	
	incc_rec.stake = 0.0;
	strcpy (incc_rec.stat_flag, "0");

	cc = abc_update ("incc", &incc_rec);
	if (cc)
		file_err (cc, "incc", "DBUPDATE");
	
	/*--------------------------------------
	| Log the fact that serial item added. |
	--------------------------------------*/
	if (NEW_SER)
		LogError (ADD_SER);

	/*--------------------------------------------------
	| Log the fact that serial not counted so deleted. |
	--------------------------------------------------*/
	if (!NEW_SER && !COUNTED)
	{
		LogError (ERR_DEL);
		/*-----------------------------------
		| Cancel serial items of coln/soln. |
		-----------------------------------*/
		CancelSerial (incc_rec.hhbr_hash,incc_rec.hhcc_hash);

		strcpy (insfRec.status, "S");
		cc = abc_update (insf, &insfRec);
		if (cc)
			file_err (cc, "insf", "DBUPDATE");
	}
	abc_unlock ("incc");
}

/*===============================================================
|	Routine to open output pipe to standard print to			|
|	provide an audit trail of events.							|
|	This also sends the output straight to the spooler.			|
===============================================================*/
void
OpenAudit (void)
{
	if ((fout = popen ("pformat","w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".SO\n");
	fprintf (fout,".LP%d\n",local_rec.printerNo);
	fprintf (fout,".10\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".ESTOCK TAKE UPDATE AUDIT REPORT\n");
	fprintf (fout,".ESELECTION [%s] STARTED : %s : %s (%s)\n",
			inscRec.stake_code,
			DateToString (inscRec.start_date),
			inscRec.start_time,
			clip (inscRec.description));

	fprintf (fout,".E%s AS AT %s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (fout,".B2\n");
	fprintf (fout,".EBr %s : W/H %s \n",clip (comm_rec.est_name),clip (comm_rec.cc_name));

	fprintf (fout, ".R================================");
	fprintf (fout, "=============================");
	fprintf (fout, "==========================");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, (COST_SALES) ? "===================================\n" : "=\n");

	fprintf (fout, "================================");
	fprintf (fout, "=============================");
	fprintf (fout, "==========================");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, (COST_SALES) ? "===================================\n" : "=\n");

	fprintf (fout, "|   GROUP    |   ITEM  NUMBER   ");
	fprintf (fout, "|     ITEM DESCRIPTION       ");
	fprintf (fout, "|      SERIAL NUMBER      ");
	fprintf (fout, "|  OLD  VALUE  ");
	fprintf (fout, "|  NEW  VALUE  ");
	fprintf (fout, (COST_SALES) ? "|      DEBIT     |      CREDIT    |\n" : "|\n");

	fprintf (fout, "|------------|------------------");
	fprintf (fout, "|----------------------------");
	fprintf (fout, "|-------------------------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------");
	fprintf (fout, (COST_SALES) ? "|----------------|----------------|\n" : "|\n");

	fprintf (fout, ".PI12\n");
}

/*=======================================================
|	Routine to close the audit trail output file.	|
=======================================================*/
void
CloseAudit (void)
{
	fprintf (fout, "|============|==================");
	fprintf (fout, "|============================");
	fprintf (fout, "|=========================");
	fprintf (fout, "|==============");
	fprintf (fout, "|==============");
	fprintf (fout, (COST_SALES) ? "|================|================|\n" : "|\n");

	fprintf (fout, "|   TOTAL    |                  ");
	fprintf (fout, "|                            ");
	fprintf (fout, "|                         ");
	fprintf (fout, "|%13.2f ", totalOld);
	fprintf (fout, "|%13.2f ", totalNew);
	fprintf (fout, (COST_SALES) ? "|                |                |\n" : "|\n");

	fprintf (fout,".EOF\n");
	pclose (fout);
}

/*================================
| Add transactions to glwk file. |
================================*/
void
AddGlwk (
 double upd_val)
{
	int	reverse,
		monthNum;

	double	workValue = 0.00;

	reverse = FALSE;
 	workValue = out_cost (upd_val, inmr_rec.outer_size);
	if (workValue < 0.00)
	{
		reverse = TRUE;
		workValue *= -1;
	}
	
	if (COST_SALES)
	{
		strcpy (glwkRec.co_no,comm_rec.co_no);
		strcpy (glwkRec.tran_type,"14");
		glwkRec.post_date = comm_rec.inv_date;
		glwkRec.tran_date = comm_rec.inv_date;

		DateToDMY (comm_rec.inv_date, NULL, &monthNum, NULL);

		sprintf (glwkRec.period_no, "%02d", monthNum);
		sprintf (glwkRec.sys_ref,"%5.1d",comm_rec.term);
		sprintf (glwkRec.user_ref,"%8.8s","STAKE.");
		strcpy (glwkRec.stat_flag,"2");
		sprintf (glwkRec.narrative,"Stock Take : %s / %s", comm_rec.est_no, comm_rec.cc_no);

		strcpy (glwkRec.alt_desc1, " ");
		strcpy (glwkRec.alt_desc2, " ");
		strcpy (glwkRec.alt_desc3, " ");
		strcpy (glwkRec.batch_no,  " ");
		glwkRec.amount = CENTS (workValue);

		sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,debitAccNo);
		glwkRec.hhgl_hash = debitHhmrHash;

		strcpy (glwkRec.jnl_type, (reverse) ? "1" : "2");
		glwkRec.loc_amount = glwkRec.amount;
		glwkRec.exch_rate = 1.00;
		strcpy (glwkRec.currency, local_rec.locCurr);
		GL_AddBatch ();

		sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,creditAccNo);
		glwkRec.hhgl_hash = creditHhmrHash;
		strcpy (glwkRec.jnl_type, (reverse) ? "2" : "1");
		glwkRec.loc_amount = glwkRec.amount;
		glwkRec.exch_rate = 1.00;
		strcpy (glwkRec.currency, local_rec.locCurr);
		GL_AddBatch ();
	}

	fprintf (fout, "|%s%s",inmr_rec.inmr_class,inmr_rec.category);
   	fprintf (fout, "| %s ",inmr_rec.item_no);
	fprintf (fout, "|%-28.28s",inmr_rec.description);
	fprintf (fout, "|%-25.25s",stts_rec.serial_no);
	fprintf (fout, "|%13.2f ",oldValue);
	fprintf (fout, "|%13.2f ",newValue);
	if (COST_SALES)
	{
		fprintf (fout, "|%-16.16s", (reverse) ? debitAccNo : creditAccNo);
		fprintf (fout, "|%-16.16s", (reverse) ? creditAccNo : debitAccNo);
	}
	fprintf (fout, "|\n");

	totalOld += (float) (oldValue);
	totalNew += (float) (newValue);
	totalVariance += workValue;
}

/*=======================
| Get Control Accounts. |
=======================*/
int
GetAccounts  (
	char	*category)
{
	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"STAKE VAR.",
		" ",
		category
	);
	strcpy (debitAccNo, glmrRec.acc_no);
	debitHhmrHash = glmrRec.hhmr_hash;

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"COSTSALE C",
		" ",
		category
	);
	strcpy (creditAccNo, glmrRec.acc_no);
	creditHhmrHash = glmrRec.hhmr_hash;

	return (EXIT_SUCCESS);
}
/*================================
| Creeate Inventory Transaction. |
================================*/
void
AddIntr (
	float 	updateQty, 
	double 	updateValue)
{
	double	workValue = 0.00;

 	workValue = out_cost (updateValue, inmr_rec.outer_size);

	strcpy (intr_rec.co_no, comm_rec.co_no);
	strcpy (intr_rec.br_no, comm_rec.est_no);
	intr_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	intr_rec.hhcc_hash 	= incc_rec.hhcc_hash;
	intr_rec.hhum_hash 	= inmr_rec.std_uom;
	intr_rec.type 		= 4;
	intr_rec.date 		= comm_rec.inv_date;
	intr_rec.qty 		= updateQty;
	intr_rec.cost_price = CENTS (workValue / (double) updateQty);
	intr_rec.sale_price = 0.00;
	strcpy (intr_rec.batch_no, "STAKE");
	sprintf (intr_rec.ref1, "ST SEL [%s]",inscRec.stake_code);
	strcpy (intr_rec.ref2, DateToString (inscRec.start_date));
	strcpy (intr_rec.stat_flag, "0");

	cc = abc_add (intr, &intr_rec);
	if (cc)
		file_err (cc, intr, "DBADD");
}

/*=================================
| Delete Stock Take Control File. |
=================================*/
void
DeleteInsc (void)
{
	inscRec.hhcc_hash = ccmr_rec.hhcc_hash;
	cc = find_rec (insc,&inscRec,COMPARISON,"r");
	if (cc)
		file_err (cc, insc, "DBFIND");

	abc_delete (insc);
}

/*==============================================
| Cancel serial items from from coln and soln. |
==============================================*/
void
CancelSerial (
	long	hhbrHash, 
	long	hhccHash)
{
	/*------------------------------------------------
	| Find Serial item for All invoice transactions. |
	------------------------------------------------*/
	coln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
	while (!cc && coln_rec.hhbr_hash == hhbrHash)
	{
		if (!strcmp (coln_rec.serial_no,insfRec.serial_no) && 
		    coln_rec.incc_hash == hhccHash)
		{
			strcpy (coln_rec.serial_no, serialSpace);
			cc = abc_update (coln, &coln_rec);
			if (cc)
				file_err (cc, "coln", "DBFIND");
				
			break;
		}
		else
			abc_unlock (coln);
		cc = find_rec (coln, &coln_rec, NEXT, "u");
	}
	abc_unlock (coln);

	/*----------------------------------------------------
	| Find Serial item for All sales order transactions. |
	----------------------------------------------------*/
	soln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhbr_hash == hhbrHash)
	{
		if (!strcmp (soln_rec.serial_no,insfRec.serial_no) && 
		    soln_rec.hhcc_hash == hhccHash)
		{
			strcpy (soln_rec.serial_no, serialSpace);
			cc = abc_update (soln, &soln_rec);
			if (cc)
				file_err (cc, soln, "DBFIND");
			break;
		}
		else
			abc_unlock (soln);
		cc = find_rec (soln, &soln_rec, NEXT, "u");
	}
	abc_unlock (soln);
	return;
}

void		
LogError (
	int 	errorType)
{
	switch (errorType)
	{
	    case ERR_SOLD :
		fprintf (fout, "|%s%s",inmr_rec.inmr_class,inmr_rec.category);
   		fprintf (fout, "| %s ",inmr_rec.item_no);
		fprintf (fout, "|%-25.25s",inmr_rec.description);
		fprintf (fout, "|%-25.25s",stts_rec.serial_no);
		fprintf (fout, "|%s", ML ("SERIAL ITEM NOT COUNTED AND SINCE SOLD."));
		if (COST_SALES)
		{
			fprintf (fout, "|                ");
			fprintf (fout, "|                ");
		}
		fprintf (fout, "|\n");
		
		break;

	    case ERR_DEL :
		strcpy (err_str, ML ("EXISTING SERIAL ITEM NOT COUNTED. THE ITEM HAS BEEN REMOVED FROM THE SERIAL FILE.   "));

		fprintf (fout, "|%7.7s---->", ML ("NOTE   "));
   		fprintf (fout, "|                  ");
		fprintf (fout, "|%s", err_str);
		if (COST_SALES)
		{
			fprintf (fout, "|                ");
			fprintf (fout, "|                ");
		}
		fprintf (fout, "|\n");
		break;

	    case ADD_SER :
		fprintf (fout, "|%7.7s---->", ML ("NOTE   "));
   		fprintf (fout, "|                  ");

		strcpy (err_str, ML ("THIS SERIAL ITEM WAS FOUND AND AS IT DID NOT EXIST IN THE SERIAL FILE IT WAS ADDED. "));


		fprintf (fout, "|%s", err_str);
		if (COST_SALES)
		{
			fprintf (fout, "|                ");
			fprintf (fout, "|                ");
		}
		fprintf (fout, "|\n");

		break;

		default :
			break;
	}
	fprintf (fout, "|------------|------------------");
	fprintf (fout, "|----------------------------");
	fprintf (fout, "|-------------------------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------");
	fprintf (fout, (COST_SALES) ? "|----------------|----------------|\n" : "|\n");
}

int
heading (
	int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlSkMess043),31,0,1);
		line_at (1,0,80);

		box (0,3,80,6);
		line_at (8,1,79);
		line_at (19,0,80);
		print_at (20,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (21,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		print_at (22,0,ML (mlStdMess099),comm_rec.cc_no,comm_rec.cc_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
