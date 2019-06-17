/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_ps_cons.c,v 5.4 2002/11/28 04:09:51 scott Exp $
|  Program Name  : (so_ps_cons.c)
|  Program Desc  : (Consolidation of Packing Slips)
|---------------------------------------------------------------------|
|  Date Written  : 22/12/88        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
| $Log: so_ps_cons.c,v $
| Revision 5.4  2002/11/28 04:09:51  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.3  2001/08/23 11:46:26  scott
| Updated from scotts machine
|
| Revision 5.2  2001/08/09 09:21:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:43  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_ps_cons.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_ps_cons/so_ps_cons.c,v 5.4 2002/11/28 04:09:51 scott Exp $";

/*
 *********************************************************************
 *  Theorectical Layout of Data :                                     
 *********************************************************************
 *                                                                   
 *  list_head                                                       
 *       |                                                         
 *       v                                                        
 *  +-----------+    +-----------+    +-----------+              
 *  | core data | -> | hhso_hash | -> | hhso_hash | -> HHSO_NUL 
 *  +-----------+    +-----------+    +-----------+            
 *       |                                                    
 *       v                                                   
 *  +-----------+    +-----------+    +-----------+         
 *  | core data | -> | hhso_hash | -> | hhso_hash | -> HHSO_NUL
 *  +-----------+    +-----------+    +-----------+           
 *       |                                                   
 *       v                                                  
 *      CORE_NUL                                           
 *                                                        
 */
 #define		NO_SCRGEN
 #include	<pslscr.h>
 #include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>
#include	<CustomerService.h>

#define		NOTAX		(ssohr_rec.tax_code [0] == 'A' || \
						 ssohr_rec.tax_code [0] == 'B')

#define		FULL_SUPPLY	(sohr_rec.full_supply [0] == 'Y')

#define		RELEASE		(envBoRtype [0] =='R' || \
						 envBoRtype [0] == 'r')

#define		SUR_CHARGE	(cumr_rec.sur_flag [0] =='Y' || \
						 cumr_rec.sur_flag [0] == 'y')

#define		COMNT_LINE	(inmr_rec.inmr_class [0] == 'Z')

#define		BONUS		(coln_rec.bonus_flag [0] == 'Y')

#define		FGN_CURR   (envDbMcurr && \
					    strcmp (cumr_rec.curr_code, envCurrCode))

#define		NEW_SO		(ssohr_rec.sohr_new [0] == 'Y')

#define		CASH_INV	(cumr_rec.cash_flag [0] == 'Y')

#define		BY_BRANCH	1
#define		BY_DEPART	2

/*===============================
| List Element for hhso records	|
===============================*/
struct	hhso_type	{
	long	_hhso_hash;
	int		_del_no;				/* delivery address no 		*/
	struct	hhso_type	*_next;
};

/*===================================
| List Element for core data record |
===================================*/
struct	core_type	{
	char	_dname 		[41];		/* delivery name			*/
	char	_din 		[3][61];	/* special instructions		*/
	char	_daddr 		[3][41];	/* delivery address			*/
	char	_pay_term 	[41];		/* payment term.			*/
	char	_cont_no 	[7];
	struct	hhso_type	*_list;	/* base of hhso list		*/
	struct	core_type	*_next;
};

typedef	struct	hhso_type	HHSO;
typedef	struct	core_type	CORE;

HHSO	*hhso_free;	/* free list for hhso records	*/

CORE	*core_head;	/* pointer to head of core list	*/
CORE	*core_free;	/* free list for core records	*/

#define	HHSO_NUL	(HHSO *)0
#define	CORE_NUL	(CORE *)0

	int		processLineNo		= 0, 	
			envDbMcurr			= 0, 
			surchargeApplied	= 0, 
			contractPrice 		= FALSE, 
			envSoNumbers 		= BY_BRANCH, 
			envSoDoi 			= FALSE, 
			envSoFreightBord	= 1,
			envSkGrinNoPlate 	= 0,
			envDbNettUsed 		= TRUE;

	char	findStatusFlag 		[2], 
			updateStatusFlag 	[2], 
			envBoRtype 			[2], 
			envCurrCode 		[4],
			use_del_name 		[41],
			use_del_add 		[3][41];

	long	systemDate = 0L;

	float	quantityOrdered = 0.00,
			totalkgs 		= 0.00;


	double	totalLevy 			= 0.00, 
			totalGross 			= 0.00, 
			totalFreight 		= 0.00, 
			totalInsurance 		= 0.00, 
			totalOnCosts [3]	= {0.00,0.00,0.00}, 
			totalTax 			= 0.00, 
			totalGst 			= 0.00, 
			totalDiscount   	= 0.00, 
			totalDeposit 		= 0.00, 
			totalExtraDisc 		= 0.00, 
			totalSupplyVal 		= 0.00;

	FILE	*ford;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cudpRecord	cudp_rec;
struct esmrRecord	esmr_rec;
struct cumrRecord	cumr_rec;
struct cudiRecord	cudi_rec;
struct sohrRecord	sohr_rec;
struct sohrRecord	ssohr_rec;
struct solnRecord	soln_rec;
struct solnRecord	ssoln_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	ccohr_rec;
struct colnRecord	coln_rec;
struct inmrRecord	inmr_rec;
struct cnchRecord	cnch_rec;
struct pocrRecord	pocr_rec;
struct sonsRecord	sons_rec;
struct inlaRecord	inla_rec;
struct trshRecord	trsh_rec;
struct skniRecord	skni_rec;

	char	*data = "data", 
			*ssoln = "ssoln", 
			*ccohr = "ccohr", 
			*ssohr = "ssohr";

#include	<cus_price.h>
#include	<cus_disc.h>
#include	<proc_sobg.h>



