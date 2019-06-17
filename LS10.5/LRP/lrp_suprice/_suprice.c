/*=====================================================================
|  Copyright (C) 1986 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( lrp_suprice.c  )                                 |
|  Program Desc  : ( Supplier Price Maintenance.                  )   |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: _suprice.c,v $
| Revision 5.3  2002/07/18 06:43:38  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 09:30:01  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:42  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:52  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/30 05:48:20  scott
| Updated to add app.schema
|
| Revision 3.0  2000/10/10 12:15:41  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:30:58  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 08:58:49  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.13  2000/07/10 02:05:26  scott
| Character deleted from comment ?
|
| Revision 1.12  2000/07/10 01:52:31  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.11  2000/02/14 10:03:17  scott
| S/C LS-ANZ 15941
| S/C LSDI 2464
| Updated to better cater for supplier at Company, Branch and warehouse level.
|
| Revision 1.10  2000/02/07 04:31:51  scott
| Updated as supplier file not being read currectly after search.
|
| Revision 1.9  1999/12/06 01:34:22  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/17 06:40:15  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.7  1999/10/27 07:33:03  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.6  1999/10/11 21:42:44  cam
| Fixed prototypes for heading ()
|
| Revision 1.5  1999/09/29 10:10:52  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/17 07:26:44  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.3  1999/09/16 09:20:47  scott
| Updated from Ansi Project
|
| Revision 1.2  1999/06/15 07:27:06  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _suprice.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_suprice/_suprice.c,v 5.3 2002/07/18 06:43:38 scott Exp $";

#define MAXSCNS 	2
#define MAXWIDTH 	140
#define MAXLINES	500

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_lrp_mess.h>

#define	PERCENT		 (tab_flag [0] == 'Y')

#include	"schema"

struct commRecord	comm_rec;
struct inisRecord	inis_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;


	char	*data	= "data";

	extern	int	TruePosition;
	char	systemDate [11];

	char	tab_flag [2];
	char	branchNumber [2];
	int		envCrCo = 0;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	supp_no [7];
	char	previousSupplier [7];
	char	levelNo [2];
	char	item_no [17];
 	long 	hhbr_hash;
 	long 	effectiveDate;
 	float	percentIncrease;
 	double	oldPrice;
 	double	newPrice;
} local_rec;

static struct	var vars [] =
{
	{1, LIN, "supp_no",	4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier Code                   ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.supp_no},
	{1, LIN, "name",		5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name                   ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "effectiveDate",	6, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, "Effective Date                  ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.effectiveDate},
	{1, LIN, "level",		7, 2, CHARTYPE,
		"U", "          ",
		" ", "C", "C(ompany) B(ranch) W(arehouse)  ", "C(ompany) / B(ranch) / W(arehouse) ",
		YES, NO,  JUSTLEFT, "", "CBW", local_rec.levelNo},
	{2, TAB, "sitem_no",	MAXLINES, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Supplier Item No  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{2, TAB, "item_no",	0,  2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "    Item Number.    ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.item_no},
	{2, TAB, "desc",		0,  1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "             Item Description             ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{2, TAB, "oldPrice",	0,  0, DOUBLETYPE,
		"NNN,NNN,NNN.NN", "          ",
		"", "", "  Old  Price  ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.oldPrice},
	{2, TAB, "hash",		0,  0, LONGTYPE,
		"NNNNNNN", "          ",
		" ", "", " Dummy ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &inmr_rec.hhbr_hash},
	{2, TAB, "percentIncrease",	0,  2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "  Inc %   ", " <Return> for Current Price ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.percentIncrease},
	{2, TAB, "newPrice",	0,  0, DOUBLETYPE,
		"NNN,NNN,NNN.NN", "          ",
		"", " ", "  New  Price  ", " <Return> for Current Price ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.newPrice},
	{2, TAB, "hhis_hash",		0,  0, LONGTYPE,
		"NNNNNNN", "          ",
		" ", "", " Dummy ", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &inis_rec.hhis_hash},
	{0, LIN, "",		0,  0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

#include <FindSumr.h>
/*==========================
| Local Function Prototype |
==========================*/

void 	shutdown_prog	 (void);
void 	InitTables 		 (void);
int 	spec_valid 		 (int);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	Update  		 (void);
void 	SrchInis 		 (char *);
int 	heading 		 (int);

