/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_roy_inp.c,v 5.4 2002/07/18 07:15:55 scott Exp $
|  Program Name  : (sk_roy_inp.c)                
|  Program Desc  : (Stock    Royalty Maintenance) 
|                  (Input/Maint)
|---------------------------------------------------------------------|
|  Author        : B.C. Lim.       | Date Written  : 29/08/88         |
|---------------------------------------------------------------------|
| $Log: sk_roy_inp.c,v $
| Revision 5.4  2002/07/18 07:15:55  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2001/08/09 09:19:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:45  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:30  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_roy_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_roy_inp/sk_roy_inp.c,v 5.4 2002/07/18 07:15:55 scott Exp $";

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	UPD	1
#define	DEL	2

#include	"schema"

struct commRecord	comm_rec;
struct dbryRecord	dbry_rec;
struct rymrRecord	rymr_rec;
struct ryhrRecord	ryhr_rec;
struct rylnRecord	ryln_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;

	float	*rymr_qty	=	&rymr_rec.qty1;
	float	*rymr_pc	=	&rymr_rec.pc1;
	
	char	branchNumber [3];

	int  	levelno 	= 0,	
			newRecord 	= 0,	
			envCrCo 	= 0,	
			envCrFind 	= 0,	
			displayOK 	= 0;	
	
	float	totalPc		= 0.00;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	code [10];
	char	previousCode [10];
	char	itemNo [17];
	char	basisDesc [17];
	float	taxRate;
	char	supplierNo [7];
	long	hhsuHash;
	char	supplierName [41];
	float	royaltyPc;
	char	royaltyGlNo [MAXLEVEL + 1];
	long	e_hhmr_hash;
	char	wgl_code [MAXLEVEL + 1];
	long	t_hhmr_hash;
	char	basis [2];
	double	amt_extract;
	double	abs_amt;
} local_rec;

