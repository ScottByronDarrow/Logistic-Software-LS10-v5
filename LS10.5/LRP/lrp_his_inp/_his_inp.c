/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (lrp_his_inp.c)                                    |
|  Program Desc  : ( Add / Update Focus Forcasting history        )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: _his_inp.c,v $
| Revision 5.4  2002/04/16 06:42:03  scott
| dito
|
| Revision 5.3  2002/04/15 10:02:40  cha
| S/C 782. Updated to make sure that records are properly updated.
|
| Revision 5.2  2001/08/09 09:29:47  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:32  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:23  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:32  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:38  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.14  2000/07/10 01:52:28  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.13  2000/06/13 05:02:02  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.12  1999/12/06 01:34:17  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/04 05:47:47  scott
| Updated to allow a warehouse to be excluded from LRP.
|
| Revision 1.10  1999/10/27 07:32:58  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.9  1999/10/11 22:38:40  scott
| Updated for Date Routines
|
| Revision 1.8  1999/09/29 10:10:48  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:26:38  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 09:20:41  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/23 00:55:16  scott
| Updated to change future demand prompt.
|
| Revision 1.4  1999/06/15 07:27:03  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _his_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_his_inp/_his_inp.c,v 5.4 2002/04/16 06:42:03 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <twodec.h>

#define		MAX_MONTHS		36
#define		LRP_MONTH		 (local_rec.per_type [0] == 'M')
#define		LRP_DAY			 (local_rec.per_type [0] == 'D')
#define		NO_DEMAND_TYPE	6

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct ffdmRecord	ffdm_rec;
struct inmrRecord	inmr_rec;

	char 	*data	= "data";

	/*-------------------------------------------------------------------
	| Set up Array to hold Months of Year used with mon in time struct. |
	-------------------------------------------------------------------*/
	static char *mth [] =
	{
		"January   ",
		"February  ",
		"March     ",
		"April     ",
		"May       ",
		"June      ",
		"July      ",
		"August    ",
		"September ",
		"October   ",
		"November  ",
		"December  "
	};

	static	struct	date_rec 
	{
		long	StartDate;
		long	EndDate;
		float	QtySold;
		int		NoRecords;
	} store_dates [37];
		
	float	con [12];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	sup_part [17];
	char	mth_nam [MAX_MONTHS] [20];
	float	demand_value [MAX_MONTHS];
	char	dem_type [2];
	char	per_type [2];
	char	dem_desc [41];
	long	st_date;
	float	spread_value;
} local_rec;

static char *demandDesc [] =
{
	"Sales            ",
	"Future Demand    ",
	"Past Exceptions  ",
	"Transfers Out    ",
	"Lost Sales       ",
	"Production Issues"
};

