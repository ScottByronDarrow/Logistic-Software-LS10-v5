/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lrp_whrepl.c,v 5.5 2002/11/25 03:16:41 scott Exp $
|  Program Name  : (lrp_whrepl.c)
|  Program Desc  : (Print Warehouse Replenishment Report)
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
| $Log: lrp_whrepl.c,v $
| Revision 5.5  2002/11/25 03:16:41  scott
| Updated to use chk_env instead of get_env when applicable.
|
| Revision 5.4  2002/07/17 09:57:23  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/09 09:30:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:59  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:12  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_whrepl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_whrepl/lrp_whrepl.c,v 5.5 2002/11/25 03:16:41 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_lrp_mess.h>
#include <Costing.h>

#define		PRN_INCLUDE		(local_rec.printInclude [0] == 'Y')
#define		PRN_EXCLUDE		(local_rec.printExclude [0] == 'Y')
#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct ffprRecord	ffpr_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct inwsRecord	inws_rec;
struct inwdRecord	inwd_rec;
struct inmrRecord	inmr_rec;
struct itlnRecord	itln_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct ffwkRecord	ffwk_rec;
struct ffwkRecord	ffwk2_rec;

	float	*ffwk_cons	= &ffwk_rec.cons_1;
	char	*data		= "data",
			*incc2		= "incc2";

struct	Supply_rec
{								/*------------------------------*/
	char	acronym [10];		/* Warehouse acronym			*/
	long	hhccHash;			/* Warehouse serial hash		*/
	float	onOrder;			/* Qty on order					*/
	float	onHand;				/* Qty on hand					*/
	float	committed;			/* Qty committed				*/
	float	safetyStock;		/* Safety stock level			*/
	float	minimumOrder;		/* Minimum Order amount			*/
	float	normalOrder;		/* Normal Order amount			*/
	float	orderMultiple;		/* Minimum Order Resolution		*/
	float	leadTimes;			/* Lead time					*/
	float	reviewPeriod;		/* Review period				*/
	float	weeksDemand;		/* Weeks Demand                 */
	float	upliftPc;			/* Uplift percent from inwd		*/
	double	upliftAmt;			/* Uplift amount from inwd		*/
	double	costPrice;			/* Cost of line.				*/
	double	totalUplift;		/* Total uplift amount 			*/
	char	allowReport [2];	/* Is xfer generation allowed?	*/
} lrpRec;						/*------------------------------*/

char	envVarSupOrdRound [2];
char	*envVarSkInvlClass;
char	*result;
int		envVarSoFwdRel;
float	envVarLrpDfltReview;

FILE	*fout;

	struct	{
		char	br_no [3];
		char	wh_no [3];
		long	hhccHash;
		char	wh_short [10];
	} supply_rec;

#include 	<RealCommit.h>

struct
{
	int		printerNumber;
	char	back [7];
	char	backDesc [7];
	char	onight [7];
	char	onightDesc [7];
	char	printInclude [7];
	char	printIncludeDesc [7];
	char	printExclude [7];
	char	printExcludeDesc [7];
	int		reportUsed;
	char	transfer [7];
	char	transfer_desc [7];
	char	sortType [12];
	char	sortTypeDesc [12];
	char	printerString [3];
	char	startClass [2];
	char	startCategory [12];
	char	endClass [2];
	char	endCategory [12];
	char	lower [13];
	char	upper [13];
	char	zeroSupplied [7];
	char	zeroSuppDesc [7];
	char	NoLevels [2];
	char	dummy [12];
} local_rec;

