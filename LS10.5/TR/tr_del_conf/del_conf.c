/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: del_conf.c,v 5.3 2001/10/25 07:54:32 scott Exp $
|	Program Name : (tr_del_conf.c)								  	  |
|	Program Desc : (Transport Delivery Confirmation.)      			  |
|---------------------------------------------------------------------|
|  Author        : Liza A. Santos  | Date Written  : 22/05/96         |
|---------------------------------------------------------------------|
| $Log: del_conf.c,v $
| Revision 5.3  2001/10/25 07:54:32  scott
| Updated to make changes related to container and seals.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: del_conf.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_del_conf/del_conf.c,v 5.3 2001/10/25 07:54:32 scott Exp $";

#define SCN_INIT	  1
#define SCN_TABLE	  2
#define SCN_HEADER	  3
#define MAXSCNS		  2
#define MAXLINES	500
#define TABLINES	 14
#include <pslscr.h>
#include <hot_keys.h>
#include <time.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <ml_tr_mess.h>
#include <ml_std_mess.h>
#include <tabdisp.h>

typedef int BOOL;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		screenWidth = 132;

	FILE	*fin,
			*fsort;

#include	"schema"

struct commRecord	comm_rec;
struct trveRecord	trve_rec;
struct trveRecord	trve2_rec;
struct trlnRecord	trln_rec;
struct trhrRecord	trhr_rec;
struct trhrRecord	trhr3_rec;
struct cohrRecord	cohr_rec;
struct cumrRecord	cumr_rec;
struct colnRecord	coln_rec;
struct skniRecord	skni_rec;

	char	*trhr3	=	"trhr3",
			*trve2	=	"trve2",
			*data	=	"data";

/*===========================
| Local & Screen Structures. |
============================*/
struct
{
	long	del_date;
	long	del_date2;
	char	trip_no  [13];
	char	trip_no2 [13];
	char	ref   [11];
	char	ref2  [11];
	char	desc  [41];
	char	desc2 [41];
	char	dummy [11];
} local_rec; 

static	struct	var	vars[] =
{
	{1, LIN, "vehicle",	 3, 25, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "  ", "  Vehicle Reference   :", "Enter Vehicle No. or [SEARCH]",
		YES, NO,  JUSTLEFT, "", "", local_rec.ref},
	{1, LIN, "vehi_desc",	 3, 59, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},

	{1, LIN, "del_date", 	4, 25, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ",   "  Expected Del Date   :", " Enter Expected Date or [Search] . ", 
		 YES, NO,  JUSTLEFT, " ", "", (char *)&local_rec.del_date}, 

	{1, LIN, "trip_no",	 5, 25, CHARTYPE,
		"UUUUUUUUUUUU", "          ",
		" ", "  ", "  Trip Number         :", "Enter Trip No or [SEARCH].",
		YES, NO,  JUSTLEFT, "", "", local_rec.trip_no},

	{SCN_HEADER, LIN, "h_vehicle",	 2, 25, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "  ", "  Vehicle Reference   :", "Enter Vehicle No. or [SEARCH]",
		NA, NO,  JUSTLEFT, "", "", local_rec.ref2},

	{SCN_HEADER, LIN, "h_vehi_desc",	 2, 59, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc2},

	{SCN_HEADER, LIN, "h_del_date", 	3, 25, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ",   "  Expected Del Date   :", " Enter Expected Date or [Search] . ", 
		 NA, NO,  JUSTLEFT, " ", "", (char *)&local_rec.del_date2}, 

