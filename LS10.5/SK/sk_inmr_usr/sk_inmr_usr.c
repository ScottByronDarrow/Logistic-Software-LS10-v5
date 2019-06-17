/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_inmr_usr.c,v 5.5 2002/11/21 01:54:01 keytan Exp $
|  Program Name  : (sk_ius_maint.c)
|  Program Desc  : (Maintain Inventory User Secifications)
|---------------------------------------------------------------------|
|  Date Written  : (04/06/1998)    | Author      : Scitt B Darrow     |
|---------------------------------------------------------------------|
| $Log: sk_inmr_usr.c,v $
| Revision 5.5  2002/11/21 01:54:01  keytan
| Updated to put validation when maximum User Define Code
| Environment Variable is exceeded.
|
| Revision 5.4  2002/11/19 06:56:36  cha
| Fix for Oracle 9i. Prompt Not updated when using default
|
| Revision 5.3  2002/07/25 11:17:36  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2002/07/19 04:57:09  scott
| S/C 004014
|
| Revision 5.1  2001/12/14 04:46:02  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_inmr_usr.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inmr_usr/sk_inmr_usr.c,v 5.5 2002/11/21 01:54:01 keytan Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	CODE		 (inputType [0] == 'C')
#define	DATA_INPUT	 (inputType [0] == 'I')

#define	UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	DEFAULT	99

#define	UD_CHAR		 (udih_rec.field_type == 1)
#define	UD_INT		 (udih_rec.field_type == 2)
#define	UD_FLOAT	 (udih_rec.field_type == 3)
#define	UD_DOUBLE	 (udih_rec.field_type == 4)

	/*
	 * Special fields and flags
	 */
	char	inputType [2];

	char	badFileName [5];

   	int		newCode = 0,
			nextSpec = 1;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct udihRecord	udih_rec;
struct udidRecord	udid_rec;

	char	*data = "data";

	int		MaxUserCodes	=	10;

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

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	int		ud_int;
	float	ud_float;
	double	ud_double;
	char	ud_char [41];
} local_rec;

