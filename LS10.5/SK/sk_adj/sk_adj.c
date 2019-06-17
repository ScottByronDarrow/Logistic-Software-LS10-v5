/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_adj.c,v 5.10 2002/07/24 08:39:11 scott Exp $
|  Program Name  : (sk_adj.c) 
|  Program Desc  : (Stock Adjustments Input/Edit)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
| $Log: sk_adj.c,v $
| Revision 5.10  2002/07/24 08:39:11  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.9  2002/06/26 05:48:39  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.8  2002/06/20 07:10:44  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.7  2002/02/26 09:22:08  scott
| Updated to open incf
|
| Revision 5.6  2002/02/26 03:30:21  scott
| S/C 00856 SKAJ7 - Input Stock Adjustment (Normal): Using an item with Last Cost or Avegrage Cost Costing Type then pressing F12 to adjust records causes an error: Database Error - File Has Not Been Opened. Manipulation of a file has been attempted before opening it.
|
| Revision 5.5  2001/08/26 23:16:58  scott
| Updated from scotts machine - ongoing WIP release 10.5
|
| Revision 5.4  2001/08/09 09:17:58  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/09 01:46:16  scott
| RELEASE 5.0
|
| Revision 5.2  2001/08/06 23:44:36  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:45  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_adj.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_adj/sk_adj.c,v 5.10 2002/07/24 08:39:11 scott Exp $";

#define MAXLINES	1000
#include <pslscr.h>
#include <twodec.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

	FILE	*fout;

	double	tot_oext  = 0.00,
			tot_next  = 0.00,
			grd_oext  = 0.00,
			grd_next  = 0.00;

	int		printerNo 	= 1,
			fifoItem	= 0,	/* TRUE if using FIFO or LIFO costing	*/
			fifoCost	= 0,	/* TRUE if using FIFO costing		*/
			maxStore 	= 0;

	char	locCurr [4],
			invAcc [MAXLEVEL + 1],
			cosAcc [MAXLEVEL + 1];

	long	invHash		= 0L,
			cosHash		= 0L;

	struct storeRec {
		long	sf_hash;
		long	sf_date;
		float	sf_qty;
		double	sf_cost;
	} store [MAXLINES];

#include	"schema"

struct comrRecord	comr_rec;
struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct intrRecord	intr_rec;

extern	int	TruePosition;
	char 	*data = "data";

