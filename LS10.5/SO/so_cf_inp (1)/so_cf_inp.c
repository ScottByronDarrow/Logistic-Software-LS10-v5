/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_cf_inp.c,v 5.7 2002/07/24 08:39:22 scott Exp $
|  Program Name  : (so_cf_inp.c)
|  Program Desc  : (Carrier file maintanence)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/04/91         |
|---------------------------------------------------------------------|
| $Log: so_cf_inp.c,v $
| Revision 5.7  2002/07/24 08:39:22  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.6  2002/07/16 08:51:50  scott
| S/C 004153 - Updated as read with lock not performed.
|
| Revision 5.5  2002/06/20 07:15:58  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2002/06/20 05:48:57  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2002/02/22 07:02:35  scott
| S/C 00773 - Added sleep on warning message. Changed so Restart would not save record.
|
| Revision 5.2  2001/08/09 09:20:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:04  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:19:13  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/15 11:00:38  robert
| updated to include scn_set (2) before getting the table values of screen 2.
| needed to properly get all the values in the table.
|
| Revision 4.0  2001/03/09 02:40:32  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/06 04:13:38  scott
| Updated to correct problems with LS10-GUI version
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_cf_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_cf_inp/so_cf_inp.c,v 5.7 2002/07/24 08:39:22 scott Exp $";

/*===========================================
| These next two lines for 132 tabular only |
===========================================*/
#define MAXWIDTH 	150
#define MAXLINES 	500
#define TABLINES 	7

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

	/*
	 * Special fields and flags.
	 */
   	int  	new_carr = 0;	

#include	"schema"

