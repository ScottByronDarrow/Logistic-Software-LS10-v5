/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: rg_rateupd.c,v 5.3 2002/07/16 02:43:03 scott Exp $
|  Program Name  : (rg_rateupd.c  )                                 |
|  Program Desc  : (Update selected resource rates.             )   |
|---------------------------------------------------------------------|
|  Author        : Mike Davy.      | Date Written : 23/04/92          |
|---------------------------------------------------------------------|
| $Log: rg_rateupd.c,v $
| Revision 5.3  2002/07/16 02:43:03  scott
| Updated from service calls and general maintenance.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: rg_rateupd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/RG/rg_rateupd/rg_rateupd.c,v 5.3 2002/07/16 02:43:03 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_rg_mess.h>
#include <ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct rgrsRecord	rgrs_rec;

	char	*data = "data";

void shutdown_prog 	(void);
int	 heading 		(int);
void OpenDB 		(void);
void CloseDB 		(void);
void Process 		(void);
void ReadRghr 		(void);
void ReadRgln 		(void);
void ReadRgrs 		(void);
void SrchRgrs 		(char *);

/*
 * Local & Screen Structures
 */
struct
{
	char	dummy [11];
	char	s_rcode [9];
	char	e_rcode [9];
	char	s_rdesc [41];
	char	e_rdesc [41];
	char	s_item [17];
	char	e_item [17];
	char	s_desc [41];
	char	e_desc [41];
} local_rec;
extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "s_item_no",	 3, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Start Item No  ", "Item no to start updating from.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.s_item},
	{1, LIN, "s_desc",	 	3, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_desc},
	{1, LIN, "e_item_no",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "End Item No    ", "Item no to stop updating at.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.e_item},
	{1, LIN, "e_desc",	 	4, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_desc},
	{1, LIN, "s_resource",	 6, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Start Resource ", "Resource code to start updating from.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.s_rcode},
	{1, LIN, "s_rdesc",	 	6, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_rdesc},
	{1, LIN, "e_resource",	 7, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "End Resource   ", "Resource code to stop updating at.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.e_rcode},
	{1, LIN, "e_rdesc",	 	7, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_rdesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};
   

/*---------------------------
| Main Processing Routine . |
---------------------------*/
int
main (
 int argc,
 char *argv [])
{
	TruePosition	=	TRUE;
	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		/*----------------------
		| Reset control flags. |
		----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_vars (1);
		search_ok 	= TRUE;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*-----------------------------
		| Edit screen 1 linear input. |
		-----------------------------*/
		heading (1);

		scn_display (1);
		edit (1);
		if (restart)
			continue;

		/*----------------------------
		| process recs for update    |
		----------------------------*/

		Process ();

	}
	
	shutdown_prog ();
	
	return EXIT_SUCCESS;


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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (rgrs, rgrs_list, RGRS_NO_FIELDS, "rgrs_id_no");
	open_rec (rgln, rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (rghr, rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
}

/*
 * Close data base files . 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (rgrs);
	abc_fclose (rgln);
	abc_fclose (rghr);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int	field)
{
	/*-----------------------------------
	| Validate Start Item Number Input. |
	-----------------------------------*/
	if (LCHECK ("s_item_no"))
	{
		if (dflt_used || !strcmp (local_rec.s_item, "                "))
		{
			sprintf (local_rec.s_item, "%-16.16s", " ");
			strcpy (local_rec.s_desc, ML ("First Item"));

			DSP_FLD ("s_item_no");
			DSP_FLD ("s_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.s_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.s_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*-----------------
			| Item not found. |
			-----------------*/
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (prog_status != ENTRY && 
				strcmp (local_rec.s_item,local_rec.e_item) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		SuperSynonymError ();

		strcpy (local_rec.s_desc,inmr_rec.description);
		DSP_FLD ("s_desc");

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Validate End Item Number Input.   |
	-----------------------------------*/
	if (LCHECK ("e_item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (dflt_used || !strcmp (local_rec.e_item, "~~~~~~~~~~~~~~~~"))
		{
			strcpy (local_rec.e_item, "~~~~~~~~~~~~~~~~");
			strcpy (local_rec.s_desc, ML ("Last Item"));

			DSP_FLD ("e_item_no");
			DSP_FLD ("e_desc");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.e_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.e_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*-----------------
			| Item not found. |
			-----------------*/
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.s_item,local_rec.e_item) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		SuperSynonymError ();

		strcpy (local_rec.e_desc,inmr_rec.description);
		DSP_FLD ("e_desc");

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Validate Start Resource Input.    |
	-----------------------------------*/
	if (LCHECK ("s_resource"))
	{
		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used || !strcmp (local_rec.s_rcode, "        "))
		{
			sprintf (local_rec.s_rcode, "%-8.8s", " ");
			strcpy (local_rec.s_rdesc, ML ("First Resource"));
			DSP_FLD ("s_resource");
			DSP_FLD ("s_rdesc");
			return (EXIT_SUCCESS);
		}

		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		strcpy (rgrs_rec.code,  local_rec.s_rcode);
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Resource not found. |
			---------------------*/
			print_mess (ML (mlRgMess011));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		else
		{
			strcpy (local_rec.s_rdesc,rgrs_rec.desc);
			DSP_FLD ("s_resource");
			DSP_FLD ("s_rdesc");
		}
		if (prog_status != ENTRY && 
				strcmp (local_rec.s_rcode,local_rec.e_rcode) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Validate End Resource Input.      |
	-----------------------------------*/
	if (LCHECK ("e_resource"))
	{
		if (dflt_used || !strcmp (local_rec.e_rcode, "~~~~~~~~"))
		{
			strcpy (local_rec.e_rcode, "~~~~~~~~");
			strcpy (local_rec.e_rdesc, ML ("Last Resource"));
			DSP_FLD ("e_resource");
			DSP_FLD ("e_rdesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		strcpy (rgrs_rec.code,  local_rec.e_rcode);
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Resource not found. |
			---------------------*/
			print_mess (ML (mlRgMess011));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		else
		{
			strcpy (local_rec.e_rdesc,rgrs_rec.desc);
			DSP_FLD ("e_resource");
			DSP_FLD ("e_rdesc");
		}
		if (strcmp (local_rec.s_rcode,local_rec.e_rcode) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}


/*
 * Process Items         
 */
void
Process (void)
{
	dsp_screen ("Items to be checked for rate update.", 
		   comm_rec.co_no, 
		   comm_rec.co_name);

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.s_item);

	abc_selfield (rgrs, "rgrs_hhrs_hash");

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
				  strcmp (inmr_rec.item_no, local_rec.e_item) <= 0)
	{
		dsp_process ("Item no : ", inmr_rec.item_no);
		ReadRghr ();
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	abc_selfield (rgrs, "rgrs_id_no");
}

/*
 * Process rghr recs     
 */
void
ReadRghr (void)
{
	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	rghr_rec.alt_no = 0;
	
	cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
	while (	!cc &&
		!strcmp (rghr_rec.co_no, comm_rec.co_no) &&
		!strcmp (rghr_rec.br_no, comm_rec.est_no) &&
		rghr_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		ReadRgln ();
		cc = find_rec (rghr, &rghr_rec, NEXT, "r");
	}
}
	
/*
 * Process rgln recs     
 */
void
ReadRgln (void)
{
	rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
	rgln_rec.seq_no = 0;
	
	cc = find_rec (rgln, &rgln_rec, GTEQ, "u");
	while (	!cc &&
		rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
	{
		ReadRgrs ();
		cc = find_rec (rgln, &rgln_rec, NEXT, "u");
	}
	abc_unlock (rgln);
}

/*
 * Process rgrs rec & update rgln rec    
 */
void
ReadRgrs (void)
{
	rgrs_rec.hhrs_hash	= rgln_rec.hhrs_hash;
	cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
	if (cc)
		file_err (cc, rgrs, "DBFIND");

	if ((strcmp (rgrs_rec.code, local_rec.s_rcode) >= 0) &&
	   (strcmp (rgrs_rec.code, local_rec.e_rcode) <= 0))
	{	
		rgln_rec.rate		= rgrs_rec.rate;
		rgln_rec.ovhd_var	= rgrs_rec.ovhd_var;
		rgln_rec.ovhd_fix	= rgrs_rec.ovhd_fix;
		cc = abc_update (rgln, &rgln_rec);
		if (cc)
			file_err (cc, rgln, "DBUPDATE");
	}
}


/*
 * Search for accs_code  
 */
void
SrchRgrs (
	char *key_val)
{
	_work_open (8,0,40);
	save_rec ("#Code", "#Description");
	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", key_val);
	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (rgrs_rec.co_no, comm_rec.co_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.est_no) &&
		!strncmp (rgrs_rec.code, key_val, strlen (key_val))
	)
	{
		cc = save_rec (rgrs_rec.code, rgrs_rec.desc);
		if (cc)
			break;
		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", temp_str);
	cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rgrs, "DBFIND");
}

int
heading (
 int scn)
{
	if (restart) 
		return 1;

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	/*
	 * Rate Maintenance 
	 */
	sprintf (err_str, " %s ", ML (mlRgMess007)); 
	rv_pr (err_str, 30,0,1);
	line_at (1,0,80);

	box (0,2,80,5);
	line_at (5,1,79);
	line_at (20,0,80);
	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	return 0;
}