/*=======================
| Function Declarations |
=======================*/
CORE 	*CoreAllocate 		(void);
HHSO 	*HhsoAllocate 		(void);
int  	CheckCohr 			(char *);
int  	CheckFullSupply		(long);
int  	NoValidLines		(long);
int  	SameCore 			(CORE *);
void 	AddAllCores 		(void);
void 	AddCore 			(int);
void 	CloseDB 			(void);
void 	CreateCohr 			(void);
void 	CreateDetail 		(long, int, CORE *);
void 	FreeList 			(void);
void 	GetCudiDetails 		(int);
void 	OpenDB 				(void);
void 	ProcessOrder 		(void);
void 	ProcessingRoutine 	(void);
void 	UpDetailINLA 		(long, long);
void	UpDetailSkni 		(long, long);
void 	UpDetailSONS 		(long, long);
void 	UpHeaderSONS 		(long, long);
void 	UpdateCohr 			(void);
void 	UpdateSoLines 		(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc, 
 char	*argv [])
{
	char	*sptr = chk_env ("DB_MCURR");
	if (sptr)
		envDbMcurr = atoi (sptr);
	else
		envDbMcurr = FALSE;

	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	if (argc != 3)
	{
		print_at (0, 0, mlSoMess701, argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (findStatusFlag, 	"%-1.1s", argv [1]);
	sprintf (updateStatusFlag, 	"%-1.1s", argv [2]);

	sptr = chk_env ("BO_RTYPE");
	sprintf (envBoRtype, "%-1.1s", (sptr != (char *)0) ? sptr : "I");

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check if number plates used. 
	 */
	sptr = chk_env ("SK_GRIN_NOPLATE");
	envSkGrinNoPlate = ((sptr == (char *)0)) ? 0 : atoi (sptr);
	sptr = chk_env ("SO_NUMBERS");
	envSoNumbers = (sptr == (char *)0) ? BY_BRANCH : atoi (sptr);

	sptr = chk_env ("SO_DOI");
	envSoDoi = (sptr == (char *)0 || sptr [1] == 'S') ? TRUE : FALSE;

	sptr = chk_env ("SO_FREIGHT_BORD");
	envSoFreightBord = (sptr == (char *)0) ? 1 : atoi (sptr);

	OpenDB ();				/*	Open Database and files		*/
	OpenPrice ();			/*	Open Pricing files.			*/
	OpenDisc ();			/*	Open Discount files.		*/
	ProcessingRoutine ();	/*	Process Data				*/
	ClosePrice ();			/* 	Close Pricing files.		*/
	CloseDisc ();			/* 	Close Discounting files.	*/

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr , comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr , &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);

	abc_alias (ssoln, soln);
	abc_alias (ssohr, "sohr");

	open_rec (ssoln,soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (ssohr,sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cudi, cudi_list, CUDI_NO_FIELDS, "cudi_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no3");
	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (sons, sons_list, SONS_NO_FIELDS, "sons_id_no");
	open_rec (inla, inla_list, INLA_NO_FIELDS, "inla_hhsl_id");
	open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhso_hash");
	if (envSkGrinNoPlate)
		open_rec (skni,  skni_list, SKNI_NO_FIELDS, "skni_hhsl_hash");
}

void
CloseDB (void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cumr);
	abc_fclose (cudi);
	abc_fclose (inmr);
	abc_fclose (sohr);
	abc_fclose (ssoln);
	abc_fclose (cnch);
	abc_fclose (pocr);
	abc_fclose (sons);
	abc_fclose (inla);
	abc_fclose (esmr);
	abc_fclose (cudp);
	if (envSkGrinNoPlate)
		abc_fclose (skni);
	abc_dbclose (data);
}

void
ProcessingRoutine (
 void)
{
	char	*sptr;
	int		saved 		= FALSE;
	int		exch_ok 	= FALSE;
	int		first_time 	= TRUE;
	long	hhcuHash 	= 0L;
	static  double	prev_exch;

	systemDate = TodaysDate ();

	core_head = CORE_NUL;
	core_free = CORE_NUL;
	hhso_free = HHSO_NUL;

	dsp_screen ("Consolidating Sales Orders", comm_rec.co_no, 
						comm_rec.co_name);

	/*-------------------
	| read sohr records |
	-------------------*/
	strcpy (sohr_rec.co_no,  comm_rec.co_no);
	strcpy (sohr_rec.br_no,  comm_rec.est_no);
	strcpy (sohr_rec.status, findStatusFlag);
	cc = find_rec (sohr , &sohr_rec, GTEQ, "r");
	while (!cc && !strcmp (sohr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (sohr_rec.br_no, comm_rec.est_no) &&
				  sohr_rec.status [0] == findStatusFlag [0])
	{
		char	sortStr [256];

		if (FULL_SUPPLY)
		{
			if (CheckFullSupply (sohr_rec.hhso_hash))
			{
				cc = find_rec (sohr , &sohr_rec, NEXT, "r");
				continue;
			}
		}
		if (NoValidLines (sohr_rec.hhso_hash))
		{
			cc = find_rec (sohr , &sohr_rec, NEXT, "r");
			continue;
		}
		/*---------------------------
		| nothing saved yet			|
		---------------------------*/
		if (!saved)
			ford = sort_open ("cpost");

		saved = TRUE;
	
		sprintf 
		(
			sortStr, 
			"%010ld %09.4f %010ld\n", 
			sohr_rec.hhcu_hash, 
			sohr_rec.exch_rate, 
			sohr_rec.hhso_hash
		);
		sort_save (ford, sortStr);

		dsp_process ("Reading : ", sohr_rec.order_no);

		cc = find_rec (sohr , &sohr_rec, NEXT, "r");
	}
	/*--------------------
	| nothing to process |
	--------------------*/
	if (!saved)
		return;

	abc_selfield (sohr , "sohr_hhso_hash");

	/*-----------
	| sort file |
	-----------*/
	dsp_process ("Sorting : ", " ");
	ford = sort_sort (ford, "cpost");
	while ((sptr = sort_read (ford)))
	{
		sohr_rec.hhso_hash	=	atol (sptr + 20);
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
		if (cc || sohr_rec.status [0] != findStatusFlag [0])
		{
			abc_unlock (sohr);
			continue;
		}
		dsp_process ("Consolidate : ", sohr_rec.order_no);
		/*------------------
		| find cumr record |
		------------------*/
		cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
		cc = find_rec (cumr , &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			abc_unlock (sohr);
			continue;
		}
		/*-------------------------------
		| exch_rate has to same as prev |
		-------------------------------*/
		if (first_time)
		{
			first_time = FALSE;
			exch_ok = TRUE;
			prev_exch = sohr_rec.exch_rate;
		}
		else
		{
			if (prev_exch == sohr_rec.exch_rate)
				exch_ok = TRUE;
			else
				exch_ok = FALSE;

			prev_exch = sohr_rec.exch_rate;
		}
		/*-----------------------
		| can consolidate order |
		-----------------------*/
		if (cumr_rec.bo_cons [0] == 'Y')
		{
			abc_unlock (sohr);

			if (hhcuHash == 0L)
				hhcuHash = sohr_rec.hhcu_hash;
			/*----------------------------
			| same debtor -> consolidate |
			----------------------------*/
			if (hhcuHash == sohr_rec.hhcu_hash && exch_ok)
				AddAllCores ();
			else
			{
				/*-----------------------------------
				| process the orders to consolidate	|
				-----------------------------------*/
				ProcessOrder ();
				/*----------------------------------
				| put list records onto free lists |
				----------------------------------*/
				FreeList ();
				/*---------------------------
				| add the sohr to the lists |
				---------------------------*/
				AddAllCores ();
				hhcuHash = sohr_rec.hhcu_hash;
			}
			add_hash 
			(
				comm_rec.co_no, 
				comm_rec.est_no, 
				"RO", 
				0, 
				cumr_rec.hhcu_hash, 
				0L, 
				0L, 
				(double) 0.00
			);
		}
		else
		{
			/*---------------------------
			| debtor cannot consolidate |
			---------------------------*/
			if (!cc && cumr_rec.bo_cons [0] == 'N')
			{
				strcpy (sohr_rec.status, "R");
				cc = abc_update (sohr , &sohr_rec);
				if (cc)
					file_err (cc, "sohr", "DBUPDATE");

				/*----------------------------------
				| update all lines to be released. |
				----------------------------------*/
				UpdateSoLines (sohr_rec.hhso_hash);
				add_hash 
				(
					comm_rec.co_no, 
					comm_rec.est_no, 
					"PC", 
					0, 
					sohr_rec.hhso_hash, 
					0L, 
					0L, 
					(double) 0.00
				);
			}	
		}
		abc_unlock (sohr);
	}
	/*-----------------------------------
	| consolidate the final orders		|
	-----------------------------------*/
	if (core_head)
	{
		ProcessOrder ();
		FreeList ();
	}
	sort_delete (ford, "cpost");
	recalc_sobg ();
}

/*===================================
| Place the list records used, onto	|
| the appropriate free lists.		|
===================================*/
void
FreeList (
 void)
{
	HHSO	*sptr;
	CORE	*tptr;
	/*-------------------------------
	| go through core list			|
	-------------------------------*/
	for (tptr = core_head; tptr; tptr = tptr->_next)
	{
		/*-----------------------------------
		| go through hhso list within core	|
		-----------------------------------*/
		if (tptr->_list)
		{
			/*-------------------------------
			| not empty free list			|
			-------------------------------*/
			if (hhso_free)
			{
				/*---------------------------
				| find the end				|
				---------------------------*/
				for (sptr = tptr->_list;sptr->_next;sptr = sptr->_next)
					;

				/*-----------------------------------
				| attach the free list to the end	|
				-----------------------------------*/
				sptr->_next = hhso_free;
			}
			hhso_free = tptr->_list;
		}
	}
	/*---------------------------------------
	| copy whole core list to the free list	|
	---------------------------------------*/
	core_free = core_head;
	core_head = CORE_NUL;
}

void
GetCudiDetails (
 int		del_no)
{
	if (del_no == 0)
	{
		/*-----------------------------
		| Address the same as header. |
		-----------------------------*/
		sprintf (use_del_name,   "%-40.40s", sohr_rec.del_name);
		sprintf (use_del_add [0], "%-40.40s", sohr_rec.del_add1);
		sprintf (use_del_add [1], "%-40.40s", sohr_rec.del_add2);
		sprintf (use_del_add [2], "%-40.40s", sohr_rec.del_add3);
	}
	else
	{
		/*----------------------------------
		| Look up cudi record for address. |
		----------------------------------*/
		cudi_rec.hhcu_hash = sohr_rec.hhcu_hash;
		cudi_rec.del_no = soln_rec.del_no;
		cc = find_rec (cudi, &cudi_rec, COMPARISON, "r");
		if (cc)
		{
			/*-----------------------------
			| Address the same as header. |
			-----------------------------*/
			sprintf (use_del_name, 	 "%-40.40s", sohr_rec.del_name);
			sprintf (use_del_add [0], "%-40.40s", sohr_rec.del_add1);
			sprintf (use_del_add [1], "%-40.40s", sohr_rec.del_add2);
			sprintf (use_del_add [2], "%-40.40s", sohr_rec.del_add3);
		}
		else
		{
			/*---------------------------
			| Address specific to line. |
			---------------------------*/
			sprintf (use_del_name,   "%-40.40s", cudi_rec.name);
			sprintf (use_del_add [0], "%-40.40s", cudi_rec.adr1);
			sprintf (use_del_add [1], "%-40.40s", cudi_rec.adr2);
			sprintf (use_del_add [2], "%-40.40s", cudi_rec.adr3);
		}
	}
}

/*=======================================
| Add data to the core & hhso lists for |
| delivery address on the order.        |
=======================================*/
void
AddAllCores (
 void)
{
	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		GetCudiDetails (soln_rec.del_no);
		/*------------------------------------------------
		| Add to list. Note if the core is already there |
		| and the hhso_hash is already in the list for   |
		| the core then it will NOT be added again.      |
		------------------------------------------------*/
		AddCore (soln_rec.del_no);

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
}

/*===================================
| Add data to the core & hhso lists |
===================================*/
void
AddCore (
 int del_no)
{
	int		i;
	HHSO	*sptr, *xptr;
	CORE	*tptr;
	/*---------------------------
	| check if core data exists |
	---------------------------*/
	for (tptr = core_head;tptr && !SameCore (tptr);tptr = tptr->_next)
		;
	/*-----------
	| not found |
	-----------*/
	if (!tptr)
	{
		tptr = CoreAllocate ();
		/*---------------
		| set core data |
		---------------*/
		strcpy (tptr->_dname, use_del_name);
		strcpy (tptr->_pay_term, sohr_rec.pay_term);
		strcpy (tptr->_cont_no, sohr_rec.cont_no);

		strcpy (tptr->_din [0], sohr_rec.din_1);
		strcpy (tptr->_din [1], sohr_rec.din_2);
		strcpy (tptr->_din [2], sohr_rec.din_3);

		for (i = 0;i < 3;i++)
			strcpy (tptr->_daddr [i], use_del_add [i]);
		
		/*---------------------------
		| put list record onto list |
		---------------------------*/
		tptr->_next = core_head;
		core_head = tptr;
		/*-----------------------------
		| put hhso_hash into sub_list |
		-----------------------------*/
		tptr->_list = HhsoAllocate ();
		tptr->_list->_hhso_hash = sohr_rec.hhso_hash;
		tptr->_list->_del_no = del_no;
	}
	else
	{
		/*-------------------------------------------
		| Check if hhso_hash is already in sub_list |
		-------------------------------------------*/
		xptr = tptr->_list;
		while (xptr != HHSO_NUL)
		{
			if (xptr->_hhso_hash == sohr_rec.hhso_hash)
				return;

			xptr = xptr->_next;
		}

		/*-----------------------------
		| put hhso_hash into sub_list |
		-----------------------------*/
		sptr = HhsoAllocate ();
		sptr->_hhso_hash = sohr_rec.hhso_hash;
		sptr->_del_no = del_no;
		sptr->_next = tptr->_list;
		tptr->_list = sptr;
	}
}

/*===============================
| Print the core & hhso lists	|
| for debugging only.		|
===============================*/
/***
print_list ()
{
	int		i;
	FILE	*fbug;
	HHSO	*sptr;
	CORE	*tptr = core_head;

	if ((fbug = fopen ("PS_CONS", "a")) == 0)
		return;

	fprintf (fbug, "*********: List :*********\n");
	fflush (fbug);

	if (!tptr)
		fprintf (fbug, "  Null List\n");

	fflush (fbug);

	while (tptr)
	{
		fprintf (fbug, "%s\n", tptr->_dname);
		fflush (fbug);

		fprintf (fbug, "%s\n", tptr->_pay_term);
		fflush (fbug);

		for (i = 0;i < 3;i++)
		{
			fprintf (fbug, "%s ", tptr->_daddr [i]);
			fprintf (fbug, "%s\n", tptr->_din [i]);
			fflush (fbug);
		}

		sptr = tptr->_list;
		while (sptr)
		{
			fprintf (fbug, "%06ld -> ", sptr->_hhso_hash);
			fflush (fbug);
			sptr = sptr->_next;
		}

		tptr = tptr->_next;
	}

	fclose (fbug);
}
***/

/*================================
| Check if core data is the same |
================================*/
int
SameCore (
 CORE *tptr)
{
	int		i;
	/*---------------------
	| check delivery name |
	---------------------*/
	if (strcmp (tptr->_dname, use_del_name))
		return (EXIT_SUCCESS);

	/*----------------------
	| check payment terms. |
	----------------------*/
	if (strcmp (tptr->_pay_term, sohr_rec.pay_term))
		return (EXIT_SUCCESS);

	/*----------------------------
	| check special instructions |
	----------------------------*/
	if (strcmp (tptr->_din [0], sohr_rec.din_1))
		return (EXIT_SUCCESS);

	/*----------------------------
	| check special instructions |
	----------------------------*/
	if (strcmp (tptr->_din [1], sohr_rec.din_2))
		return (EXIT_SUCCESS);

	/*----------------------------
	| check special instructions |
	----------------------------*/
	if (strcmp (tptr->_din [2], sohr_rec.din_3))
		return (EXIT_SUCCESS);

	for (i = 0;i < 3;i++)
	{
		/*------------------------
		| check delivery address |
		------------------------*/
		if (strcmp (tptr->_daddr [i], use_del_add [i]))
			return (EXIT_SUCCESS);
	}

	/*-----------------------
	| check to make sure same 
	| contract number
	-------------------*/
	if (strcmp (tptr->_cont_no, sohr_rec.cont_no))
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

/*=======================
| Consolidate orders	|
=======================*/
void
ProcessOrder (
 void)
{
	int		first_time = TRUE;
	HHSO	*sptr;
	CORE	*tptr = core_head;
	/*----------------------
	| go through core list |
	----------------------*/
	for (tptr = core_head; tptr; tptr = tptr->_next)
	{
		first_time  		= TRUE;
		totalGross 			= 0.00;
		totalLevy 			= 0.00;
		totalFreight 		= 0.00;
		totalkgs 			= 0.00;
		totalInsurance		= 0.00;
		totalOnCosts [0] 	= 0.00;
		totalOnCosts [1] 	= 0.00;
		totalOnCosts [2] 	= 0.00;
		totalTax 			= 0.00;
		totalGst 			= 0.00;
		totalDiscount 		= 0.00;
		totalDeposit 		= 0.00;
		totalExtraDisc 		= 0.00;
		processLineNo 		= 0;

		/*----------------------
		| go through hhso list |
		----------------------*/
		for (sptr = tptr->_list; sptr; sptr = sptr->_next)
		{
			ssohr_rec.hhso_hash	= sptr->_hhso_hash;
			cc = find_rec ("ssohr", &ssohr_rec, COMPARISON, "u");
			if (cc)
			{
				sprintf (err_str, "Error in ssohr (%06ld) during (DBFIND)", sptr->_hhso_hash);
				file_err (cc, err_str, PNAME);
			}

			/*------------
			| creat cohr |
			------------*/
			if (first_time)
			{
				sprintf (use_del_name,   "%-40.40s", tptr->_dname);
				sprintf (use_del_add [0], "%-40.40s", tptr->_daddr [0]);
				sprintf (use_del_add [1], "%-40.40s", tptr->_daddr [1]);
				sprintf (use_del_add [2], "%-40.40s", tptr->_daddr [2]);

				abc_selfield (sons, "sons_id_no3");
				CreateCohr ();
				abc_selfield (sons, "sons_id_no");
			}

			/*-------------------
			| sum header values |
			-------------------*/
			if (ssohr_rec.sohr_new [0] == 'Y' || !envSoFreightBord)
			{
				totalFreight 		+=	ssohr_rec.freight;
				totalInsurance 		+=  ssohr_rec.insurance;
				totalOnCosts [0] 	+=  ssohr_rec.other_cost_1;
				totalOnCosts [1] 	+=  ssohr_rec.other_cost_2;
				totalOnCosts [2] 	+=  ssohr_rec.other_cost_3;
				totalDeposit 		+=  ssohr_rec.deposit;
				totalExtraDisc 		+=  ssohr_rec.discount;
			}
			totalkgs 			+= ssohr_rec.no_kgs;

			first_time = FALSE;
			/*--------------
			| creat coln's |
			--------------*/
			CreateDetail (sptr->_hhso_hash, sptr->_del_no, tptr);
		}
		/*----------------------------
		| add the totals to the cohr |
		----------------------------*/
		if (tptr->_list)
			UpdateCohr ();
	}
}

/*===========================
| Creat Packing Slip Header |
===========================*/
void
CreateCohr (
 void)
{
	char	tmp_prefix 	 [3]; 
	char	tmp_inv_no [9];
	char	tmp_mask [12];
	long	inv_no	=	0L;
	int		len;

	/*----------------
	| get price type |
	----------------*/
	totalSupplyVal 		= 0.00;
	quantityOrdered 	= 0.00;
	surchargeApplied 	= TRUE;
	abc_selfield ("cohr", "cohr_id_no2");

	/*------------------------------------------------------
	| Is invoice number to come from department of branch. |
	------------------------------------------------------*/
	if (envSoNumbers == BY_DEPART)
	{
		strcpy (cudp_rec.co_no, ssohr_rec.co_no);
		strcpy (cudp_rec.br_no, ssohr_rec.br_no);
		strcpy (cudp_rec.dp_no, ssohr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "cudp", "DBFIND");

		inv_no	=	(CASH_INV) ? cudp_rec.nx_csh_no : cudp_rec.nx_chg_no;
		inv_no++;
	}
	else
	{
		strcpy (esmr_rec.co_no, ssohr_rec.co_no);
		strcpy (esmr_rec.est_no, ssohr_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "esmr", "DBFIND");
	
		inv_no	=	(CASH_INV) ? esmr_rec.nx_csh_inv : esmr_rec.nx_inv_no;
		inv_no++;
	}

	if (envSoNumbers == BY_BRANCH)
	{
		if (CASH_INV)
			strcpy (tmp_prefix, esmr_rec.csh_pref);
		else
			strcpy (tmp_prefix, esmr_rec.chg_pref);
	}
	else
	{
		if (CASH_INV)
			strcpy (tmp_prefix, cudp_rec.csh_pref);
		else
			strcpy (tmp_prefix, cudp_rec.chg_pref);
	}

	clip (tmp_prefix);
	len = strlen (tmp_prefix);

	sprintf (tmp_mask, "%%s%%0%dld", 8 - len);
	sprintf (tmp_inv_no, tmp_mask, tmp_prefix, inv_no);

	while (CheckCohr (tmp_inv_no) == 0)
		sprintf (tmp_inv_no, tmp_mask, tmp_prefix, inv_no++);

	if (envSoNumbers == BY_DEPART)
	{
		if (CASH_INV)
			cudp_rec.nx_csh_no	=	inv_no;
		else
			cudp_rec.nx_chg_no	=	inv_no;

		cc = abc_update (cudp, &cudp_rec);
		if (cc)
			file_err (cc, "cudp", "DBUPDATE");
	}
	else
	{
		if (CASH_INV)
			esmr_rec.nx_csh_inv	=	inv_no;
		else
			esmr_rec.nx_inv_no	=	inv_no;

		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, "esmr", "DBUPDATE");
	}
	/*--------------------------
	| add all the header stuff |
	--------------------------*/
	strcpy (cohr_rec.co_no,  ssohr_rec.co_no);
	strcpy (cohr_rec.br_no,  ssohr_rec.br_no);
	strcpy (cohr_rec.dp_no,  ssohr_rec.dp_no);
	strcpy (cohr_rec.inv_no, ccohr_rec.inv_no);

	cumr_rec.hhcu_hash	=	ssohr_rec.hhcu_hash;
	cc = find_rec (cumr , &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (err_str, "Error in cumr (%06ld) during (DBFIND)", 
							ssohr_rec.hhcu_hash);
		file_err (cc, err_str, PNAME);
	}

	cohr_rec.hhcu_hash = ssohr_rec.hhcu_hash;
	cohr_rec.hhso_hash = ssohr_rec.hhso_hash;
	cohr_rec.exch_rate = ssohr_rec.exch_rate;
	strcpy (cohr_rec.type, "P");
	
	cohr_rec.no_cartons 	=  	 ssohr_rec.no_cartons;
	cohr_rec.no_kgs     	= 	 ssohr_rec.no_kgs;
	strcpy (cohr_rec.cons_no,    ssohr_rec.cons_no);
	strcpy (cohr_rec.carr_code,  ssohr_rec.carr_code);
	strcpy (cohr_rec.carr_area,  ssohr_rec.carr_area);
	strcpy (cohr_rec.op_id,      ssohr_rec.op_id);
	strcpy (cohr_rec.cont_no,    ssohr_rec.cont_no);
	strcpy (cohr_rec.del_zone, 	 ssohr_rec.del_zone);
	strcpy (cohr_rec.del_req, 	 ssohr_rec.del_req);
	cohr_rec.del_date		=	 ssohr_rec.del_date;
	strcpy (cohr_rec.asm_req, 	 ssohr_rec.asm_req);
	cohr_rec.asm_date		=	 ssohr_rec.asm_date;
	strcpy (cohr_rec.s_timeslot, ssohr_rec.s_timeslot);
	strcpy (cohr_rec.e_timeslot, ssohr_rec.e_timeslot);
	cohr_rec.date_create 	= TodaysDate ();
	strcpy (cohr_rec.time_create, TimeHHMM ());
	strcpy (cohr_rec.cus_ord_ref, ssohr_rec.cus_ord_ref);
	strcpy (cohr_rec.frei_req,    ssohr_rec.frei_req);
	cohr_rec.date_raised   	= (envSoDoi) ? systemDate : comm_rec.dbt_date;
	cohr_rec.date_required 	= systemDate;
	strcpy (cohr_rec.tax_code,  ssohr_rec.tax_code);
	strcpy (cohr_rec.tax_no,    ssohr_rec.tax_no);
	strcpy (cohr_rec.area_code, ssohr_rec.area_code);
	strcpy (cohr_rec.sale_code, ssohr_rec.sman_code);
	cohr_rec.item_levy     	= 0.00;
	cohr_rec.gross 	    	= 0.00;
	cohr_rec.freight 		= 0.00;
	cohr_rec.no_kgs 		= 0.00;
	cohr_rec.insurance 		= 0.00;
	cohr_rec.other_cost_1 	= 0.00;
	cohr_rec.other_cost_2 	= 0.00;
	cohr_rec.other_cost_3 	= 0.00;
	cohr_rec.tax 	    	= 0.00;
	cohr_rec.gst	 		= 0.00;
	cohr_rec.disc 	    	= 0.00;
	cohr_rec.deposit 		= 0.00;
	cohr_rec.ex_disc 		= 0.00;
	strcpy (cohr_rec.fix_exch,   ssohr_rec.fix_exch);
	strcpy (cohr_rec.batch_no,   ssohr_rec.batch_no);
	strcpy (cohr_rec.dl_name,    use_del_name);
	strcpy (cohr_rec.dl_add1,    use_del_add [0]);
	strcpy (cohr_rec.dl_add2,    use_del_add [1]);
	strcpy (cohr_rec.dl_add3,    use_del_add [2]);
	strcpy (cohr_rec.din_1,      ssohr_rec.din_1);
	strcpy (cohr_rec.din_2,      ssohr_rec.din_2);
	strcpy (cohr_rec.din_3,      ssohr_rec.din_3);
	strcpy (cohr_rec.pay_terms,  ssohr_rec.pay_term);
	strcpy (cohr_rec.sell_terms, ssohr_rec.sell_terms);
	strcpy (cohr_rec.ins_det,    ssohr_rec.ins_det);
	strcpy (cohr_rec.pri_type,   ssohr_rec.pri_type);
	strcpy (cohr_rec.prt_price,  ssohr_rec.prt_price);
	strcpy (cohr_rec.ord_type,   ssohr_rec.ord_type);
	strcpy (cohr_rec.status,     updateStatusFlag);
	strcpy (cohr_rec.stat_flag,  updateStatusFlag);
	strcpy (cohr_rec.ps_print,   "N");
	strcpy (cohr_rec.inv_print,  "N");
	cc = abc_add ("cohr", &cohr_rec);
	if (cc)
		file_err (cc, "cohr", "DBADD");

	/*--------------------------------
	| refind cohr for cohr_hhco_hash |
	--------------------------------*/
	cc = find_rec ("cohr", &cohr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "cohr", "DBFIND");

	UpHeaderSONS (ssohr_rec.hhso_hash, cohr_rec.hhco_hash);
	UpdateSosf 	 
	(
		cohr_rec.hhcu_hash, 
		ssohr_rec.hhso_hash, 
		cohr_rec.hhco_hash
	);
	/*-------------------------------------------
	| Create a log file record for sales Order. |
	-------------------------------------------*/
	LogCustService 
	(
		cohr_rec.hhco_hash, 
		ssohr_rec.hhso_hash, 
		cohr_rec.hhcu_hash, 
		cohr_rec.cus_ord_ref, 
		cohr_rec.cons_no, 
		cohr_rec.carr_code, 
		cohr_rec.del_zone, 
		LOG_PCCREATE
	);
}

/*===============================
| Creat Packing Slip Details	|
===============================*/
void
CreateDetail (
	long	hhsoHash, 
 	int		del_no, 
 	CORE 	*tptr)
{
	double	s_order;
	double	l_levy;
	double	l_total;
	double	t_total;
	double	l_disc;
	double	l_tax;
	double	l_gst;
	double	gross_pri 	= 0.00;
	double	net_pri 	= 0.00;

	int		pType;
	int		cumDisc;
	float	regPc;
	float	discArray [3];
	
	/*------------------------------------------------------
	| Read contract if order header has a contract number. |
	------------------------------------------------------*/
	if (strcmp (sohr_rec.cont_no, "      "))
	{
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, sohr_rec.cont_no);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "cnch", "DBFIND");
	}
	else
	{
		cnch_rec.hhch_hash = 0L;
		strcpy (cnch_rec.exch_type, " ");
	}
	/*--------------------------------------------
	| Find currency record for debtors currency. |
	--------------------------------------------*/
	if (envDbMcurr)
	{
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", cumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		if (pocr_rec.ex1_factor == 0.00)
			pocr_rec.ex1_factor = 1.00; /* converted to assignment statement */
	}
	/*-------------------------
	| process all order lines |
	-------------------------*/
	soln_rec.hhso_hash	= hhsoHash;
	soln_rec.line_no	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		/*-----------------------------------
		| forward or order line not M (anual |
		-----------------------------------*/
		if (soln_rec.status [0] != findStatusFlag [0])
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}
		/*----------------------------------------------------
		| Delivery address does not match this packing slip. |
		----------------------------------------------------*/
		if (soln_rec.del_no != del_no)
		{
			GetCudiDetails (soln_rec.del_no);
			if (strcmp (use_del_name, tptr->_dname)
			||  strcmp (use_del_add [0], tptr->_daddr [0])
			||  strcmp (use_del_add [1], tptr->_daddr [1])
			||  strcmp (use_del_add [2], tptr->_daddr [2]))
			{
				abc_unlock (soln);
				cc = find_rec (soln, &soln_rec, NEXT, "u");
				continue;
			}
		}
		/*------------------
		| find item master |
		------------------*/
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}
		/*-----------------
		| add coln record |
		-----------------*/
		strcpy (coln_rec.bonus_flag, soln_rec.bonus_flag);
		strcpy (coln_rec.hide_flag, soln_rec.hide_flag);
		strcpy (coln_rec.serial_no, soln_rec.serial_no);
		coln_rec.hhco_hash 		= cohr_rec.hhco_hash;
		coln_rec.line_no 		= processLineNo++;
		coln_rec.hhbr_hash 		= soln_rec.hhbr_hash;
		coln_rec.hhsl_hash 		= soln_rec.hhsl_hash;
		coln_rec.incc_hash 		= soln_rec.hhcc_hash;
		coln_rec.hhum_hash 		= soln_rec.hhum_hash;
		coln_rec.qty_org_ord 	= soln_rec.qty_org_ord;
		coln_rec.q_order 		= soln_rec.qty_order;
		coln_rec.q_backorder 	= soln_rec.qty_bord;
		coln_rec.cont_status 	= soln_rec.cont_status;
		coln_rec.gsale_price 	= soln_rec.gsale_price;
		coln_rec.reg_pc 		= soln_rec.reg_pc;
		coln_rec.disc_pc 		= soln_rec.dis_pc;
		coln_rec.disc_a 		= soln_rec.disc_a;
		coln_rec.disc_b 		= soln_rec.disc_b;
		coln_rec.disc_c 		= soln_rec.disc_c;
		coln_rec.cumulative		= soln_rec.cumulative;
		coln_rec.cost_price 	= soln_rec.cost_price;
		coln_rec.item_levy 		= soln_rec.item_levy;

		pType = atoi (ssohr_rec.pri_type);
		gross_pri	= GetCusPrice 
					  (
						comm_rec.co_no, 
						comm_rec.est_no, 
						comm_rec.cc_no, 
						sohr_rec.area_code,
						cumr_rec.class_type, 
						inmr_rec.sellgrp, 
						cumr_rec.curr_code, 
						pType, 
						cumr_rec.disc_code, 
						cnch_rec.exch_type, 
						cumr_rec.hhcu_hash, 
						soln_rec.hhcc_hash, 
						soln_rec.hhbr_hash, 
						inmr_rec.category, 
						cnch_rec.hhch_hash, 
						(envSoDoi) ?  TodaysDate () : comm_rec.dbt_date, 
						soln_rec.qty_order + soln_rec.qty_bord, 
						pocr_rec.ex1_factor, 
						FGN_CURR, 
						&regPc
					);

		net_pri = GetCusGprice (gross_pri, regPc);

		contractPrice = (_CON_PRICE) ? TRUE : FALSE;

		if (BONUS)
			coln_rec.sale_price = 0.00;
		else
		{
			/*--------------------------------------------
			| Contract is blank so recalculate discount. |
			--------------------------------------------*/
			if (RELEASE && !coln_rec.cont_status && soln_rec.pri_or [0] == 'N') 
				coln_rec.sale_price = net_pri;
			else
				coln_rec.sale_price = soln_rec.sale_price;
		}
		if (BONUS)
		{
			coln_rec.sale_price 	= 0.00;
			coln_rec.reg_pc 		= 0.00;
			coln_rec.disc_pc 		= 0.00;
			coln_rec.disc_a 		= 0.00;
			coln_rec.disc_b 		= 0.00;
			coln_rec.disc_c 		= 0.00;
			coln_rec.cumulative		= 0;
		}
		else
		{
			/*----------------------------------------------------
			| If release time determins price AND not a contract |
            | AND discount has not been overiden.                |
			----------------------------------------------------*/
			if (RELEASE && !coln_rec.cont_status && soln_rec.dis_or [0] == 'N') 
			{
				cumDisc		=	GetCusDisc 
								(
									comm_rec.co_no, 
									comm_rec.est_no, 
									soln_rec.hhcc_hash, 
									cumr_rec.hhcu_hash, 
									cumr_rec.class_type, 
									cumr_rec.disc_code, 
									soln_rec.hhbr_hash, 
									inmr_rec.category, 
									inmr_rec.sellgrp, 
									pType, 
									gross_pri, 
									regPc, 
									soln_rec.qty_order + 
									soln_rec.qty_bord, 
									discArray
								);

				coln_rec.disc_pc	=	CalcOneDisc 
										(
											cumDisc, 
											discArray [0], 
											discArray [1], 
											discArray [2]
										);
				coln_rec.reg_pc 	= 	regPc;
				coln_rec.disc_a 	= 	discArray [0];
				coln_rec.disc_b 	= 	discArray [1];
				coln_rec.disc_c 	= 	discArray [2];
				coln_rec.cumulative = 	cumDisc;

				/*-------------------------------
				| check against master override |
				-------------------------------*/
				if (inmr_rec.disc_pc > coln_rec.disc_pc &&
				   	inmr_rec.disc_pc != 0.0)
				{
					coln_rec.disc_pc = inmr_rec.disc_pc;
					coln_rec.disc_a  = inmr_rec.disc_pc;
					coln_rec.disc_b  = 0.00;
					coln_rec.disc_c  = 0.00;
				}
			}
		}
		coln_rec.tax_pc = (float) ((NOTAX) ? 0.00 : soln_rec.tax_pc);
		coln_rec.gst_pc = (float) ((NOTAX) ? 0.00 : soln_rec.gst_pc);
		coln_rec.o_xrate = soln_rec.o_xrate;
		coln_rec.n_xrate = soln_rec.n_xrate;
		strcpy (coln_rec.pack_size, soln_rec.pack_size);
		strcpy (coln_rec.sman_code, soln_rec.sman_code);
		strcpy (coln_rec.cus_ord_ref, soln_rec.cus_ord_ref);
		strcpy (coln_rec.item_desc, soln_rec.item_desc);
		coln_rec.due_date = soln_rec.due_date;
		strcpy (coln_rec.status, updateStatusFlag);
		strcpy (coln_rec.stat_flag, updateStatusFlag);
		/*---------------------------
		| calcs for header			|
		---------------------------*/
		l_total = coln_rec.q_order * out_cost (coln_rec.sale_price, 
						        	 inmr_rec.outer_size);
		l_total = no_dec (l_total);

		t_total = coln_rec.q_order * out_cost (inmr_rec.tax_amount, 
									 inmr_rec.outer_size);
		t_total = no_dec (t_total);

		l_levy	= coln_rec.item_levy;

		quantityOrdered += coln_rec.q_order;
		s_order = (coln_rec.q_order + coln_rec.q_backorder) * 
			out_cost (coln_rec.sale_price, inmr_rec.outer_size);

		totalSupplyVal += s_order;
		totalSupplyVal = no_dec (totalSupplyVal);

		l_disc = (double) (coln_rec.disc_pc / 100.00);
		l_disc *= l_total;
		l_disc = no_dec (l_disc);

		l_tax = (double) (coln_rec.tax_pc / 100.00);

		if (cumr_rec.tax_code [0] == 'D')
			l_tax *= t_total;
		else
		{
			if (envDbNettUsed)
				l_tax *= (l_total - l_disc) + l_levy;
			else
				l_tax *= l_total + l_levy;
		}

		l_tax = no_dec (l_tax);

		l_gst = (double) (coln_rec.gst_pc / 100.00);
		if (envDbNettUsed)
			l_gst *= ((l_total - l_disc) + l_tax + l_levy);
		else
			l_gst *= (l_total + l_tax + l_levy);

		coln_rec.gross    = l_total;
		coln_rec.amt_disc = l_disc;
		coln_rec.amt_tax  = l_tax;
		coln_rec.amt_gst  = l_gst;

		totalLevy 		+= l_levy;
		totalGross 		+= l_total;
		totalDiscount  	+= l_disc;
		totalTax   		+= l_tax;
		totalGst   		+= l_gst;

		cc = abc_add ("coln", &coln_rec);
		if (cc)
			file_err (cc, "coln", "DBADD");

		cc = find_rec (coln, &coln_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "coln", "DBFIND");

		if (envSkGrinNoPlate)
			UpDetailSkni (soln_rec.hhsl_hash, coln_rec.hhcl_hash);
		
		UpDetailSONS (soln_rec.hhsl_hash, coln_rec.hhcl_hash);
		UpDetailINLA (soln_rec.hhsl_hash, coln_rec.hhcl_hash);

		/*---------------------------------------
		| if all of line placed on b/o then	    |
		| update status line as a b/o. Non	    |
		| stock items can wait for confirmation	|
		---------------------------------------*/
	    if (soln_rec.qty_order == 0.00 && !COMNT_LINE)
		{
			soln_rec.qty_order = soln_rec.qty_bord;
			soln_rec.qty_bord = 0.00;
			if (soln_rec.qty_order == 0.00)
			{
				strcpy (soln_rec.status, "D");
				strcpy (soln_rec.stat_flag, "D");
			}
			else
			{
				strcpy (soln_rec.status, "B");
				strcpy (soln_rec.stat_flag, "B");

				add_hash 
				(
					comm_rec.co_no, 
					comm_rec.est_no, 
					"RC", 
					0, 
					soln_rec.hhbr_hash, 
					soln_rec.hhcc_hash, 
					0L, 
					(double) 0.00
				);
			}
		}
		else
			strcpy (soln_rec.status, updateStatusFlag);

		/*--------------------
		| update soln record |
		--------------------*/
		cc = abc_update (soln, &soln_rec);
		if (cc)
			file_err (cc, soln, "DBUPDATE");

		cc = find_rec (soln, &soln_rec, NEXT, "u");
	}
	/*------------------------------------
	| if backorder in consolidation then |
	| no backorders.                     |
	------------------------------------*/
	if (ssohr_rec.sohr_new [0] == 'N')
		surchargeApplied = FALSE;

	/*-------------
	| update sohr |
	-------------*/
	strcpy (ssohr_rec.inv_no, cohr_rec.inv_no);
	strcpy (ssohr_rec.status, updateStatusFlag);
	cc = abc_update ("ssohr", &ssohr_rec);
	if (cc)
		file_err (cc, "ssohr", "DBUPDATE");
}

