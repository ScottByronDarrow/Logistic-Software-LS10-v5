/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( fa_sale.c      )                                 |
|  Program Desc  : ( Fixed Assets Disposal Maintenance            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, famr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  famr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 31/10/87         |
|---------------------------------------------------------------------|
|  Date Modified : (31/10/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (22/07/92)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (11/09/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      :                                                    |
|  (22/07/92)    : General Update.                                    |
|  (11/09/97)    : Updated for Multilingual Conversion. 			  |
|                :                                                    |
|                                                                     |
| $Log: fa_sale.c,v $
| Revision 5.2  2001/08/09 09:13:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:25:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:57  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:26:30  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:14:38  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:55:21  gerry
| forced Revsion No start 2.0 Rel-15072000
|
| Revision 1.15  1999/12/06 01:46:58  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.14  1999/11/17 06:40:04  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.13  1999/11/08 04:54:45  scott
| Updated due to warnings using -Wall flag on compiler.
|
| Revision 1.12  1999/10/01 07:48:37  scott
| Updated for standard function calls.
|
| Revision 1.11  1999/09/29 10:10:39  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 07:26:28  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.9  1999/09/16 02:49:17  scott
| Updated from Ansi Project.
|
| Revision 1.8  1999/09/10 01:48:35  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.7  1999/06/14 23:57:40  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fa_sale.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FA/fa_sale/fa_sale.c,v 5.2 2001/08/09 09:13:13 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_fa_mess.h>

int	new_item = 0;
int	edit_mode = 0;

	/*===============================
	| Common Record Structure	|
	===============================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"}
	};

	int comm_no_fields = 3;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
	} comm_rec;

	/*=============================+
	 | Fixed Assets Master Record. |
	 +=============================*/
#define	FAMR_NO_FIELDS	13

	struct dbview	famr_list [FAMR_NO_FIELDS] =
	{
		{"famr_co_no"},
		{"famr_ass_group"},
		{"famr_ass_no"},
		{"famr_ass_desc1"},
		{"famr_ass_desc2"},
		{"famr_ass_desc3"},
		{"famr_ass_desc4"},
		{"famr_pur_date"},
		{"famr_ass_life"},
		{"famr_cost_price"},
		{"famr_disp_date"},
		{"famr_disp_price"},
		{"famr_stat_flag"},
	};

	struct tag_famrRecord
	{
		char	co_no [3];
		char	ass_group [6];
		char	ass_no [6];
		char	ass_desc[4] [41];
		Date	pur_date;
		char	ass_life [8];
		Money	cost_price;
		Date	disp_date;
		Money	disp_price;
		char	stat_flag [2];
	}	famr_rec;

	/*================================+
	 | Fixed Asset Transactions file. |
	 +================================*/
#define	FATR_NO_FIELDS	4

	struct dbview	fatr_list [FATR_NO_FIELDS] =
	{
		{"fatr_co_no"},
		{"fatr_group"},
		{"fatr_group_desc"},
		{"fatr_stat_flag"}
	};

	struct tag_fatrRecord
	{
		char	co_no [3];
		char	group [6];
		char	group_desc [41];
		char	stat_flag [2];
	}	fatr_rec;

	char	*fatr	=	"fatr",
			*famr	=	"famr";

extern	int	TruePosition;
extern	int	EnvScreenOK;

	char	systemDate[11];
struct
{
	char	dummy[11];
	char	AssetGroup[6],
			AssetNumber[6];
	char	PrevNumber[6];
	char	AssYears[4],
			AssMonth[2];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "AssetGroup",	 2, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Asset Group code   : ", "Enter Asset group code. [SEARCH] available ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.AssetGroup},
	{1, LIN, "AssetGroupDesc",3, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Group desc   : ", " ",
		 NA, NO,  JUSTLEFT, "", "", fatr_rec.group_desc},
	{1, LIN, "AssetNumber",	 4, 2, CHARTYPE,
		"UUUUU", "          ",
		"0", " ", "Asset number       : ", "Enter Asset number. <Default = new asset> [SEARCH] available. ",
		 NE, NO,  JUSTRIGHT, "0123456789", "", local_rec.AssetNumber},
	{1, LIN, "AssetDesc1",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Desc #1      : ", "Enter asset description ",
		 NA, NO,  JUSTLEFT, "", "", famr_rec.ass_desc[0]},
	{1, LIN, "AssetDesc2",	 6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Desc #2      : ", "Enter asset description ",
		 NA, NO,  JUSTLEFT, "", "", famr_rec.ass_desc[1]},
	{1, LIN, "AssetDesc3",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Desc #2      : ", "Enter asset description ",
		 NA, NO,  JUSTLEFT, "", "", famr_rec.ass_desc[2]},
	{1, LIN, "AssetDesc4",	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Desc #3      : ", "Enter asset description ",
		 NA, NO,  JUSTLEFT, "", "", famr_rec.ass_desc[3]},
	{1, LIN, "PurchaseDate",	 10, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		"0", " ", "Purchase date      : ", "Enter purchase date of asset. <default = today> ",
		NA, NO,  JUSTLEFT, "", "", (char *)&famr_rec.pur_date},
	{1, LIN, "CostPrice",	 11, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", " ", "Cost price         : ", "Enter cost price of Asset. ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&famr_rec.cost_price},
	{1, LIN, "AssetLife",	 12, 2, CHARTYPE,
		"NNN", "          ",
		" ", "", "Asset Life Years   : ", "Enter Asset life Years ",
		 NA, NO,  JUSTRIGHT, "", "", local_rec.AssYears},
	{1, LIN, "AssetMonth",	 12, 26, CHARTYPE,
		"NN", "          ",
		" ", "0", ":", "Enter Asset life Months ",
		 NA, NO,  JUSTLEFT, "0", "12", local_rec.AssMonth},
	{1, LIN, "SaleDate",	 14, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ",systemDate, "Asset Sale Date    : ", "Enter Date Asset was sold ",
		YES, NO,  JUSTLEFT, "", "", (char *) &famr_rec.disp_date},
	{1, LIN, "saleprice",	 15, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "Sale price         : ", " ",
		 NO, NO,  JUSTRIGHT, "", "", (char *) &famr_rec.disp_price},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

int		main (int argc, char *argv []);
void	shutdown_prog (void);
void	OpenDB (void);
void	CloseDB (void);
int		heading (int scn);
int		spec_valid (int field);
void	update (void);
void	SrchFatr (char *key_val);
void	SrchFamr (char *key_val);

int
main
(
	int argc,
	char *argv []
)
{

	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	SETUP_SCR( vars );

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/

	OpenDB();

	strcpy (systemDate, DateToString (TodaysDate()));

	strcpy(local_rec.PrevNumber,"00000");

	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		new_item = 0;
		search_ok = 1;
		edit_mode = 0;
		init_vars(1);

		heading(1);
		entry(1);

		if (prog_exit)
			break;

		edit_all();

		if ( !restart )
			update();

		abc_unlock("famr");
	}
	shutdown_prog();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB
(
	void
)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec (famr, famr_list, FAMR_NO_FIELDS, "famr_id_no");
	open_rec (fatr, fatr_list, FATR_NO_FIELDS, "fatr_id_no");
}

void
CloseDB
(
	void
)
{
	abc_fclose (fatr);
	abc_fclose (fatr);
	abc_dbclose("data");
}

int
spec_valid 
(
	int field
)
{
	/*----------------------------
	| Validate Asset group code. |
	----------------------------*/
	if (LCHECK ("AssetGroup"))
	{
		if (SRCH_KEY)
		{
			SrchFatr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (fatr_rec.co_no,comm_rec.tco_no);
		strcpy (fatr_rec.group,local_rec.AssetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML(mlFaMess003));
			return (EXIT_FAILURE);
		}
		DSP_FLD ("AssetGroupDesc");
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Fixed Asset number. |
	------------------------------*/
	if (LCHECK ("AssetNumber"))
	{
		if (SRCH_KEY)
		{
			SrchFamr (temp_str);
			return (EXIT_SUCCESS);
		}
		abc_selfield(famr,"famr_id_no");
		strcpy (famr_rec.co_no,comm_rec.tco_no);
		strcpy (famr_rec.ass_group,	local_rec.AssetGroup);
		strcpy (famr_rec.ass_no,	local_rec.AssetNumber);
		cc = find_rec (famr, &famr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML(mlFaMess004));
			sleep (sleepTime);
			return(1);
		}
		sprintf (local_rec.AssYears, "%-3.3s", famr_rec.ass_life);
		sprintf (local_rec.AssMonth, "%-2.2s", famr_rec.ass_life + 5);
		DSP_FLD ("AssetLife");
		DSP_FLD ("AssetMonth");
		
		return (EXIT_SUCCESS);
	}

	if ( LCHECK("SaleDate") )
	{
		if (famr_rec.disp_date < famr_rec.pur_date)
		{
			print_mess(ML(mlFaMess005));
			return(1);
		}
		return(0);
	}

	return( 0 );
}

void
update
(
	void
)
{
	clear();

	strcpy (famr_rec.stat_flag, "D");
	cc = abc_update("famr",&famr_rec);
	if (cc)
		file_err(cc, "famr", "DBUPDATE" );

	strcpy(local_rec.PrevNumber,local_rec.AssetNumber);
	return;
}

/*===================================
| Search for inventory master file. |
===================================*/
void
SrchFatr 
(
	char	*key_val
)
{
	work_open ();
	save_rec ("#Asset group", "#Asset Group Description");
	strcpy (fatr_rec.co_no, comm_rec.tco_no);
	sprintf (fatr_rec.group, "%-5.5s", key_val);
	cc = find_rec (fatr, &fatr_rec, GTEQ, "r");
	while (!cc && !strncmp (fatr_rec.group, key_val, strlen (key_val)) && 
				  !strcmp (fatr_rec.co_no, comm_rec.tco_no))
	{
		cc = save_rec (fatr_rec.group, fatr_rec.group_desc);
		if (cc)
			break;

		cc = find_rec (fatr, &fatr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (fatr_rec.group, "%-5.5s", " ");
		return;
	}
	strcpy (fatr_rec.co_no, comm_rec.tco_no);
	sprintf (fatr_rec.group, "%-5.5s", temp_str);
	cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, fatr, "DBFIND");
}

/*===================================
| Search for inventory master file. |
===================================*/
void
SrchFamr 
(
	char	*key_val
)
{
	work_open ();
	save_rec ("#Asset No", "#Asset Description");
	strcpy (famr_rec.co_no, comm_rec.tco_no);
	strcpy (famr_rec.ass_group,	local_rec.AssetGroup);
	sprintf (famr_rec.ass_no, "%-5.5s", key_val);
	cc = find_rec (famr, &famr_rec, GTEQ, "r");
	while (!cc && !strcmp (famr_rec.co_no, comm_rec.tco_no) &&
				  !strcmp (famr_rec.ass_group, local_rec.AssetGroup) &&
				  !strncmp (famr_rec.ass_no, key_val, strlen (key_val)))
	{
		cc = save_rec (famr_rec.ass_no, famr_rec.ass_desc [0]);
		if (cc)
			break;

		cc = find_rec (famr, &famr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (famr_rec.ass_no, "%-5.5s", " ");
		return;
	}
	strcpy (famr_rec.co_no, comm_rec.tco_no);
	strcpy (famr_rec.ass_group,	local_rec.AssetGroup);
	sprintf (famr_rec.ass_no, "%-5.5s", temp_str);
	cc = find_rec (famr, &famr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, famr, "DBFIND");
}

int
heading
(
	int scn
)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();

		rv_pr(ML(mlFaMess006),28,0,1);

		print_at(0,53, ML(mlFaMess007) ,local_rec.PrevNumber);

		box(0,1,80,14);
		move(1,9);
		line(79);
		move(1,13);
		line(79);

		move(0,20);
		line(80);

		strcpy(err_str, ML(mlStdMess038));
		print_at(21,0, err_str ,comm_rec.tco_no,comm_rec.tco_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