#include <Costing.h>

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	prev_item [17];
	char	reference [16];
	char	cost_type [5];   /*------------------------------------------*/
	float	oh_qty;			/* qty on hand     incc_closing_stock	  	*/
	double	l_cost;			/* last cost       inei_last_cost	  		*/
	long	l_date;			/* last cost date  inei_last_cost_date    	*/
	double	a_cost;			/* average cost    inei_agve_cost	  		*/
	double	s_cost;			/* standard cost   inei_std_cost	  		*/
	double	o_cost;			/* old cost				  					*/
	long	o_date;			/* old date				  					*/
	float	f_qty;			/* FIFO quantity			  				*/
	double	f_cost;			/* FIFO cost				  				*/
	long	f_date;			/* FIFO date				  				*/
	long	f_hash;			/* Unique Serial hash to find/update recs 	*/
	char	systemDate [11];   /*------------------------------------------*/
	char	item_no [17];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "item_no",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item number.      ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description       ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{1, LIN, "ctype",	 7, 2, CHARTYPE,
		"AAAA", "          ",
		" ", "", "Cost Type         ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cost_type},
	{1, LIN, "lcost",	 8, 2, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Last Cost         ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_cost},
	{1, LIN, "ldate",	 9, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Last Cost Date    ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.l_date},
	{1, LIN, "acost",	10, 2, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Average Cost      ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.a_cost},
	{1, LIN, "scost",	11, 2, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Standard Cost     ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.s_cost},
	{1, LIN, "q_on_hand",	12, 2, FLOATTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Quantity On Hand  ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.oh_qty},
	{1, LIN, "reference",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Reference         ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.reference},

	{2, TAB, "f_date",	MAXLINES, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, " FIFO Date ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.f_date},
	{2, TAB, "f_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNNN", "          ",
		" ", " ", " ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.f_hash},
	{2, TAB, "f_qty",	 0, 0, FLOATTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", " FIFO Quantity ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.f_qty},
	{2, TAB, "f_cost",	 0, 0, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", " FIFO Cost ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.f_cost},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
int  	AddInei 			(void);
int  	ReadDefault 		(char *);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	AddGlwk 			(void);
void 	AddIntr 			(void);
void 	AvgeCosted 			(void);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
void 	FifoCosted 			(int);
void 	LastCosted 			(void);
void 	LoadDetailScreen 	(void);
void 	OpenAudit 			(void);
void 	OpenDB 				(void);
void 	ReadMisc 			(void);
void 	Update 				(void);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc < 2)
		printerNo = 1;
	else
		printerNo = atoi (argv [1]);

	TruePosition	=	TRUE;
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

	GL_SetMask ("XXXXXXXXXXXXXXXX");

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	OpenDB ();


	OpenAudit ();

	prog_exit = FALSE;
	while (!prog_exit) 
	{
		entry_exit	= FALSE;
		restart		= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		search_ok	= TRUE;
		lcount [2]	= 0;

		abc_unlock (inmr);
		abc_unlock (incc);

		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (fifoItem)
		{
			LoadDetailScreen ();
			edit_all ();
		}
		else
		{
			heading (1);
			scn_display (1);
			edit (1);
		}
		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();	
    return (EXIT_SUCCESS);
}

/*========================
| program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseAudit ();
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	ReadMisc ();

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");

	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
	OpenGlmr ();

	open_rec	(comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (locCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (locCurr, "%-3.3s", comr_rec.base_curr);

	OpenInei ();
	OpenIncf ();
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (comr);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (intr);
	abc_fclose (glmr);

	SearchFindClose ();
	CloseCosting ();
	GL_CloseBatch (0);
	GL_Close ();
	abc_dbclose (data);
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
void
ReadMisc (
 void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

/*==========================================
| Primary validation and file access here. |
==========================================*/
int
spec_valid (
 int	field)
{
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
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.item_no,inmr_rec.item_no);
		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		/*------------------------------------------
		| Look up to see if item is on Cost Centre |
		------------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "u");
		if (cc)
		{
			print_mess (ML (mlStdMess192));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.l_cost = 	FindIneiCosts 
							(
								"L", 
								comm_rec.est_no, 
								inmr_rec.hhbr_hash
							);
		local_rec.l_cost = twodec (local_rec.l_cost);
		local_rec.l_date = ineiRec.date_lcost;

		local_rec.a_cost = ineiRec.avge_cost;
		local_rec.a_cost = twodec (local_rec.a_cost);

		local_rec.s_cost = ineiRec.std_cost;
		local_rec.s_cost = twodec (local_rec.s_cost);

		local_rec.oh_qty = incc_rec.closing_stock;

		local_rec.o_date = local_rec.l_date;

		fifoItem = FALSE;
		entry_exit = TRUE;

		switch (inmr_rec.costing_flag [0])
		{
		case 'A':
			local_rec.o_cost = twodec (local_rec.a_cost);
			strcpy (local_rec.cost_type,"AVGE");
			break;

		case 'L':
			local_rec.o_cost = twodec (local_rec.l_cost);
			strcpy (local_rec.cost_type,"LAST");
			break;

		case 'F':
		case 'I':
			fifoItem = TRUE;
			fifoCost = (inmr_rec.costing_flag [0] == 'F');
			strcpy (local_rec.cost_type, (fifoCost) ? "FIFO" : "LIFO");
			entry_exit = TRUE;
			break;

		default:
			print_mess (ML (" Not Last/Avge/Fifo/Lifo Costed "));
			sleep (sleepTime);
			return (EXIT_FAILURE);
			break;
		}            

		/*-----------------------------
		| Read GL codes for category. |
		-----------------------------*/
		ReadDefault (inmr_rec.category);

		DSP_FLD ("item_no");
		DSP_FLD ("desc");
		DSP_FLD ("ctype");
		DSP_FLD ("lcost");
		DSP_FLD ("ldate");
		DSP_FLD ("acost");
		DSP_FLD ("scost");
		DSP_FLD ("q_on_hand");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*========================================
| Read all nessasary files for defaults. |
========================================*/
int
ReadDefault (
 char *	cat_no)
{
	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"INVENTORY",
		" ", 
		cat_no
	);
	strcpy (invAcc, glmrRec.acc_no);
	invHash	=	glmrRec.hhmr_hash;

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"COSTSALE D",
		" ",
		cat_no
	);
	strcpy (cosAcc, glmrRec.acc_no);
	cosHash = glmrRec.hhmr_hash;

	return (EXIT_SUCCESS);
}
void
LoadDetailScreen (
 void)
{
	int	i;
	float	on_hand = incc_rec.closing_stock;

	maxStore 	= 0;
	scn_set (2);
	lcount [2] 	= 0;
	local_rec.f_date 	= 0L;
	local_rec.f_hash 	= 0L;
	local_rec.f_qty 	= 0.00;
	local_rec.f_cost 	= 0.00;
	putval (lcount [2]);

	for (i = 0; i < MAXLINES; i++)
	{
		store [i].sf_hash 	= 0L;
		store [i].sf_date 	= 0L;
		store [i].sf_qty 	= 0.00;
		store [i].sf_cost 	= 0.00;
	}

	cc = FindIncf (incc_rec.hhwh_hash, !fifoCost, "r");
	while (!cc && on_hand > 0.00 && 
		   incfRec.hhwh_hash == incc_rec.hhwh_hash && 
		   lcount [2] < MAXLINES)
	{
		store [lcount [2]].sf_date = local_rec.f_date = incfRec.fifo_date;
		store [lcount [2]].sf_hash = incfRec.hhcf_hash;
		local_rec.f_hash = incfRec.hhcf_hash;

		if (incfRec.fifo_qty > on_hand)
		{
			local_rec.f_qty = on_hand;
			on_hand = 0.00;
		}
		else
		{
			local_rec.f_qty = incfRec.fifo_qty;
			on_hand -= incfRec.fifo_qty;
		}
		store [lcount [2]].sf_qty  	= local_rec.f_qty;
		store [lcount [2]].sf_cost 	= twodec (incfRec.fifo_cost);
		local_rec.f_cost 			= twodec (incfRec.fifo_cost);
		maxStore++;

		putval (lcount [2]++);
	
		cc = FindIncf (0L, !fifoCost, "r");
	}
}

