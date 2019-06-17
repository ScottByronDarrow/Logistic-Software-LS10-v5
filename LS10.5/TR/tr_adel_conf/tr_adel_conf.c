/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: tr_adel_conf.c,v 5.3 2001/10/25 07:54:30 scott Exp $
|	Program Name : (tr_adel_conf.c)								  	  |
|	Program Desc : (Automatic Transport Delivery Confirmation.)       |
|---------------------------------------------------------------------|
|  Author        : Marnie I. Organo| Date Written  : 04/12/98         |
|---------------------------------------------------------------------|
| Date Modified  : (03/01/1999)    | Modified By  : Marnie I. Organo. |
-----------------------------------------------------------------------
| $Log: tr_adel_conf.c,v $
| Revision 5.3  2001/10/25 07:54:30  scott
| Updated to make changes related to container and seals.
|
| Revision 5.2  2001/08/09 09:22:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:53:45  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:21:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:42:47  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.6  2001/03/06 02:33:10  scott
| Updated to for buttons on LS10-GUI
|
| Revision 1.5  2001/01/25 01:13:09  scott
| Updated to fix small warning when compiled on Linux.
|
| Revision 1.4  2000/12/19 09:32:24  ramon
| For LS10-GUI compatibility, I added the necessary include files.
|
| Revision 1.3  2000/12/13 03:04:55  scott
| Removed getchar and debug statement
|
| Revision 1.2  2000/12/12 08:10:44  scott
| Removed unused code.
|
| Revision 1.1  2000/12/11 06:24:57  scott
| New Program taken from Ben Foods (once cleaned and make to shine)
|
| Comments :                                                          |
| (03/01/1999)   | Returned the previous SEARCH on Date and Trip No.  |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tr_adel_conf.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_adel_conf/tr_adel_conf.c,v 5.3 2001/10/25 07:54:30 scott Exp $";

#define SCN_INIT	  1
#define SCN_HEADER	  3
#define MAXSCNS		  2
#define MAXLINES	500
#define TABLINES	 14
#include <pslscr.h>
#include <hot_keys.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <ml_tr_mess.h>
#include <ml_std_mess.h>
#include <tabdisp.h>
#include <assert.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		ScrWidth = 130;

	char	*sptr;

#include	"schema"

struct commRecord	comm_rec;
struct colnRecord	coln_rec;
struct inmrRecord	inmr_rec;
struct trveRecord	trve_rec;
struct trveRecord	trve2_rec;
struct trlnRecord	trln_rec;
struct trhrRecord	trhr_rec;
struct trhrRecord	trhr2_rec;
struct trhrRecord	trhr3_rec;
struct cohrRecord	cohr_rec;
struct cumrRecord	cumr_rec;
struct skniRecord	skni_rec;

	char	*trhr3	=	"trhr3",
			*data	=	"data",
			*trve2	=	"trve2";


/*===========================
| Local & Screen Structures. |
============================*/
struct
{
	long	del_date;
	long	del_date2;
	char	trip_no [13];
	char	trip_no2 [13];
	char	ref [11];
	char	ref2 [11];
	char	desc [41];
	char	desc2 [41];
	int		ln_no;
	char	dummy [11];
} local_rec; 

static	struct	var	vars [] =
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

int		TOGGLE = FALSE;
int		INIT_TOGGLE = FALSE;
int		confirm = FALSE;
int		heading	 			(int);
void	shutdown_prog		(void);
void 	SrchTrhrTrip 		(char *);
void 	UpdateTrhr 			(void);
void 	SrchTrhr 			(char *);
void 	SrchTrve 			(char *);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
static 	void InitFiles 		(void);

char	*TransportAllocation	=	"TrAlloc";

char		*tableFormat = " %-1.1s      %-8.8s %-6.6s  %-100.100s %10ld %10ld %10ld %10ld";

static	char    tableBuffer [256];
static int	line_no;
static int	curr_scn = SCN_INIT;

static struct {
	long		hhtrHash;	
	long		hhcoHash;	
	char		status [2];
	} packingSlipStore [MAXLINES];

static int		noInDsp = 0;	/* number of lines added to table */	