static	struct	var	vars [] =
{
	/*
	 * Specification code.
	 */
	{2, LIN, "prompt_no",	 3, 2, INTTYPE,
		"NN", "          ",
		" ", " ", "User Defined Prompt No. ", "Enter <return> for next new specification No or [SEARCH] for existing.",
		 NE, NO, JUSTRIGHT, "", "", (char *)&udih_rec.prompt_no},
	{2, LIN, "prompt_desc",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Prompt Description      ", " ",
		YES, NO,  JUSTLEFT, "", "", udih_rec.prompt_desc},
	{2, LIN, "field_type",	 5, 2, INTTYPE,
		"N", "          ",
		" ", "1", "Field Type              ", "Enter field type. 1=Character, 2=Integer, 3=Float, 4=Double.",
		 YES, NO, JUSTRIGHT, "1", "4", (char *)&udih_rec.field_type},
	/*
	 * Specification Data Input.
	 */
	{2, LIN, "item_no", 3, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number          ", " ", 
		YES, NO, JUSTLEFT, "", "", inmr_rec.item_no}, 
	{2, LIN, "item_desc", 3, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	{2, LIN, "data_prompt_no",	 4, 2, INTTYPE,
		"NN", "          ",
		" ", "", "User Prompt No.      ", "Enter <return> for next new specification No or [SEARCH] for existing.",
		 NE, NO, JUSTRIGHT, "", "", (char *)&udih_rec.prompt_no},
	{2, LIN, "data_prompt_desc",	 4, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "", " ",
		NA, NO,  JUSTLEFT, "", "", udih_rec.prompt_desc},
	{2, LIN, "ud_int",	 5, 2, INTTYPE,
		"NNNNN", "          ",
		" ", "", "User Defined Value.  ", "Enter user defined value.",
		 YES, NO, JUSTRIGHT, "", "", (char *)&udid_rec.field_int},
	{2, LIN, "ud_float",	 5, 2, FLOATTYPE,
		"NNNNNNNN.NNNN", "          ",
		" ", "", "User Defined Value.  ", "Enter user defined value.",
		 YES, NO, JUSTRIGHT, "", "", (char *)&udid_rec.field_flt},
	{2, LIN, "ud_double",	 5, 2, DOUBLETYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "", "User Defined Value.  ", "Enter user defined value.",
		 YES, NO, JUSTRIGHT, "", "", (char *)&udid_rec.field_dbl},
	{2, LIN, "ud_char",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "User Defined Value.  ", "Enter user defined value.",
		 YES, NO, JUSTLEFT, "", "", udid_rec.field_chr},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

extern	int	TruePosition;

/*
 * Function Declarations
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	Updateudih 		(void);
void 	Updateudid 		(void);
void 	Updateiudi 		(void);
void 	SrchUdih 		(char *);
int  	heading 		(int);
int  	spec_valid 		(int);
int  	UdihDelOk 		(void);
int  	UdidDelOk 		(void);

/*
 * Main processing routine.
 */
int
main (
	int 	argc, 
	char 	*argv [])
{
	char	*sptr;
	int		i;

	TruePosition	=	TRUE;

	if (argc != 2)
	{
		print_at (0,0, "Usage: %s < C(ode) I(nput)>\007\n\r",argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Check for advertising levy/ freight charges.
	 */
	sptr = chk_env ("SK_MAX_UD_ICODE");
	MaxUserCodes = (sptr == (char *)0) ? 10 : atoi (sptr);

	/*
	 * Printer Number
	 */
	sprintf (inputType,"%-1.1s", argv [1]);

	if (!CODE && !DATA_INPUT)
	{
		print_at (0,0, "Usage: %s < C(ode) I(nput)>\007\n\r",argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Setup required parameters.
	 */
	SETUP_SCR (vars);

	if (CODE)
	{
		for (i = label ("prompt_no"); i <= label ("field_type"); i++)
			vars [i].scn = 1;
	}

	if (DATA_INPUT)
	{
		for (i = label ("item_no"); i <= label ("ud_char"); i++)
			vars [i].scn = 1;
	}

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	/*
	 * Beginning of input control loop .
	 */
	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);	

		heading (1);
		if (CODE)
		{
			char	nextSpecStr [16];

			strcpy (udih_rec.co_no,comm_rec.co_no);
			udih_rec.prompt_no = 0;
			nextSpec = 1;
			cc = find_rec (udih,&udih_rec,GTEQ,"r");
		
			while (!cc && !strcmp (udih_rec.co_no,comm_rec.co_no))
			{
				if (nextSpec <= udih_rec.prompt_no)
				{
					if (nextSpec < MaxUserCodes)
						nextSpec++;	
				}
				cc = find_rec (udih,&udih_rec,NEXT,"r");
			}

			sprintf (nextSpecStr,"%hd",nextSpec);
			vars [label ("prompt_no")].highval = p_strsave (nextSpecStr);
		}
		entry (1);
		if (prog_exit || restart)
			continue;

		/*
		 * Edit screen 1 linear input 
		 */
		heading (1);
		scn_display (1);
		edit (1);

		if (!restart)
			Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files .
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	/*
	 * Read common terminal record.
	 */
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (udih, udih_list, UDIH_NO_FIELDS, "udih_id_no");
	open_rec (udid, udid_list, UDID_NO_FIELDS, "udid_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*
 * Close data base files .
 */
void
CloseDB (
 void)
{
	abc_fclose (udih);
	abc_fclose (udid);
	abc_fclose (inmr);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*
	 * Validate Specification Type     
	 */
	if (LCHECK ("prompt_no"))
	{
		if (udih_rec.prompt_no > MaxUserCodes)
		{
			print_mess (ML ("User Prompt number is greater than system defined value."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		 
		if (SRCH_KEY)
		{
			SrchUdih (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used) 
		{
			if (nextSpec >= MaxUserCodes)
			{
				print_mess (ML("User Defined Prompt Exceeded Maximum Defined "));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			
			if (nextSpec < MaxUserCodes)
			{
				char	nextSpecStr [16];

				udih_rec.prompt_no = nextSpec++;
				newCode = 1;

				sprintf (nextSpecStr,"%hd",nextSpec);
				vars [label ("prompt_no")].highval = p_strsave (nextSpecStr);
			}
		}
		else    
		{
			strcpy (udih_rec.co_no,comm_rec.co_no);
			udih_rec.prompt_no = atoi (temp_str);
			cc = find_rec (udih, &udih_rec, COMPARISON, "w");
			if (cc)
			{
				print_mess (ML ("User Defined Prompt number not found."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			newCode = 0;
			entry_exit = 1;
		}
		
		DSP_FLD ("prompt_no");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Item Number.
	 */
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
		DSP_FLD ("item_desc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Specification Type     
	 */
	if (LCHECK ("data_prompt_no"))
	{
		if (SRCH_KEY)
		{
			SrchUdih (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (udih_rec.co_no,comm_rec.co_no);
		udih_rec.prompt_no = atoi (temp_str);
		cc = find_rec (udih, &udih_rec, COMPARISON, "w");
		if (cc)
		{
			print_mess (ML ("User Defined Prompt number not found."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		FLD ("ud_int")		=	ND;
		FLD ("ud_float")	=	ND;
		FLD ("ud_char")		=	ND;
		FLD ("ud_double")	=	ND;

		
		udid_rec.udih_hash		=	udih_rec.udih_hash;
		udid_rec.hhbr_hash		=	inmr_rec.hhbr_hash;
		cc = find_rec ("udid", &udid_rec, COMPARISON, "r");
		if (cc)
			newCode = 1;
		else
			newCode = 0;
		
		if (UD_CHAR)
			FLD ("ud_char")		=	(newCode) ? YES : NI;
		else if (UD_INT)
			FLD ("ud_int")		=	(newCode) ? YES : NI;
		else if (UD_FLOAT)
			FLD ("ud_float")	=	(newCode) ? YES : NI;
		else if (UD_DOUBLE)
			FLD ("ud_double")	=	(newCode) ? YES : NI;

		DSP_FLD ("data_prompt_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Updated records.
 */
void
Update (void)
{
	if (CODE)
		Updateudih ();

	if (DATA_INPUT)
		Updateudid ();

}

/*
 * Add or update iuds record .
 */
void
Updateudih (void)
{
	int		exitLoop;

	strcpy (udih_rec.co_no, comm_rec.co_no);
	if (newCode)
	{
		char	nextSpecStr [16];

		cc = abc_add (udih, &udih_rec);
		if (cc) 
			file_err (cc, udih, "DBADD");

		if (nextSpec < MaxUserCodes)
			nextSpec++;	

		sprintf (nextSpecStr,"%hd",nextSpec);
		vars [label ("prompt_no")].highval = p_strsave (nextSpecStr);
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
				cc = abc_update (udih, &udih_rec);
				if (cc) 
					file_err (cc, udih, "DBUPDATE");

				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (udih);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (UdihDelOk ())
				{
					clear_mess ();
					cc = abc_delete (udih);
					if (cc)
						file_err (cc, udih, "DBUPDATE");
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
	abc_unlock (udih);
}

/*
 * Add or update iudc record .
 */
void
Updateudid (void)
{
	int		exitLoop;

	if (newCode)
	{
		cc = abc_add (udid, &udid_rec);
		if (cc) 
			file_err (cc, udid, "DBADD");
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
				cc = abc_update (udid, &udid_rec);
				if (cc) 
					file_err (cc, udid, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (udid);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (UdidDelOk ())
				{
					clear_mess ();
					cc = abc_delete (udid);
					if (cc)
						file_err (cc, udid, "DBUPDATE");
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
	abc_unlock (udid);
}

/*
 * Check whether it is OK to delete the iuds record.
 */
int
UdihDelOk (void)
{
	/*
	 * Currently no checking
	 */
    return (EXIT_FAILURE);
}
/*
 * Check whether it is OK to delete the iudc record. 
 */
int
UdidDelOk (void)
{
	/*
	 * Currently no checking
	 */
    return (EXIT_FAILURE);
}

void
Updateiudi (void)
{
}

void
SrchUdih (
	char	*key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Specification Type Description");
	strcpy (udih_rec.co_no,comm_rec.co_no);
	udih_rec.prompt_no = atoi (key_val);
	cc = find_rec (udih,&udih_rec,GTEQ,"r");

	while (!cc && !strcmp (udih_rec.co_no,comm_rec.co_no))
	{
    	sprintf (err_str,"%2d",udih_rec.prompt_no);
		cc = save_rec (err_str,udih_rec.prompt_desc);
		if (cc)
			break;

		cc = find_rec (udih,&udih_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (udih_rec.co_no,comm_rec.co_no);
	udih_rec.prompt_no = atoi (temp_str);
	cc = find_rec (udih,&udih_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, udih, "DBFIND");
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	if (CODE)
	{
		rv_pr (ML (" User Defined Code Maintenance "), 20,0,1);
		line_at (1,0,80);
		box (0,2,80,3);

	}

	if (DATA_INPUT)
	{
		rv_pr (ML (" User Define field entry "), 20,0,1);
		box (0,2,80,3);
		line_at (1,0,80);
	}
	scn_set (scn);
	line_at (20,0,80);
	print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
