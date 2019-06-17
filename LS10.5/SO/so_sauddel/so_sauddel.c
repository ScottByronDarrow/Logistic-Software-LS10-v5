/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_sauddel.c,v 5.2 2001/08/09 09:21:55 scott Exp $
|  Program Name  : (so_sauddel.c)
|  Program Desc  : (Inventory Audit Delete Program)
|---------------------------------------------------------------------|
|  Author        : Noraliza  A. Santos  Date Written : 14 Mar 96      |
|---------------------------------------------------------------------|
| $Log: so_sauddel.c,v $
| Revision 5.2  2001/08/09 09:21:55  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:59  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_sauddel.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_sauddel/so_sauddel.c,v 5.2 2001/08/09 09:21:55 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <hot_keys.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <tabdisp.h>

#define	FGN_CURR	(strcmp (cumr_rec.curr_code, Curr_code))
#define	CUIN_CHK	(cuin_chk [0] == 'Y')

extern int TruePosition;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cuinRecord	cuin_rec;
struct esmrRecord	esmr_rec;
struct trlnRecord	trln_rec;
struct trhrRecord	trhr_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct arhrRecord	arhr_rec;
struct arlnRecord	arln_rec;
struct inlsRecord	inls_rec;
struct sonsRecord	sons_rec;


/*=============================
| Local and Screen Structures |
=============================*/
struct {
	char	dummy [11];
	char	ans [2];
} local_rec;