static struct	var vars [] ={
	{1, LIN, "printerNumber",		 4, 2,   INTTYPE, "NN",
		"          ", " ",
		"1", "Printer number         ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNumber},
	{1, LIN, "back",		 5, 2,  CHARTYPE, "U",
		"          ", " ",
		"N", "Background             ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 5, 28,  CHARTYPE, "AAA",
		"          ", " ",
		"", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.backDesc},
	{1, LIN, "onight",		 5, 40,  CHARTYPE, "U",
		"          ", " ",
		"N", "Overnight              ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	 5, 66,  CHARTYPE, "AAA",
		"          ", " ",
		"", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.onightDesc},
	{1, LIN, "startClass",	 7, 2,  CHARTYPE, "U",
		"          ", " ",
		"A", "Start Class            ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startClass},
	{1, LIN, "startCategory",	 7, 40,  CHARTYPE, "UUUUUUUUUUU",
		"          ", " ",
		" ", "Start Category         ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCategory},
	{1, LIN, "endClass",	 8, 2,  CHARTYPE, "U",
		"          ", " ",
		"A", "End Class              ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endClass},
	{1, LIN, "endCategory",	 8, 40,  CHARTYPE, "UUUUUUUUUUU",
		"          ", " ",
		" ", "End Category           ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCategory},
	{1, LIN, "zeroSupplied",	10, 2,  CHARTYPE, "U",
		"          ", " ",
		"Y", "Suppress zero qty's    ", " Suppress printing/updating if quantity is zero. ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.zeroSupplied},
	{1, LIN, "zeroSuppDesc",	10, 28,  CHARTYPE, "AAA",
		"          ", " ",
		"", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.zeroSuppDesc},
	{1, LIN, "sortType",	11, 2,  CHARTYPE, "U",
		"          ", " ",
		"G", "Sort By                ", " G(roup  I(tem Number ",
		YES, NO, JUSTRIGHT, "GI", "", local_rec.sortType},
	{1, LIN, "sortTypeDesc",	11, 28,  CHARTYPE, "AAAAAAAA",
		"          ", " ",
		"", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.sortTypeDesc},
	{1, LIN, "printInclude",	12, 2,  CHARTYPE, "U",
		"          ", " ",
		"Y", "Print Included Items   ", "Print items set to be included in replenishment.",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.printInclude},
	{1, LIN, "printIncludeDesc",	12, 28,  CHARTYPE, "AAA",
		"          ", " ",
		"", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.printIncludeDesc},
	{1, LIN, "printExclude",	13, 2,  CHARTYPE, "U",
		"          ", " ",
		"N", "Print Excluded Items   ", "Print items set to be excluded from replenishment",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.printExclude},
	{1, LIN, "printExcludeDesc",	13, 28,  CHARTYPE, "AAA",
		"          ", " ",
		"", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.printExcludeDesc},
	{1, LIN, "transfer",	14, 2,  CHARTYPE, "U",
		"          ", " ",
		"N", "Generate Transfers     ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.transfer},
	{1, LIN, "transfer_desc",	14, 28,  CHARTYPE, "AAA",
		"          ", " ",
		"", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.transfer_desc},
	{0, LIN, "dummy",	 0,  0,  CHARTYPE, "A",
		"          ", " ",
		"", " ", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

extern int TruePosition;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	LoadDefault 		(void);
void 	SrchExcf 			(char *);
void 	ScreenHeading 		(void);
void 	HeadingOutput 		(void);
void 	Process 			(void);
void 	ProcessItem 		(long, int);
void 	PrintUpdateItem 	(long);
int 	heading 			(int);
int 	spec_valid 			(int);
int 	check_page 			(void);
int 	ValidateGroup 		(void);
int 	ProcessData 		(long, long);
double 	FindCost 			(void);
float 	CheckInwardsGoods 	(float);
static 	float Rnd_Mltpl 	(float, char *, float, float);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	TruePosition	=	TRUE;

	if (argc != 2 && argc != 10)
	{
		print_at (0,0, "Usage : %s <LPNO> <start_group> <end_group> <Suppress 0 (Y/N)> <sortType> <print included (Y/N)> <print excluded (Y/N)> <Gen. transfer (Y/N)> <No Levels>\007\n\r", argv [0]);
		print_at (2,0, "  Or  : %s <prog_desc>\007\n\r", argv [0]);
        return (EXIT_FAILURE);
	}

	sptr = chk_env ("LRP_DFLT_REVIEW");
	envVarLrpDfltReview = (sptr == (char *) 0) ? 4 : atof (sptr);

	sptr = chk_env("SK_IVAL_CLASS");
	envVarSkInvlClass = (sptr == (char *) 0) ? strdup (sptr) : strdup ("KNPZ");

	upshift (envVarSkInvlClass); 

	if (argc == 10)
	{
		set_tty ();

		sptr = chk_env ("SUP_ORD_ROUND");
		sprintf (envVarSupOrdRound,"%-1.1s",(sptr == (char *) 0) ? "B" : sptr);

		sptr = chk_env ("SO_FWD_REL");
		envVarSoFwdRel = (sptr == (char *) 0) ? FALSE : atoi (sptr);

		local_rec.printerNumber = atoi (argv [1]);
		sprintf (local_rec.lower,     	"%-12.12s", argv [2]);
		sprintf (local_rec.upper,     	"%-12.12s", argv [3]);
		sprintf (local_rec.zeroSupplied,"%-1.1s",	argv [4]);
		sprintf (local_rec.sortType, 	"%-1.1s",	argv [5]);
		sprintf (local_rec.printInclude,"%-1.1s",	argv [6]);
		sprintf (local_rec.printExclude,"%-1.1s",	argv [7]);
		sprintf (local_rec.transfer,  	"%-1.1s",	argv [8]);
		sprintf (local_rec.NoLevels,  	"%-1.1s",	argv [9]);

		init_scr ();

		OpenDB ();

		ReadMisc ();

		ScreenHeading ();

		local_rec.reportUsed = FALSE;

		Process ();

		if (PRN_INCLUDE || PRN_EXCLUDE)
		{
			if (local_rec.reportUsed)
			{
				fprintf (fout, ".EOF\n");
				pclose (fout);
			}
		}

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	else
	{
		init_scr 	();
		set_tty 	();
		set_masks 	();
		init_vars 	(1);

		OpenDB ();
		ReadMisc ();

		/*=====================
		| Reset control flags |
		=====================*/
		LoadDefault ();

		while (prog_exit == 0)
		{
			entry_exit	= FALSE;
			edit_exit 	= FALSE;
			prog_exit 	= FALSE;
			restart 	= FALSE;
			search_ok 	= TRUE;
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
            {
				shutdown_prog ();
                return (EXIT_SUCCESS);
            }
			prog_exit = 1;
		}

		sprintf (local_rec.printerString, "%d", local_rec.printerNumber);
		sprintf (local_rec.lower, "%1.1s%-11.11s",
						local_rec.startClass, local_rec.startCategory);

		sprintf (local_rec.upper, "%1.1s%-11.11s",
						local_rec.endClass, local_rec.endCategory);
		local_rec.sortType [1] = (char) NULL;

		shutdown_prog ();

		/*================================
		| Test for Overnight Processing. | 
		================================*/
		if (local_rec.onight [0] == 'Y') 
		{
			print_at (0,0,ML(mlStdMess035));
			fflush (stdout);
			execlp ("ONIGHT",
                    "ONIGHT", 
				    argv [0],
					local_rec.printerString,
					local_rec.lower,
					local_rec.upper, 
                    local_rec.zeroSupplied,
                    local_rec.sortType, 
                    local_rec.printInclude,
                    local_rec.printExclude, 
                    local_rec.transfer,
                    local_rec.NoLevels,
                    argv [1], (char *) 0);
		}
		/*====================================
		| Test for forground or background . |
		====================================*/
		else if (local_rec.back [0] == 'Y') 
		{
			if (fork () != 0)
			{
				clear ();
				print_at (0,0,ML(mlStdMess035));
				fflush (stdout);
			}
			else
            {
				execlp (argv [0], 
					     argv [0], 
					     local_rec.printerString,
					     local_rec.lower, 
					     local_rec.upper,
						 local_rec.zeroSupplied, 
					     local_rec.sortType,
						 local_rec.printInclude, 
					     local_rec.printExclude,
						 local_rec.transfer, 
					  	 local_rec.NoLevels,
					     (char *) 0);
            }
		}
		else 
		{
			clear ();
			print_at (0,0,ML(mlStdMess035));
			fflush (stdout);

			execlp (argv [0],
					argv [0],
					local_rec.printerString,
					local_rec.lower,
					local_rec.upper,
					local_rec.zeroSupplied,
					local_rec.sortType,
					local_rec.printInclude,
					local_rec.printExclude,
					local_rec.transfer, 
					local_rec.NoLevels,
					(char *) 0);
        }
        return (EXIT_SUCCESS);
	}
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
	abc_dbopen ("data");

	open_rec (excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (ffpr,  ffpr_list, FFPR_NO_FIELDS, "ffpr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inws,  inws_list, INWS_NO_FIELDS, "inws_id_no2");
	open_rec (inwd,  inwd_list, INWD_NO_FIELDS, "inwd_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, (local_rec.sortType [0] == 'I')
									 ? "inmr_id_no" : "inmr_id_no_3");
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_id_no2");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_id_date");
	open_rec (pohr,  pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (soic,  soic_list, soic_no_fields, "soic_id_no2");
	open_rec (ffwk,  ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no_2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (excf);
	abc_fclose (ffpr);
	abc_fclose (ffwk);
	abc_fclose (incc);
	abc_fclose (inws);
	abc_fclose (inwd);
	abc_fclose (inmr);
	abc_fclose (itln);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (soic);
	CloseCosting ();

	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no,	comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, 	comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	strcpy (supply_rec.br_no,	ccmr_rec.est_no);
	strcpy (supply_rec.wh_no,	ccmr_rec.cc_no);
	strcpy (supply_rec.wh_short,ccmr_rec.acronym);
	supply_rec.hhccHash	=	ccmr_rec.hhcc_hash;

	abc_selfield (ccmr, "ccmr_hhcc_hash");
}

void
LoadDefault (void)
{
	local_rec.printerNumber = 1;
	strcpy (local_rec.back, 			"N");
	strcpy (local_rec.backDesc, 		ML ("No "));
	strcpy (local_rec.onight, 			"N");
	strcpy (local_rec.onightDesc, 		ML ("No "));
	strcpy (local_rec.startClass, 		"A");
	strcpy (local_rec.startCategory, 	"           ");
	strcpy (local_rec.endClass, 		"Z");
	strcpy (local_rec.endCategory, 		"~~~~~~~~~~~");
	strcpy (local_rec.zeroSupplied, 	"Y");
	strcpy (local_rec.zeroSuppDesc, 	ML ("Yes"));
	strcpy (local_rec.sortType, 		"G");
	strcpy (local_rec.sortTypeDesc, 	ML ("Group"));
	strcpy (local_rec.printInclude, 	"Y");
	strcpy (local_rec.printIncludeDesc, ML ("Yes"));
	strcpy (local_rec.printExclude, 	"N");
	strcpy (local_rec.printExcludeDesc, ML ("No "));
	strcpy (local_rec.transfer, 		"N");
	strcpy (local_rec.transfer_desc, 	ML ("No "));
	strcpy (local_rec.NoLevels, 		"1");
}


int
spec_valid (
	int    field)
{
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML(mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
		{
			strcpy (local_rec.backDesc, "Yes");
			strcpy (local_rec.onightDesc, "No ");
			DSP_FLD ("onightDesc");
		}
		else
			strcpy (local_rec.backDesc, "No ");

		DSP_FLD("backDesc"); 
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
		{
			strcpy (local_rec.onightDesc, "Yes");
			strcpy (local_rec.backDesc, "No ");
			DSP_FLD ("backDesc");
		}
		else
			strcpy (local_rec.onightDesc, "No ");

		DSP_FLD("onightDesc"); 
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startCategory"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCategory, "           ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", local_rec.startCategory);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess004));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endCategory"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCategory, "~~~~~~~~~~~");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", local_rec.endCategory);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess004));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sortType"))
	{
		sprintf (local_rec.sortTypeDesc, "%-9.9s",
		    (local_rec.sortType [0] == 'G') ? "Group" : "Item No.");
		DSP_FLD ("sortTypeDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("zeroSupplied"))
	{
		if (local_rec.zeroSupplied [0] == 'Y')
			strcpy (local_rec.zeroSuppDesc, "Yes");
		else
			strcpy (local_rec.zeroSuppDesc, "No ");
		DSP_FLD ("zeroSuppDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printInclude"))
	{
		if (PRN_INCLUDE)
			strcpy (local_rec.printIncludeDesc, "Yes");
		else
			strcpy (local_rec.printIncludeDesc, "No ");
		DSP_FLD ("printIncludeDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printExclude"))
	{
		if (PRN_EXCLUDE)
			strcpy (local_rec.printExcludeDesc, "Yes");
		else
			strcpy (local_rec.printExcludeDesc, "No ");
		DSP_FLD ("printExcludeDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("transfer"))
	{
		if (local_rec.transfer [0] == 'Y')
			strcpy (local_rec.transfer_desc, "Yes");
		else
			strcpy (local_rec.transfer_desc, "No ");
		DSP_FLD ("transfer_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Category", "#Description");
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp(excf_rec.cat_no, key_val, strlen (key_val)) &&
		      !strcmp (excf_rec.co_no, comm_rec.co_no))
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

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

void
ScreenHeading (void)
{
	if (!PRN_INCLUDE && !PRN_EXCLUDE)
	{
		dsp_screen ("Generating Warehouse Replenishment Transfers",
			comm_rec.co_no,
			comm_rec.co_name);
		return;
	}

	dsp_screen ("Printing Warehouse Replenishment Report",
			comm_rec.co_no,
			comm_rec.co_name);
}

/*=====================================================================
| Start Out Put To Standard Print.
=====================================================================*/
void
HeadingOutput (void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s\n", DateToString (comm_rec.inv_date));
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);

	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L156\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EWAREHOUSE REPLENISHMENT REPORT\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".CREPLENISHMENT FROM %s - %s / %s -%s \n",
								comm_rec.est_no,
								comm_rec.est_name,
								comm_rec.cc_no,
								comm_rec.cc_name);
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n", SystemTime ());

	fprintf (fout, ".R==========================");
	fprintf (fout, "===========");
	fprintf (fout, "=====================");
	fprintf (fout, "========================");
	fprintf (fout, "========================");
	fprintf (fout, "========================");
	fprintf (fout, "===============\n");

	fprintf (fout, "==========================");
	fprintf (fout, "===========");
	fprintf (fout, "=====================");
	fprintf (fout, "========================");
	fprintf (fout, "========================");
	fprintf (fout, "========================");
	fprintf (fout, "===============\n");


	fprintf (fout, "|   ITEM NUMBER    | UOM. ");
	fprintf (fout, "|   WEEKS  ");
	fprintf (fout, "| SUPPLY TO WAREHOUSE");
	fprintf (fout, "|      AVAILABLE        ");
	fprintf (fout, "|    STOCK ON ORDER     ");
	fprintf (fout, "|     TOTAL COVER       ");
	fprintf (fout, "|   MINIMUM   |\n");

	fprintf (fout, "|                  |      ");
	fprintf (fout, "|  DEMAND  ");
	fprintf (fout, "|                    ");
	fprintf (fout, "|   QTY.     |  WEEKS   ");
	fprintf (fout, "|   COMPANY  |   W/H    ");
	fprintf (fout, "|   QTY.     |  WEEKS   ");
	fprintf (fout, "| REQUIREMENT |\n");

	fprintf (fout, "|------------------|------");
	fprintf (fout, "|----------");
	fprintf (fout, "|--------------------");
	fprintf (fout, "|------------|----------");
	fprintf (fout, "|------------|----------");
	fprintf (fout, "|------------|----------");
	fprintf (fout, "|-------------|\n");

	local_rec.reportUsed = TRUE;

	fflush (fout);
}

int
check_page (void)
{
	return (EXIT_SUCCESS);
}

void
Process (void)
{
	HeadingOutput ();
	strcpy  (inmr_rec.co_no,    comm_rec.co_no);
	sprintf (inmr_rec.inmr_class,    "%-1.1s",   local_rec.lower);
	sprintf (inmr_rec.category, "%-11.11s", local_rec.lower + 1);
	sprintf (inmr_rec.item_no,  "%-16.16s", " ");
	sprintf (excf_rec.cat_no,   "%-11.11s", " ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		if ((result = strstr (envVarSkInvlClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
		if (strcmp (excf_rec.cat_no, inmr_rec.category))
		{
			strcpy (excf_rec.co_no, comm_rec.co_no);
			sprintf (excf_rec.cat_no, "%-11.11s", inmr_rec.category);
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (inmr, &inmr_rec, NEXT, "r");
				continue;
			}
		}

		if (ValidateGroup())
		{
			ProcessItem 
			(
				inmr_rec.hhbr_hash,
				atoi (local_rec.NoLevels)
			);
		}

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

/*--------------------------
| Check that item belongs  |
| to a valid group that is |
| to be processed.         |
--------------------------*/
int
ValidateGroup (void)
{
	if (inmr_rec.inmr_class [0] < local_rec.lower [0])
		return (EXIT_SUCCESS);

	if (inmr_rec.inmr_class [0] == local_rec.lower [0] && 
	    strcmp (inmr_rec.category, local_rec.lower + 1) < 0)
	{
		return (EXIT_SUCCESS);
	}

	/*---------------
	| Valid Class	|
	---------------*/
	if (inmr_rec.inmr_class [0] < local_rec.upper [0])
		return (EXIT_FAILURE);

	/*---------------
	| Invalid Class	|
	---------------*/
	if (inmr_rec.inmr_class [0] > local_rec.upper [0])
		return (EXIT_SUCCESS);

	/*---------------------------------------
	| Classes Sames but Invalid Category	|
	---------------------------------------*/
	if (strcmp (inmr_rec.category, local_rec.upper + 1) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

/*---------------
| Process item. |
---------------*/
void
ProcessItem (
	long   hhbrHash,
	int    NoLevels)
{
	int		ErrorFound	=	TRUE,
			i;

	dsp_process ("Item", inmr_rec.item_no);

	/*-------------------------------------------------
	| Process on all levels for exact match for item. |
	-------------------------------------------------*/
	for (i = 0; i < NoLevels && ErrorFound; i++)
	{
		inws_rec.hhbr_hash	=	hhbrHash;
		inws_rec.hhcf_hash	=	0L;
		inws_rec.hhcc_hash	= 	supply_rec.hhccHash;
		sprintf (inws_rec.sup_priority, "%d", NoLevels);
		ErrorFound = find_rec (inws, &inws_rec, COMPARISON, "r");
	}
	/*-----------------------------------------------------
	| Process on all levels for exact match for category. |
	-----------------------------------------------------*/
	if (ErrorFound)
	{
		for (i = 0; i < NoLevels && ErrorFound; i++)
		{
			inws_rec.hhbr_hash	=	0L;
			inws_rec.hhcf_hash	=	excf_rec.hhcf_hash;
			inws_rec.hhcc_hash	= 	supply_rec.hhccHash;
			sprintf (inws_rec.sup_priority, "%d", NoLevels);
			ErrorFound = find_rec (inws, &inws_rec, COMPARISON, "r");
		}
	}
	/*-----------------------------------
	| Process on all levels for Global. |
	-----------------------------------*/
	if (ErrorFound)
	{
		for (i = 0; i < NoLevels && ErrorFound; i++)
		{
			inws_rec.hhcc_hash	= 	supply_rec.hhccHash;
			sprintf (inws_rec.sup_priority, "%d", NoLevels);
			inws_rec.hhbr_hash	=	0L;
			inws_rec.hhcf_hash	=	0L;
			ErrorFound = find_rec (inws, &inws_rec, COMPARISON, "r");
		}
	}
	if (ErrorFound)
		return;

	PrintUpdateItem (hhbrHash);

	/*----------------------------------------------------
	| Print item if it should  be included in the report |
	----------------------------------------------------*/
	if (PRN_INCLUDE || PRN_EXCLUDE)
		PrintUpdateItem (hhbrHash);
}

/*--------------------------------
| Lookup details and store data. |
--------------------------------*/
int
ProcessData (
	long   hhbrHash,
	long   hhccHash)
{
	float	realCommitted	=	0.00;
	double	UpliftValue		=	0.00;

	incc2_rec.hhbr_hash	=	hhbrHash;
	incc2_rec.hhcc_hash	=	hhccHash;
	cc = find_rec (incc, &incc2_rec, COMPARISON, "r");
	if (cc)
		return (cc);

	lrpRec.costPrice	=	FindCost ();

	/*---------------------------------------
	| Find out what the review-period		|
	| is for this product. Firstly, try for	|
	| a match on branch/item. Then try for	|
	| a match on item. Then try for a match	|
	| on branch/category. If this fails,	|
	| then use LRP_DFLT_REVIEW environment-	|
	| value. If not found, dflt to 4 weeks.	|
	---------------------------------------*/
	ffpr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (ffpr_rec.br_no, ccmr_rec.est_no);
	cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
	if (cc)
	{
	    strcpy (ffpr_rec.br_no, "  ");
	    cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
	    if (cc)
	    {
			abc_selfield (ffpr, "ffpr_id_no_1");
			strcpy (ffpr_rec.category, inmr_rec.category);
			strcpy (ffpr_rec.br_no, ccmr_rec.est_no);
			cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
			if (cc)
			{
				strcpy (ffpr_rec.br_no, "  ");
				cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
				if (cc)
					ffpr_rec.review_prd = envVarLrpDfltReview;
			}
			abc_selfield (ffpr, "ffpr_id_no");
	    }
	}

	lrpRec.hhccHash = ccmr_rec.hhcc_hash;
	sprintf (lrpRec.acronym, "%-9.9s", ccmr_rec.acronym);

	/*---------------------------------
	| Calculate Actual Qty Committed. |
	---------------------------------*/
	realCommitted = RealTimeCommitted (incc2_rec.hhbr_hash,incc2_rec.hhcc_hash);

	lrpRec.onHand			= 	incc2_rec.closing_stock;
	lrpRec.committed		= 	incc2_rec.committed + 
								realCommitted +
								incc2_rec.backorder;
	lrpRec.safetyStock		= 	incc2_rec.safety_stock;
	lrpRec.minimumOrder		= 	inws_rec.min_order;
	lrpRec.normalOrder		= 	inws_rec.norm_order;
	lrpRec.upliftPc			= 	inwd_rec.upft_pc;
	lrpRec.upliftAmt		= 	inwd_rec.upft_amt;
	lrpRec.orderMultiple	= 	inws_rec.ord_multiple;
	lrpRec.leadTimes		= 	0.00;

	/*--------------------
	| Lead time by Land. |
	--------------------*/
	if (inwd_rec.dflt_lead [0] == 'L')
		lrpRec.leadTimes	= 	inwd_rec.lnd_time;

	/*--------------------
	| Lead time by Land. |
	--------------------*/
	else if (inwd_rec.dflt_lead [0] == 'S')
		lrpRec.leadTimes	= 	inwd_rec.sea_time;

	/*--------------------
	| Lead time by Land. |
	--------------------*/
	else if (inwd_rec.dflt_lead [0] == 'A')
		lrpRec.leadTimes	= 	inwd_rec.air_time;

	lrpRec.reviewPeriod	= 	ffpr_rec.review_prd;
	lrpRec.weeksDemand	= 	incc2_rec.wks_demand;
	lrpRec.onOrder		= 	CheckInwardsGoods (lrpRec.leadTimes);
	strcpy (lrpRec.allowReport, incc2_rec.allow_repl);

	UpliftValue	=	0.00;
	if (inwd_rec.upft_pc > 0.00)
	{
		UpliftValue	=	(double) inwd_rec.upft_pc;
		UpliftValue	*=	lrpRec.costPrice;
		UpliftValue	=	DOLLARS (UpliftValue);
		UpliftValue	=	twodec (UpliftValue);
	}
	if (inwd_rec.upft_amt > 0.00)
		UpliftValue	+=	DOLLARS (inwd_rec.upft_amt);

	lrpRec.totalUplift	=	UpliftValue;
	return (EXIT_SUCCESS);
}

void
PrintUpdateItem (
	long	hhbrHash)
{
	float	weeksAvailable	=	0.0,
			weeksCover		=	0.0,
			qtyAvailable	=	0.0,
			qtyCover		=	0.0,
			minRequired		=	0.0,
			leadWeeks		=	0.0;


	inwd_rec.inws_hash	=	inws_rec.inws_hash;
	inwd_rec.hhbr_hash	=	inws_rec.hhbr_hash;
	inwd_rec.hhcf_hash	=	inws_rec.hhcf_hash;
	inwd_rec.hhcc_hash	=	0L;
	cc = find_rec (inwd, &inwd_rec, GTEQ, "r");

	while (!cc && inwd_rec.inws_hash == inws_rec.inws_hash &&
				  inwd_rec.hhbr_hash == inws_rec.hhbr_hash &&
				  inwd_rec.hhcf_hash == inws_rec.hhcf_hash)
	{

		ccmr_rec.hhcc_hash	=	inwd_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
			continue;
		}
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
			continue;
		}

		if (ProcessData (hhbrHash, inwd_rec.hhcc_hash))
		{
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
			continue;
		}

		/*-------------------------------------------------------
		| Bypass if printInclude != Y AND item is set as include.	|
		-------------------------------------------------------*/
		if (!PRN_INCLUDE && lrpRec.allowReport [0] == 'I')
		{
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
			continue;
		}

		qtyAvailable 	= 	lrpRec.onHand - lrpRec.committed;
		qtyAvailable 	+= 	lrpRec.onOrder;
		weeksAvailable 	= 	qtyAvailable;

		if (lrpRec.weeksDemand > 0.00)
			weeksAvailable /= lrpRec.weeksDemand;
		else
			weeksAvailable = 0.00;

		leadWeeks 	=	lrpRec.leadTimes / 7;

		weeksCover	=	lrpRec.safetyStock + 
						leadWeeks + 
						lrpRec.reviewPeriod;

		qtyCover	=	weeksCover;
		qtyCover	*=	lrpRec.weeksDemand;

		minRequired	=	qtyCover - qtyAvailable;

		if (minRequired > 0)
		{
			if (minRequired < lrpRec.minimumOrder)
				minRequired = lrpRec.minimumOrder;

			if (minRequired < lrpRec.normalOrder)
				minRequired = lrpRec.normalOrder;

			minRequired = 	Rnd_Mltpl 
							(
								minRequired, 
								envVarSupOrdRound, 
								lrpRec.orderMultiple, 
								lrpRec.minimumOrder
							);
		}
		if (minRequired < 0.0)
			minRequired = 0.00;

		if (local_rec.zeroSupplied [0] == 'Y' && minRequired <= 0.0)
		{
			cc = find_rec (inwd, &inwd_rec, NEXT, "r");
			continue;
		}

		fprintf (fout, "| %-16.16s | %4.4s ",
										inmr_rec.item_no, inmr_rec.sale_unit);
		fprintf (fout, "|%9.2f ", 		lrpRec.weeksDemand);
		fprintf (fout, "| %s/%s %9.9s    ", ccmr_rec.est_no,
											ccmr_rec.cc_no,
											ccmr_rec.acronym);
		fprintf (fout, "|%11.1f |%9.2f ",	qtyAvailable,			
											weeksAvailable);
		fprintf (fout, "|%11.1f |%9.2f ",	inmr_rec.on_order,
											lrpRec.onOrder);
		fprintf (fout, "|%11.1f |%9.2f ",	qtyCover,		
											weeksCover);
		fprintf (fout, "|%12.1f |\n",		minRequired);

		if ((local_rec.transfer [0] == 'Y' || local_rec.transfer [0] == 'y') &&
			minRequired > 0.00)
		{
			ffwk_rec.hhcc_hash	=	supply_rec.hhccHash;
			ffwk_rec.hhbr_hash	=	hhbrHash;
			ffwk_rec.hhsu_hash	=	0L;
			ffwk_rec.hhpo_hash	=	0L;
			ffwk_rec.hhit_hash	=	0L;
			sprintf (ffwk_rec.filename , "TRN FROM %s/%s",
										supply_rec.br_no,
										supply_rec.wh_no);

			if (local_rec.sortType [0] == 'G')
				sprintf (ffwk_rec.sort,"%-1.1s%-11.11s%-16.16s",
										inmr_rec.inmr_class,
										inmr_rec.category,
										inmr_rec.item_no);
			else
				sprintf (ffwk_rec.sort,"%-16.16s%-18.18s", 
										inmr_rec.item_no," ");

			sprintf (ffwk_rec.crd_no, "%s%s%s", 
									ccmr_rec.co_no,
									ccmr_rec.est_no,
									ccmr_rec.cc_no);
									
			ffwk_rec.r_hhcc_hash	=	ccmr_rec.hhcc_hash;
			ffwk_rec.review_pd		=	1.00;
			ffwk_rec.wks_demand		=	lrpRec.weeksDemand;	
			ffwk_rec.sugg_qty		=	minRequired;
			ffwk_rec.order_qty		=	minRequired;
			ffwk_cons [0]			=	qtyAvailable;
			ffwk_cons [1]			=	lrpRec.onOrder;
			ffwk_cons [2]			=	weeksCover;
			ffwk_cons [3]			=	qtyCover;
			ffwk_cons [4]			=	qtyCover - qtyAvailable;
			ffwk_cons [5]			=	qtyCover - qtyAvailable;
			ffwk_cons [6]			=	0.00;
			ffwk_cons [7]			=	0.00;
			ffwk_cons [8]			=	0.00;
			ffwk_cons [9]			=	0.00;
			ffwk_cons [10]			=	0.00;
			ffwk_cons [11]			=	0.00;
			ffwk_cons [12]			=	0.00;
			ffwk_rec.cost_price		=	lrpRec.costPrice;
			ffwk_rec.uplift_amt		=	lrpRec.totalUplift;
			strcpy (ffwk_rec.source, "W");

			memcpy 
			(
				(char *)&ffwk2_rec,
				(char *)&ffwk_rec,
				sizeof (struct ffwkRecord)
			);

			cc = find_rec (ffwk, &ffwk2_rec, COMPARISON, "u");
			if (cc)
			{
				cc = abc_add (ffwk, &ffwk_rec);
				if (cc)
					file_err (cc, ffwk, "DBADD");
			}
			else
			{
				cc = abc_update (ffwk, &ffwk_rec);
				if (cc)
					file_err (cc, ffwk, "DBADD");
			}
		}
		cc = find_rec (inwd, &inwd_rec, NEXT, "r");
	}
}


double
FindCost (void)
{
	double	CostValue	=	0.00;

	switch (inmr_rec.costing_flag [0])
	{
	case 'L':
	case 'A':
	case 'P':
	case 'T':
		CostValue	=	FindIneiCosts
						(
							inmr_rec.costing_flag,
							comm_rec.est_no,
							inmr_rec.hhbr_hash
						);
		break;

	case 'F':
		CostValue	= 	FindIncfValue 
						(
							incc_rec.hhwh_hash,
							incc_rec.closing_stock, 
							TRUE, 
							TRUE,
							inmr_rec.dec_pt
						);
		break;

	case 'I':
		CostValue	= 	FindIncfValue 
						(
							incc_rec.hhwh_hash,
							incc_rec.closing_stock, 
							TRUE, 
							FALSE,
							inmr_rec.dec_pt
						);
		break;

	case 'S':
		CostValue	= FindInsfValue (incc_rec.hhwh_hash, TRUE);
		break;
	}
	return (CostValue);
}

/*====================================================
| Calculate the "On-Order" qty for the cover period. |
====================================================*/
float	
CheckInwardsGoods (
	float  leadTimes)
{
	float	coverRequired	= 0.00,
			orderQty 		= 0.00;

	/*-------------------------------
	| Calculate how many days fwd	|
	| to look for 'on-order'.		|
	-------------------------------*/
	coverRequired = incc_rec.safety_stock + ffpr_rec.review_prd;
	coverRequired *= 7;
	coverRequired += leadTimes;

	itln_rec.hhbr_hash 	= incc_rec.hhbr_hash;
	itln_rec.due_date 	= 0L;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc &&
           itln_rec.hhbr_hash == incc_rec.hhbr_hash &&
           itln_rec.due_date <= (comm_rec.inv_date + coverRequired))
	{
		switch (itln_rec.status [0])
		{
		case	'B':
		case	'M':
		case	'T':
		case	'U':
			if (itln_rec.r_hhcc_hash == incc_rec.hhcc_hash)
			{
				orderQty += itln_rec.qty_order;
				orderQty += itln_rec.qty_border;
			}
			break;

		default:
			break;
		}
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}

	poln_rec.hhbr_hash 	= incc_rec.hhbr_hash;
	poln_rec.due_date 	= 0L;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while
	(
		!cc &&
		poln_rec.hhbr_hash == incc_rec.hhbr_hash &&
		poln_rec.due_date <= (comm_rec.inv_date + coverRequired)
	)
	{
		if (poln_rec.hhcc_hash == incc_rec.hhcc_hash)
		{
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
			if (pohr_rec.drop_ship [0] != 'Y')
				orderQty += poln_rec.qty_ord - poln_rec.qty_rec;
		}
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	return (orderQty);
}

static float
Rnd_Mltpl (
	float	orderQty,
	char*	roundType,
	float	orderMultiple,
	float	minimumQty)
{
	double	workQty			=	0.0,
			roundUpQty		=	0.0,
			roundDownQty	=	0.0;

	if (orderQty < 1.00)
		return (0.00);

	if (orderMultiple == 0.00)
		return ((orderQty < minimumQty) ? minimumQty : orderQty);

	orderQty -= minimumQty;
	if (orderQty < 0.00)
		orderQty = 0.00;

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	workQty = (double) (orderQty / orderMultiple);
	if (ceil (workQty) == workQty)
		return (orderQty);

	/*------------------
	| Perform Rounding |
	------------------*/
	switch (roundType [0])
	{
	case 'U':
		/*------------------------------
		| Round Up To Nearest Multiple |
		------------------------------*/
		workQty 	= (double) (orderQty / orderMultiple);
		workQty 	= ceil (workQty);
		orderQty 	= (float) (workQty * orderMultiple);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		workQty 	= (double) (orderQty / orderMultiple);
		workQty 	= floor (workQty);
		orderQty 	= (float) (workQty * orderMultiple);
		break;

	case 'B':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		roundUpQty 	= (double) orderQty;
		workQty 	= (roundUpQty / (double)orderMultiple);
		workQty 	= ceil (workQty);
		roundUpQty 	= (float) (workQty * orderMultiple);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		roundDownQty 	= (double) orderQty;
		workQty 		= (roundDownQty / (double) orderMultiple);
		workQty 		= floor (workQty);
		roundDownQty 	= (float) (workQty * orderMultiple);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((roundUpQty - (double) orderQty) <= 
						 ((double) orderQty - roundDownQty))
			orderQty 	= (float) roundUpQty;
		else
			orderQty 	= (float) roundDownQty;

		break;

	default:
		break;
	}
	return (orderQty);
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
		rv_pr (ML(mlLrpMess025), 24, 0, 1);

		box (0, 3, 80, 11);
		line_at (1, 0,80);
		line_at (6, 1,79);
		line_at (9, 1,79);
		line_at (19,0,80);

		strcpy(err_str,ML(mlStdMess038));
		print_at (20,1,err_str, comm_rec.co_no, comm_rec.co_name);

		strcpy(err_str,ML(mlStdMess039));
		print_at (21,1,err_str, comm_rec.est_no, comm_rec.est_name);

		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
