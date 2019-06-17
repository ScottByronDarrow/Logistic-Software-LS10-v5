/*=====================================================================
|  Copyright (C) 1996 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_locminp.c   )                                 |
|  Program Desc  : ( Location Master File Maintenance.            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ccmr, lomr,                                 |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  lomr                                              |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 04/01/89         |
|---------------------------------------------------------------------|
|  Date Modified : (11/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (26/02/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (01/10/97)      | Modified  by  : Marnie Organo    |
|  Date Modified : (15/10/97)      | Modified  by  : Marnie Organo    |
|                                                                     |
|  Comments      : (11/09/90) - General Update for New Scrgen. S.B.D. |
|                : (26/02/91) - Updated to add sk_locmprt option.     |
|                : (01/10/97) - Updated for Multilingual Conversion   |
|                : (15/10/97) - Fixed bugs on company display.        | 
|                :                                                    |
|                :                                                    |
| $Log: sk_locminp.c,v $
| Revision 5.4  2002/02/26 04:18:39  scott
| S/C 00729 SKLC8 - Maintain Location Master: Maximum and Minimum Weight rounds off to the nearest .5 /.0 only when value is >999999, anything lower than that is accepted as is.  Mask for value is NNNNNNN.NN but 9999999.99 rounds off to 10000000.00.  In GUI, field accepts value with 10 digits.
|
| Revision 5.3  2001/08/28 10:12:28  robert
| additional update for LS10.5-GUI
|
| Revision 5.2  2001/08/09 09:18:55  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:12  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:16  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.3  2001/03/22 06:27:59  scott
| Updated to add app.schema - removes code related to tables from program and
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
| Revision 4.2  2001/03/15 03:17:13  scott
| Updated as last code displayed on header screen was missing last character.
|
| Revision 4.1  2001/03/15 03:15:44  scott
| Updated as prompt for input and prompt for display was on same line.
| This does not work for LS10-GUI as it thinks fields (all) are NA.
| Also make location type search look a little better.
|
| Revision 4.0  2001/03/09 02:37:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/21 10:33:33  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/11/20 07:40:09  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:24  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:04  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.21  2000/07/10 01:53:39  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.20  2000/03/03 02:14:11  marnie
| SC2584 - Modified to correct the updating of inlo details when SK_LOC_SYNC is set to 1.
|
| Revision 1.19  2000/02/14 06:09:34  scott
| Updated to correct programs that update database tables without locking the record. This does not cause a problem with the character based code but causes a problem with GVision.
|
| Revision 1.18  1999/12/06 01:30:53  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.17  1999/11/11 05:59:46  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.16  1999/11/03 07:32:06  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.15  1999/10/20 01:38:57  nz
| Updated for remainder of old routines.
|
| Revision 1.14  1999/10/13 02:42:01  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.13  1999/10/08 05:32:30  scott
| First Pass checkin by Scott.
|
| Revision 1.12  1999/06/22 02:29:34  scott
| Updated to ensure blank location could not be added.
|
| Revision 1.11  1999/06/20 05:20:11  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_locminp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_locminp/sk_locminp.c,v 5.4 2002/02/26 04:18:39 scott Exp $";

#include <pslscr.h>
#include <ml_sk_mess.h>
#include <ml_std_mess.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <LocHeader.h>
#include <minimenu.h>

#define		LO_REPORT		 (ProgramType[0] == 'P')
#define		LO_DISPLAY		 (ProgramType[0] == 'D')
#define		LO_INPUT		 (ProgramType[0] == 'I')

#define	UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	DEFAULT	99

#define	X_OFF	lp_x_off
#define	Y_OFF	lp_y_off

	char	ProgramType[2];

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inloRecord	inlo_rec;
struct lomrRecord	lomr_rec;
struct lomrRecord	lomr2_rec;

	char	*data	=	"data",
			*lomr2	=	"lomr2";

	long	currentHhccHash;

	int		printerNumber = 1;

	FILE	*fout;

	int		SK_LOC_REPLIC = FALSE;
	int		SK_LOC_SYNC = FALSE;
	int  	new_code = FALSE;	
	extern int		lp_x_off;
	extern int		lp_y_off;

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
	char	dummy[11];
	char	prev_code[11];
	char	access_desc[31];
	char	loc_desc[31];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "locn", 4, 18, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", " Location       : ", " ", 
		YES, NO, JUSTLEFT, "", "", lomr_rec.location}, 
	{1, LIN, "desc", 5, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " Description    : ", " ", 
		YES, NO, JUSTLEFT, "", "", lomr_rec.desc}, 
	{1, LIN, "comm1", 7, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " Comments #1    : ", " ", 
		YES, NO, JUSTLEFT, "", "", lomr_rec.comm1}, 
	{1, LIN, "comm2", 8, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " Comments #2    : ", " ", 
		YES, NO, JUSTLEFT, "", "", lomr_rec.comm2}, 
	{1, LIN, "comm3", 9, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " Comments #3    : ", " ", 
		YES, NO, JUSTLEFT, "", "", lomr_rec.comm3}, 
	{1, LIN, "min_wgt", 11, 18, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", " Minimum Weight : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *) &lomr_rec.min_wgt}, 
	{1, LIN, "max_wgt", 12, 18, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", " Maximum Weight : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *) &lomr_rec.max_wgt}, 
	{1, LIN, "min_vol", 13, 18, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", " Minimum Volume : ", " ", 
		ND, NO, JUSTLEFT, "", "", (char *) &lomr_rec.min_vol}, 
	{1, LIN, "max_vol", 14, 18, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", " Maximum Volume : ", " ", 
		ND, NO, JUSTLEFT, "", "", (char *) &lomr_rec.max_vol}, 
	{1, LIN, "loc_type", 14, 18, CHARTYPE, 
		"U", "          ", 
		" ", "P", " Location type  : ", "[SEARCH] available.", 
		YES, NO, JUSTLEFT, "", "", lomr_rec.loc_type}, 
	{1, LIN, "type_desc", 14, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.loc_desc},
	{1, LIN, "access", 15, 18, CHARTYPE, 
		"U", "          ", 
		" ", "1", " Access Level   : ", "Enter access level 1=Easy 5=Hard. <default = 1>", 
		YES, NO, JUSTLEFT, "12345", "", lomr_rec.access}, 
	{1, LIN, "access_desc", 15, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.access_desc},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

#include	<loc_types.h>

/*=======================
| Function Declarations |
=======================*/
int		FindLomr 			 (long, char *);
int  	HeadingPrint 		 (void);
int  	LomrDelOk 			 (void);
int  	heading 			 (int);
int  	spec_valid 			 (int);
void 	CloseAudit 			 (void);
void 	CloseDB 			 (void);
void 	DispHeading 		 (void);
void 	LotDisplay 			 (void);
void 	OpenDB 				 (void);
void 	ProcessPrinting 	 (void);
void 	Replicate 			 (void);
void 	SetAccess 			 (void);
void 	SetClass 			 (void);
void 	SrchLocationType 	 (void);
void 	SrchLomr 			 (char *);
void 	Update 				 (void);
void 	UpdateLots 			 (void);
void 	shutdown_prog 		 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	int	mult_loc = 0;
	char	*sptr;

	sptr = chk_env ("MULT_LOC");
	if (sptr != (char *)0)
		mult_loc = atoi (sptr);

	if (!mult_loc)
	{
		init_scr ();
		no_option ("MULT_LOC (Multi Bin Locations)");
		return (EXIT_FAILURE);
	}
	sptr = chk_env ("SK_LOC_REPLIC");
	if (sptr != (char *)0)
		SK_LOC_REPLIC = atoi (sptr);

	sptr = chk_env ("SK_LOC_SYNC");
	if (sptr != (char *)0)
		SK_LOC_SYNC = atoi (sptr);

	sptr = strrchr (argv[0], '/');
	if (sptr == (char *) 0)
		sptr = argv[0];
	else
		sptr++;

	if (!strcmp (sptr, "sk_locmdsp"))
		strcpy (ProgramType, "D");

	if (!strcmp (sptr, "sk_locminp"))
		strcpy (ProgramType, "I");

	if (!strcmp (sptr, "sk_locmprt"))
		strcpy (ProgramType, "P");

	/*-------------------------------
	| Print location master report. |
	-------------------------------*/
	if (LO_REPORT)
	{
		if ( argc != 2 )
		{
			print_at (0,0,mlStdMess036,argv[0]);
			return (EXIT_FAILURE);
		}

		printerNumber = atoi ( argv[1] );

		init_scr ();

		OpenDB ();

		dsp_screen (" Print Location master file details. ",
					comm_rec.co_no, comm_rec.co_name);

		ProcessPrinting ();
		
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	if (LO_DISPLAY || LO_INPUT)
	{
		SETUP_SCR (vars);
	
		init_scr ();	
		set_tty ();
		set_masks ();		
		init_vars (1);
	
		OpenDB ();
	
		if (LO_DISPLAY)
		{
			FLD ("desc")		=	NA;
			FLD ("comm1")		=	NA;
			FLD ("comm2")		=	NA;
			FLD ("comm3")		=	NA;
			FLD ("min_wgt")		=	NA;
			FLD ("max_wgt")		=	NA;
			FLD ("min_vol")		=	ND;
			FLD ("max_vol")		=	ND;
			FLD ("loc_type")	=	NA;
			FLD ("type_desc")	=	NA;
			FLD ("access")		=	NA;
			FLD ("access_desc")	=	NA;
		}
		
		strcpy (local_rec.prev_code,"          ");

		/*-----------------------------------
		| Beginning of input control loop . |
		-----------------------------------*/
		while (prog_exit == 0)
		{
			entry_exit = 0;
			edit_exit = 0;
			prog_exit = 0;
			restart = 0;
			search_ok = 1;
			init_vars (1);
	
			/*------------------------------
			| Enter screen 1 linear input. |
			------------------------------*/
			heading (1);
			entry (1);
	
			if (prog_exit || restart)
				continue;
	
			heading (1);
			scn_display (1);
			edit (1);
	
			if (restart)
				continue;

			if (LO_INPUT)
				Update ();

			if (LO_DISPLAY)
				LotDisplay ();
		}
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm ( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );

	abc_alias (lomr2, lomr);
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhwh_hash");
	open_rec (lomr,  lomr_list, LOMR_NO_FIELDS, "lomr_id_no");
	open_rec (lomr2, lomr_list, LOMR_NO_FIELDS, "lomr_id_no");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_location");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)",cc, PNAME);

	currentHhccHash	=	ccmr_rec.hhcc_hash;

	OpenLocation (ccmr_rec.hhcc_hash);
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (ccmr);
	abc_fclose (lomr2);
	CloseLocation ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*--------------------------------
	| Validate Royalty Code  Search. |
	--------------------------------*/
	if (LCHECK ("locn"))
	{
		if (SRCH_KEY)
		{
			SearchLomr (currentHhccHash, temp_str);
			return (EXIT_SUCCESS);
		}
			
		if (dflt_used || !strcmp (lomr_rec.location, "          "))
		{
			print_mess (ML ("Location cannot be blank"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cc = FindLomr (currentHhccHash, lomr_rec.location);
		if (cc)
		{
			if (LO_DISPLAY)
			{
				print_mess (ML (mlStdMess209));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			new_code = TRUE;
		}
		else
		{
			new_code	= FALSE;
			entry_exit	= TRUE;
			display_field (field + 1);
			SetClass ();
			SetAccess ();
			DSP_FLD ("type_desc");
			DSP_FLD ("access_desc");
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Minimum weight. |
	--------------------------*/
	if (LCHECK ("min_wgt"))
	{
		if (lomr_rec.min_wgt > lomr_rec.max_wgt && prog_status != ENTRY)
		{
			print_mess (ML (mlSkMess335));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Minimum weight. |
	--------------------------*/
	if (LCHECK ("max_wgt"))
	{
		if (lomr_rec.max_wgt < lomr_rec.min_wgt)
		{
			print_mess (ML (mlSkMess336));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Minimum volume. |
	--------------------------*/
	if (LCHECK ("min_vol"))
	{
		if (lomr_rec.min_vol > lomr_rec.max_vol && prog_status != ENTRY)
		{
			print_mess (ML (mlSkMess337));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Minimum volume. |
	--------------------------*/
	if (LCHECK ("max_vol"))
	{
		if (lomr_rec.max_vol < lomr_rec.min_vol)
		{
			print_mess (ML (mlSkMess338));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Storage class. |
	-------------------------*/
	if (LCHECK ("loc_type"))
	{
		int		val_loctype	=	FALSE,
				i;

		if (SRCH_KEY)
		{
			SrchLocationType ();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (loc_types[i]._loc_code);i++)
		{
			if (!strncmp (lomr_rec.loc_type,loc_types[i]._loc_code,strlen (loc_types[i]._loc_code)))
			{
				sprintf (local_rec.loc_desc,"%-30.30s",loc_types[i]._loc_desc);
				val_loctype = TRUE;
				break;
			}
		}
		if ( !val_loctype )
		{
			print_mess (ML (mlStdMess209));
			sleep ( 2 );
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ( "loc_type" );
		DSP_FLD ( "type_desc" );

		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Access code.   |
	-------------------------*/
	if (LCHECK ("access"))
	{
		SetAccess ();
		DSP_FLD ("access_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

void
SetClass (
 void)
{
	int		i;

	strcpy (local_rec.loc_desc, " ");
	for (i = 0;strlen (loc_types[i]._loc_code);i++)
	{
		if (!strncmp (lomr_rec.loc_type,loc_types[i]._loc_code,strlen (loc_types[i]._loc_code)))
		{
			sprintf (local_rec.loc_desc,"%-30.30s",loc_types[i]._loc_desc);
			break;
		}
	}
}

void
SetAccess ( 
 void)
{
	if (lomr_rec.access[0] == '1')
		strcpy (local_rec.access_desc, "1 = Access level easy.        ");
	if (lomr_rec.access[0] == '2')
		strcpy (local_rec.access_desc, "2 = Access level two.         ");
	if (lomr_rec.access[0] == '3')
		strcpy (local_rec.access_desc, "3 = Access level three.       ");
	if (lomr_rec.access[0] == '4')
		strcpy (local_rec.access_desc, "4 = Access level four.        ");
	if (lomr_rec.access[0] == '5')
		strcpy (local_rec.access_desc, "5 = Access level hard.        ");
}

void
SrchLomr (
 char *key_val)
{
	_work_open (10,0,40);
	save_rec ("#Location","#Location Description ");
	lomr_rec.hhcc_hash = currentHhccHash;
	strcpy (lomr_rec.location,key_val);
	cc = find_rec (lomr, &lomr_rec, GTEQ, "r");
	while (!cc && lomr_rec.hhcc_hash == currentHhccHash && 
		!strncmp (lomr_rec.location,key_val,strlen (key_val)))
	{
		cc = save_rec (lomr_rec.location,lomr_rec.desc);
		if (cc)
			break;
		cc = find_rec (lomr, &lomr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	lomr_rec.hhcc_hash = currentHhccHash;
	strcpy (lomr_rec.location,temp_str);
	cc = find_rec (lomr, &lomr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)lomr, "DBFIND");
}

/*===========================
| Search for Payment Terms. |
===========================*/
void
SrchLocationType (
 void)
{
	int		i = 0;

	_work_open (1,0,40);
	save_rec ("#T","#Location type description.");

	for (i = 0;strlen (loc_types[i]._loc_code);i++)
	{
		cc = save_rec (loc_types[i]._loc_code,loc_types[i]._loc_desc);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*=============================
| Add or update area record . |
=============================*/
void
Update (
 void)
{
	int		exitLoop;

	strcpy (local_rec.prev_code,lomr_rec.location);

	if (new_code)
	{
		lomr_rec.hhcc_hash = currentHhccHash;
		cc = abc_add (lomr,&lomr_rec);
		if (cc)
			file_err (cc, (char *)lomr, "DBADD");

		if (SK_LOC_REPLIC)
			Replicate ();
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case DEFAULT :
			case UPDATE :
				UpdateLots ();

				cc = abc_update (lomr,&lomr_rec);
				if (cc)
					file_err (cc, (char *)lomr, "DBUPDATE");

				if (SK_LOC_REPLIC)
					Replicate ();

				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (lomr);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (LomrDelOk ())
				{
					clear_mess ();
					cc = abc_delete (lomr);
					if (cc)
						file_err (cc, (char *)lomr, "DBUPDATE");
				}
				else
				{
					print_mess (ML ("Matching locations records. not deleted."));
					sleep (sleepTime);
					clear_mess ();
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (lomr);
}

int
LomrDelOk (
 void)
{
	strcpy (inlo_rec.location, lomr_rec.location);
	cc = find_rec (inlo, &inlo_rec, GTEQ, "r");	
	while (!cc && !strcmp (inlo_rec.location, lomr_rec.location))
	{
		incc_rec.hhwh_hash 	=	inlo_rec.hhwh_hash;
		cc = find_rec ("incc", &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");	
			continue;
		}
		if (incc_rec.hhcc_hash == currentHhccHash)
			return (FALSE);

		cc = find_rec (inlo, &inlo_rec, NEXT, "r");	
	}
	return (TRUE);
}

void
Replicate (
 void)
{
	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no,  "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no,	comm_rec.co_no))
	{
		lomr2_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		strcpy (lomr2_rec.location, lomr_rec.location);
		cc = find_rec (lomr2, &lomr2_rec, COMPARISON, "u");
		if (cc)
		{
			memcpy (&lomr2_rec, &lomr_rec, sizeof (lomr_rec));   
			lomr2_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
			
			cc = abc_add (lomr2, &lomr2_rec);
			if (cc)
				file_err (cc, lomr2, "DBADD");
		}
		else
		{
			if (SK_LOC_SYNC)
			{
				memcpy (&lomr2_rec, &lomr_rec, sizeof (lomr_rec));   
				lomr2_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
				cc = abc_update (lomr2, &lomr2_rec);
				if (cc)
					file_err (cc, lomr2, "DBUPDATE");
			}
			else
				abc_unlock (lomr2);
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
}

void
UpdateLots (
 void)
{

	abc_selfield (inlo, "inlo_location");

	strcpy (inlo_rec.location, lomr_rec.location);
	cc = find_rec (inlo, &inlo_rec, GTEQ, "u");	
	while (!cc && !strcmp (inlo_rec.location, lomr_rec.location))
	{
		if (SK_LOC_SYNC == FALSE)
		{
			incc_rec.hhwh_hash 	=	inlo_rec.hhwh_hash;
			cc = find_rec ("incc", &incc_rec, COMPARISON, "r");
			if (cc || incc_rec.hhcc_hash != currentHhccHash)
			{
				abc_unlock (inlo);
				cc = find_rec (inlo, &inlo_rec, NEXT, "r");	
				continue;
			}
		}
		if ( inlo_rec.loc_type[0] == lomr_rec.loc_type[0])
			abc_unlock (inlo);
		else
		{
			strcpy (inlo_rec.loc_type, lomr_rec.loc_type);
			SetOpID (FALSE);
			cc = abc_update (inlo, &inlo_rec);
			if 	 (cc)
				file_err (cc, (char *)inlo, "DBUPDATE");
		}
		cc = find_rec (inlo, &inlo_rec, NEXT, "u");	
	}
	abc_unlock (inlo);
}

/*=======================
| Process whole report. |
=======================*/
void
ProcessPrinting (
 void)
{
	HeadingPrint ();

	lomr_rec.hhcc_hash = currentHhccHash;
	strcpy (lomr_rec.location, "          ");

	cc = find_rec (lomr,&lomr_rec,GTEQ,"r");

	while (!cc && lomr_rec.hhcc_hash == currentHhccHash)
	{
		dsp_process (" Location : ",lomr_rec.location);

		fprintf (fout, "          | %10.10s |",lomr_rec.location);
		fprintf (fout, " %-40.40s |\n", lomr_rec.desc);

		cc = find_rec (lomr,&lomr_rec,NEXT,"r");
	}
	CloseAudit ();
}

int
HeadingPrint (
 void)
{
	if ( (fout = popen ("pformat","w")) == NULL) 
	{
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);
		return (EXIT_FAILURE);
	}
	fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".12\n");
	fprintf (fout,".L100\n");
	fprintf (fout,".ELOCATION MASTER FILE PRINTOUT.\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".Eas at %s\n",SystemTime ());
	fprintf (fout,".B1\n");
	fprintf (fout,".EBranch   : %s : %s\n",comm_rec.est_no,
					      clip (comm_rec.est_name));
	fprintf (fout,".EWarehouse: %s : %s\n",comm_rec.cc_no,
					      clip (comm_rec.cc_name));
	fprintf (fout,".B1\n");

	fprintf (fout, ".R          ==============");
	fprintf (fout, "===========================================\n");

	fprintf (fout, "          ==============");
	fprintf (fout, "===========================================\n");

	fprintf (fout, "          | LOCATION.  |");
	fprintf (fout, " L O C A T I O N    D E S C R I P T I O N |\n");

	fprintf (fout, "          |------------|");
	fprintf (fout, "------------------------------------------|\n");

	fflush (fout);
	return (EXIT_SUCCESS);
}

/*=======================================================
|	Routine to close the audit trail output file.	|
=======================================================*/
void
CloseAudit (
 void)
{
	fprintf (fout,".EOF\n");
	pclose (fout);
}

/*=====================================
| Display cheque History Information. |
=====================================*/
void
LotDisplay (
 void)
{
	char	DispStr[200];
	char	HeadStr[61];
	float	TotWgt 	=	0.00;
	lomr_rec.no_picks	=	0; 
	lomr_rec.no_hits 	=	0;

	lp_x_off = 4;
	lp_y_off = 3;

	sprintf (HeadStr, "Location %s %s", lomr_rec.location,lomr_rec.desc);

	DispHeading ();
	Dsp_prn_open (0, 7, 11, HeadStr, 
							comm_rec.co_no, comm_rec.co_name, 
							comm_rec.est_no, comm_rec.est_name, 
							 (char *) 0, (char *) 0);
		

	Dsp_saverec ("  ITEM NUMBER   |        ITEM DESCRIPTION            |UOM.|QUANTITY | WEIGHT  ");
	Dsp_saverec ("");
	Dsp_saverec ("[PRINT] [NEXT SCREEN] [PREV SCREEN] [INPUT/END]");

	strcpy (inlo_rec.location, lomr_rec.location);
	cc = find_rec (inlo, &inlo_rec, GTEQ, "r");	
	while (!cc && !strcmp (inlo_rec.location, lomr_rec.location))
	{
		incc_rec.hhwh_hash 	=	inlo_rec.hhwh_hash;
		cc = find_rec ("incc", &incc_rec, COMPARISON, "r");
		if (cc || incc_rec.hhcc_hash != currentHhccHash)
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");	
			continue;
		}
		inmr_rec.hhbr_hash = incc_rec.hhbr_hash;
		cc = find_rec ("inmr", &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");	
			continue;
		}
		if (inlo_rec.cnv_fct == 0.00)
			inlo_rec.cnv_fct = 1.00;

		sprintf (DispStr, "%16.16s^E%-36.36s^E%s^E%8.2f ^E%8.2f ",
					inmr_rec.item_no,
					inmr_rec.description,
					inlo_rec.uom,
					inlo_rec.qty / inlo_rec.cnv_fct,
					inlo_rec.qty * inmr_rec.weight);

		TotWgt 	+= inlo_rec.qty * inmr_rec.weight;

		lomr_rec.no_picks += inlo_rec.no_picks;
		lomr_rec.no_hits += inlo_rec.no_hits;
		Dsp_saverec (DispStr);
		cc = find_rec (inlo, &inlo_rec, NEXT, "r");	
	}
	if (TotWgt > lomr_rec.max_wgt)
		print_at (3,2, ML (mlSkMess358), TotWgt);
	else
		print_at (3,2, ML (mlSkMess358), TotWgt);

	print_at (4,2, ML (mlSkMess359), lomr_rec.no_picks);
	print_at (4,40, ML (mlSkMess627), lomr_rec.no_hits);


	Dsp_srch ();
	Dsp_close ();
}

void
DispHeading (
 void)
{
	clear ();
	box (0,0,80,6);
 	rv_pr (ML (mlSkMess628),17,0,1); 

	print_at (1,2, ML (mlSkMess362), lomr_rec.location, lomr_rec.desc); 
	print_at (2,2, ML (mlSkMess363), lomr_rec.min_wgt, lomr_rec.max_wgt);
	print_at (3,2, ML (mlSkMess358), 0.0);
	print_at (4,2, ML (mlSkMess359), lomr_rec.no_picks); 
	print_at (5,2, ML (mlSkMess360), local_rec.access_desc); 
	print_at (6,2, ML (mlSkMess361), local_rec.loc_desc); 

	print_at (23,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_short);
	print_at (23,27,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_short);
	print_at (23,55,ML (mlStdMess099),comm_rec.cc_no, comm_rec.cc_short); 

}

/*=====================================
| Check Location id a valid location. |
=====================================*/
int
FindLomr
 (
	long	HHCC_HASH,
	char	*Loc
)
{
	lomr_rec.hhcc_hash 	= 	HHCC_HASH;
	if (Loc == (char *)0)
	{
		strcpy (lomr_rec.location, "          ");
		return (find_rec (lomr, &lomr_rec, GTEQ , (LO_INPUT) ? "u" : "r"));
	}
	sprintf (lomr_rec.location,"%-10.10s",Loc);
	return (find_rec (lomr, &lomr_rec, COMPARISON, (LO_INPUT) ? "u" : "r"));
}
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		print_at (0,58, ML (mlSkMess084), local_rec.prev_code);

		if (LO_INPUT)
			rv_pr (ML (mlSkMess341), 23,0,1);

		if (LO_DISPLAY)
			rv_pr (ML (mlSkMess342),30,0,1);


		box (0,3,80,12);
		line_at (1,0,80);
		line_at (6,1,79);
		line_at (10,1,79);
		line_at (13,1,79);
		line_at (20,0,80);

		print_at (21,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_short);
		print_at (22,40,ML (mlStdMess099),comm_rec.cc_no, comm_rec.cc_short); 

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

