/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_app_mnt.c,v 5.7 2002/11/20 09:00:46 kaarlo Exp $
|  Program Name  : (po_app_mnt.c & po_app_inp.c)                      |
|  Program Desc  : (Maintain purchase order approval)                 |
|                  (                                            )     |
|---------------------------------------------------------------------|
| $Log: po_app_mnt.c,v $
| Revision 5.7  2002/11/20 09:00:46  kaarlo
| SC00039. Updated to use NN,NNN,NNN,NNN.NN" instead of "NNN,NNN,NNN,NNN.NN" for app_limit (Oracle only).
|
| Revision 5.6  2002/07/25 11:17:31  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.5  2001/10/19 02:44:22  cha
| Fix Issue # 00626 by Scott.
|
| Revision 5.4  2001/10/17 08:47:21  robert
| Update from Scott's machine
|
| Revision 5.3  2001/09/10 10:31:40  cha
| SE-90.
|
| Revision 5.2  2001/08/09 09:15:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:36  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:00  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:19  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.5  2001/03/06 01:21:07  scott
| Updated for LS10-GUI buttons.
|
| Revision 3.4  2001/03/01 23:18:27  scott
| Updated to make LS10-GUI look better.
|
| Revision 3.3  2001/02/03 01:21:28  scott
| Updated as password being shown on edit.
|
| Revision 3.2  2000/11/20 07:39:10  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.1  2000/10/13 05:41:33  ramon
| Added sleep() call to see the error message in LS10-GUI version.
|
| Revision 3.0  2000/10/10 12:17:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:04:55  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.18  1999/12/10 04:14:49  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.17  1999/11/16 22:44:10  scott
| General clean up.
|
| Revision 1.16  1999/11/11 06:43:08  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.15  1999/11/05 05:17:07  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.14  1999/10/14 03:04:19  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.13  1999/09/29 10:11:51  scott
| Updated to be consistant on function names.
|
| Revision 1.12  1999/09/21 04:37:54  scott
| Updated from Ansi project
|
| Revision 1.11  1999/06/17 10:06:13  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_app_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_app_mnt/po_app_mnt.c,v 5.7 2002/11/20 09:00:46 kaarlo Exp $";

#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>
#include <tabdisp.h>

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poapRecord	poap_rec;
struct inmrRecord	inmr_rec;

	char	*data = "data";

	extern	int		EnvScreenOK;

	char	mlAppMnt [10] [101];
            
	int		envVarDbCo			= 0,
			envVarDbFind		= 0,
			numberAllocations	= 0,
			newCode 	 		= FALSE,
			maintOption 		= FALSE;

	char	branchNumber [3];

static	int	tag_func (int c, KEY_TAB *psUnused);