/*===========================
| Update the cohr details	|
===========================*/
void
UpdateCohr (
 void)
{
	double	value;
	double	wk_value;
	/*--------------
	| g.s.t. value |
	--------------*/
	if (NOTAX)
		wk_value = 0.00;
	else
		wk_value = (double) (comm_rec.gst_rate / 100.00);

	/*-----------------------
	| small order surcharge |
	-----------------------*/
	if (SUR_CHARGE && surchargeApplied && quantityOrdered > 0.00 && 
		totalSupplyVal < comr_rec.sur_cof && totalSupplyVal > 0.00)
	{
		cohr_rec.sos = comr_rec.sur_amt;
	}
	else
		cohr_rec.sos = 0.00;
	/*------------------
	| value for g.s.t. |
	------------------*/
	value = totalFreight  	+ totalInsurance 	+ totalOnCosts [0] + 
	        totalOnCosts [1] + totalOnCosts [2]  	+ cohr_rec.sos;

	/*-------------------
	| set header fields |
	-------------------*/
	cohr_rec.item_levy 		= totalLevy;
	cohr_rec.gross 			= totalGross;
	cohr_rec.tax 			= totalTax;
	cohr_rec.gst 			= totalGst;
	cohr_rec.other_cost_1 	= totalOnCosts [0];
	cohr_rec.other_cost_2 	= totalOnCosts [1];
	cohr_rec.other_cost_3 	= totalOnCosts [2];
	cohr_rec.freight 		= totalFreight;
	cohr_rec.no_kgs 		= totalkgs;
	cohr_rec.insurance 		= totalInsurance;
	cohr_rec.deposit 		= totalDeposit;
	cohr_rec.ex_disc		= totalExtraDisc;
	cohr_rec.disc 			= totalDiscount;

	/*-------------
	| calc g.s.t. |
	-------------*/
	wk_value *= value;
	wk_value = no_dec (wk_value);
	cohr_rec.gst += wk_value;
	cohr_rec.gst = no_dec (cohr_rec.gst);
	cc = abc_update ("cohr", &cohr_rec);
	if (cc)
		file_err (cc, "cohr", "DBUPDATE");
}