static  int Confirm 		(int, KEY_TAB *);
static  int RestartFunc 	(int, KEY_TAB *);
static  int ExitFunc 		(int, KEY_TAB *);
static	int	TagFunc 		(int, KEY_TAB *);
static	int	UnTagFunc 		(int, KEY_TAB *);

#define DETAIL_KEY		'C'
#define HEADER_KEY		'H'
#define TAG	            'T' 
#define UNTAG	        'U' 

#ifdef	GVISION
static  KEY_TAB list_keys [] =
{
   { " Tag", TAG , TagFunc,
    "Tag current line.",                  "A" },
   { " Untag", UNTAG , UnTagFunc,
    "Untag current line.",                  "A" },
   { " Confirm ",		DETAIL_KEY,			Confirm,
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
   { " [T]ag", TAG , TagFunc,
    "Tag current line.",                  "A" },
   { " [U]ntag", UNTAG , UnTagFunc,
    "Untag current line.",                  "A" },
   { " [C]onfirm ",		DETAIL_KEY,			Confirm,
    "Confirm Deliver ",						"A" },
   { NULL,				FN1,				RestartFunc,
    "Exit without update.",					"A" },
   { NULL,				FN16,				ExitFunc,
    "Exit and update the database.",		"A" },
   END_KEYS
};
#endif

static	int
TagFunc (
	int			iUnused,
	KEY_TAB*	psUnused)
{
    int     old_line;

	line_no 		= tab_tline (TransportAllocation);
	local_rec.ln_no = line_no;

	if (!strcmp (packingSlipStore [line_no].status, "D"))
	{
		print_err (ML (mlTrMess022));
		sleep (sleepTime);
		clear_mess ();
		abc_selfield (trhr, "trhr_id_no");
		return (EXIT_FAILURE);     
	}

    old_line = tab_tline (TransportAllocation);
    cc = tab_get (TransportAllocation, tableBuffer, EQUAL, old_line);
    if (!cc)
    {
        tableBuffer [1] = '*';
        tab_update (TransportAllocation, "%s", tableBuffer);
        cc = tab_get (TransportAllocation, tableBuffer, NEXT, 0);
        if (cc)
            cc = tab_get (TransportAllocation, tableBuffer, EQUAL, old_line);
    }
    cc = tab_get (TransportAllocation, tableBuffer, EQUAL, 0);

	TOGGLE = TRUE;		
	INIT_TOGGLE = FALSE;
	return			iUnused;
}

static	int
Confirm (
	int			iUnused,
	KEY_TAB*	psUnused)
{
    int     old_line;
	char	cmd [60];

	line_no = tab_tline (TransportAllocation);
	local_rec.ln_no = line_no;

	if (!strcmp (packingSlipStore [line_no].status, "D"))
	{
		print_err (ML (mlTrMess022));
		sleep (sleepTime);
		clear_mess ();
		abc_selfield (trhr, "trhr_id_no");
		return (EXIT_FAILURE);     
	}

    old_line = tab_tline (TransportAllocation);
    cc = tab_get (TransportAllocation, tableBuffer, EQUAL, old_line);
    if (!cc)
    {
        tableBuffer [1] = 'C';
        tab_update (TransportAllocation, "%s", tableBuffer);

		sprintf 
		(
			cmd,
			"tr_delivery %010ld %010ld %010ld %010ld", 	
			atol (tableBuffer + 126),
			atol (tableBuffer + 137),
			atol (tableBuffer + 148),
			atol (tableBuffer + 159)
		);
		sys_exec (cmd);
		confirm = TRUE;
        cc = tab_get (TransportAllocation, tableBuffer, NEXT, 0);
        if (cc)
            cc = tab_get (TransportAllocation, tableBuffer, EQUAL, old_line);
    }
    old_line 	= tab_tline (TransportAllocation);
	confirm 	= TRUE;
	restart 	= FALSE;
    return (FN16); 
}

static	int
UnTagFunc (
	int			iUnused,
	KEY_TAB*	psUnused)
{
    int     old_line;

	line_no = tab_tline (TransportAllocation);
	local_rec.ln_no = line_no;

	if (!strcmp (packingSlipStore [line_no].status, "D"))
	{
		print_err (ML (mlTrMess022));
		sleep (sleepTime);
		clear_mess ();
		abc_selfield (trhr, "trhr_id_no");
		return (EXIT_FAILURE);     
	}

    old_line = tab_tline (TransportAllocation);
    cc = tab_get (TransportAllocation, tableBuffer, EQUAL, old_line);
    if (!cc)
    {
        tableBuffer [1] = ' ';
        tab_update (TransportAllocation, "%s", tableBuffer);
        cc = tab_get (TransportAllocation, tableBuffer, NEXT, 0);
        if (cc)
            cc = tab_get (TransportAllocation, tableBuffer, EQUAL, old_line);
    }
    cc = tab_get (TransportAllocation, tableBuffer, EQUAL, 0);
	TOGGLE = FALSE;
	return			iUnused;
}

/*=================
| Init trld table |
=================*/
static int
InitTrlds ()
{
	char	tot_add [200];

	noInDsp = 0;

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
			
			strcpy (tot_add, clip (cumr_rec.dl_adr1));
			strcat (tot_add," ");
			strcat (tot_add, clip (cumr_rec.dl_adr2));
			strcat (tot_add," ");
			strcat (tot_add, clip (cumr_rec.dl_adr3));
			strcat (tot_add," ");
			strcat (tot_add, cumr_rec.dl_adr4);

			memset 
			(
				(char *) &packingSlipStore [noInDsp], 
				0, 
				sizeof (packingSlipStore [noInDsp])
			);

			packingSlipStore [noInDsp].hhtrHash = trhr_rec.hhtr_hash;
			packingSlipStore [noInDsp].hhcoHash = cohr_rec.hhco_hash;
			strcpy (packingSlipStore [noInDsp].status , cohr_rec.status); 

			noInDsp++;
			if (!strcmp (cohr_rec.status, "D"))
			{
				tab_add 
				(
					TransportAllocation, 
					tableFormat,
					cohr_rec.status , 
					cohr_rec.inv_no , 
					cumr_rec.dbt_no , 
					tot_add, 
					trhr_rec.hhve_hash,
					trhr_rec.hhtr_hash,
					cohr_rec.hhco_hash,
					cohr_rec.hhcu_hash,
					"A"
				);
			}
			else
			{
				tab_add 
				(
					TransportAllocation, 
					tableFormat,
					"*" , 
					cohr_rec.inv_no , 
					cumr_rec.dbt_no , 
					tot_add , 
					trhr_rec.hhve_hash,
					trhr_rec.hhtr_hash,
					cohr_rec.hhco_hash,
					cohr_rec.hhcu_hash,
					"A"
				);
				INIT_TOGGLE = TRUE;
        		tableBuffer [1] = '*';
			}
			cc = find_rec (trln, &trln_rec, NEXT, "r");
		}
	}
	abc_selfield (cohr, "cohr_hhtr_hash");
	abc_selfield (trhr, "trhr_id_no");
	return (EXIT_SUCCESS);
}

