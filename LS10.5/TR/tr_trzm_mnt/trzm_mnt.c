/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( tr_trzm_mnt.c )                                  |
|  Program Desc  : ( Transport Zone Maintenance.                  )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (10/03/1999)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: trzm_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_trzm_mnt/trzm_mnt.c,v 5.3 2002/07/25 11:17:39 scott Exp $";

#define		TABLINES	12
#define		MAXSCNS		2
#define		MAXLINES	12
#include <pslscr.h>
#include <ml_std_mess.h>
#include <minimenu.h>

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	SEL_DEFAULT	99

#define	TR_EVERY_DAY	8
#define	TR_WEEK_DAYS	9
#define	TR_WEEK_ENDS	0

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int	comm_no_fields = 5;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tes_no [3];
		char	tes_name [41];
	} comm_rec;

	/*=============================+
	 | TRansport Zone Maintenance. |
	 +=============================*/
#define	TRZM_NO_FIELDS	6

	struct dbview	trzm_list [TRZM_NO_FIELDS] =
	{
		{"trzm_co_no"},
		{"trzm_br_no"},
		{"trzm_del_zone"},
		{"trzm_desc"},
		{"trzm_dflt_chg"},
		{"trzm_trzm_hash"}
	};

	struct tag_trzmRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	del_zone [7];
		char	desc [41];
		double	dflt_chg;
		long	trzm_hash;
	}	trzm_rec;

	/*==========================================+
	 | TRansport Timeslot Capacity Maintenance. |
	 +==========================================*/
#define	TRZC_NO_FIELDS	4

	struct dbview	trzc_list [TRZC_NO_FIELDS] =
	{
		{"trzc_trzm_hash"},
		{"trzc_del_dcode"},
		{"trzc_time_slot"},
		{"trzc_capacity"}
	};

	struct tag_trzcRecord
	{
		long	trzm_hash;
		int		del_dcode;
		char	time_slot [2];
		float	capacity;
	}	trzc_rec;

	/*==================================+
	 | TRansport Zone Time Maintenance. |
	 +==================================*/
#define	TRZT_NO_FIELDS	4

	struct dbview	trzt_list [TRZT_NO_FIELDS] =
	{
		{"trzt_co_no"},
		{"trzt_time_code"},
		{"trzt_start_time"},
		{"trzt_end_time"}
	};

	struct tag_trztRecord
	{
		char	co_no [3];
		char	time_code [2];
		long	start_time;
		long	end_time;
	}	trzt_rec;

char	badFileName[5];

char	*data  = "data",
		*trzm  = "trzm",
		*trzc  = "trzc",
		*trzt  = "trzt";

#include	<tr_schedule.h>
	
MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
	  "" },
	{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
	  "" },
	{ " 3. DELETE RECORD.                     ",
	  "" },
	{ ENDMENU }
};

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	del_dcode [2];
	char	del_dcode_desc [12];
	char	timeSlotLeft [2],
			timeSlotRight [2];
	long	startTimeLeft,
			startTimeRight,
			endTimeLeft,
			endTimeRight;
	float	timeSlotCapLeft,
			timeSlotCapRight;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "deliveryZoneCode",	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Del. Zone Code    ", "Enter Delivery Zone Code [SEARCH]. ",
		 NE, NO, JUSTLEFT, "", "", trzm_rec.del_zone},
	{1, LIN, "deliveryZoneDesc",	 3, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Desc. ", " ",
		YES, NO,  JUSTLEFT, "", "", trzm_rec.desc},
	{1, LIN, "defaultDays",	 	4, 2, CHARTYPE,
		"N", "          ",
		" ", "", "Day code.             ", "1 to 7 = Mon to Sun, 8=Every Day, 9=WeekDays, 0=Weekends. [SEARCH]. ",
		 NE, NO, JUSTLEFT, "0", "9", local_rec.del_dcode},
	{1, LIN, "defaultDaysDesc", 	4, 30, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "Desc. ", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.del_dcode_desc},
	{1, LIN, "defaultCharge",	 5, 2, DOUBLETYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Del. Zone Charge ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&trzm_rec.dflt_chg},
	{2, TAB, "timeSlotLeft",	 MAXLINES, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Code", " ",
		NA, NO,  JUSTLEFT, "", "", (char *)&local_rec.timeSlotLeft},
	{2, TAB, "startTimeLeft",	 0, 2, TIMETYPE,
		"NN:NN", "          ",
		" ", "", "Start Time", " ",
		NA, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.startTimeLeft},
	{2, TAB, "endTimeLeft",	 0, 2, TIMETYPE,
		"NN:NN", "          ",
		" ", "", " End Time ", " ",
		NA, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.endTimeLeft},
	{2, TAB, "timeSlotCapacityLeft",	 0, 2, FLOATTYPE,
		"NNNN", "          ",
		" ", "", "Deliveries", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.timeSlotCapLeft},
	{2, TAB, "timeSlotRight",	 0, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Code", " ",
		NA, NO,  JUSTLEFT, "", "", (char *)&local_rec.timeSlotRight},
	{2, TAB, "startTimeRight",	 0, 2, TIMETYPE,
		"NN:NN", "          ",
		" ", "", "Start Time", " ",
		NA, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.startTimeRight},
	{2, TAB, "endTimeRight",	 0, 1, TIMETYPE,
		"NN:NN", "          ",
		" ", "", " End Time ", " ",
		NA, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.endTimeRight},
	{2, TAB, "timeSlotCapacityRight",	 0, 2, FLOATTYPE,
		"NNNN", "          ",
		" ", "", "Deliveries", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.timeSlotCapRight},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},

};

