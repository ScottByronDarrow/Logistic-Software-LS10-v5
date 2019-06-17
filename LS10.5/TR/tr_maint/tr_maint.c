/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tr_maint.c,v 5.5 2002/06/25 04:34:28 scott Exp $
|  Program Name  : (tr_maint.c)                    
|  Program Desc  : (Maintenance of trucker files)	
|---------------------------------------------------------------------|
| $Log: tr_maint.c,v $
| Revision 5.5  2002/06/25 04:34:28  scott
| General Look and clean. No changes
|
|====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tr_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_maint/tr_maint.c,v 5.5 2002/06/25 04:34:28 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_tr_mess.h>
#include 	<ml_std_mess.h>

static char	*data	  = "data",
			*DBADD    = "DBADD",
			*DBUPDATE = "DBUPDATE";

#include	"schema"

struct commRecord	comm_rec;
struct extfRecord	extf_rec;

/*
 * Local & Screen Structures. 
 */
struct {
	char	code [7];
	char	name [41];
	char	dummy [11];
} local_rec;


static	struct	var	vars[] =
{
	{1, LIN, "code",	 4, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Driver Code        ", "Enter Driver Code  [SEARCH]",
		 NE, NO,  JUSTLEFT, "", "", local_rec.code},
	{1, LIN, "name",	 6, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Driver Name        ", "",
		 YES, NO,  JUSTLEFT, "", "", local_rec.name},
	{0, LIN, "",	0, 0, INTTYPE,
		"A", "           ",
		" ", "", "dummy", " ",
		 YES, NO,  JUSTRIGHT, "", "", local_rec.dummy},

};

int		newCode = FALSE;

/*
 * Function declarations
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	SrchExtf 		(char *);
int 	heading 		(int);
int 	spec_valid 		(int);

/*
 * Main Processing Routine  
 */
int
main (
	int 	argc, 
	char 	*argv[])
{
	SETUP_SCR (vars);

	/*
	 * Set up required parameters 
	 */
	init_scr ();			
	set_tty ();         
	set_masks ();	
	init_vars (1);
	OpenDB ();
	while (prog_exit == 0)
	{
		/*
		 *   Reset control flags   
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		newCode		= FALSE;
		init_vars (1);

		/*
		 * Entry screen 1 linear input 
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit screen 1 linear input 
		 */
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
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);
	open_rec (extf, extf_list, EXTF_NO_FIELDS, "extf_id_no");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (extf);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*
	 * Validate Driver Code And Allow Search. 
	 */
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
	 		SrchExtf (temp_str);
  			return (EXIT_SUCCESS);
		}
		if (!strlen (clip (local_rec.code)))
		{
			print_mess (ML ("Blank codes are not allowed"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (extf_rec.co_no, comm_rec.co_no);
		strcpy (extf_rec.code, local_rec.code);
		cc = find_rec (extf, &extf_rec, COMPARISON, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			strcpy (local_rec.name, extf_rec.name);
			DSP_FLD ("name"); 
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Update (void)
{
	clear ();
	/*--------------------------------
	| Add or update trucker record . |
	--------------------------------*/
	strcpy (extf_rec.co_no,comm_rec.co_no);
	if (newCode == TRUE)
	{
		strcpy (extf_rec.name, local_rec.name);
		cc = abc_add (extf,&extf_rec);
		if (cc) 
			file_err (cc, extf, DBADD);
	}
	else
	{
		strcpy (extf_rec.code, local_rec.code);
		strcpy (extf_rec.name, local_rec.name);
		cc = abc_update (extf,&extf_rec);
		if (cc) 
			file_err (cc, extf, DBUPDATE);

	}
	abc_unlock (extf);
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	scn_set (scn);    
	clear ();
	/*---------------------
	| Driver Maintenance |
	---------------------*/
	sprintf (err_str, " %s ", ML ("Driver Code Maintenance."));
	rv_pr (err_str,27,0,1);

	line_at (1,0,80);
	box (0,3,80,3);

	line_at (5,1,79);
	
	line_at (20,0,80);
	print_at (21,0, ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	line_at (22,0,80);
	line_cnt = 0;
	scn_write (scn);   

    return (EXIT_SUCCESS);
}

/*
 * Search for Driver file. 
 */
void
SrchExtf (
	char *key_val)
{
	_work_open (6,0,40);
	strcpy (extf_rec.co_no,  comm_rec.co_no);
	sprintf (extf_rec.code,  "%-6.6s", key_val);
	save_rec ("#Driver", "#Driver Name");
	cc = find_rec (extf, &extf_rec, GTEQ, "r");
	while (!cc && !strcmp (extf_rec.co_no, comm_rec.co_no) &&
				  !strncmp (extf_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (extf_rec.code, extf_rec.name);
		if (cc)
			break;

		cc = find_rec (extf, &extf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (extf_rec.co_no, comm_rec.co_no);
	sprintf (extf_rec.code, "%-6.6s", temp_str);
	cc = find_rec (extf, &extf_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, extf, "DBFIND");
}
