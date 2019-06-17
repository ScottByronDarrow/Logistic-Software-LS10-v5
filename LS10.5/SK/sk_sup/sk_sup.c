/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_sup.c,v 5.4 2001/10/17 09:10:24 cha Exp $
|  Program Name  : (sk_sup.c       )                                  |
|  Program Desc  : (Supercession Item Number Input.              )    |
|---------------------------------------------------------------------|
|  Date Written  : (08/06/87)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: sk_sup.c,v $
| Revision 5.4  2001/10/17 09:10:24  cha
| Updated as getchar left in program.
| Changes made by Scott.
|
| Revision 5.3  2001/08/09 09:20:12  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:46:01  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:37  scott
| Update - LS10.5
|
| Revision 4.0  2001/03/09 02:39:15  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:35  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:21:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/13 05:51:15  scott
| Updated to remove inlo_gr_number as not used.
|
| Revision 2.0  2000/07/15 09:12:05  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.23  2000/06/13 05:03:37  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.22  1999/12/10 04:16:06  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.21  1999/11/11 06:00:09  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.20  1999/11/09 00:24:58  scott
| Updated to remove ASL specific. Had to add index (s) to incp and inds.
|
| Revision 1.19  1999/10/28 06:21:27  scott
| Updated for writable strings causing core dump.
|
| Revision 1.18  1999/10/13 03:44:15  marnie
| Modified to add env var ALLOW_ZERO_COST for inei_avge_cost.
|
| Revision 1.17  1999/10/13 02:42:18  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.16  1999/10/08 05:32:57  scott
| First Pass checkin by Scott.
|
| Revision 1.15  1999/07/23 00:05:53  scott
| ?
|
| Revision 1.14  1999/06/30 10:16:29  jonel
| Updated hhbr_hash for insf ... corrected wrong hash update
| :dd
|
| Revision 1.13  1999/06/20 05:20:50  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_sup.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_sup/sk_sup.c,v 5.4 2001/10/17 09:10:24 cha Exp $";

#include <pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

#define	STORE	store [loopCounter]
#define	MAX_WH	100

#define	OLD			inmr_old
#define	NEW			inmr_new
#define	FIFO		 (OLD.costing_flag [0] == 'F')
#define	LIFO		 (OLD.costing_flag [0] == 'I')
#define	SERIAL		 (OLD.serial_item [0] == 'Y')
#define	SA_DT_NONE	 (envVarSaProd == 0)
#define	SA_DT_SAPC	 (envVarSaProd == 1)
#define	SA_DT_SADF	 (envVarSaProd == 2)

#define	LEFT		4
#define	RIGHT		54
#define	BASE_ROW	3

#define	INMR		0
#define	BOTH		1
#define	INCF		2
#define	INSF		3
#define	COLN		4
#define	POLN		5
#define	SOLN		6
#define	INTR		7
#define	FFHS		8
#define	SALE		9
#define	INIS		10
#define	ITGL		INTR

	int		loopCounter 		= 0,
			envVarSaProd 		= 0,
			envVarAllowZeroCost = FALSE;
	
#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inexRecord	inex_rec;
struct inmrRecord	inmr_old;
struct inmrRecord	inmr_new;
struct inmrRecord	inmr_rec;
struct inloRecord	inlo_rec;
struct inloRecord	inlo2_rec;
struct ffdmRecord	ffdm_rec;
struct ffdmRecord	ffdm2_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct inwuRecord	inwu_rec;
struct inwuRecord	inwu2_rec;
struct colnRecord	coln_rec;
struct polnRecord	poln_rec;
struct solnRecord	soln_rec;
struct itglRecord	itgl_rec;
struct itlnRecord	itln_rec;
struct qtlnRecord	qtln_rec;
struct qtpdRecord	qtpd_rec;
struct intrRecord	intr_rec;
struct sapcRecord	sapc_rec;
struct sapcRecord	sapc2_rec;
struct sadfRecord	sadf_rec;
struct sadfRecord	sadf2_rec;
struct inisRecord	inis_rec;
struct inisRecord	inis2_rec;
struct incpRecord	incp_rec;
struct inprRecord	inpr_rec;
struct indsRecord	inds_rec;

	float	*incc2_con	=	&incc2_rec.c_1;
	Money	*incc2_val	=	&incc2_rec.c_val_1;
	Money	*incc2_prf	=	&incc2_rec.c_prf_1;

	Money	*incc_val	=	&incc_rec.c_val_1;
	float	*incc_con	=	&incc_rec.c_1;
	Money	*incc_prf	=	&incc_rec.c_prf_1;

	float	*sadf2_qty	=	&sadf2_rec.qty_per1;
	double	*sadf2_sal	=	&sadf2_rec.sal_per1;
	double	*sadf2_cst	=	&sadf2_rec.cst_per1;
	double	*sadf2_fri	=	&sadf2_rec.fri_per1;

	float	*sadf_qty	=	&sadf_rec.qty_per1;
	double	*sadf_sal	=	&sadf_rec.sal_per1;
	double	*sadf_cst	=	&sadf_rec.cst_per1;
	double	*sadf_fri	=	&sadf_rec.fri_per1;
		
	INEI_STRUCT	inei2Rec;

	char 	*inis2 	= "inis2",
			*sapc2 	= "sapc2",
			*sadf2 	= "sadf2",
			*incc2 	= "incc2",
			*inlo2 	= "inlo2",
			*ffdm2 	= "ffdm2",
			*inei2 	= "inei2",
			*inwu2 	= "inwu2",
			*data 	= "data";


	char	tmpWorkString [61];

	struct	{
		int	y;
		char	*desc;
	} pr_desc [] = {
		{ 0, "Inventory Master File" },
		{ 0, "Branch/Warehouse File" },
		{ 0, " Fifo / Lifo Records " },
		{ 0, "Serial Number Records" },
		{ 0, "   Invoice Details   " },
		{ 0, "   P/Order Details   " },
		{ 0, " Sales Order Details " },
		{ 0, " Stock Transactions  " },
		{ 0, "   History Details   " },
		{ 0, "   Sales Analysis    " },
		{ 0, " Inventory Supplier. " },
		{ 0, "" },
	};

	/*==============================================================
	| This Structure holds all branch information to read records. |
	==============================================================*/
	struct {
		char	sr_co_no [3];
		char	sr_est_no [3];
		char	sr_cc_no [3];
		long	sr_ccmr_hash;
	} store [MAX_WH];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	previousItem [17];
	long	oldHhbrHash;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "old_itemno",	 5, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Old Item Number", "Old Item Must Be On File.",
		 NE, NO,  JUSTLEFT, "", "", OLD.item_no},
	{1, LIN, "description",	 6, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description", " ",
		 NA, NO,  JUSTLEFT, "", "", OLD.description},
	{1, LIN, "new_itemno",	 8, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "New Item Number", "New Item Must Be On File.",
		YES, NO,  JUSTLEFT, "", "", NEW.item_no},
	{1, LIN, "description",	 9, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description", " ",
		 NA, NO,  JUSTLEFT, "", "", NEW.description},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