	{SCN_HEADER, LIN, "h_trip_no",	 4, 25, CHARTYPE,
		"UUUUUUUUUUUU", "          ",
		" ", "  ", "  Trip Number         :", "Enter Trip No.",
		NA, NO,  JUSTLEFT, "", "", local_rec.trip_no2},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

char	*tableFormat 	= "  %-1.1s      %-8.8s %-6.6s  %-100.100s";
char	*tableName		= "delConfirm";

char	tableBuffer [135];
int		currentScreen = SCN_INIT;

static struct {
	long	hhtrHash;	
	long	hhveHash;	
	long	hhcoHash;	
	long	hhcuHash;	
	char	statusFlag [2];
	char	invoiceNo [9];
	char	customerNo [41];
	char	deliveryAddress[41];
	} dataStructure [MAXLINES];

static	int		noLines = 0;	/* number of lines added to table */	
static  int 	ConfirmFunc 	(int, KEY_TAB *);
static  int 	RestartFunc 	(int, KEY_TAB *);
static  int 	ExitFunc 		(int, KEY_TAB *);

#define DETAIL_KEY		'C'

#ifdef	GVISION
static  KEY_TAB list_keys [] =
{
   { " Confirm ",		DETAIL_KEY,			ConfirmFunc,
    "Confirm Deliver ",						"A" },
   { NULL,				FN1,				RestartFunc,
    "Exit without update.",					"A" },
   { NULL,				FN16,				ExitFunc,
    "Exit and update the database.",		"A" },
   END_KEYS
};
#else
static  KEY_TAB list_keys [] =
{
   { "[C]onfirm ",		DETAIL_KEY,			ConfirmFunc,
    "Confirm Deliver ",						"A" },
   { NULL,				FN1,				RestartFunc,
    "Exit without update.",					"A" },
   { NULL,				FN16,				ExitFunc,
    "Exit and update the database.",		"A" },
   END_KEYS
};
#endif

/*=======================
| Function declarations |
=======================*/
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	TrveSrch 				(char *);
void 	ShowExpectedDelivery 	(char *);
void 	TrhrSrch 				(char *);
void 	InitFiles 				(void);
void 	UpdateTrhr 				(void);
void 	InitTrlds 				(void);
void	ProcessTrlos 			(void);
int 	heading 				(int);
int 	spec_valid 				(int);

/*==========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	SETUP_SCR 	 (vars);
	init_scr 	 ();
	set_tty 	 ();
	set_masks 	 ();
	
	OpenDB		 ();
	clear		 ();

	while (!prog_exit)
	{
		InitFiles ();
		search_ok 	= 	TRUE;
		entry_exit 	= 	FALSE;
		edit_exit 	= 	FALSE;
		prog_exit 	= 	FALSE;
		restart 	= 	FALSE;
		init_ok 	= 	TRUE;
		init_vars (1);	
		heading	(SCN_INIT);
		entry	(SCN_INIT);   
		if (!restart && !prog_exit)
		{
			heading	(SCN_INIT);
			scn_display (SCN_INIT);
			ProcessTrlos ();
		}

		while (!restart && !prog_exit)
		{
			strcpy (local_rec.ref2, local_rec.ref);
			local_rec.del_date2 = local_rec.del_date;
			strcpy (local_rec.trip_no2, local_rec.trip_no);
			strcpy (local_rec.desc2, local_rec.desc);

			heading 	 (SCN_HEADER);
			ProcessTrlos ();
			prog_exit = FALSE;

			if (currentScreen == SCN_INIT)
				restart = TRUE;
		}
		if (noLines)
			UpdateTrhr ();  
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

/*======================= 
| Open data base files. |
======================= */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	abc_alias (trve2, trve);
	abc_alias (trhr3, trhr);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (trve,  trve_list, TRVE_NO_FIELDS, "trve_id_no"); 
	open_rec (trve2, trve_list, TRVE_NO_FIELDS, "trve_hhve_hash"); 
	open_rec (trhr,  trhr_list, TRHR_NO_FIELDS, "trhr_id_no"); 
	open_rec (trhr3, trhr_list, TRHR_NO_FIELDS, "trhr_trip_name"); 
	open_rec (trln,  trln_list, TRLN_NO_FIELDS, "trln_hhtr_hash"); 
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash"); 
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash"); 
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_hhcl_hash"); 
	open_rec (skni,  skni_list, SKNI_NO_FIELDS, "skni_hhcl_hash"); 

}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (trhr);
	abc_fclose (trhr3);
	abc_fclose (cohr);
	abc_fclose (trln);
	abc_fclose (trve);
	abc_fclose (trve2);
	abc_fclose (cumr);
	abc_fclose (skni);
	abc_fclose (coln);
	abc_dbclose (data);
}

