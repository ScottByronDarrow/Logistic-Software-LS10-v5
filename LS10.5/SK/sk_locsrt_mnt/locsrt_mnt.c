/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_locsrt_mnt.c )                                |
|  Program Desc  : ( Location Sort Maintenance                    )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: locsrt_mnt.c,v $
| Revision 5.4  2002/11/28 09:27:32  kaarlo
| LS00737 SC3994. Updated to fix search problem on "sortValue".
|
| Revision 5.3  2002/07/08 06:58:38  scott
| S/C 004069 - Updated for lineup on GUI
|
| Revision 5.2  2001/08/09 09:18:57  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:15  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:19  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:06  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/11/11 05:59:47  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.7  1999/11/03 07:32:07  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.6  1999/10/13 02:42:01  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.5  1999/10/08 05:32:31  scott
| First Pass checkin by Scott.
|
| Revision 1.4  1999/08/27 10:43:05  scott
| Updated for incorrect arguments to file_err
|
| Revision 1.3  1999/06/20 05:20:12  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: locsrt_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_locsrt_mnt/locsrt_mnt.c,v 5.4 2002/11/28 09:27:32 kaarlo Exp $";

#define TABLINES 12
#define MAXLINES 500

#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define HEADER 1
#define DETAIL 2

#define UPDATE 0
#define SEL_IGNORE 1
#define SEL_DELETE 2
#define DEFAULT 99

#define SLEEPTIME 2

#define COMM_NO_FIELDS 7

	struct dbview comm_list [COMM_NO_FIELDS] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"}
	};

	struct {
		int		term;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		char	tcc_no [3];
		char	tcc_name [41];
	} commRec;

#define CCMR_NO_FIELDS 4

	struct dbview ccmr_list [CCMR_NO_FIELDS] = {
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"}
	};

	struct {
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		long	hhcc_hash;
	} ccmrRec;

#define INSL_NO_FIELDS 3

	struct dbview insl_list [INSL_NO_FIELDS] = {
		{"insl_ccmr_hash"},
		{"insl_pick_sort"},
		{"insl_location"}
	};

	struct {
		long	ccmr_hash;
		char	pick_sort [11];
		char	location [11];
	} inslRec;

#define LOMR_NO_FIELDS 3

	struct dbview lomr_list [LOMR_NO_FIELDS] = {
		{"lomr_hhcc_hash"},
		{"lomr_location"},
		{"lomr_desc"}
	};

	struct {
		long	hhcc_hash;
		char	location [11];
		char	desc [41];
	} lomrRec;

static char * data	= "data",
			* ccmr	= "ccmr",
			* insl	= "insl",
            * insl2	= "insl2",
			* lomr	= "lomr";

struct tag_localRec
{
	char	sortValue [11];
	char	locationCode [11];
	char	locationDesc [41];
} localRec;

static struct var vars [] =
{
	{HEADER, LIN, "sortValue", 3, 12, CHARTYPE,
	 "UUUUUUUUUU", "          ",
	 "", "", " Sort Value  ", " ",
	 YES, NO, JUSTLEFT, "", "", localRec.sortValue},

	{DETAIL, TAB, "locationCode", MAXLINES, 0, CHARTYPE,
	 "UUUUUUUUUU", "          ",
	 " ", " ", "Location  ", " ",
	 YES, NO, JUSTLEFT, "", "", localRec.locationCode},
	{DETAIL, TAB, "description",	 0, 0, CHARTYPE,
	 "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
	 " ", " ", "Location Description                    ", " ",
	 NA, NO, JUSTLEFT, "", "", localRec.locationDesc},

	{ 0 }
};

int		newSortValue;

