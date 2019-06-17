/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_womaint.c,v 5.8 2002/11/18 08:31:27 kaarlo Exp $
|  Program Name  : (sk_womaint.c)
|  Program Desc  : (Stock write-off Maintenance)
|---------------------------------------------------------------------|
| $Log: sk_womaint.c,v $
| Revision 5.8  2002/11/18 08:31:27  kaarlo
| S/C 4154. Revised SrchExwo ().
|
| Revision 5.7  2002/04/11 03:46:24  scott
| Updated to add comments to audit files.
|
| Revision 5.6  2002/03/06 07:43:46  scott
| S/C 00829 - BOMMT11-Maintain Stock Write-Offs box is not aligned
|
| Revision 5.5  2001/10/05 03:00:43  cha
| Added code to produce audit files.
|
| Revision 5.4  2001/08/09 09:20:27  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/09 01:53:35  scott
| RELEASE 5.0
|
| Revision 5.2  2001/08/06 23:46:11  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:43  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_womaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_womaint/sk_womaint.c,v 5.8 2002/11/18 08:31:27 kaarlo Exp $";

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <DBAudit.h>

#include	"schema"

struct commRecord	comm_rec;
struct exwoRecord	exwo_rec;

	char	*data  = "data",
			*glmr2 = "glmr2";

char	locAcc [MAXLEVEL + 1];
int		newItem	= FALSE;

extern	int	TruePosition;
		
static	struct	var vars [] =
{
	{1, LIN, "woCode",	2, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Reason code   ", "",
		 NE, NO,  JUSTLEFT, "", "", exwo_rec.code},
	{1, LIN, "woDesc",	2, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", "Description   ", "",
		YES, NO,  JUSTLEFT, "", "", exwo_rec.description},
	{1, LIN, "accountNo",	4, 2, CHARTYPE,
		GlMask, "                          ",
		" ", " ", "G/L Account   ", " ",
		YES, NO,  JUSTLEFT, "0123456789*-", "", locAcc},
	{1, LIN, "accountDesc",	4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", "",  "G/L Desc.     ", " ",
		 NA, NO,  JUSTLEFT, "", "", glmrRec.desc},

	{0, TAB, "",		0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", NULL}
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	heading 		(int);
int  	spec_valid 		(int);
void 	SrchExwo 		(char *);
void 	Update 			(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	  */
	set_tty ();
	OpenDB ();

	GL_SetMask (GlFormat);

	set_masks();
	clear ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (!prog_exit)
	{
		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		search_ok 	= TRUE;
		prog_exit 	= FALSE;
		restart 	= FALSE;

		heading (1);
		entry (1);

		if (restart)
			continue;

		if (prog_exit)
			break;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Update ();
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

void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (glmr2, "glmr");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (exwo,  exwo_list, EXWO_NO_FIELDS, "exwo_id_no");
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");

	OpenGlmr ();
	/*
	 * Open audit file.
	 */
	OpenAuditFile ("StockWriteOffReason.txt");
}

void
CloseDB (void)
{
	abc_fclose (exwo);
	abc_fclose (glmr2);
	GL_Close ();
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();

	abc_dbclose (data);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		clear ();

		if (scn != cur_screen)
			scn_set (scn);

		print_at (0, 22, ML(mlSkMess580));
		box (0, 1, 80, 3);
		line_at (3, 1, 79);

		line_at (21, 0, 80);

		print_at (22, 0, ML(mlStdMess038), comm_rec.co_no, comm_rec.co_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("woCode"))
	{
		if (SRCH_KEY)
		{
			SrchExwo (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (exwo_rec.code) < 1)
		{
			errmess (ML(mlSkMess581));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (exwo_rec.co_no, comm_rec.co_no);
		newItem = find_rec (exwo, &exwo_rec, EQUAL, "r");
		if (!newItem)
		{
			glmrRec.hhmr_hash	=	exwo_rec.hhmr_hash;
			cc = find_rec (glmr2, &glmrRec,EQUAL, "r");
			if (!cc)
			{
				sprintf (locAcc, "%-*.*s", MAXLEVEL,MAXLEVEL,glmrRec.acc_no);
				DSP_FLD ("woDesc");
				DSP_FLD ("accountNo");
				DSP_FLD ("accountDesc");
			}
			entry_exit = TRUE;
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&exwo_rec, sizeof (exwo_rec));
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("accountNo")) 
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		GL_FormAccNo (locAcc, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess024));
			sleep (sleepTime);
			clear_mess();
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [0][0] != 'F' ||
		    glmrRec.glmr_class [2][0] != 'P')
		{
			print_mess (ML(mlSkMess582));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("accountDesc");
		return (EXIT_SUCCESS);
	}
	return(0);
}

void
SrchExwo (
	char *keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Reason Description");

	strcpy (exwo_rec.co_no, comm_rec.co_no);
	sprintf (exwo_rec.code, "%-2.2s", keyValue);
	cc = find_rec (exwo, &exwo_rec, GTEQ, "r");
	while (!cc && !strcmp (exwo_rec.co_no, comm_rec.co_no) &&
		   !strncmp (exwo_rec.code, keyValue, strlen (keyValue)))
	{
			cc = save_rec (exwo_rec.code, exwo_rec.description);
			if (cc)
				break;
			cc = find_rec (exwo, &exwo_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exwo_rec.co_no, comm_rec.co_no);
	sprintf (exwo_rec.code, "%-2.2s", temp_str);
	cc = find_rec (exwo, &exwo_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exwo, "DBFIND");
}

void
Update (void)
{
	exwo_rec.hhmr_hash = glmrRec.hhmr_hash;
	if (newItem)
	{
		cc = abc_add (exwo, &exwo_rec);
		if (cc)
			file_err (cc, exwo, "DBADD");
	}
	else
	{
		/*
		 * Update changes audit record.
		 */
		cc = abc_update (exwo, &exwo_rec);
		if (cc)
			file_err (cc, exwo, "DBUPDATE");

		 sprintf (err_str, "%s : %s (%s)", ML ("Code"), exwo_rec.code, exwo_rec.description);
		 AuditFileAdd (err_str, &exwo_rec, exwo_list, EXWO_NO_FIELDS);
	}
}