static	struct	var	vars []	={	
	{1, LIN, "rcode", 4, 24, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", "", "Royalty Code          ", " ", 
		NE, NO, JUSTLEFT, "", "", local_rec.code}, 
	{1, LIN, "rdesc1", 5, 24, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Royalty Description   ", " ", 
		NA, NO, JUSTLEFT, "", "", dbry_rec.desc}, 
	{1, LIN, "rdesc2", 6, 24, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                      ", " ", 
		NA, NO, JUSTLEFT, "", "", rymr_rec.desc}, 
	{1, LIN, "item", 7, 24, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number           ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.itemNo}, 
	{1, LIN, "idesc", 8, 24, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Item Description      ", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	{1, LIN, "rbasis", 10, 24, CHARTYPE, 
		"U", "          ", 
		" ", "", "Royalty Basis         ", "Enter R(etail) or N(et Price) or A(bsolute Value) or D( Obsolete Royalty - not applicable for item)", 
		YES, NO, JUSTLEFT, "RNAD", "", local_rec.basis}, 
	{1, LIN, "basisDesc", 10, 61, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", "", " -", "Retail", 
		NA, NO, JUSTLEFT, " ", "", local_rec.basisDesc}, 
	{1, LIN, "amount", 11, 24, MONEYTYPE, 
		"N,NNN,NNN,NNN.NN", "          ", 
		" ", "0", "Royalty Amount        ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.abs_amt}, 
	{1, LIN, "extr_amt", 12, 24, MONEYTYPE, 
		"N,NNN,NNN,NNN.NN", "          ", 
		" ", "0", "Royalty Extract Amount", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.amt_extract}, 
	{2, TAB, "supplierNo", MAXLINES, 1, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Supplier No ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.supplierNo}, 
	{2, TAB, "hhsuHash", 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "not_shown", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhsuHash}, 
	{2, TAB, "name", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "             Supplier Name              ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.supplierName}, 
	{2, TAB, "royaltyPc", 0, 3, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", " ", "Royalty Pc", " ", 
		YES, NO, JUSTRIGHT, "0", "100", (char *)&local_rec.royaltyPc}, 
	{2, TAB, "roy_glcode", 0, 0, CHARTYPE, 
		GlMask, "          ", 
		"0", "0", GlDesc, " Royalty G/L Code ", 
		YES, NO, JUSTLEFT, "0123456789*", "", local_rec.royaltyGlNo}, 
	{2, TAB, "e_hhmr_hash", 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "not_shown", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.e_hhmr_hash}, 
	{2, TAB, "w_taxrate", 0, 1, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0.00", "Tax Rate", " Withholding Tax Rate ", 
		YES, NO, JUSTRIGHT, "0", "999.99", (char *)&local_rec.taxRate}, 
	{2, TAB, "w_glcode", 0, 0, CHARTYPE, 
		GlMask, "          ", 
		"0", "0", GlDesc, " Withholding G/L Code ", 
		YES, NO, JUSTLEFT, "0123456789", "", local_rec.wgl_code}, 
	{2, TAB, "t_hhmr_hash", 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "not_shown", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.t_hhmr_hash}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SetDefault 			(void);
int  	spec_valid 			(int);
int  	FindGlAccount		(char *);
void 	DisplayWindow 		(void);
void 	LoadRyln 			(void);
void 	SrchRymr 			(char *);
void 	Update 				(void);
void 	UpdateRyln 			(int);
int  	heading 			(int);
int  	CalculatePC			(void);
void 	FindHhglHash 		(void);
int  	DeleteText 			(void);
int  	ListStuff 			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	int		dataSaved	= FALSE;

	SETUP_SCR (vars);

	init_scr ();				/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	OpenDB (); 

	GL_SetMask (GlFormat);

	envCrCo 	= atoi (get_env ("CR_CO"));
	envCrFind 	= atoi (get_env ("CR_FIND"));

	strcpy (branchNumber,(envCrCo) ? comm_rec.est_no : " 0");

	strcpy (local_rec.previousCode,"         ");

	swide ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		totalPc 	= 0.00;
		dataSaved 	= TRUE;

		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		displayOK 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (newRecord)
		{
			heading (2);
			entry (2);
			if (restart)
				continue;
		}
		do
		{
			edit_all ();
			if (restart)
			{
				dataSaved	= FALSE;
				break;
			}
		} while (!CalculatePC ()) ;

		if (dataSaved == TRUE)
			Update ();
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

	open_rec (dbry, dbry_list, DBRY_NO_FIELDS, "dbry_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (rymr, rymr_list, RYMR_NO_FIELDS, "rymr_id_no");
	open_rec (ryhr, ryhr_list, RYHR_NO_FIELDS, "ryhr_id_no");
	open_rec (ryln, ryln_list, RYLN_NO_FIELDS, "ryln_hhry_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no");

	OpenGlmr ();
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (dbry);
	abc_fclose (inmr);
	abc_fclose (rymr);
	abc_fclose (ryhr);
	abc_fclose (ryln);
	abc_fclose (sumr);
	abc_fclose (glmr);
	SearchFindClose ();
	GL_Close ();
	abc_dbclose ("data");
}

void
SetDefault (
 void)
{
	FLD ("rcode")		= dflt_used ? NA : YES;
	FLD ("item")		= dflt_used ? NA : YES;
	FLD ("rbasis")		= dflt_used ? NA : YES;
	FLD ("amount")		= dflt_used ? NA : YES;
	FLD ("extr_amt")	= dflt_used ? NA : YES;
}

int
spec_valid (
 int field)
{
	/*--------------------------------
	| Validate Royalty Code  Search. |
	--------------------------------*/
	if (LCHECK ("rcode"))
	{
		if (SRCH_KEY)
		{
			SrchRymr (temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (rymr_rec.co_no,comm_rec.co_no);
		strcpy (rymr_rec.code,local_rec.code);
		cc = find_rec (rymr, &rymr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess221));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (dbry_rec.co_no,comm_rec.co_no);
		sprintf (dbry_rec.cr_type,"%-3.3s",local_rec.code);
		cc = find_rec (dbry, &dbry_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess187));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		DSP_FLD ("rdesc1");
		DSP_FLD ("rdesc2");
		displayOK = 1;
		DisplayWindow ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.itemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		DSP_FLD ("item");
		DSP_FLD ("idesc");

		strcpy (ryhr_rec.code,local_rec.code);
		ryhr_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (ryhr, &ryhr_rec, COMPARISON, "u");
		if (cc)
			newRecord = 1;
		else
		{
			newRecord = 0;
			entry_exit = TRUE;
			strcpy (local_rec.basis,ryhr_rec.basis);
			local_rec.amt_extract 	= ryhr_rec.amt_extract;
			local_rec.abs_amt 		= ryhr_rec.abs_amt;
			LoadRyln ();
		}

		DSP_FLD ("basisDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rbasis"))
	{
		switch (local_rec.basis [0])
		{
		case	'R':
			strcpy (local_rec.basisDesc, ML ("Retail          "));
			FLD ("amount") = NA;
			break;

		case	'N':
			strcpy (local_rec.basisDesc, ML ("Net Price       "));
			FLD ("amount") = NA;
			break;

		case	'A':
			strcpy (local_rec.basisDesc, ML ("Absolute Value  "));
			FLD ("amount") = YES;
			break;

		case	'D':
			strcpy (local_rec.basisDesc, ML ("Obsolete Royalty"));
			break;

		default:
			strcpy (local_rec.basisDesc,"                ");
			break;
		}
		DSP_FLD ("basisDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("supplierNo"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
	
		/*-----------------------------------------------
		| Delete line if default used & line not blank	|
		-----------------------------------------------*/
		if (dflt_used)
			return (DeleteText ());
	
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (local_rec.supplierNo, 6));
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess022));
			return (EXIT_FAILURE);
		}

		if (ListStuff ())
			return (EXIT_FAILURE);

		ryln_rec.hhry_hash 	= ryhr_rec.hhry_hash;
		local_rec.hhsuHash = sumr_rec.hhsu_hash;
		strcpy (local_rec.supplierName,sumr_rec.crd_name);
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("royaltyPc"))
	{
		if (local_rec.royaltyPc == 0.0)
			return (DeleteText ());
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate General Ledger Account Number . |
	------------------------------------------*/
	if (LCHECK ("roy_glcode"))
	{
		if (SRCH_KEY)
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
		else
		{
			cc = FindGlAccount (local_rec.royaltyGlNo);
			if (cc)
				return (cc);
		}

		local_rec.e_hhmr_hash = glmrRec.hhmr_hash;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("w_glcode"))
	{
		if (SRCH_KEY)
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
		else
		{
			cc = FindGlAccount (local_rec.wgl_code);
			if (cc)
				return (cc);
		}

		local_rec.t_hhmr_hash = glmrRec.hhmr_hash;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*===============================================
| Find account number, account description etc. |
===============================================*/
int
FindGlAccount (
	char *accountNo)
{
	GL_FormAccNo (accountNo, glmrRec.acc_no, 0);

	if (!strncmp (glmrRec.acc_no,"0000000000000000", MAXLEVEL))
	{
		errmess (ML (mlStdMess186));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	strcpy (glmrRec.co_no,comm_rec.co_no);
	if ( (cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
	{
		print_err (ML (mlStdMess024));
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (glmrRec.glmr_class [2] [0] != 'P')
		return print_err (ML (mlStdMess232));
	
	print_at (4, 0, ML (mlStdMess087), glmrRec.acc_no, glmrRec.desc);
	return (EXIT_SUCCESS);
}

void
DisplayWindow (
 void)
{
	print_at (5,78,"%11.2f",rymr_qty [0]);
	print_at (5,96,"%6.2f",rymr_pc [0]);

	print_at (6,78,"%11.2f",rymr_qty [1]);
	print_at (6,96,"%6.2f",rymr_pc [1]);

	print_at (7,78,"%11.2f",rymr_qty [2]);
	print_at (7,96,"%6.2f",rymr_pc [2]);

	print_at (5,106,"%11.2f",rymr_qty [3]);
	print_at (5,124,"%6.2f",rymr_pc [3]);

	print_at (6,106,"%11.2f",rymr_qty [4]);
	print_at (6,124,"%6.2f",rymr_pc [4]);

	print_at (7,106,"%11.2f",rymr_qty [5]);
	print_at (7,124,"%6.2f",rymr_pc [5]);

	crsr_on ();
}

void
LoadRyln (
 void)
{
	switch (local_rec.basis [0])
	{
	case	'R':
		strcpy (local_rec.basisDesc, ML ("Retail          "));
		break;
	case	'N':
		strcpy (local_rec.basisDesc, ML ("Net Price       "));
		break;
	case	'A':
		strcpy (local_rec.basisDesc, ML ("Absolute Value  "));
		FLD ("amount") = YES;
		break;
	case	'D':
		strcpy (local_rec.basisDesc, ML ("Obsolete Royalty"));
		break;
	default:
		strcpy (local_rec.basisDesc,"                ");
		break;
	}

	scn_set (2);
	lcount [2] = 0;

	abc_selfield (sumr,"sumr_hhsu_hash");

	ryln_rec.hhry_hash	=	ryhr_rec.hhry_hash;
	cc = find_rec (ryln,&ryln_rec,GTEQ,"r");
	while (!cc && ryln_rec.hhry_hash == ryhr_rec.hhry_hash)
	{
		FindHhglHash ();

		sumr_rec.hhsu_hash	=	ryln_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON,"r");
		if (!cc)
		{
			local_rec.royaltyPc = ryln_rec.roy_pc;
			local_rec.taxRate 	= ryln_rec.t_rate;
			strcpy (local_rec.supplierNo,sumr_rec.crd_no);
			strcpy (local_rec.supplierName,sumr_rec.crd_name);
			local_rec.hhsuHash = sumr_rec.hhsu_hash;
			putval (lcount [2]++);
		}
		cc = find_rec (ryln, &ryln_rec, NEXT, "r");
	}
	abc_selfield (sumr,"sumr_id_no");
	scn_set (1);
}		

void
SrchRymr (
 char *key_val)
{
	_work_open (9,0,40);
	save_rec ("#Royalty  ","#Royalty Description ");
	strcpy (rymr_rec.co_no,comm_rec.co_no);
	sprintf (rymr_rec.code,"%-9.9s",key_val);
	cc = find_rec (rymr, &rymr_rec, GTEQ, "r");
	while (!cc && !strcmp (rymr_rec.co_no,comm_rec.co_no) && 
				  !strncmp (rymr_rec.code,key_val,strlen (key_val)))
	{
		cc = save_rec (rymr_rec.code,rymr_rec.desc);
		if (cc)
			break;
		cc = find_rec (rymr, &rymr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (rymr_rec.co_no,comm_rec.co_no);
	sprintf (rymr_rec.code,"%-9.9s",temp_str);
	cc = find_rec (rymr, &rymr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rymr, "DBFIND");

}

void
Update (void)
{
	scn_set (1);

	if (newRecord)
	{
		strcpy (ryhr_rec.basis, 	local_rec.basis);
		strcpy (ryhr_rec.code,		local_rec.code);
		strcpy (ryhr_rec.stat_flag,	"0");
		ryhr_rec.amt_extract 	= local_rec.amt_extract;
		ryhr_rec.abs_amt 		= local_rec.abs_amt;
		ryhr_rec.hhbr_hash 		= inmr_rec.hhbr_hash;
		cc = abc_add (ryhr,&ryhr_rec);
		if (cc)
	   		file_err (cc, ryhr, "DBADD");

		strcpy (ryhr_rec.code,local_rec.code);
		ryhr_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (ryhr,&ryhr_rec,COMPARISON,"r");
		if (cc)
	   		file_err (cc, ryhr, "DBFIND");

		UpdateRyln (UPD);
	}
	else
	{
		last_char = prmptmsg (ML (mlSkMess085),"UuIiDd",14,2);

		switch (last_char)
		{
		case	'U':
		case	'u':
			strcpy (ryhr_rec.code,local_rec.code);
			ryhr_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (ryhr,&ryhr_rec,COMPARISON,"u");
			if (cc)
				file_err (cc, ryhr, "DBFIND");

			strcpy (ryhr_rec.basis, local_rec.basis);
			ryhr_rec.amt_extract 	= local_rec.amt_extract;
			ryhr_rec.abs_amt 		= local_rec.abs_amt;
			cc = abc_update (ryhr,&ryhr_rec);
			if (cc)
				file_err (cc, ryhr, "DBUPDATE");
			       
			UpdateRyln (UPD);
			break;

		case	'I':
		case	'i':
			abc_unlock (ryhr);
			break;

		case	'D':
		case	'd':
			abc_unlock (ryhr);
			UpdateRyln (DEL);
			cc = abc_delete (ryhr);
			if (cc)
				file_err (cc, ryhr, "DBDELETE");
			break;
		}
	}
	strcpy (local_rec.previousCode,ryhr_rec.code);
}

void
UpdateRyln (
	int		action)
{
	int		line_cnt;
	int		adding = 0;

	switch (action)
	{
	case	DEL:
		ryln_rec.hhry_hash	=	ryhr_rec.hhry_hash;
		cc = find_rec (ryln, &ryln_rec, GTEQ, "u");
		while (!cc && ryln_rec.hhry_hash == ryhr_rec.hhry_hash)
		{
			cc = abc_delete (ryln);
			if (cc)
				continue;

			ryln_rec.hhry_hash	=	ryhr_rec.hhry_hash;
			cc = find_rec (ryln, &ryln_rec, GTEQ, "u");
		}
		abc_unlock (ryln);
		break;

	case	UPD:
		scn_set (2);

		ryln_rec.hhry_hash	=	ryhr_rec.hhry_hash;
		cc = find_rec (ryln, &ryln_rec, GTEQ, "u");
		if (cc || ryln_rec.hhry_hash != ryhr_rec.hhry_hash)
			adding = 1;

		abc_selfield (sumr,"sumr_hhsu_hash");

		/*-----------------------
		| For All Valid Lines	|
		-----------------------*/
		for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
		{
			/*---------------------------------------
			| Check If Record Already Exists	|
			---------------------------------------*/
			getval (line_cnt);
		
			sumr_rec.hhsu_hash	=	local_rec.hhsuHash;
			cc = find_rec (sumr,&sumr_rec, COMPARISON, "r");
			if (cc)
		   		file_err (cc, sumr, "DBFIND");
			
			ryln_rec.hhry_hash 		= ryhr_rec.hhry_hash;
			ryln_rec.hhsu_hash 		= local_rec.hhsuHash;
			ryln_rec.roy_pc 		= local_rec.royaltyPc;
			ryln_rec.t_rate 		= local_rec.taxRate;
			ryln_rec.e_hhgl_hash 	= local_rec.e_hhmr_hash;
			ryln_rec.t_hhgl_hash 	= local_rec.t_hhmr_hash;

			if (adding) 
			{
				/*-----------------------------------
				| Doesn't Exist so have to add it	|
				-----------------------------------*/
				ryln_rec.stat_flag [0] = '0';
				cc = abc_add (ryln,&ryln_rec);
				if (cc)
			   		file_err (cc, ryln, "DBADD");
			}
			else
			{
				cc = abc_update (ryln,&ryln_rec);
				if (cc) 
			   		file_err (cc, ryln, "DBUPDATE");
					
				ryln_rec.hhry_hash	=	ryhr_rec.hhry_hash;
				cc = find_rec (ryln, &ryln_rec, NEXT, "u");
				if (cc || ryln_rec.hhry_hash != ryhr_rec.hhry_hash)
					adding = 1;
				else
					adding = 0;
			}
		}
		abc_selfield (sumr,"sumr_id_no");

		/*-------------------------
		| Whats left over delete  |
		-------------------------*/
		while (!cc && !adding && ryln_rec.hhry_hash == ryhr_rec.hhry_hash)
		{
			cc = abc_delete (ryln);
			adding = cc;
			if (cc)
				continue;

			ryln_rec.hhry_hash	=	ryhr_rec.hhry_hash;
			cc = find_rec (ryln, &ryln_rec, GTEQ, "u");
		}
		abc_unlock (ryln);
		break;
	}
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
		print_at (0,110,"Last Code: %s",local_rec.previousCode);
		rv_pr (ML (mlSkMess082),52,0,1);
		line_at (1,0,132);

		switch (scn)
		{
		case	1:
			move (0,2);
			cl_line ();
			box (75,3,57,5);
			box (0,3,132,9);
			line_at (3,76,56);
			line_at (9,1,131);
			print_at (4,76,"%-14.14s %-12.12s %-14.14s %-12.12s"," QUANTITY 1-3 "," ROYALTY PC "," QUANTITY 4-6 "," ROYALTY PC ");
			if (displayOK)
				DisplayWindow ();
			break;

		case	2:
			break;
		}

		line_at (20,0,132);
		print_at (21,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		line_at (22,0,132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

int
CalculatePC (void)
{
	int		line_cnt;

	totalPc = 0.00;

	scn_set (2);
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		/*-------------------------------
		| Check If Total Pc is 100 %	|
		-------------------------------*/
		getval (line_cnt);
		totalPc += local_rec.royaltyPc;
	}
	
	if (totalPc == 100.00)
		return (EXIT_FAILURE);
	else
	{
		print_mess (ML (mlSkMess083));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}
}

void
FindHhglHash (void)
{
	abc_selfield (glmr,"glmr_hhmr_hash");

	glmrRec.hhmr_hash	=	ryln_rec.e_hhgl_hash;
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
	{
		sprintf (local_rec.royaltyGlNo,"%*.*s", MAXLEVEL,MAXLEVEL,"  ");
		local_rec.e_hhmr_hash = 0L;
	}
	else
	{
		sprintf (local_rec.royaltyGlNo,"%-*.*s", 
										MAXLEVEL,MAXLEVEL,glmrRec.acc_no);
		local_rec.e_hhmr_hash = glmrRec.hhmr_hash;
	}
	
	glmrRec.hhmr_hash	=	ryln_rec.t_hhgl_hash;
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
	{
		sprintf (local_rec.wgl_code,"%*.*s", MAXLEVEL,MAXLEVEL,"  ");
		local_rec.t_hhmr_hash = 0L;
	}
	else
	{
		sprintf (local_rec.wgl_code,"%-*.*s", 
										MAXLEVEL,MAXLEVEL,glmrRec.acc_no);
		local_rec.t_hhmr_hash = glmrRec.hhmr_hash;
	}
	abc_selfield (glmr,"glmr_id_no");
}

int
DeleteText (void)
{
	int	i;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	/*
	 * One Less Line On Tabular Screen
	 */
	lcount [2]--;

	/*
	 * Move Lines from current (line_cnt) down 1
	 * display lines on current page only	
	 */
	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		line_display ();
	}

	/*
	 * Blank Last Line & Put Back into Tabular
	 * Memory - Display if on current page only
	 */
	sprintf (local_rec.supplierNo,"%-6.6s"," ");
	sprintf (local_rec.supplierName,"%-40.40s"," ");
	local_rec.hhsuHash = 0L;
	local_rec.royaltyPc = 0.00;
	local_rec.taxRate = 0.00;
	sprintf (local_rec.royaltyGlNo,"%-*.*s",MAXLEVEL,MAXLEVEL," ");
	sprintf (local_rec.wgl_code,"%-*.*s",MAXLEVEL,MAXLEVEL," ");
	local_rec.e_hhmr_hash = 0L;
	local_rec.t_hhmr_hash = 0L;
	putval (line_cnt);
	line_display ();

	/*
	 * Reset Current Line Get data for current line
	*/
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

int
ListStuff (void)
{
	char	presentSupplier [7];
	int		maxLines,i;

	maxLines = (prog_status == ENTRY) ? line_cnt : lcount [2];
	putval (line_cnt);
	strcpy (presentSupplier,local_rec.supplierNo);

	for (i = 0; i < maxLines; i++)
	{
		if (i != line_cnt)
		{
			getval (i);
			if (!strcmp (presentSupplier,local_rec.supplierNo))
			{
				sprintf (err_str,ML (mlSkMess632),local_rec.supplierNo,i+1);
				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
	}
	getval (line_cnt);
	return (EXIT_SUCCESS);
}
