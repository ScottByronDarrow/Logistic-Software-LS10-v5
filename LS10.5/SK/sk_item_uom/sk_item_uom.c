/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (sk_item_uom.c)
|  Program Desc  : (Maintain stock item UOM file)
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: sk_item_uom.c,v $
| Revision 5.2  2002/01/30 07:18:12  robert
| SC 00721 - Updated to add sleep on error message.
|
| Revision 5.1  2001/12/10 07:56:15  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_item_uom.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_item_uom/sk_item_uom.c,v 5.2 2002/01/30 07:18:12 robert Exp $";

#define MAXSCNS 	2
#define MAXLINES	100

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

	/*
	 * Special fields and flags.
	 */
	int		NewINUV = 0, 
			envDbCo = 0;

#include	"schema"

struct commRecord	comm_rec;
struct inuvRecord	inuv_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inumRecord	inum3_rec;
struct inisRecord	inis_rec;

	char	*inum2	=	"inum2", 
			*inum3	=	"inum3";

/*===========================
| Local & Screen Structures |
===========================*/
struct {			
	char	dummy [11];
} local_rec;            
		
static	struct	var	vars []	={	

	{1, LIN, "item_no", 4, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number.", " ", 
		YES, NO, JUSTLEFT, "", "", inmr_rec.item_no}, 
	{1, LIN, "desc", 5, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Item Description ", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	{2, TAB, "UOM", 	 MAXLINES, 1, CHARTYPE, 
		"AAAA", "          ", 
		" ", "", " UOM. ", " Unit of Measure ", 
		 YES, NO, JUSTLEFT, "", "", inum2_rec.uom}, 
	{2, TAB, "uom_desc", 0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "        Item Description                  ", " ", 
		NA, NO, JUSTLEFT, "", "", inum2_rec.desc}, 
	{2, TAB, "weight", 0, 1, FLOATTYPE, 
		"NNNNNNN.NNNN", "          ", 
		" ", "", "  Weight (kg)  ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&inuv_rec.weight}, 
	{2, TAB, "volume", 0, 1, FLOATTYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "", "    Volume    ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&inuv_rec.volume}, 
	{2, TAB, "height", 0, 1, FLOATTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "", "  Height (cm)  ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&inuv_rec.height}, 
	{2, TAB, "width", 0, 1, FLOATTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "", "  Width (cm)   ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&inuv_rec.width}, 
	{2, TAB, "hhum_hash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", "", 
		 ND, NO, JUSTLEFT, "", "", (char *)&inum2_rec.hhum_hash}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
int  	DeleteLine 		(void);
int  	LoadInuv 		(long);
void 	Update 			(void);
void 	PrintCoStuff	(void);
void 	SrchInum 		(char *);
int  	heading 		(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	SETUP_SCR (vars);

	tab_row = 7;
	tab_col = 10;

	init_scr ();
	swide ();
	set_tty ();
	set_masks ();

	OpenDB ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		lcount [2] = 0;
		init_vars (1);	
		init_vars (2);	

		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		scn_write (1);
		scn_display (1);
		scn_write (2);
		scn_display (2);

		if (NewINUV == 1) 
			entry (2);
		else
			edit (2);

		if (restart)
			continue;

		Update ();

	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inum2, inum);
	abc_alias (inum3, inum);

	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (inum3, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inuv, inuv_list, INUV_NO_FIELDS, "inuv_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (inum3);
	abc_fclose (inum2);
	abc_fclose (inum);
	abc_fclose (inuv);
	abc_fclose (inmr);
	abc_fclose (inis);
	SearchFindClose ();
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
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
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		/*---------------------
		| Find for UOM GROUP. |
		----------------------*/
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");
	
		NewINUV = 0;
		cc = LoadInuv (inmr_rec.hhbr_hash);
		if (cc)
			NewINUV = 1;
		else
			entry_exit = 1;

		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| Validate Unit of Measure. | 
	---------------------------*/
	if (LCHECK ("UOM"))
	{
		if (last_char == DELLINE || dflt_used)
			return (DeleteLine ());

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Invalid Unit of Measure.
			 */
			print_mess (ML ("Invalid unit of measure."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("UOM");
		DSP_FLD ("uom_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	/*
	 * entry
	 */
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	/*
	 * no lines to delete
	 */
	if (lcount [2] <= 0)
	{
		print_mess (ML (mlStdMess032));
		return (EXIT_FAILURE);
	}

	/*
	 * delete lines
	 */
	lcount [2]--;
	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		if (line_cnt / TABLINES == this_page)
			line_display ();
	}

	/*
	 * blank last line - if required
	 */
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	/*
	 * zap buffer if deleted all
	 */
	if (lcount [2] <= 0)
	{
		init_vars (2);
		putval (i);
	}
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}
/*
 * Read detail lines from note-pad detail file.
 */
int
LoadInuv (
	long	hhbrHash)
{
	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (2);
	lcount [2] = 0;

	inuv_rec.hhbr_hash	=	hhbrHash;
	inuv_rec.hhum_hash	=	0L;

	cc = find_rec (inuv, &inuv_rec, GTEQ, "r");
	while (!cc && inuv_rec.hhbr_hash == hhbrHash)
	{
		inum3_rec.hhum_hash	=	inuv_rec.hhum_hash;
		
		cc = find_rec (inum3, &inum3_rec, COMPARISON, "r");
		if (!cc)
		{
			inum2_rec.hhum_hash	=	inum3_rec.hhum_hash;
			strcpy (inum2_rec.desc, inum3_rec.desc);
			strcpy (inum2_rec.uom, inum3_rec.uom);

			inis_rec.hhbr_hash	=	hhbrHash;
			strcpy (inis_rec.sup_priority, "  ");
			strcpy (inis_rec.co_no, "  ");
			strcpy (inis_rec.br_no, "  ");
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec (inis, &inis_rec, GTEQ, "r");
			while (!cc && inis_rec.hhbr_hash == hhbrHash)
			{
				if (inis_rec.sup_uom == inuv_rec.hhum_hash)
				{
					if (inuv_rec.weight == 0 && inis_rec.weight != 0.0)
						inuv_rec.weight	=	inis_rec.weight;

					if (inuv_rec.volume == 0 && inis_rec.volume != 0.0)
						inuv_rec.volume	=	inis_rec.volume;

					break;
				}
				cc = find_rec (inis, &inis_rec, NEXT, "r");
			}
			putval (lcount [2]++);
		}
		cc = find_rec (inuv, &inuv_rec, NEXT, "r");
	}
	scn_set (1);

	/*
	 * No entries to edit.
	 */
	if (lcount [2] == 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * Update all files.
 */
void
Update (void)
{
	inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	inuv_rec.hhum_hash	=	0L;
	cc = find_rec (inuv, &inuv_rec, GTEQ, "u");
	while (!cc && inuv_rec.hhbr_hash	==	inmr_rec.hhbr_hash)
	{
		abc_delete (inuv);

		inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inuv_rec.hhum_hash	=	0L;
		cc = find_rec (inuv, &inuv_rec, GTEQ, "r");
	}
	scn_set (2);
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++) 
	{
		getval (line_cnt);

		inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inuv_rec.hhum_hash	=	inum2_rec.hhum_hash;

		inis_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		strcpy (inis_rec.sup_priority, "  ");
		strcpy (inis_rec.co_no, "  ");
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, GTEQ, "u");
		while (!cc && inis_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (inis_rec.sup_uom == inuv_rec.hhum_hash)
			{
				if (inis_rec.weight == 0 && inuv_rec.weight != 0.0)
					inis_rec.weight	=	inuv_rec.weight;

				if (inis_rec.volume == 0 && inuv_rec.volume != 0.0)
					inis_rec.volume	=	inuv_rec.volume;

				cc = abc_update (inis, &inis_rec);
				if (cc)
					file_err (cc, inis, "DBUPDATE");

			}
			else
				abc_unlock (inis);

			cc = find_rec (inis, &inis_rec, NEXT, "u");
		}
		abc_unlock (inis);
		cc = abc_add (inuv, &inuv_rec);
		if (cc) 
			file_err (cc, inuv, "DBADD");

	}
}

/*
 * Print Company Details.
 */
void
PrintCoStuff (void)
{
	line_at (20, 0, 130);
	strcpy (err_str, ML (mlStdMess038));
	print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name);

	line_at (22, 0, 130);
}

/*==========================
| Search on UOM (inum)     |
==========================*/
void
SrchInum (
 char *key_val)
{
	_work_open (4, 0, 40);
	save_rec ("#UOM ", "#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc && !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
	{                        
		if (strncmp (inum2_rec.uom_group, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (inum2_rec.uom, inum2_rec.desc);
		if (cc)
			break;

		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
 	        file_err (cc, inum2, "DBFIND");
}

/*================
| Print Heading. |
================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		swide ();
		rv_pr (ML ("Inventory Item Unit of Measure Maintenance."), 25, 0, 1);
		
		line_at (1, 0, 130);

		move (1, input_row);
		switch (scn)
		{
		case	1:
			scn_set (2);
			scn_write (2);
			scn_display (2);

			box (0, 3, 130, 2);
			break;

		case	2:
			scn_set (1);
			scn_write (1);
			scn_display (1);
			box (0, 3, 130, 2);
		}
		
		PrintCoStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
