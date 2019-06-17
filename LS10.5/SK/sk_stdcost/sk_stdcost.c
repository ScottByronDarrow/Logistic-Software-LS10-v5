/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_stdcost.c,v 5.5 2002/11/22 08:36:16 kaarlo Exp $
|  Program Name  : (sk_stdcost.c) 
|  Program Desc  : (Standard Cost Maintenance)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 13/09/91         |
|---------------------------------------------------------------------|
| $Log: sk_stdcost.c,v $
| Revision 5.5  2002/11/22 08:36:16  kaarlo
| .
|
| Revision 5.4  2002/11/22 08:31:56  kaarlo
| LS01127 SC4184. Updated to add validation on start and end class.
|
| Revision 5.3  2001/08/09 09:20:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:59  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:35  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_stdcost.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_stdcost/sk_stdcost.c,v 5.5 2002/11/22 08:36:16 kaarlo Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	BY_ITEM		 (local_rec.range_type[0] == 'I')
#define	BY_LAST		 (local_rec.by_last[0] == 'Y')

#include	"schema"

struct commRecord	comm_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;

/*-----------------------------
| Local and screen structure  |
-----------------------------*/
struct {
	char	systemDate[11];
	long	lsystemDate;
	char	st_item[17];
	char	st_item_desc[41];
	char	end_item[17];
	char	end_item_desc[41];
	char	st_class[2];
	char	st_cat[12];
	char	st_cat_desc[41];
	char	end_class[2];
	char	end_cat[12];
	char	end_cat_desc[41];
	char	range_type[2];
	char	back[2];
	char	back_desc[4];
	char	onight[2];
	char	onight_desc[4];
	char	dummy[11];
	char	by_last[2];
	char	by_last_desc[4];
	float	percent;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "st_item",	 3, 12, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Start Item:", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.st_item},
	{1, LIN, "st_item_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_item_desc},
	{1, LIN, "end_item",	 4, 12, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " End Item:", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.end_item},
	{1, LIN, "end_item_desc", 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_item_desc},

	{1, LIN, "st_class",	 3, 15, CHARTYPE,
		"U", "          ",
		" ", "A", " Start Class:", "Input Start Class A-Z.",
		ND, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.st_class},
	{1, LIN, "st_cat",	 4, 15, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", " Start Category:", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_cat},
	{1, LIN, "st_cat_desc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_cat_desc},
	{1, LIN, "end_class",	 6, 15, CHARTYPE,
		"U", "          ",
		" ", "A", " End Class:", "Input End Class A-Z.",
		ND, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.end_class},
	{1, LIN, "end_cat",	 7, 15, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", " End Category:", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_cat},
	{1, LIN, "end_cat_desc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_cat_desc},

	{1, LIN, "by_last",	 6, 18, CHARTYPE,
		"U", "          ",
		" ", "N", " Base On Last Cost:", " Update standard cost based on last cost ",
		NO, NO,  JUSTLEFT, "YN", "", local_rec.by_last},
	{1, LIN, "by_last_desc",	 6, 21, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.by_last_desc},
	{1, LIN, "percent",	 7, 18, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", "", " Percent Increase:", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.percent},
	{1, LIN, "back",	 9, 12, CHARTYPE,
		"U", "          ",
		" ", "N", " Background:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.back},
	{1, LIN, "back_desc",	 9, 15, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onight",10, 12, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.onight},
	{1, LIN, "onight_desc",	 10, 15, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.onight_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	RunProgram 		(char *, char *);
int  	heading 		(int);
int  	spec_valid 		(int);
void 	SrchExcf 		(char *);
void 	Process 		(void);
int  	UpdateItem 		(int);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv[])
{

	if (argc != 3 && argc != 6)
	{
		print_at (0,0,mlSkMess090, argv[0]);
		print_at (0,0,mlSkMess091, argv[0]);
		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	if (argc == 3)
	{
		sprintf (local_rec.range_type, "%-1.1s", argv[2]);

		OpenDB ();

		SETUP_SCR (vars);

		if (!BY_ITEM)
		{
			FLD ("st_item") 		= ND;
			FLD ("st_item_desc") 	= ND;
			FLD ("end_item") 		= ND;
			FLD ("end_item_desc") 	= ND;
	
			vars [label ("by_last")].row 		= 9;
			vars [label ("by_last_desc")].row 	= 9;
			vars [label ("percent")].row 		= 10;
			vars [label ("back")].row 			= 12;
			vars [label ("back_desc")].row 		= 12;
			vars [label ("onight")].row 		= 13;
			vars [label ("onight_desc")].row 	= 13;
	
			FLD ("st_class") 		= YES;
			FLD ("end_class") 		= YES;
			FLD ("st_cat") 			= NO;
			FLD ("st_cat_desc") 	= NA;
			FLD ("end_cat") 		= NO;
			FLD ("end_cat_desc") 	= NA;
		}
	
		init_scr ();
		clear ();
		set_tty ();
		set_masks ();
	
		while (prog_exit == 0) 
		{
			/*---------------------
			| Reset Control flags |
			---------------------*/
			entry_exit 	= FALSE;
			edit_exit 	= FALSE;
			prog_exit 	= FALSE;
			restart 	= FALSE;
			init_ok 	= TRUE;
			search_ok 	= TRUE;
	
			/*---------------------
			| Entry Screen Input  |
			---------------------*/
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;

			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;
	
			if (local_rec.percent != 0.00)
				RunProgram (argv[0], argv[1]);

			prog_exit = 1;
        } /* while () */
        shutdown_prog ();
	}
	else
	{
		sprintf (local_rec.range_type, "%-1.1s", argv[1]);
		sprintf (local_rec.by_last,    "%-1.1s", argv[4]);
		local_rec.percent = (float) (atof (argv[5]));
		local_rec.percent = (float) (local_rec.percent / 100.00);

		OpenDB ();

		if (BY_ITEM)
		{
			sprintf (local_rec.st_item, "%-16.16s", argv[2]);
			sprintf (local_rec.end_item, "%-16.16s", argv[3]);
		}
		else
		{
			sprintf (local_rec.st_class,  "%-1.1s",   argv[2]);
			sprintf (local_rec.st_cat,    "%-11.11s", argv[2] + 1);
			sprintf (local_rec.end_class, "%-1.1s",   argv[3]);
			sprintf (local_rec.end_cat,   "%-11.11s", argv[3] + 1);
		}

		Process ();
		shutdown_prog ();
	}

    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, (BY_ITEM) ? "inmr_id_no" :
								"inmr_id_no_3");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inei);
	abc_fclose (excf);
	abc_fclose (inmr);
	SearchFindClose ();
	abc_dbclose ("data");
}

void
RunProgram (
	char	*prog_name, 
	char	*prog_desc)
{
	char	tmp_lower[17];
	char	tmp_upper[17];
	char	tmp_percent[8];

	CloseDB (); FinishProgram ();;
	rset_tty ();

	sprintf (tmp_percent, "%7.2f", local_rec.percent);

	if (BY_ITEM)
	{
		sprintf (tmp_lower, "%-16.16s", local_rec.st_item);
		sprintf (tmp_upper, "%-16.16s", local_rec.end_item);
	}
	else
	{
		sprintf (tmp_lower, 
			"%-1.1s%-11.11s", 
			local_rec.st_class,
			local_rec.st_cat);

		sprintf (tmp_upper, 
			"%-1.1s%-11.11s", 
			local_rec.end_class,
			local_rec.end_cat);
	}

	if (local_rec.onight[0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.range_type,
				tmp_lower,
				tmp_upper,
				local_rec.by_last,
				tmp_percent,
				prog_desc, (char *)0);
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				local_rec.range_type,
				tmp_lower,
				tmp_upper, 
				local_rec.by_last,
				tmp_percent, (char *)0);
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			local_rec.range_type,
			tmp_lower,
			tmp_upper, 
			local_rec.by_last,
			tmp_percent, (char *)0);
	}
	return;
}

int
heading (
 int scn)
{

	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		rv_pr (ML (mlSkMess124),30, 0, 1);

		line_at (1,0,80);

		if (BY_ITEM)
		{
			box (0, 2, 80, 8);
			line_at (5,1,79);
			line_at (8,1,79);
		}
		else
		{
			box (0, 2, 80, 11);
			line_at (5,1,79);
			line_at (8,1,79);
			line_at (11,1,79);
		}

		line_at (20,0,80);
		print_at (21,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name); 
		print_at (22,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("by_last"))
	{
		if (local_rec.by_last[0] == 'Y')
			strcpy (local_rec.by_last_desc, "Yes");
		else
			strcpy (local_rec.by_last_desc, "No ");

		DSP_FLD ("by_last_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back[0] == 'Y')
			strcpy (local_rec.back_desc, "Yes");
		else
			strcpy (local_rec.back_desc, "No ");

		DSP_FLD ("back_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight[0] == 'Y')
			strcpy (local_rec.onight_desc, "Yes");
		else
			strcpy (local_rec.onight_desc, "No ");

		DSP_FLD ("onight_desc");
		return (EXIT_SUCCESS);
	}

        if (LCHECK ("st_item"))
        {
		if (FLD ("st_item") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.st_item, "%-16.16s", " ");
			sprintf (local_rec.st_item_desc, 
				"%-40.40s", 
				"First Item");

			DSP_FLD ("st_item");
			DSP_FLD ("st_item_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
		   	return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.st_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.st_item);
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

		if (strcmp (inmr_rec.source, "RM"))
		{
			print_mess (ML (mlSkMess633));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.st_item_desc,inmr_rec.description);
		DSP_FLD ("st_item_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_item"))
	{
		if (FLD ("end_item") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.end_item, "~~~~~~~~~~~~~~~~");
			sprintf (local_rec.end_item_desc, "%-40.40s", "Last Item");

			DSP_FLD ("end_item");
			DSP_FLD ("end_item_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
		   	return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.end_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.end_item);
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

		if (strcmp (inmr_rec.source, "RM"))
		{
			print_mess ( ML ("Item must be a raw material (source RM)"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.end_item_desc,inmr_rec.description);
		DSP_FLD ("end_item_desc");

		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate start class |
	----------------------*/
	if (LCHECK ("st_class"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY && 
				strcmp (local_rec.st_class, local_rec.end_class) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate end class |
	----------------------*/
	if (LCHECK ("end_class"))
	{
		if (strcmp (local_rec.st_class, local_rec.end_class) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		return (EXIT_SUCCESS);
	}

 	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("st_cat")) 
	{
		if (FLD ("st_cat") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.st_cat, "%-11.11s", " ");
			sprintf (local_rec.st_cat_desc, "%-40.40s", "First Category");

			DSP_FLD ("st_cat");
			DSP_FLD ("st_cat_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.st_cat);

		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		if (prog_status != ENTRY && 
		    strcmp (local_rec.st_cat,local_rec.end_cat) > 0 )
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS); 
		}
		sprintf (local_rec.st_cat_desc,
			"%-40.40s",
			excf_rec.cat_desc);

		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("end_cat")) 
	{
		if (FLD ("end_cat") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.end_cat, "~~~~~~~~~~~");
			sprintf (local_rec.end_cat_desc, "%-40.40s", "Last Category");

			DSP_FLD ("end_cat");
			DSP_FLD ("end_cat_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.end_cat);
		
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		if (strcmp (local_rec.st_cat,local_rec.end_cat) > 0 )
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.end_cat_desc,excf_rec.cat_desc);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char *key_val)
{
	work_open ();
	save_rec ("#Category", "#Description");

	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no ,"%-11.11s",key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in excf During (DBFIND)",cc, PNAME);
}

void
Process (
 void)
{

	if (BY_ITEM)
	{
		sprintf (inmr_rec.item_no, "%-16.16s", local_rec.st_item);
		dsp_screen ("Reading Bills By Item", 
			comm_rec.co_no, 
			comm_rec.co_name);
	}
	else
	{
		sprintf (inmr_rec.inmr_class, "%-1.1s", local_rec.st_class);
		sprintf (inmr_rec.category, "%-11.11s", local_rec.st_cat);
		dsp_screen ("Reading Bills By Group", 
			comm_rec.co_no, 
			comm_rec.co_name);
	}
	
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
	       ( (BY_ITEM && 
		strcmp (inmr_rec.item_no,local_rec.end_item) <= 0) ||
	        (!BY_ITEM && 
		 strcmp (inmr_rec.inmr_class,local_rec.end_class) <= 0)))
	{
		if (!strcmp (inmr_rec.inmr_class, local_rec.end_class) &&
		    strcmp (inmr_rec.category, local_rec.end_cat) > 0)	
			break;
		
		if (strcmp (inmr_rec.source, "RM"))
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		dsp_process ("Item :", inmr_rec.item_no);

		UpdateItem (inmr_rec.hhbr_hash);

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

}

int
UpdateItem (
 int hhbr_hash)
{
	inei_rec.hhbr_hash = hhbr_hash;
	strcpy (inei_rec.est_no, comm_rec.est_no);	
	cc = find_rec ("inei", &inei_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	if (BY_LAST)
	{
		inei_rec.std_cost = inei_rec.last_cost +
				       (inei_rec.last_cost * 
					 (double)local_rec.percent);
	}
	else
	{
		inei_rec.std_cost += (inei_rec.std_cost * 
					 (double)local_rec.percent);
	}

	inei_rec.std_cost = twodec (inei_rec.std_cost);

	inei_rec.date_lcost = local_rec.lsystemDate;

	cc = abc_update ("inei", &inei_rec);
	if (cc)
		return (FALSE);

	return (TRUE);
}

