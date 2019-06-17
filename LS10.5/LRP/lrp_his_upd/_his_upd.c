/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: _his_upd.c,v 5.3 2001/09/11 01:54:06 cha Exp $
|  Program Name  : (ff_his_upd.c)                                     |
|  Program Desc  : ( Update demand history from intr and/or ffdm  )   |
|---------------------------------------------------------------------|
|  Author        : Roel Michels    | Date Written  : 10/05/95         |
|---------------------------------------------------------------------|
| $Log: _his_upd.c,v $
| Revision 5.3  2001/09/11 01:54:06  cha
| SE-221.Updated to put delay in error messages.
|
| Revision 5.2  2001/08/09 09:29:48  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:32  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:24  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2001/01/30 05:48:52  scott
| Updated to add app.schema
|
| Revision 3.2  2001/01/24 01:20:36  scott
| Updated to add app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _his_upd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_his_upd/_his_upd.c,v 5.3 2001/09/11 01:54:06 cha Exp $";

#define	BY_TRANSACTIONS		 (local_rec.intr [0] == 'Y')
#define	BY_HISTORY			 (local_rec.std  [0] == 'Y')

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct ffhsRecord	ffhs_rec;
struct ffdmRecord	ffdm_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct intrRecord	intr_rec;

	float	*incc_cons	=	&incc_rec.c_1;
	float	*ffhs_cons	=	&ffhs_rec.per1;
	char	*data	= "data";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	item_no [17];
	char	item_desc [41];
	char	sup_part [17];
	float	curr_his [12];
	float	last_his [24];
	char	dem_type [2];
	char	per_type [2];
	char	dem_desc [21];
	char	intr [2];
	char	std [2];
	char	month [2];
} local_rec;

static	struct	var vars [] =
{
	{1, LIN, "item_no",	 5,  24, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "ALL", "Item Number.", "Enter Item Number [SEARCH], or (default) ALL (items)",
		 YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc",		 6,  24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Item Description.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{1, LIN, "intr",		 8,  24, CHARTYPE,
		"U", "          ",
		" ", "Y", "From Transactions (Y/N)", "Use Inventory Transaction Table for Update?",
		 YES, NO,  JUSTLEFT, "YN", "", local_rec.intr},
	{1, LIN, "std",		 9,  24, CHARTYPE,
		"U", "          ",
		" ", "Y", "From Old DRP files (Y/N)", "Use Existing DRP history tables for Update?",
		 YES, NO,  JUSTLEFT, "YN", "", local_rec.std},
	{0, LIN, "",		 0,   0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
int 	spec_valid 				(int);
void 	Update 					(void);
void 	UpdateItem 				(void);
void 	UpdateAll 				(void);
void 	createFromFfhs 			(void);
void 	createFromIncc 			(void);
void 	DeleteDemandHistory 	(long);
void 	CreateDemandHistory 	(long, long, long, int, float);
int		heading (int scn);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	SETUP_SCR (vars);

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();
	set_tty ();
	snorm ();

	set_masks ();
	init_vars (1);

	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

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

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
		{
			abc_unlock (incc);
			abc_unlock (ffhs);
			abc_unlock (ffdm);
			continue;
		}

		if (!restart)
			Update ();

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
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	open_rec (ffhs, ffhs_list, FFHS_NO_FIELDS, "ffhs_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (ffdm, ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (intr,  intr_list,INTR_NO_FIELDS, "intr_id_no2");

}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ffhs);
	abc_fclose (incc);
	abc_fclose (ffdm);
	abc_fclose (inmr);
	abc_fclose (intr);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int    field)
{
	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{
		if (dflt_used || !strncmp (local_rec.item_no,"ALL", 3))
		{
			strcpy (local_rec.item_no, "ALL");
			strcpy (local_rec.item_desc, ML ("All Items"));
			DSP_FLD ("item_no");
			DSP_FLD ("desc");
			return (EXIT_SUCCESS);
		}

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
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		strcpy (local_rec.item_no, inmr_rec.item_no);
		strcpy (local_rec.item_desc, inmr_rec.description);
		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		return (cc);
	}
	if (LCHECK ("intr"))
	{
		if (BY_TRANSACTIONS)
		{
			FLD ("std") 	=	NA;
			FLD ("intr") 	=	YES;
			strcpy (local_rec.std, 	"N");
			strcpy (local_rec.intr,	"Y");
		}
		else
		{
			FLD ("std") 	=	YES;
			FLD ("intr") 	=	NA;
			strcpy (local_rec.std, 	"Y");
			strcpy (local_rec.intr,	"N");
		}
		DSP_FLD ("intr");
		DSP_FLD ("std");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("std"))
	{
		if (BY_HISTORY)
		{
			FLD ("std") 	=	YES;
			FLD ("intr") 	=	NA;
			strcpy (local_rec.std, 	"Y");
			strcpy (local_rec.intr,	"N");
		}
		else
		{
			FLD ("std") 	=	NA;
			FLD ("intr") 	=	YES;
			strcpy (local_rec.std, 	"N");
			strcpy (local_rec.intr,	"Y");
		}
		DSP_FLD ("intr");
		DSP_FLD ("std");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
Update (void)
{
	if (!strncmp (local_rec.item_no, "ALL",3))
		UpdateAll ();
	else
		UpdateItem ();
}

void
UpdateItem (void)
{
	dsp_screen ("Creating Demand History", comm_rec.co_no,comm_rec.co_name);

	/*----------------------------------------------------------
	| Clear any existing demand history from ffdm demand file. |
	----------------------------------------------------------*/
	abc_selfield (ffdm, "ffdm_id_no");

	DeleteDemandHistory ( inmr_rec.hhbr_hash );

	abc_selfield (ffdm, "ffdm_id_no2");

	if (BY_TRANSACTIONS)
	{
		intr_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		intr_rec.date		=	0L;
		cc = find_rec (intr, &intr_rec, GTEQ, "r");
		while (!cc && intr_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (intr_rec.type == 6 || intr_rec.type == 7)
			{
				CreateDemandHistory
				 (
					intr_rec.hhbr_hash,
					intr_rec.hhcc_hash,
					intr_rec.date,
					 (intr_rec.type == 6) ? TRUE : FALSE,
					intr_rec.qty
				);
			}
			cc = find_rec (intr, &intr_rec, NEXT, "r");
		}
	}
	else
	{
		incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, GTEQ, "r");
		while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			createFromIncc ();

			cc = find_rec (incc, &incc_rec, NEXT, "r");
		}
		ffhs_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (ffhs, &ffhs_rec, GTEQ, "r");
		while (!cc && ffhs_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			createFromFfhs ();

			cc = find_rec (ffhs, &ffhs_rec, NEXT, "r");
		}
	}
}