typedef int BOOL;
static BOOL	NewCode = FALSE;

extern	int	TruePosition;

/*=======================
| Function Declarations |
=======================*/
void	shutdown_prog 	(void);
void	shutdown_prog 	(void);
void	OpenDB 			(void);
void	CloseDB 		(void);
int 	spec_valid 		(int);
void 	ShowDays 		(void);
void 	SrchTrzm 		(char *);
void	Update 			(void);
void 	DeleteTimeCodes (int);
void 	UpdateTabularScreen (int);
void	LoadTabularScreen (void);
int 	heading 		(int);


/*==========================
| Main processing routine. |
==========================*/
int 
main (
 int argc,
 char * argv[])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();

	TruePosition	=	TRUE;

	tab_row	=	7;
	tab_col	=	2;
	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		lcount [2] = 0;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		scn_write (1);
		scn_display (1);
		scn_display (2);
		edit (2);
		if (prog_exit || restart)
			continue;

		edit_all();

		if (restart)
			continue;

		Update ();

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void 
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();;
}

/*========================
| Open data base files . |
========================*/
void 
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, "trzm_id_no");
	open_rec (trzt, trzt_list, TRZT_NO_FIELDS, "trzt_id_no");
	open_rec (trzc, trzc_list, TRZC_NO_FIELDS, "trzc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB (
 void)
{
	abc_fclose (trzm);
	abc_fclose (trzc);
	abc_fclose (trzt);

	abc_dbclose (data);
}

int 
spec_valid (
 int field)
{
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("deliveryZoneCode"))
	{
		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (trzm_rec.co_no, comm_rec.tco_no);
		strcpy (trzm_rec.br_no, comm_rec.tes_no);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			abc_unlock (trzm);
			NewCode = TRUE;
		}
		else
			NewCode = FALSE;

		DSP_FLD ("deliveryZoneDesc");
		if (!NewCode)
			skip_entry = goto_field(field, label("defaultDays"));

		return(0);
	}
	/*----------------
	| Validate Days. |
	----------------*/
	if (LCHECK ("defaultDays"))
	{
		int		i;

		if (SRCH_KEY)
		{
			ShowDays();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (DC [i].day_code);i++)
		{
			if (local_rec.del_dcode[0] == DC [i].day_code[0])
			{
				sprintf(local_rec.del_dcode_desc,"%-10.10s", DC [i].day_desc);
				break;
			}
		}
		DSP_FLD ("defaultDays");
		DSP_FLD ("defaultDaysDesc");
		LoadTabularScreen ();
		if (!NewCode)
			entry_exit	=	TRUE;

		return(0);
	}
	return(0);
}