/*========================
| Display trlds in Table |
========================*/
static int
ProcessTrlos ()
{
	int i;
	int	responceKey	=	' ';
	char	cmd [60];

	tab_open (TransportAllocation, list_keys, 7, 1, 10, FALSE);
	tab_add (TransportAllocation, "# %-6.6s %-8.8s %-8.8s  %-100.100s",
			"Status",
			"Inv No",
			"Customer",
			"            Delivery Address");
		
	assert (strlen (tableBuffer)  < sizeof tableBuffer);

	InitTrlds ();

	if (noInDsp == 0)
	{
		/*--------------
		| No  record ! |
		--------------*/
		errmess (ML (mlStdMess009));
		sleep (sleepTime);
		tab_close (TransportAllocation, TRUE);
		curr_scn =	SCN_INIT;
		restart = TRUE;
		return (EXIT_FAILURE);
	}
	else
	{
		tab_scan (TransportAllocation);
	}

	if ((TOGGLE || !prog_exit) && !confirm)
		responceKey = prmptmsg ("Update/Untag [Y/N]? Press 'N' to RESTART.","YyNn",30,23);


	if (!prog_exit  && !confirm && (responceKey == 'Y'  || responceKey == 'y'))
	{
		for (i = 0; i < noInDsp; i++)
		{
			tab_get (TransportAllocation, tableBuffer, EQUAL, i);

			tag_unset (TransportAllocation);

			if (INIT_TOGGLE)
			{
				sprintf (tableBuffer, "%-2.2s", tableBuffer);
				tableBuffer [1] = tableBuffer [1];
				TOGGLE = TRUE;
			}

			switch (tableBuffer [1])
			{
			case 'A' :
			case '*' :
				if (TOGGLE && !restart)
				{
					sprintf (cmd,"tr_tagfunc %10ld %10ld %10ld %10ld", 	
						atol (tableBuffer + 126),
						atol (tableBuffer + 137),
						atol (tableBuffer + 148),
						atol (tableBuffer + 159));
					sys_exec (cmd);
				}
				break;

			case ' ':
				break;
			}
		}
    }

	tab_close (TransportAllocation, TRUE);
	return (EXIT_SUCCESS);
}


