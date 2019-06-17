/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: cm_stk_iss.c,v 5.5 2002/11/28 04:09:46 scott Exp $
|  Program Name  : (cm_stk_iss.c)
|  Program Desc  : (Stock Issue to Contract (G/L Interface)) 
|---------------------------------------------------------------------|
|  Date Written  : (03/03/93)      | Author       : Simon Dubey.      |
|---------------------------------------------------------------------|
| $Log: cm_stk_iss.c,v $
| Revision 5.5  2002/11/28 04:09:46  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.4  2002/07/24 08:38:43  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.3  2002/07/03 04:21:42  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 08:57:46  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:28  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:02:34  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/23 10:42:49  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to remove usage of old include files.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "cm_stk_iss";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_stk_iss/cm_stk_iss.c,v 5.5 2002/11/28 04:09:46 scott Exp $";

#define	MAXWIDTH 300
#define	MAXLINES 300
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>
#include <GlUtils.h>
#include <Costing.h>
#include <twodec.h>
#include <LocHeader.h>

#define	LCL_TAB_ROW 4
#define	LCL_TAB_COL 5
#define TAB_SIZE 106

#define TAB_1  (LCL_TAB_COL + 4)
#define TAB_2  (TAB_1 + 17)
#define TAB_3  (TAB_2 + 11)
#define TAB_4  (TAB_3 + 8)
#define TAB_5  (TAB_4 + 26) 
#define TAB_6  (TAB_5 + 5) 
#define TAB_7  (TAB_6 + 11)
#define TAB_8  (TAB_7 + 11)
#define TAB_9  (TAB_8 + 2)

#define	COMPANY	2
#define	BRANCH	1
#define	USER	0

#define	SR		store [line_cnt]

#define	SERIAL		 (inmr_rec.costing_flag [0] == 'S') 
#define sixteen_space "                "

#define		FGN_CURR    (MCURR && strcmp (cumr_rec.curr_code, loc_curr))

	FILE	*pp;

	int		printerNo = 1;

	char	cbranchNo [3];
	int		MCURR          = FALSE;

	char	*ser_space = "                         ";

	int		PRIME	 = FALSE;
	int		by_what;
	int		first_line = TRUE;
	int		con_price = FALSE;

	int 	sub_cat;

	long	iss_hash = 0L;
	char	iss_acc [sizeof glwkRec.acc_no];
	long	rec_hash = 0L;
	char	rec_acc [sizeof glwkRec.acc_no];

	double	batch_total = 0;
	float	qty_total = 0;
	float	regPc 		= 0.0;

	struct	storeRec {
		char	cost_flag [2];
		char	ser_flag [2];
		long	hhwh_hash;
		long	hhcc_hash;
		long	hhbr_hash;
		long	hhhr_hash;
		long	hhcm_hash;
		long	_hhum_hash;
		float	_cnv_fct;
		char	dbt_acc [sizeof glwkRec.acc_no];
		long	dbt_hash;
		char	crd_acc [sizeof glwkRec.acc_no];
		long	crd_hash;
		double	sale_price;
		double	closing;
		double	avail;
		float	gst_pc;
		float	tax_pc;
		float	disc_pc;
		char	_UOM [5];
		char	_lot_ctrl [2];
		int		_lot_select_flag;
		int		_dec_pt;
	} store [MAXLINES];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cmcdRecord	cmcd_rec;
struct cmcmRecord	cmcm_rec;
struct cmcmRecord	cmcm2_rec;
struct cmhrRecord	cmhr_rec;
struct cumrRecord	cumr_rec;
struct cmtrRecord	cmtr_rec;
struct cmcbRecord	cmcb_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;

char	*cmcm2	= "cmcm2",
		*data	= "data",
		*inum2 	= "inum2";

	char	 loc_curr [4];

int		first_time = TRUE;

#include	<MoveRec.h>
#include	<MoveAdd.h>

