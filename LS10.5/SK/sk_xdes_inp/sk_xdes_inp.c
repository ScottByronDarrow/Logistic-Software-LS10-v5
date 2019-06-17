/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_xdes_inp.c,v 5.1 2001/12/12 02:47:09 scott Exp $
|  Program Name  : (sk_xdes_inp.c)
|  Program Desc  : (Inventory Message Maintenance)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 07/11/88         |
|---------------------------------------------------------------------|
| $Log: sk_xdes_inp.c,v $
| Revision 5.1  2001/12/12 02:47:09  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_xdes_inp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_xdes_inp/sk_xdes_inp.c,v 5.1 2001/12/12 02:47:09 scott Exp $";

#define	MAXSCNS		1
#define	INPUT		 (inputMode [0] == 'I')
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

   	int 	newCode 	= 0,
   			printerNo 	= 1;

	char	inputMode [2];

	FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct inedRecord	ined_rec;

	extern	int	TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/ 
struct {
	char	dummy [11];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "msg_code", 4, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Message Code  ", " ", 
		NE, NO, JUSTLEFT, "/ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ined_rec.code}, 
	{1, LIN, "desc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description   ", " ", 
		YES, NO, JUSTLEFT, "", "", ined_rec.desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*
 * Function Declarations
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	SrchIned 		(char *);
void 	PrintRecords	(void);
void 	HeadingOutput	(void);
int  	spec_valid 		(int);
int  	heading 		(int scn);


/*
 * Main Processing Routine.
 */
int
main (
	int 	argc, 
	char 	*argv [])
{
	TruePosition	=	TRUE;
	if (argc != 2 && argc != 3)
	{
		print_at (0, 0, mlSkMess513, argv [0]);
		return (EXIT_FAILURE);
	}

	switch (argv [1][0])
	{
	case 'I':
	case 'i':
		strcpy (inputMode, "I");
		break;

	case 'P':
	case 'p':
		printerNo = atoi (argv [2]);
		break;
	
	default :
		print_at (0, 0, mlSkMess513, argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	OpenDB ();

	if (INPUT)
	{
		/*
		 * Beginning of input control loop.
		 */
		while (prog_exit == 0)
		{
			entry_exit 	= FALSE;
			edit_exit 	= FALSE;
			prog_exit 	= FALSE;
			search_ok 	= TRUE;
			restart 	= FALSE;
			init_vars (1);

			/*
			 * Enter screen 1 linear input.
			 */
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;

			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;

			Update ();
		}
	}
	else
	{
		rset_tty ();
		PrintRecords ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ined, ined_list, INED_NO_FIELDS, "ined_id_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (ined);
	abc_dbclose ("data");
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Message code 
	 */
	if (LCHECK ("msg_code"))
	{
		if (SRCH_KEY)
		{
	 		SrchIned (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ined_rec.co_no, comm_rec.co_no);
		cc = find_rec (ined, &ined_rec, COMPARISON, "u");
		if (cc) 
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;
			DSP_FLD ("desc");
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Update (void)
{
	if (newCode)
	{
		cc = abc_add (ined, &ined_rec);
		if (cc)
			file_err (cc, ined, "DBADD");
	}
	else
	{
		cc = abc_update (ined, &ined_rec);
		if (cc)
			file_err (cc, ined, "DBUPDATE");
	}
	abc_unlock (ined);
}

/*
 * Search for Message Code
 */
void
SrchIned (
	char	*key_val)
{
	_work_open (3, 0, 40);
	save_rec ("#No.", "#Message code description");
	strcpy (ined_rec.co_no, comm_rec.co_no);
	sprintf (ined_rec.code, "%-3.3s", key_val);
	cc = find_rec (ined, &ined_rec, GTEQ, "r");
	while (!cc && !strncmp (ined_rec.code, key_val, strlen (key_val)) && 
				  !strcmp (ined_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (ined_rec.code, ined_rec.desc);
		if (cc)
			break;
		cc = find_rec (ined, &ined_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (ined_rec.co_no, comm_rec.co_no);
	sprintf (ined_rec.code, "%-3.3s", temp_str);
	cc = find_rec (ined, &ined_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ined, "DBFIND");
}

void
PrintRecords (void)
{
   	int 	firstTime = 1;

	dsp_screen ("Processing : Printing Inventory Message File ", comm_rec.co_no, comm_rec.co_name);

	strcpy (ined_rec.co_no, comm_rec.co_no);
	strcpy (ined_rec.code, "   ");
	cc = find_rec (ined, &ined_rec, GTEQ, "r");
	while (!cc && !strcmp (ined_rec.co_no, comm_rec.co_no))
	{
		if (firstTime)
		{
			HeadingOutput ();
			firstTime = 0;
		}

		dsp_process ("Processing : ", ined_rec.code);

		fprintf (fout, "|     %3.3s     ", ined_rec.code);
		fprintf (fout, "|            %40.40s            |\n", ined_rec.desc);
		fflush (fout);

		cc = find_rec (ined, &ined_rec, NEXT, "r");
	}

	if (!firstTime)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
}

void
HeadingOutput (void)
{
	if ((fout = popen ("pformat", "w")) == 0)
		file_err (errno, "pformat", "popen");

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);

	fprintf (fout, ".LP%d\n", printerNo);
	fprintf (fout, ".12\n");
	fprintf (fout, ".L80\n");
	fprintf (fout, ".PI12\n");

	fprintf (fout, ".EINVENTORY MESSAGE MASTER FILE\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECO : %s : %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT : %s\n", SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".R========================================");
	fprintf (fout, "========================================\n");
	fprintf (fout, "========================================");
	fprintf (fout, "========================================\n");
	fprintf (fout, "|    CODE     ");
	fprintf (fout, "|                           DESCRIPTION                          |\n");
	fprintf (fout, "|-------------");
	fprintf (fout, "|----------------------------------------------------------------|\n");
	fflush (fout);
}

int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlSkMess501), 22, 0, 1);

		box (0, 3, 80, 2);

		line_at (1, 0, 80);
		line_at (20, 0, 80);
		line_at (22, 0, 80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