/*=======================================
|	print audit trail.		            |
|	update incf records.		        |
|	update inmr_on_hand		            |
|	update incc_adj & incc_yadj &	    |
|		incc_closing_stock	            |
=======================================*/
void
Update (
 void)
{
	int	one_off = TRUE;

	tot_oext  = 0.00;
	tot_next  = 0.00;

	clear ();
	fflush (stdout);

	abc_selfield (incf,"incf_hhcf_hash");
	/*-----------------------------------
	| Print detail line to audit trail. |
	-----------------------------------*/
	scn_set (2);
	for (line_cnt = 0; (fifoItem && line_cnt < lcount [2]) || one_off;
				line_cnt++,one_off = FALSE)
	{
		if (line_cnt == 0 || one_off)
		{
			fprintf (fout,"|%-16.16s",inmr_rec.item_no);
			fprintf (fout,"|%-40.40s",inmr_rec.description);
		}
		else
		{
			fprintf (fout,"|%-16.16s"," ");
			fprintf (fout,"|%-40.40s"," ");
		}

		switch (inmr_rec.costing_flag [0])
		{
		case 'A':
			AvgeCosted ();
			break;

		case 'L':
			LastCosted ();
			break;

		case 'I':
		case 'F':
			FifoCosted (line_cnt);
			break;

		default:
			break;
		}
	}

	cc = FindInei (inmr_rec.hhbr_hash, comm_rec.est_no, "u");
	if (cc)
	{
		abc_unlock (inei);
		cc = AddInei ();
	}

	if (cc)
		file_err (cc, inei, "DBFIND");

	ineiRec.avge_cost 	= local_rec.a_cost;
	ineiRec.prev_cost 	= ineiRec.last_cost;
	ineiRec.last_cost 	= local_rec.l_cost;
	ineiRec.std_cost 	= local_rec.s_cost;
	ineiRec.date_lcost 	= local_rec.l_date;
	cc = abc_update (inei,&ineiRec);
	if (cc) 
		file_err (cc, inei, "DBUPDATE");

	AddIntr ();

	AddGlwk ();

	abc_selfield (incf,"incf_id_no");

	strcpy (local_rec.prev_item,local_rec.item_no);
	scn_set (1);
}