/*===============================
| Get valid packing slip number	|
===============================*/
int
CheckCohr (
	char	*invoiceNumber)
{
	/*-----------------------------------
	| check for existing packing slip	|
	-----------------------------------*/
	strcpy (ccohr_rec.co_no, ssohr_rec.co_no);
	strcpy (ccohr_rec.br_no, ssohr_rec.br_no);
	strcpy (ccohr_rec.type, "P");
	sprintf (ccohr_rec.inv_no, "%-8.8s", invoiceNumber);
	cc = find_rec ("cohr", &ccohr_rec, COMPARISON, "r");
	if (!cc)
		return (cc);

	/*-----------------------------------
	| check for existing packing slip	|
	-----------------------------------*/
	strcpy (ccohr_rec.co_no, ssohr_rec.co_no);
	strcpy (ccohr_rec.br_no, ssohr_rec.br_no);
	strcpy (ccohr_rec.type, "T");
	sprintf (ccohr_rec.inv_no, "%-8.8s", invoiceNumber);
	cc = find_rec ("cohr", &ccohr_rec, COMPARISON, "r");
	if (!cc)
		return (cc);

	/*---------------------------------------
	| check for existing inspection slip	|
	---------------------------------------*/
	strcpy (ccohr_rec.co_no, ssohr_rec.co_no);
	strcpy (ccohr_rec.br_no, ssohr_rec.br_no);
	strcpy (ccohr_rec.type, "S");
	sprintf (ccohr_rec.inv_no, "%-8.8s", invoiceNumber);
	cc = find_rec ("cohr", &ccohr_rec, COMPARISON, "r");
	if (!cc)
		return (cc);

	/*-----------------------------------
	| check for existing invoice		|
	-----------------------------------*/
	strcpy (ccohr_rec.co_no, ssohr_rec.co_no);
	strcpy (ccohr_rec.br_no, ssohr_rec.br_no);
	strcpy (ccohr_rec.type, "I");
	sprintf (ccohr_rec.inv_no, "%-8.8s", invoiceNumber);
	return (find_rec ("cohr", &ccohr_rec, COMPARISON, "r"));
}

