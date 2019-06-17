/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: rg_copy.c,v 5.4 2002/03/06 07:48:04 scott Exp $
|  Program Name  : (rg_copy.c) 
|  Program Desc  : (Routing Copy) 
|---------------------------------------------------------------------|
|  Date Written  : (22/01/92)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
| $Log: rg_copy.c,v $
| Revision 5.4  2002/03/06 07:48:04  scott
| S/C 00831 - RTGMR4-Product Routing Copy; WINDOWS CLIENT (1) Source Item Number and Destination Item Number  display error without delay CHAR-BASED / WINDOWS CLIENT (2) When selected an item with invalid Source Alternate Number, press F1 to cancel, then select a new item, the Source Description will display the previous item description and the Source Alternate Number is not found even if it exists for the newly selected item.  F1 does not work, will need to exit from the program to clear fields.
|
| Revision 5.3  2001/11/08 00:59:29  scott
| Updated to use app.schema
| Updated to clean code and remove some VERY non standard database stuff.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: rg_copy.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/RG/rg_copy/rg_copy.c,v 5.4 2002/03/06 07:48:04 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_rg_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct rghrRecord	rghr_rec;
struct rghrRecord	rghr2_rec;
struct rglnRecord	rgln_rec;
struct rglnRecord	rgln2_rec;
struct ineiRecord	inei_rec;

	char	*data	= "data",
			*rghr2	= "rghr2",
			*rgln2	= "rgln2";

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	src_item [17];
	char	src_desc [41];
	long	src_hhbr;
	int		src_alt;
	char	dst_item [17];
	char	dst_desc [41];
	long	dst_hhbr;
	int		dst_alt;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "src_item",	 2, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Source Item Number: ", "",
		 NE, NO,  JUSTLEFT, "", "", local_rec.src_item},
	{1, LIN, "src_alt",	 2, 65, INTTYPE,
		"NNNNN", "          ",
		" ", "0", "Source Alternate #: ", "",
		 NO, NO, JUSTRIGHT, "0", "32767", (char *) &local_rec.src_alt},
	{1, LIN, "src_desc",	 3, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Source Description: ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.src_desc},
	{1, LIN, "src_hhbr",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.src_hhbr},
	{1, LIN, "dst_item",	 5, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Destn. Item Number: ", "",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dst_item},
	{1, LIN, "dst_alt",	 5, 65, INTTYPE,
		"NNNNN", "          ",
		" ", "0", "Destn. Alternate #: ", "",
		 NO, NO, JUSTRIGHT, "0", "32767", (char *) &local_rec.dst_alt},
	{1, LIN, "dst_desc",	 6, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Destn. Description: ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dst_desc},
	{1, LIN, "dst_hhbr",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.dst_hhbr},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		"", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*===========================
| Local function prototypes |
===========================*/
int		heading			 (int);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
void	update			 (void);
void	AlternateSrch	 (long);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		search_ok	= TRUE;
		init_vars (1);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit)
		{
			shutdown_prog ();
			return (EXIT_SUCCESS);
		}

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		update ();
	}	/* end of input control loop	*/
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
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
	abc_dbopen (data);

	abc_alias (rghr2, rghr);
	abc_alias (rgln2, rgln);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (rghr, rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rghr2,rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln, rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (rgln2,rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (rghr);
	abc_fclose (rghr2);
	abc_fclose (rgln);
	abc_fclose (rgln2);
	abc_fclose (inei);
	SearchFindClose ();
	abc_dbclose (data);
}

int
heading (
	int	scn)
{
	swide ();
	if (!restart)
	{
		scn_set (scn);
		clear ();
		rv_pr (ML (mlRgMess001), 54, 0, 1);

		box (0, 1, 132, 5);
		line_at (4,1,131);
		line_at (20,0,132);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22, 0, err_str, comm_rec.est_no, comm_rec.est_name);

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
	if (LCHECK ("src_item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.src_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.src_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		local_rec.src_hhbr = inmr_rec.hhbr_hash;
		strcpy (local_rec.src_desc, inmr_rec.description);
		DSP_FLD ("src_item");
		DSP_FLD ("src_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("src_alt"))
	{
		if (dflt_used)
		{
			/* read branch record for default rtg no */
			strcpy (inei_rec.est_no, comm_rec.est_no);
			inei_rec.hhbr_hash = local_rec.src_hhbr;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			if (cc || inei_rec.dflt_rtg <= 0)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				inmr_rec.hhbr_hash	=	local_rec.src_hhbr;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, inmr, "DBFIND");

				if (inmr_rec.dflt_rtg <= 0)
				{
					print_mess (ML (mlStdMess007));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				local_rec.src_alt = inmr_rec.dflt_rtg; 
				DSP_FLD ("src_alt");
				abc_selfield (inmr, "inmr_id_no");
			}
			else
			{
				local_rec.src_alt = inei_rec.dflt_rtg;
				DSP_FLD ("src_alt");
			}
		}

		if (SRCH_KEY)
		{
			AlternateSrch (local_rec.src_hhbr);
			return (EXIT_SUCCESS);
		}
		strcpy (rghr_rec.co_no, comm_rec.co_no);
		strcpy (rghr_rec.br_no, comm_rec.est_no);
		rghr_rec.hhbr_hash = local_rec.src_hhbr;
		rghr_rec.alt_no = local_rec.src_alt;
		cc = find_rec (rghr, &rghr_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (err_str,
					 ML (mlStdMess002),
					 local_rec.src_alt,
					 local_rec.src_item);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dst_item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.dst_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.dst_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		local_rec.dst_hhbr = inmr_rec.hhbr_hash;
		strcpy (local_rec.dst_desc, inmr_rec.description);
		DSP_FLD ("dst_item");
		DSP_FLD ("dst_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dst_alt"))
	{
		if (dflt_used)
		{
			/* read branch record for default rtg no */
			strcpy (inei_rec.est_no, comm_rec.est_no);
			inei_rec.hhbr_hash = local_rec.dst_hhbr;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			/* if branch default is 0, read company default rtg no */
			if (cc || inei_rec.dflt_rtg <= 0)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				inmr_rec.hhbr_hash	=	local_rec.dst_hhbr;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, inmr, "DBFIND");

				if (inmr_rec.dflt_rtg <= 0)
				{
					print_mess (ML (mlStdMess007));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				local_rec.dst_alt = inmr_rec.dflt_rtg; 
				DSP_FLD ("dst_alt");
				abc_selfield (inmr, "inmr_id_no");
			}
			else
			{
				local_rec.dst_alt = inei_rec.dflt_rtg;
				DSP_FLD ("dst_alt");
			}
		}

		if (SRCH_KEY)
		{
			AlternateSrch (local_rec.dst_hhbr);
			return (EXIT_SUCCESS);
		}
		strcpy (rghr_rec.co_no, comm_rec.co_no);
		strcpy (rghr_rec.br_no, comm_rec.est_no);
		rghr_rec.hhbr_hash = local_rec.dst_hhbr;
		rghr_rec.alt_no = local_rec.dst_alt;
		cc = find_rec (rghr, &rghr_rec, EQUAL, "r");
		if (!cc)
		{
			sprintf 
			(
				err_str, 
				ML (mlStdMess002),
				local_rec.dst_alt,
				local_rec.dst_item
			);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
update (void)
{
	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = local_rec.src_hhbr;
	rghr_rec.alt_no = local_rec.src_alt;
	cc = find_rec (rghr, &rghr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, rghr, "DBFIND");

	strcpy (rghr2_rec.co_no, comm_rec.co_no);
	strcpy (rghr2_rec.br_no, comm_rec.est_no);
	rghr2_rec.hhbr_hash = local_rec.dst_hhbr;
	rghr2_rec.alt_no = local_rec.dst_alt;
	strcpy (rghr2_rec.print_all, rghr_rec.print_all);
	cc = abc_add (rghr2, &rghr2_rec);
	if (cc)
		file_err (cc, rghr2, "DBADD");

	cc = find_rec (rghr2, &rghr2_rec, EQUAL, "u");
	if (cc)
		file_err (cc, rghr2, "DBFIND");

	rgln_rec.hhgr_hash 	= rghr_rec.hhgr_hash;
	rgln_rec.seq_no 	= 0;
	cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
	while (!cc && rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
	{
		rgln2_rec.hhgr_hash 	= rghr2_rec.hhgr_hash;
		rgln2_rec.seq_no 		= rgln_rec.seq_no;
		rgln2_rec.hhwc_hash 	= rgln_rec.hhwc_hash;
		rgln2_rec.hhrs_hash 	= rgln_rec.hhrs_hash;
		rgln2_rec.rate 			= rgln_rec.rate;
		rgln2_rec.ovhd_var 		= rgln_rec.ovhd_var;
		rgln2_rec.ovhd_fix 		= rgln_rec.ovhd_fix;
		rgln2_rec.setup 		= rgln_rec.setup;
		rgln2_rec.run 			= rgln_rec.run;
		rgln2_rec.clean 		= rgln_rec.clean;
		rgln2_rec.qty_rsrc 		= rgln_rec.qty_rsrc;
		rgln2_rec.instr_no 		= 0;
		sprintf (rgln2_rec.yld_clc, "%-4.4s", rgln_rec.yld_clc);
		sprintf (rgln2_rec.can_split, "%-1.1s",rgln_rec.can_split);
		cc = abc_add (rgln2, &rgln2_rec);
		if (cc)
			file_err (cc, rgln2, "DBADD");
		cc = find_rec (rgln, &rgln_rec, NEXT, "r");
	}

	abc_unlock (rghr2);
}

/*=======================
| Search for Alternates	|
=======================*/
void
AlternateSrch (
	long	hhbrHash)
{
	char	alt_str [6];

	_work_open (5,0,2);
	save_rec ("#Alt.", "#         ");

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash 	= hhbrHash;
	rghr_rec.alt_no 		= 1;
	cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (rghr_rec.co_no, comm_rec.co_no) &&
		!strcmp (rghr_rec.br_no, comm_rec.est_no) &&
		rghr_rec.hhbr_hash == hhbrHash
	)
	{
		sprintf (alt_str, "%5d", rghr_rec.alt_no);

		cc = save_rec (alt_str, " ");
		if (cc)
			break;
		cc = find_rec (rghr, &rghr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbrHash;
	rghr_rec.alt_no = atoi (temp_str);
	cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
}
