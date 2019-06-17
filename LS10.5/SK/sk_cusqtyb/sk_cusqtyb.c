/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_cusqtyb.c  )                                  |
|  Program Desc  : ( Customer Qty Break Update                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ingp, excf, cuqb,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cuqb,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 14/10/93         |
|---------------------------------------------------------------------|
|  Date Modified : (xx/xx/xx)      | Modified  by :                   |
|  (xx/xx/xx)    :                                                    |
| $Log: sk_cusqtyb.c,v $
| Revision 5.2  2001/08/09 09:18:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:27  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:36:56  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:01  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:37  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  2000/01/18 23:52:24  cam
| Changes for GVision compatibility.  Add conditional compile code in spec_valid
|
| Revision 1.10  1999/11/11 05:59:36  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.9  1999/11/03 07:31:56  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.8  1999/10/13 02:41:55  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.7  1999/10/12 21:20:31  scott
| Updated by Gerry from ansi project.
|
| Revision 1.6  1999/10/08 05:32:19  scott
| First Pass checkin by Scott.
|
| Revision 1.5  1999/06/20 05:19:54  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
char	*PNAME = "$RCSfile: sk_cusqtyb.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_cusqtyb/sk_cusqtyb.c,v 5.2 2001/08/09 09:18:23 scott Exp $";

#define	CCMAIN
#define	MOD		5
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"},
		{"comm_price6_desc"},
		{"comm_price7_desc"},
		{"comm_price8_desc"},
		{"comm_price9_desc"},
	};

	int	comm_no_fields = 13;

	struct	{
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	price_type[9][16];
	} comm_rec;

	/*=====================================
	| Inventory Buying and Selling Groups |
	=====================================*/
	struct dbview ingp_list [] =
	{
		{"ingp_co_no"},
		{"ingp_code"},
		{"ingp_desc"},
		{"ingp_type"},
		{"ingp_sell_reg_pc"}
	};

	int	ingp_no_fields = 5;

	struct tag_ingpRecord
	{
		char	co_no [3];
		char	code [7];
		char	desc [41];
		char	type [2];
		float	sell_reg_pc;
	} ingp_rec;

	/*======================================
	| Customer Quantity Break Default file |
	======================================*/
	struct dbview cuqb_list [] =
	{
		{"cuqb_co_no"},
		{"cuqb_price_type"},
		{"cuqb_buygrp"},
		{"cuqb_sellgrp"},
		{"cuqb_category"},
		{"cuqb_qty_brk1"},
		{"cuqb_qty_brk2"},
		{"cuqb_qty_brk3"},
		{"cuqb_qty_brk4"},
		{"cuqb_qty_brk5"},
		{"cuqb_qty_brk6"},
		{"cuqb_qty_brk7"},
		{"cuqb_qty_brk8"},
		{"cuqb_qty_brk9"},
		{"cuqb_update_flag"}
	};

	int	cuqb_no_fields = 15;

	struct tag_cuqbRecord
	{
		char	co_no [3];
		int		price_type;
		char	buygrp [7];
		char	sellgrp [7];
		char	category [12];
		float	qtybr[9];
		char	update_flag [3];
	} cuqb_rec;

	/*===============================
	| External Category File Record |
	===============================*/
	struct dbview excf_list [] =
	{
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
	};

	int	excf_no_fields = 3;

	struct tag_excfRecord
	{
		char	co_no [3];
		char	cat_no [12];
		char	desc [41];
	} excf_rec;

	char	*comm   = "comm",
			*data   = "data",
			*excf	= "excf",
			*cuqb	= "cuqb",
			*ingp   = "ingp";

	int		numOfBreaks;
	int		numOfPrices;