#ifdef	GVISION
static	KEY_TAB pc_keys [] = 
{
    { " TAG/UNTAG ",	'T', tag_func,
	"Tag/Untag current line.",					"A" },
    { " All TAG/UNTAG ",	'A', tag_func,
	"Tag/Untag All Lines.",						"A" },
    END_KEYS
};
#else
static	KEY_TAB pc_keys [] = 
{
    { " [T]AG/UNTAG ",	'T', tag_func,
	"Tag/Untag current line.",					"A" },
    { " [A]ll TAG",	'A', tag_func,
	"Tag/Untag All Lines.",						"A" },
    END_KEYS
};
#endif

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	char	passwd [14];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "app_code",	 3, 22, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "Approval Code. ", "",
		 NE, NO,  JUSTLEFT, "", "", poap_rec.app_code},
	{1, LIN, "app_name",	 4, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Approval Name.      ", " ",
		 YES, NO,  JUSTLEFT, "", "", poap_rec.app_name},
	{1, LIN, "passwd",	 6, 22, CHARTYPE,
		"_____________", "          ",
		" ", "", "Approval Password ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.passwd},
#ifdef ORA734
	{1, LIN, "app_limit",	 8, 22, MONEYTYPE,
		"NN,NNN,NNN,NNN.NN", "          ",
		" ", " ", "Approval Limit. ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&poap_rec.app_limit},
#else
	{1, LIN, "app_limit",	 8, 22, MONEYTYPE,
		"NNN,NNN,NNN,NNN.NN", "          ",
		" ", " ", "Approval Limit. ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&poap_rec.app_limit},
#endif
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*======================= 
| Function Declarations |
=======================*/
void 	InitML 			(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	SrchPoap 		(char *);
void 	OpenAllocate 	(void);
void 	StoreApp 		(double);
void 	LoadApp		 	(void);
void 	UpdateTab 		(void);
void 	UpdatePoln 		(long, char *, char *);
double 	CalculatePo 	(long);
int 	spec_valid 		(int);
int 	heading 		(int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr ==  (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp  (sptr, "po_app_mnt"))
		maintOption = TRUE;
	else
		maintOption = FALSE;

	EnvScreenOK	=	FALSE;

	SETUP_SCR (vars);


	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	envVarDbCo 		= atoi (get_env ("DB_CO"));
	envVarDbFind 	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	InitML ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	strcpy (local_rec.systemDate, DateToString  (TodaysDate ()));

	FLD ("app_limit") = (maintOption) ? YES : ND;
	FLD ("app_name")  = (maintOption) ? YES : NA;
		
	/*=====================
	| Reset control flags |
	=====================*/
	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		init_vars (1);

		init_ok	= TRUE;
		eoi_ok	= TRUE;
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		if (maintOption)
		{
			edit_all ();
			if (restart)
				continue;

			Update ();
		}
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
InitML (void)
{
	strcpy (mlAppMnt [1], ML  ("NO UNAPPROVED PURCHASE ORDERS EXIST"));
}

void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS,  (char *) &comm_rec);
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhsu_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (poap, poap_list, POAP_NO_FIELDS, "poap_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

void
CloseDB (void)
{
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (poap);
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("app_code"))
	{
		if (SRCH_KEY)
		{
			SrchPoap (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (poap_rec.co_no, comm_rec.co_no);
		newCode = find_rec (poap, &poap_rec, COMPARISON, "r");
		if (!maintOption && newCode)
		{
			errmess (ML (mlStdMess179));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (!newCode && maintOption)
		{
			strcpy (local_rec.passwd , poap_rec.passwd);
			entry_exit = TRUE;
		}
		DSP_FLD ("app_name");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("passwd"))
	{
		if (maintOption)
		{
			strcpy (poap_rec.passwd, local_rec.passwd);
			return (EXIT_SUCCESS);
		}
		if (strcmp (local_rec.passwd , poap_rec.passwd))
		{
			errmess (ML (mlStdMess180));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		OpenAllocate ();
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============
| Update Lines. |
===============*/
void
Update (void)
{
	clear ();
	
	strcpy (poap_rec.stat_flag, "0");
	
	if (newCode)
	{
		cc = abc_add (poap, &poap_rec);
		if (cc)
			file_err (cc, poap, "DBADD");
	}
	else
	{
		cc = abc_update (poap, &poap_rec);
		if (cc)
			file_err (cc, poap, "DBUPDATE");
	}
}

/*===============================
| Search on approval code file. |
===============================*/
void
SrchPoap (
 char *key_val)
{
	work_open ();
	save_rec ("#Name","#Description");
	strcpy (poap_rec.co_no,comm_rec.co_no);
	sprintf (poap_rec.app_code,"%-15.15s",key_val);

	cc = find_rec (poap ,&poap_rec,GTEQ,"r");

	while (!cc && !strcmp (poap_rec.co_no,comm_rec.co_no) &&
		!strncmp (poap_rec.app_code, key_val, strlen (key_val)))
	{
		cc = save_rec (poap_rec.app_code,poap_rec.app_name);
		if (cc)
			break;

		cc = find_rec (poap , &poap_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (poap_rec.co_no,comm_rec.co_no);
	sprintf (poap_rec.app_code,"%-15.15s",key_val);

	if ((cc = find_rec (poap ,&poap_rec,COMPARISON,"r")))
		sys_err ("Error in poap during (DBFIND)",cc,PNAME);
}

/*======================================================================
| Open up allocation window and call relevent functions to store data. |
======================================================================*/
void
OpenAllocate (void)
{
	double	tot_amount = 0.00;

	numberAllocations = 0;

	tab_open ("po_app", pc_keys, 7, 0, 10, FALSE);
	tab_add ("po_app",
		"# PURCHASE ORDER |SUPP NO.|        SUPPLIER NAME         |CURR| LOCAL P/O VALUE");

	abc_selfield (pohr, "pohr_up_id");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	strcpy (pohr_rec.status, "U");
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
		       !strcmp (pohr_rec.br_no, comm_rec.est_no) &&
		       pohr_rec.status [0] == 'U')
	{
		tot_amount = CalculatePo (pohr_rec.hhpo_hash);

		if (DOLLARS (poap_rec.app_limit) > tot_amount)
			StoreApp (tot_amount);

		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	LoadApp ();
}
/*===================================
| Store details of allocation type. | 
===================================*/
void
StoreApp (
 double tot_amt)
{
	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	if (find_rec (sumr,&sumr_rec,EQUAL,"r"))
		sprintf (sumr_rec.crd_no,"%-6.6s", "      ");
		
	tab_add ("po_app",
		" %15.15s| %6.6s |%-30.30s|%3.3s |%14.14s     %08ld",
				pohr_rec.pur_ord_no,
				sumr_rec.crd_no,
				sumr_rec.crd_name,
				pohr_rec.curr_code,
				comma_fmt (tot_amt, "NNN,NNN,NNN.NN"),
				pohr_rec.hhpo_hash);
	numberAllocations++;
}

double	
CalculatePo (
	long	hhpoHash)
{
	double	lineTotal = 0.00,
			poTotal = 0.00;

	poln_rec.hhpo_hash 	= hhpoHash;
	poln_rec.line_no 	= 0;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			inmr_rec.outer_size = 1.00;

		lineTotal = twodec (poln_rec.land_cst);
		lineTotal = out_cost (lineTotal, inmr_rec.outer_size);
		lineTotal *= (double) (poln_rec.qty_ord - poln_rec.qty_rec);
		poTotal += lineTotal;
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}

	return (poTotal);
}

/*======================
| Load stored details. |
======================*/
void
LoadApp (void)
{
	if (numberAllocations > 0)
	{
		cc = tab_scan ("po_app");
	        if (cc != 1) 
                UpdateTab ();
	}
	else
	{
		tab_add ("po_app", "         ** %-35.35s **         ", mlAppMnt [1]);
		tab_display ("po_app", TRUE);
		putchar (BELL);
		fflush (stdout);
		sleep (sleepTime);
	}
	tab_close ("po_app", TRUE);
}

/*======================
| Update tagged lines. |
======================*/
void
UpdateTab (void)
{
	int	i;
	char	get_buf [300];

	abc_selfield (pohr, "pohr_hhpo_hash");
	abc_selfield (sumr, "sumr_hhsu_hash");

	/*--------------------------
	| Process all tagged lines |
	--------------------------*/
	for (i = 0; i < numberAllocations; i++)
	{
		tab_get ("po_app", get_buf, EQUAL, i);

		pohr_rec.hhpo_hash = atol (get_buf + 81);
		cc = find_rec (pohr, &pohr_rec,EQUAL,"u");
		if (cc)
		{
			abc_unlock (pohr);
			continue;
		}

		if (!tagged (get_buf))
		{
			if (pohr_rec.status [0] == 'U')
			{
				abc_unlock (pohr);
				continue;
			}
			strcpy (pohr_rec.status, "U");
			cc = abc_update (pohr, &pohr_rec);
			if (cc)
				file_err (cc, pohr, "DBUPDATE");

			UpdatePoln (pohr_rec.hhpo_hash, "O", "U");

			continue;
		}
		
		if (pohr_rec.status [0] == 'O')
		{
			abc_unlock (pohr);
			continue;
		}

		strcpy (pohr_rec.status, "O");
		strcpy (pohr_rec.app_code, poap_rec.app_code);
		cc = abc_update (pohr, &pohr_rec);
		if (cc)
			file_err (cc, pohr, "DBUPDATE");

		UpdatePoln (pohr_rec.hhpo_hash, "U", "O");
	}
	abc_selfield (pohr, "pohr_up_id");
}
/*=====================================
| Allow user to tag lines for release |
=====================================*/
static	int 
tag_func (
	int 	c, 
	KEY_TAB *psUnused)
{
	if (c == 'T')
		tag_toggle ("po_app");
	else
		tag_all ("po_app");

	return (c);
}
/*==============================
| Update purchase order lines. |
==============================*/
void
UpdatePoln (
	long 	hhpoHash, 
	char 	*FromStatus, 
	char 	*toStatus)
{
	poln_rec.hhpo_hash = hhpoHash;
	poln_rec.line_no = 0;
	cc = find_rec (poln, &poln_rec, GTEQ, "u");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		if (poln_rec.pur_status [0] == FromStatus [0])
		{
			strcpy (poln_rec.pur_status, toStatus);
			cc = abc_update (poln, &poln_rec);
			if (cc)
				file_err (cc, poln, "DBUPDATE");
		}
		else
			abc_unlock (poln);

		cc = find_rec (poln, &poln_rec, NEXT, "u");
	}
	abc_unlock (poln);
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

		if (maintOption)
			rv_pr (ML (mlPoMess127),20,0,1);
		else
			rv_pr (ML ("Purchase order Approval"),20,0,1);

		line_at (1,0,80);

		switch (scn)
		{
		case	1:
			box (0, 2, 80, (maintOption) ? 6 : 4);
			line_at (5,1,79);
			if (maintOption)
				line_at (7,1,79);
			
			break;

		default:
			break;
		}

		line_at (21,0,80);
		print_at (22,0, ML (mlStdMess038) , comm_rec.co_no, comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