/*------------------+
|  Field Validation |
------------------*/
int
spec_valid (
 int field)
{

	if (LCHECK ("vehicle"))
	{
		FLD ("del_date")	=	YES;
		if (dflt_used) 
		{
			strcpy (trve_rec.ref, "          ");
			FLD ("del_date")	=	NI;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			TrveSrch (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (trve_rec.co_no, comm_rec.co_no);
		strcpy (trve_rec.br_no, comm_rec.est_no);
		strcpy (trve_rec.ref, local_rec.ref);
		cc = find_rec (trve, &trve_rec, EQUAL, "r");
		if (cc)
		{
			/*--------------------
			| Vehicle not found. |
			--------------------*/
			errmess (ML (mlStdMess218));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.desc ,trve_rec.desc);
		DSP_FLD ("vehi_desc");
		return (EXIT_SUCCESS);
	}

 	/*------------------------------- 
  	|	   Validate Date Entered	| 
  	-------------------------------*/
	if (LCHECK ("del_date"))
	{
		if (dflt_used)
		 	local_rec.del_date	=	TodaysDate (); 

		if (SRCH_KEY)
		{
			ShowExpectedDelivery (temp_str);
			if (strcmp (local_rec.trip_no,"            "))
			{
				skip_entry = 1;
				return (EXIT_SUCCESS);
			}
			else
				return (EXIT_FAILURE);
		}

		DSP_FLD ("del_date");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Trip Number. |
	-----------------------*/
	if (LCHECK ("trip_no"))
	{
		if (SRCH_KEY)
		{
			TrhrSrch (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.trip_no, "            "))
		{
			/* -----------------------
			| Trip Number not found. |
			 -----------------------*/
			errmess (ML (mlTrMess003));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (trhr3_rec.trip_name,local_rec.trip_no);
		cc = find_rec (trhr3, &trhr3_rec, EQUAL ,"r");
		if (!cc)
		{
			if (!strcmp (local_rec.ref, "          ") 
				&& local_rec.del_date == 0L)
			{
				local_rec.del_date = trhr3_rec.del_date;

				trve2_rec.hhve_hash = trhr3_rec.hhve_hash;
				cc	=	find_rec (trve2,&trve2_rec, EQUAL,"r");
				if (!cc)
				{
					strcpy (local_rec.ref, trve2_rec.ref);
					strcpy (local_rec.desc,trve2_rec.desc);
				}
				DSP_FLD ("vehicle");
				DSP_FLD ("vehi_desc");
				DSP_FLD ("del_date");
			}
		}
		DSP_FLD ("trip_no");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==================
| Search for trve. |
==================*/
void
TrveSrch (
 char	*key_val)
{
	work_open ();
	save_rec ("#Vehicle","#Description");
	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	sprintf (trve_rec.ref, "%-10.10s", key_val);
	cc = find_rec (trve, &trve_rec, GTEQ, "r");
	while (!cc && !strcmp (trve_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trve_rec.br_no, comm_rec.est_no) &&
			      !strncmp (trve_rec.ref,key_val,strlen (key_val)))
	{
		cc = save_rec (trve_rec.ref, trve_rec.desc);
		if (cc)
			break;
		cc = find_rec (trve, &trve_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	sprintf (trve_rec.ref,"%-10.10s", temp_str);
	cc = find_rec (trve, &trve_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trve, "DBFIND");
}

/*====================+
| Search for Date     |
---------------------*/
void
ShowExpectedDelivery (
 char	*key_val)
{
 	char 	date_trp[25];
	int		trhr_found;

	work_open ();
	save_rec ("#Tr Date  Trip Number", "#Vehicle Number");
	trhr_rec.hhve_hash = trve_rec.hhve_hash;
	trhr_rec.del_date  = StringToDate (key_val);
	cc = find_rec (trhr, &trhr_rec, GTEQ, "r");
	while (!cc 
			&&  !strncmp (DateToString (trhr_rec.del_date),key_val,strlen (key_val))
			&&  trhr_rec.hhve_hash == trve_rec.hhve_hash)
	{
		if (strcmp (trhr_rec.status, "D"))
		{
			sprintf (date_trp,"%-10.10s %-12.12s",
						DateToString (trhr_rec.del_date), trhr_rec.trip_name);
			cc = save_rec (date_trp, trve_rec.ref);
			if (cc)
				break;
		}
		cc = find_rec (trhr,&trhr_rec,NEXT,"r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	trhr_found = FALSE;

	trhr_rec.hhve_hash = trve_rec.hhve_hash;
	trhr_rec.del_date  = StringToDate (temp_str);
	cc = find_rec (trhr, &trhr_rec, GTEQ, "r");
	while (!cc)
	{
		if (!strcmp (trhr_rec.trip_name, temp_str + 11))
		{
			trhr_found = TRUE;
			strcpy (local_rec.trip_no, trhr_rec.trip_name);
			break;
		}
		cc = find_rec (trhr,&trhr_rec,NEXT,"r");
	}

	if (!trhr_found)
		file_err (cc, "trhr", "DBFIND");
}

/*================================================
| Search for Trip Number Transport Header File . |
================================================*/
void
TrhrSrch (
 char *key_val)
{
	work_open ();
	save_rec ("#Trip Number", "#Del Date");
	sprintf (trhr3_rec.trip_name, "%-12.12s", key_val);
	cc = find_rec (trhr3, &trhr3_rec, GTEQ, "r");
	while (!cc 
			&&  !strncmp (trhr3_rec.trip_name,key_val,strlen (key_val)))
	{
		if (trhr3_rec.status[0] != 'D' &&
			 trhr3_rec.del_date <= local_rec.del_date)
		{
			strcpy (err_str, DateToString (trhr3_rec.del_date));
			cc = save_rec (trhr3_rec.trip_name, err_str);
			if (cc)
				break;
		}
		cc = find_rec (trhr3,&trhr3_rec,NEXT,"r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (trhr3_rec.trip_name, "%-12.12s", temp_str);
	cc = find_rec (trhr3, &trhr3_rec, EQUAL, "r");
	if (cc)
		file_err (cc, trhr3, "DBFIND");
}

void
InitFiles (void)
{
	memset (&trhr_rec,0,sizeof (trhr_rec));
	memset (&trln_rec,0,sizeof (trln_rec));
	memset (&trve_rec,0,sizeof (trve_rec));
	memset (&cohr_rec,0,sizeof (cohr_rec));
	memset (&cumr_rec,0,sizeof (cumr_rec));
	memset (&local_rec,0,sizeof (local_rec));

}

void
UpdateTrhr (void)
{
	int	i,
		allconf = TRUE;
		
	for (i = 0; i < noLines; i++)
	{
		if (dataStructure [i].statusFlag [0] == 'D')
		{
			coln_rec.hhco_hash 	= dataStructure [i].hhcoHash;
			coln_rec.line_no 	= 0;
			cc = find_rec (coln, &coln_rec, GTEQ, "r");
			while (!cc && coln_rec.hhco_hash == dataStructure [i].hhcoHash)
			{
				skni_rec.hhcl_hash	=	coln_rec.hhcl_hash;
				cc = find_rec (skni, &skni_rec, COMPARISON, "u");
				if (!cc && skni_rec.sknd_hash == 0L)
					abc_delete (skni);
				else
					abc_unlock (skni);

				cc = find_rec (coln, &coln_rec, NEXT, "r");
			}
		}
		else
			allconf = FALSE;
	}
	if (allconf)
	{
		abc_selfield (trhr, "trhr_hhtr_hash");
		trhr_rec.hhtr_hash = dataStructure [0].hhtrHash;
		cc = find_rec (trhr, &trhr_rec, EQUAL, "u");
		if (!cc)
		{
			strcpy (trhr_rec.status, "D");
			cc = abc_update (trhr, &trhr_rec);
			if (cc)
				file_err (cc, trhr, "DBFIND");
		}
		abc_selfield (trhr, "trhr_id_no");
	}
}
/*=================
| Init trld table |
=================*/
void
InitTrlds (void)
{
	char	formatAddress [150];

	noLines = 0;

	abc_selfield (cohr, "cohr_hhco_hash");
	abc_selfield (trhr, "trhr_trip_name");
    strcpy (trhr_rec.trip_name, local_rec.trip_no);
	cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
	if (!cc)
	{
		trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
		cc = find_rec (trln, &trln_rec, GTEQ, "r");
		while (!cc && trln_rec.hhtr_hash == trhr_rec.hhtr_hash)
		{
			cohr_rec.hhco_hash = trln_rec.hhco_hash;
			cc = find_rec (cohr, &cohr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (trln, &trln_rec, NEXT, "r");
				continue;
			}
			
			cumr_rec.hhcu_hash = cohr_rec.hhcu_hash; 
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (trln, &trln_rec, NEXT, "r");
				continue;
			}
			
			strcpy (formatAddress, clip (cumr_rec.dl_adr1));
			strcat (formatAddress," ");
			strcat (formatAddress, clip (cumr_rec.dl_adr2));
			strcat (formatAddress," ");
			strcat (formatAddress, clip (cumr_rec.dl_adr3));
			strcat (formatAddress," ");
			strcat (formatAddress, cumr_rec.dl_adr4);

			memset 
			(
				(char *) &dataStructure [noLines], 
				0, 
				sizeof (dataStructure [noLines])
			);

			dataStructure [noLines].hhtrHash = trhr_rec.hhtr_hash;
			dataStructure [noLines].hhveHash = trhr_rec.hhve_hash;
			dataStructure [noLines].hhcoHash = cohr_rec.hhco_hash;
			dataStructure [noLines].hhcuHash = cohr_rec.hhcu_hash;
			strcpy (dataStructure [noLines].statusFlag, cohr_rec.status); 
			strcpy (dataStructure [noLines].invoiceNo, cohr_rec.inv_no);
			strcpy (dataStructure [noLines].customerNo, cumr_rec.dbt_no);
			strcpy (dataStructure [noLines].deliveryAddress, formatAddress);

			noLines++;

			tab_add 
			(
				tableName, 
				tableFormat,
				(cohr_rec.status [0] == 'D') ? "D" : " ",
				cohr_rec.inv_no, 
				cumr_rec.dbt_no, 
				formatAddress, 
				"A"
			);
			cc = find_rec (trln, &trln_rec, NEXT, "r");
		}
	}
	abc_selfield (cohr, "cohr_hhtr_hash");
	abc_selfield (trhr, "trhr_id_no");
	return;
}

/*========================
| Display trlds in Table |
========================*/
void
ProcessTrlos (void)
{
	tab_open (tableName, list_keys, 7, 1, 10, FALSE);
	tab_add (tableName, "# %-6.6s %-8.8s %-8.8s  %-100.100s",
			"Status",
			"Document",
			"Customer",
			"            Delivery Address");
		
	InitTrlds ();

	if (!noLines)
	{
		/*--------------
		| No  record ! |
		--------------*/
		errmess (ML (mlStdMess009));
		sleep (sleepTime);
		tab_close (tableName, TRUE);
		currentScreen	=	SCN_INIT;
		restart			=	TRUE;
		return;
	}
	else
		tab_scan (tableName);
	
	tab_close (tableName, TRUE);

	return;
}
static int
ConfirmFunc (
 int 	key, KEY_TAB *psUnused)
{
	int		activeLine; 
			
	activeLine = tab_tline (tableName);
	 
	if (!strcmp (dataStructure [activeLine].statusFlag, "D"))
	{
		/*-------------------------------
		| Status is already delivered. |
		-------------------------------*/
		print_err (ML (mlTrMess022));
		sleep (sleepTime);
		clear_mess ();
		abc_selfield (trhr, "trhr_id_no");
		return (EXIT_FAILURE);     
	}
	sprintf (err_str, "tr_delivery %010ld %010ld %010ld %010ld",
							dataStructure [activeLine].hhveHash,
							dataStructure [activeLine].hhtrHash,
							dataStructure [activeLine].hhcoHash,
							dataStructure [activeLine].hhcuHash);

	sys_exec (err_str);
	currentScreen = SCN_TABLE;   
	restart = FALSE;
	return (FN16); 
}
static int
RestartFunc (
 int 	key, KEY_TAB *psUnused)
{
	currentScreen = SCN_INIT;
	restart  = TRUE;
	return key;
}

static int
ExitFunc (
 int 	key, KEY_TAB *psUnused)
{
	currentScreen = SCN_INIT;
	restart  = TRUE;
	return key;
}

int
heading (
 int scn)
{
		swide ();
		clear ();

		/*--------------------------------------------------
		|%R Transport & Sales Liq - Input Delivery/Returns |
		--------------------------------------------------*/
		centre_at (0, screenWidth, ML (mlTrMess038));
		line_at (1,0, screenWidth + 1);
		line_at (21,0, screenWidth + 1);

		sprintf (err_str, ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		print_at (22,0, "%s", err_str);
		line_at (23,0, screenWidth + 1);
		
		switch (scn)
		{
		case SCN_INIT:
			box (0,2,screenWidth,3);
			sprintf (err_str, "%-2.2s%-2.2s%-2.2s", comm_rec.co_no,
							comm_rec.est_no, comm_rec.cc_no);
			print_at (22,0, "%s", err_str);
			scn_write (scn);	/*Draw prompts only */
			break;
		
		case SCN_HEADER:
			box (0,2,screenWidth,3);
			/*------------------------
			| Vehicle           :  %s|
			| Expected Del Date :  %s|
			| Trip Name         :  %s|
			------------------------*/
			print_at (3,5, ML (mlTrMess009),local_rec.ref2);
			print_at (3,59," %s", local_rec.desc2);
			print_at (4,5, ML (mlTrMess011),DateToString (local_rec.del_date2));
			print_at (5,5, ML (mlTrMess012), local_rec.trip_no2);
			break;
		
		case SCN_TABLE:
			tab_row = 6;
			tab_col = 16;
			break;
		
		default:
			break;
		}

    return (EXIT_SUCCESS);
}
