/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: fa_depdisp.c,v 5.5 2001/09/20 03:58:10 val Exp $
|  Program Name  : (fa_depdisp.c)
|  Program Desc  : (Fixed Assets Depreciation display/print)
|---------------------------------------------------------------------|
|  Date Written  : (15/05/1997)    | Author       : Scott B Darrow.   |
|---------------------------------------------------------------------|
| $Log: fa_depdisp.c,v $
| Revision 5.5  2001/09/20 03:58:10  val
| Modified program to initialize the GrandTotal field
|
| Revision 5.4  2001/09/19 00:38:24  scott
| General cleanup while looking at code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fa_depdisp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FA/fa_depdisp/fa_depdisp.c,v 5.5 2001/09/20 03:58:10 val Exp $";

#include	<pslscr.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_fa_mess.h>

#define		INTERNAL	 (ReportType [0] == 'I')
#define		TAX			 (ReportType [0] == 'T')

#define	X_OFF	lp_x_off
#define	Y_OFF	lp_y_off

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
	extern int		lp_x_off,
					lp_y_off;

#include	"schema"

struct commRecord	comm_rec;
struct famrRecord	famr_rec;
struct faglRecord	fagl_rec;
struct fatrRecord	fatr_rec;

	double	AccDep			= 0.00;
	double	DepThisPeriod	= 0.00;
	double	BookValue		= 0.00;

struct {
	char	dummy [11];
	char	StartAssetGroup [6];
	char	StartAssetDesc [41];
	char	EndAssetGroup [6];
	char	EndAssetDesc [41];
			
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "StartAssetGroup",	 2, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Start Asset Group code   : ", "Enter Start Asset group code. [SEARCH] available ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.StartAssetGroup},
	{1, LIN, "StartAssetDesc",2, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.StartAssetDesc},
	{1, LIN, "EndAssetGroup",	 3, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Start Asset Group code   : ", "Enter Start Asset group code. [SEARCH] available ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.EndAssetGroup},
	{1, LIN, "EndAssetDesc",3, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.EndAssetDesc},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

extern	int	TruePosition;
extern	int	EnvScreenOK;
char	ReportType [2];

double	GroupTotal [4],
		GrandTotal [4];

char	DspValue [4][21];
char	disp_str [300];
char	*BlankDesc	=	"                                        ";

