/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_qa_grp.c,v 5.3 2001/12/12 05:15:10 scott Exp $
|  Program Name  : (sk_qa_grp.c)
|  Program Desc  : (Add Maintain Qa text by buying group)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 10/10/95         |
|---------------------------------------------------------------------|
| $Log: sk_qa_grp.c,v $
| Revision 5.3  2001/12/12 05:15:10  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_qa_grp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_qa_grp/sk_qa_grp.c,v 5.3 2001/12/12 05:15:10 scott Exp $";

#define	TXT_REQD
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct qabsRecord	qabs_rec;
struct ingpRecord	ingp_rec;

	char	*LineBlank	=	"                                                            ";
	char	*data   = "data";

	int		newItem = FALSE;

struct
{
	char	dummy [11];
	char	textLine [61];
} local_rec;

extern	int	TruePosition;

static	struct	var	vars []	=	
{
	{1, LIN, "code", 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Buying Code            ", "Enter Buying Group Code, Full Serach Available ", 
		NE, NO, JUSTLEFT, "", "", ingp_rec.code}, 
	{1, LIN, "desc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Code Description       ", " ", 
		NA, NO, JUSTLEFT, "", "", ingp_rec.desc}, 
	{2, TXT,"textLine",7,8,0,"","          "," "," ",
	   " QUALITY ASSURANCE PURCHASE SPECIFICATION TEXT DETAILS    "," ",12,60,50,"","",local_rec.textLine},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
void 	LoadSpec 		(void);
void 	SrchIngp 		(char *);
int  	heading 		(int);
void 	UpdateSpec 		(void);
void 	shutdown_prog 	(void);

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

	/*
	 * Read common terminal record.
	 */
	OpenDB ();

	init_scr ();
	set_tty (); 
	set_masks ();

	prog_exit 	= FALSE;

	/*
	 * Beginning of input control loop
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags .
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		lcount [2] = 0;
		init_vars (1);
		init_vars (2);

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		scn_write (1);
		scn_display (1);
		scn_display (2);

		if (newItem)
			entry (2);
		else
			edit (2);
		
		UpdateSpec ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (qabs, qabs_list, QABS_NO_FIELDS, "qabs_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (ingp);	
	abc_fclose (qabs);	
	abc_dbclose (data);
}

int
spec_valid (
	int field)
{
	if (LCHECK ("code"))
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		strcpy (ingp_rec.type, "B");
		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess207));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (qabs_rec.co_no,comm_rec.co_no);
		strcpy (qabs_rec.buygrp, ingp_rec.code);
		qabs_rec.line_no = 0;
		cc = find_rec (qabs, &qabs_rec, GTEQ, "r");	
		if (cc || strcmp (qabs_rec.co_no,comm_rec.co_no) || 
				  strcmp (qabs_rec.buygrp,ingp_rec.code))
			newItem = TRUE;
		else
		{
			newItem = FALSE;
			LoadSpec ();
		}

		DSP_FLD ("desc");
        return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
LoadSpec (void)
{
	init_vars (2);

	lcount [2] = 0;

	strcpy (qabs_rec.co_no,comm_rec.co_no);
	strcpy (qabs_rec.buygrp, ingp_rec.code);
	qabs_rec.line_no = 0;
	cc = find_rec (qabs, &qabs_rec, GTEQ, "r");	
	while (!cc && !strcmp (qabs_rec.co_no,comm_rec.co_no) && 
				  !strcmp (qabs_rec.buygrp, ingp_rec.code))
	{
		strcpy (local_rec.textLine, qabs_rec.desc);
		putval (lcount [2]++);
		cc = find_rec (qabs, &qabs_rec, NEXT, "r");	
	}
	scn_set (1);
}

void
SrchIngp (
	char	*keyValue)
{
	_work_open (6,0,40);
	save_rec ("#Code", "#Description ");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "B");
	sprintf (ingp_rec.code, "%-6.6s", keyValue);

	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, keyValue, strlen (keyValue)))
	{
		if (ingp_rec.type [0] == 'B')
			cc = save_rec (ingp_rec.code, ingp_rec.desc);
		else
			break;

		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "B");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

int
heading (
 int scn)
{
	int	page_len;

	if (restart) 
		return (EXIT_SUCCESS);

	page_len = 80;

	clear ();
	strcpy (err_str,ML (mlSkMess081));
	rv_pr (err_str, (page_len - strlen (err_str)) / 2, 0, 1);
	
	line_at (1,0,page_len);

	box (0,3,page_len, 2);

	if (scn == 1)
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
	scn_set (scn);
	line_at (21,0,page_len);

	print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
UpdateSpec (void)
{
	int	i;

	scn_set (2);

	/*
	 * Remove blank lines at bottom
	 */
	for (i = lcount [2] - 1; i > 0;i--)
	{
		getval (i);
		if (!strcmp (local_rec.textLine, LineBlank))
			lcount [2]--;
	}

	for (i = 0;i < lcount [2];i++)
	{
		getval (i);

		strcpy (qabs_rec.co_no,comm_rec.co_no);
		strcpy (qabs_rec.buygrp, ingp_rec.code);
		qabs_rec.line_no = i;
		cc = find_rec (qabs, &qabs_rec, COMPARISON,"u");	

		/*
		 * Didn't find record so add one
		 */
		if (cc)
		{
			getval (i);
			strcpy (qabs_rec.co_no,comm_rec.co_no);
			strcpy (qabs_rec.buygrp, ingp_rec.code);
			qabs_rec.line_no = i;
			strcpy (qabs_rec.desc,local_rec.textLine);
			cc = abc_add (qabs,&qabs_rec);
			if (cc)
			       file_err (cc, qabs, "DBADD");
		}
		else
		{
			getval (i);
			strcpy (qabs_rec.desc,local_rec.textLine);
			cc = abc_update (qabs,&qabs_rec);
			if (cc)
			       file_err (cc, qabs, "DBUPDATE");
		}
		abc_unlock (qabs);
	}

	for (i = lcount [2];i < MAXLINES;i++)
	{
		strcpy (qabs_rec.co_no,comm_rec.co_no);
		strcpy (qabs_rec.buygrp, ingp_rec.code);
		qabs_rec.line_no = i;
		cc = find_rec (qabs,&qabs_rec,COMPARISON,"r");	
		if (!cc)
			abc_delete (qabs);
	}
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}