void
UpdateAll (void)
{
	dsp_screen ("Creating Demand History", comm_rec.co_no,comm_rec.co_name);

	abc_selfield (ffdm, "ffdm_id_no2");
	abc_selfield (inmr, "inmr_id_no");

	strcpy	 (inmr_rec.co_no, comm_rec.co_no);
	strcpy	 (inmr_rec.item_no, "                ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		dsp_process ("Item", inmr_rec.item_no);
		/*----------------------------------------------------------
		| Clear any existing demand history from ffdm demand file. |
		----------------------------------------------------------*/
		abc_selfield (ffdm, "ffdm_id_no");

		DeleteDemandHistory ( inmr_rec.hhbr_hash );

		abc_selfield (ffdm, "ffdm_id_no2");

		if (BY_TRANSACTIONS)
		{
			intr_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			intr_rec.date		=	0L;
			cc = find_rec (intr, &intr_rec, GTEQ, "r");
			while (!cc && intr_rec.hhbr_hash == inmr_rec.hhbr_hash)
			{
				if (intr_rec.type == 6 || intr_rec.type == 7)
				{
					CreateDemandHistory
					 (
						intr_rec.hhbr_hash,
						intr_rec.hhcc_hash,
						intr_rec.date,
						 (intr_rec.type == 6) ? TRUE : FALSE,
						intr_rec.qty
					);
				}
				cc = find_rec (intr, &intr_rec, NEXT, "r");
			}
		}
		else
		{
			incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			cc = find_rec (incc, &incc_rec, GTEQ, "r");
			while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
			{
				createFromIncc ();

				cc = find_rec (incc, &incc_rec, NEXT, "r");
			}
			ffhs_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			cc = find_rec (ffhs, &ffhs_rec, GTEQ, "r");
			while (!cc && ffhs_rec.hhbr_hash == inmr_rec.hhbr_hash)
			{
				createFromFfhs ();

				cc = find_rec (ffhs, &ffhs_rec, NEXT, "r");
			}
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
createFromFfhs (void)
{
	int		Invoice;
	long	CalcDate;
	int		dmy [3];
	int		currMonth;
	int		i;

	DateToDMY (comm_rec.inv_date, &dmy [0],&dmy [1],&dmy [2]);

	dmy [2]--;

	currMonth	=	dmy [1] - 1;

	for (i = 0; i < 24; i++)
	{
		dmy [0]	=	1;
		dmy [1]	=	i + 1;
		if (dmy [1] > 12)
			dmy [1] -= 12;
		if (i > currMonth)
		{
			dmy [2]--;
			currMonth	+=	12;
		}

		if (ffhs_cons [i] == 0.00)
			continue;

		if ( ffhs_cons [i] > 0.00)
			Invoice	=	TRUE;
		else
		{
			Invoice	=	FALSE;
			ffhs_cons [i] *= -1;
		}
		CalcDate = DMYToDate (dmy [0], dmy [1], dmy [2]);
		CreateDemandHistory
		 (
			ffhs_rec.hhbr_hash,
			ffhs_rec.hhcc_hash,
			CalcDate,
			Invoice,
			ffhs_cons [i]
		);
	}
}

void
createFromIncc (void)
{
	int		Invoice;
	long	CalcDate;
	int		dmy [3];
	int		currMonth;
	int		i;

	DateToDMY (comm_rec.inv_date, &dmy [0],&dmy [1],&dmy [2]);

	currMonth	=	dmy [1] - 1;

	for (i = 0; i < 12; i++)
	{
		dmy [0]	=	1;
		dmy [1]	=	i + 1;
		if (i > currMonth)
		{
			dmy [2]--;
			currMonth	+=	12;
		}

		if (incc_cons [i] == 0.00)
			continue;

		if ( incc_cons [i] > 0.00)
			Invoice	=	TRUE;
		else
		{
			Invoice	=	FALSE;
			incc_cons [i] *= -1;
		}
		CalcDate = DMYToDate (dmy [0], dmy [1], dmy [2]);
		CreateDemandHistory
		 (
			incc_rec.hhbr_hash,
			incc_rec.hhcc_hash,
			CalcDate,
			Invoice,
			incc_cons [i]
		);
	}
}
			
/*================================================
| Delete Demand History for each item processed. |
================================================*/
void
DeleteDemandHistory (
	long	hhbrHash)
{
	ffdm_rec.hhbr_hash	=	hhbrHash;
	ffdm_rec.hhcc_hash	=	0L;
	strcpy (ffdm_rec.type, "1");
	ffdm_rec.date		=	0L;
	cc = find_rec (ffdm, &ffdm_rec, GTEQ, "r");
	while (!cc && ffdm_rec.hhbr_hash == hhbrHash &&
		   ffdm_rec.type [0] == '1')
	{
		abc_delete (ffdm);

		ffdm_rec.hhbr_hash	=	hhbrHash;
		strcpy (ffdm_rec.type, "1");
		ffdm_rec.date		=	0L;
		cc = find_rec (ffdm, &ffdm_rec, GTEQ, "r");
	}
}

void
CreateDemandHistory (
	long	hhbrHash,
	long	hhccHash,
	long	transDate,
	int		Invoice,
	float	transQuantity)
{

	ffdm_rec.hhbr_hash	=	hhbrHash;
	ffdm_rec.hhcc_hash	=	hhccHash;
	ffdm_rec.date		=	transDate;
	strcpy (ffdm_rec.type, "1");
	cc 	=	find_rec (ffdm, &ffdm_rec, COMPARISON, "r");
	if (cc)
	{
		ffdm_rec.qty	=	 (Invoice) ? transQuantity : 0 - transQuantity;
		cc	=	abc_add (ffdm, &ffdm_rec);
		if (cc)
			file_err (cc, ffdm, "DBADD");
	}
	else
	{
		if (Invoice)
			ffdm_rec.qty	+= 	transQuantity;
		else
			ffdm_rec.qty	-= 	transQuantity;

		cc	=	abc_update (ffdm, &ffdm_rec);
		if (cc)
			file_err (cc, ffdm, "DBUPDATE");
	}
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (" LRP Demand History Update "), 20, 0, 1);
		move (0, 1);
		line (79);
	
		box (0, 4, 79, 5);
		move (1, 7);
		line (78);

		move (0, 20);
		line (79);
		move (0, 21);
		print_at (21,0," Company no. : %s   %s", comm_rec.co_no, comm_rec.co_name);
		move (0, 22);
		line (79);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

	return (EXIT_SUCCESS);
}