/*=====================================================================
| Main Processing Routine.
=====================================================================*/
int
main (
 int    argc,
 char*  argv [])
{
	if (argc != 2) 
	{
		print_at (0,0, "Usage : %s <With Percent Y|N> \007\n\r",argv [0]);
		return (EXIT_FAILURE);
	}

	TruePosition	=	TRUE;
	SETUP_SCR (vars);

	sprintf (tab_flag,"%-1.1s",argv [1]);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	if (PERCENT)
	{
		vars [label ("percentIncrease")].required = YES;
		vars [label ("newPrice")].required = NA;
	}

	OpenDB ();

	envCrCo = atoi (get_env ("CR_CO"));

	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	strcpy (systemDate, DateToString (TodaysDate ()));

	swide ();
	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;
		abc_unlock (inis);

		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		/*-------------------------------
		| Enter screen 2 Tabular input. |
		-------------------------------*/
		InitTables ();
		heading (2);
		entry (2);

		if (prog_exit || restart)
			continue;

		edit_all ();
		if (restart)
			continue;

		/*-----------------
		| Update records. |
		-----------------*/
		Update ();
	}
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
	snorm ();
}

void
InitTables (void)
{
	int	j;

	lcount [2] = 0;
	scn_set (2);
	for (j = 0; j < MAXLINES; j++)
	{
		sprintf (local_rec.item_no,"%16.16s"," ");
		sprintf (inmr_rec.item_no,"%16.16s"," ");
		sprintf (inmr_rec.description,"%40.40s"," ");
		inmr_rec.hhbr_hash	= 0L;
		local_rec.percentIncrease 		= 0.00;
		local_rec.oldPrice 		= 0.00;
		local_rec.newPrice 		= 0.00;
	}
	scn_set (1);
}