void
AddIntr (
 void)
{
	double	price;

	price = tot_next - tot_oext;
	price = CENTS (price);

	strcpy (intr_rec.co_no,comm_rec.co_no);
	strcpy (intr_rec.br_no,comm_rec.est_no);
	intr_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	intr_rec.hhcc_hash 	= incc_rec.hhcc_hash;
	intr_rec.type 		= 4;	/* stock balance */
	intr_rec.date 		= comm_rec.inv_date;
	strcpy (intr_rec.ref1,	local_rec.reference);
	sprintf (intr_rec.ref2,"%-15.15s",getenv ("LOGNAME"));
	intr_rec.qty 		= 0.00;
	intr_rec.cost_price = price;
	cc = abc_add (intr,&intr_rec);
	if (cc)
		file_err (cc, intr, "DBADD");
}

/*================================
| Add transactions to glwk file. |
================================*/
void
AddGlwk (
 void)
{
	int		monthNum;
	double	price;
	int		Reverse	=	FALSE;

	price = tot_next - tot_oext;
	price = CENTS (price);

	if (price < 0.00)
	{
		Reverse	=	TRUE;
		price *= -1;
	}

	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.tran_type, "12");
	glwkRec.post_date = StringToDate (local_rec.systemDate);
	glwkRec.tran_date = comm_rec.inv_date;

	DateToDMY (comm_rec.inv_date, NULL, &monthNum, NULL);
	sprintf (glwkRec.period_no, "%02d", monthNum);

	sprintf (glwkRec.sys_ref,"%5.1d",comm_rec.term);
	strcpy (glwkRec.user_ref, local_rec.reference);
	strcpy (glwkRec.stat_flag,"2");

	strcpy (glwkRec.narrative, "Stock adjustment.");
	sprintf (glwkRec.alt_desc1, "BR : %s  / WH : %s",
						comm_rec.est_no, comm_rec.cc_no);
	sprintf (glwkRec.alt_desc2, "SKU:%s", local_rec.item_no);
	sprintf (glwkRec.alt_desc3, " ");
	sprintf (glwkRec.batch_no,  " ");

	glwkRec.amount = price;

	sprintf (glwkRec.acc_no, "%-*.*s",MAXLEVEL,MAXLEVEL,invAcc);
	glwkRec.hhgl_hash = invHash;

	strcpy (glwkRec.jnl_type, (Reverse) ? "2" : "1");
	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate	= 1.00;
	strcpy (glwkRec.currency, locCurr);

	GL_AddBatch ();

	sprintf (glwkRec.acc_no, "%-*.*s",MAXLEVEL,MAXLEVEL,cosAcc);
	glwkRec.hhgl_hash = cosHash;

	strcpy (glwkRec.jnl_type, (Reverse) ? "1" : "2");
	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate	= 1.00;
	strcpy (glwkRec.currency, locCurr);

	GL_AddBatch ();
}

void
AvgeCosted (
 void)
{
	double	nval = 0.00;
	double	oval = 0.00;

	oval = out_cost (local_rec.o_cost,inmr_rec.outer_size);
	nval = out_cost (local_rec.a_cost,inmr_rec.outer_size);

	oval = twodec (oval);
	nval = twodec (nval);

	fprintf (fout,"|%-4.4s ","AVGE");
	fprintf (fout,"|%-10.10s ",DateToString (local_rec.o_date));
	fprintf (fout,"|%-10.10s",DateToString (local_rec.l_date));
	fprintf (fout,"|%11.2f",oval);
	fprintf (fout,"|%11.2f",nval);
	fprintf (fout,"|%12.2f",  (double) local_rec.oh_qty  * oval);
	fprintf (fout,"|%12.2f|\n", (double) local_rec.oh_qty * nval);

	tot_oext += (double) local_rec.oh_qty * oval;
	tot_next += (double) local_rec.oh_qty * nval;
	grd_oext += (double) local_rec.oh_qty * oval;
	grd_next += (double) local_rec.oh_qty * nval;
}

