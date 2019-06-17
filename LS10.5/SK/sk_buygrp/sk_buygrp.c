/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_buygrp.c,v 5.7 2002/04/16 09:53:02 kaarlo Exp $
|  Program Name  : (sk_buygrp.c  )                                    |
|  Program Desc  : (Add Maintain Buying/Selling Groups.        )      |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 13/09/93         |
|---------------------------------------------------------------------|
| $Log: sk_buygrp.c,v $
| Revision 5.7  2002/04/16 09:53:02  kaarlo
| S/C 894, 895, 896. Updated to display description code and details.
|
| Revision 5.6  2002/04/16 09:49:43  cha
| S/C 894, 895, 896. Updated to display description code and details.
|
| Revision 5.5  2002/04/11 03:46:21  scott
| Updated to add comments to audit files.
|
| Revision 5.4  2001/12/13 04:30:34  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_buygrp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_buygrp/sk_buygrp.c,v 5.7 2002/04/16 09:53:02 kaarlo Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <DBAudit.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct ingpRecord	ingp_rec;

	char	*data   = "data";

	char	prompt [25],
			promptdesc [50];

	int		ReverseDiscount = FALSE,
			newIngp			= FALSE,
			buyerGroup 		= FALSE,
			deleteOption 	= FALSE;

struct
{
	char	desc [41];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "code",	 4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", prompt, promptdesc,
		 NE, NO,  JUSTLEFT, "", "", ingp_rec.code},
	{1, LIN, "desc",	5, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Code Description       ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "sellpc",	7, 24, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", "", "Regulatory Percentage  ", " ",
		 ND, NO,  JUSTLEFT, "-999.99", "100", (char *) &ingp_rec.sell_reg_pc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function Declarations
 */
void 	OpenDB			(void);
void 	CloseDB			(void);
int  	spec_valid		(int);
void 	SrchIngp		(char *);
int  	heading			(int);
void 	Update			(void);
void 	shutdown_prog 	(void);
int  	CheckUsed 		(char *);
float 	ScreenDisc 		(float);


/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char * argv [])
{

	char	*sptr = strrchr (argv [0], '/');

	if (sptr)
		argv [0] = sptr + 1;

	if (!strncmp (argv [0], "sk_buygrp", 9))
		buyerGroup = TRUE;
	else
		buyerGroup = FALSE;

	if (argc < 2)
	{
		print_at (0,0,mlSkMess617, argv [0]);
		return (EXIT_FAILURE);
	}

	if (argv [1][0] == 'D' || argv [1][0] == 'd')
		deleteOption = TRUE;
	else
		deleteOption = FALSE;

	sptr = chk_env ("SO_DISC_REV");
	ReverseDiscount = (sptr == (char *)0) ? FALSE : atoi (sptr);

	SETUP_SCR (vars);

	if (buyerGroup)
	{
		sprintf (prompt, ML ("Buying Code            "));
		sprintf (promptdesc, ML ("Enter Buying Group Code, Full Serach Available "));
		FLD ("sellpc") = ND;
		if (deleteOption)
			FLD ("desc") = NA;
	}
	else
	{
		sprintf (prompt, ML ("Selling Code           "));
		sprintf (promptdesc, ML ("Enter Selling Group Code, Full Serach Available "));
		if (deleteOption)
		{
			FLD ("desc")	= NA;
			FLD ("sellpc")	= NA;
		}
		else
			FLD ("sellpc")	= YES;
	}

	/*
	 * Read common terminal record.
	 */
	OpenDB ();

	init_scr ();
	set_tty (); 
	set_masks ();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
	
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
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS,
			(buyerGroup) ? "inmr_id_buy" : "inmr_id_sell");
	/*
	 * Open audit file.
	 */
	OpenAuditFile ("BuyingSellingGroups.txt");
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (ingp);	
	abc_fclose (inmr);	
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	char	mess [80];

	if (LCHECK ("code"))
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (clip (ingp_rec.code)) == 0)
			return (EXIT_FAILURE);

		sprintf (ingp_rec.code, "%-6.6s", ingp_rec.code);
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		strcpy (ingp_rec.type, (buyerGroup) ? "B" : "S");

		newIngp = find_rec (ingp, &ingp_rec, EQUAL, "u");
		
		if (newIngp && deleteOption)
		{
			abc_unlock (ingp);
			print_mess (ML (mlStdMess233));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (deleteOption && !newIngp)
		{
			if (CheckUsed (mess))
			{
				abc_unlock (ingp);
				print_mess (mess);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		 	ingp_rec.sell_reg_pc	=	ScreenDisc (ingp_rec.sell_reg_pc);
		}

		if (!newIngp)
		{
			strcpy (local_rec.desc, ingp_rec.desc);
			scn_display (1);
			entry_exit = TRUE;
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&ingp_rec, sizeof (ingp_rec));
		}
		else
			abc_unlock (ingp);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchIngp (
	char	*keyValue)
{
	_work_open (6,0,40);
	save_rec ("#Code", "#Description ");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, (buyerGroup) ? "B" : "S");
	sprintf (ingp_rec.code, "%-6.6s", keyValue);

	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, keyValue, strlen (keyValue)))
	{
		if ((buyerGroup && ingp_rec.type [0] == 'B') ||
			(!buyerGroup && ingp_rec.type [0] == 'S'))
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
	strcpy (ingp_rec.type, (buyerGroup) ? "B" : "S");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

void
Update (void)
{
	if (deleteOption)
	{
		cc = abc_delete (ingp);
		if (cc)
			file_err (cc, ingp, "DBDELETE");
		return;
	}
	ingp_rec.sell_reg_pc	=	ScreenDisc (ingp_rec.sell_reg_pc);

	if (buyerGroup)
	{
		strcpy (ingp_rec.type, "B");
		ingp_rec.sell_reg_pc = 0.00;
	}
	else
		strcpy (ingp_rec.type, "S");

	strcpy (ingp_rec.desc, local_rec.desc);
	if (newIngp)
		cc = abc_add (ingp, &ingp_rec);
	else
	{
		cc = abc_update (ingp,&ingp_rec);
		/*
		 * Update changes audit record.
		 */

		 sprintf (err_str, "%s : %s (%s)", ML ("Group"), ingp_rec.code, ingp_rec.desc);
		 AuditFileAdd (err_str, &ingp_rec, ingp_list, INGP_NO_FIELDS);
	}
	if (cc)
		file_err (cc, ingp, (newIngp) ? "DBADD" : "DBUPDATE");
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
CheckUsed (
	char	*mess)
{
	/*
	 * look up inmr
	 */
	memset (&inmr_rec, 0, sizeof (inmr_rec));
	strcpy (inmr_rec.co_no, comm_rec.co_no);

	if (buyerGroup)
		strcpy (inmr_rec.buygrp, ingp_rec.code);
	else
		strcpy (inmr_rec.sellgrp, ingp_rec.code);

	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");

	if (!cc)
	{
		if (buyerGroup)
			sprintf (mess,ML (mlSkMess705));
		else
			sprintf (mess,ML (mlSkMess706));
		return (TRUE);
	}

	return (EXIT_SUCCESS);
}

/*
 * Reverse Screen Discount.
 */
float	
ScreenDisc (
	float	DiscountPercent)
{
	if (ReverseDiscount)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	if (buyerGroup)
		sprintf (err_str, (deleteOption) ? ML (mlSkMess294) : ML (mlSkMess295));
	else
		sprintf (err_str, (deleteOption) ? ML (mlSkMess296) : ML (mlSkMess297));

	rv_pr (err_str, 24, 0, 1);

	box (0, 3, 80, (buyerGroup) ? 2 : 4);

	if (!buyerGroup)
		line_at (6,1,79);
	
	line_at (1,0,80);
	line_at (21,0,80);

	print_at (22, 1,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
