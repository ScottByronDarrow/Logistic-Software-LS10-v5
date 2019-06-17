/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: inir_inp.c,v 5.5 2002/04/11 03:46:22 scott Exp $
|  Program Name  : (sk_inir_inp.c)
|  Program Desc  : (Stock Misc issue / receipt maintenance)
|---------------------------------------------------------------------|
| $Log: inir_inp.c,v $
| Revision 5.5  2002/04/11 03:46:22  scott
| Updated to add comments to audit files.
|
| Revision 5.4  2001/10/05 03:00:34  cha
| Added code to produce audit files.
|
| Revision 5.3  2001/08/09 09:18:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:00  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:07  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: inir_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inir_inp/inir_inp.c,v 5.5 2002/04/11 03:46:22 scott Exp $";

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <DBAudit.h>

#include	"schema"

struct commRecord	comm_rec;
struct inirRecord	inir_rec;

	char	*data  = "data",
			*glmr2 = "glmr2";

	char	localAccount [MAXLEVEL + 1];
	int		newItem	= FALSE;
		
static	struct	var vars [] =
{
	{1, LIN, "irType",	 2, 15, CHARTYPE,
		"UU", "          ",
		" ", " ", "Issue Type      ", "Enter Type [SEARCH]",
		 NO, NO, JUSTLEFT, "", "", inir_rec.ir_type},
	{1, LIN, "irDesc",	 3, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Type Desc.", " ",
		 YES, NO, JUSTLEFT, "", "", inir_rec.ir_desc},
	{1, LIN, "accountNo",	5, 15, CHARTYPE,
		GlMask, "                          ",
		" ", " ", "G/L Account", " ",
		YES, NO,  JUSTLEFT, "0123456789*-", "", localAccount},
	{1, LIN, "accountDesc",	5, 51, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", "", "   G/L Desc.", " ",
		 NA, NO,  JUSTLEFT, "", "", glmrRec.desc},
	{0, TAB, "",		0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", NULL}
};

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog 		(void);
void OpenDB 			(void);
void CloseDB 			(void);
void SrchInir 			(char *);
void Update 			(void);
int  heading 			(int);
int  spec_valid 		(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc, 
	char 	*argv [])
{
	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	  */
	set_tty ();
	OpenDB ();

	GL_SetMask (GlFormat);
	set_masks ();
	clear ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (!prog_exit)
	{
		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit = 0;
		edit_exit = 0;
		search_ok = TRUE;
		prog_exit = 0;
		restart = 0;

		heading (1);
		scn_display (1);
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

	open_rec (inir,  inir_list, INIR_NO_FIELDS, "inir_id_no");
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
	OpenGlmr ();
	/*
	 * Open audit file.
	 */
	OpenAuditFile ("IssueReceiptTypes.txt");
}

void
CloseDB (
 void)
{
	abc_fclose (inir);
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

		print_at (0, 22, "%R %s", ML (mlSkMess714));
		box (0, 1, 80, 4);
		line_at (4, 1, 79);

		line_at (21, 0, 80);

		/*" Company no. : %s   %s",  */

		print_at (22, 0, ML (mlStdMess038),
			comm_rec.co_no,
			comm_rec.co_name);

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
	/*----------------------------
	| Validate issue_rec type    |
	----------------------------*/
	if (LCHECK ("irType"))
	{
		if (SRCH_KEY)
		{
			SrchInir (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (inir_rec.co_no,comm_rec.co_no);
		cc = find_rec (inir,&inir_rec,EQUAL,"r");
		newItem = cc;
		if (!cc)
		{
			cc = find_hash (glmr2, &glmrRec,EQUAL, "r", inir_rec.hhmr_hash);
			if (!cc)
			{
				sprintf (localAccount, "%-*.*s", MAXLEVEL,MAXLEVEL,glmrRec.acc_no);
				DSP_FLD ("irDesc");
				DSP_FLD ("accountNo");
				DSP_FLD ("accountDesc");
			}
			entry_exit = TRUE;
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&inir_rec, sizeof (inir_rec));
		}
		DSP_FLD ("irDesc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("accountNo")) 
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		GL_FormAccNo (localAccount, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [0] [0] != 'F' ||
		    glmrRec.glmr_class [2] [0] != 'P')
		{
			print_mess (ML (mlSkMess582));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("accountDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*==================
| Search for inir. |
==================*/
void
SrchInir (
	char	*keyedValue)
{
	_work_open (2,0,40);
	save_rec ("#No","#Issue/Receipt Description.");
	strcpy (inir_rec.co_no, comm_rec.co_no);
	sprintf (inir_rec.ir_type,"%-2.2s",keyedValue);
	cc = find_rec (inir,&inir_rec,GTEQ,"r");
	while (!cc && !strcmp (inir_rec.co_no, comm_rec.co_no) &&
		      !strncmp (inir_rec.ir_type,keyedValue,strlen (keyedValue)))
	{
		cc = save_rec (inir_rec.ir_type,inir_rec.ir_desc);
		if (cc)
			break;

		cc = find_rec (inir,&inir_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inir_rec.co_no, comm_rec.co_no);
	sprintf (inir_rec.ir_type,"%-2.2s", temp_str);
	cc = find_rec (inir,&inir_rec,EQUAL,"r");
	if (cc)
		file_err (cc, inir, "DBFIND");
}

void
Update (void)
{
	inir_rec.hhmr_hash = glmrRec.hhmr_hash;
	if (newItem)
	{
		cc = abc_add (inir, &inir_rec);
		if (cc)
			file_err (cc, inir, "DBADD");
	}
	else
	{
		/*
		 * Update changes audit record.
		 */
		 sprintf (err_str, "%s : %s (%s)", ML ("Type"), inir_rec.ir_type, inir_rec.ir_desc);
		 AuditFileAdd (err_str, &inir_rec, inir_list, INIR_NO_FIELDS);
		cc = abc_update (inir, &inir_rec);
		if (cc)
			file_err (cc, inir, "DBUPDATE");
	}
}


