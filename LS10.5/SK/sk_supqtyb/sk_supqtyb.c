/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: sk_supqtyb.c,v 5.2 2001/08/09 09:20:15 scott Exp $
|  Program Name  : ( sk_supqtyb.c  )                                  |
|  Program Desc  : ( Supplier Qty Break Update                    )   |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 13/10/93         |
|---------------------------------------------------------------------|
| $Log: sk_supqtyb.c,v $
| Revision 5.2  2001/08/09 09:20:15  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:46:03  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:04  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/28 04:10:36  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated for changes to insp schema to change money fields to double to allow
| for four digit price.
|
=====================================================================*/
char	*PNAME = "$RCSfile: sk_supqtyb.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_supqtyb/sk_supqtyb.c,v 5.2 2001/08/09 09:20:15 scott Exp $";

#define	CCMAIN
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>


#include	"schema"

struct commRecord	comm_rec;
struct ingpRecord	ingp_rec;
struct sumrRecord	sumr_rec;
struct inspRecord	insp_rec;
struct inmrRecord	inmr_rec;

	float	*insp_qty_brk	=	&insp_rec.qty_brk1;

	char	*data   = "data";

	int		envVarCrFind	=	0,
			envVarCrCo		=	0;
	extern	int		TruePosition;

	char	branchNumber [3];

struct
{
	char	sumrdesc [41];
	char	startBuyGroup [7];
	char	startBuyGroupDesc [41];
	char	endBuyGroup [7];
	char	endBuyGroupDesc [41];
	float	qtybr [5];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNo",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier            ", "Select Supplier, Full Search Available",
		 NE, NO,  JUSTLEFT, "", "", sumr_rec.crd_no},
	{1, LIN, "supplierDesc",	4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sumrdesc},
	{1, LIN, "startBuyGroup",	6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Buying Group  ", "Enter Start Buying Group, Default = Start Of File, Full Search Available",
		YES, NO, JUSTLEFT, "", "", local_rec.startBuyGroup},
	{1, LIN, "startBuyGroupDesc",	6, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startBuyGroupDesc},
	{1, LIN, "endBuyGroup",	7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Buying Group    ", "Enter End Buying Group, Default = End Of File, Full Search Available",
		YES, NO, JUSTLEFT, "", "", local_rec.endBuyGroup},
	{1, LIN, "endBuyGroupDesc",	7, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endBuyGroupDesc},
	{1, LIN, "qb1",	 9, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		"", "",  "Qty Break 1         ", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr [0]},
	{1, LIN, "qb2",	 10, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 2         ", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr [1]},
	{1, LIN, "qb3",	 11, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 3         ", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr [2]},
	{1, LIN, "qb4",	 12, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 4         ", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr [3]},
	{1, LIN, "qb5",	 13, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 5         ", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr [4]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
int  	spec_valid 			(int);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SrchIngp 			(char *);
int  	heading 			(int);
void 	update 				(void);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{

	envVarCrFind	= atoi (get_env ("CR_FIND"));
	envVarCrCo		= atoi (get_env ("CR_CO"));

	TruePosition	= TRUE;

	SETUP_SCR (vars);

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");
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
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
			
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (envVarCrFind) 
											? "sumr_id_no3" : "sumr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_buy");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ingp);	
	abc_fclose (sumr);	
	abc_fclose (insp);	
	abc_fclose (inmr);	
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int i;
	int	count;

	if (LNCHECK ("qb",2))
	{
		i = atoi (FIELD.label + 2);
		i--;

		/*---------------------------------------------------------------
		| compare value keyed  with prev keyed value - has to be bigger	|
		----------------------------------------------------------------*/
		if (!i)
		{
			if (local_rec.qtybr [i] == 0.00)
			{
				print_mess (ML(mlSkMess121));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (i)	/* do not need to compare local_rec.qtybr [0] */
		{
			if (dflt_used || local_rec.qtybr [i] == 0.00)
			{
				for (count = i; count < 5; count++)
					local_rec.qtybr [count] = 0.00;

				entry_exit = TRUE;
				if (prog_status != ENTRY)
					scn_display (1);
			}

			if (local_rec.qtybr [i] != 0.00 &&
				local_rec.qtybr [i] <= local_rec.qtybr [i -1])
			{
				sprintf (err_str, ML(mlSkMess123), i + 1, i);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (local_rec.qtybr [i - 1] == 0.00 && local_rec.qtybr [i] != 0.00)
			{
				print_mess (ML(mlSkMess122));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("startBuyGroup"))
	{
		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.startBuyGroup, "      ");
			strcpy (ingp_rec.desc, 	   ML ("Start Of File"));
			
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.type, "B");
			strcpy (ingp_rec.code, local_rec.startBuyGroup);

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML(mlStdMess207));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.startBuyGroup, local_rec.endBuyGroup) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.startBuyGroupDesc, ingp_rec.desc);
		DSP_FLD ("startBuyGroupDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endBuyGroup"))
	{
		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.endBuyGroup, "~~~~~~");
			strcpy (ingp_rec.desc, 	   ML ("End Of File"));
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.type, "B");
			strcpy (ingp_rec.code, local_rec.endBuyGroup);

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML(mlStdMess207));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (strcmp (local_rec.startBuyGroup, local_rec.endBuyGroup) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endBuyGroupDesc, ingp_rec.desc);
		DSP_FLD ("endBuyGroupDesc");
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("supplierNo"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		pad_num (sumr_rec.crd_no);
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc) 
		{
			print_mess (ML(mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		strcpy (local_rec.sumrdesc, sumr_rec.crd_name);
		DSP_FLD ("supplierDesc");
		return(0);
	}
	return (EXIT_SUCCESS);
}

void
SrchIngp (
 char *key_val)
{
	_work_open (6,0,40);
	save_rec ("#Code", "#Description ");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "B");
	sprintf (ingp_rec.code, "%-6.6s", key_val);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		if (ingp_rec.type [0] == 'B')
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
	strcpy (ingp_rec.type, "B");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
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

	rv_pr (ML(mlSkMess092), 23, 0, 1);

	box (0, 3, 80, 10);

	line_at (5,1,79);
	line_at (8,1,79);
	line_at (21,0,80);
	line_at (1,0,80);
	print_at (22, 1, ML(mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
update (
 void)
{
	int	count;
	clear ();
	/*---------------------------------------------------
	| for all buying groups from start buying group to	|
	| end buying group keyed update insp.				|
	---------------------------------------------------*/
	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "B");
	strcpy (ingp_rec.code, local_rec.startBuyGroup);

	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");

	/*----------------------------------
	| loop thru' range of keyed codes. |
	----------------------------------*/
	while (!cc && (strcmp (ingp_rec.code, local_rec.endBuyGroup) <= 0))
	{
		if (ingp_rec.type [0] != 'B')
			break;

		/*---------------------------------------------------
		| find all items for this buying group and all the	|
		| insp records for the item/supplier combination	|
		---------------------------------------------------*/
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.buygrp, ingp_rec.code);
		cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
		while (!cc && 
			   !strcmp (inmr_rec.buygrp, ingp_rec.code) &&
			   !strcmp (inmr_rec.co_no, comm_rec.co_no))
		{
			insp_rec.hhsu_hash = sumr_rec.hhsu_hash;
			insp_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (insp, &insp_rec, EQUAL, "u");
			if (!cc)
			{
				for (count = 0; count < 5; count++)
					insp_qty_brk [count] = local_rec.qtybr [count];

				cc = abc_update (insp, &insp_rec);
				if (cc)
					file_err (cc, insp, "DBUPDATE");
			}
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
		}
		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