static	struct	var vars [] =
{
	{1, LIN, "item_no",	 3,  2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number    ", " ",
		 NE, NO,  JUSTLEFT, "", "", inmr_rec.item_no},
	{1, LIN, "desc",		 3,  40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{1, LIN, "dem_type",	 3,  86, CHARTYPE,
		"N", "          ",
		" ", "", "Demand Type   ", "1 = Sales, 2 = Future Demand, 3 = Past Exceptions, 4 = Transfers Out, 5 = Lost Sales, 6 = Production Issues",
		 NE, NO,  JUSTLEFT, "123456", "", local_rec.dem_type},
	{1, LIN, "dem_desc",		 3,  104, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dem_desc},
	{1, LIN, "per_type",	 4,  2, CHARTYPE,
		"U", "          ",
		" ", "M", "Period Type  ", "Enter Type of Period: M (onth) or D (ay)",
		 NE, NO,  JUSTLEFT, "DM", "", local_rec.per_type},
	{1, LIN, "st_date",		 4,  40, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Start Date  ", "Enter Start Date",
		 NE, NO,  JUSTLEFT, "", "", (char *) &local_rec.st_date},
	{1, LIN, "spread",	 4,  86, FLOATTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", "Spread Value  ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.spread_value},
	{1, LIN, "value36",	 7,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [0], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [35]},
	{1, LIN, "value35",	 8,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [1], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [34]},
	{1, LIN, "value34",	 9,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [2], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [33]},
	{1, LIN, "value33",	10,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [3], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [32]},
	{1, LIN, "value32",	11,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [4], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [31]},
	{1, LIN, "value31",	12,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [5], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [30]},
	{1, LIN, "value30",	13,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [6], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [29]},
	{1, LIN, "value29",	14,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [7], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [28]},
	{1, LIN, "value28",	15,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [8], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [27]},
	{1, LIN, "value27",	16,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [9], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [26]},
	{1, LIN, "value26",	17,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [10], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [25]},
	{1, LIN, "value25",	18,  2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [11], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [24]}, 
	{1, LIN, "value24",	 7,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [12], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [23]},
	{1, LIN, "value23",	 8,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [13], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [22]},
	{1, LIN, "value22",	 9,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [14], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [21]},
	{1, LIN, "value21",	10,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [15], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [20]},
	{1, LIN, "value20",	11,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [16], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [19]},
	{1, LIN, "value19",	12,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [17], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [18]},
	{1, LIN, "value18",	13,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [18], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [17]},
	{1, LIN, "value17",	14,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [19], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [16]},
	{1, LIN, "value16",	15,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [20], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [15]},
	{1, LIN, "value15",	16,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [21], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [14]},
	{1, LIN, "value14",	17,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [22], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [13]},
	{1, LIN, "value13",	18,  45, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [23], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [12]}, 
	{1, LIN, "value12",	 7, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [24], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [11]},
	{1, LIN, "value11",	 8, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [25], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [10]},
	{1, LIN, "value10",	 9, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [26], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [9]},
	{1, LIN, "value09",	10, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [27], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [8]},
	{1, LIN, "value08",	11, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [28], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [7]},
	{1, LIN, "value07",	12, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [29], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [6]},
	{1, LIN, "value06",	13, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [30], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [5]},
	{1, LIN, "value05",	14, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [31], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [4]},
	{1, LIN, "value04",	15, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [32], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [3]},
	{1, LIN, "value03",	16, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [33], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [2]},
	{1, LIN, "value02",	17, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [34], " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [1]},
	{1, LIN, "value01",	18, 88, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", local_rec.mth_nam [35], " ",
		 YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.demand_value [0]},
	{0, LIN, "",		 0,   0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

int		HistoryFound	=	FALSE;
extern	int	TruePosition;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	LoadHistory 		(long, char *);
void 	CalcDates 			(long);
void 	LoadDailyHistory 	(void);
void 	Update 				(void);
void 	SrchDemand 			(void);
int 	heading 			(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	int		cur_mth,
			i, 
			j;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();
	set_tty ();
	swide ();

	set_masks ();
	init_vars (1);

	OpenDB ();

	ReadMisc ();

	/*-------------------------------
	| Setup month names for display	|
	-------------------------------*/
	DateToDMY (comm_rec.inv_date, NULL, &cur_mth, NULL);
	cur_mth--;
	for (j = 0; j < 3; j++)
		for (i = 0; i < 12; i++)
			sprintf (local_rec.mth_nam [i + j * 12], " %-9.9s     ", 
				mth [ (i + cur_mth + 1) % 12]);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (!restart)
			Update ();

	}	/* end of input control loop	*/
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (ffdm, ffdm_list, FFDM_NO_FIELDS,  "ffdm_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ffdm);
	abc_fclose (inmr);

	SearchFindClose ();
	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm ( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err ( cc, "ccmr", "DBFIND" );
		
	abc_fclose (ccmr);
}

int
spec_valid (
 int    field)
{
	int		tdmy [3];
	int		cur_mth;
	int		i,
			j;

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();

		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			return (EXIT_FAILURE);
		}
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			print_mess (ML ("LRP not available for this warehouse"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		return (cc);
	}

	/*-----------------------
	| Validate Demand Type. |
	-----------------------*/ 
	if (LCHECK ("dem_type"))
	{
		if (SRCH_KEY)
		{
			SrchDemand ();
			return (EXIT_SUCCESS);
		}
		strcpy (local_rec.dem_desc, demandDesc [atoi (local_rec.dem_type) - 1]);
		DSP_FLD ("dem_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Period Type. |
	-----------------------*/ 
	if (LCHECK ("per_type"))
	{
		if (LRP_MONTH)
		{
			FLD ("st_date") = NA;
			if (local_rec.dem_type [0] != '2')
			{
				local_rec.st_date =	MonthEnd (comm_rec.inv_date);
				DateToDMY (local_rec.st_date, &tdmy [0],&tdmy [1],&tdmy [2]);
				tdmy [0] = 1;
				tdmy [2] -= 3;
				local_rec.st_date = DMYToDate (tdmy [0],tdmy [1],tdmy [2]);
			}
			else
			{
				local_rec.st_date =	MonthEnd (comm_rec.inv_date) + 1;
				DateToDMY (local_rec.st_date, &tdmy [0],&tdmy [1],&tdmy [2]);
			}
			DSP_FLD ("st_date");

			/*-------------------------------
			| Setup month names for display	|
			-------------------------------*/
			cur_mth = tdmy [1] - 1;
			for (j = 0; j < 3; j++)
				for (i = 0; i < 12; i++)
					sprintf (local_rec.mth_nam [i + j * 12], " %-9.9s       ", 
						mth [ (i + cur_mth) % 12]);

			LoadHistory ( local_rec.st_date, local_rec.dem_type );
		}
		else
			FLD ("st_date") = NE;

		DSP_FLD ("st_date");

		FLD ("spread")	=	 (HistoryFound) ? NA : YES;
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Start Date   |
	-----------------------*/ 
	if (LCHECK ("st_date"))
	{
		if (dflt_used)
		{
			if (local_rec.dem_type [0] == '1' || local_rec.dem_type [0] > '3')
			{
				local_rec.st_date = comm_rec.inv_date - 
				 ( (LRP_MONTH) ? 1095 : MAX_MONTHS);
			}

			if (local_rec.dem_type [0] == '2')
				local_rec.st_date = comm_rec.inv_date + 1;
			
			if (local_rec.dem_type [0] == '3')
			{
				local_rec.st_date = comm_rec.inv_date - 
					 ( (LRP_MONTH) ? 1095 :MAX_MONTHS);
			}
		}
		if (local_rec.dem_type [0] != '2' &&
			local_rec.dem_type [0] != '3' &&
			local_rec.st_date > comm_rec.inv_date - 
				 ( (LRP_MONTH) ? 1095 :MAX_MONTHS))
		{
			local_rec.st_date = comm_rec.inv_date - 
				 ( (LRP_MONTH) ? 1095 :MAX_MONTHS);
		}

		/*-------------------------------
		| Setup month names for display	|
		-------------------------------*/
		DateToDMY (local_rec.st_date, NULL, &cur_mth, NULL);
		cur_mth--;
		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 12; i++)
			{
				sprintf (local_rec.mth_nam [i + j * 12], " %-9.9s      ", 
					mth [ (i + cur_mth) % 12]);
			}
		}
		if (LRP_DAY)
		{
			for (i = 0; i < MAX_MONTHS ; i++)
				sprintf (local_rec.mth_nam [i], "%-10.10s     ", 
					DateToString (local_rec.st_date + i));

			LoadDailyHistory ();
		}
		FLD ("spread")	=	 (HistoryFound) ? NA : YES;
		entry_exit = TRUE;
	}
	/*------------------------
	| Validate spread value. |
	------------------------*/ 
	if (LCHECK ("spread"))
	{
		float	PeriodAmount	=	0.00;

		if (local_rec.spread_value > 0.00)
		{
			PeriodAmount	=	local_rec.spread_value / MAX_MONTHS;
			for (i = 0; i < MAX_MONTHS; i++)
			{
	    		local_rec.demand_value [i]	= PeriodAmount;
				sprintf (err_str, "value%02d", i + 1);
				DSP_FLD (err_str);
			}
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================================
| Read the ffdm record for the appropriate year	|
| if record doesn't exist and year is valid		|
| then get data from incc record				|
===============================================*/

void
LoadHistory (
 long   StartDate,
 char*  type)
{
	int		i;
	char	wk_date [4] [11];

	HistoryFound	=	FALSE;

	CalcDates ( StartDate );

	for (i = 0; i < MAX_MONTHS; i++)
	    local_rec.demand_value [i]		= 0.00;


	ffdm_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	ffdm_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
	sprintf (ffdm_rec.type, "%1.1s", type);
	ffdm_rec.date	=	store_dates [0].StartDate;
	cc = find_rec (ffdm, &ffdm_rec, GTEQ, "r");

	while (!cc && ffdm_rec.hhbr_hash	==	inmr_rec.hhbr_hash &&
				  ffdm_rec.hhcc_hash	==	ccmr_rec.hhcc_hash &&
				  ffdm_rec.type [0] 		==  type [0] &&
				  ffdm_rec.date 		<= store_dates [MAX_MONTHS - 1].EndDate)
	{
		for (i = 0; i < MAX_MONTHS; i++)
		{
			strcpy (wk_date [0], DateToString (ffdm_rec.date));
			strcpy (wk_date [1], DateToString (store_dates [i].StartDate));
			strcpy (wk_date [2], DateToString (store_dates [i].EndDate));
			if (ffdm_rec.date >= store_dates [i].StartDate &&
			    ffdm_rec.date <= store_dates [i].EndDate)
			{
				HistoryFound	=	TRUE;
				store_dates [i].QtySold	+= twodec (ffdm_rec.qty);
				store_dates [i].NoRecords++;
			}
		}
		cc = find_rec (ffdm, &ffdm_rec, NEXT, "r");
	}
	for (i = 0; i < MAX_MONTHS; i++)
		local_rec.demand_value [ (MAX_MONTHS -1) - i] = store_dates [i].QtySold;
}

/*===============================================
| Calculate start and end dates for each month. |
===============================================*/
void
CalcDates (
 long StartDate)
{
	int		i;

	for (i = 0; i < MAX_MONTHS; i++)
	{
		store_dates [i].StartDate	=	MonthStart (StartDate);
		store_dates [i].EndDate		=	MonthEnd (StartDate);
		store_dates [i].QtySold		=	0.00;
		store_dates [i].NoRecords	=	0;
		StartDate	=	MonthEnd (StartDate) + 1;
	}
}

/*=====================================================================
| Load days information from demand file ffdm.
=====================================================================*/
void
LoadDailyHistory (void)
{
	int	i;

	HistoryFound	=	FALSE;

	for (i = 0; i < MAX_MONTHS ; i++)
	{
		ffdm_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		ffdm_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		strcpy (ffdm_rec.type,	local_rec.dem_type);
		ffdm_rec.date		=	local_rec.st_date + i;
		cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "r");
		if (cc)
			local_rec.demand_value [ (MAX_MONTHS - 1) - i]	=	0.00;
		else
		{
			HistoryFound	=	TRUE;
			local_rec.demand_value [ (MAX_MONTHS - 1) - i]	=	ffdm_rec.qty;
		}
	}
}

void
Update (void)
{
	int	i;
	float	CalcQty	=	0.00;

	if (LRP_DAY)
	{
		for (i = 0; i < MAX_MONTHS ; i++)
		{
			ffdm_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			ffdm_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
			strcpy (ffdm_rec.type,	local_rec.dem_type);
			ffdm_rec.date		=	local_rec.st_date + i;
			cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "u");
			if (cc)
			{
				ffdm_rec.qty = local_rec.demand_value [ (MAX_MONTHS - 1) -i];
				cc = abc_add (ffdm, &ffdm_rec);
				if (cc)
					file_err (cc, ffdm, "DBADD");
			}
			else
			{
				ffdm_rec.qty = local_rec.demand_value [ (MAX_MONTHS - 1) -i];
				cc = abc_update (ffdm, &ffdm_rec);
				if (cc)
					file_err (cc, ffdm, "DBUPDATE");
			}
		}
	}
	else
	{
		for (i = 0; i < MAX_MONTHS ; i++)
		{
			CalcQty	=	local_rec.demand_value [ (MAX_MONTHS -1) - i] -
						store_dates [i].QtySold;

			ffdm_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			ffdm_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
			strcpy (ffdm_rec.type,	local_rec.dem_type);
			ffdm_rec.date		=	store_dates [i].StartDate;
			cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "u");
			if (cc)
			{
				ffdm_rec.qty = CalcQty;
				if (ffdm_rec.qty != 0.00)
				{
					cc = abc_add (ffdm, &ffdm_rec);
					if (cc)
						file_err (cc, ffdm, "DBADD");
				}
				else
					abc_unlock (ffdm);
			}
			else
			{
				ffdm_rec.qty += CalcQty;
				ffdm_rec.date		=	store_dates [i].StartDate;
				cc = abc_update (ffdm, &ffdm_rec);
				if (cc)
					file_err (cc, ffdm, "DBUPDATE");
			}
		}
	}
}

/*===========================
| Search for Payment Terms. |
===========================*/
void
SrchDemand (void)
{
	int		i;
	_work_open (1,0,30);
	save_rec ("#D","#Demand Type");

	for (i = 0;i < NO_DEMAND_TYPE; i++)
	{
		sprintf (err_str, "%d", i + 1);
		save_rec (err_str, demandDesc [i]);
	}
	cc = disp_srch ();
	work_close ();
	strcpy (local_rec.dem_type, temp_str); 
}

int
heading (
 int    scn)
{
	char	cur_str [13];
	char	ls1_str [13];
	char	ls2_str [13];
	int		cur_mth,
			cur_yer;

	if (local_rec.st_date)
		DateToDMY (local_rec.st_date, NULL,&cur_mth,&cur_yer);
	else
		DateToDMY (comm_rec.inv_date,NULL,&cur_mth,&cur_yer);

	cur_yer += 3;

	if (cur_mth != 1)
	{
		sprintf (cur_str, "%d/%d", cur_yer - 1 , cur_yer);
		sprintf (ls1_str, "%d/%d", cur_yer - 2 , cur_yer - 1 );
		sprintf (ls2_str, "%d/%d", cur_yer - 3 , cur_yer - 2 );
	}
	else
	{
		sprintf (cur_str, "   %d  ", cur_yer - 1);
		sprintf (ls1_str, "   %d  ", cur_yer - 2);
		sprintf (ls2_str, "   %d  ", cur_yer - 3);
	}

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		
		rv_pr (ML (" LRP History/Future Demand Maintenance "), 50, 0, 1);
		line_at (1,0,130);
	
		box (0, 2, 130, 16);
		line_at (5,1,129);
		PGCHAR (11);
		box (43, 5, 44, 13);
		move (43, 5);
		PGCHAR (8);
		move (86, 5);
		PGCHAR (8);
		move (43, 19);
		PGCHAR (9);
		move (86, 19);
		PGCHAR (9);
	
		if (LRP_MONTH)
		{
			rv_pr (cur_str, 105, 5, 1);
			rv_pr (ls1_str, 63, 5, 1);
			rv_pr (ls2_str, 20, 5, 1);
		}
		line_at (20,0,130);
		line_at (22,0,130);

		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