int  	heading 		(int);
int  	spec_valid 		(int);
void 	AddIncc 		(long, long);
void 	AddInei 		(long, char *);
void 	CloseDB 		(void);
void 	DeleteInex 		(void);
void 	FindCcmr 		(void);
void 	OpenDB 			(void);
void 	PaintScreen 	(void);
void 	ProcessColn 	(void);
void 	ProcessIncc 	(void);
void 	ProcessIncf 	(void);
void 	ProcessIncp 	(void);
void 	ProcessInds 	(void);
void 	ProcessInei 	(int);
void 	ProcessInis 	(void);
void 	ProcessInmr 	(void);
void 	ProcessInpr 	(void);
void 	ProcessInsf 	(void);
void 	ProcessIntr 	(void);
void 	ProcessItgl 	(void);
void 	ProcessItln 	(void);
void 	ProcessPoln 	(void);
void 	ProcessQtln 	(void);
void 	ProcessSadf 	(void);
void 	ProcessSapc 	(void);
void 	ProcessSoln 	(void);
void 	ReadMisc 		(void);
void 	UpdateFfdm 		(long, long);
void 	UpdateIncc 		(void);
void 	UpdateInei 		(void);
void 	UpdateInlo 		(long, long);
void 	UpdateInmr 		(void);
void 	UpdateInwu 		(long, long);
void 	UsPrint 		(int, int);
void 	ZeroIncc 		(void);
void 	ZeroInei 		(void);
void 	ZeroInmr 		(void);
void 	CopyRecord 		(int);
void 	shutdown_prog 	(void);
void 	Update 			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;

	envVarSaProd = atoi (get_env ("SA_PROD"));

	sptr = chk_env ("ALLOW_ZERO_COST");
	envVarAllowZeroCost = (sptr == (char *) 0) ? 0 : atoi (sptr);

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();	
	init_vars (1);

	OpenDB ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	while (prog_exit == 0)
	{
		/*  reset control flags  */
		entry_exit	= 0;
		edit_exit	= 0;
		prog_exit	= 0;
		restart		= 0;
		search_ok	= 1;
		init_ok		= 1;
		init_vars (1);
		/*---------------------------------------
		| Enter screen 1 linear input.		|
		---------------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;
		/*---------------------------------------
		| Update inmr & incc records.		|
		---------------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Update (); 
		crsr_on ();
		strcpy (local_rec.previousItem,OLD.item_no);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program Exit Sequence. |
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
	abc_dbopen (data);

	ReadMisc ();
	abc_alias (incc2, incc);
	abc_alias (inei2, "inei");
	abc_alias (inlo2, inlo);
	abc_alias (inwu2, inwu);
	abc_alias (ffdm2, ffdm);
	abc_alias (inis2,  inis);
	if (SA_DT_SAPC)
	{
		abc_alias (sapc2,sapc);
		open_rec (sapc,  sapc_list, SAPC_NO_FIELDS, "sapc_id_no");
		open_rec (sapc2, sapc_list, SAPC_NO_FIELDS, "sapc_hhbr_hash");
	}
	if (SA_DT_SADF)
	{
		abc_alias (sadf2,sadf);
		open_rec (sadf,  sadf_list, SADF_NO_FIELDS, "sadf_id_no");
		open_rec (sadf2, sadf_list, SADF_NO_FIELDS, "sadf_id_no4");
	}
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inex,  inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_hhwh_hash");
	open_rec (inlo2, inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (incc2, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inei2, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (ffdm,  ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec (ffdm2, ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec (inwu,  inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inwu2, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (incp,  incp_list, INCP_NO_FIELDS, "incp_hhbr_hash");
	open_rec (inpr,  inpr_list, INPR_NO_FIELDS, "inpr_hhbr_hash");
	open_rec (inds,  inds_list, INDS_NO_FIELDS, "inds_hhbr_hash");
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
	abc_fclose (inlo);
	abc_fclose (inlo2);
	abc_fclose (incc);
	abc_fclose (incc2);
	abc_fclose (inei2);
	abc_fclose (inwu);
	abc_fclose (inwu2);
	abc_fclose (ffdm);
	abc_fclose (ffdm2);
	abc_fclose (inpr);
	abc_fclose (incp);
	abc_fclose (inds);
	if (SA_DT_SAPC)
	{
		abc_fclose (sapc);
		abc_fclose (sapc2);
	}
	if (SA_DT_SADF)
	{
		abc_fclose (sadf);
		abc_fclose (sadf2);
	}
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose (data);
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	FindCcmr ();
}

int
spec_valid (
 int field)
{
	/*---------------------------------
	| Validate old Item Number Input. |
	---------------------------------*/
	if (LCHECK ("old_itemno"))
	{
		abc_selfield (inmr, "inmr_id_no");
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, OLD.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, OLD.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc) 
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		CopyRecord (0);

		if (strcmp (OLD.supercession, "                "))
		{
			sprintf (err_str, ML (mlSkMess526) ,clip (OLD.item_no));
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate new Item Number Input. |
	---------------------------------*/
	if (LCHECK ("new_itemno"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, NEW.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, NEW.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		SuperSynonymError ();

		if (!strcmp (OLD.item_no,NEW.item_no))
		{
			print_mess (ML (mlSkMess527));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (cc) 
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		CopyRecord (1);

		if (strcmp (NEW.supercession, "                "))
		{
			sprintf (err_str, ML (mlSkMess526) ,clip (NEW.item_no));
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (strcmp (OLD.costing_flag,NEW.costing_flag))
		{
			print_mess (ML (mlSkMess528));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (strcmp (OLD.serial_item,NEW.serial_item))
		{
			print_mess (ML (mlSkMess529));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
CopyRecord (
 int indx)
{
	if (indx == 0)
	{
		strcpy (OLD.item_no, inmr_rec.item_no);
		OLD.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (OLD.supercession, inmr_rec.supercession);
		strcpy (OLD.inmr_class, inmr_rec.inmr_class);
		strcpy (OLD.description, inmr_rec.description);
		strcpy (OLD.category, inmr_rec.category);
		strcpy (OLD.serial_item, inmr_rec.serial_item);
		strcpy (OLD.costing_flag, inmr_rec.costing_flag);
		strcpy (OLD.sale_unit, inmr_rec.sale_unit);
		OLD.on_hand 	= inmr_rec.on_hand;
		OLD.on_order 	= inmr_rec.on_order;
		OLD.committed 	= inmr_rec.committed;
		OLD.backorder 	= inmr_rec.backorder;
		OLD.forward 	= inmr_rec.forward;
		OLD.min_quan 	= inmr_rec.min_quan;
		OLD.max_quan 	= inmr_rec.max_quan;
		OLD.ltd_sales	= inmr_rec.ltd_sales;
		strcpy (OLD.stat_flag, inmr_rec.stat_flag);
	}
	else
	{
		strcpy (NEW.item_no, inmr_rec.item_no);
		NEW.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (NEW.supercession, inmr_rec.supercession);
		strcpy (NEW.inmr_class, inmr_rec.inmr_class);
		strcpy (NEW.description, inmr_rec.description);
		strcpy (NEW.category, inmr_rec.category);
		strcpy (NEW.serial_item, inmr_rec.serial_item);
		strcpy (NEW.costing_flag, inmr_rec.costing_flag);
		strcpy (NEW.sale_unit, inmr_rec.sale_unit);
		NEW.on_hand 	= inmr_rec.on_hand;
		NEW.on_order 	= inmr_rec.on_order;
		NEW.committed 	= inmr_rec.committed;
		NEW.backorder 	= inmr_rec.backorder;
		NEW.forward 	= inmr_rec.forward;
		NEW.min_quan 	= inmr_rec.min_quan;
		NEW.max_quan 	= inmr_rec.max_quan;
		NEW.ltd_sales	= inmr_rec.ltd_sales;
		strcpy (NEW.stat_flag, inmr_rec.stat_flag);
	}
}
/*==============================================
| Create new inmr record with new item number. |
==============================================*/
void
Update (
 void)
{
	int	i;
	int	d_row;
	/*---------------------------------------
	| set display structure			|
	---------------------------------------*/
	for (i = 0,d_row = BASE_ROW;strlen (pr_desc [i].desc);i++)
	{
		pr_desc [i].y = 0;
		switch (i)
		{
		case	INCF:
			if (FIFO || LIFO)
			{
				pr_desc [i].y = d_row;
				d_row += 2;
			}
			break;

		case	INSF:
			if (SERIAL)
			{
				pr_desc [i].y = d_row;
				d_row += 2;
			}
			break;

		default:
			pr_desc [i].y = d_row;
			d_row += 2;
			break;
		}
	}
	abc_selfield (inmr, "inmr_hhbr_hash");
	PaintScreen ();

	ProcessInmr ();
	DeleteInex ();
	ProcessIncc ();
	ProcessColn ();
	ProcessPoln ();
	ProcessSoln ();
	ProcessItln ();
	ProcessQtln ();
	ProcessIntr ();
	ProcessItgl ();
	ProcessInds ();
	ProcessInpr ();
	ProcessIncp ();
	if (SA_DT_SAPC)
		ProcessSapc ();

	if (SA_DT_SADF)
		ProcessSadf ();

	ProcessInis ();
}

void
ProcessInmr (
 void)
{
	/*-----------------------------------------------
	| Updates Inventory Master Record For New Item. |
	-----------------------------------------------*/
	UpdateInmr ();
	/*---------------------------------------------------
	| Set Inventory Master Record For Old Item to Zero. |
	---------------------------------------------------*/
	ZeroInmr ();
	UsPrint (INMR, FALSE);
}

void
ProcessInei (
 int cnt)
{
	UsPrint (BOTH, FALSE);
	/*---------------------------------------
	| find old inei record.			|
	---------------------------------------*/
	cc = FindInei (OLD.hhbr_hash, store [cnt].sr_est_no, "u");
	if (cc)
	{
		abc_unlock (inei);
		return;
	}
	/*---------------------------------------
	| find new inei record.			|
	---------------------------------------*/
	inei2Rec.hhbr_hash = NEW.hhbr_hash;
	strcpy (inei2Rec.est_no,store [cnt].sr_est_no);
	cc = find_rec (inei2, &inei2Rec, COMPARISON, "w");
	if (cc)
		AddInei (inei2Rec.hhbr_hash,inei2Rec.est_no);
	/*---------------------------------------
	| zero old inei & update new inei	|
	---------------------------------------*/
	UpdateInei ();
	ZeroInei ();
	UsPrint (BOTH, TRUE);
}

void
ProcessColn (
 void)
{
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhbr_hash");

	UsPrint (COLN,TRUE);
	cc = find_hash (coln, &coln_rec, COMPARISON, "u",OLD.hhbr_hash);
	while (!cc)
	{
		coln_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (coln, &coln_rec);
		if (cc)
			file_err (cc, "coln", "DBUPDATE");

		cc = find_hash (coln, &coln_rec, COMPARISON, "u",OLD.hhbr_hash);
	}
	abc_unlock (coln);
	abc_fclose (coln);
	UsPrint (COLN,FALSE);
}

void
ProcessPoln (
 void)
{
	UsPrint (POLN,TRUE);
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	cc = find_hash (poln, &poln_rec, COMPARISON, "u", OLD.hhbr_hash);
	while (!cc)
	{
		poln_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (poln, &poln_rec);
		if (cc)
			file_err (cc, "poln", "DBUPDATE");

		cc = find_hash (poln, &poln_rec, COMPARISON,"u",OLD.hhbr_hash);
	}
	abc_unlock (poln);
	abc_fclose (poln);
	UsPrint (POLN,FALSE);
}

void
ProcessSoln (
 void)
{
	UsPrint (SOLN,TRUE);
	open_rec (soln,soln_list,SOLN_NO_FIELDS, "soln_hhbr_hash");
	cc = find_hash (soln, &soln_rec, COMPARISON, "u",OLD.hhbr_hash);
	while (!cc)
	{
		soln_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (soln, &soln_rec);
		if (cc)
			file_err (cc, "soln", "DBUPDATE");
		
		cc = find_hash (soln, &soln_rec, COMPARISON, "u",OLD.hhbr_hash);
	}
	abc_unlock (soln);
	abc_fclose (soln);
	UsPrint (SOLN,FALSE);
}

void
ProcessItln (
 void)
{
	UsPrint (SOLN,TRUE);
	open_rec (itln,itln_list,ITLN_NO_FIELDS, "itln_hhbr_hash");
	cc = find_hash (itln, &itln_rec, COMPARISON, "u",OLD.hhbr_hash);
	while (!cc)
	{
		itln_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (itln, &itln_rec);
		if (cc)
			file_err (cc, "itln", "DBUPDATE");
	
		cc = find_hash (itln, &itln_rec, COMPARISON, "u",OLD.hhbr_hash);
	}
	abc_unlock (itln);
	abc_selfield (itln, "itln_r_hhbr_hash");
	cc = find_hash (itln, &itln_rec, COMPARISON, "u",OLD.hhbr_hash);
	while (!cc)
	{
		itln_rec.r_hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (itln, &itln_rec);
		if (cc)
			file_err (cc, "itln", "DBUPDATE");

		cc = find_hash (itln, &itln_rec, COMPARISON, "u",OLD.hhbr_hash);
	}
	abc_unlock (itln);

	abc_fclose (itln);
	UsPrint (SOLN,FALSE);
}

void
ProcessQtln (
 void)
{
	UsPrint (SOLN,TRUE);
	open_rec (qtln,qtln_list,QTLN_NO_FIELDS, "qtln_hhbr_hash");
	cc = find_hash (qtln, &qtln_rec, COMPARISON, "u",OLD.hhbr_hash);
	while (!cc)
	{
		qtln_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (qtln, &qtln_rec);
		if (cc)
			file_err (cc, "qtln", "DBUPDATE");

		cc = find_hash (qtln, &qtln_rec, COMPARISON, "u",OLD.hhbr_hash);

	}
	abc_unlock (qtln);
	abc_fclose (qtln);

	open_rec (qtpd,qtpd_list,QTPD_NO_FIELDS, "qtpd_hhbr_hash");
	cc = find_hash (qtpd, &qtpd_rec, COMPARISON, "u",OLD.hhbr_hash);
	while (!cc)
	{
		qtpd_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (qtpd, &qtpd_rec);
		if (cc)
			file_err (cc, "qtpd", "DBUPDATE");

		cc = find_hash (qtpd, &qtpd_rec, COMPARISON, "u",OLD.hhbr_hash);

	}
	abc_unlock (qtpd);
	abc_fclose (qtpd);
	UsPrint (SOLN,FALSE);
}

void
ProcessIntr (
 void)
{
	UsPrint (INTR,TRUE);
	open_rec (intr,intr_list,INTR_NO_FIELDS, "intr_hhbr_hash");
	cc = find_hash (intr, &intr_rec, COMPARISON, "u",OLD.hhbr_hash);
	while (!cc)
	{
		intr_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (intr, &intr_rec);
		if (cc)
			file_err (cc, "intr", "DBUPDATE");

		cc = find_hash (intr, &intr_rec, COMPARISON, "u",OLD.hhbr_hash);
	}
	abc_unlock (intr);
	abc_fclose (intr);
	UsPrint (INTR,FALSE);
}

void
ProcessItgl (
 void)
{
	UsPrint (ITGL, TRUE);
	open_rec (itgl, itgl_list, ITGL_NO_FIELDS, "itgl_id_no");
	cc = find_rec (itgl, &itgl_rec, FIRST, "u");
	while (!cc)
	{
		if (itgl_rec.hhbr_hash == OLD.hhbr_hash)
		{
			itgl_rec.hhbr_hash = NEW.hhbr_hash;
			cc = abc_update (itgl, &itgl_rec);
			if (cc)
				file_err (cc, "itgl", "DBUPDATE");
		}
		abc_unlock (itgl);
		cc = find_rec (itgl, &itgl_rec, NEXT, "u");
	}
	abc_fclose (itgl);
	UsPrint (ITGL, FALSE);
}

void
ProcessInis (
 void)
{
	UsPrint (INIS, TRUE);
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_hhbr_hash");
	open_rec (inis2, inis_list, INIS_NO_FIELDS, "inis_id_no2");

	cc = find_hash (inis, &inis_rec, EQUAL, "u", OLD.hhbr_hash);
	while (!cc)
	{
		inis2_rec.hhbr_hash = NEW.hhbr_hash;
		strcpy (inis2_rec.sup_priority, inis_rec.sup_priority);
		strcpy (inis2_rec.co_no, inis_rec.co_no);
		strcpy (inis2_rec.br_no, inis_rec.br_no);
		strcpy (inis2_rec.wh_no, inis_rec.wh_no);
		cc = find_rec (inis2, &inis2_rec, EQUAL, "r");
		if (cc)
		{
			inis_rec.hhbr_hash = NEW.hhbr_hash;
			cc = abc_update (inis, &inis_rec);
			if (cc)
				file_err (cc, "inis", "DBUPDATE");
		}
		else
		{
			cc = abc_delete (inis);
			if (cc)
				file_err (cc, "inis", "DBDELETE");
		}
		cc = find_hash (inis, &inis_rec, EQUAL, "u", OLD.hhbr_hash);
	}
	abc_unlock (inis);

	abc_fclose (inis);
	abc_fclose (inis2);
	UsPrint (INIS, FALSE);
}
void
ProcessSapc (
 void)
{
	UsPrint (SALE,TRUE);
	cc = find_hash (sapc2, &sapc2_rec, GTEQ, "u",OLD.hhbr_hash);
	while (!cc && sapc2_rec.hhbr_hash == OLD.hhbr_hash)
	{
		strcpy (sapc_rec.co_no,sapc2_rec.co_no);
		strcpy (sapc_rec.br_no,sapc2_rec.br_no);
		sapc_rec.hhbr_hash = NEW.hhbr_hash;
		sapc_rec.hhcu_hash = sapc2_rec.hhcu_hash;
		strcpy (sapc_rec.sman,sapc2_rec.sman);
		cc = find_rec (sapc, &sapc_rec, COMPARISON, "u");
		if (cc)
		{
			/*---------------------------------------
			| add new sales analysis record		|
			---------------------------------------*/
			strcpy (sapc_rec.co_no,sapc2_rec.co_no);
			strcpy (sapc_rec.br_no,sapc2_rec.br_no);
			sapc_rec.hhbr_hash 	= NEW.hhbr_hash;
			sapc_rec.hhcu_hash 	= sapc2_rec.hhcu_hash;
			strcpy (sapc_rec.sman,sapc2_rec.sman);
			sapc_rec.mtd_sales 	= sapc2_rec.mtd_sales;
			sapc_rec.mtd_csale 	= sapc2_rec.mtd_csale;
			sapc_rec.mtd_qty 	= sapc2_rec.mtd_qty;
			sapc_rec.ytd_sales 	= sapc2_rec.ytd_sales;
			sapc_rec.ytd_csale	= sapc2_rec.ytd_csale;
			sapc_rec.ytd_qty 	= sapc2_rec.ytd_qty;
			cc = abc_add (sapc, &sapc_rec);
			if (cc)
				file_err (cc, "sapc", "DBADD");
		}
		else
		{
			/*---------------------------------------
			| add data to existing record		|
			---------------------------------------*/
			sapc_rec.mtd_sales 	+= sapc2_rec.mtd_sales;
			sapc_rec.mtd_csale 	+= sapc2_rec.mtd_csale;
			sapc_rec.mtd_qty 	+= sapc2_rec.mtd_qty;
			sapc_rec.ytd_sales 	+= sapc2_rec.ytd_sales;
			sapc_rec.ytd_csale 	+= sapc2_rec.ytd_csale;
			sapc_rec.ytd_qty 	+= sapc2_rec.ytd_qty;

			cc = abc_update (sapc, &sapc_rec);
			if (cc)
				file_err (cc, "sapc", "DBUPDATE");
		}
		/*---------------------------------------
		| update original record		|
		---------------------------------------*/
		sapc2_rec.mtd_sales = 0.00;
		sapc2_rec.mtd_csale = 0.00;
		sapc2_rec.mtd_qty = 0.00;
		sapc2_rec.ytd_sales = 0.00;
		sapc2_rec.ytd_csale = 0.00;
		sapc2_rec.ytd_qty = 0.00;
		cc = abc_update (sapc2, &sapc2_rec);
		if (cc)
			file_err (cc, "sapc", "DBUPDATE");
		
		/*---------------------------------------
		| find next history record		|
		---------------------------------------*/
		cc = find_hash (sapc2, &sapc2_rec, NEXT, "u",OLD.hhbr_hash);
	}
	UsPrint (SALE,FALSE);
	abc_unlock (sapc2);
}

void
ProcessSadf (
 void)
{
	int	i;

	UsPrint (SALE,TRUE);
	sadf2_rec.hhbr_hash = OLD.hhbr_hash;
	sadf2_rec.hhcu_hash = 0L;
	
	cc = find_rec (sadf2, &sadf2_rec, GTEQ, "u");
	while (!cc && sadf2_rec.hhbr_hash == OLD.hhbr_hash)
	{
		strcpy (sadf_rec.co_no,sadf2_rec.co_no);
		strcpy (sadf_rec.br_no,sadf2_rec.br_no);
		strcpy (sadf_rec.year,sadf2_rec.year);
		sadf_rec.hhbr_hash = NEW.hhbr_hash;
		sadf_rec.hhcu_hash = sadf2_rec.hhcu_hash;
		strcpy (sadf_rec.sman,sadf2_rec.sman);
		strcpy (sadf_rec.area,sadf2_rec.area);
		cc = find_rec (sadf, &sadf_rec, COMPARISON, "u");
		if (cc)
		{
			/*-----------------------------------
			| add new sales analysis record		|
			------------------------------------*/
			strcpy (sadf_rec.co_no,sadf2_rec.co_no);
			strcpy (sadf_rec.br_no,sadf2_rec.br_no);
			strcpy (sadf_rec.year,sadf2_rec.year);
			sadf_rec.hhbr_hash = NEW.hhbr_hash;
			sadf_rec.hhcu_hash = sadf2_rec.hhcu_hash;
			strcpy (sadf_rec.sman,sadf2_rec.sman);
			strcpy (sadf_rec.area,sadf2_rec.area);

			for (i = 0; i < 12; i++)
			{
				sadf_qty [i] = sadf2_qty [i];
				sadf_sal [i] = sadf2_sal [i];
				sadf_cst [i] = sadf2_cst [i];
				sadf_fri [i] = sadf2_fri [i];
			}
			cc = abc_add (sadf, &sadf_rec);
			if (cc)
				file_err (cc, "sadf", "DBADD");
		}
		else
		{
			/*---------------------------------------
			| add data to existing record		|
			---------------------------------------*/
			for (i = 0; i < 12; i++)
			{
				sadf_qty [i] += sadf2_qty [i];
				sadf_sal [i] += sadf2_sal [i];
				sadf_cst [i] += sadf2_cst [i];
				sadf_fri [i] += sadf2_fri [i];
			}
			cc = abc_update (sadf, &sadf_rec);
			if (cc)
				file_err (cc, "sadf", "DBUPDATE");
		}
		/*-------------------------------
		| update original record		|
		-------------------------------*/
		for (i = 0; i < 12; i++)
		{
			sadf2_qty [i] = 0.00;
			sadf2_sal [i] = 0.00;
			sadf2_cst [i] = 0.00;
			sadf2_fri [i] = 0.00;
		}
		cc = abc_update (sadf2, &sadf2_rec);
		if (cc)
			file_err (cc, "sadf", "DBUPDATE");
		
		/*-------------------------------
		| find next history record		|
		-------------------------------*/
		cc = find_rec (sadf2, &sadf2_rec, NEXT, "u");
	}
	UsPrint (SALE,FALSE);
	abc_unlock (sadf2);
}

void
ProcessIncc (
 void)
{
	int	sfifo = NEW.costing_flag [0] == 'F';
	int	sfifo_c = sfifo || NEW.costing_flag [0] == 'I';
	int	fifo = OLD.costing_flag [0] == 'F';
	int	fifo_c = fifo || OLD.costing_flag [0] == 'I';
	int	cnt = 0;
	/*-------------------------------
	| process all warehouses		|
	-------------------------------*/
	for (cnt = 0;cnt < loopCounter;cnt++)
	{
		/*-------------------------------
		| find old incc record.			|
		-------------------------------*/
		incc_rec.hhcc_hash = store [cnt].sr_ccmr_hash;
		incc_rec.hhbr_hash = OLD.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "w");
		if (cc)
			continue;

		/*-------------------------------
		| fifo  or lifo costed			|
		-------------------------------*/
		if (fifo_c)
			ReduceIncf (incc_rec.hhwh_hash,incc_rec.closing_stock,fifo);

		/*-------------------------------
		| find new incc record.			|
		-------------------------------*/
		incc2_rec.hhcc_hash = store [cnt].sr_ccmr_hash;
		incc2_rec.hhbr_hash = NEW.hhbr_hash;
		cc = find_rec (incc2, &incc2_rec, COMPARISON, "w");
		if (cc)
			AddIncc (incc2_rec.hhcc_hash,incc2_rec.hhbr_hash);

		/*-------------------------------
		| transfer inlo records			|
		-------------------------------*/
		UpdateInlo (incc_rec.hhwh_hash,incc2_rec.hhwh_hash);

		/*-------------------------------
		| transfer inwu records			|
		-------------------------------*/
		UpdateInwu (incc_rec.hhwh_hash,incc2_rec.hhwh_hash);

		/*-------------------------------
		| transfer ffdm records			|
		-------------------------------*/
		UpdateFfdm (incc_rec.hhbr_hash,incc2_rec.hhbr_hash);

		/*-------------------------------
		| fifo  or lifo costed			|
		-------------------------------*/
		if (sfifo_c)
			ReduceIncf (incc2_rec.hhwh_hash,incc2_rec.closing_stock,sfifo);

		/*-----------------------------------
		| zero old incc & update new incc	|
		-----------------------------------*/
		ProcessInei (cnt);

		/*-----------------------------------
		| zero old incf & update new incf	|
		-----------------------------------*/
		ProcessIncf ();
		ProcessInsf ();

		UpdateIncc ();
		ZeroIncc ();
		UsPrint (BOTH,FALSE);

	}
}

void
UpdateInmr (
 void)
{
	cc = find_hash (inmr, &inmr_rec, COMPARISON, "u",NEW.hhbr_hash);
	if (cc)
		file_err (cc, "inmr", "DBFIND");

	inmr_rec.on_hand 	+= OLD.on_hand;
	inmr_rec.on_order 	+= OLD.on_order;
	inmr_rec.committed 	+= OLD.committed;
	inmr_rec.ltd_sales 	+= OLD.ltd_sales;
	inmr_rec.backorder 	+= OLD.backorder;
	inmr_rec.forward 	+= OLD.forward;
	cc = abc_update (inmr, &inmr_rec);
	if (cc)
		file_err (cc, "inmr", "DBUPDATE");
}

void
UpdateIncc (
 void)
{
	int	i;

	incc2_rec.on_order 		+= incc_rec.on_order;
	incc2_rec.committed 	+= incc_rec.committed;
	incc2_rec.backorder 	+= incc_rec.backorder;
	incc2_rec.forward 		+= incc_rec.forward;
	incc2_rec.opening_stock += incc_rec.opening_stock;
	incc2_rec.receipts 		+= incc_rec.receipts;
	incc2_rec.qc_qty 		+= incc_rec.qc_qty;
	incc2_rec.wo_qty_anti	+= incc_rec.wo_qty_anti;
	incc2_rec.pur 			+= incc_rec.pur;
	incc2_rec.issues 		+= incc_rec.issues;
	incc2_rec.adj 			+= incc_rec.adj;
	incc2_rec.sales 		+= incc_rec.sales;
	incc2_rec.stake 		+= incc_rec.stake;
	incc2_rec.ytd_receipts	+= incc_rec.ytd_receipts;
	incc2_rec.ytd_pur 		+= incc_rec.ytd_pur;
	incc2_rec.ytd_issues 	+= incc_rec.ytd_issues;
	incc2_rec.ytd_adj 		+= incc_rec.ytd_adj;
	incc2_rec.ytd_sales 	+= incc_rec.ytd_sales;
	incc2_rec.closing_stock	= incc2_rec.opening_stock 
								+ incc2_rec.pur 
								+ incc2_rec.receipts 
								+ incc2_rec.adj
								- incc2_rec.issues
								- incc2_rec.sales ;

	/*---------------------------------------
	| consumption, value & profit		|
	---------------------------------------*/
	for (i = 0;i < 12;i++)
	{
		incc2_con [i] += incc_con [i];
		incc2_val [i] += incc_val [i];
		incc2_prf [i] += incc_prf [i];
	}
	cc = abc_update (incc2, &incc2_rec);
	if (cc)
		file_err (cc, "incc", "DBUPDATE");
}

void
UpdateInei (
 void)
{
	double 	old_qty = 0.00;
	double	new_qty = 0.00;
	double	xx_qty = 0.00;
	double	new_cost = 0.00;
	double	old_cost = 0.00;

	new_qty		= incc2_rec.closing_stock;
	old_qty		= incc_rec.closing_stock;
	xx_qty		= incc_rec.closing_stock;

	new_qty		= incc2_rec.closing_stock;
	new_cost	= inei2Rec.avge_cost;

	if (old_qty != 0.00)
	{
		/*---------------------------------------
		| average Cost From New Item.		|
		---------------------------------------*/
		old_cost = ineiRec.avge_cost; 
		if (old_qty < 0.00) 
			xx_qty = 0.00;

		if (old_qty + new_qty == 0.00)
			inei2Rec.avge_cost = new_cost;
		else 	
		{
			if (old_qty + new_qty < 0.00)
           	{       
				if (envVarAllowZeroCost)         
					inei2Rec.avge_cost = 0.00;
				else
					inei2Rec.avge_cost = old_cost;
			}   
			else 
				inei2Rec.avge_cost = ((xx_qty * old_cost) +
							 (new_qty * new_cost)) /
							  (xx_qty + new_qty);
    	}
	}
	cc = abc_update (inei2, &inei2Rec);
	if (cc)
		file_err (cc, inei, "DBUPDATE");
	
}

void
ProcessIncf (
 void)
{
	int	sfifo = NEW.costing_flag [0] == 'F';
	int	sfifo_c = sfifo || NEW.costing_flag [0] == 'I';
	int	fifo = OLD.costing_flag [0] == 'F';
	int	fifo_c = fifo || OLD.costing_flag [0] == 'I';

	if (!sfifo_c || !fifo_c)
		return;

	UsPrint (INCF,TRUE);

	cc	=	TransIncf 
			(
				incc_rec.hhwh_hash,
				incc2_rec.hhwh_hash,
				incc_rec.closing_stock,
				incc_rec.closing_stock,
				fifo
			);
	if (!cc)
	{
		cc = FindIncf (incc_rec.hhwh_hash, fifo, "u");
		while (!cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			abc_delete (incf);
			cc = FindIncf (incc_rec.hhwh_hash, fifo, "u");
		}
		abc_unlock (incf);
	}
	UsPrint (INCF,FALSE);
}

void
ProcessInsf (
 void)
{
	if (OLD.serial_item [0] != 'Y')
		return;

	if (NEW.serial_item [0] != 'Y')
		return;
	
	UsPrint (INSF,TRUE);

	abc_selfield (insf, "insf_id_no2");

	insfRec.hhwh_hash = incc_rec.hhwh_hash;
    insfRec.hhbr_hash = incc_rec.hhbr_hash; 
    sprintf (insfRec.serial_no, "%-25.25s", " ");
	cc = find_rec (insf, &insfRec, GTEQ, "u");
	while (!cc && insfRec.hhwh_hash == incc_rec.hhwh_hash)
	{
		insfRec.hhwh_hash = incc2_rec.hhwh_hash;
        insfRec.hhbr_hash = incc2_rec.hhbr_hash; 

       	cc = abc_update (insf, &insfRec);
		if (cc)
			file_err (cc, insf, "DBUPDATE");
		
		insfRec.hhwh_hash = incc_rec.hhwh_hash;
		sprintf (insfRec.serial_no, "%-25.25s", " ");
    	insfRec.hhbr_hash = incc_rec.hhbr_hash; 
		cc = find_rec (insf, &insfRec, GTEQ, "u");
	}
	abc_unlock (insf);
	abc_selfield (insf, "insf_id_no");
	UsPrint (INSF,FALSE);
}

/*============================================================
| Set And Update All Figures and Status Flag for old Record. |
============================================================*/
void
ZeroInmr (
 void)
{
	cc = find_hash (inmr, &inmr_rec, COMPARISON, "u",OLD.hhbr_hash);
	if (cc)
		file_err (cc, "inmr", "DBFIND");

	strcpy (inmr_rec.supercession,NEW.item_no);
	inmr_rec.on_hand = 0.00;
	inmr_rec.on_order = 0.00;
	inmr_rec.committed = 0.00;
	inmr_rec.backorder = 0.00;
	inmr_rec.forward = 0.00;
	inmr_rec.min_quan = 0.00;
	inmr_rec.max_quan = 0.00;
	inmr_rec.ltd_sales = 0.00;
	strcpy (inmr_rec.stat_flag, "9");
	cc = abc_update (inmr, &inmr_rec);
	if (cc)
		file_err (cc, "inmr", "DBUPDATE");
}

/*============================================================
| Set And Update All Figures and Status Flag for old Record. |
============================================================*/
void
ZeroIncc (
 void)
{
	int	i;

	incc_rec.qc_qty 		= 0.00;
	incc_rec.wo_qty_anti	= 0.00;
	incc_rec.wks_demand 	= 0.00;
	incc_rec.on_order 		= 0.00;
	incc_rec.committed 		= 0.00;
	incc_rec.backorder 		= 0.00;
	incc_rec.forward 		= 0.00;
	incc_rec.opening_stock	= 0.00;
	incc_rec.receipts 		= 0.00;
	incc_rec.pur 			= 0.00;
	incc_rec.issues 		= 0.00;
	incc_rec.adj 			= 0.00;
	incc_rec.sales 			= 0.00;
	incc_rec.stake 			= 0.00;
	incc_rec.closing_stock 	= 0.00;
	incc_rec.ytd_receipts 	= 0.00;
	incc_rec.ytd_pur 		= 0.00;
	incc_rec.ytd_issues 	= 0.00;
	incc_rec.ytd_adj 		= 0.00;
	incc_rec.ytd_sales 		= 0.00;
	/*---------------------------------------
	| consumption, value & profit		|
	---------------------------------------*/
	for (i = 0;i < 12;i++)
	{
		incc_con [i] = 0.00;
		incc_val [i] = 0.00;
		incc_prf [i] = 0.00;
	}
	strcpy (incc_rec.stat_flag, "0");
	cc = abc_update (incc, &incc_rec);
	if (cc)
		file_err (cc, "incc", "DBUPDATE");
}

/*============================================================
| Set And Update All Figures and Status Flag for old Record. |
============================================================*/
void
ZeroInei (
 void)
{
	ineiRec.avge_cost = 0.00;
	ineiRec.last_cost = 0.00;
	ineiRec.date_lcost = 0;
	ineiRec.lpur_qty = 0.00;
	ineiRec.min_stock = 0.00;
	ineiRec.max_stock = 0.00;
	strcpy (ineiRec.stat_flag, "9");
	cc = abc_update (inei, &ineiRec);
	if (cc)
		file_err (cc, inei, "DBUPDATE");
}

/*==========================================================================
| Read All branch master records for company one and two and store values. |
==========================================================================*/
void
FindCcmr (
 void)
{
	loopCounter = 0;

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS, "ccmr_id_no");
	
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no))
	{
		strcpy (STORE.sr_co_no,ccmr_rec.co_no);
		strcpy (STORE.sr_est_no,ccmr_rec.est_no);
		strcpy (STORE.sr_cc_no,ccmr_rec.cc_no);
		STORE.sr_ccmr_hash = ccmr_rec.hhcc_hash;

		if (loopCounter++ >= MAX_WH)
			break;
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	abc_fclose (ccmr);
}

void
PaintScreen (
 void)
{
	int	i;
	/*---------------------------------------
	| draw heading				|
	---------------------------------------*/
	clear ();
	crsr_off ();
	print_at (0,15, ML (mlSkMess525),OLD.item_no,NEW.item_no);
	move (0,1);
	line (79);

	/*---------------------------------------
	| draw boxes & display statuses		|
	---------------------------------------*/
	for (i = 0;strlen (pr_desc [i].desc);i++)
	{
		if (pr_desc [i].y > 0)
		{
			box (LEFT - 2,pr_desc [i].y - 1,25,1);
			box (RIGHT - 2,pr_desc [i].y - 1,25,1);
			strcpy (tmpWorkString, ML (pr_desc [i].desc));
			rv_pr (tmpWorkString, LEFT,pr_desc [i].y,1);
			rv_pr (tmpWorkString, RIGHT,pr_desc [i].y,0);
		}
	}
}

void
UsPrint (
 int i, 
 int start)
{
	if (pr_desc [i].y == 0)
		return;

	strcpy (tmpWorkString, ML (pr_desc [i].desc));
	rv_pr (tmpWorkString, LEFT,pr_desc [i].y, (start) ? 1 : 0);
	rv_pr (tmpWorkString, RIGHT,pr_desc [i].y, (start) ? 0 : 1);
	fflush (stdout);
}

/*========================================
| Add incc record as it was not on file. |
========================================*/
void
AddIncc (
	long	hhccHash, 
	long	hhbrHash)
{
	char	tempSort [29];  

	memset (&incc2_rec, 0, sizeof (incc2_rec));

	sprintf (tempSort, "%s%-11.11s%-16.16s",NEW.inmr_class,NEW.category,NEW.item_no);
	incc2_rec.hhcc_hash = hhccHash;
	incc2_rec.hhbr_hash = hhbrHash;
	strcpy (incc2_rec.sort,tempSort);
	strcpy (incc2_rec.stocking_unit,NEW.sale_unit);
	strcpy (incc2_rec.ff_option, "A");
	strcpy (incc2_rec.ff_method, "A");
	strcpy (incc2_rec.abc_code, "A");
	strcpy (incc2_rec.abc_update, "N");
	incc2_rec.first_stocked = local_rec.lsystemDate;
	strcpy (incc2_rec.stat_flag, "0");
	
	cc = abc_add (incc2, &incc2_rec);
	if (cc) 
		file_err (cc, "incc", "DBADD");

	incc2_rec.hhcc_hash = hhccHash;
	incc2_rec.hhbr_hash = hhbrHash;
	cc = find_rec (incc2, &incc2_rec, COMPARISON, "w");
	if (cc) 
		file_err (cc, "incc", "DBFIND");
}

/*========================================
| Add inei record as it was not on file. |
========================================*/
void
AddInei (
	long	hhbrHash, 
	char	*branchNumber)
{
	memset (&inei2Rec, 0, sizeof (inei2Rec));
	inei2Rec.hhbr_hash = hhbrHash;
	strcpy (inei2Rec.est_no,branchNumber);
	strcpy (inei2Rec.stat_flag, "0");
	cc = abc_add (inei2, &inei2Rec);
	if (cc) 
		file_err (cc, inei, "DBADD");

	inei2Rec.hhbr_hash = hhbrHash;
	strcpy (inei2Rec.est_no,branchNumber);
	cc = find_rec (inei2, &inei2Rec, COMPARISON, "u");
	if (cc) 
		file_err (cc, inei, "DBFIND");
}

void
UpdateFfdm (
	long	oldHhbrHash,
	long	newHhbrHash)
{
	/*-----------------------------------
	| process all ffdm for warehouse	|
	-----------------------------------*/
	ffdm_rec.hhbr_hash	=	oldHhbrHash;
	ffdm_rec.hhcc_hash 	=	0L;
	ffdm_rec.date 		=	0L;
	strcpy (ffdm_rec.type, " ");
	cc = find_rec (ffdm, &ffdm_rec, GTEQ, "u");
	while (!cc && ffdm_rec.hhbr_hash == oldHhbrHash)
	{
		ffdm2_rec.hhbr_hash	=	newHhbrHash;
		ffdm2_rec.hhcc_hash =	ffdm_rec.hhcc_hash;
		ffdm2_rec.date		=	ffdm_rec.date;
		strcpy (ffdm2_rec.type, ffdm_rec.type);
		cc = find_rec (ffdm2, &ffdm2_rec, COMPARISON, "u");
		if (cc)
		{
			ffdm_rec.hhbr_hash	= newHhbrHash;
								
			/*-----------------------------------
			| transfer record to supercession	|
			-----------------------------------*/
			cc = abc_update (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, "ffdm", "DBUPDATE");
		}
		else
		{
			ffdm2_rec.qty	+= ffdm_rec.qty;
		
			cc = abc_update (ffdm2, &ffdm2_rec);
			if (cc)
				file_err (cc, "ffdm", "DBUPDATE");
			
			/*-----------------------------------
			| delete original item location		|
			-----------------------------------*/
			cc = abc_delete (ffdm);
			if (cc)
				file_err (cc, "ffdm", "DBUPDATE");
		}

		ffdm_rec.hhbr_hash	=	oldHhbrHash;
		ffdm_rec.hhcc_hash 	=	0L;
		ffdm_rec.date 		=	0L;
		strcpy (ffdm_rec.type, " ");
		cc = find_rec (ffdm, &ffdm_rec, GTEQ, "u");
	}
	abc_unlock (ffdm);
}

void
UpdateInwu (
 long	oldHhwhHash,
 long	newHhwhHash)
{
	/*-----------------------------------
	| process all inwu for warehouse	|
	-----------------------------------*/
	inwu_rec.hhwh_hash = oldHhwhHash;
	inwu_rec.hhum_hash = 0L;
	cc = find_rec (inwu, &inwu_rec, GTEQ, "u");
	while (!cc && inwu_rec.hhwh_hash == oldHhwhHash)
	{
		/*-----------------------------------
		| check for supercession location	|
		-----------------------------------*/
		inwu2_rec.hhwh_hash 	= newHhwhHash;
		inwu2_rec.hhum_hash		= inwu_rec.hhum_hash;
		cc = find_rec (inwu2, &inwu2_rec, COMPARISON, "u");
		if (cc)
		{
			inwu_rec.hhwh_hash 		= newHhwhHash;
								
			/*-----------------------------------
			| transfer record to supercession	|
			-----------------------------------*/
			cc = abc_update (inwu, &inwu_rec);
			if (cc)
				file_err (cc, "inwu", "DBUPDATE");
		}
		else
		{
			inwu2_rec.opening_stock	+= inwu_rec.opening_stock;
			inwu2_rec.receipts		+= inwu_rec.receipts;
			inwu2_rec.pur			+= inwu_rec.pur;
			inwu2_rec.issues		+= inwu_rec.issues;
			inwu2_rec.adj			+= inwu_rec.adj;
			inwu2_rec.sales			+= inwu_rec.sales;
			inwu2_rec.closing_stock	+= inwu_rec.closing_stock;
		
			cc = abc_update (inwu2, &inwu2_rec);
			if (cc)
				file_err (cc, "inwu2", "DBUPDATE");
			
			/*-----------------------------------
			| delete original item location		|
			-----------------------------------*/
			cc = abc_delete (inwu);
			if (cc)
				file_err (cc, "inwu", "DBUPDATE");
		}
		inwu_rec.hhwh_hash 	=	oldHhwhHash;
		inwu_rec.hhum_hash	=	0L;
		cc = find_rec (inwu, &inwu_rec, GTEQ, "u");
	}
	abc_unlock (inwu);
}

void
UpdateInlo (
	long	oldHhwhHash, 
	long	newHhwhHash)
{
	/*-----------------------------------
	| process all inlo for warehouse	|
	-----------------------------------*/
	abc_selfield (inlo, "inlo_hhwh_hash");		
	inlo_rec.hhwh_hash 		= oldHhwhHash;
	cc = find_rec (inlo, &inlo_rec, GTEQ, "u");
	while (!cc && inlo_rec.hhwh_hash == oldHhwhHash)
	{
		/*-----------------------------------
		| check for supercession location	|
		------------------------------------*/
		abc_selfield (inlo2, "inlo_mst_id");		
		inlo2_rec.hhwh_hash = newHhwhHash;
		inlo2_rec.hhum_hash	= inlo_rec.hhum_hash;
		strcpy (inlo2_rec.location, inlo_rec.location);
		strcpy (inlo2_rec.lot_no,   inlo_rec.lot_no);
		cc = find_rec (inlo2, &inlo2_rec, COMPARISON, "u");
		if (cc)
		{
			/*-----------------------------------
			| transfer record to supercession	|
			-----------------------------------*/
			inlo_rec.hhwh_hash = newHhwhHash;
			cc = abc_update (inlo, &inlo_rec);
			if (cc)
				file_err (cc, "inlo", "DBUPDATE");
		}
		else
		{
			/*-----------------------------------
			| update supercession location qty	|
			-----------------------------------*/
			inlo2_rec.qty 		+= inlo_rec.qty;
			inlo2_rec.no_picks 	+= inlo_rec.no_picks;
			inlo2_rec.no_hits 	+= inlo_rec.no_hits;
			cc = abc_update (inlo2, &inlo2_rec);
			if (cc)
				file_err (cc, "inlo2", "DBUPDATE");
			
			/*-----------------------------------
			| delete original item location		|
			-----------------------------------*/
			cc = abc_delete (inlo);
			if (cc)
				file_err (cc, "inlo", "DBUPDATE");
		}
		abc_selfield (inlo, "inlo_hhwh_hash");		
		inlo_rec.hhwh_hash = oldHhwhHash;
		sprintf (inlo_rec.location, "%-10.10s", " ");
		cc = find_rec (inlo, &inlo_rec, GTEQ, "u");
	}
	abc_unlock (inlo);
}

void
ProcessIncp (void)
{
	incp_rec.hhbr_hash = OLD.hhbr_hash; 
	cc = find_rec (incp, &incp_rec, GTEQ, "u");
	while (!cc && incp_rec.hhbr_hash == OLD.hhbr_hash)
	{
		incp_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (incp, &incp_rec);
		if (cc)
			file_err (cc, "incp", "DBUPDATE");


		incp_rec.hhbr_hash = OLD.hhbr_hash; 
		cc = find_rec (incp, &incp_rec, GTEQ, "u");

	}
	abc_unlock (incp);
}
void
ProcessInpr (void)
{
	inpr_rec.hhbr_hash = OLD.hhbr_hash; 
	cc = find_rec (inpr, &inpr_rec, GTEQ, "u");
	while (!cc && inpr_rec.hhbr_hash == OLD.hhbr_hash)
	{
		inpr_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (inpr, &inpr_rec);
		if (cc)
		{
			abc_unlock (inpr);
			cc = find_rec (inpr, &inpr_rec, NEXT, "u");
		}
		else
		{
			inpr_rec.hhbr_hash = OLD.hhbr_hash; 
			cc = find_rec (inpr, &inpr_rec, GTEQ, "u");
		}
	}
	abc_unlock (inpr);
}
void
ProcessInds (void)
{
	inds_rec.hhbr_hash = OLD.hhbr_hash; 
	cc = find_rec (inds, &inds_rec, GTEQ, "u");
	while (!cc && inds_rec.hhbr_hash == OLD.hhbr_hash)
	{
		inds_rec.hhbr_hash = NEW.hhbr_hash;
		cc = abc_update (inds, &inds_rec);
		if (cc)
			file_err (cc, "inds", "DBUPDATE");


		inds_rec.hhbr_hash = OLD.hhbr_hash; 
		cc = find_rec (inds, &inds_rec, GTEQ, "u");

	}
	abc_unlock (inds);
}
void
DeleteInex (
 void)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		abc_delete (inex);
		inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inex_rec.line_no = 0;
		cc = find_rec ("inex", &inex_rec, GTEQ, "r");
	}
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (ML (mlSkMess366),20,0,1);

	print_at (0,55, ML (mlSkMess096),local_rec.previousItem);

	move (0,1);
	line (80);

	if (scn == 1)
	{
		box (0,4,80,5);
		move (1,7);
		line (79);
	}

	move (0,20);
	line (80);
	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