static int
RestartFunc (
 int 		key,
 KEY_TAB*	psUnused)
{
	assert (key == FN1);
	curr_scn = SCN_INIT;
	restart  = TRUE;
	return key;
}

static int
ExitFunc (
 int 	key,
 KEY_TAB*	psUnused)
{
	assert (key == FN16);
	curr_scn = SCN_INIT;
	if (confirm || INIT_TOGGLE)		
		restart  = FALSE;
	else
		restart  = TRUE;

	confirm = FALSE;		
	return key;
}

/*=====================================================================
| Main Processing Routine.
=====================================================================*/
int
main (
 int        argc,
 char*      argv [])
{
	SETUP_SCR 	 (vars);
	init_scr 	 ();
	set_tty 	 ();
	set_masks 	 ();
	
	OpenDB		 ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);
	clear		 ();

	while (!prog_exit)
	{
		InitFiles ();
		search_ok 	= 	1;
		entry_exit 	= 	0;
		edit_exit 	= 	0;
		prog_exit 	= 	0;
		restart 	= 	0;
		init_ok 	= 	1;
		init_vars 	 (1);	
		heading 	 (SCN_INIT);
		entry 		 (SCN_INIT);   
		if (!restart && !prog_exit)
			ProcessTrlos ();  

		while (!restart && !prog_exit)
		{
			strcpy (local_rec.ref2, local_rec.ref);
			local_rec.del_date2 = local_rec.del_date;
			strcpy (local_rec.trip_no2, local_rec.trip_no);
			strcpy (local_rec.desc2, local_rec.desc);

			heading 	 (SCN_HEADER);

			if (confirm)
				ProcessTrlos ();
			
			if (!confirm)
				prog_exit = FALSE;

			if (curr_scn == SCN_INIT  && !confirm)
				restart = TRUE;
		}

		if (noInDsp)
			UpdateTrhr ();  
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}


/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================= 
| Open data base files. |
======================= */
void
OpenDB (void)
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
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash"); 
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_hhcl_hash"); 
	open_rec (skni,  skni_list, SKNI_NO_FIELDS, "skni_hhcl_hash"); 

}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (trhr);
	abc_fclose (trhr3);
	abc_fclose (cohr);
	abc_fclose (trln);
	abc_fclose (trve);
	abc_fclose (trve2);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (skni);
	abc_fclose (coln);
	abc_dbclose (data);
}

/*------------------+
|  Field Validation |
------------------*/
int
spec_valid (
 int	field)
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
			SrchTrve (temp_str);
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
			SrchTrhr (temp_str);
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
			SrchTrhrTrip (temp_str);
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
SrchTrve (
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
		file_err (cc, (char *)trve, "DBFIND");
}