/*=========================== 
| Local & Screen Structures.|
===========================*/
struct {
	char	dummy [11];
	char	wh_no [3];
	char	systemDate [11];
	long	lsystemDate;
	char	prev_item [17];
	char 	j_ref [sizeof glwkRec.user_ref];
	long 	j_date;
	float	wk_qty;
	double	wk_cost;
	char	item_no [17];
	long	hhwh_hash;
	long	rec_date;
	char	last_cont [7];
	char	last_costhd [5];
	char	last_wh [3];
	char	r_date [11];
	char	d_date [11];
	char	ser_no [26];
	char	location [11];
	char	item_desc [41];
	char	dflt_ser_no [26];
	char	UOM [5];
	char	lot_ctrl [2];
	char	LL [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "ref",	 4, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Journal/Batch ref.", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.j_ref},
	{1, LIN, "jdate",	 5, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.r_date, "Journal date    ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.j_date},
	{2, TAB, "wh_no",	MAXLINES, 0, CHARTYPE,
		"NN", "          ",
		" ", " ", "WH.", " ",
		YES, NO,  JUSTRIGHT, "1234567890", "", ccmr_rec.cc_no},
	{2, TAB, "item_no",	0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Item number.  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{2, TAB, "cont_no",	 15, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Cont No. ", "Enter Contract Number, Default = Last Contract Number Entered, Search Available. ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.cont_no},
	{2, TAB, "costhd",	 0, 1, CHARTYPE,
		"UUUU", "          ",
		" ", " ",  " Csthd ", "Enter Costhead Code, Default = Last Costhead Entered, Search Available ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.ch_code},
	{2, TAB, "ser_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "       Reference.        ", "Please note that only 1st 10 character are stored in stock transaction file. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.ser_no},
	{2, TAB, "UOM",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", "", "UOM.", " Unit of Measure ",
		 YES, NO, JUSTLEFT, "", "", local_rec.UOM},
	{2, TAB, "qty",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "1.00", " Quantity ", " ",
		YES, NO, JUSTRIGHT, "0.00", "9999999.99", (char *) &local_rec.wk_qty},
	{2, TAB, "cost",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "", " Est Cost ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.wk_cost},
	{2, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{2, TAB, "r_date",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.d_date, "   Date.  ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.rec_date},
	{2, TAB, "cont_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", "",
		 ND, NO,  JUSTLEFT, "", "", cmcd_rec.text},
	{2, TAB, "csthd_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", "",
		 ND, NO,  JUSTLEFT, "", "", cmcm_rec.desc},
	{2, TAB, "desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "            Item Description.           ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<cus_price.h>
#include	<cus_disc.h>

/*===========================
| Local function prototypes |
===========================*/
void		shutdown_prog	 (void);
void		OpenDB			 (void);
void		CloseDB		 (void);
int			spec_valid		 (int field);
int			validDate		 (long inp_date);
void		update			 (void);
void		add_glwk		 (void);
void		prnt_details	 (void);
void		open_audit		 (void);
int			findincc		 (void);
void		close_audit		 (void);
int			ck_dup_ser		 (char *ser, long hhbr_hash, int line_no);
int			get_accs		 (char *cat_no, int ln_pos);
int			heading			 (int scn);
void		tab_other		 (int line_no);
void		update_tab		 (int line_no);
void		SrchCmcm		 (char *key_val);
void		SrchCmcb		 (void);
void		SrchCmhr		 (char *key_val);
void		find_desc		 (void);
void		add_new_cmcbs	 (void);
int			not_in_cont		 (void);
void		show_wh_no		 (char *key_val);
void		add_cmtr		 (void);
void		PriceProcess	 (void);
void		DiscProcess		 (void);
int			find_cost		 (void);
void		calc_avail		 (void);
void		srch_uom		 (char *key_val);
float		ToStdUom		 (float lclQty);
float		ToLclUom		 (float lclQty);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr = chk_env ("CM_AUTO_CON");

	if (sptr)
		by_what = atoi (sptr);
	else
		by_what = COMPANY;

	if (argc < 2)	
	{
		print_at (0,0, mlCmMess748, argv [0]);
		return (EXIT_FAILURE);
	}

	tab_row = LCL_TAB_ROW;
	tab_col = LCL_TAB_COL;

	sptr = chk_env ("DIS_FIND");
	if (sptr == (char *)0)
		sub_cat = 11;
	else
		sub_cat = atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	MCURR = (sptr == (char *)0) ? TRUE : atoi (sptr);

	printerNo = atoi (argv [1]);

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	OpenDB ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	GL_SetMask (GlFormat);
	OpenPrice ();
	OpenDisc ();

	strcpy (cbranchNo, (by_what != COMPANY) ? comm_rec.est_no : " 0");

	strcpy (local_rec.r_date, DateToString (comm_rec.inv_date));
	strcpy (local_rec.systemDate,DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	while (!prog_exit) 
	{
		/*  reset control flags  */

		entry_exit = 0;
		restart = 0;
		edit_exit = 0;
		prog_exit = 0;
		search_ok = 1;

		init_vars (1);	
		init_vars (2);	
		lcount [2] = 0;
		first_line = TRUE;

		sprintf (local_rec.last_cont, "%-6.6s", " ");
		sprintf (local_rec.last_costhd, "%-4.4s", " ");
		sprintf (local_rec.last_wh, "%-2.2s", " ");
		abc_selfield (cmhr, "cmhr_id_no2");

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (prog_exit || restart)
			continue;

		heading (2);
		entry (2);
		if (restart)
			continue;

		edit_all ();

		if (restart)
			continue;

		if (first_time)
		{
			open_audit ();
			first_time = FALSE;
		}

		update ();

		abc_unlock (inmr);
		abc_unlock (inei);
		abc_unlock (incc);
		abc_unlock (incf);
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
	if (!first_time)
		close_audit ();

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

	MoveOpen	=	TRUE;
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (loc_curr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	abc_alias (cmcm2, cmcm);
	abc_alias (inum2, inum);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");

	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmcm, cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmcm2,cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_hhhr_hash");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmcb, cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
    open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
    open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");

	/*-----------------------------------------
	| Read ccmr record for current warehouse. |
	-----------------------------------------*/
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	OpenLocation (ccmr_rec.hhcc_hash);

	OpenGlmr ();
	OpenGlwk ();
	OpenInei ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (cmhr);
	abc_fclose (cmcm);
	abc_fclose (cmcm2);
	abc_fclose (cmtr);
	abc_fclose (cmcd);
	abc_fclose (cmcb);
	SearchFindClose ();

	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inwu);
	abc_fclose (ccmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose ("move");
	CloseLocation ();

	GL_CloseBatch (printerNo);
	CloseCosting ();
	GL_Close ();

	ClosePrice ();
	CloseDisc ();

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int i;
	int	new_costhd = FALSE;
	int	TempLine;
	int	TempLineCnt;

	if (LCHECK ("wh_no"))
	{
		if (dflt_used)
		{
			if (first_line)
				return (EXIT_FAILURE);
			
			strcpy (ccmr_rec.cc_no, local_rec.last_wh);
		}

		if (SRCH_KEY)
		{
			show_wh_no (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		strcpy (ccmr_rec.est_no,comm_rec.est_no);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SR.hhcc_hash = ccmr_rec.hhcc_hash;

		ReadLLCT (SR.hhcc_hash);

		if (llctInput [0] == 'V')
			SR._lot_select_flag	=	INP_VIEW;
		if (llctInput [0] == 'A')
			SR._lot_select_flag	=	INP_AUTO;
		if (llctInput [0] == 'M')
		{
			strcpy (StockTake, "Y");
			SR._lot_select_flag	=	INP_VIEW;
		}

		strcpy (local_rec.last_wh, ccmr_rec.cc_no);
		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("costhd"))
	{
		if (dflt_used)
		{
			if (first_line)
				return (EXIT_FAILURE);
			
			SR.hhcm_hash = cmcm_rec.hhcm_hash;
			strcpy (cmcm_rec.ch_code, local_rec.last_costhd);
		}

		if (SRCH_KEY)
		{
			if (search_key == FN9)
			{
				SrchCmcm (temp_str);
				return (EXIT_SUCCESS);
			}
			else
			{
				SrchCmcb ();
				return (EXIT_SUCCESS);
			}
		}

		strcpy (cmcm_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");

		while (cc)
		{
			i= prmptmsg (ML (mlCmMess016), "YNyn", 1, 2);

			if (i == 'n' || i == 'N')
			{
				print_at (2,1, "%-100.100s", " ");
				return (EXIT_FAILURE);
			}

			i = line_cnt;
			putval (line_cnt);

			new_costhd = TRUE;
			* (arg) = "cm_costhd";
			* (arg+ (1)) = (char *)0;
			shell_prog (2);
			heading (2);

			line_cnt = i;
			if (prog_status == ENTRY)
				lcount [2] = line_cnt;

			scn_display (2);
			getval (line_cnt);
			line_display ();

			cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");
			if (!cc)
				add_new_cmcbs ();
		}

		if (!new_costhd && not_in_cont ())
		{
			sprintf (err_str,ML (mlCmMess016));

			i = prmptmsg (err_str, "YNyn", 1, 2);

			print_at (2,1, "%-100.100s", " ");

			if (i == 'N' || i == 'n')
				return (EXIT_FAILURE);
			
			add_new_cmcbs ();
		}

		SR.hhcm_hash = cmcm_rec.hhcm_hash;
		strcpy (local_rec.last_costhd, cmcm_rec.ch_code);
		first_line = FALSE;
		tab_other (line_cnt);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cont_no"))
	{
		if (dflt_used)
		{
			if (first_line)
				return (EXIT_FAILURE);
			
			SR.hhhr_hash = cmhr_rec.hhhr_hash;
			strcpy (cmhr_rec.cont_no, local_rec.last_cont);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		pad_num (cmhr_rec.cont_no);
		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, cbranchNo);
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");

		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cmhr_rec.status [0] != 'O')
		{
			print_mess (ML (mlCmMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SR.hhhr_hash = cmhr_rec.hhhr_hash;
		find_desc ();
		strcpy (local_rec.last_cont, cmhr_rec.cont_no);
		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}
	/*---------------
	| Batch number. |
	---------------*/
	if (LCHECK ("ref"))
	{
		sprintf (local_rec.j_ref,"%10.10s",zero_pad (local_rec.j_ref,10));
		DSP_FLD ("ref");
		return (EXIT_SUCCESS);
	}
	/*---------------
	| Journal Date. |
	---------------*/
	if (LCHECK ("jdate"))
		return (validDate (local_rec.j_date));

	/*---------------
	| Journal Date. |
	---------------*/
	if (LCHECK ("r_date"))
	{
		return (validDate (local_rec.rec_date));
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{

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
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.item_no, inmr_rec.item_no);

		DSP_FLD ("item_no");

		if (get_accs (inmr_rec.category, line_cnt))
			return (EXIT_FAILURE);

		/*------------------------------------------
		| Look up to see if item is on Cost Centre |
		------------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		if (findincc ())
		{
			clear_mess ();
			sprintf (err_str, ML (mlStdMess192), inmr_rec.item_no);
			errmess (err_str);
	    		sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SR.hhbr_hash = incc_rec.hhbr_hash;
		strcpy (local_rec.item_desc, inmr_rec.description);

		strcpy (SR.cost_flag, inmr_rec.costing_flag);
		strcpy (SR.ser_flag,  inmr_rec.serial_item);

		ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (ineiRec.est_no,comm_rec.est_no);
		cc = find_rec (inei, &ineiRec, COMPARISON, "r");
		if (cc) 
		{
		    clear_mess ();
		    sprintf (err_str, ML (mlCmMess180), inmr_rec.item_no);
		    errmess (err_str);
	    	    sleep (sleepTime);
		    clear_mess ();
		    return (EXIT_FAILURE);
		}

		strcpy (SR._lot_ctrl		, inmr_rec.lot_ctrl);
		SR._dec_pt	=	inmr_rec.dec_pt;
		strcpy (local_rec.lot_ctrl	, inmr_rec.lot_ctrl);
		strcpy (local_rec.UOM, inmr_rec.sale_unit);

    	/*---------------------
    	| Find for UOM GROUP. |
    	----------------------*/
    	strcpy (inum_rec.uom, inmr_rec.sale_unit);
    	cc = find_rec (inum, &inum_rec, EQUAL, "r");
    	if (cc)
        	file_err (cc, inum, "DBFIND");

    	SR._hhum_hash   = inum_rec.hhum_hash;
    	SR._cnv_fct     = inum_rec.cnv_fct;

		if (prog_status != ENTRY)
				update_tab (line_cnt);

		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Check If Serial No already exists |
	-----------------------------------*/
	if (LCHECK ("ser_no"))
	{
		if (last_char == EOI && prog_status == ENTRY)
		{
			skip_entry = -2;
			return (EXIT_SUCCESS);
		}
		
		if (end_input)
			return (EXIT_SUCCESS);

		if (SR.ser_flag [0] != 'Y')
		{
			if (dflt_used)
				strcpy (local_rec.ser_no, local_rec.dflt_ser_no);
			strcpy (local_rec.dflt_ser_no, local_rec.ser_no);
			DSP_FLD ("ser_no");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SearchInsf (local_rec.hhwh_hash, "F",temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (FindInsf (local_rec.hhwh_hash, 0L, local_rec.ser_no,"F","r"))
		{
			errmess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.wk_cost = DOLLARS (SerialValue (insfRec.est_cost,
					       insfRec.act_cost));
		local_rec.wk_qty = 1.00;
		DSP_FLD ("qty");
		DSP_FLD ("cost");
		skip_entry = goto_field (field, label ("r_date"));
		if (ck_dup_ser (local_rec.ser_no,SR.hhbr_hash,line_cnt))
		{
			print_mess (ML (mlCmMess029));
			/*print_mess (" Duplicate Serial Number ");*/
	    	    	sleep (sleepTime);
		    	clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Unit of Measure. | 
	---------------------------*/
	if (LCHECK ("UOM"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.UOM, inum_rec.uom);
			strcpy (SR._UOM, inum_rec.uom);
			SR._hhum_hash = inum_rec.hhum_hash;
		}

		if (SRCH_KEY)
		{
			srch_uom (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		strcpy (inum2_rec.uom, local_rec.UOM);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	
		if (ValidItemUom (SR.hhbr_hash, inum2_rec.hhum_hash))
		{
			sprintf (err_str, "Invalid Unit of Measure for Item.");
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.UOM, inum2_rec.uom);
		strcpy (SR._UOM, inum2_rec.uom);
		SR._hhum_hash 	= inum2_rec.hhum_hash;

		if (inum_rec.cnv_fct == 0.00)
			inum_rec.cnv_fct = 1.00;

		SR._cnv_fct 	= inum2_rec.cnv_fct/inum_rec.cnv_fct;
		PriceProcess ();
		DiscProcess ();	/*here*/
		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Qty validation lookup. |
	------------------------*/
	if (LCHECK ("qty"))
	{
		calc_avail ();

		if (SERIAL)
		{
			local_rec.wk_qty = 1.00;
			/*
			return (EXIT_SUCCESS);
			*/
		}
			
		if (last_char == EOI && prog_status == ENTRY) 
		{
			skip_entry = -3;
			return (EXIT_SUCCESS);
		}

		if (store [line_cnt].avail < local_rec.wk_qty)
		{
			sprintf (err_str, ML (mlCmMess175),BELL, store [line_cnt].avail, clip (local_rec.item_no),local_rec.wk_qty);
			i = prmptmsg (err_str,"YyNn",1,2);
			print_at (2,1, "                                                                                                  ");
			if (i == 'n' || i == 'N') 
				return (EXIT_FAILURE); 
		}
		cc = find_cost ();
		if (cc)
			return (cc);
		
	
		local_rec.rec_date	=	StringToDate (local_rec.d_date);
		DSP_FLD ("cost");

		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate lots and locations. |
	------------------------------*/
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		TempLine	=	lcount [2];
		TempLineCnt	=	line_cnt;
		cc = DisplayLL
			 (										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.hhwh_hash, 						/*	Warehouse hash.		*/
				SR._hhum_hash,						/*	UOM hash			*/
				SR.hhcc_hash,						/*	CC hash.			*/
				SR._UOM,							/* UOM					*/
				local_rec.wk_qty,					/* Quantity.			*/
				SR._cnv_fct,						/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				SR._lot_select_flag,				/* Silent mode			*/
				 (local_rec.LL [0] == 'Y'),			/* Input Mode.			*/
				SR._lot_ctrl						/* Lot controled item. 	*/
													/*----------------------*/
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount [2] = (line_cnt + 1 > lcount [2]) ? line_cnt + 1 : lcount [2];
		heading (2);
		line_cnt = TempLineCnt;
		scn_display (2);
		lcount [2] = TempLine;
		if (cc)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
validDate (
 long	inp_date)
{
	if (inp_date < MonthStart (comm_rec.inv_date) ||
			inp_date > MonthEnd (comm_rec.inv_date))
	{
		return print_err (ML (mlStdMess246));

	}
	strcpy (local_rec.d_date, DateToString (inp_date));

	return (EXIT_SUCCESS);
}
/*===============================
| add transaction to incc files |
===============================*/
void
update (
 void)
{
	char	wk_ref [11];
	int		NoLots;
	int		i;

	int	tran_type = 0;

	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	abc_selfield (ccmr, "ccmr_hhcc_hash");

	/*--------------------------------------------------
	| Update all inventory and general ledger records. |
	--------------------------------------------------*/
	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
	{
		getval (line_cnt);

		putchar ('.');
		fflush (stdout);

		/*-------------------------------------------------
		| Find inmr record from item number in structure. | 
		-------------------------------------------------*/
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		strcpy (inmr_rec.item_no,local_rec.item_no);
		cc = find_rec (inmr , &inmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, inmr, "DBFIND");
				
		if (inmr_rec.hhsi_hash != 0L)
			inmr_rec.hhbr_hash = inmr_rec.hhsi_hash;

		/*----------------------------------------------
		| Find warehouse record from master item hash. |
		----------------------------------------------*/
		incc_rec.hhcc_hash = SR.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = findincc ();
		if (cc)
			file_err (cc, incc, "DBFIND");

		local_rec.hhwh_hash = incc_rec.hhwh_hash;
			
		incc_rec.issues += local_rec.wk_qty;

		incc_rec.closing_stock = incc_rec.opening_stock 
								  + incc_rec.pur 
								  + incc_rec.receipts 
								  + incc_rec.adj
								  - incc_rec.issues
								  - incc_rec.sales;

		/*--------------------------
		| Update warehouse record. |
		--------------------------*/
		cc = abc_update (incc ,&incc_rec);
		if (cc) 
			file_err (cc, incc, "DBUPDATE");

		/*--------------------------------------
		| Find Warehouse unit of measure file. |
		--------------------------------------*/
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
		{
			memset (&inwu_rec, 0, sizeof (inwu_rec));
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = abc_add (inwu, &inwu_rec);
			if (cc)
				file_err (cc, inwu, "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
			if (cc)
				file_err (cc, inwu, "DBFIND");
		}

		inwu_rec.issues	+= local_rec.wk_qty;
		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, "inwu", "DBUPDATE");

		if (local_rec.wk_qty != 0.00)
		{
			if (SERIAL)
			{
				cc = UpdateInsf (local_rec.hhwh_hash, 0L,
						local_rec.ser_no, "F", "S");
				if (cc) 
					file_err (cc, insf, "DBUPDATE");

				abc_unlock (insf);
			}
		}

		strcpy (inmr_rec.item_no,local_rec.item_no);
		
		inmr_rec.on_hand -= local_rec.wk_qty;

		/*----------------------------------
		| Update inventory master records. |
		----------------------------------*/
		cc = abc_update (inmr ,&inmr_rec);
		if (cc) 
			file_err (cc, inmr, "DBUPDATE");

		sprintf (wk_ref, "%-10.10s", local_rec.ser_no);
		
		tran_type = 3;

		/*-----------------------
		| Read warehouse master |
		-----------------------*/
		ccmr_rec.hhcc_hash = incc_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, ccmr, "DBFIND");

		/*--------------------------------------------
		| Add General Ledger inventory transactions. |
		--------------------------------------------*/
		add_glwk ();

		sprintf (err_str, "CNT%s", cmhr_rec.cont_no);

		NoLots	=	TRUE;
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (!LL_Valid (line_cnt, i))
				break;

			NoLots	=	FALSE;
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd 
			 (
				comm_rec.co_no, 
				comm_rec.est_no, 
				ccmr_rec.cc_no,
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash,
				SR._hhum_hash,
				local_rec.rec_date, 
				tran_type, 
				GetLotNo (line_cnt, i),
				inmr_rec.inmr_class, 		
				inmr_rec.category, 
				local_rec.j_ref,
				err_str, 
				GetBaseQty (line_cnt, i),
				0.00, 
				CENTS (local_rec.wk_cost) 
			); 
		}
		if (NoLots)
		{
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd 
			 (
				comm_rec.co_no, 
				comm_rec.est_no, 
				ccmr_rec.cc_no,
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash,
				SR._hhum_hash,
				local_rec.rec_date, 
				tran_type, 
				" ",
				inmr_rec.inmr_class, 		
				inmr_rec.category, 
				local_rec.j_ref,
				err_str, 
				 (float) ToStdUom (local_rec.wk_qty), 
				0.00, 
				CENTS (local_rec.wk_cost) 
			); 
		}

		if (SK_BATCH_CONT || MULT_LOC)
		{
			UpdateLotLocation 
			 (
				line_cnt,
				TRUE
			);
		}
	}
	strcpy (local_rec.prev_item,inmr_rec.item_no);

	abc_selfield (ccmr, "ccmr_id_no");
}

/*================================
| Add transactions to glwk file. |
================================*/
void
add_glwk (
 void)
{
	int		monthPeriod;

	/*---------------------
	| get gls from header |
	---------------------*/
	abc_selfield (cmhr, "cmhr_hhhr_hash");

	cc = find_hash (cmhr, &cmhr_rec, EQUAL, "r", SR.hhhr_hash);

	if (cc)
		file_err (cc, cmhr, "DBFIND");

	/*-----------------
	| get debtor info |
	-----------------*/
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");

	cc = find_hash (cumr, &cumr_rec,EQUAL,"r",cmhr_rec.hhcu_hash);
	if (cc)
		file_err (cc, cumr, "DBFIND");
	abc_fclose (cumr);

	PriceProcess ();
	DiscProcess ();

	strcpy (SR.dbt_acc, cmhr_rec.wip_glacc);

	/*---------------------------------
	| Somes details have been printed |
	---------------------------------*/
	prnt_details ();

	/*----------
	| post WIP |
	----------*/
	/*------------------
	| lookup hhgl_hash |
	------------------*/
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	strcpy (glmrRec.acc_no, cmhr_rec.wip_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	strcpy (glwkRec.acc_no, cmhr_rec.wip_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym,   "%-10.10s", cumr_rec.dbt_no);
	sprintf (glwkRec.name,      "%-30.30s", cumr_rec.dbt_name);
	sprintf (glwkRec.chq_inv_no,"%-15.15s", cmhr_rec.cont_no);
	sprintf (glwkRec.jnl_type,  "%-1.1s",   "1");  
	sprintf (glwkRec.tran_type, "%-2.2s",   "22"); 
	glwkRec.amount = CENTS ((local_rec.wk_cost / SR._cnv_fct) * local_rec.wk_qty);
	glwkRec.ci_amt = glwkRec.amount;

	sprintf (glwkRec.narrative,"Stock Issue %s", cmhr_rec.cont_no);
	strcpy (glwkRec.alt_desc1, " ");
	strcpy (glwkRec.alt_desc2, " ");
	strcpy (glwkRec.alt_desc3, " ");
	strcpy (glwkRec.batch_no, " ");

	sprintf (glwkRec.sys_ref, "%5d", comm_rec.term);
	glwkRec.post_date = StringToDate (local_rec.r_date);
	glwkRec.tran_date = local_rec.rec_date;
	sprintf (glwkRec.user_ref, "%-10.10s", local_rec.j_ref);
	sprintf (glwkRec.stat_flag, "%-1.1s",   "2");  
	
	DateToDMY (local_rec.rec_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no,"%02d", monthPeriod);

	glwkRec.loc_amount		= glwkRec.amount;
	glwkRec.exch_rate		= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();

	sprintf (glwkRec.narrative,"Issue From WH %s", ccmr_rec.cc_no);
	strcpy (glwkRec.alt_desc1, " ");
	strcpy (glwkRec.alt_desc2, " ");
	strcpy (glwkRec.alt_desc3, " ");
	strcpy (glwkRec.batch_no, " ");

	strcpy (glwkRec.acc_no, SR.crd_acc);
	glwkRec.hhgl_hash = SR.crd_hash;
	strcpy (glwkRec.jnl_type,"2");
	glwkRec.loc_amount		= glwkRec.amount;
	glwkRec.exch_rate		= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();

	add_cmtr ();
}

/*==============================
| Print details of data input. |
==============================*/
void
prnt_details (
 void)
{
	int		i;

	for (i = 0; i < MAX_LOTS; i++)
	{
		if (!LL_Valid (line_cnt, i))
			break;

		/*---------------------------------
		| Somes details have been printed |
		---------------------------------*/
		if (i == 0)
		{
			fprintf (pp, "!%2.2s",		ccmr_rec.cc_no);
			fprintf (pp, "!%16.16s!",	inmr_rec.item_no);
			fprintf (pp, "%25.25s!",	local_rec.ser_no);
			fprintf (pp, "%10.10s!",	local_rec.j_ref);
			fprintf (pp, "%10.2f!", 	local_rec.wk_cost);
			fprintf (pp, "%-4.4s!", 	GetUOM (line_cnt, i));
			fprintf (pp, "%8.2f!", 		GetQty (line_cnt, i));
			fprintf (pp, "%10.10s!", 	DateToString (local_rec.rec_date));
			fprintf (pp, "%-10.10s!",	GetLoc (line_cnt, i));
			fprintf (pp, "%-16.16s!", 	SR.dbt_acc);
			fprintf (pp, "%-16.16s!", 	SR.crd_acc);
			 
			fprintf (pp, "%10.2f!\n", 	local_rec.wk_qty * local_rec.wk_cost);
		}
		else
		{
			fprintf (pp, "!%2.2s",		" ");
			fprintf (pp, "!%16.16s!",	" ");
			fprintf (pp, "%25.25s!",	" ");
			fprintf (pp, "%10.10s!",	" ");
			fprintf (pp, "%-10.10s!", 	" ");
			fprintf (pp, "%-4.4s!", 	GetUOM (line_cnt, i));
			fprintf (pp, "%8.2f!", 		GetQty (line_cnt, i));
			fprintf (pp, "%-10.10s!", 	" ");
			fprintf (pp, "%-10.10s!",	GetLoc (line_cnt, i));
			fprintf (pp, "%-16.16s!", 	" ");
			fprintf (pp, "%-16.16s!", 	" ");
			fprintf (pp, "%-10.10s!\n",	" ");
		}
	}

	fflush (pp);

	batch_total += (double) local_rec.wk_qty * local_rec.wk_cost;
	qty_total   += local_rec.wk_qty;
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
open_audit (
 void)
{
	if ((pp = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	sprintf (err_str,"%s <%s>",DateToString (comm_rec.inv_date),PNAME);
	fprintf (pp, ".START%s\n",clip (err_str));
	fprintf (pp, ".LP%d\n",printerNo);
	fprintf (pp, ".13\n");
	fprintf (pp, ".PI12\n");
	fprintf (pp, ".SO\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".EDIRECT ISSUES TO CONTRACTS\n");

	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s as at %24.24s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (pp, ".B2\n");

	fprintf (pp,".EBRANCH %s  \n",clip (comm_rec.est_name));

	fprintf (pp,".R===");
	fprintf (pp,"==================");
	fprintf (pp,"==========================");
	fprintf (pp,"===========");
	fprintf (pp,"===========");
	fprintf (pp,"=====");
	fprintf (pp,"=========");
	fprintf (pp,"===========");
	fprintf (pp,"===========");
	fprintf (pp,"=================");
	fprintf (pp,"=================");
	fprintf (pp,"===========\n");

	fprintf (pp,"===");
	fprintf (pp,"==================");
	fprintf (pp,"==========================");
	fprintf (pp,"===========");
	fprintf (pp,"===========");
	fprintf (pp,"=====");
	fprintf (pp,"=========");
	fprintf (pp,"===========");
	fprintf (pp,"===========");
	fprintf (pp,"=================");
	fprintf (pp,"=================");
	fprintf (pp,"===========\n");

	fprintf (pp,"!WH");
	fprintf (pp,"!  ITEM NUMBER   !");
	fprintf (pp,"       REFERENCE         !");
	fprintf (pp," JOURNAL  !");
	fprintf (pp,"   @COST  !");
	fprintf (pp,"UOM !");
	fprintf (pp,"QUANTITY!");
	fprintf (pp,"   DATE   !");
	fprintf (pp," LOCATION !");
	fprintf (pp,"      DEBIT     !");
	fprintf (pp,"     CREDIT     !");
	fprintf (pp," EXTENDED !\n");

	fprintf (pp,"!  ");
	fprintf (pp,"!                !");
	fprintf (pp,"                         !");
	fprintf (pp,"REFERENCE !");
	fprintf (pp,"          !");
	fprintf (pp,"    !");
	fprintf (pp," ISSUED !");
	fprintf (pp,"  ISSUED  !");
	fprintf (pp,"          !");

	fprintf (pp,"     ACCOUNT    !");
	fprintf (pp,"     ACCOUNT    !");
	 
	fprintf (pp," VALUE.   !\n");

	fprintf (pp,"!--");
	fprintf (pp,"!----------------!");
	fprintf (pp,"-------------------------!");
	fprintf (pp,"----------!");
	fprintf (pp,"----------!");
	fprintf (pp,"----!");
	fprintf (pp,"--------!");
	fprintf (pp,"----------!");
	fprintf (pp,"----------!");
	fprintf (pp,"----------------!");
	fprintf (pp,"----------------!");
	fprintf (pp,"----------!\n");
}

/*========================================================================
| Routine to find incc record. Returns 0 if found ok, 1 if not on file . |
| 999 if a database error occurred.                                      |
========================================================================*/
int
findincc (
 void)
{
	cc = find_rec (incc , &incc_rec, COMPARISON, "r");

	local_rec.hhwh_hash = incc_rec.hhwh_hash;
	if (cc)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
close_audit (
 void)
{
	fprintf (pp,"!--");
	fprintf (pp,"!----------------!");
	fprintf (pp,"-------------------------!");
	fprintf (pp,"----------!");
	fprintf (pp,"----------!");
	fprintf (pp,"----!");
	fprintf (pp,"--------!");
	fprintf (pp,"----------!");
	fprintf (pp,"----------!");
	fprintf (pp,"----------------!");
	fprintf (pp,"----------------!");
	fprintf (pp,"----------!\n");

	fprintf (pp,"!  ");
	fprintf (pp,"! BATCH TOTAL    !");
	fprintf (pp,"                         !");
	fprintf (pp,"          !");
	fprintf (pp,"          !");
	fprintf (pp,"    !");
	fprintf (pp,"%8.2f!", qty_total);
	fprintf (pp,"          !");
	fprintf (pp,"          !");
	fprintf (pp,"                !");
	fprintf (pp,"                !");
	fprintf (pp,"%10.2f!\n", batch_total);

	fprintf (pp,".EOF\n");
	pclose (pp);
}

/*=======================================================
| Check Whether A Serial Number For This Item Number	|
| Has Already Been Used.								|
| Return 1 if duplicate									|
=======================================================*/
int
ck_dup_ser (
 char *	ser,
 long	hhbr_hash,
 int	line_no)
{
	int	i;
	int	no_items = (prog_status == ENTRY) ? line_cnt : lcount [2];

	for (i = 0;i < no_items;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*---------------------------------------
		| cannot duplicate item_no/ser_no	|
		| unless serial no was not input	|
		---------------------------------------*/
		if (!strcmp (local_rec.ser_no,ser_space))
			continue;

		/*---------------------------------------
		| Only compare serial numbers for	|
		| the same item number			|
		---------------------------------------*/
		if (store [i].hhbr_hash == hhbr_hash)
		{
			if (!strcmp (local_rec.ser_no,ser))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}
 
/*===========================
| Process control Accounts. |
===========================*/
int
get_accs (
 char *	cat_no,
 int	ln_pos)
{
	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MIS SK ISS",
		"   ",
		cat_no
	);
	strcpy (store [ln_pos].dbt_acc, glmrRec.acc_no);
	store [ln_pos].dbt_hash = glmrRec.hhmr_hash;

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"COSTSALE C",
		"   ",
		cat_no
	);
	strcpy (store [ln_pos].crd_acc, glmrRec.acc_no);
	store [ln_pos].crd_hash = glmrRec.hhmr_hash;
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();

	if (scn == 1)
	{
		rv_pr (ML (mlCmMess174), 50,0,1);
		box (0, 3, 132, 2);
	}
	else
	{
		rv_pr (ML (mlCmMess174),50,0,1);
		/*-----
		| top |
		-----*/
		/* cnr */
		move (LCL_TAB_COL, LCL_TAB_ROW - 1);
		PGCHAR (0);
		/* line */
		move (LCL_TAB_COL + 1, LCL_TAB_ROW -1);
		line (TAB_SIZE);
		/* cnr */
		move (LCL_TAB_COL + TAB_SIZE, LCL_TAB_ROW - 1);
		PGCHAR (1);
		/* little bits */
		move (TAB_1, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_2, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_3, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_4, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_5, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_6, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_7, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_8, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_9, LCL_TAB_ROW - 1);
		PGCHAR (8);

		/*--------
		| bottom |
		--------*/
		/* cnr */
		move (LCL_TAB_COL, LCL_TAB_ROW + 12);
		PGCHAR (2);
		/* cnr */
		move (LCL_TAB_COL + TAB_SIZE, LCL_TAB_ROW + 12);
		PGCHAR (3);
		/* line */
		move (LCL_TAB_COL + 1, LCL_TAB_ROW + 12);
		line (TAB_SIZE);
		/* little bits */
		move (TAB_1, LCL_TAB_ROW + 12);
		PGCHAR (9);
		move (TAB_2, LCL_TAB_ROW + 12);
		PGCHAR (9);
		move (TAB_3, LCL_TAB_ROW + 12);
		PGCHAR (9);
		move (TAB_4, LCL_TAB_ROW + 12);
		PGCHAR (9);
		move (TAB_5, LCL_TAB_ROW + 12);
		PGCHAR (9);
		move (TAB_6, LCL_TAB_ROW + 12);
		PGCHAR (9);
		move (TAB_7, LCL_TAB_ROW + 12);
		PGCHAR (9);
		move (TAB_8, LCL_TAB_ROW + 12);
		PGCHAR (9);
		move (TAB_9, LCL_TAB_ROW + 12);
		PGCHAR (9);

		print_at (0,102,ML (mlCmMess173),local_rec.prev_item);
	}

	line_at (1,0,132);
	line_at (21,0,132);
	strcpy (err_str,ML (mlStdMess038));
	print_at (22,0,err_str, comm_rec.co_no,comm_rec.co_short);
	strcpy (err_str,ML (mlStdMess039));
	print_at (22,40,err_str,comm_rec.est_no,comm_rec.est_short);
	line_cnt = 0;	
	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
tab_other (
 int line_no)
{
	if (prog_status == ENTRY || line_no < lcount [2])
	{
		print_at (17,18,ML (mlStdMess247), local_rec.item_desc);
		print_at (18,18,ML (mlStdMess069), cmhr_rec.cont_no,
		 					cmcd_rec.text);
		print_at (19,18,ML (mlStdMess070), cmcm_rec.ch_code,
		  					cmcm_rec.desc);
		if (prog_status == EDIT)
		{
			strcpy (ccmr_rec.co_no,comm_rec.co_no);
			strcpy (ccmr_rec.est_no,comm_rec.est_no);
			cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		}
		print_at (20,18,ML (mlStdMess099), ccmr_rec.cc_no, ccmr_rec.name);


	}
	else
	{
		print_at (17,18,ML (mlStdMess247), " ");
		print_at (18,18,ML (mlStdMess069), " "," ");
		print_at (19,18,ML (mlStdMess070), " "," ");
		print_at (20,18,ML (mlStdMess099), " "," ");
	}
}

void
update_tab (
 int line_no)
{

	/*-----------------
	| Check If Serial |
	-----------------*/
	if (store [line_no--].ser_flag [0] == 'Y')
	{
		while (TRUE)
		{
			get_entry (label ("ser_no"));
			if (!spec_valid (label ("ser_no")))
			{
				DSP_FLD ("ser_no");
				break;
			}
		}
	}
	else
	{
		while (TRUE)
		{
			get_entry (label ("ser_no"));
			if (!spec_valid (label ("ser_no")))
			{
				DSP_FLD ("ser_no");
				break;
			}
		}

		get_entry (label ("qty"));
		DSP_FLD ("qty");
	}
}

void
SrchCmcm (
 char *	key_val)
{
	work_open ();
	cc = save_rec ("#Costhead", "#Costhead Description");

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", key_val);
	cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmcm_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmcm_rec.ch_code, key_val, strlen (key_val)))
	{
		cc = save_rec (cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmcm, &cmcm_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

void
SrchCmcb (
 void)
{
	int cc1;

	work_open ();
	cc = save_rec ("#Costhead", "#Costhead Description");

	cmcb_rec.hhhr_hash = SR.hhhr_hash;
	cmcb_rec.hhcm_hash = 0L;

	cc1 = find_rec (cmcb, &cmcb_rec, GTEQ, "r");
	if (cmcb_rec.hhhr_hash != SR.hhhr_hash)
		cc1 = TRUE;

	if (!cc1)
		cc=find_hash (cmcm2,&cmcm2_rec,EQUAL,"r",cmcb_rec.hhcm_hash);

	while (!cc && !cc1 &&
	       !strcmp (cmcm2_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (cmcm2_rec.ch_code, cmcm2_rec.desc);
		if (cc)
			break;

		cc1 = find_rec (cmcb, &cmcb_rec, NEXT, "r");
		if (cmcb_rec.hhhr_hash != SR.hhhr_hash)
			cc1 = TRUE;

		if (!cc1)
			cc=find_hash (cmcm2,&cmcm2_rec,EQUAL,"r",
						cmcb_rec.hhcm_hash);
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

void
SrchCmhr (
 char *	key_val)
{
	work_open ();
	cc = save_rec ("#Cont. No.", "#Customer Order No.");
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, cbranchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", key_val);

	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmhr_rec.cont_no, key_val, strlen (key_val)))
	{
		if (strcmp (cmhr_rec.br_no, cbranchNo))
			break;

		if (cmhr_rec.status [0] != 'O')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		if (cc)
			break;
		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, cbranchNo);
	sprintf (cmhr_rec.cont_no, "%6.6s", temp_str);

	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

void
find_desc (
 void)
{
	cmcd_rec.line_no = 0;
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcd_rec.stat_flag [0] = 'D';
	cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cmcd, "DBFIND");
}

void
add_new_cmcbs (
 void)
{
	strcpy (cmcb_rec.budg_type, "V");
	cmcb_rec.budg_cost = 0.00;
	cmcb_rec.budg_qty = 0.00;
	cmcb_rec.budg_value = 0.00;
	cmcb_rec.hhhr_hash = SR.hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;
	strcpy (cmcb_rec.dtl_lvl, "A");

	cc = abc_add (cmcb, &cmcb_rec);
	if (cc)
		file_err (cc, cmcb, "DBADD");
}

int
not_in_cont (
 void)
{

	cmcb_rec.hhhr_hash = SR.hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;

	cc = find_rec (cmcb, &cmcb_rec, EQUAL, "r");
	return (cc);
}

void
show_wh_no (
 char *	key_val)
{
    work_open ();
	cc = save_rec ("#Wh","#Warehouse");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	sprintf (ccmr_rec.cc_no,"%2.2s",key_val);
	cc = find_rec (ccmr ,&ccmr_rec,GTEQ,"r");
	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (ccmr_rec.est_no, comm_rec.est_no) && 
		      !strncmp (ccmr_rec.cc_no,key_val,strlen (key_val)))
    	{                        
	        cc = save_rec (ccmr_rec.cc_no,ccmr_rec.name);                       
		if (cc)
		        break;

		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	sprintf (ccmr_rec.cc_no,"%2.2s",temp_str);
	cc = find_rec (ccmr ,&ccmr_rec,COMPARISON,"r");
	if (cc)
 	        file_err (cc, ccmr, "DBFIND");
}

void
add_cmtr (
 void)
{
	cmtr_rec.hhhr_hash 	= SR.hhhr_hash;
	cmtr_rec.hhcm_hash 	= SR.hhcm_hash;
	cmtr_rec.hhbr_hash 	= SR.hhbr_hash;
	cmtr_rec.sale_price 	= SR.sale_price / SR._cnv_fct;
	cmtr_rec.disc_pc 	= SR.disc_pc;
	cmtr_rec.gst_pc 		= SR.gst_pc;
	cmtr_rec.tax_pc 		= SR.tax_pc;
	cmtr_rec.cost_price 	= CENTS (local_rec.wk_cost / SR._cnv_fct);
	cmtr_rec.qty 		= local_rec.wk_qty;
	cmtr_rec.date 		= local_rec.rec_date;

	strcpy (cmtr_rec.time, TimeHHMMSS ());
	strcpy (cmtr_rec.ser_no, local_rec.ser_no);

	cc = abc_add (cmtr, &cmtr_rec);
	if (cc)
		file_err (cc, cmtr, "DBADD");
}

void
PriceProcess (
 void)
{
	int		pType;
	double	gsale_price;
	double	sale_price;

	pType = atoi (cumr_rec.price_type);

	gsale_price = GetCusPrice (comm_rec.co_no,
					  		  comm_rec.est_no,
							  ccmr_rec.cc_no,
							  "  ",
							  cumr_rec.class_type,
							  inmr_rec.sellgrp,
							  cumr_rec.curr_code,
							  pType,
						 	  cumr_rec.disc_code,
							  " ",
							  cumr_rec.hhcu_hash,
							  incc_rec.hhcc_hash,
							  incc_rec.hhbr_hash,
						 	  inmr_rec.category,
							  0L,
							  comm_rec.inv_date,
							  ToStdUom (local_rec.wk_qty),
							  1.0,
							  FGN_CURR,
							  &regPc);

	/*----------------------------------------------------------
	| Inclusion of the conversion factor for the multiple unit |
	| of measure in computing the SR._calc_sprice.             |
	----------------------------------------------------------*/
	sale_price = GetCusGprice (gsale_price, regPc) * SR._cnv_fct;
	SR.sale_price = sale_price;
}

void
DiscProcess (
 void)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	pType = atoi (cumr_rec.price_type);
	cumDisc	= GetCusDisc (comm_rec.co_no,
						 comm_rec.est_no,
						 incc_rec.hhcc_hash,
						 cumr_rec.hhcu_hash,
						 cumr_rec.class_type,
						 cumr_rec.disc_code,
						 incc_rec.hhbr_hash,
						 inmr_rec.category,
						 inmr_rec.sellgrp,
						 pType,
						 SR.sale_price,
						 regPc,
						 ToStdUom (local_rec.wk_qty),
						 discArray);
							
	SR.disc_pc 		= CalcOneDisc (cumDisc,
									  discArray [0],
									  discArray [1],
									  discArray [2]);

	if (inmr_rec.disc_pc > SR.disc_pc && inmr_rec.disc_pc != 0.0)
		SR.disc_pc 	= inmr_rec.disc_pc;

	SR.gst_pc  = inmr_rec.gst_pc;
	SR.tax_pc  = inmr_rec.tax_pc;
}

/*-----------------
| Find item cost. |
-----------------*/
int
find_cost (
 void)
{
	double	wk_cost = (double) 0;

	ineiRec.hhbr_hash = SR.hhbr_hash;
	strcpy (ineiRec.est_no, comm_rec.est_no);
	cc = find_rec (inei, &ineiRec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlCmMess180));
	
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	switch (SR.cost_flag [0])
	{
	case 'A':
	case 'L':
	case 'P':
	case 'T':
		wk_cost = 	FindIneiCosts
					 (
						SR.cost_flag,
						comm_rec.est_no,
						SR.hhbr_hash
					);
		break;
	case 'F':
	case 'I':
		wk_cost	=	FindIncfValue 
					(
						SR.hhwh_hash,
						(float)SR.closing,
						local_rec.wk_qty,
						(SR.cost_flag [0] == 'F') ? TRUE : FALSE,
						SR._dec_pt
					);
		break;

	case 'S':
		wk_cost = FindInsfValue (SR.hhwh_hash, TRUE);
		break;
	}

	if (wk_cost < 0.00)
	{
		wk_cost = 	FindIneiCosts
					 (
						"L",
						comm_rec.est_no,
						SR.hhbr_hash
					);
	}
	local_rec.wk_cost = twodec (wk_cost * SR._cnv_fct);

	return (EXIT_SUCCESS);
}

void
calc_avail (
 void)
{
	store [line_cnt].hhwh_hash = incc_rec.hhwh_hash;
	store [line_cnt].closing   = incc_rec.closing_stock;
	store [line_cnt].avail     = incc_rec.closing_stock -
							    incc_rec.backorder -
							    incc_rec.committed -
							    incc_rec.forward;
}

/*==========================
| Search on UOM (inum)     |
==========================*/
void
srch_uom (
 char *	key_val)
{
    work_open ();
    save_rec ("#UOM ","#Description");

    strcpy (inum2_rec.uom_group, inum_rec.uom_group);
    strcpy (inum2_rec.uom, key_val);
    cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
    while (!cc && !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
    {
        if (strncmp (inum2_rec.uom_group, key_val, strlen (key_val)))
        {
            cc = find_rec (inum2, &inum2_rec, NEXT, "r");
            continue;
        }

		if (!ValidItemUom (SR.hhbr_hash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom,inum2_rec.desc);
			if (cc)
				break;
		}

        cc = find_rec (inum2, &inum2_rec, NEXT, "r");
    }

    cc = disp_srch ();
    work_close ();
    if (cc)
            return;

    strcpy (inum2_rec.uom_group, inum_rec.uom_group);
    strcpy (inum2_rec.uom, key_val);
    cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
    if (cc)
		file_err (cc, inum2, "DBFIND");
}

/*=============================
| To standard unit of measure |
=============================*/
float
ToStdUom (
 float	lclQty)
{
    float   cnvQty;

    if (F_HIDE (label ("UOM")))
        return (lclQty);

    if (SR._cnv_fct == 0.00 || lclQty == 0.00)
        return (0.00);

    cnvQty = lclQty * SR._cnv_fct;

    return (cnvQty);
}

/*==========================
| To local unit of measure |
==========================*/
float
ToLclUom (
 float	lclQty)
{
    float   cnvQty;

    if (F_HIDE (label ("UOM")))
        return (lclQty);

    if (SR._cnv_fct == 0.00 || lclQty == 0.00)
        return (0.00);

    cnvQty = lclQty / SR._cnv_fct;

    return (cnvQty);
}

