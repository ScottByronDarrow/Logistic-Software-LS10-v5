/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_inlo_smnt.c,v 5.11 2002/07/25 11:17:35 scott Exp $
|  Program Name  : (sk_inlo_lmnt.c)                                   |
|  Program Desc  : (Stock Location status maintenance.          )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 16th Nov 2000    |
|---------------------------------------------------------------------|
| $Log: sk_inlo_smnt.c,v $
| Revision 5.11  2002/07/25 11:17:35  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.10  2001/11/29 06:35:50  scott
| Updated for Additional checks
|
| Revision 5.9  2001/11/29 04:06:06  scott
| Updated for additional validations.
|
| Revision 5.8  2001/11/28 01:35:01  scott
| Updated to have a warning message that location allocated to picking slip.
|
| Revision 5.7  2001/11/08 08:53:32  scott
| Updated to ignore zero quantities.
|
| Revision 5.6  2001/11/05 01:40:43  scott
| Updated from Testing.
|
| Revision 5.5  2001/10/24 06:56:25  scott
| Updated to still display even of active status not defined.
|
| Revision 5.4  2001/10/09 23:06:51  scott
| Updated for returns
|
| Revision 5.3  2001/09/24 01:51:26  scott
| Updated for number plate returns
|
| Revision 5.2  2001/08/09 09:18:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:02  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:56  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:15  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.10  2000/12/19 10:28:09  ramon
| Updated to remove the errors when compiled in LS10-GUI.
|
| Revision 1.9  2000/12/19 08:01:07  scott
| Updated to fix problem with missing lines.
|
| Revision 1.8  2000/12/12 00:57:49  scott
| Updated after final testing and code checking.
|
| Revision 1.7  2000/12/06 08:37:35  scott
| Updated to fix problem with not being able to split multiple times.
|
| Revision 1.6  2000/12/06 05:47:18  scott
| Updated as status not changed when non split item
|
| Revision 1.5  2000/12/06 05:34:19  scott
| Updated to use sknd_sknd_hash == inlo_sknd_hash instead of
| 		       sklo_inlo_hash == inlo_inlo_hash as not 1:M
| Updated to allow any transaction type to be split.
|
| Revision 1.4  2000/12/06 03:00:05  scott
| Updated to remove debug
|
| Revision 1.3  2000/12/05 05:48:31  scott
| Updated to add both pack and unit quantity. Added QC quantity and allow lines to be split.
|
| Revision 1.2  2000/11/22 04:18:45  scott
| Updated to only include locations with quantity > 0.00;
|
| Revision 1.1  2000/11/20 07:32:18  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_inlo_smnt.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inlo_smnt/sk_inlo_smnt.c,v 5.11 2002/07/25 11:17:35 scott Exp $";

#define MAXLINES	500
#define TABLINES	12
#define	INPUT		(prog_status == ENTRY)

#define		ITEM_SCN		1
#define		LOC_SCN			2

#define		LOC_NOCHANGE	0
#define		LOC_CHANGED		1
#define		LOC_ADDED		2

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inloRecord	inlo_rec;
struct inloRecord	inlo2_rec;
struct inloRecord	inlo3_rec;
struct llstRecord	llst_rec;
struct sknhRecord	sknh_rec;
struct skndRecord	sknd_rec;
struct skniRecord	skni_rec;

	char	*data 	= "data",
			*inlo2 	= "inlo2";

	int		envVarThreePlSystem = FALSE;

	extern	int		EnvScreenOK;

/*
 * Local & Screen Structures. 
 */
struct {
	char 	dummy [11];
	char	item_no [17];
	char	previousStatus [2];	
	float	splitQuantity;
	int		lineAddedStatus;
} local_rec;