void
UpdateSoLines (
	long	hhsoHash)
{
	/*-------------------------------
	| update order lines			|
	-------------------------------*/
	ssoln_rec.hhso_hash = hhsoHash;
	ssoln_rec.line_no 	= 0;
	cc = find_rec ("ssoln", &ssoln_rec, GTEQ, "u");
	while (!cc && ssoln_rec.hhso_hash == hhsoHash)
	{
		/*---------------------------------------
		| update to released if status is ok.	|
		---------------------------------------*/
		if (ssoln_rec.status [0] != findStatusFlag [0])
		{
			abc_unlock ("ssoln");
			cc = find_rec ("ssoln", &ssoln_rec, NEXT, "u");
			continue;
		}
		strcpy (ssoln_rec.status, "R");
		cc = abc_update ("ssoln", &ssoln_rec);
		if (cc)
			file_err (cc, "ssoln", "DBUPDATE");

		cc = find_rec ("ssoln", &ssoln_rec, NEXT, "u");
	}
}

/*===========================
| Allocate a hhso record	|
| from free list or memory	|
===========================*/
HHSO *
HhsoAllocate (
 void)
{
	HHSO	*tptr;
	/*---------------------------
	| empty free list			|
	---------------------------*/
	if (!hhso_free)
	{
		tptr = (HHSO *) malloc ((unsigned) sizeof (HHSO));
		if (!tptr)
			file_err (-1, "Error in hhso_list during (MALLOC)", PNAME);
	}
	else
	{
		tptr = hhso_free;
		hhso_free = hhso_free->_next;
	}
	tptr->_next = HHSO_NUL;
	return (tptr);
}