/*=======================
| Function Declarations |
=======================*/
int  UpdateMenu (void);
int  Update (void);
int  Delete (void);
int  spec_valid (int field);
void SrchInsl (char *keyVal);
void SrchLomr (char *keyVal);
void ReadDetail (void);
int  DeleteLine (void);
int  AlreadyExists (void);
void OpenDB (void);
void CloseDB (void);
int  heading (int screen);
void tab_other (int iline);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 1)
	{
		printf ("Usage:  %s\n", argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (HEADER);

	OpenDB ();


	strcpy (ccmrRec.co_no, commRec.tco_no);
	strcpy (ccmrRec.est_no, commRec.test_no);
	strcpy (ccmrRec.cc_no, commRec.tcc_no);
	cc = find_rec (ccmr, &ccmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	prog_exit = FALSE;
	while (!prog_exit)
	{
		entry_exit = FALSE;
		edit_exit = FALSE;
		prog_exit = FALSE;
		search_ok = TRUE;
		restart = FALSE;
		newSortValue = TRUE;
		lcount [DETAIL] = 0;
		init_vars (DETAIL);

		heading (HEADER);
		entry (HEADER);

		if (prog_exit || restart)
			continue;

		if (newSortValue)
		{
			heading (DETAIL);
			entry (DETAIL);
			if (prog_exit || restart)
				continue;
		}

		heading (DETAIL);
		edit (DETAIL);

		if (restart)
			continue;

		while (!UpdateMenu ())
		{
			heading (HEADER);
			edit (DETAIL);
		}
	}

	CloseDB (); 
	FinishProgram ();

	return (EXIT_SUCCESS);
}

MENUTAB updateMenu [] =
{
	{" 1. UPDATE RECORDS WITH CHANGES.      ", ""},
	{" 2. IGNORE CHANGES MADE TO RECORDS.   ", ""},
	{" 3. DELETE CURRENT RECORDS.           ", ""},
	{ENDMENU}
};

int
UpdateMenu (
 void)
{
	for (;;)
	{
		mmenu_print ("   U P D A T E    S E L E C T I O N.  ", updateMenu, 0);
		switch (mmenu_select (updateMenu))
		{
		  case UPDATE:
			return (Update ());
		  case SEL_IGNORE:
			return (TRUE);
		  case SEL_DELETE:
			return (Delete ());
		  case DEFAULT:
			return (Update ());
		}
	}
}

int
Update (
 void)
{
	int		i;
	int		oldScreen;

	if (!Delete ())
		return (FALSE);

	inslRec.ccmr_hash = ccmrRec.hhcc_hash;
	strcpy (inslRec.pick_sort, localRec.sortValue);

	oldScreen = cur_screen;
	scn_set (DETAIL);

	for (i = 0; i < lcount [DETAIL]; i++)
	{
		getval (i);
		strcpy (inslRec.location, localRec.locationCode);
		cc = abc_add (insl, &inslRec);
		if (cc)
			file_err (cc, insl, "DBFIND");
	}
	scn_set (oldScreen);
    return (TRUE);
}

int
Delete (void)
{
	inslRec.ccmr_hash = ccmrRec.hhcc_hash;
	strcpy (inslRec.pick_sort, localRec.sortValue);
	sprintf (inslRec.location, "%10.10s", " ");
	for (cc = find_rec (insl, &inslRec, GTEQ, "u");
		 !cc &&
		  inslRec.ccmr_hash == ccmrRec.hhcc_hash &&
		  !strcmp (inslRec.pick_sort, localRec.sortValue);
		 cc = find_rec (insl, &inslRec, GTEQ, "u"))
	{
		cc = abc_delete (insl);
		if (cc)
			file_err (cc, insl, "DBFIND");
	}

	return (TRUE);
}

int
spec_valid (
 int	field)
{
	if (LCHECK ("sortValue"))
	{
		if (SRCH_KEY)
		{
			SrchInsl (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (clip (temp_str)) == 0)
		{
			print_mess (ML (mlSkMess581));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (localRec.sortValue, "%-10.10s", temp_str);

		inslRec.ccmr_hash = ccmrRec.hhcc_hash;
		strcpy (inslRec.pick_sort, localRec.sortValue);
		sprintf (inslRec.location, "%10.10s", " ");
		cc = find_rec (insl, &inslRec, GTEQ, "r");
		if (cc ||
			inslRec.ccmr_hash != ccmrRec.hhcc_hash ||
			strcmp (inslRec.pick_sort, localRec.sortValue))
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		ReadDetail ();

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("locationCode"))
	{
		if (SRCH_KEY)
		{
			SrchLomr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
			return (DeleteLine ());

		if (strlen (clip (temp_str)) == 0)
		{
			print_mess (ML (mlSkMess581));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		putval (line_cnt);
		if (AlreadyExists ())
			return (EXIT_FAILURE);

		lomrRec.hhcc_hash = ccmrRec.hhcc_hash;
		sprintf (lomrRec.location, "%-10.10s", temp_str);
		cc = find_rec (lomr, &lomrRec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess209));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (localRec.locationDesc, lomrRec.desc);
		putval (line_cnt);
		DSP_FLD ("description");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchInsl (
 char *	keyVal)
{
	char prevPickSort [11];

	sprintf (prevPickSort, "%10.10s", " ");

	work_open ();
	save_rec ("#Sort Value", "#");
	inslRec.ccmr_hash = ccmrRec.hhcc_hash;
	sprintf (inslRec.pick_sort, "%-10.10s", keyVal);
	sprintf (inslRec.location, "%-10.10s", " ");
	for (cc = find_rec (insl, &inslRec, GTEQ, "r");
		 !cc &&
		  inslRec.ccmr_hash == ccmrRec.hhcc_hash &&
		  !strncmp (inslRec.pick_sort, keyVal, strlen (keyVal));
		 cc = find_rec (insl, &inslRec, NEXT, "r"))
	{
		if (!strcmp (inslRec.pick_sort, prevPickSort))
			continue;
		if (save_rec (inslRec.pick_sort, " "))
			break;
		strcpy (prevPickSort, inslRec.pick_sort);
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	inslRec.ccmr_hash = ccmrRec.hhcc_hash;
	sprintf (inslRec.pick_sort, "%-10.10s", temp_str);
	sprintf (inslRec.location, "%10.10s", " ");
	cc = find_rec (insl, &inslRec, GTEQ, "r");
	if (cc ||
		inslRec.ccmr_hash != ccmrRec.hhcc_hash ||
		strcmp (inslRec.pick_sort, temp_str))
	{
		file_err (cc, insl, "DBFIND");
	}
}

void 
SrchLomr (
 char *	keyVal)
{
	work_open ();
	save_rec ("#Location", "#Description");
	lomrRec.hhcc_hash = ccmrRec.hhcc_hash;
	sprintf (lomrRec.location, "%-10.10s", keyVal);
	for (cc = find_rec (lomr, &lomrRec, GTEQ, "r");
		 !cc &&
		  lomrRec.hhcc_hash == ccmrRec.hhcc_hash &&
		  !strncmp (lomrRec.location, keyVal, strlen (keyVal));
		 cc = find_rec (lomr, &lomrRec, NEXT, "r"))
	{
		if (save_rec (lomrRec.location, lomrRec.desc))
			break;
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	lomrRec.hhcc_hash = ccmrRec.hhcc_hash;
	sprintf (lomrRec.location, "%-10.10s", temp_str);
	cc = find_rec (lomr, &lomrRec, EQUAL, "r");
	if (cc)
		file_err (cc, lomr, "DBFIND");
}

void
ReadDetail (
 void)
{
	scn_set (DETAIL);

	lcount [DETAIL] = 0;

	inslRec.ccmr_hash = ccmrRec.hhcc_hash;
	strcpy (inslRec.pick_sort, localRec.sortValue);
	sprintf (inslRec.location, "%10.10s", " ");
	for (cc = find_rec (insl, &inslRec, GTEQ, "r");
		 !cc && !strcmp (inslRec.pick_sort, localRec.sortValue);
		 cc = find_rec (insl, &inslRec, NEXT, "r"))
	{
		lomrRec.hhcc_hash = ccmrRec.hhcc_hash;
		strcpy (lomrRec.location, inslRec.location);
		cc = find_rec (lomr, &lomrRec, EQUAL, "r");
		if (cc)
			file_err (cc, lomr, "DBFIND");
		strcpy (localRec.locationCode, lomrRec.location);
		strcpy (localRec.locationDesc, lomrRec.desc);
		putval (lcount [DETAIL]++);

		newSortValue = FALSE;
	}

	scn_set (HEADER);
}

int
DeleteLine (
 void)
{
	int i;

	if (prog_status != EDIT)
	{
		print_mess (ML (mlStdMess005));
		sleep (SLEEPTIME);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	for (i = line_cnt; i < lcount [cur_screen] - 1; i++)
	{
		getval (i + 1);
		putval (i);
	}
	lcount [cur_screen]--;
	getval (line_cnt);

	scn_write (cur_screen);
	scn_display (cur_screen);

	return (EXIT_SUCCESS);
}

int
AlreadyExists (
 void)
{
	int		i,
			numLines,
			found = FALSE;
	char	location [11];

	strcpy (location, localRec.locationCode);

	numLines = (prog_status == ENTRY) ? line_cnt : lcount [DETAIL];

	for (i = 0; i < numLines; i++)
	{
		getval (i);
		if (!strcmp (localRec.locationCode, location) && i != line_cnt)
		{
			found = TRUE;
			break;
		}
	}

	getval (line_cnt);

	if (found)
	{
		print_mess (ML (mlSkMess717));
		sleep (SLEEPTIME);
		clear_mess ();
		return (TRUE);
	}

	inslRec.ccmr_hash = ccmrRec.hhcc_hash;
	strcpy (inslRec.location, location);
	if (!find_rec (insl2, &inslRec, EQUAL, "r") &&
		strcmp (inslRec.pick_sort, localRec.sortValue))
	{
		sprintf (err_str, ML (mlSkMess718), clip (location), clip (inslRec.pick_sort));
		print_mess (err_str);
		sleep (SLEEPTIME);
		clear_mess ();
		return (TRUE);
	}

	return (FALSE);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &commRec);

	abc_alias (insl2, insl);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (insl, insl_list, INSL_NO_FIELDS, "insl_id_no1");
	open_rec (insl2, insl_list, INSL_NO_FIELDS, "insl_id_no2");
	open_rec (lomr, lomr_list, LOMR_NO_FIELDS, "lomr_id_no");

}

void
CloseDB (
 void)
{
	abc_fclose (lomr);
	abc_fclose (insl2);
	abc_fclose (insl);
	abc_fclose (ccmr);

	abc_dbclose (data);
}

int
heading (
 int	screen)
{
	tab_row = 6;
	tab_col = 13;

	if (!restart)
	{
		if (screen != cur_screen)
			scn_set (screen);

		clear ();
		rv_pr (ML (mlSkMess719), (80 - (strlen (mlSkMess719))) / 2, 0, 1);

		line_at (1, 0, 80);

		box (0, 2, 78, 1);

		switch (screen)
		{
		  case HEADER:
			scn_write (HEADER);
			scn_display (HEADER);
			scn_set (DETAIL);
			scn_write (DETAIL);
			scn_display (DETAIL);
			scn_set (HEADER);
			break;
		  case DETAIL:
			scn_set (HEADER);
			scn_write (HEADER);
			scn_display (HEADER);
			scn_set (DETAIL);
			scn_write (DETAIL);
			scn_display (DETAIL);
			break;
		}

		move (0, 21);
		line (80);

		print_at (22,0, ML (mlStdMess038), commRec.tco_no, commRec.tco_name);

		line_cnt = 0;
	}
    return (EXIT_SUCCESS);
}

void
tab_other (
 int	iline)
{
	switch (cur_screen)
	{
	  case DETAIL:
		getval (iline);
		break;
	}
}