struct
{
	char	range[2];
	char	rangedesc[14];
	char	scat[12];
	char	ecat[12];
	char	scatdesc[41];
	char	ecatdesc[41];
	char	ssellgrp[7];
	char	sselldesc[41];
	char	esellgrp[7];
	char	eselldesc[41];
	char	sbuygrp[7];
	char	sbuydesc[41];
	char	ebuygrp[7];
	char	ebuydesc[41];
	float	qtybr[9];
	int		price_type;
	char	price_desc[16];
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "range",	4, 24, CHARTYPE,
		"U", "          ",
		" ", "S",  "Range Type          :", "Enter Range Type B)uying S)elling C)ategory, Default = S)elling",
		NE, NO, JUSTLEFT, "BSC", "", local_rec.range},
	{1, LIN, "rangedesc",	4, 34, CHARTYPE,
		"AAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rangedesc},
	{1, LIN, "price",	5, 24, INTTYPE,
		"N", "          ",
		"", "",  "Price Type          :", "Enter Price Type To Update. ",
		YES, NO, JUSTLEFT, "1", "9", (char *) &local_rec.price_type},
	{1, LIN, "price_desc",5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		"", "",  "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.price_desc},
	{1, LIN, "sbuygrp",	7, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Buying Group  :", "Enter Start Buying Group, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.sbuygrp},
	{1, LIN, "sbuydesc",	7, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sbuydesc},
	{1, LIN, "ebuygrp",	8, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Buying Group    :", "Enter End Buying Group, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ebuygrp},
	{1, LIN, "ebuydesc",	8, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ebuydesc},
	{1, LIN, "ssellgrp",	7, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Selling Group :", "Enter Start Selling Group, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ssellgrp},
	{1, LIN, "sselldesc",	7, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sselldesc},
	{1, LIN, "esellgrp",	8, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Selling Group   :", "Enter End Selling Group, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.esellgrp},
	{1, LIN, "eselldesc",	8, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.eselldesc},
	{1, LIN, "scat",	7, 24, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ",  "Start Category      :", "Enter Start Category, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.scat},
	{1, LIN, "scatdesc",	7, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.scatdesc},
	{1, LIN, "ecat",	8, 24, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ",  "End Category        :", "Enter End Category, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ecat},
	{1, LIN, "ecatdesc",	8, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ecatdesc},
	{1, LIN, "qb1",	 10, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		"", "",  "Qty Break 1         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[0]},
	{1, LIN, "qb2",	 11, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 2         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[1]},
	{1, LIN, "qb3",	 12, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 3         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[2]},
	{1, LIN, "qb4",	 13, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 4         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[3]},
	{1, LIN, "qb5",	 14, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 5         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[4]},
	{1, LIN, "qb6",	 15, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 6         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[5]},
	{1, LIN, "qb7",	 16, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 7         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[6]},
	{1, LIN, "qb8",	 17, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 8         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[7]},
	{1, LIN, "qb9",	 18, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ",  "Qty Break 9         :", "Enter Quantity Break ",
		 YES, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qtybr[8]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
void srch_ingp (char *key_val);
int  heading (int scn);
void update (void);
void shutdown_prog (void);
void srch_excf (char *key_val);
void proc_sell (void);
void proc_buy (void);
void proc_cat (void);
void UpdCuqb (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv[])
{

	char	tmpStr[4];
	char	*sptr = chk_env ("SK_DBQTYNUM");
	int		count;

	if (sptr)
	{
		numOfBreaks = atoi (sptr);
		if (numOfBreaks > 9 || numOfBreaks < 0)
			numOfBreaks = 9;
	}
	else
		numOfBreaks = 0;

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr)
	{
		numOfPrices = atoi (sptr);
		if (numOfPrices > 9 || numOfPrices < 1)
			numOfPrices = 9;
	}
	else
		numOfPrices = 5;

	SETUP_SCR (vars);
	init_scr ();
	set_tty (); 

	if (!numOfBreaks)
	{
		clear ();
		sprintf (err_str, "Environment Not Set Up For Quantity Breaks - Press Any Key\007"); 
		rv_pr (err_str, 10, (80 - (strlen (clip (err_str)))) / 2, 1); 
		PauseForKey (0, 0, "", 0);
		clear ();
		return (EXIT_FAILURE);
	}

	for (count = numOfBreaks + 1; count <= 9; count++)
	{
		sprintf (tmpStr, "qb%d", count);
		FLD (tmpStr) = ND;
	}
		
	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();
	
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
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec (ingp, ingp_list, ingp_no_fields, "ingp_id_no2");
	open_rec (cuqb, cuqb_list, cuqb_no_fields, "cuqb_id_sellgrp");
	open_rec (excf, excf_list, excf_no_fields, "excf_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ingp);	
	abc_fclose (cuqb);	
	abc_fclose (excf);	
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int i;
	int	count;

	if (LCHECK ("price"))
	{
		if (SRCH_KEY)
			return (EXIT_FAILURE);

		if (local_rec.price_type > numOfPrices)
		{
			sprintf (err_str, "Only %d Prices Set Up \007", numOfPrices);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.price_desc, 
				comm_rec.price_type [local_rec.price_type - 1]);
		DSP_FLD ("price_desc");
		return (EXIT_SUCCESS);
	}

	if (LNCHECK ("qb",2))
	{
		i = atoi (FIELD.label + 2);
		i--;

		if (!i && local_rec.qtybr[i] == 0.00)
		{
			print_mess ("The First Quantity Break May Not Be Zero");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*-----------------------
		| if dflt used or zero
		| entered then set all
		| below to zero as well
		----------------------*/
		if (dflt_used || local_rec.qtybr[i] == 0.00)
		{
			for (count = i; count < numOfBreaks; count++)
				local_rec.qtybr[count] = 0.00;

			entry_exit = TRUE;
			if (prog_status != ENTRY)
				scn_display (1);
		}

		/*--------------------
		| compare value keyed   
		| with prev keyed value
		| - has to be bigger
		---------------------*/
		if (i && local_rec.qtybr[i] != 0.00 &&
			local_rec.qtybr[i] <= local_rec.qtybr[i -1])
		{
			sprintf (err_str,
					 "Qty Break%d Should Be Greater Than Qty Break%d \007",
					 i + 1,
					 i
					);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (i && local_rec.qtybr[i - 1] == 0.00 && local_rec.qtybr[i] != 0.00)
		{
			print_mess ("As Previously Entered Quantity Was Zero, This Quantity Should Be Zero\007");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| buying group
	---------------------------*/
	if (LCHECK ("sbuygrp"))
	{
		if (FLD ("sbuygrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_ingp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.sbuygrp, "      ");
			strcpy (ingp_rec.desc, "Start Of File");
			
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.tco_no);
			strcpy (ingp_rec.code, local_rec.sbuygrp);
			strcpy (ingp_rec.type, "B");

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			
			if (!cc)
			{
				if (ingp_rec.type[0] == 'S')
				{
					sprintf (err_str, "This Code Is Assigned To A Selling Group ");
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (cc)
			{
				print_mess ("Buying Group Not On File \007");
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.sbuygrp, local_rec.ebuygrp) > 0)
			{
				sprintf (err_str, 
						"Start Group %s Can Not Be Greater Than End Group %s\007", 
						local_rec.sbuygrp,
						local_rec.ebuygrp
						);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.sbuydesc, ingp_rec.desc);
		DSP_FLD ("sbuydesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ebuygrp"))
	{
		if (FLD ("ebuygrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_ingp (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.ebuygrp, "~~~~~~");
			strcpy (ingp_rec.desc, "End Of File");
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.tco_no);
			strcpy (ingp_rec.code, local_rec.ebuygrp);
			strcpy (ingp_rec.type, "B");

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			
			if (!cc)
			{
				if (ingp_rec.type[0] == 'S')
				{
					sprintf (err_str, "This Code Is Assigned To A Selling Group ");
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (cc)
			{
				print_mess ("Buying Group Not On File \007");
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (strcmp (local_rec.sbuygrp, local_rec.ebuygrp) > 0)
		{
			sprintf (err_str, 
					"End Group %s Can Not Be Less Than Start Group %s\007", 
					local_rec.ebuygrp,
					local_rec.sbuygrp
					);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ebuydesc, ingp_rec.desc);
		DSP_FLD ("ebuydesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| selling group
	---------------------------*/
	if (LCHECK ("ssellgrp"))
	{
		if (FLD ("ssellgrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_ingp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.ssellgrp, "      ");
			strcpy (ingp_rec.desc, "Start Of File");
			
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.tco_no);
			strcpy (ingp_rec.code, local_rec.ssellgrp);
			strcpy (ingp_rec.type, "S");

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			
			if (!cc)
			{
				if (ingp_rec.type[0] != 'S')
				{
					sprintf (err_str, "This Code Is Assigned To A Buying Group ");
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (cc)
			{
				print_mess ("Selling Group Not On File \007");
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.ssellgrp, local_rec.esellgrp) > 0)
			{
				sprintf (err_str, 
						"Start Group %s Can Not Be Greater Than End Group %s\007", 
						local_rec.ssellgrp,
						local_rec.esellgrp
						);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.sselldesc, ingp_rec.desc);
		DSP_FLD ("sselldesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("esellgrp"))
	{
		if (FLD ("esellgrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_ingp (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.esellgrp, "~~~~~~");
			strcpy (ingp_rec.desc, "End Of File");
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.tco_no);
			strcpy (ingp_rec.code, local_rec.esellgrp);
			strcpy (ingp_rec.type, "S");

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			
			if (!cc)
			{
				if (ingp_rec.type[0] != 'S')
				{
					sprintf (err_str, "This Code Is Assigned To A Buying Group ");
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (cc)
			{
				print_mess ("Selling Group Not On File \007");
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (strcmp (local_rec.ssellgrp, local_rec.esellgrp) > 0)
		{
			sprintf (err_str, 
					"End Group %s Can Not Be Less Than Start Group %s\007", 
					local_rec.esellgrp,
					local_rec.ssellgrp
					);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.eselldesc, ingp_rec.desc);
		DSP_FLD ("eselldesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| category 
	---------------------------*/
	if (LCHECK ("scat"))
	{
		if (FLD ("scat") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_excf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.scat, "           ");
			strcpy (excf_rec.desc, "Start Of File");
			
		}
		else
		{
			strcpy (excf_rec.co_no, comm_rec.tco_no);
			strcpy (excf_rec.cat_no, local_rec.scat);

			cc = find_rec (excf, &excf_rec, EQUAL, "r");
			
			if (cc)
			{
				print_mess ("Category Not On File \007");
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.scat, local_rec.ecat) > 0)
			{
				sprintf (err_str, 
						"Start Category %s Can Not Be Greater Than End Category %s\007", 
						local_rec.scat,
						local_rec.ecat
						);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.scatdesc, excf_rec.desc);
		DSP_FLD ("scatdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ecat"))
	{
		if (FLD ("ecat") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			srch_excf (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.ecat, "~~~~~~~~~~~");
			strcpy (excf_rec.desc, "End Of File");
		}
		else
		{
			strcpy (excf_rec.co_no, comm_rec.tco_no);
			strcpy (excf_rec.cat_no, local_rec.ecat);

			cc = find_rec (excf, &excf_rec, EQUAL, "r");
			
			if (cc)
			{
				print_mess ("Category Not On File \007");
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (strcmp (local_rec.scat, local_rec.ecat) > 0)
		{
			sprintf (err_str, 
					"End Cateogry %s Can Not Be Less Than Start Cateogry %s\007", 
					local_rec.ecat,
					local_rec.scat
					);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ecatdesc, excf_rec.desc);
		DSP_FLD ("ecatdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("range"))
	{
		FLD ("sbuygrp")   = ND;
		FLD ("ebuygrp")   = ND;
		FLD ("sbuydesc")  = ND;
		FLD ("ebuydesc")  = ND;
		FLD ("ssellgrp")  = ND;
		FLD ("esellgrp")  = ND;
		FLD ("sselldesc") = ND;
		FLD ("eselldesc") = ND;
		FLD ("scat")      = ND;
		FLD ("scatdesc")  = ND;
		FLD ("ecat")      = ND;
		FLD ("ecatdesc")  = ND;

#ifndef GVISION
		print_at (7, 2, "%70.70s", " ");
		print_at (8, 2, "%70.70s", " ");
#endif	/* GVISION */

		/*
		if (dflt_used)
			local_rec.range[0] = 'S';
		*/
		/*-------------------------
		| if choice is buying group
		--------------------------*/
		if (local_rec.range[0] == 'B')
		{
			FLD ("sbuygrp")  = YES;
			FLD ("ebuygrp")  = YES;
			FLD ("sbuydesc") = NA;
			FLD ("ebuydesc") = NA;
			strcpy (local_rec.rangedesc, "Buying Group");
			scn_write (1);
			DSP_FLD ("rangedesc");
		}

		/*-------------------------
		| if choice is selling group
		--------------------------*/
		if (local_rec.range[0] == 'S')
		{
			FLD ("ssellgrp")  = YES;
			FLD ("esellgrp")  = YES;
			FLD ("sselldesc") = NA;
			FLD ("eselldesc") = NA;
			strcpy (local_rec.rangedesc, "Selling Group");
			scn_write (1);
			DSP_FLD ("rangedesc");
		}

		/*-------------------------
		| if choice is category
		--------------------------*/
		if (local_rec.range[0] == 'C')
		{
			FLD ("scat")  = YES;
			FLD ("ecat")  = YES;
			FLD ("scatdesc")  = NA;
			FLD ("ecatdesc")  = NA;
			strcpy (local_rec.rangedesc, "Category");
			scn_write (1);
			DSP_FLD ("rangedesc");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
srch_ingp (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description ");

	if (local_rec.range[0] == 'S')
		strcpy (ingp_rec.type, "S");
	else
		strcpy (ingp_rec.type, "B");

	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	sprintf (ingp_rec.code, "%-6.6s", key_val);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.tco_no) &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		if (ingp_rec.type[0] == 'S' && local_rec.range[0] == 'S')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (ingp_rec.type[0] == 'B' && local_rec.range[0] == 'B')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	if (local_rec.range[0] == 'S')
		strcpy (ingp_rec.type, "S");
	else
		strcpy (ingp_rec.type, "B");
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
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

	sprintf (err_str,   " Customer Pricing Mass Quantity Break SetUp ");

	rv_pr (err_str, (80 - (strlen (clip(err_str)))) / 2, 0, 1);

	box (0, 3, 80, numOfBreaks + 6);

	move (1, 6);
	line (79);
	move (1, 9);
	line (79);
	move (0, 21);
	line (80);

	print_at (22, 1, "Co : %s - %s", comm_rec.tco_no, comm_rec.tco_name);
	move (0,1);
	line (80);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
update (
 void)
{
	if (local_rec.range[0] == 'S')
	{
		dsp_screen ("Updating Selling Groups", 
					 comm_rec.tco_no, 
					 comm_rec.tco_name);
		proc_sell ();
		return;
	}
	if (local_rec.range[0] == 'B')
	{
		dsp_screen ("Updating Buying Groups", 
					 comm_rec.tco_no, 
					 comm_rec.tco_name);
		proc_buy ();
		return;
	}
	if (local_rec.range[0] == 'C')
	{
		dsp_screen ("Updating Categories", 
					 comm_rec.tco_no, 
					 comm_rec.tco_name);
		proc_cat ();
		return;
	}
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
srch_excf (
 char *key_val)
{
	work_open ();
	save_rec ("#Category No.    ", "#Category Description    ");

	strcpy (excf_rec.co_no, comm_rec.tco_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");

	while (!cc && !strcmp (excf_rec.co_no, comm_rec.tco_no) && 
		  !strncmp (excf_rec.cat_no, key_val, strlen (key_val)))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excf_rec.co_no, comm_rec.tco_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

void
proc_sell (
 void)
{
	/* set index */
	abc_selfield (cuqb, "cuqb_id_sellgrp");

	/*-------------------
	| read thru' all sell
	| groups and create
	| or update cuqbs
	--------------------*/
	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	strcpy (ingp_rec.type, "S");
	strcpy (ingp_rec.code, local_rec.ssellgrp);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (ingp_rec.co_no, comm_rec.tco_no) &&
		   ingp_rec.type[0] == 'S' &&
		   (strcmp (ingp_rec.code, local_rec.esellgrp) < 1)
		 )
		 {
			/*------------------
			| clean up record &&
			| re-initialise
			-------------------*/
			memset (&cuqb_rec, 0, sizeof (cuqb_rec));
			strcpy (cuqb_rec.sellgrp, ingp_rec.code);

			dsp_process ("Selling Groups", ingp_rec.code);
			UpdCuqb ();

			cc = find_rec (ingp, &ingp_rec, NEXT, "r");
		 }
}

void
proc_buy (
 void)
{
	/* set index */
	abc_selfield (cuqb, "cuqb_id_buygrp");

	/*-------------------
	| read thru' all buy
	| groups and create
	| or update cuqbs
	--------------------*/
	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	strcpy (ingp_rec.type, "B");
	strcpy (ingp_rec.code, local_rec.sbuygrp);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (ingp_rec.co_no, comm_rec.tco_no) &&
		   ingp_rec.type[0] == 'B' &&
		   (strcmp (ingp_rec.code, local_rec.ebuygrp) < 1)
		 )
		 {
			/*------------------
			| clean up record &&
			| re-initialise
			-------------------*/
			memset (&cuqb_rec, 0, sizeof (cuqb_rec));
			strcpy (cuqb_rec.buygrp, ingp_rec.code);

			dsp_process ("Buying Groups", ingp_rec.code);
			UpdCuqb ();

			cc = find_rec (ingp, &ingp_rec, NEXT, "r");
		 }
}

void
proc_cat (
 void)
{
	/* set index */
	abc_selfield (cuqb, "cuqb_id_cat");

	/*-------------------
	| read thru' all cats
	| groups and create
	| or update cuqbs
	--------------------*/
	strcpy (excf_rec.co_no, comm_rec.tco_no);
	strcpy (excf_rec.cat_no, local_rec.scat);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (excf_rec.co_no, comm_rec.tco_no) &&
		   (strcmp (excf_rec.cat_no, local_rec.ecat) < 1)
		 )
		 {
			/*------------------
			| clean up record &&
			| re-initialise
			-------------------*/
			memset (&cuqb_rec, 0, sizeof (cuqb_rec));
			strcpy (cuqb_rec.category, excf_rec.cat_no);

			dsp_process ("Category", excf_rec.cat_no);
			UpdCuqb ();

			cc = find_rec (excf, &excf_rec, NEXT, "r");
		 }
}

void
UpdCuqb (
 void)
{
	int	count;

	/*---------------------
	| indexes and parts of
	| indexes and initilised
	| in the calling functions
	-------------------------*/
	strcpy (cuqb_rec.co_no, comm_rec.tco_no);
	cuqb_rec.price_type = local_rec.price_type;
	cc = find_rec (cuqb, &cuqb_rec, EQUAL, "u");

	/*--------------------------
	| fill up the qty breaks up
	| to numOfBreaks.
	--------------------------*/

	for (count = 0; count < numOfBreaks; count++)
		cuqb_rec.qtybr[count] = local_rec.qtybr[count];

	/*-------------------------
	| set the others to zero
	-------------------------*/
	for (; count < 9; count++)
		cuqb_rec.qtybr[count] = 0.00;

	/*------------------------
	| set or reset update flag
	--------------------------*/
	strcpy (cuqb_rec.update_flag, "N");

	/*-------------------------
	| if read (cc) failed then
	| add else update
	--------------------------*/
	if (cc)
	{
		cc = abc_add (cuqb, &cuqb_rec);
		if (cc)
			file_err (cc, cuqb, "DBADD");
		return;
	}
	else
	{
		cc = abc_update (cuqb, &cuqb_rec);
		if (cc)
			file_err (cc, cuqb, "DBUPDATE");
		return;
	}
}