/*
 * Allocate a core record from free list or memory
 */
CORE *
CoreAllocate (
 void)
{
	CORE	*tptr;
	/*---------------------------
	| empty free list			|
	---------------------------*/
	if (!core_free)
	{
		tptr = (CORE *) malloc ((unsigned) sizeof (CORE));
		if (!tptr)
			file_err (-1, "Error in core_list during (MALLOC)", PNAME);
	}
	else
	{
		tptr = core_free;
		core_free = core_free->_next;
	}
	tptr->_next = CORE_NUL;
	return (tptr);
}

int
CheckFullSupply (
	long	hhsoHash)
{
	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		if (soln_rec.status [0] != findStatusFlag [0])
		{
			/*-----------------------------------------------
			| Some stock backordered or line is backordered |
			-----------------------------------------------*/
			if (soln_rec.qty_bord > 0.00 || soln_rec.status [0] == 'B')
				return (TRUE);
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (FALSE);
}
/*===================================================================
| This function was added as coln lines cannot be created if status |
| is incorrect so why add header ?									|
===================================================================*/
int
NoValidLines (
	long	hhsoHash)
{
	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		/*---------------------------------
		| Line with correct status so ok. |
		---------------------------------*/
		if ((soln_rec.status [0] == findStatusFlag [0]) && 
			(soln_rec.qty_order + soln_rec.qty_bord > 0.0))
			return (FALSE);
		
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (TRUE);
}

/*========================================================
| Updated sales order line description to packing lines. |
========================================================*/
void	
UpDetailINLA (
	long	hhslHash, 
	 long	hhclHash)
{
	inla_rec.hhsl_hash 	= hhslHash;
	inla_rec.inlo_hash	= 0L;
	cc = find_rec (inla, &inla_rec, GTEQ, "u");
	while (!cc && inla_rec.hhsl_hash == hhslHash)
	{
		inla_rec.hhcl_hash = hhclHash;
		cc = abc_update (inla, &inla_rec);
		if (cc)
			file_err (cc, inla, "DBUPDATE");

		cc = find_rec (inla, &inla_rec, NEXT, "u");
	}
	abc_unlock (inla);
}

/*
 * Updated number plates lines allocated to sales order lines.
 */
void	
UpDetailSkni (
	long	hhslHash,
	long	hhclHash)
{
	skni_rec.hhsl_hash 	= hhslHash;
	cc = find_rec (skni, &skni_rec, GTEQ, "u");
	while (!cc && skni_rec.hhsl_hash == hhslHash)
	{
		skni_rec.hhcl_hash = hhclHash;
		cc = abc_update (skni, &skni_rec);
		if (cc)
			abc_unlock (skni);

		cc = find_rec (skni, &skni_rec, NEXT, "u");
	}
	abc_unlock (skni);
}
/*========================================================
| Updated sales order line description to packing lines. |
========================================================*/
void	
UpDetailSONS (
	long	hhslHash, 
	long	hhclHash)
{
	sons_rec.hhsl_hash 	= hhslHash;
	sons_rec.line_no 	= 0;
	cc = find_rec (sons, &sons_rec, GTEQ, "u");
	while (!cc && sons_rec.hhsl_hash == hhslHash)
	{
		sons_rec.hhcl_hash = hhclHash;
		cc = abc_update (sons, &sons_rec);
		if (cc)
			file_err (cc, sons, "DBUPDATE");

		cc = find_rec (sons, &sons_rec, NEXT, "u");
	}
	abc_unlock (sons);
}
/*===========================================================
| Updated sales order header description to packing header. |
===========================================================*/
void	
UpHeaderSONS (
	long	hhsoHash, 
	long	hhcoHash)
{
	sons_rec.hhso_hash 	= hhsoHash;
	sons_rec.line_no 	= 0;
	cc = find_rec (sons, &sons_rec, GTEQ, "u");
	while (!cc && sons_rec.hhso_hash == hhsoHash)
	{
		sons_rec.hhco_hash = hhcoHash;
		cc = abc_update (sons, &sons_rec);
		if (cc)
			file_err (cc, sons, "DBUPDATE");

		cc = find_rec (sons, &sons_rec, NEXT, "u");
	}
	abc_unlock (sons);
}