static	struct	var	vars [] =
{
	{1, LIN, "ans", 20, 25, CHARTYPE,
		"U", "          ",
		" ", "N", "Are you sure you wish to delete invoice/credit note (Y/N) ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.ans},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


	long	offset = 0L;

	char	fnd_stat 	[2],
			cuin_chk 	[2],
			Curr_code 	[4],
			head_str 	[200],
			branchNo 	[3];

	int		envDbCo 	= FALSE,
			envDbFind  	= FALSE,
			deletion 	= FALSE,
			noInTab		= 0,
			noInLog		= 0,
			noDel		= 0,
			archive_rec = 0;

/*=======================
| Callback Declarations |
=======================*/
static	int	TagFunc 	(int, KEY_TAB *);
static	int	UndelFunc 	(int, KEY_TAB *);
static	int	TagFunc2 	(int, KEY_TAB *);
static	int	UndelFunc2 	(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB list_keys [] =
{
   { " DELETE ",		'D',		TagFunc,
	"Delete Line.",					"A" },
   { " DELETE ALL ",		CTRL ('D'), 	TagFunc,
	"Delete All Lines.",				"A" },
   { " UNDELETE ",		'U', 	UndelFunc,
	"Undelete line. ",		"A" },
   { " UNDELETE ALL ",	CTRL ('U'), 	UndelFunc,
	"Undelete all lines.",						"A" },
   END_KEYS
};

static	KEY_TAB list_keys2 [] =
{
   { " LOG TO LOST SALES ",	'B',	TagFunc2,
	"FLAG CURRENT LINE FOR DELETION.",	"A" },
   { " LOG ALL TO L. SALES", CTRL ('B'),TagFunc2,
	"FLAG ALL LINES FOR DELETION.",		"A" },
   { " DONT LOG TO LOST SALES ",		'D', 	UndelFunc2,
	"Undelete line. ",		"A" },
   { " DONT LOG ALL TO L. SALES ",	CTRL ('D'), 	UndelFunc2,
	"Undelete all lines.",						"A" },
   END_KEYS
};
#else
static	KEY_TAB list_keys [] =
{
   { " [D]ELETE ",		'D',		TagFunc,
	"Delete Line.",					"A" },
   { " [^D]ELETE ALL",		CTRL ('D'), 	TagFunc,
	"Delete All Lines.",				"A" },
   { " [U]NDELETE ",		'U', 	UndelFunc,
	"Undelete line. ",		"A" },
   { " [^U]NDELETE ALL ",	CTRL ('U'), 	UndelFunc,
	"Undelete all lines.",						"A" },
   END_KEYS
};

static	KEY_TAB list_keys2 [] =
{
   { " [B] Log to Lost Sales ",	'B',	TagFunc2,
	"Flag Current Line for Deletion.",	"A" },
   { " [^B] Log all to L. Sales", CTRL ('B'),TagFunc2,
	"Flag All Lines for Deletion.",		"A" },
   { " [D]on't Log to Lost Sales ",		'D', 	UndelFunc2,
	"Undelete line. ",		"A" },
   { " [^D]on't Log All to L. Sales ",	CTRL ('D'), 	UndelFunc2,
	"Undelete all lines.",						"A" },
   END_KEYS
};
#endif

/*=======================
| Function Declarations |
=======================*/
int  	CheckCuin 		(char *);
int  	CreateArhr 		(void);
int  	CreateArln 		(void);
int  	GetCumr 		(long);
int  	TagLine2 		(int);
int  	TagLine 		(int);
int  	UndLine2 		(int);
int  	UndLine 		(int);
static 	int FindInvoice (char *, char *, char *, char *);
void 	CloseDB 		(void);
void 	DeleteSONS 		(long);
void 	DisplayInvoice  (char *);
void 	Heading 		(void);
void 	LoadGrps 		(void);
void 	LoadLog 		(void);
void 	LogToLost 		(void);
void 	OpenDB 			(void);
void 	OpenDel 		(void);
void 	OpenLog 		(void);
void 	ProcColn 		(long);
void 	ProcessDelete 	(void);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr = get_env ("SO_ARCHIVE");
	TruePosition = TRUE;

  	if (argc != 3)
	{
		print_at (0,0, mlSoMess800, argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

  
	if (sptr && !strncmp (sptr, "Y", 1))
		archive_rec = TRUE;
	else
		archive_rec = FALSE;
	
	sprintf (fnd_stat,"%-1.1s",argv [1]);
	sprintf (cuin_chk,"%-1.1s",argv [2]);

	OpenDB ();

	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = FALSE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;
		init_vars (1);

		if (restart || prog_exit)
			continue;

		noInTab = 0;
		noInLog = 0;
		noDel = FALSE;

        swide ();
		Heading ();
		OpenDel ();
		snorm ();
		if (prog_exit)
			break;
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");
	open_rec (trhr, trhr_list, TRHR_NO_FIELDS, "trhr_hhtr_hash");
	open_rec (trln, trln_list, TRLN_NO_FIELDS, "trln_hhtr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inls, inls_list, INLS_NO_FIELDS, "inls_hhbr_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_up_id");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (sons, sons_list, SONS_NO_FIELDS, "sons_id_no2");

	if (archive_rec)
	{
		open_rec (arhr, arhr_list, ARHR_NO_FIELDS, "arhr_hhco_hash");
		open_rec (arln, arln_list, ARLN_NO_FIELDS, "arln_id_no");
	}
}

void
CloseDB (
 void)
{
	abc_fclose (esmr);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (sons);
	abc_fclose (cuin);
	abc_fclose (cumr);
	abc_fclose (trhr);
	abc_fclose (trln);

	if (archive_rec)
	{
		abc_fclose (arhr);
		abc_fclose (arln);
	}
	abc_dbclose ("data");
}

/*=================
| Open delete Tab |
=================*/
void
OpenDel (
 void)
{
	tab_open ("auddel", list_keys, 1, 10, 15, FALSE);
	tab_add ("auddel", 
		"# CO | BR |   TYPE   | DOCUMENT NO | INVOICE DATE | CUSTOMER |                CUSTOMER NAME           | TAG ");

	LoadGrps ();
	if (noInTab == 0)
	{
		tab_add ("auddel", " There Are No Invoices or Credit Notes on file.");
		tab_display ("auddel", TRUE);
		sleep (sleepTime);
		tab_close ("auddel", TRUE);
		restart = TRUE;
		prog_exit = TRUE;
		return;
	}
	else
		tab_scan ("auddel");

	if (noDel == FALSE)
		OpenLog ();

	restart = TRUE;
	prog_exit = TRUE;

	tab_close ("auddel", TRUE);
}

/*====================
| Tab Scan function. |
====================*/
static int 
TagFunc (
 int c, 
 KEY_TAB *psUnused)
{
	int	i;
	int	st_line;
	char	get_buf [200];

	st_line = tab_tline ("auddel");

	if (c == 'D')
		TagLine (st_line);
	else
	{
		for (i = st_line; i < noInTab; i++)
			TagLine (i);

		tab_display ("auddel", TRUE);
		tab_get ("auddel", get_buf, EQUAL, st_line);
		redraw_keys ("auddel");
	}
	return (c);
}

/*===================
| TabLine function. |
===================*/
int
TagLine (
 int line_no)
{
	char	get_buf [200];

	tab_get ("auddel", get_buf, EQUAL, line_no);

	tab_update ("auddel", "%-104.104s%-2.2s ", get_buf, "*");

	return (EXIT_SUCCESS);
}

static int 
UndelFunc (
 int c, 
 KEY_TAB *psUnused)
{
	int	i;
	int	st_line;
	char	get_buf [200];

	st_line = tab_tline ("auddel");

	if (c == 'U')
		UndLine (st_line);
	else
	{
		for (i = st_line; i < noInTab; i++)
			UndLine (i);

		tab_display ("auddel", TRUE);
		tab_get ("auddel", get_buf, EQUAL, st_line);
		redraw_keys ("auddel");
	}

	return (c);
}

int
UndLine (
 int line_no)
{
	char	get_buf [200];

	tab_get ("auddel", get_buf, EQUAL, line_no);

	tab_update ("auddel", "%-104.104s  %-9.9s ", get_buf, " ");

	return (EXIT_SUCCESS);
}

static int 
TagFunc2 (
 int c, 
 KEY_TAB *psUnused)
{
	int		i;
	int		st_line;
	char	get_buf [200];

	st_line = tab_tline ("auddel2");

	if (c == 'B')
		TagLine2 (st_line);
	else
	{
		for (i = st_line; i < noInTab; i++)
			TagLine2 (i);

		tab_display ("auddel2", TRUE);
		tab_get ("auddel2", get_buf, EQUAL, st_line);
		redraw_keys ("auddel2");
	}
	return (c);
}

int
TagLine2 (
 int line_no)
{
	char	get_buf [200];

	tab_get ("auddel2", get_buf, EQUAL, line_no);

	tab_update ("auddel2", "%-104.104s%-2.2s ", get_buf, "*");

	return (EXIT_SUCCESS);
}

static int 
UndelFunc2 (
 int c, 
 KEY_TAB *psUnused)
{
	int	i;
	int	st_line;
	char	get_buf [200];

	st_line = tab_tline ("auddel2");

	if (c == 'D')
		UndLine2 (st_line);
	else
	{
		for (i = st_line; i < noInLog; i++)
			UndLine2 (i);

		tab_display ("auddel2", TRUE);
		tab_get ("auddel2", get_buf, EQUAL, st_line);
		redraw_keys ("auddel2");
	}
	return (c);
}

int
UndLine2 (
 int line_no)
{
	char	get_buf [200];

	tab_get ("auddel2", get_buf, EQUAL, line_no);

	tab_update ("auddel2", "%-104.104s  %-9.9s ", get_buf, " ");

	return (EXIT_SUCCESS);
}
/*============================================
|  Load Invoices to be deleted on tab window |
============================================*/
void
LoadGrps (
 void)
{
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		DisplayInvoice  ("I");
		/*
		DisplayInvoice  ("C");
		*/
		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	return;
}

/*=======================================================
| Load and display invoice and credit note information. |
=======================================================*/
void
DisplayInvoice  (
 char	*type)
{
	strcpy (cohr_rec.co_no,esmr_rec.co_no);
	strcpy (cohr_rec.br_no,esmr_rec.est_no);
	sprintf (cohr_rec.type,"%-1.1s", type);
	strcpy (cohr_rec.stat_flag,fnd_stat);

	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");

	while (!cc && !strcmp (cohr_rec.co_no,esmr_rec.co_no) &&
	              !strcmp (cohr_rec.br_no,esmr_rec.est_no) &&
				  cohr_rec.type [0] == type [0] &&
				  cohr_rec.stat_flag [0] == fnd_stat [0])
	{
		if (cohr_rec.date_raised <= comm_rec.dbt_date)
		{
			if (CUIN_CHK && !CheckCuin (cohr_rec.inv_no))
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;
			}
			else
			{
				cc = GetCumr (cohr_rec.hhcu_hash);
				if (cc)
				{
					cc = find_rec (cohr, &cohr_rec, NEXT, "r");
					continue;
				}
			}
			tab_add ("auddel", " %2.2s | %2.2s | %-9.9s |  %8.8s  |  %10.10s  |  %6.6s  |%40.40s| %4.4s ",
				cohr_rec.co_no,	
				cohr_rec.br_no,	
				(cohr_rec.type [0] == 'I') ? "INVOICE" : "CREDIT",
				cohr_rec.inv_no,	
				DateToString (cohr_rec.date_raised),	
				cumr_rec.dbt_no,	
				cumr_rec.dbt_name,	
				"");
			noInTab++;
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
}

/*==============
| Open Log Tab |            
==============*/
void
OpenLog (
 void)
{
    char  DelAns [2];

	Heading ();
	tab_open ("auddel2", list_keys2, 1, 10, 15, FALSE);
#ifndef GVISION
	tab_add ("auddel2", 
		"# CO | BR |   TYPE   | DOCUMENT NO | INVOICE DATE | CUSTOMER |                CUSTOMER NAME           | TAG ");
#else	/* GVISION */
	tab_add ("auddel2", 
		"# CO | BR |   TYPE   | DOCUMENT NO | INVOICE DATE | CUSTOMER |                CUSTOMER NAME                  | TAG ");
#endif	/* GVISION */

	LoadLog ();

	if (noInLog == 0)  
	{
		tab_add ("auddel2", "    There Are No Invoice/ Credit Note");
		tab_add ("auddel2", "    To Log On Lost Sale.");
		tab_display ("auddel2", TRUE);
		sleep (sleepTime);
		tab_close ("auddel2", TRUE);
		restart  = TRUE; 
		prog_exit = TRUE;
		return;
	}
	else
		tab_scan ("auddel2");

	strcpy (DelAns,"N");
	scn_set (1);
	scn_write (1);
	entry (1);
    
	if (strcmp (DelAns,local_rec.ans))
    {
		ProcessDelete ();
		tab_close ("auddel2", TRUE);
	}
}

/*======================
| Deletion  Processing |
=======================*/
void
ProcessDelete (
 void)
{
	int		count;
	char	get_buf [200];
	char	curr_stat [2];
	char    CoNo [3];
	char    BrNo [3];
	char    InvNo [7];
	char    Type [2];

	dsp_screen ("Delete invoices", comm_rec.co_no, comm_rec.co_name);

	for (count = 0; count < noInLog; count++)
	{
		tab_get ("auddel2", get_buf, EQUAL, count);
		sprintf (curr_stat, "%-1.1s", get_buf + 104);

		sprintf (CoNo, "%2.2s", get_buf + 1);
		sprintf (BrNo, "%2.2s", get_buf + 6);
		sprintf (InvNo, "%8.8s", get_buf + 24);
		sprintf (Type, "%-1.1s", get_buf + 11);

		cc = FindInvoice (CoNo, BrNo, InvNo, Type);
		if (cc)
			continue;

		if (curr_stat [0] == '*')
		{
			coln_rec.hhco_hash = cohr_rec.hhco_hash;
			coln_rec.line_no = 0;
			cc = find_rec (coln, &coln_rec, GTEQ, "r");
			while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
			{
				LogToLost (); 
				cc = find_rec (coln, &coln_rec, NEXT, "r");
			}
		}
		if (archive_rec)
		{
			if (CreateArhr ()) 
				file_err (cc, arhr, "DBADD");
		}
		ProcColn (cohr_rec.hhco_hash);

		if (cohr_rec.hhtr_hash)
		{
			trhr_rec.hhtr_hash = cohr_rec.hhtr_hash;
			cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
			if (!cc)
			{
				cc = abc_delete (trhr);
				if (cc)
					file_err (cc, trhr, "DBDELETE");
			}
		
			trln_rec.hhtr_hash = cohr_rec.hhtr_hash;
			cc = find_rec (trln, &trln_rec, GTEQ, "r");
			while (!cc && trln_rec.hhtr_hash == cohr_rec.hhtr_hash)
			{
				cc = abc_delete (trln);
				if (cc)
					file_err (cc, trln, "DBDELETE");

				trln_rec.hhtr_hash = cohr_rec.hhtr_hash;
				cc = find_rec (trln, &trln_rec, GTEQ, "r");
			}
		}
		cc = abc_delete (cohr);
		if (cc)
			file_err (cc, cohr, "DBDELETE");

		dsp_process ("Trans : ",cohr_rec.inv_no);
	}
}
/*=======================
| Find invoice details. |
=======================*/
static	int
FindInvoice (
 char	*CoNo, 
 char	*BrNo, 
 char	*InvNo, 
 char	*Type)
{
	sprintf (cohr_rec.co_no,"%-2.2s", CoNo);
	sprintf (cohr_rec.br_no,"%-2.2s", BrNo);
	sprintf (cohr_rec.type,"%-1.1s", Type);
	sprintf (cohr_rec.inv_no,"%-8.8s",InvNo);

	cc = find_rec (cohr,&cohr_rec,COMPARISON,"r");
	return (cc);
}

/*======================================================================
| Log lost sales from stock quantity on hand less-than input quantity. |
======================================================================*/
void
LogToLost (
 void)
{
	double	Value;

	/*---------------------------------------------------
	| If MCURR convert sales value into local currency. |
	---------------------------------------------------*/
	if (cohr_rec.exch_rate == 0.0)
		cohr_rec.exch_rate = 1.00;

	if (FGN_CURR && cohr_rec.exch_rate != 0.00)
		Value = no_dec (coln_rec.sale_price / cohr_rec.exch_rate);
	else
		Value = no_dec (coln_rec.sale_price);

	strcpy (inls_rec.co_no , comm_rec.co_no);
	strcpy (inls_rec.est_no, comm_rec.est_no);
	inls_rec.date		=	comm_rec.inv_date;
	inls_rec.hhbr_hash	=	coln_rec.hhbr_hash;
	inls_rec.hhcc_hash	=	coln_rec.incc_hash;
	inls_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
	strcpy (inls_rec.area_code, cohr_rec.area_code);
	strcpy (inls_rec.sale_code, coln_rec.sman_code);
	inls_rec.qty		=	coln_rec.q_order + coln_rec.q_backorder;
	inls_rec.value		=	Value;
	inls_rec.cost		=	coln_rec.cost_price;
	strcpy (inls_rec.res_code, "ID");
	strcpy (inls_rec.res_desc, "Invoice Deleted");
	strcpy (inls_rec.status, "F");

	cc = abc_add ("inls", &inls_rec);
	if (cc)
		file_err (cc, "inls", "DBADD");
		
    return;
}

/*=========================
| Display tagged invoices |
=========================*/
void
LoadLog (
 void)
{
	/*---------------------------------------------------
	| in all cases will loop thru reading line by line. |
	---------------------------------------------------*/
	int		count;
	char	get_buf [200];
	char	curr_stat [2];

	for (count = 0; count < noInTab; count++)
	{
		tab_get ("auddel", get_buf, EQUAL, count);
		sprintf (curr_stat, "%-1.1s", get_buf + 104);

		/*---------------
		| find records  |
		---------------*/
		if (curr_stat [0] == '*')
		{
#ifndef GVISION
			tab_add 
			(
				"auddel2", " %2.2s | %2.2s | %-9.9s |  %8.8s  |   %8.8s   |  %6.6s  |%40.40s| %4.4s ",
				get_buf + 1,
				get_buf + 6,
				get_buf + 11,
				get_buf + 24,
				get_buf + 38,
				get_buf + 52,
				get_buf + 61,
				" "
			);
#else	/* GVISION */
			tab_add 
			(
				"auddel2", " %2.2s | %2.2s | %-9.9s |  %8.8s  |   %8.8s   |  %6.6s  |%40.40s       | %4.4s ",
				get_buf + 1,
				get_buf + 6,
				get_buf + 11,
				get_buf + 24,
				get_buf + 38,
				get_buf + 52,
				get_buf + 61,
				" "
			);
#endif	/* GVISION */
			noInLog++;
		}
	}
}

/*===========================
| Process all order lines . |
===========================*/
void
ProcColn (
 long	shash)
{
	coln_rec.hhco_hash = shash;
	coln_rec.line_no = 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == shash) 
	{
		if (archive_rec)
		{
			/*---------------------
			| Create arln records |
			---------------------*/
			if (CreateArln ())
				file_err (cc, arln, "DBADD");
		}

		DeleteSONS (coln_rec.hhcl_hash);

		cc = abc_delete (coln);
		if (cc)
			file_err (cc, coln, "DBDELETE");

		coln_rec.hhco_hash = shash;
		coln_rec.line_no = 0;
		cc = find_rec (coln, &coln_rec, GTEQ, "r");
	}
}

/*=============================================
| Delete purchase order non stock lines file. |
=============================================*/
void	
DeleteSONS (
 long	hhcl_hash)
{
	sons_rec.hhcl_hash 	= hhcl_hash;
	sons_rec.line_no 	= 0;
	cc = find_rec (sons, &sons_rec, GTEQ, "r");
	while (!cc && sons_rec.hhcl_hash == hhcl_hash)
	{
		cc = abc_delete (sons);
		if (cc)
			file_err (cc, sons, "DBDELETE");

		sons_rec.hhcl_hash 	= hhcl_hash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
	}
}

/*===============================
| Create arcive header details. |
===============================*/
int
CreateArhr (
 void)
{
	memcpy ((char *)&arhr_rec, (char *)&cohr_rec, sizeof (struct arhrRecord));

	/*----------------------------
	| Create arcive header file. |
	----------------------------*/
	return (abc_add (arhr, &arhr_rec));
}

/*===========================
| Create arcive line items. |
===========================*/
int
CreateArln (
 void)
{
	memcpy ((char *)&arln_rec, (char *)&coln_rec, sizeof (struct arlnRecord));

	/*-------------------------------------
	| Add an archive coln record to arln. |
	-------------------------------------*/
	return (abc_add (arln, &arln_rec));
}

/*===============================================
| Returns 1 if the cohr/coln is deleteable,  	|
| Returns 0 otherwise				            |
===============================================*/
int
CheckCuin (
 char *inv_no)
{
	cc = GetCumr (cohr_rec.hhcu_hash);
	if (cc)
		return (EXIT_FAILURE);

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cuin_rec.inv_no,"%-8.8s",inv_no);

	return (find_rec (cuin, &cuin_rec, COMPARISON, "r"));
}

/*=================================
| Find customer and check if H/O. |
=================================*/
int
GetCumr (
	long hhcuHash)
{
	cumr_rec.hhcu_hash = hhcuHash;
	cc = find_rec (cumr,&cumr_rec,EQUAL,"r");
	if (cc)
		return (EXIT_FAILURE);

	if (cumr_rec.ho_dbt_hash == 0L)
		return (EXIT_SUCCESS);

	cumr_rec.hhcu_hash = cumr_rec.ho_dbt_hash;
	return (find_rec (cumr,&cumr_rec,EQUAL,"r"));
}

void
Heading (
 void)
{
	clear ();
	
	line_at (21,0,132);

	rv_pr (ML (" Invoice / Credit note delete tag screen."),45,0,1);

	print_at (22,0,
			 ML (mlStdMess038),
			 comm_rec.co_no, clip (comm_rec.co_name));

	print_at (22,40,
			 ML (mlStdMess039),
			 comm_rec.est_no, comm_rec.est_name);
}


