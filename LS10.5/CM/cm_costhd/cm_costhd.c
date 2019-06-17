/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_costhd.c,v 5.3 2002/01/10 07:06:44 scott Exp $
|  Program Name  : (cm_iss_to.c)
|  Program Desc  : (Contract Management Costhead Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (24/02/93)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: cm_costhd.c,v $
| Revision 5.3  2002/01/10 07:06:44  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_costhd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_costhd/cm_costhd.c,v 5.3 2002/01/10 07:06:44 scott Exp $";

#include	<pslscr.h>	
#include	<ml_std_mess.h>	
#include	<ml_cm_mess.h>	

#include	"schema"

struct commRecord	comm_rec;
struct cmcmRecord	cmcm_rec;
struct inumRecord	inum_rec;
struct inmrRecord	inmr_rec;

	char	*data	= "data",
			*inmr2	= "inmr2",
			*inum2	= "inum2";

   	int  	newCode = 0;

	extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy [11];
	char	uom [5];
	char	item_no [17];
	char	item_desc [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "code",	 	4, 2, CHARTYPE,
		"UUUU", "          ",
		" ", "",  "Costhead Code       ", " ",
		 NE, NO,  JUSTLEFT, "", "", cmcm_rec.ch_code},
	{1, LIN, "desc",	 	5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description         ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.desc},

	{1, LIN, "analysis1",	7, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "Analysis Code       ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.usr_ref1},
	{1, LIN, "analysis2",	8, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "                    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.usr_ref2},
	{1, LIN, "analysis3",	9, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "                    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.usr_ref3},
	{1, LIN, "analysis4",	10, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "                    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.usr_ref4},
	{1, LIN, "analysis5", 	11, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "                    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.usr_ref5},
	{1, LIN, "uom",		13, 2, CHARTYPE,
		"AAAA", "          ",
		" ", "",  "Unit Of Measure     ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.uom},
	{1, LIN, "rep_conv", 	14, 2, FLOATTYPE,
		"NNNN.NNNN", "          ",
		" ", "1", "Reporting Factor    ", " ",
		 YES, NO,  JUSTLEFT, "", "", (char *)&cmcm_rec.rep_conv},
	{1, LIN, "dtl_lvl", 	15, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Detail Level        ", " Enter A - Consolidate All, L - Consolidate Like, N - No Consolidation ",
		 YES, NO,  JUSTLEFT, "ALN", "", cmcm_rec.dtl_lvl},
	{1, LIN, "item_no",	16, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "",  "Item Number         ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "item_desc",	16, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*
 * Function prototypes.    
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
int		Update			(void);
void	SrchCmcm		(char *);
void	SrchInum		(char *);
int		heading			(int);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char * argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	OpenDB (); 

	while (prog_exit == 0)
	{
   		entry_exit 	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		search_ok 	= TRUE;
   		restart 	= FALSE;
   		newCode 	= FALSE;
		init_vars (1);
	
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			break;
		
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
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inum2, inum);
	abc_alias (inmr2, inmr);

	open_rec (cmcm,  cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cmcm);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	abc_fclose (inum2);
	SearchFindClose ();

	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	/*
	 * Valdate Tax Code.
	 */
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
			SrchCmcm (temp_str);
			return (EXIT_SUCCESS);
		}

		newCode = FALSE;
		strcpy (cmcm_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			/*
			 * Lookup unit of measure.
			 */
			inum_rec.hhum_hash	=	cmcm_rec.hhum_hash;
			cc = find_rec (inum2, &inum_rec, COMPARISON, "r");
			if (!cc)
			{
				sprintf (local_rec.uom, "%-4.4s", inum_rec.uom);
			}

			/*
			 * Lookup item number.
			 */
			inmr_rec.hhbr_hash	=	cmcm_rec.hhbr_hash;
			cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
			if (!cc)
			{
				sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);
				sprintf (local_rec.item_desc,"%-40.40s",inmr_rec.description);
			}

			scn_display (1);
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("uom"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		sprintf (inum_rec.uom, "%-4.4s", local_rec.uom);
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		abc_selfield (inmr, "inmr_id_no");

		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		/*
		 * Check for NON-STOCK ITEMS.
		 */
		if (inmr_rec.inmr_class [0] != 'N')
		{
			print_mess (ML (mlStdMess228));
			sleep (sleepTime);
			clear_mess ();	
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.item_desc, "%-40.40s", inmr_rec.description);
		DSP_FLD ("item_desc");
		
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Update files.
 */
int
Update (void)
{
	cmcm_rec.hhum_hash = inum_rec.hhum_hash;
	cmcm_rec.hhbr_hash = inmr_rec.hhbr_hash;

	if (newCode)
	{
		cc = abc_add (cmcm, &cmcm_rec);
		if (cc)
			file_err (cc, cmcm, "DBADD");
	}
	else
	{
		cc = abc_update (cmcm, &cmcm_rec);
		if (cc)
			file_err (cc, cmcm, "DBUPDATE");
	}

	return (EXIT_SUCCESS);
}

/*
 * Search for Category master file.
 */
void
SrchCmcm (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#No.", "#Costhead Description");

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", key_val);
	cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc && !strcmp (cmcm_rec.co_no, comm_rec.co_no) &&
	       		  !strncmp (cmcm_rec.ch_code, key_val, strlen (key_val)))
	{
		cc = save_rec (cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmcm, &cmcm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

void
SrchInum (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#UOM ", "#Unit Description");

	sprintf (inum_rec.uom, key_val);
	cc = find_rec (inum, &inum_rec, GTEQ, "r");
	while (!cc && !strncmp (inum_rec.uom, key_val, strlen (key_val)))
	{
	    cc = save_rec (inum_rec.uom, inum_rec.desc);
	    if (cc)
			break;
	    cc = find_rec (inum, &inum_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	sprintf (inum_rec.uom, "%-4.4s", temp_str);
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");
}

int
heading (
 int	scn)
{
	int	s_size = 80;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlCmMess157), (80 - strlen (ML (mlCmMess157))) / 2, 0, 1);

		box (0, 3, 80, 13);
		line_at (1, 0, s_size);
		line_at (6, 1, s_size - 1);
		line_at (12, 1, s_size - 1);
		line_at (20, 0, s_size);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22,0,err_str, comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;

		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