void
LastCosted (
 void)
{
	double	nval = 0.00;
	double	oval = 0.00;

	oval = out_cost (local_rec.o_cost,inmr_rec.outer_size);
	nval = out_cost (local_rec.l_cost,inmr_rec.outer_size);

	oval = twodec (oval);
	nval = twodec (nval);
	fprintf (fout,"|%-4.4s ","LAST");
	fprintf (fout,"|%-10.10s ",DateToString (local_rec.o_date));
	fprintf (fout,"|%-10.10s",DateToString (local_rec.l_date));
	fprintf (fout,"|%11.2f",oval);
	fprintf (fout,"|%11.2f",nval);
	fprintf (fout,"|%12.2f", (double) local_rec.oh_qty * oval);
	fprintf (fout,"|%12.2f|\n", (double) local_rec.oh_qty * nval);

	tot_oext += (double)local_rec.oh_qty * oval;
	tot_next += (double)local_rec.oh_qty * nval;
	grd_oext += (double)local_rec.oh_qty * oval;
	grd_next += (double)local_rec.oh_qty * nval;
}

void
FifoCosted (
 int line_no)
{
	double	nval = 0.00;
	double	oval = 0.00;
	double	oextd  = 0.00;
	double	extend  = 0.00;

	getval (line_no);
	putchar ('.');
	fflush (stdout);

	if (line_no == 0)
		fprintf (fout,"|%-4.4s ", (fifoCost) ? "FIFO" : "LIFO");
	else
		fprintf (fout,"|%-4.4s "," ");

	nval = out_cost (local_rec.f_cost,inmr_rec.outer_size);
	nval = twodec (nval);
	extend = (double) local_rec.f_qty;
	extend *= nval;
	
	/*-------------------
	|	new fifo item	|
	-------------------*/
	if (local_rec.f_hash == 0L)
	{
		fprintf (fout,"|%-10.10s ",DateToString (local_rec.f_date));
		fprintf (fout,"|%-10.10s",DateToString (local_rec.f_date));
		fprintf (fout,"|%11.2f",0.00);
		fprintf (fout,"|%11.2f",local_rec.f_cost);
		fprintf (fout,"|%12.2f",0.00);
		fprintf (fout,"|%12.2f|\n",extend);

		if (lcount [2] <= 0)
			return;

		tot_oext += oextd;
		tot_next += extend;
		grd_oext += oextd;
		grd_next += extend;

		incfRec.hhwh_hash = incc_rec.hhwh_hash;
		incfRec.hhcf_hash = 0L;
		incfRec.fifo_qty  = local_rec.f_qty;
		incfRec.fifo_date = local_rec.f_date;
		incfRec.fifo_cost = local_rec.f_cost;

		cc = abc_add (incf,&incfRec);
		if (cc)
			file_err (cc, incf, "DBADD");
	}
	else
	{
		cc = find_hash (incf,&incfRec,EQUAL,"u",local_rec.f_hash);
		if (cc)
			file_err (cc, incf, "DBFIND");

		oval = out_cost (store [line_no].sf_cost,inmr_rec.outer_size);
		oval = twodec (oval);
		oextd = (double) store [line_no].sf_qty;
		oextd *= oval;

		fprintf (fout,"|%-10.10s ",DateToString (store [line_no].sf_date));
		fprintf (fout,"|%-10.10s",DateToString (local_rec.f_date));
		fprintf (fout,"|%11.2f",store [line_no].sf_cost);
		fprintf (fout,"|%11.2f",local_rec.f_cost);
		fprintf (fout,"|%12.2f",oextd);
		fprintf (fout,"|%12.2f|\n",extend);

		if (lcount [2] <= 0)
		{
			abc_unlock (incf);
			return;
		}

		tot_oext += oextd;
		tot_next += extend;
		grd_oext += oextd;
		grd_next += extend;

		incfRec.hhwh_hash = incc_rec.hhwh_hash;
		incfRec.fifo_qty  = local_rec.f_qty;
		incfRec.fifo_date = local_rec.f_date;
		incfRec.fifo_cost = local_rec.f_cost;
		if (incfRec.fifo_qty == 0)
			cc = abc_delete (incf);
		else
			cc = abc_update (incf,&incfRec);
		if (cc)
			file_err (cc, incf, "DBUPDATE/DELETE");
	}
	abc_unlock (incf);
		
}