/*====================+
| Search for Date     |
---------------------*/
void
SrchTrhr (
	char	*key_val)
{
	int		trhrFound	=	FALSE;

	work_open ();
	save_rec ("#Tr Date  Trip Number", "#Vehicle Number");
	trhr_rec.hhve_hash = trve_rec.hhve_hash;
	trhr_rec.del_date  = StringToDate (key_val);
	cc = find_rec (trhr, &trhr_rec, GTEQ, "r");
	while (!cc &&  
		!strncmp (DateToString (trhr_rec.del_date),key_val,strlen (key_val)) &&
		trhr_rec.hhve_hash == trve_rec.hhve_hash)
	{
		if (strcmp (trhr_rec.status, "D"))
		{
			sprintf (err_str,"%-10.10s %-12.12s",
						DateToString (trhr_rec.del_date), trhr_rec.trip_name);

			cc = save_rec (err_str, trve_rec.ref);
			if (cc)
				break;
		}
		cc = find_rec (trhr,&trhr_rec,NEXT,"r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	trhrFound = FALSE;

	trhr_rec.hhve_hash = trve_rec.hhve_hash;
	trhr_rec.del_date  = StringToDate (temp_str);
	cc = find_rec (trhr, &trhr_rec, GTEQ, "r");
	while (!cc)
	{
		if (!strcmp (trhr_rec.trip_name, temp_str + 11))
		{
			trhrFound = TRUE;
			strcpy (local_rec.trip_no, trhr_rec.trip_name);
			break;
		}
		cc = find_rec (trhr,&trhr_rec,NEXT,"r");
	}

	if (!trhrFound)
		file_err (cc, "trhr", "DBFIND");
}

/*================================================
| Search for Trip Number Transport Header File . |
================================================*/
void
SrchTrhrTrip (
	char *key_val)
{
	work_open ();
	save_rec ("#Trip Number", "#Del Date");
	sprintf (trhr3_rec.trip_name, "%-12.12s", key_val);
	cc = find_rec (trhr3, &trhr3_rec, GTEQ, "r");
	while (!cc 
			&&  !strncmp (trhr3_rec.trip_name,key_val,strlen (key_val)))
	{
		if (trhr3_rec.status [0] != 'D' &&
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

static void
InitFiles (void)
{
	memset (&trhr_rec,0,sizeof (trhr_rec));
	memset (&trln_rec,0,sizeof (trln_rec));
	memset (&trve_rec,0,sizeof (trve_rec));
	memset (&cohr_rec,0,sizeof (cohr_rec));
	memset (&cumr_rec,0,sizeof (cumr_rec));
	memset (&local_rec,0,sizeof (local_rec));

}
int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	swide ();
	clear ();

	/*--------------------------------------------------
	|%R Transport & Sales Liq - Input Delivery/Returns |
	--------------------------------------------------*/
	centre_at (0, ScrWidth, ML (mlTrMess038));
	move (0, 1); line (ScrWidth + 1);

	move (0, 20); line (ScrWidth + 1);
	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	move (0, 22); line (ScrWidth + 1);
	
	switch (scn)
	{
		case SCN_INIT:
			box (0,2,ScrWidth,3);
			scn_write (scn);	/*Draw prompts only */
		break;
		
		case SCN_HEADER:
			box (0,2,ScrWidth,3);
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
		
		default:
			assert (FALSE);
	}
	return 0;
}

void
UpdateTrhr (void)
{
	int		i,
			allConfirmed = TRUE;
		
	for (i = 0; i < noInDsp; i++)
	{
		if (packingSlipStore [i].status [0] == 'D')
		{
			coln_rec.hhco_hash 	= packingSlipStore [i].hhcoHash;
			coln_rec.line_no 	= 0;
			cc = find_rec (coln, &coln_rec, GTEQ, "r");
			while (!cc && coln_rec.hhco_hash == packingSlipStore [i].hhcoHash)
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
			allConfirmed = FALSE;
	}

	if (allConfirmed)
	{
		abc_selfield (trhr, "trhr_hhtr_hash");
		trhr_rec.hhtr_hash = packingSlipStore [0].hhtrHash;
		cc = find_rec (trhr, &trhr_rec, EQUAL, "u");
		if (!cc)
		{
			strcpy (trhr_rec.status, "D");
			cc = abc_update (trhr, &trhr_rec);
			if (cc)
				file_err (cc, (char *)trhr, "DBFIND");
		}
		abc_selfield (trhr, "trhr_id_no");
	}
}