int
spec_valid (
 int    field)
{
	double	ext_val;

	/*---------------------------
	| Validate Supplier Number. |
	---------------------------*/
	if ( LCHECK ("supp_no") )
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.supp_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("name");
	}

	/*--------------------------------
	| Validate Supplier Item Number. |
	--------------------------------*/
	if ( LCHECK ("sitem_no") )
	{
		if (SRCH_KEY)
		{
	 		SrchInis (temp_str);
			return (EXIT_SUCCESS);
		}

		inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (inis_rec.sup_part,local_rec.item_no);
		if (local_rec.levelNo [0] == 'C')
		{
			strcpy (inis_rec.co_no,comm_rec.co_no);
			strcpy (inis_rec.br_no,"  ");
			strcpy (inis_rec.wh_no,"  ");
		}
		else if (local_rec.levelNo [0] == 'B')
		{
			strcpy (inis_rec.co_no,comm_rec.co_no);
			strcpy (inis_rec.br_no,comm_rec.est_no);
			strcpy (inis_rec.wh_no,"  ");
		}
		else if (local_rec.levelNo [0] == 'W')
		{
			strcpy (inis_rec.co_no,comm_rec.co_no);
			strcpy (inis_rec.br_no,comm_rec.est_no);
			strcpy (inis_rec.wh_no,comm_rec.cc_no);
		}
		cc = find_rec (inis,&inis_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		inmr_rec.hhbr_hash	=	inis_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc)
			strcpy (inmr_rec.description, " ");
			
		local_rec.oldPrice = inis_rec.fob_cost;
		DSP_FLD ("item_no");
		DSP_FLD ("desc");
		DSP_FLD ("oldPrice");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Percentage Increase. |
	-------------------------------*/
	if ( LCHECK ("percentIncrease") )
	{
		if (dflt_used)
		{
			local_rec.percentIncrease = 0.00;
			local_rec.newPrice = local_rec.oldPrice;
			DSP_FLD ("percentIncrease");
			DSP_FLD ("newPrice");
			return (EXIT_SUCCESS);
		}

		if (local_rec.percentIncrease != 0.00)
		{
			ext_val = (double)local_rec.percentIncrease;
			ext_val /= 100.00;
			local_rec.newPrice = local_rec.oldPrice * (1.00 + ext_val);
		}
		DSP_FLD ("newPrice");
		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate Nett Price. |
	----------------------*/
	if ( LCHECK ("newPrice") )
	{
		if (dflt_used)
		{
			local_rec.newPrice = local_rec.oldPrice;
			DSP_FLD ("newPrice");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm ( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no4");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (inis);
	abc_dbclose (data);
}

/*=====================
| Update inis record. |
=====================*/
void
Update  (void)
{
	scn_set (2);
	abc_selfield (inis,"inis_hhis_hash");

	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);
		cc = find_rec (inis, &inis_rec, COMPARISON, "u");
		if (!cc)
		{
			inis_rec.fob_cost = local_rec.newPrice;
			inis_rec.lcost_date = local_rec.effectiveDate;
			cc = abc_update (inis,&inis_rec);
			if (cc)
				file_err (cc, "inis", "DBUPDATE");
		}
		else
			abc_unlock (inis);
	}
	strcpy (local_rec.previousSupplier,sumr_rec.crd_no);
	abc_unlock (inis);
	abc_selfield (inis,"inis_id_no4");
	scn_set (1);
}


/*==================================
| Search for Supplier Item         |
==================================*/
void
SrchInis (
 char*  key_val)
{
	work_open ();
	save_rec ("#Item No","#Price - Item Description");
	if (local_rec.levelNo [0] == 'C')
	{
		strcpy (inis_rec.co_no,comm_rec.co_no);
		strcpy (inis_rec.br_no,"  ");
		strcpy (inis_rec.wh_no,"  ");
	}
	else if (local_rec.levelNo [0] == 'B')
	{
		strcpy (inis_rec.co_no,comm_rec.co_no);
		strcpy (inis_rec.br_no,comm_rec.est_no);
		strcpy (inis_rec.wh_no,"  ");
	}
	else if (local_rec.levelNo [0] == 'W')
	{
		strcpy (inis_rec.co_no,comm_rec.co_no);
		strcpy (inis_rec.br_no,comm_rec.est_no);
		strcpy (inis_rec.wh_no,comm_rec.cc_no);
	}
	sprintf (inis_rec.sup_part,"%-16.16s",key_val);
	inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
	cc = find_rec (inis,&inis_rec,GTEQ,"r");
	while (!cc && !strcmp (inis_rec.co_no,comm_rec.co_no) && 
		      	  inis_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		      	  !strncmp (inis_rec.sup_part,key_val,strlen (key_val)))
	{
		if (local_rec.levelNo [0] == 'C' && inis_rec.sup_priority [0] != 'C')
		{
			cc = find_rec (inis,&inis_rec,NEXT,"r");
			continue;
		}
		if (local_rec.levelNo [0] == 'B' && inis_rec.sup_priority [0] != 'B')
		{
			cc = find_rec (inis,&inis_rec,NEXT,"r");
			continue;
		}
		if (local_rec.levelNo [0] == 'W' && inis_rec.sup_priority [0] != 'W')
		{
			cc = find_rec (inis,&inis_rec,NEXT,"r");
			continue;
		}
			
		inmr_rec.hhbr_hash	=	inis_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			sprintf (err_str,"%70.70s"," ");
		else
			sprintf (err_str,"%10.2f - %s", inis_rec.fob_cost,
					clip (inmr_rec.description));

		cc = save_rec (inis_rec.sup_part, err_str);
		if (cc)
			break;

		cc = find_rec (inis,&inis_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	if (local_rec.levelNo [0] == 'C')
	{
		strcpy (inis_rec.co_no,comm_rec.co_no);
		strcpy (inis_rec.br_no,"  ");
		strcpy (inis_rec.wh_no,"  ");
	}
	else if (local_rec.levelNo [0] == 'B')
	{
		strcpy (inis_rec.co_no,comm_rec.co_no);
		strcpy (inis_rec.br_no,comm_rec.est_no);
		strcpy (inis_rec.wh_no,"  ");
	}
	else if (local_rec.levelNo [0] == 'W')
	{
		strcpy (inis_rec.co_no,comm_rec.co_no);
		strcpy (inis_rec.br_no,comm_rec.est_no);
		strcpy (inis_rec.wh_no,comm_rec.cc_no);
	}
	inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (inis_rec.sup_part,"%-16.16s",temp_str);
	cc = find_rec (inis,&inis_rec,GTEQ,"r");
	if (cc)
		file_err (cc, "inis", "DBFIND");
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		
		switch (scn)
		{
		case	1:
			rv_pr (ML (mlLrpMess023),53,0,1);
			print_at (0,108,ML (mlLrpMess022),local_rec.previousSupplier);
			box (0,3,132,4);
			break;
		case	2:
			print_at (2,2,ML(mlStdMess063),local_rec.supp_no,sumr_rec.crd_name);
			rv_pr (ML (mlLrpMess024),54,0,1);
			break;
		}

		move (0,1);
		line (132);

		move (0,19);
		line (132);

		strcpy (err_str,ML (mlStdMess038));
		print_at (20,1,err_str,comm_rec.co_no,comm_rec.co_name);

		strcpy (err_str,ML (mlStdMess039));
		print_at (21,1,err_str,comm_rec.est_no,comm_rec.est_name);
		move (0,22);
		line (132);
		move (1,input_row);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

	return (EXIT_SUCCESS);
}