int
heading (
 int	scn)
{
	char	fifoHeading [256];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
        print_at (0, 55, ML (mlSkMess089), local_rec.prev_item);

		line_at (1,0,80);

		switch (scn)
		{
		case	1:
			rv_pr (ML (mlSkMess350), 25, 0, 1);
			box (0,3,80,10);
			line_at (6,1,79);
			break;

		case	2:
			sprintf (fifoHeading, "%s ", ML (mlSkMess350));
			strcat (fifoHeading, ML (mlSkMess709));
			rv_pr (fifoHeading, 20, 0, 1);

			print_at (4, 2, "%s %s - %s", ML (mlSkMess097),
						  local_rec.item_no, inmr_rec.description);
			break;
		}

		line_at (20,0,80);

		print_at (21, 0,
				  ML (mlStdMess276),
				  comm_rec.co_no,
				  clip (comm_rec.co_short),
				  comm_rec.est_no,
				  clip (comm_rec.est_short),
				  comm_rec.cc_no,
				  clip (comm_rec.cc_short));

		line_at (22,0,80);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*===============================================================
|	Routine to open output pipe to standard print to	|
|	provide an audit trail of events.			|
|	This also sends the output straight to the spooler.	|
===============================================================*/
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL) 
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".LP%d\n",printerNo);
	fprintf (fout,".SO\n");
	fprintf (fout,".10\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".ESTOCK ADJUSTMENTS\n");
	fprintf (fout,".E%s AS AT %s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (fout,".B2\n");
	fprintf (fout,".EBRANCH %s : Warehouse %s \n",clip (comm_rec.est_name),clip (comm_rec.cc_name));

	fprintf (fout,".R=================");
	fprintf (fout,"=========================================");
	fprintf (fout,"======");
	fprintf (fout,"============");
	fprintf (fout,"===========");
	fprintf (fout,"============");
	fprintf (fout,"============");
	fprintf (fout,"=============");
	fprintf (fout,"==============\n");

	fprintf (fout,"=================");
	fprintf (fout,"=========================================");
	fprintf (fout,"======");
	fprintf (fout,"============");
	fprintf (fout,"===========");
	fprintf (fout,"============");
	fprintf (fout,"============");
	fprintf (fout,"=============");
	fprintf (fout,"==============\n");

	fprintf (fout,"|  ITEM NUMBER   ");
	fprintf (fout,"|  ITEM DESCRIPTION                      ");
	fprintf (fout,"|TYPE ");
	fprintf (fout,"|OLD DATE   ");
	fprintf (fout,"|NEW DATE  ");
	fprintf (fout,"| OLD COST  ");
	fprintf (fout,"| NEW COST  ");
	fprintf (fout,"| OLD EXTENSN");
	fprintf (fout,"| NEW EXTENSN|\n");

	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------------------------------");
	fprintf (fout,"|-----");
	fprintf (fout,"|-----------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-----------");
	fprintf (fout,"|-----------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------|\n");
}

/*=======================================================
|	Routine to close the audit trail output file.	|
=======================================================*/
void
CloseAudit (
 void)
{
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------------------------------");
	fprintf (fout,"|-----");
	fprintf (fout,"|-----------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-----------");
	fprintf (fout,"|-----------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------|\n");

	/*-------------------------------
	| Print total extended values	|
	-------------------------------*/
	fprintf (fout,"|%-16.16s"," ");
	fprintf (fout,"|%-40.40s"," ");
	fprintf (fout,"|%5.5s"," ");
	fprintf (fout,"|%11.11s"," ");
	fprintf (fout,"|%10.10s"," ");
	fprintf (fout,"|%11.11s"," ");
	fprintf (fout,"|%11.11s"," ");
	fprintf (fout,"|%12.2f",grd_oext);
	fprintf (fout,"|%12.2f|\n",grd_next);

	fprintf (fout,".EOF\n");
	pclose (fout);
}

int
AddInei (
 void)
{
	long	hhbrHash	=	0L;
	ineiRec.avge_cost 	= 0.00;
	ineiRec.last_cost 	= 0.00;
	ineiRec.std_cost 	= 0.00;
	strcpy (ineiRec.stat_flag, "0");
	cc = abc_add (inei, &ineiRec);
	if (cc)
		file_err (cc, inei, "DBADD");

	hhbrHash	=	alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	return (FindInei (hhbrHash, comm_rec.est_no, "u"));
}
