/*=====================================================================
|  Copyright (C) 1999 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: bord_del.c,v 5.4 2001/10/23 07:16:35 scott Exp $
|  Program Name  : (so_bord_del.c) 
|  Program Desc  : (Selective Order Line Delete BY Item Order)
|                  (No or Customer)
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 09/11/88         |
|---------------------------------------------------------------------|
| $Log: bord_del.c,v $
| Revision 5.4  2001/10/23 07:16:35  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.3  2001/08/28 06:13:48  scott
| Updated to change " ( to "(
|
| Revision 5.2  2001/08/09 09:20:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:50:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:51  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.3  2001/06/11 05:04:52  cha
| Updated to correct typo error in pcwo_hhsl_hash
|
| Revision 4.2  2001/05/31 10:01:51  scott
| Updated to ensure works order is checked before delete alowed.
|
| Revision 4.1  2001/04/21 03:53:40  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to add sleep delay - did not work with LS10-GUI
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bord_del.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_bord_del/bord_del.c,v 5.4 2001/10/23 07:16:35 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define	SLEEP_TIME	2

#define	ITEM		0
#define	ORDER		1
#define	CUSTOMER	2
#define	NON_STOCK	 (inmr_rec.class [0] == 'Z')
#define	MAXDELETE	2000
#define	SERIAL_ITEM	 (inmr_rec.serial_item [0] == 'Y')
#define	FGN_CURR	 (strcmp (cumr_rec.curr_code, envCurrCode))
#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct sohrRecord	sohr2_rec;
struct solnRecord	soln_rec;
struct solnRecord	soln2_rec;
struct inmrRecord	inmr_rec;
struct insfRecord	insf_rec;
struct cumrRecord	cumr_rec;
struct pocrRecord	pocr_rec;
struct sobgRecord	sobg_rec;
struct exlsRecord	exls_rec;
struct inlsRecord	inls_rec;
struct inlaRecord	inla_rec;
struct pcwoRecord	pcwo_rec;

	char	*soln2	=	"soln2",
			*sohr2	=	"sohr2";

	long	addItem [MAXDELETE],
			addCust [MAXDELETE];

	int		printerNumber	= 1,
			envDbCo 		= 0,
			envDbFind 		= 0,
			envSoWoAllowed 	= 0,
			firstTime 		= 1,
			deleteBy 		= 0,
			auditOpen 		= FALSE,
			selectionMade  	= FALSE;

	char	validStatus 	[5],
			envCurrCode 	[4],
			branchNumber 	[3];

	FILE	*pp;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy 		[11];
	char	order 		[9];
	char	cust 		[7];
	char 	rep_type 	[4] [2];
	char 	repTypeDesc [4] [8];
	char 	status 		[2];
	char 	statusDesc 	[12];
} local_rec;

static	struct	var	vars []	={	
	{1, LIN, "sel_item", 4, 25, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Delete By Item.", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.rep_type [0]}, 
	{1, LIN, "sel_item_desc", 4, 28, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.repTypeDesc [0]}, 

	{1, LIN, "sel_order", 5, 25, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Delete By Orders.", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.rep_type [1]}, 
	{1, LIN, "sel_order_desc", 5, 28, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.repTypeDesc [1]}, 

	{1, LIN, "sel_cust", 6, 25, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Delete By Customer.", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTLEFT, "YyNn", "", local_rec.rep_type [2]}, 
	{1, LIN, "sel_cust_desc", 6, 28, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.repTypeDesc [2]}, 

	{1, LIN, "item_no", 8, 25, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Item Number ", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.item_no}, 
	{1, LIN, "desc", 9, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description ", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	{1, LIN, "ord_no", 10, 25, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", " ", "Order Number ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.order}, 
	{1, LIN, "dbt_no", 11, 25, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Customer No ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cust}, 
	{1, LIN, "name", 12, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Customer Name ", " ", 
		NA, NO, JUSTLEFT, "", "", cumr_rec.dbt_name}, 
	{1, LIN, "ord_stat", 14, 25, CHARTYPE, 
		"U", "          ", 
		" ", "M", "Order Status.", "Enter F(orward,B(ackorder,M(anual,H(eld or '*' for all previous.", 
		YES, NO, JUSTLEFT, "FBMH*", "", local_rec.status}, 
	{1, LIN, "ord_status_desc", 14, 28, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.statusDesc}, 
	{1, LIN, "ls_yn",	 16, 25, CHARTYPE,
		"U", "          ",
		" ", "", "Log to Lost Sales (Y/N).", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type [3]},
	{1, LIN, "ls_yn_desc", 16, 28, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.repTypeDesc [3]}, 
	{1, LIN, "ls_code",	 17, 25, CHARTYPE,
		"UU", "          ",
		" ", "", "Code.", "Enter Lost Sale Code ",
		YES, NO,  JUSTLEFT, "", "", exls_rec.code},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include	<FindCumr.h>
/*======================= 
| Function Declarations |
=======================*/
int 	DeleteLine 			(void);
int  	heading 			(int);
int  	spec_valid 			(int);
int		CheckWorksOrder 	(long);
void 	AddSobg 			(int, char *, long);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
void 	DeleteAllocation 	(long);
void 	DeleteCustomer 		(void);
void 	DeleteHeader 		(long);
void 	DeleteItem 			(void);
void 	DeleteOrder 		(void);
void 	LogLostSales 		(long, long, long, char *, char *, float);
void 	OpenAudit 			(void);
void 	OpenDB 				(void);
void 	PrintDetail 		(void);
void 	ProcessData 		(void);
void 	SetLable 			(void);
void 	SetupDefault 		(void);
void 	SrchExls 			(char *);
void 	SrchSohr 			(char *);
void 	UpdateInsf 			(long, char *);
void 	UpdateOther 		(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;
	int	i;

	if (argc != 2)	
	{
		print_at (0,0,"Usage: %s <lpno>",argv [0]);
		return (EXIT_FAILURE);
	}

	/*--------------------------
	| Get local currency code. |
	--------------------------*/
	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	for (i = 0; i < MAXDELETE; i++)
	{
		addItem [i] = 0L;
		addCust [i] = 0L;
	}

	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	(); 
	set_masks 	();

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	/*----------------------------------------------------------
	| Check if works order can be created from backorder line. |
	----------------------------------------------------------*/
	sptr = chk_env ("SO_WO_ALLOWED");
	envSoWoAllowed = (sptr == (char *)0) ? 0 : atoi (sptr);

	OpenDB ();

	printerNumber = atoi (argv [1]);
	
	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	/*  reset control flags  */

	firstTime = 1;

	init_vars (1);	

	SetupDefault ();

	entry_exit		= FALSE;
	restart 		= FALSE;
	edit_exit 		= FALSE;
	prog_exit 		= FALSE;
	search_ok 		= TRUE;
	init_ok 		= TRUE;
	selectionMade 	= FALSE;

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);

	if (!restart && selectionMade)
	{
		swide ();
		ProcessData ();
		UpdateOther ();
		snorm ();
	}
	if (auditOpen)
		CloseAudit ();

	CloseDB (); 
	FinishProgram ();
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

	abc_alias (soln2, soln);
	abc_alias (sohr2, sohr);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDbFind) ? "cumr_id_no3" 
							   : "cumr_id_no");

	open_rec (sohr2,sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (soln2,soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_hhbr_id");
	open_rec (sobg, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (exls, exls_list, EXLS_NO_FIELDS, "exls_id_no");
	open_rec (inls, inls_list, INLS_NO_FIELDS, "inls_id_no");
	open_rec (inla, inla_list, INLA_NO_FIELDS, "inla_hhsl_id");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhsl_hash");
 }

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (soln2);
	abc_fclose (insf);
	abc_fclose (cumr);
	abc_fclose (pocr);
	abc_fclose (exls);
	abc_fclose (inls);
	abc_fclose (inla);
	abc_fclose (pcwo);
	abc_dbclose ("data");
}

void
SetupDefault (
 void)
{
	strcpy (local_rec.rep_type [0],"N");
	strcpy (local_rec.repTypeDesc [0], ML("NO"));

	strcpy (local_rec.rep_type [1],"Y");
	strcpy (local_rec.repTypeDesc [1], ML("YES"));

	strcpy (local_rec.rep_type [2],"N");
	strcpy (local_rec.repTypeDesc [2], ML("NO"));

	strcpy (local_rec.rep_type [3],"Y");
	strcpy (local_rec.repTypeDesc [3], ML("YES"));

	strcpy (local_rec.status,"*");
	strcpy (local_rec.statusDesc, ML("All Avail"));

	strcpy (validStatus,"BFMH");
	deleteBy = 1;
}

int
spec_valid (
 int field)
{
	char	valid_inp [2];
	int	i;

	if (last_char == EOI)
	{
		prog_exit = 1;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sel_item") || LCHECK ("sel_order") || LCHECK ("sel_cust"))
	{
		int		offset;

		if (LCHECK ("sel_item"))
			offset = 0;
		else if (LCHECK ("sel_order"))
			offset = 1;
		else
			offset = 2;

		if (local_rec.rep_type [offset] [0] == 'Y')
		{
			for (i = 0; i < 3; i++)
			{
				strcpy (local_rec.rep_type [i], "N");
				strcpy (local_rec.repTypeDesc [i], ML ("NO"));
			}
			strcpy (local_rec.rep_type [offset], "Y");
			strcpy (local_rec.repTypeDesc [offset], ML ("YES"));

			deleteBy = offset;
		}
		else
		{
			strcpy (local_rec.rep_type [offset], "N");
			strcpy (local_rec.repTypeDesc [offset], ML ("NO"));

			if (local_rec.rep_type [0] [0] == 'N' &&
				local_rec.rep_type [1] [0] == 'N' &&
				local_rec.rep_type [2] [0] == 'N')
			{
				strcpy (local_rec.rep_type [0], "Y");
				strcpy (local_rec.repTypeDesc [0], ML ("YES"));

				deleteBy = 0;
			}
		}

		for (i = label ("sel_item"); i < label ("sel_item") + 6; i++)
			display_field (i);

		SetLable ();
	}

	if (LCHECK ("ls_yn"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.rep_type [3]);

		if (local_rec.rep_type [3] [0] == 'Y')
			strcpy (local_rec.repTypeDesc [3], ML ("YES"));
		else
			strcpy (local_rec.repTypeDesc [3], ML ("NO"));

		DSP_FLD ("ls_yn_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{
		abc_selfield (inmr,"inmr_id_no");

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
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

		DSP_FLD ("item_no");
		DSP_FLD ("desc");
		selectionMade = TRUE;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ord_no"))
	{
		abc_selfield (sohr,"sohr_id_no2");
		abc_selfield (cumr,"cumr_hhcu_hash");
		if (SRCH_KEY)
		{
			SrchSohr (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (sohr_rec.co_no,comm_rec.co_no);
		strcpy (sohr_rec.br_no,comm_rec.est_no);
		strcpy (sohr_rec.order_no, zero_pad (local_rec.order, 8));

		cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess122));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!strchr (validStatus, sohr_rec.status [0]))
		{
			print_mess (ML ("Order status is not valid for deletion."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code,  cumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			pocr_rec.ex1_factor = 1.00;

		if (CheckWorksOrder (sohr_rec.hhso_hash))
		{
			print_mess (ML ("Order still has an active works order"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	
		strcpy (local_rec.cust,cumr_rec.dbt_no);
		DSP_FLD ("ord_no");
		DSP_FLD ("dbt_no");
		DSP_FLD ("name");
		abc_selfield (cumr, (envDbFind) ? "cumr_id_no3" : "cumr_id_no");
		selectionMade = TRUE;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dbt_no")) 
	{
		abc_selfield (cumr, (envDbFind) ? "cumr_id_no3" : "cumr_id_no");

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.cust));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code,  cumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			pocr_rec.ex1_factor = 1.00;

		strcpy (local_rec.cust,cumr_rec.dbt_no);
		DSP_FLD ("dbt_no");
		DSP_FLD ("name");
		selectionMade = TRUE;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ord_stat"))
	{
		switch (local_rec.status [0])
		{
		case	'F':
			strcpy (local_rec.statusDesc, ML ("Forward"));
			strcpy (validStatus, "F");
			break;

		case	'B':
			strcpy (local_rec.statusDesc,ML ("Backorder"));
			strcpy (validStatus, "B");
			break;

		case	'M':
			strcpy (local_rec.statusDesc,ML ("Manual"));
			strcpy (validStatus, "M");
			break;

		case	'H':
			strcpy (local_rec.statusDesc, ML ("Held"));
			strcpy (validStatus, "H");
			break;

		case	'*':
			strcpy (local_rec.statusDesc,"(All Avail)");
			strcpy (validStatus, "FBMH");
			break;
		default:
			strcpy (local_rec.status," ");
			strcpy (local_rec.statusDesc, ML ("ERROR"));
			break;
		}
		display_field (field);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ls_code")) 
	{
		if (SRCH_KEY)
		{
			SrchExls (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exls_rec.co_no,comm_rec.co_no);
		cc = find_rec (exls, &exls_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess (ML (mlSoMess297));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=====================
| Search for Order no |
======================*/
void
SrchSohr (
 char *key_val)
{
	_work_open (6,0,60);
	save_rec ("#Ord No","#Customer No - Name ");                       
	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	sprintf (sohr_rec.order_no,"%-8.8s",key_val);
	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
	while (!cc && !strncmp (sohr_rec.order_no,key_val,strlen (key_val)) &&
		  !strcmp (sohr_rec.co_no,comm_rec.co_no) && 
		  !strcmp (sohr_rec.br_no,comm_rec.est_no))
	{
		if (!strchr (validStatus, sohr_rec.status [0]))
		{
			cc = find_rec (sohr,&sohr_rec,NEXT,"r");
			continue;
		}

		cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf (cumr_rec.dbt_no,"%-40.40s","Unknown Customer");
			pocr_rec.ex1_factor = 1.00;
		}
		else
		{
			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code,  cumr_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
			if (cc)
				pocr_rec.ex1_factor = 1.00;
		}

		sprintf (err_str,"%-6.6s %-40.40s",cumr_rec.dbt_no, cumr_rec.dbt_name);
		cc = save_rec (sohr_rec.order_no,err_str);      
		if (cc)
			break;
		
		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	sprintf (sohr_rec.order_no,"%-8.8s",temp_str);
	cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, sohr, "DBFIND");
}

/*================
| Set defaults . |
================*/
void
SetLable (
 void)
{
	switch (deleteBy)
	{
	case	ITEM:
		FLD ("item_no") = YES;
		FLD ("desc") 	= NA;
		FLD ("ord_no") 	= NA;
		FLD ("dbt_no") 	= NA;
		FLD ("name") 	= NA;
		sprintf (local_rec.order,"%8.8s"," ");
		sprintf (local_rec.cust,"%6.6s"," ");
		sprintf (cumr_rec.dbt_name,"%40.40s"," ");
		DSP_FLD ("ord_no");
		DSP_FLD ("dbt_no");
		DSP_FLD ("name");
		break;

	case	ORDER:
		FLD ("item_no")	= NA;
		FLD ("desc") 	= NA;
		FLD ("ord_no") 	= YES;
		FLD ("dbt_no") 	= NA;
		FLD ("name") 	= NA;
		sprintf (inmr_rec.item_no,"%16.16s"," ");
		sprintf (inmr_rec.description,"%40.40s"," ");
		sprintf (local_rec.cust,"%6.6s"," ");
		sprintf (cumr_rec.dbt_name,"%40.40s"," ");
		DSP_FLD ("item_no");
		DSP_FLD ("desc");
		DSP_FLD ("dbt_no");
		DSP_FLD ("name");
		break;

	case	CUSTOMER:
		FLD ("item_no")	= NA;
		FLD ("desc") 	= NA;
		FLD ("ord_no") 	= NA;
		FLD ("dbt_no") 	= YES;
		FLD ("name")	= NA;
		sprintf (inmr_rec.item_no,"%16.16s"," ");
		sprintf (inmr_rec.description,"%40.40s"," ");
		sprintf (local_rec.order,"%8.8s"," ");
		DSP_FLD ("item_no");
		DSP_FLD ("desc");
		DSP_FLD ("ord_no");
		break;

	}
}

/*========================
| Process relevent Data. |
========================*/
void
ProcessData (
 void)
{
	switch (deleteBy)
	{
	case	ITEM:
		print_mess (ML (" Selective Order Line Delete By Item "));
		DeleteItem ();
		break;

	case	ORDER:
		print_mess (ML (" Selective Order Line Delete By Order No "));
		DeleteOrder ();
		break;

	case	CUSTOMER:
		print_mess (ML (" Selective Order Line Delete By Customer"));
		DeleteCustomer ();
		break;
	}
}

/*===================================
| Delete all orders for a customer. |
===================================*/
void
DeleteCustomer (
 void)
{
	abc_selfield (soln, "soln_id_no");
	abc_selfield (sohr, "sohr_hhcu_hash");
	abc_selfield (inmr, "inmr_hhbr_hash");

	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
	while (!cc && sohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		/*-------------------------------------------
		| Process all lines and delete as required. |
		-------------------------------------------*/
		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = 0;
		cc = find_rec (soln,&soln_rec,GTEQ,"u");
		while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
		{
			if (DeleteLine ())
			{
				abc_unlock (soln);
				cc = find_rec (soln,&soln_rec,NEXT,"u");
			}
			else
			{
				abc_unlock (soln);
				cc = find_rec (soln,&soln_rec,GTEQ,"u");
			}
		}
		abc_unlock (soln);

		/*--------------------------------------
		| If no lines left then delete header. |
		--------------------------------------*/
		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = 0;
		cc = find_rec (soln,&soln_rec,GTEQ,"r");
		if (cc || soln_rec.hhso_hash != sohr_rec.hhso_hash)
			DeleteHeader (sohr_rec.hhso_hash);

		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}
}

/*===============================
| Delete all Orders for an Item.|
===============================*/
void
DeleteItem (
 void)
{
	abc_selfield (soln, "soln_hhbr_hash");
	abc_selfield (sohr, "sohr_hhso_hash");
	abc_selfield (cumr, "cumr_hhcu_hash");

	soln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (soln,&soln_rec,GTEQ,"u");
	while (!cc && soln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sohr_rec.hhso_hash = soln_rec.hhso_hash;	
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (cumr_rec.dbt_no,"%-40.40s","Unknown Customer");
			pocr_rec.ex1_factor = 1.00;
		}
		else
		{
			cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;	
			cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
			if (!cc)
			{
				strcpy (pocr_rec.co_no, comm_rec.co_no);
				strcpy (pocr_rec.code,  cumr_rec.curr_code);
				cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
				if (cc)
					pocr_rec.ex1_factor = 1.00;
			}
		}
		if (DeleteLine ())
		{
			abc_unlock (soln);
			cc = find_rec (soln,&soln_rec,NEXT,"u");
		}
		else
			cc = find_rec (soln,&soln_rec,GTEQ,"u");
	}
	abc_unlock (soln);
}

/*========================
| Delete selected order. |
========================*/
void
DeleteOrder (
 void)
{
	abc_selfield (soln, "soln_id_no");
	abc_selfield (inmr,"inmr_hhbr_hash");
	abc_selfield (cumr,"cumr_hhcu_hash");

	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"u");
	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;	
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (!cc)
		{
			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code,  cumr_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
			if (cc)
				pocr_rec.ex1_factor = 1.00;
		}
		if (DeleteLine ())
		{
			abc_unlock (soln);
			cc = find_rec (soln,&soln_rec,NEXT,"u");
		}
		else
			cc = find_rec (soln,&soln_rec,GTEQ,"u");
	}
	abc_unlock (soln);

	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"r");
	if (cc || soln_rec.hhso_hash != sohr_rec.hhso_hash)
	{
		DeleteHeader (sohr_rec.hhso_hash);
		return;
	}
}

/*=============================
| Delete relevent valid line. |
=============================*/
int
DeleteLine (
 void)
{
	long	hhsoHash = 0L;
	int		i;

	if (!strchr (validStatus, soln_rec.status [0]))
		return (EXIT_FAILURE);

	/*-----------------------------
	| Check for works order line. |
	-----------------------------*/
	if (envSoWoAllowed)
	{
		pcwo_rec.hhsl_hash	=	soln_rec.hhsl_hash;
		cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
		if (!cc)
		{
			if (pcwo_rec.order_status [0] != 'D' && 
			    pcwo_rec.order_status [0] != 'Z')
				return (EXIT_FAILURE);
		}
	}
	hhsoHash = soln_rec.hhso_hash;

	/*--------------------
	| Add a sobg_record  |
	--------------------*/
	for (i = 0; i < MAXDELETE; i++)
	{
		if (addItem [i] == soln_rec.hhbr_hash)
			break;

		if (!addItem [i])
		{
			addItem [i] = soln_rec.hhbr_hash;
			break;
		}
	}

	if (!auditOpen)
	{
		OpenAudit ();
		auditOpen = TRUE;
	}
	PrintDetail ();

	if (SERIAL_ITEM)
		UpdateInsf (soln_rec.hhbr_hash, soln_rec.serial_no);

	if (local_rec.rep_type [3] [0] == 'Y')
	{
		LogLostSales
		(
			soln_rec.hhbr_hash,
			soln_rec.hhcc_hash,
			cumr_rec.hhcu_hash,
			cumr_rec.area_code,
			cumr_rec.sman_code,
			soln_rec.qty_order + soln_rec.qty_bord
		);
	}
	DeleteAllocation (soln_rec.hhsl_hash);

	cc = abc_delete (soln);
	if (cc)
		file_err (cc, soln, "DBDELETE");

	/*-------------------------------------
	| Check if other lines exists,if not  |
	| delete it's header as well.         |
	-------------------------------------*/
	soln2_rec.hhso_hash = hhsoHash;
	soln2_rec.line_no = 0;
	cc = find_rec (soln2,&soln2_rec,GTEQ,"r");
	if (cc || soln2_rec.hhso_hash != hhsoHash)
		DeleteHeader (hhsoHash);
	
	return (EXIT_SUCCESS);
}

void
DeleteHeader (
	long	hhsoHash)
{
	int	i;

	sohr2_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr2,&sohr2_rec,EQUAL,"u");
	if (cc)
	{
		abc_unlock (sohr2);
		return;
	}

	cc = abc_delete (sohr2);
	if (cc)
	{
		abc_unlock (sohr2);
		return;
	}

	/*--------------------
	| Add a sobg_record  |
	--------------------*/
	for (i = 0; i < MAXDELETE; i++)
	{
		if (addCust [i] == sohr2_rec.hhcu_hash)
			break;
	
		if (!addCust [i])
		{
			addCust [i] = sohr2_rec.hhcu_hash;
			break;
		}
	}
	return;
}

void
DeleteAllocation (
 long	hhsl_hash)
{
	inla_rec.inlo_hash	=	0L;
	inla_rec.hhsl_hash	=	hhsl_hash;
	cc = find_rec (inla, &inla_rec, GTEQ, "u");
	while (!cc && inla_rec.hhsl_hash ==	hhsl_hash)
	{
		cc = abc_delete (inla);
		if (cc)
			file_err (cc, inla, "DBDELETE");

		inla_rec.inlo_hash	=	0L;
		inla_rec.hhsl_hash	=	hhsl_hash;
		cc = find_rec (inla, &inla_rec, GTEQ, "u");
	}
	abc_unlock (inla);
}

void
PrintDetail (
 void)
{
	float	backorder = 0.0;
	double	l_total	=	0.00,
			l_disc	=	0.00;
	double	value = 0.0;

	backorder	=	soln_rec.qty_order + soln_rec.qty_bord;
	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	(double) backorder;
		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);
	}
	value	=	l_total - l_disc;
	
	switch (deleteBy)
	{
	case	ITEM:
		sohr_rec.hhso_hash = soln_rec.hhso_hash;
		cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
		if (cc)
			sprintf (sohr_rec.order_no,"%-8.8s","??????");

		cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (firstTime)
		{
			fprintf (pp,"|-----------------|");
			fprintf (pp,"-------------------------------------------|");
			fprintf (pp,"--------|");
			fprintf (pp,"--------|");
			fprintf (pp,"-----------|");
			fprintf (pp,"--------------|");
			fprintf (pp,"---------------|\n");

			fprintf (pp,"| %-16.16s|",inmr_rec.item_no);
			fprintf (pp," %40.40s  |",inmr_rec.description);
		}
		else
		{
			fprintf (pp,"| %-16.16s|"," ");
			fprintf (pp," %40.40s  |"," ");
		}

		fprintf (pp,"%8.8s|",sohr_rec.order_no);
		fprintf (pp," %6.6s |",cumr_rec.dbt_no);
		fprintf (pp," %9.9s |",cumr_rec.dbt_acronym);
		fprintf (pp," %12.2f |",backorder);
		fprintf (pp," %13.2f |\n",DOLLARS (value));
		break;

	case	ORDER:
	case	CUSTOMER:
		inmr_rec.hhbr_hash = soln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf (inmr_rec.item_no,"%-16.16s"," Unknown Item.  ");
			sprintf (inmr_rec.description,"%-40.40s"," ");
		}

		if (firstTime)
		{
			fprintf (pp,"|--------|");
			if (deleteBy == ORDER)
			{
				fprintf (pp,"--------|");
				fprintf (pp,"-----------|");
			}
			else
			{
				fprintf (pp,"-----------|");
				fprintf (pp,"--------|");
			}

			fprintf (pp,"--------------|");
			fprintf (pp,"---------------|");
			fprintf (pp,"-----------------|");
			fprintf (pp,"-------------------------------------------|\n");
			if (deleteBy == ORDER)
			{
				fprintf (pp,"|%8.8s|",sohr_rec.order_no);
				fprintf (pp," %6.6s |",cumr_rec.dbt_no);
				fprintf (pp," %9.9s |",cumr_rec.dbt_acronym);
			}
			else
			{
				fprintf (pp,"| %6.6s |",cumr_rec.dbt_no);
				fprintf (pp," %9.9s |",cumr_rec.dbt_acronym);
				fprintf (pp,"%8.8s|",sohr_rec.order_no);
			}
		}
		else
		{
			fprintf (pp,"| %6.6s |"," ");
			if (deleteBy == ORDER)
			{
				fprintf (pp," %6.6s |"," ");
				fprintf (pp," %9.9s |"," ");
			}
			else
			{
				fprintf (pp," %9.9s |"," ");
				fprintf (pp,"%8.8s|",sohr_rec.order_no);
			}
		}
		fprintf (pp," %12.2f |",backorder);
		fprintf (pp," %13.2f |",DOLLARS (value));
		fprintf (pp," %-16.16s|",inmr_rec.item_no);
		fprintf (pp," %40.40s  |\n",inmr_rec.description);
		break;

	}

	firstTime = 0;
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((pp = popen ("pformat","w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pp,".SO\n");
	fprintf (pp,".LP%d\n",printerNumber);
	fprintf (pp,".PI12\n");
	fprintf (pp,".9\n");
	fprintf (pp,".L158\n");
	switch (deleteBy)
	{
	case	ITEM:
		fprintf (pp,".ESELECTIVE ORDER LINE DELETE BY ITEM\n");
		break;

	case	ORDER:
		fprintf (pp,".ESELECTIVE ORDER LINE DELETE BY ORDER NUMBER\n");
		break;

	case	CUSTOMER:
		fprintf (pp,".ESELECTIVE ORDER LINE DELETE BY CUSTOMER NO\n");
		break;
	}

	fprintf (pp,".EAS AT %24.24s\n",SystemTime ());

	fprintf (pp,".ECOMPANY : %s - %s\n",clip (comm_rec.co_no),clip (comm_rec.co_name));
	fprintf (pp,".B1\n");

	fprintf (pp,".R===================");
	fprintf (pp,"============================================");
	fprintf (pp,"=========");
	fprintf (pp,"=========");
	fprintf (pp,"============");
	fprintf (pp,"===============");
	fprintf (pp,"================\n");

	fprintf (pp,"===================");
	fprintf (pp,"============================================");
	fprintf (pp,"=========");
	fprintf (pp,"=========");
	fprintf (pp,"============");
	fprintf (pp,"===============");
	fprintf (pp,"================\n");

	switch (deleteBy)
	{
	case	ITEM:
		fprintf (pp,"|   ITEM NUMBER   |");
		fprintf (pp,"             ITEM DESCRIPTION              |");
		fprintf (pp," ORDER  |");
		fprintf (pp,"CUSTOMER|");
		fprintf (pp,"  ACRONYM  |");
		fprintf (pp,"  QUANTITY    |");
		fprintf (pp,"     VALUE     |\n");

		fprintf (pp,"|                 |");
		fprintf (pp,"                                           |");
		fprintf (pp," NUMBER |");
		fprintf (pp," NUMBER |");
		fprintf (pp,"           |");
		fprintf (pp,"  OF  ORDER   |");
		fprintf (pp,"  OF  ORDER    |\n");
		break;

	case	ORDER:
		fprintf (pp,"| ORDER  |");
		fprintf (pp,"CUSTOMER|");
		fprintf (pp,"  ACRONYM  |");
		fprintf (pp,"  QUANTITY    |");
		fprintf (pp,"     VALUE     |");
		fprintf (pp,"   ITEM NUMBER   |");
		fprintf (pp,"             ITEM DESCRIPTION              |\n");

		fprintf (pp,"| NUMBER |");
		fprintf (pp," NUMBER |");
		fprintf (pp,"           |");
		fprintf (pp,"   OF ORDER   |");
		fprintf (pp,"   OF ORDER    |");
		fprintf (pp,"                 |");
		fprintf (pp,"                                           |\n");
		break;

	case	CUSTOMER:
		fprintf (pp,"|CUSTOMER|");
		fprintf (pp,"  ACRONYM  |");
		fprintf (pp," ORDER  |");
		fprintf (pp,"  QUANTITY    |");
		fprintf (pp,"     VALUE     |");
		fprintf (pp,"   ITEM NUMBER   |");
		fprintf (pp,"             ITEM DESCRIPTION              |\n");

		fprintf (pp,"| NUMBER |");
		fprintf (pp,"           |");
		fprintf (pp," NUMBER |");
		fprintf (pp,"   OF ORDER   |");
		fprintf (pp,"   OF ORDER    |");
		fprintf (pp,"                 |");
		fprintf (pp,"                                           |\n");
		break;

	}
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (pp,".EOF\n");
	pclose (pp);
}

void
UpdateInsf (
 long hhbr_hash, 
 char *serial_no)
{
	insf_rec.hhbr_hash = hhbr_hash;
	sprintf (insf_rec.serial_no, "%-25.25s",serial_no);
	strcpy (insf_rec.status,"C");
	if (!find_rec (insf,&insf_rec,COMPARISON,"u"))
	{
		strcpy (insf_rec.status,"F");
		cc = abc_update (insf,&insf_rec);
		if (cc)
			file_err (cc, insf, "DBUPDATE");

		return;
	}
	insf_rec.hhbr_hash = hhbr_hash;
	sprintf (insf_rec.serial_no, "%-25.25s",serial_no);
	strcpy (insf_rec.status,"S");
	if (!find_rec (insf,&insf_rec,COMPARISON,"u"))
	{
		strcpy (insf_rec.status,"F");
		cc = abc_update (insf,&insf_rec);
		if (cc)
			file_err (cc, insf, "DBUPDATE");
		return;
	}
	insf_rec.hhbr_hash = hhbr_hash;
	sprintf (insf_rec.serial_no, "%-25.25s",serial_no);
	strcpy (insf_rec.status,"T");
	if (!find_rec (insf,&insf_rec,COMPARISON,"u"))
	{
		strcpy (insf_rec.status,"F");
		cc = abc_update (insf,&insf_rec);
		if (cc)
			file_err (cc, insf, "DBUPDATE");

		abc_unlock (insf);
		return;
	}
}

/*=================================================================
| Add record to background records for Order values and on order. |
=================================================================*/
void
UpdateOther (
 void)
{
	int	i;

	clear ();
	
	print_mess (ML ("Updating stock committed/backorder quantities."));

	/*----------------------------
	| Recalc all records found . |
	----------------------------*/
	for (i = 0; i < MAXDELETE; i++)
	{
		if (addItem [i] == 0L)
			break;

		AddSobg (0,"RC",addItem [i]);
	}

	print_mess (ML ("Updating customer order values."));
	/*----------------------------
	| Recalc all records found . |
	----------------------------*/
	for (i = 0; i < MAXDELETE; i++)
	{
		if (addCust [i] == 0L)
			break;

		AddSobg (0,"RO",addCust [i]);
	}
}

int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlSoMess151),25,0,1);
	box (0,3,80,14);
	line_at (7,1,79);
	line_at (13,1,79);
	line_at (15,1,79);
	line_at (1,0,80);
	line_at (20,0,80);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	print_at (22,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
	scn_write (scn);

    return (EXIT_SUCCESS);
}

/*===========================================
| Add record to background processing file. |
===========================================*/
void
AddSobg (
 int _printerNumber, 
 char *_type_flag, 
 long _hash)
{
	strcpy (sobg_rec.co_no, comm_rec.co_no);
	strcpy (sobg_rec.br_no, comm_rec.est_no);
	strcpy (sobg_rec.type, _type_flag);
	sobg_rec.lpno = _printerNumber;
	sobg_rec.hash = _hash;

	cc = find_rec (sobg,&sobg_rec,COMPARISON,"r");
	/*---------------------------------------------------------------
	| Add the record iff an identical one doesn't already exist	|
	---------------------------------------------------------------*/
	if (cc)
	{
		strcpy (sobg_rec.co_no, comm_rec.co_no);
		strcpy (sobg_rec.br_no, comm_rec.est_no);
		strcpy (sobg_rec.type, _type_flag);
		sobg_rec.lpno = _printerNumber;
		sobg_rec.hash = _hash;

		cc = abc_add (sobg,&sobg_rec);
		if (cc)
			file_err (cc, sobg, "DBADD");
	}
}

/*======================================================================
| Log lost sales from stock quantity on hand less-than input quantity. |
======================================================================*/
void
LogLostSales (
 long	hhbr_hash,
 long	hhcc_hash,
 long	hhcu_hash,
 char	*AreaCode,
 char	*SmanCode,
 float	Qty)
{
	double	Value;

	/*---------------------------------------------------
	| If MCURR convert sales value into local currency. |
	---------------------------------------------------*/
	if (FGN_CURR && pocr_rec.ex1_factor != 0.00)
		Value = no_dec (soln_rec.sale_price / pocr_rec.ex1_factor);
	else
		Value = no_dec (soln_rec.sale_price);

	strcpy (inls_rec.co_no , sohr_rec.co_no);
	strcpy (inls_rec.est_no, sohr_rec.br_no);
	inls_rec.date		=	comm_rec.inv_date;
	inls_rec.hhbr_hash	=	hhbr_hash;
	inls_rec.hhcc_hash	=	hhcc_hash;
	inls_rec.hhcu_hash	=	hhcu_hash;
	strcpy (inls_rec.area_code, AreaCode);
	strcpy (inls_rec.sale_code, SmanCode);
	inls_rec.qty		=	Qty;
	inls_rec.value		=	Value;
	inls_rec.cost		=	0.00;
	strcpy (inls_rec.res_code, exls_rec.code);
	strcpy (inls_rec.res_desc, exls_rec.description);
	strcpy (inls_rec.status, "F");

	cc = abc_add (inls, &inls_rec);
	if (cc)
		file_err (cc, inls, "DBADD");

}

void
SrchExls (
 char *key_val)
{
	work_open ();
	save_rec ("#Ls","#Lost Sale Description");

	strcpy (exls_rec.co_no, comm_rec.co_no);
	sprintf (exls_rec.code, "%-2.2s", key_val);
	cc = find_rec (exls, &exls_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (exls_rec.code, key_val, strlen (key_val)) && 
		   !strcmp (exls_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exls_rec.code, exls_rec.description);
		if (cc)
			break;

		cc = find_rec (exls, &exls_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exls_rec.co_no, comm_rec.co_no);
	sprintf (exls_rec.code, "%-2.2s", temp_str);
	cc = find_rec (exls, &exls_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exls, "DBFIND");
}

int
CheckWorksOrder (
	long	hhsoHash)
{
	soln_rec.hhso_hash	=	hhsoHash;
	soln_rec.line_no	=	0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		pcwo_rec.hhsl_hash	=	soln_rec.hhsl_hash;
		cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
		if (!cc)
		{
			if (pcwo_rec.order_status [0] != 'D' &&
				pcwo_rec.order_status [0] != 'Z')
					return (EXIT_FAILURE);
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}