/*========================
| Function Prototypes    |
=========================*/
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void Process (void);
void ProcAsset (char *AssGroup);
int CheckAsset (char *AssGroup);
void CalcClosing (void);
void SrchFatr (char *key_val);
void SrchFamr (char *key_val);
int heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char *argv [])
{
	if (argc < 2)
	{
		print_at (0,0,mlFaMess700,argv [0]);
		return (EXIT_FAILURE);
	}

	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	sprintf (ReportType, "%-1.1s", argv [1]);

	if (ReportType [0] != 'I' && ReportType [0] != 'T')
	{
		print_at (0,0,mlFaMess700,argv [0]);
		return (EXIT_FAILURE);
	}
	SETUP_SCR (vars);
	init_scr ();	
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	while (!prog_exit)
    {
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit != 0 || restart != 0)
			continue;

		crsr_off ();
		Process ();
		crsr_on ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================= 
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	open_rec ("fagl",fagl_list, FAGL_NO_FIELDS, "fagl_famr_hash");
	open_rec ("famr", famr_list, FAMR_NO_FIELDS, "famr_id_no");
	open_rec ("fatr", fatr_list, FATR_NO_FIELDS, "fatr_id_no");
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose ("fagl");
	abc_fclose ("famr");
	abc_fclose ("fatr");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*----------------------------
	| Validate Asset group code. |
	----------------------------*/
	if (LCHECK ("StartAssetGroup"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.StartAssetGroup, "     ");
			strcpy (local_rec.StartAssetDesc,  "Start of range.");
			DSP_FLD ("StartAssetDesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchFatr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (fatr_rec.co_no,comm_rec.co_no);
		strcpy (fatr_rec.group,local_rec.StartAssetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlFaMess003));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.StartAssetDesc, fatr_rec.group_desc);
		DSP_FLD ("StartAssetDesc");
		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Asset group code. |
	----------------------------*/
	if (LCHECK ("EndAssetGroup"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.EndAssetGroup, "~~~~~");
			strcpy (local_rec.EndAssetDesc,  "End of range.");
			DSP_FLD ("EndAssetDesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchFatr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (fatr_rec.co_no,comm_rec.co_no);
		strcpy (fatr_rec.group,local_rec.EndAssetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlFaMess003));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.EndAssetDesc, fatr_rec.group_desc);
		DSP_FLD ("EndAssetDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Process (
 void)
{
	lp_x_off = 0;
	lp_y_off = 1;

	GrandTotal [0] = GrandTotal [1] = GrandTotal [2] = GrandTotal [3] = 0.00;
	Dsp_nc_prn_open (0, 1, 15, "Fixed Asset Depreciation Schedule",comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);
			
	Dsp_saverec ("Asset|   F i x e d   A s s e t   D e s c .  |  Date    |Life in|  Acquisition  |  Accumulated   |  Depreciation  |  Current Book  ");
	Dsp_saverec (" No. |                                      | Acquired | Years |     Cost      |  Depreciation  |   This Period  |     Value.     ");

	Dsp_saverec (" [PRINT]   [NEXT SCREEN]   [PREV SCREEN]   [EDIT/END] ");

	strcpy (fatr_rec.co_no,comm_rec.co_no);
	strcpy (fatr_rec.group,local_rec.StartAssetGroup);
	cc = find_rec (fatr, &fatr_rec, GTEQ, "r");
	while (!cc && !strcmp (fatr_rec.co_no,comm_rec.co_no) &&
				  strcmp (fatr_rec.group, local_rec.StartAssetGroup) >= 0 &&
				  strcmp (fatr_rec.group, local_rec.EndAssetGroup) <= 0)
	{
		if (CheckAsset (fatr_rec.group))
			ProcAsset (fatr_rec.group);

		cc = find_rec (fatr, &fatr_rec, NEXT, "r");
	}
	sprintf (DspValue [0], comma_fmt (GrandTotal [0], "NNNN,NNN,NNN.NN"));
	sprintf (DspValue [1], comma_fmt (GrandTotal [1], "NNNNN,NNN,NNN.NN"));
	sprintf (DspValue [2], comma_fmt (GrandTotal [2], "NNNNN,NNN,NNN.NN"));
	sprintf (DspValue [3], comma_fmt (GrandTotal [3], "NNNNN,NNN,NNN.NN"));
	sprintf (disp_str, "%5.5s^E^2GRAND TOTAL^6%27.27s^E%-10.10s^E%7.7s^E%15.15s^E%16.16s^E%16.16s^E%16.16s",
			" ",
			" ",
			" ",
			" ",
			DspValue [0],
			DspValue [1],
			DspValue [2],
			DspValue [3]);
	Dsp_saverec (disp_str);
	Dsp_saverec ("^^GGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGG");
	Dsp_srch ();
	Dsp_close ();
}

void
ProcAsset (
 char	*AssGroup)
{
	char	SoldDate [11];
	char	*expand (char *, char *);

	expand (err_str, fatr_rec.group_desc);
	sprintf (disp_str, "%-5.5s - (%80.80s)", fatr_rec.group, err_str);
	Dsp_saverec (disp_str);

	strcpy (famr_rec.co_no, comm_rec.co_no);
	strcpy (famr_rec.ass_group, AssGroup);
	strcpy (famr_rec.ass_no, "     ");
	cc = find_rec ("famr", &famr_rec, GTEQ, "r");
	while (!cc && !strcmp (famr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (famr_rec.ass_group, AssGroup))
	{
		if (famr_rec.stat_flag [0] == 'D')
		{
			cc = find_rec ("famr", &famr_rec, NEXT, "r");
			continue;
		}
		if (famr_rec.disp_date > 0L)
		{
			strcpy (SoldDate, DateToString (famr_rec.disp_date));
			sprintf (disp_str, "%s^E%-38.38s^E%-10.10s^E%7.7s^E ^2NOTE : Asset Sold on %s for %16.16s^6",
			famr_rec.ass_no,
			famr_rec.ass_desc1,
			DateToString (famr_rec.pur_date),
			famr_rec.ass_life,
			SoldDate,
			comma_fmt (DOLLARS (famr_rec.disp_price),"N,NNN,NNN,NNN.NN"));
			Dsp_saverec (disp_str);
			
			cc = find_rec ("famr", &famr_rec, NEXT, "r");
			continue;
		}
		CalcClosing ();
		sprintf (DspValue [0], comma_fmt (DOLLARS (famr_rec.cost_price),
														"NNNN,NNN,NNN.NN"));
		sprintf (DspValue [1], comma_fmt (AccDep,	   "N,NNN,NNN,NNN.NN"));
		sprintf (DspValue [2], comma_fmt (DepThisPeriod, "N,NNN,NNN,NNN.NN"));
		sprintf (DspValue [3], comma_fmt (BookValue,    "N,NNN,NNN,NNN.NN"));

		sprintf (disp_str, "%s^E%-38.38s^E%-10.10s^E%7.7s^E%15.15s^E%16.16s^E%16.16s^E%16.16s",
			famr_rec.ass_no,
			famr_rec.ass_desc1,
			DateToString (famr_rec.pur_date),
			famr_rec.ass_life,
			DspValue [0],
			DspValue [1],
			DspValue [2],
			DspValue [3]);
	
		Dsp_saverec (disp_str);

		if (strcmp (famr_rec.ass_desc2, BlankDesc))
		{
			sprintf (disp_str, "%5.5s^E%-38.38s^E%10.10s^E%7.7s^E%15.15s^E%16.16s^E%16.16s^E%16.16s",
			" ", famr_rec.ass_desc2, " ", " ", " ", " ", " ", " ");
			Dsp_saverec (disp_str);
		}
		if (strcmp (famr_rec.ass_desc3, BlankDesc))
		{
			sprintf (disp_str, "%5.5s^E%-38.38s^E%10.10s^E%7.7s^E%15.15s^E%16.16s^E%16.16s^E%16.16s",
			" ", famr_rec.ass_desc3, " ", " ", " ", " ", " ", " ");
			Dsp_saverec (disp_str);
		}
		if (strcmp (famr_rec.ass_desc4, BlankDesc))
		{
			sprintf (disp_str, "%5.5s^E%-38.38s^E%10.10s^E%7.7s^E%15.15s^E%16.16s^E%16.16s^E%16.16s",
			" ", famr_rec.ass_desc4, " ", " ", " ", " ", " ", " ");
			Dsp_saverec (disp_str);

		}
		GroupTotal [0]	+=	DOLLARS (famr_rec.cost_price);
		GroupTotal [1]	+=  AccDep;
		GroupTotal [2]	+= 	DepThisPeriod;
		GroupTotal [3]	+=  BookValue;
		GrandTotal [0]	+=	DOLLARS (famr_rec.cost_price);
		GrandTotal [1]	+=  AccDep;
		GrandTotal [2]	+= 	DepThisPeriod;
		GrandTotal [3]	+=  BookValue;

		cc = find_rec ("famr", &famr_rec, NEXT, "r");
	}
	sprintf (DspValue [0], comma_fmt (GroupTotal [0], "NNNN,NNN,NNN.NN"));
	sprintf (DspValue [1], comma_fmt (GroupTotal [1], "N,NNN,NNN,NNN.NN"));
	sprintf (DspValue [2], comma_fmt (GroupTotal [2], "N,NNN,NNN,NNN.NN"));
	sprintf (DspValue [3], comma_fmt (GroupTotal [3], "N,NNN,NNN,NNN.NN"));
	sprintf (disp_str, "%5.5s^E^1GROUP TOTAL^6%27.27s^E%-10.10s^E%7.7s^E%15.15s^E%16.16s^E%16.16s^E%16.16s",
			" ",
			" ",
			" ",
			" ",
			DspValue [0],
			DspValue [1],
			DspValue [2],
			DspValue [3]);
	Dsp_saverec (disp_str);

	GroupTotal [0]	= 0.00;
	GroupTotal [1]	= 0.00;
	GroupTotal [2]	= 0.00;
	GroupTotal [3]	= 0.00;
	Dsp_saverec ("^^GGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGG");
}

int
CheckAsset (
 char	*AssGroup) 
{
	strcpy (famr_rec.co_no, comm_rec.co_no);
	strcpy (famr_rec.ass_group, AssGroup);
	strcpy (famr_rec.ass_no, "     ");
	cc = find_rec ("famr", &famr_rec, GTEQ, "r");
	if (!cc && !strcmp (famr_rec.co_no, comm_rec.co_no) &&
			   !strcmp (famr_rec.ass_group, AssGroup))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
CalcClosing (
 void)
{
	BookValue 		= DOLLARS (famr_rec.int_open_val);
	AccDep			= 0.00;
	DepThisPeriod	= 0.00;

	fagl_rec.famr_hash 	=	famr_rec.famr_hash;
	cc = find_rec ("fagl", &fagl_rec, GTEQ, "r");
	while (!cc && fagl_rec.famr_hash 	==	famr_rec.famr_hash)
	{
		if (fagl_rec.tran_date <= MonthEnd (comm_rec.crd_date))
		{
			if (MonthStart (fagl_rec.tran_date) == MonthStart (comm_rec.crd_date))
			{
				if (INTERNAL)
					DepThisPeriod = DOLLARS (fagl_rec.int_amt);
				else
					DepThisPeriod = DOLLARS (fagl_rec.tax_amt);
			}

			if (INTERNAL)
			{
				BookValue	-= DOLLARS (fagl_rec.int_amt);
				AccDep		+= DOLLARS (fagl_rec.int_amt);
			}
			else
			{
				BookValue	-= DOLLARS (fagl_rec.tax_amt);
				AccDep		+= DOLLARS (fagl_rec.tax_amt);
			}
		}
		
		cc = find_rec ("fagl", &fagl_rec, NEXT, "r");
	}
	if (BookValue < 0.00)
		BookValue = 0.00;
}

/*===================================
| Search for inventory master file. |
===================================*/
void
SrchFatr (
 char	*key_val)
{
	_work_open (5,0,40);
	save_rec ("#Asset group", "#Asset Group Description");
	strcpy (fatr_rec.co_no, comm_rec.co_no);
	sprintf (fatr_rec.group, "%-5.5s", key_val);
	cc = find_rec (fatr, &fatr_rec, GTEQ, "r");
	while (!cc && !strncmp (fatr_rec.group, key_val, strlen (key_val)) && 
				  !strcmp (fatr_rec.co_no, comm_rec.co_no))
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
	strcpy (fatr_rec.co_no, comm_rec.co_no);
	sprintf (fatr_rec.group, "%-5.5s", temp_str);
	cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, fatr, "DBFIND");
}

/*===================================
| Search for inventory master file. |
===================================*/
void
SrchFamr (
 char	*key_val)
{
	_work_open (5,0,40);
	save_rec ("#Asset No", "#Asset Description");
	strcpy (famr_rec.co_no, comm_rec.co_no);
	sprintf (famr_rec.ass_no, "%-5.5s", key_val);
	cc = find_rec (famr, &famr_rec, GTEQ, "r");
	while (!cc && !strncmp (famr_rec.ass_no, key_val, strlen (key_val)) && 
				  !strcmp (famr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (famr_rec.ass_no, famr_rec.ass_desc1);
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
	strcpy (famr_rec.co_no, comm_rec.co_no);
	sprintf (famr_rec.ass_no, "%-5.5s", temp_str);
	cc = find_rec (famr, &famr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, famr, "DBFIND");
}

/*================================================================
| Heading concerns itself with clearing the screen, painting the |
| screen overlay in preparation for input.                       |
================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();
		
		box (0,1,132,2);

		if (INTERNAL)
			rv_pr (	ML (mlFaMess008), (130 - strlen (ML (mlFaMess008))) / 2,0, 1);
		else
			rv_pr (	ML (mlFaMess009), (130 - strlen (ML (mlFaMess009))) / 2,0, 1);
		line_at (21,1,131);
		print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
