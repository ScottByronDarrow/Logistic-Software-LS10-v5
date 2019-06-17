/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_jnlpage.c,v 5.4 2002/07/18 06:39:31 scott Exp $
|  Program Name  : (gl_jnlpage.c)
|  Program Desc  : (General Ledger Journal Page Number Input)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: gl_jnlpage.c,v $
| Revision 5.4  2002/07/18 06:39:31  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2001/08/09 09:13:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:25  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:51  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_jnlpage.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_jnlpage/gl_jnlpage.c,v 5.4 2002/07/18 06:39:31 scott Exp $";

#define TABLINES	15
#include <pslscr.h>
#include <gl_jnlpage.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct gljcRecord	gljc_rec;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char 	co_no [3];
	char 	jrn_desc [16];
	long 	nxt_pge_no;
	char 	stat_flag [2];
} local_rec;
	
static	struct	var	vars [] =
{
	{1, TAB, "jnl_name",	MAXLINES, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Journal Description.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.jrn_desc},
	{1, TAB, "jnl_page",	 0, 3, LONGTYPE,
		"NNNNNN", "          ",
		"0", "", "Next Page No", "",
		YES, NO, JUSTRIGHT, "0", "999999", (char *)&local_rec.nxt_pge_no},
	{1, TAB, "jnl_cons",	 0, 5, CHARTYPE,
		"U", "          ",
		" ", "N", "Consolidation.", "C(onsolidate N(ormal",
		YES, NO, JUSTRIGHT, "CN", "", local_rec.stat_flag},
	{1, TAB, "rep_prog1",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", ""," ",
		ND, NO,  JUSTLEFT, "", "", gljc_rec.rep_prog1},
	{1, TAB, "rep_prog2",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", ""," ",
		ND, NO,  JUSTLEFT, "", "", gljc_rec.rep_prog2},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes.
 */
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	shutdown_prog 	 (void);
int 	spec_valid 		 (int);
void 	Update 			 (void);
void 	LoadGljc 		 (void);
int 	heading 		 (int);

/*
 * Main Processing Routine.
 */
int
main (
	int      argc,
	char	*argv [])
{
	SETUP_SCR (vars);

	tab_row 	=	3;
	tab_col 	=	10;
	/*
	 * Setup required parameters.
	 */
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();   			/*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	clear ();

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);
		lcount [1] = 0;

		LoadGljc ();
		heading (1);
		scn_display (1);
		edit (1);
		if (restart) 
			continue;

		Update ();

		/*
		 * UnLock whole file as numbers are maintained.
		 */
		/* abc_flock (gljc); */
		prog_exit = TRUE;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen ("data");
	open_rec (gljc, gljc_list, GLJC_NO_FIELDS, "gljc_id_no");
}

void
CloseDB (void)
{
	abc_fclose (gljc);
	abc_dbclose ("data");
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

int
spec_valid (
	int		field)
{
	return (EXIT_SUCCESS);
}

/*
 * Update all files.
 */
void
Update (void)
{
	char	RunProg [2][41];
	int	i;

	clear ();
	sprintf (err_str, ML (mlStdMess035));
	print_at (0,0, err_str);

	fflush (stdout);

	scn_set (1);
	for (i = 0;i < lcount [1];i++) 
	{
		getval (i);
		strcpy (RunProg [0], gljc_rec.rep_prog1);
		strcpy (RunProg [1], gljc_rec.rep_prog2);
		strcpy (gljc_rec.co_no,comm_rec.co_no);
		sprintf (gljc_rec.journ_type,"%2d",i + 1);
		cc = find_rec (gljc, &gljc_rec, EQUAL, "u");
		if (cc)
		{
			strcpy (gljc_rec.jnl_desc, local_rec.jrn_desc);
			gljc_rec.nxt_pge_no = local_rec.nxt_pge_no;
			strcpy (gljc_rec.stat_flag,local_rec.stat_flag);
			cc = abc_add (gljc,&gljc_rec);
			if (cc)
				file_err (cc, gljc, "DBADD");

			abc_unlock (gljc);
		}
		else
		{
			strcpy (gljc_rec.jnl_desc,local_rec.jrn_desc);
			gljc_rec.nxt_pge_no = local_rec.nxt_pge_no;
			strcpy (gljc_rec.stat_flag,local_rec.stat_flag);
			strcpy (gljc_rec.rep_prog1, RunProg [0]);
			strcpy (gljc_rec.rep_prog2, RunProg [1]);
			cc = abc_update (gljc,&gljc_rec);
			if (cc)
				file_err (cc, gljc, "DBUPDATE");
		}
	}
}

/*
 * Load gljc records and add if required.
 */
void
LoadGljc (void)
{
	int	i;

	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (1);
	lcount [1] = 0;

	for (i = 0 ; strlen (jrnl [i * 3]); i++)
	{
		strcpy (gljc_rec.co_no,comm_rec.co_no);
		sprintf (gljc_rec.journ_type,"%2d", i + 1);
		cc = find_rec (gljc,&gljc_rec,EQUAL,"u");
		if (cc)
		{
			gljc_rec.nxt_pge_no = 0L;
			strcpy (gljc_rec.stat_flag,"N");
		}
		sprintf (gljc_rec.jnl_desc,	"%-15.15s",	jrnl [ i * 3 ]);
		sprintf (gljc_rec.rep_prog1, "%-40.40s", jrnl [ (i * 3) + 1]);
		sprintf (gljc_rec.rep_prog2, "%-40.40s", jrnl [ (i * 3) + 2]);
		strcpy (local_rec.co_no,comm_rec.co_no);
		strcpy (local_rec.jrn_desc,gljc_rec.jnl_desc);
		local_rec.nxt_pge_no = gljc_rec.nxt_pge_no;
		strcpy (local_rec.stat_flag,gljc_rec.stat_flag);

		putval (lcount [1]++);
		if (lcount [1] > MAXLINES) 
			break;
	}
	scn_set (1);
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

		rv_pr (ML (mlGlMess021) ,16,0,1);

		line_at (1,0,80);
		line_at (20,0,80);

		sprintf (err_str, ML (mlStdMess038),
						 comm_rec.co_no,comm_rec.co_name);
		print_at (21,0, err_str);

		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