struct commRecord	comm_rec;
struct cfhrRecord	cfhr_rec;
struct cflnRecord	cfln_rec;
struct exafRecord	exaf_rec;

	char	*scn_desc [] = {
		"Carrier Header information.",
		"Carrier Detail lines."
	};

	struct storeRec {
		char	workArea [3];
	} store [MAXLINES];

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	area_code [3];
	char	areaDesc [41];
	double	cost_kg;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "carr_code", 3, 19, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "Carrier Code.", " [SEARCH] for valid carriers.", 
		NE, NO, JUSTLEFT, "", "", cfhr_rec.carr_code}, 
	{1, LIN, "carr_name", 4, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Carrier Name", " ", 
		YES, NO, JUSTLEFT, "", "", cfhr_rec.carr_desc}, 
	{1, LIN, "carr_markup", 6, 19, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0.00", "Carrier Markup % ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&cfhr_rec.markup_pc}, 
	{1, LIN, "carr_phone", 7, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "Carrier Phone No", " ", 
		YES, NO, JUSTLEFT, "", "", cfhr_rec.phone}, 
	{1, LIN, "carr_fax", 7, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "Carrier Fax No", " ", 
		YES, NO, JUSTLEFT, "", "", cfhr_rec.fax_no}, 
	{1, LIN, "carr_cont", 8, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Carrier Contact.", " ", 
		YES, NO, JUSTLEFT, "", "", cfhr_rec.contact_name}, 
	{2, TAB, "area_code", MAXLINES, 5, CHARTYPE, 
		"UU", "          ", 
		" ", " ", " Area Code. ", " ", 
		NE, NO, JUSTRIGHT, "", "", local_rec.area_code}, 
	{2, TAB, "areaDesc", 0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "     A R E A    D E S C R I P T I O N .   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.areaDesc}, 
	{2, TAB, "cost_kg", 0, 2, DOUBLETYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0.00", " Cost Per Kg. ", "", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.cost_kg}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};


/*
 * Function Declarations 
 */
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
int  	spec_valid 			 (int);
int  	CheckDuplicateArea 	 (char *, int);
void 	LoadCfln 			 (void);
void 	SrchCfhr 			 (char *);
void 	SrchArea 			 (char *);
void 	Update 				 (void);
int  	heading 			 (int);


/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char *	argv [])
{
	int		i;

	SETUP_SCR (vars);


	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	for (i = 0; i < 2; i++)
		tab_data [i]._desc = scn_desc [i];

	init_vars (1);			/*  set default values		*/

	tab_row = 10;
	tab_col = 3;

	OpenDB ();

	/*
	 * Beginning of input control loop . 
	 */
	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		lcount [2] = 0;
		init_vars (1);

		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;
		
		scn_write (1);
		scn_display (1);
		scn_write (2);
		scn_display (2);

		/*
		 * Enter screen 2 Tabular input. 
		 */
		if (lcount [2] == 0)
			entry (2);
		else
			edit (2);

		if (restart)
			continue;

		edit_all ();

		/*
		 * Update records. 
		 */
		if (!restart)
			Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequenc
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files
 */
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cfhr, cfhr_list, CFHR_NO_FIELDS, "cfhr_id_no");
	open_rec (cfln, cfln_list, CFLN_NO_FIELDS, "cfln_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (cfhr);
	abc_fclose (cfln);
	abc_fclose (exaf);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*------------------------------------------
	| Validate Customer Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("carr_code"))
	{
		if (SRCH_KEY)
		{
	 		SrchCfhr (temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (cfhr_rec.co_no, comm_rec.co_no);
		strcpy (cfhr_rec.br_no, comm_rec.est_no);
		cc = find_rec (cfhr, &cfhr_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (cfhr);
			new_carr = TRUE;
			return (EXIT_SUCCESS);
		}

		LoadCfln ();
		
		entry_exit = 1;

		new_carr = FALSE;

		DSP_FLD ("carr_name");
		DSP_FLD ("carr_phone");
		DSP_FLD ("carr_fax");
		DSP_FLD ("carr_cont");

		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Area Code. |
	---------------------*/
	if (LCHECK ("area_code"))
	{
		if (SRCH_KEY)
		{
			SrchArea (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,local_rec.area_code);
		cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (CheckDuplicateArea (exaf_rec.area_code, line_cnt))
		{
			print_mess (ML (mlSoMess312));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (store [line_cnt].workArea, local_rec.area_code);
		strcpy (local_rec.areaDesc, exaf_rec.area);

		DSP_FLD ("areaDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}	

/*==================================
| Check for duplicate area number. |
==================================*/
int
CheckDuplicateArea (
 char *area_no, 
 int line_no)
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

		/*------------------------
		| cannot duplicate area. |
		------------------------*/
		if (!strcmp (store [i].workArea , area_no))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
LoadCfln (
 void)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	init_vars (2);
	lcount [2] = 0;

	cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
	strcpy (cfln_rec.area_code, "  ");

	cc = find_rec (cfln, &cfln_rec, GTEQ, "r");
	while (!cc && cfhr_rec.cfhh_hash == cfln_rec.cfhh_hash)
	{
		fflush (stdout);

		strcpy (local_rec.area_code, cfln_rec.area_code);
		strcpy (store [lcount [2]].workArea, cfln_rec.area_code);
		local_rec.cost_kg = cfln_rec.cost_kg;

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,cfln_rec.area_code);
		if (find_rec (exaf, &exaf_rec, COMPARISON, "r"))
			sprintf (local_rec.areaDesc, "%40.40s", "?");
		else
			sprintf (local_rec.areaDesc, exaf_rec.area);
		
		putval (lcount [2]++);

		cc = find_rec (cfln, &cfln_rec, NEXT, "r");
	}

	new_carr = (lcount [2] == 0);
	
	scn_set (1);
}

/*
 * Search for Carrier. 
 */
void
SrchCfhr (
	char *key_val)
{
	_work_open (4,0,40);
	save_rec ("#No","#Carrier Description ");

	strcpy (cfhr_rec.co_no, comm_rec.co_no);
	strcpy (cfhr_rec.br_no, comm_rec.est_no);
	sprintf (cfhr_rec.carr_code,"%-4.4s", key_val);
	cc = find_rec (cfhr, &cfhr_rec, GTEQ, "r");
	while (!cc && !strcmp (cfhr_rec.co_no, comm_rec.co_no) &&
		      !strcmp (cfhr_rec.br_no, comm_rec.est_no) &&
		      !strncmp (cfhr_rec.carr_code, key_val,strlen (key_val)))
	{
		cc = save_rec (cfhr_rec.carr_code,cfhr_rec.carr_desc);
		if (cc)
			break;

		cc = find_rec (cfhr, &cfhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cfhr_rec.co_no, comm_rec.co_no);
	strcpy (cfhr_rec.br_no, comm_rec.est_no);
	sprintf (cfhr_rec.carr_code,"%-4.4s", temp_str);
	cc = find_rec (cfhr, &cfhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cfhr, "DBFIND");
}

/*==================
| Search for area. |
==================*/
void
SrchArea (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Area Description.");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) && 
		!strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code,exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf, &exaf_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}

/*===========================
| Update cfhr/cfln records. |
===========================*/
void
Update (void)
{
	clear ();
	strcpy (err_str, ML (mlStdMess035));
	print_at (0,0,err_str);

	strcpy (cfhr_rec.stat_flag, "0");
	if (new_carr)
	{
		cc = abc_add (cfhr, &cfhr_rec);
		if (cc)
			file_err (cc, cfhr, "DBADD");

		strcpy (cfhr_rec.co_no, comm_rec.co_no);
		strcpy (cfhr_rec.br_no, comm_rec.est_no);

		cc = find_rec (cfhr, &cfhr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cfhr, "DBADD");
	}
	else
	{
		cc = abc_update (cfhr, &cfhr_rec);
		if (cc)
			file_err (cc, cfhr, "DBUPDATE");
	}

	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);

		cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
		strcpy (cfln_rec.area_code, local_rec.area_code);
		cc = find_rec (cfln, &cfln_rec, COMPARISON, "u");
		if (cc)
		{
			cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
			cfln_rec.cost_kg = local_rec.cost_kg;
			strcpy (cfln_rec.area_code, local_rec.area_code);
			strcpy (cfln_rec.carr_code, cfhr_rec.carr_code);

			cc = abc_add (cfln, &cfln_rec);
			if (cc)
				file_err (cc, cfln, "DBADD");
		}
		else
		{
			cfln_rec.cost_kg = local_rec.cost_kg;

			cfln_rec.cost_kg = local_rec.cost_kg;
			cc = abc_update (cfln, &cfln_rec);
			if (cc)
				file_err (cc, cfln, "DBUPDATE");
		}
	}
}

int
heading (
 int scn)
{
	if (restart) 
	{
		abc_unlock (cfhr);
		return (EXIT_SUCCESS);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (ML (mlSoMess224),18,0,1);

	line_at (1,0,79);

	box (0,2,80,6);
	line_at (5,1,79);

	scn_set ((scn == 1) ? 2 : 1);
	scn_write ((scn == 1) ? 2 : 1);
	scn_display ((scn == 1) ? 2 : 1);
	
	line_at (19,0,79);
	print_at (20,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}