/*===========================
| Search for Selling Terms. |
===========================*/
void
ShowDays (
 void)
{
	int		i;

	work_open();
	save_rec("#C","#Days Code Description.");
	for (i = 0;strlen (DC [i].day_code);i++)
	{
		cc = save_rec(DC  [i].day_code, DC [i].day_desc);
		if (cc)
			break;
	}
	cc = disp_srch();
	work_close();
	strcpy (local_rec.del_dcode, temp_str);
	
}
/*=========================
| Search for Zome Master. |
=========================*/
void
SrchTrzm (
 char *key_val)
{
	work_open ();

	save_rec("#Zone. ","#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.tco_no);
	strcpy (trzm_rec.br_no, comm_rec.tes_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", key_val);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.tco_no) &&
				  !strcmp (trzm_rec.br_no, comm_rec.tes_no) &&
				  !strncmp (trzm_rec.del_zone, key_val, strlen (key_val)))
	{
		cc = save_rec (trzm_rec.del_zone, trzm_rec.desc);
		if (cc)
			break;
		
		cc = find_rec (trzm, &trzm_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (trzm_rec.co_no, comm_rec.tco_no);
	strcpy (trzm_rec.br_no, comm_rec.tes_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trzm, "DBFIND");
}

/*==================
| Updated records. |
==================*/
void 
Update (
 void)
{
	int		i;

	if (NewCode)
	{
		cc = abc_add (trzm, &trzm_rec);
		if (cc) 
			file_err(cc, trzm, "DBADD");

		strcpy (trzm_rec.co_no, comm_rec.tco_no);
		strcpy (trzm_rec.br_no, comm_rec.tes_no);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
			file_err(cc, trzm, "DBFIND");

		scn_set (2);
	
		for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
		{
			getval (line_cnt);
			
			if (TR_EVERY_DAY	==	atoi (local_rec.del_dcode))
			{
				for (i = 1; i < 8; i++)
					UpdateTabularScreen (i);
			}
			else if (TR_WEEK_DAYS	==	atoi (local_rec.del_dcode))
			{
				for (i = 1; i < 6; i++)
					UpdateTabularScreen (i);
			}
			else if (TR_WEEK_ENDS	==	atoi (local_rec.del_dcode))
			{
				for (i = 6; i < 8; i++)
					UpdateTabularScreen (i);
			}
			else
				UpdateTabularScreen (atoi (local_rec.del_dcode));
		}
	}
	else
	{
		BOOL exitLoop = FALSE;

		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N   ", upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (trzm, &trzm_rec);
				if (cc) 
					file_err (cc, trzm, "DBUPDATE");

				scn_set (2);
			
				for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
				{
					getval (line_cnt);

					if (TR_EVERY_DAY	==	atoi (local_rec.del_dcode))
					{
						for (i = 1; i < 8; i++)
							UpdateTabularScreen (i);
					}
					else if (TR_WEEK_DAYS	==	atoi (local_rec.del_dcode))
					{
						for (i = 1; i < 6; i++)
							UpdateTabularScreen (i);
					}
					else if (TR_WEEK_ENDS	==	atoi (local_rec.del_dcode))
					{
						for (i = 6; i < 8; i++)
							UpdateTabularScreen (i);
					}
					else
						UpdateTabularScreen (atoi (local_rec.del_dcode));
				}
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (trzm);
				exitLoop = TRUE;
				break;

			case SEL_DELETE :
			{
				clear_mess ();
				if (TR_EVERY_DAY	==	atoi (local_rec.del_dcode))
				{
					for (i = 1; i < 8; i++)
						DeleteTimeCodes (i);
				}
				else if (TR_WEEK_DAYS	==	atoi (local_rec.del_dcode))
				{
					for (i = 1; i < 6; i++)
						DeleteTimeCodes (i);
				}
				else if (TR_WEEK_ENDS	==	atoi (local_rec.del_dcode))
				{
					for (i = 6; i < 8; i++)
						DeleteTimeCodes (i);
				}
				else
					DeleteTimeCodes (atoi (local_rec.del_dcode));

				trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
				trzc_rec.del_dcode	=	0;
				strcpy (trzc_rec.time_slot, " ");
				cc = find_rec (trzc, &trzc_rec, GTEQ, "r");
				if (cc || trzc_rec.trzm_hash != trzm_rec.trzm_hash)
				{
					cc = abc_delete (trzm);
					if (cc)
						file_err (cc, trzm, "DBUPDATE");
				}
				exitLoop = TRUE;
				break;
			}

			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	scn_set (1);

	abc_unlock (trzm);
}

void
DeleteTimeCodes (
 int		delCode)
{
	trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
	trzc_rec.del_dcode	=	delCode;
	strcpy (trzc_rec.time_slot, " ");
	cc = find_rec (trzc, &trzc_rec, GTEQ, "r");
	while (!cc && trzc_rec.trzm_hash == trzm_rec.trzm_hash &&
			  trzc_rec.del_dcode ==	delCode)
	{
		cc = abc_delete (trzc);
		if (cc)
			file_err (cc, trzc, "DBUPDATE");

		trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
		trzc_rec.del_dcode	=	delCode;
		strcpy (trzc_rec.time_slot, " ");
		cc = find_rec (trzc, &trzc_rec, GTEQ, "r");
	}
}
void
UpdateTabularScreen (
 int		del_code)
{
	trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
	trzc_rec.del_dcode	=	del_code;
	strcpy (trzc_rec.time_slot, local_rec.timeSlotLeft);
	cc = find_rec (trzc, &trzc_rec, COMPARISON, "r");
	if (cc)
	{
		trzc_rec.capacity	=	local_rec.timeSlotCapLeft;
		cc = abc_add (trzc, &trzc_rec);
		if (cc) 
			file_err (cc, trzc, "DBADD");
	}
	else
	{
		trzc_rec.capacity	=	local_rec.timeSlotCapLeft;
		cc = abc_update (trzc, &trzc_rec);
		if (cc) 
			file_err (cc, trzc, "DBUPDATE");
	}
	trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
	strcpy (trzc_rec.time_slot, local_rec.timeSlotRight);
	cc = find_rec (trzc, &trzc_rec, COMPARISON, "r");
	if (cc)
	{
		trzc_rec.capacity	=	local_rec.timeSlotCapRight;
		cc = abc_add (trzc, &trzc_rec);
		if (cc) 
			file_err (cc, trzc, "DBADD");
	}
	else
	{
		trzc_rec.capacity	=	local_rec.timeSlotCapRight;
		cc = abc_update (trzc, &trzc_rec);
		if (cc) 
			file_err (cc, trzc, "DBUPDATE");
	}
}
void
LoadTabularScreen (void)
{
	int		i;

	scn_set (2);
	lcount [2]	=	0;

	for (i = 0; strlen (TC [i].time_code); i++)
	{
		strcpy (trzt_rec.co_no, comm_rec.tco_no);
		strcpy (trzt_rec.time_code, TC [i].time_code);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, trzt, "DBFIND");
		
		trzc_rec.capacity	=	0.00;
		if (!NewCode)
		{
			trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
			trzc_rec.del_dcode	=	atoi (local_rec.del_dcode);
			strcpy (trzc_rec.time_slot, trzt_rec.time_code);
			cc = find_rec (trzc, &trzc_rec, COMPARISON, "r");
			if (cc)
				trzc_rec.capacity	=	0.00;
		}
		if (i % 2)
		{
			strcpy (local_rec.timeSlotRight, trzt_rec.time_code);
			local_rec.startTimeRight 	=	trzt_rec.start_time;
			local_rec.endTimeRight 	 	=	trzt_rec.end_time;
			local_rec.timeSlotCapRight	=	trzc_rec.capacity;
			putval (lcount[2]++);
		}
		else
		{
			strcpy (local_rec.timeSlotLeft, trzt_rec.time_code);
			local_rec.startTimeLeft 	=	trzt_rec.start_time;
			local_rec.endTimeLeft 	 	=	trzt_rec.end_time;
			local_rec.timeSlotCapLeft	=	trzc_rec.capacity;
		}
	}
	scn_set (1);
}
/*===========================
| edit () callback function |
===========================*/
int 
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML ("Transport Zone Maintenance"), 23, 0, 1);
		move (0, 1); line (80);

		box (0, 2, 80, 3);

		if (scn	==	1)
		{
			scn_set (2);
			scn_write (2);
			scn_display (2);
		}
		else
		{
			scn_set (1);
			scn_write (1);
			scn_display (1);
		}
		move (0, 21); line(80);
		print_at (22,0, mlStdMess038, comm_rec.tco_no, comm_rec.tco_name);
	
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