static	struct	var	vars [] =
{
	{ITEM_SCN, LIN, "item_no",	4, 20, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Item number      ", " ",
		NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{ITEM_SCN, LIN, "itemDesc",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Item Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{LOC_SCN, TAB, "location",	 MAXLINES, 1, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "  Location  ", " ",
		NA, NO,  JUSTLEFT, "", "", inlo_rec.location},
	{LOC_SCN, TAB, "lotNumber",	 0, 2, CHARTYPE,
		"AAAAAAA", "          ",
		" ", " ", "Lot Number", " ",
		NA, NO,  JUSTLEFT, "", "", inlo_rec.lot_no},
	{LOC_SCN, TAB, "numberPlate",	 0, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "Number Plate/Grin.", " ",
		NA, NO,  JUSTLEFT, "", "", sknh_rec.plate_no},
	{LOC_SCN, TAB, "unitQty",	 0, 1, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "  Unit Qty   ", "",
		NA, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&inlo_rec.qty},
	{LOC_SCN, TAB, "UOM",	 0, 1, CHARTYPE,
		"AAAA", "          ",
		" ", " ", " UOM. ", " ",
		NA, NO,  JUSTLEFT, "", "", inlo_rec.uom},
	{LOC_SCN, TAB, "packageQty",	 0, 1, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Package Qty ", "",
		ND, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&inlo_rec.pack_qty},
	{LOC_SCN, TAB, "LocationStatus",	 0, 3, CHARTYPE,
		"U", "          ",
		" ", " ", "Status", "",
		YES, NO,  JUSTLEFT, "", "", inlo_rec.loc_status},
	{LOC_SCN, TAB, "PreviousLocationStatus",	 0, 2, CHARTYPE,
		"A", "          ",
		" ", " ", "Stat", "",
		ND, NO,  JUSTLEFT, "", "", local_rec.previousStatus},
	{LOC_SCN, TAB, "LocationStatusDesc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " Location Status Desc ", "",
		NA, NO,  JUSTLEFT, "", "", llst_rec.desc},
	{LOC_SCN, TAB, "inloHash",	 0, 2, LONGTYPE,
		"NNNNNNNN", "        ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&inlo_rec.inlo_hash},
	{LOC_SCN, TAB, "skndHash",	 0, 2, LONGTYPE,
		"NNNNNNNN", "        ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&inlo_rec.sknd_hash},
	{LOC_SCN, TAB, "lineAddedStatus",	 0, 0, INTTYPE,
		"N", "        ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.lineAddedStatus},
	{LOC_SCN, TAB, "cnvFct",	 0, 2, FLOATTYPE,
		"NNNNNNNN.NNNN", "        ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&inlo_rec.cnv_fct},
	{LOC_SCN, TAB, "splitQuantity",	 0, 1, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Split Quantity", "",
		YES, NO, JUSTRIGHT, "0.00","9999999.99", (char *)&local_rec.splitQuantity},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <LocHeader.h>

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
void	tab_other			(int);
void 	CloseDB 			(void);
void	InsertNewLine		(void);
void 	LoadInlo 			(void);
void 	OpenDB 				(void);
void 	SrchLlst			(char *);
void	Update 				(void);
int 	spec_valid 			(int);
int 	heading 			(int);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	EnvScreenOK	=	FALSE;

	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(ITEM_SCN);
	
	/*
	 * Check for 3pl Environment.
	 */
	sptr = chk_env ("PO_3PL_SYSTEM");
	envVarThreePlSystem = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (envVarThreePlSystem)
		FLD ("packageQty")	=	NA;
	
	OpenDB ();

	tab_row = 7;
	tab_col = 0;

	while (prog_exit == 0) 
	{
		lcount [LOC_SCN] 	= 0;
		entry_exit 	= 0;
		edit_exit 	= 0;
		prog_exit 	= 0;
		restart 	= 0;
		search_ok 	= 1;
		init_vars (ITEM_SCN);

		/*
		 * Enter screen ITEM_SCN linear input.
		 */
		heading (ITEM_SCN);
		scn_display (ITEM_SCN);
		entry (ITEM_SCN);
		if (prog_exit || restart) 
			continue;

		scn_write (ITEM_SCN);
		scn_display (ITEM_SCN);
		scn_write (LOC_SCN);
		scn_display (LOC_SCN);

		/*
		 * Enter screen LOC_SCN tabular input
		 */
		edit (LOC_SCN);

		if (prog_exit || restart) 
			continue;

		edit_all ();
		if (restart) 
			continue;

		/*
		 * Update selection status.    
		 */
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

	abc_alias (inlo2, inlo);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec (inlo2, inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");
	open_rec (llst,  llst_list, LLST_NO_FIELDS, "llst_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (sknh,  sknh_list, SKNH_NO_FIELDS, "sknh_sknh_hash");
	open_rec (sknd,  sknd_list, SKND_NO_FIELDS, "sknd_sknd_hash");
	open_rec (skni,  skni_list, SKNI_NO_FIELDS, "skni_sknd_hash");

	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,	comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
	{
		errmess (ML (mlStdMess100));
	   	sleep (sleepTime);
		clear_mess ();
	   	return;
	}
	OpenLocation (ccmr_rec.hhcc_hash);
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inlo);
	abc_fclose (inlo2);
	abc_fclose (llst);
	abc_fclose (ccmr);
	abc_fclose (sknh);
	abc_fclose (sknd);
	abc_fclose (skni);
	CloseLocation ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int		i;

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
	
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		/*
		 * Discontinued Product ?
		 */
		if (inmr_rec.active_status [0] == 'D')
		{
			print_mess (ML (mlSkMess558));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.item_no, inmr_rec.item_no);
		DSP_FLD ("item_no");
		DSP_FLD ("itemDesc");

		/*
		 * Look up to see if item is on Cost Centre
		 */
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = alt_hash 
								 (
									inmr_rec.hhbr_hash,
									inmr_rec.hhsi_hash
								);

		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			clear_mess ();
			errmess (ML (mlSkMess364));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		LoadInlo ();

		vars [label ("location")].row = lcount [LOC_SCN];
		scn_set (ITEM_SCN);

		if (lcount [LOC_SCN] == 0)
		{
			restart = TRUE;
			errmess (ML ("No location found for item"));
			sleep (sleepTime);
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Location Status
	 */
	if (LCHECK ("LocationStatus"))
	{
		if (SRCH_KEY)
		{
			SrchLlst (temp_str);
			return (EXIT_SUCCESS);
		}
		skni_rec.sknd_hash	=	inlo_rec.sknd_hash;
		cc = find_rec (skni, &skni_rec, COMPARISON, "r");
		if (!cc)
		{
			i = prmptmsg (ML ("Location allocated to packing slip, are you sure you want to continue. [Y/N] ?"), "YyNn", 1, 23);
			move (0, 23); cl_line ();
			if (i == 'N' || i == 'n')
				strcpy (inlo_rec.loc_status, local_rec.previousStatus);
		}
		strcpy (llst_rec.co_no, comm_rec.co_no);
		strcpy (llst_rec.code,  inlo_rec.loc_status);
		cc = find_rec (llst, &llst_rec, COMPARISON, "r");
		if (cc)
		{
			clear_mess ();
			errmess (ML ("Location Status not on file"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("LocationStatus");
		DSP_FLD ("LocationStatusDesc");
		if (local_rec.lineAddedStatus != LOC_ADDED)
	  		local_rec.lineAddedStatus = LOC_CHANGED;

		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Location Status
	 */
	if (LCHECK ("splitQuantity"))
	{
		if (inlo_rec.loc_status [0] == local_rec.previousStatus [0])
		{
			clear_mess ();
			errmess (ML ("Sorry cannot Split as new status same as old status."));
			sleep (sleepTime);
			clear_mess ();
		    local_rec.splitQuantity = 0.00;
			DSP_FLD ("splitQuantity");
			return (EXIT_SUCCESS);
		}
		skni_rec.sknd_hash	=	inlo_rec.sknd_hash;
		cc = find_rec (skni, &skni_rec, COMPARISON, "r");
		if (!cc)
		{
			i = prmptmsg (ML ("Location allocated to packing slip, are you sure you want to continue. [Y/N] ?"), "YyNn", 1, 23);
			move (0, 23); cl_line ();
			if (i == 'N' || i == 'n')
			{
				local_rec.splitQuantity = 0.00;
				DSP_FLD ("splitQuantity");
				return (EXIT_SUCCESS);
			}
		}
		if ((SK_BATCH_CONT || MULT_LOC))
		{
		    if (local_rec.splitQuantity > 0.00 && inlo_rec.qty != local_rec.splitQuantity)
		    {
				i = prmptmsg (ML ("Are you sure want to have split quantity [Y/N] ?"), "YyNn", 1, 23);
				move (0, 23); cl_line ();
				if (i == 'Y' || i == 'y')
			    	InsertNewLine ();
				else
				{
					local_rec.splitQuantity = 0.00;
					DSP_FLD ("splitQuantity");
				}
		    }
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}
/*
 * Load Invoice / Credit Detail Into Tabular. 
 */
void
LoadInlo (void)
{
	init_vars (LOC_SCN);
	scn_set (LOC_SCN);
	lcount [LOC_SCN] = 0;
	
	inlo_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	inlo_rec.hhum_hash	=	0L;
	strcpy (inlo_rec.location, " ");
	strcpy (inlo_rec.lot_no, " ");
	cc = find_rec (inlo, &inlo_rec, GTEQ, "r");	
	while (!cc && incc_rec.hhwh_hash == inlo_rec.hhwh_hash)
	{
		if (inlo_rec.qty <= 0.00)
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");	
			continue;
		}
		strcpy (llst_rec.co_no, comm_rec.co_no);
		strcpy (llst_rec.code, inlo_rec.loc_status);
		cc = find_rec (llst, &llst_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (llst_rec.co_no, comm_rec.co_no);
			strcpy (llst_rec.code, "A");
			cc = find_rec (llst, &llst_rec, COMPARISON, "r");
		}
		if (cc)
		{
			strcpy (llst_rec.code, "A");
			strcpy (llst_rec.desc, " ");
		}
		sknd_rec.sknd_hash	=	inlo_rec.sknd_hash;
		cc = find_rec (sknd, &sknd_rec, COMPARISON, "r");
		if (!cc)
		{	
			sknh_rec.sknh_hash	=	sknd_rec.sknh_hash;
			cc = find_rec (sknh, &sknh_rec, COMPARISON, "r");
			if (cc)
				strcpy (sknh_rec.plate_no, " ");
		}
		else
			strcpy (sknh_rec.plate_no, " ");

		strcpy (local_rec.previousStatus, inlo_rec.loc_status);
		local_rec.splitQuantity		=	0.00;
		local_rec.lineAddedStatus	=	LOC_NOCHANGE;

	   	putval (lcount [LOC_SCN]++);
	   	cc = find_rec (inlo, &inlo_rec, NEXT, "r");	
	}
}
/*
 * Update Files.  
 */
void
Update (void)
{
	scn_set (LOC_SCN);

	/*
	 * Process all lines in tabular screen.
	 */
	for (line_cnt = 0;line_cnt < lcount [LOC_SCN];line_cnt++)
	{
		getval (line_cnt);

		/*
		 * Only process lines that have been changed or added.
		 */
		if (local_rec.lineAddedStatus == LOC_ADDED ||
		    local_rec.lineAddedStatus == LOC_CHANGED)
		{
			inlo2_rec.inlo_hash	= inlo_rec.inlo_hash;
			cc = find_rec (inlo2, &inlo2_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, inlo2, "DBFIND");

			strcpy (inlo2_rec.loc_status, inlo_rec.loc_status);
			inlo2_rec.qty		=	inlo_rec.qty;
			inlo2_rec.pack_qty	=	inlo_rec.pack_qty;

			if (local_rec.lineAddedStatus == LOC_CHANGED)
			{
				cc = abc_update (inlo2, &inlo2_rec);
				if (cc)
					file_err (cc, inlo2, "DBUPDATE");
			}
			else
			{
				inlo_rec.inlo_hash	=	0L;
				sprintf 
				(
					inlo2_rec.lot_no, 
					"%07ld", 
					NextThreePlNo (ccmr_rec.hhcc_hash)
				);
				cc = abc_add (inlo2, &inlo2_rec);
				if (cc)
					file_err (cc, inlo2, "DBADD");
			}
			/*-------------------------------
			| Updated number plate details. |
			-------------------------------*/
			sknd_rec.sknd_hash	=	inlo_rec.sknd_hash;
			cc = find_rec (sknd, &sknd_rec, GTEQ, "u");
			while (!cc && sknd_rec.sknd_hash == inlo_rec.sknd_hash)
			{
				strcpy (sknd_rec.lstat_chg, "1");
				cc = abc_update (sknd, &sknd_rec);
				if (cc)
					file_err (cc, (char *)sknd, "DBUPDATE");

				cc = find_rec (sknd, &sknd_rec, NEXT, "u");
			}
			abc_unlock (sknd);
		}
	}
}
/*
 * Search routine for location status (llst)
 */
void
SrchLlst (
 char *key_val)
{
	_work_open (1,1,20);
	save_rec ("#C", "#Code Description");
	strcpy (llst_rec.co_no, comm_rec.co_no);
	sprintf (llst_rec.code, "%-1.1s", key_val);

	cc = find_rec (llst, &llst_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (llst_rec.co_no, comm_rec.co_no) &&
		   !strncmp (llst_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (llst_rec.code, llst_rec.desc);
		if (cc)
			break;

		cc = find_rec (llst, &llst_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (llst_rec.desc, "%-20.20s", " ");
		return;
	}

	strcpy (llst_rec.co_no, comm_rec.co_no);
	sprintf (llst_rec.code, "%-1.1s", temp_str);
	cc = find_rec (llst, &llst_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)llst, "DBFIND");
}
/*
 * Screen Heading.
 */
int
heading (
	int		scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();

	fflush (stdout);
	rv_pr (ML ("Location Status Maintenance."),50,0,1);
	line_at (1,0,130);

	move (1,input_row);
	switch (scn)
	{
	case  ITEM_SCN :
		box (0,3,130,2);
		scn_set (LOC_SCN);

		/* Need to set line_cnt to 0 because we changed screen */
		line_cnt = 0;
		scn_write (LOC_SCN);
		scn_display (LOC_SCN);
		break;
		
	case  LOC_SCN :
		box (0,3,130,2);
		scn_set (ITEM_SCN);

		/* Need to set line_cnt to 0 because we changed screen */
		line_cnt = 0;
		scn_write (ITEM_SCN);
		scn_display (ITEM_SCN);
		break;
	}

	line_at (21,0,130);
	strcpy (err_str, ML (mlStdMess038));		
	print_at (22, 0, err_str,comm_rec.co_no,clip (comm_rec.co_short));
	strcpy (err_str, ML (mlStdMess039));		
	print_at (22, 32, err_str, comm_rec.est_no,clip (comm_rec.est_short));
	strcpy (err_str, ML (mlStdMess099));		
	print_at (22, 52, err_str, comm_rec.cc_no,clip (comm_rec.cc_short));
		
	line_cnt = 0;
	scn_write (scn);
	
    return (EXIT_SUCCESS);
}

/*
 * Routine to insert lines when quantities split.
 */
void
InsertNewLine (void)
{
	int		doInsert = TRUE,
			i;
	long	tmpInloHash;

	float	holdSplitQty	=	local_rec.splitQuantity;

	inlo_rec.qty 				= inlo_rec.qty - local_rec.splitQuantity;
	local_rec.splitQuantity		= 0.0;
	inlo_rec.pack_qty			= inlo_rec.qty / inlo_rec.cnv_fct;
	local_rec.lineAddedStatus	= LOC_CHANGED;

	putval (line_cnt);

	tmpInloHash = inlo_rec.inlo_hash;

	if (doInsert)
	{
		for (i = lcount [LOC_SCN]; i > line_cnt; i--)
		{
			getval (i);
			putval (i + 1);
		}
		lcount [LOC_SCN]++;
		vars [scn_start].row = lcount [LOC_SCN];
	}
	getval (line_cnt);

	inlo_rec.qty 				= holdSplitQty;
	inlo_rec.pack_qty			= inlo_rec.qty / inlo_rec.cnv_fct;
	local_rec.lineAddedStatus	= LOC_ADDED;
	strcpy (local_rec.previousStatus, inlo_rec.loc_status);

	putval (line_cnt + 1);

	getval (line_cnt);

	strcpy (inlo_rec.loc_status, local_rec.previousStatus);
	strcpy (llst_rec.co_no, comm_rec.co_no);
	strcpy (llst_rec.code, inlo_rec.loc_status);
	cc = find_rec (llst, &llst_rec, COMPARISON, "r");
	if (cc)
		strcpy (llst_rec.desc, " ");

	putval (line_cnt);

	scn_display (LOC_SCN);
}

