/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_np_disp.c,v 5.4 2002/07/17 09:57:56 scott Exp $
|  Program Name  : (sk_np_disp.c)                                     |
|  Program Desc  : (Stock Number Plate Display.                )      |
|---------------------------------------------------------------------|
| $Log: sk_np_disp.c,v $
| Revision 5.4  2002/07/17 09:57:56  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/11/05 01:40:46  scott
| Updated from Testing.
|
| Revision 5.2  2001/08/09 09:19:21  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:26  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:48  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:08  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.5  2000/12/20 05:37:40  ramon
| Updated to remove the errors when compiled in LS10-GUI.
|
| Revision 3.4  2000/12/06 06:03:00  scott
| Updated to allow for number plate details to be a one:many relationship
| with locations due to split status flags.
| This means that the field sknd_inlo_hash can not longer be used and will be
| removed.
|
| Revision 3.3  2000/12/05 10:01:37  scott
| Updated to remove container and replace with customer order ref.
|
| Revision 3.2  2000/11/20 07:40:17  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.1  2000/11/10 00:50:35  scott
| Updated to change open of inmr from using id_no to hhbr_hash
|
| Revision 3.0  2000/10/10 12:20:45  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 1.2  2000/09/01 03:07:19  scott
| First release of number plate print/display
|
| Revision 1.1  2000/08/22 05:29:18  scott
| New Programs - Number plate display and print.
|
*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_np_disp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_np_disp/sk_np_disp.c,v 5.4 2002/07/17 09:57:56 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

#define	DSP_NP		 (npDisp)

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct skndRecord	sknd_rec;
struct skniRecord	skni_rec;
struct sknhRecord	sknh_rec;
struct inloRecord	inlo_rec;
struct llstRecord	llst_rec;

#define	INT_MAX_LINES	15

	int		printerNumber	 	= 0,
			npDisp			 	= 0,
			npPrnt			 	= 0,
			envSkGrinNoPlate 	= 0,
			envVarthreePlSystem = 0,
			first_loc 		 	= TRUE;	

	FILE	*fout;

	char	lowerNoPlate [sizeof (sknh_rec.plate_no)], 
			upperNoPlate [sizeof (sknh_rec.plate_no)]; 

	char	*noteBlank = "                                                            ";
	char	disp_str [300];


#include	<LocHeader.h>

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	noPlateFrom [sizeof (sknh_rec.plate_no)];
	char	noPlateTo [sizeof (sknh_rec.plate_no)];
	int		printerNumber;
	char	printerString [3];
	char 	back [6];
	char	onite [6];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "noPlateFrom",	 4, 22, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Number Plate From :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.noPlateFrom},
	{1, LIN, "noPlateTo",	 5, 22, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Number Plate To   :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.noPlateTo},
	{1, LIN, "printerNumber",		7, 22, INTTYPE,
		"NN", "          ",
		" ", "1",        "Printer Number    :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",		8, 22, CHARTYPE,
		"U", "          ",
		" ", "N (o",      "Background        :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	9, 22, CHARTYPE,
		"U", "          ",
		" ", "N (o",      "Overnight         :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void	SrchSknh 		 (char *);
void	ReadMisc 		 (void);
void	OpenDB 			 (void);
void	CloseDB 		 (void);
void	RunProgram 		 (char *);
void 	NoPlateHeading 	 (void);
void 	NoPlateDisplay 	 (void);
void 	NoPlatePrint 	 (void);
int  	heading 		 (int);
void 	shutdown_prog 	 (void);
int  	spec_valid 		 (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "sk_np_disp"))
		npDisp = TRUE;

	if (!strcmp (sptr, "sk_np_prnt"))
		npPrnt = TRUE;

	envSkGrinNoPlate = (sptr = chk_env ("SK_GRIN_NOPLATE")) ? atoi (sptr) : 1;

	/*
	 * Check for 3pl Environment.
	 */
	sptr = chk_env ("PO_3PL_SYSTEM");
	envVarthreePlSystem = (sptr == (char *)0) ? 0 : atoi (sptr);

	OpenDB ();

	/*-----------------------------------------------
	| No point in running reports if not multi bin. |
	-----------------------------------------------*/
    if (!envSkGrinNoPlate)
	{
		shutdown_prog ();
        return (EXIT_FAILURE);
    }
	/*-----------------------------------------
	| Print locations by class/category/item. |
	-----------------------------------------*/
	if (npPrnt && argc == 4)
	{
		printerNumber = atoi (argv [1]);
		sprintf (lowerNoPlate, "%-15.15s", argv [2]);
		sprintf (upperNoPlate, "%-15.15s", argv [3]);

		dsp_screen ("Processing : Stock Location Report.", 
					comm_rec.co_no, comm_rec.co_name);

		local_rec.printerNumber = printerNumber;
		NoPlateHeading ();
		NoPlatePrint ();
		fprintf (fout,".EOF\n");

		/*========================= 
		| Program exit sequence	. |
		=========================*/
		pclose (fout);

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Print/display locations by location master. |
	---------------------------------------------*/
	if (npDisp && argc == 4)
	{
		local_rec.printerNumber = atoi (argv [1]);
		sprintf (lowerNoPlate, "%-15.15s", argv [2]);
		sprintf (upperNoPlate, "%-15.15s", argv [3]);

		init_scr ();
		swide ();
		set_tty ();

		print_at (20,0, ML (mlStdMess038), 
					comm_rec.co_no, comm_rec.co_name);
	
		print_at (21,0, ML (mlStdMess039), 
					comm_rec.est_no, comm_rec.est_name);

		print_at (22,0, ML (mlStdMess099), 
					comm_rec.cc_no, comm_rec.cc_name);

		move (0,23);
		line (130);

		NoPlateHeading ();
		NoPlateDisplay ();
		Dsp_srch ();
		Dsp_close ();
		
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	/*========================= 
	| Program exit sequence	. |
	=========================*/
	init_scr 	 ();
	swide 		 ();
	set_tty 	 ();
	set_masks 	 ();
	init_vars 	 (1);

	if (DSP_NP)
	{
		FLD ("printerNumber")   = ND;
		FLD ("back")   			= ND;
		FLD ("onight") 			= ND;
	}

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart 	= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		RunProgram (argv [0]);

		prog_exit	= TRUE;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*==============================================
| Number plate heading for report and display. |
==============================================*/
void
NoPlateHeading (void)
{
	char	head_text [300];

	if (DSP_NP)
	{
		sprintf (head_text, " Number Plate from (%15.15s) To (%s15.15) ",
				lowerNoPlate, upperNoPlate);

		Dsp_prn_open (0, 0, 15, head_text, 
					comm_rec.co_no, comm_rec.co_name,
					comm_rec.est_no, comm_rec.est_name,
					comm_rec.cc_no, comm_rec.cc_name);

		sprintf (err_str, " NUMBER PLATE DETAILS (FROM NUMBER PLATE : %s TO NUMBER PLATE : %s)                                   ", lowerNoPlate, upperNoPlate);
		Dsp_saverec (err_str);

		Dsp_saverec ("");
		Dsp_saverec (" [REDRAW] [NEXT] [PREV] [EDIT/END] ");
		return;
	}
	
	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((fout = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "POPEN");
	
	/*=======================
	| Start output to file. |
	=======================*/
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.printerNumber);
	fprintf (fout, ".12\n");
	fprintf (fout, ".L158\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".ESTOCK LOCATION MASTER REPORT.\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s \n",clip (comm_rec.co_name));
	fprintf (fout, ".EBRANCH: %s \n",clip (comm_rec.est_name));
	fprintf (fout, ".EWAREHOUSE: %s \n",clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E AS AT : %s\n",SystemTime ());

	fprintf (fout, ".R=======");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=======");
	fprintf (fout, "============");
	fprintf (fout, "==============");
	fprintf (fout, "===========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "================================\n");

	fprintf (fout, "=======");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=======");
	fprintf (fout, "============");
	fprintf (fout, "==============");
	fprintf (fout, "===========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "================================\n");

	fprintf (fout, ".PI12\n");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	ReadMisc ();

	open_rec (sknh, sknh_list, SKNH_NO_FIELDS, "sknh_id_no");
	open_rec (sknd, sknd_list, SKND_NO_FIELDS, "sknd_id_no");
	open_rec (skni, skni_list, SKNI_NO_FIELDS, "skni_sknd_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_sknd_hash");
	open_rec (llst, llst_list, LLST_NO_FIELDS, "llst_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sknh);
	abc_fclose (sknd);
	abc_fclose (skni);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inlo);
	abc_fclose (llst);
	abc_dbclose ("data");
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,	comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	abc_fclose (ccmr);
}

/*=============================
| Special validation section. |
=============================*/
int
spec_valid (
 int field)
{
	/*--------------------------
	| Validate Location from . |
	--------------------------*/
	if (LCHECK ("noPlateFrom"))
	{
		if (SRCH_KEY)
		{
			SrchSknh (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (local_rec.noPlateFrom, " ");
			DSP_FLD ("noPlateFrom");
			return (EXIT_SUCCESS);
		}
		strcpy (sknh_rec.co_no,		comm_rec.co_no);
		strcpy (sknh_rec.br_no,		comm_rec.est_no);
		strcpy (sknh_rec.plate_no,	local_rec.noPlateFrom);
		cc = find_rec (sknh, &sknh_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML ("Number plate not found"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Location from . |
	--------------------------*/
	if (LCHECK ("noPlateTo"))
	{
		if (SRCH_KEY)
		{
			SrchSknh (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (local_rec.noPlateTo, "~");
			DSP_FLD ("noPlateTo");
			return (EXIT_SUCCESS);
		}
		strcpy (sknh_rec.co_no,		comm_rec.co_no);
		strcpy (sknh_rec.br_no,		comm_rec.est_no);
		strcpy (sknh_rec.plate_no,	local_rec.noPlateTo);
		cc = find_rec (sknh, &sknh_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML ("Number plate not found"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*==========================
	| Validate printer number. |
	==========================*/
	if (LCHECK ("printerNumber"))
	{
		if (F_HIDE (label ("printerNumber")))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back, (local_rec.back [0] == 'Y') ? "Y (es" : "N (o ");
		DSP_FLD ("back");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onite, (local_rec.onite [0] == 'Y') ? "Y (es" : "N (o ");
		DSP_FLD ("onight");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*======================================
| Number plate processing for Display. |
======================================*/
void
NoPlateDisplay (void)
{
	char	formatStr [151],
			workStr [151];
	int		lineNo	=	0;

	float	noPlateQtyRemain = 0.00;
	int		i,
			intLineCounter	 = 0;

	strcpy (sknh_rec.co_no,		comm_rec.co_no);
	strcpy (sknh_rec.br_no,		comm_rec.est_no);
	strcpy (sknh_rec.plate_no,	lowerNoPlate);
	cc = find_rec (sknh, &sknh_rec, GTEQ, "r");
	while (	!cc && 
			!strcmp (sknh_rec.co_no, comm_rec.co_no) &&
			!strcmp (sknh_rec.br_no, comm_rec.est_no) &&
			 strncmp (sknh_rec.plate_no,upperNoPlate,15) <= 0)
	{
		intLineCounter	 = 0;

		strcpy (formatStr, ML (" NUMBER PLATE (%s)  / RECEIPT DATE %10.10s / DESC : %-60.60s "));

		sprintf 
		 (
			workStr, 
			formatStr, 
			sknh_rec.plate_no,
			DateToString (sknh_rec.rec_date),
			sknh_rec.lab_note1
		);
		Dsp_saverec (workStr);
		intLineCounter++;

		strcpy (formatStr, "%65.65s : %60.60s ");
		if (strcmp (sknh_rec.lab_note2, noteBlank))
		{
			sprintf 
			 (
				workStr, 
				formatStr, 
				" ",
				sknh_rec.lab_note2
			);
			Dsp_saverec (workStr);
			intLineCounter++;
		}
		if (strcmp (sknh_rec.lab_note3, noteBlank))
		{
			sprintf 
			 (
				workStr, 
				formatStr, 
				" ",
				sknh_rec.lab_note3
			);
			Dsp_saverec (workStr);
			intLineCounter++;
		}
		if (strcmp (sknh_rec.lab_note4, noteBlank))
		{
			sprintf 
			 (
				workStr, 
				formatStr, 
				" ",
				sknh_rec.lab_note4
			);
			Dsp_saverec (workStr);
			intLineCounter++;
		}
		if (strcmp (sknh_rec.lab_note5, noteBlank))
		{
			sprintf 
			 (
				workStr, 
				formatStr, 
				" ",
					sknh_rec.lab_note5
			);
			Dsp_saverec (workStr);
			intLineCounter++;
		}
		intLineCounter +=3;
		if (envVarthreePlSystem)
		{
			Dsp_saverec ("^^GGGGIGGGGGGGGGGGGGGGGIGGGGGGGGGGGIGGGGGGGGGGGIGGGGGGGGGGGIGGGGGGGGGGGIGGGGGGGGGGGIGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGG");
			Dsp_saverec ("LINE^E   ITEM NUMBER  ^E UNIT QTY. ^E  PACK QTY ^ECHARGE  WGT^E TOT GROSS ^ETOTAL  CBM ^E LOCATION ^E  LOCATION STATUS   ^E ORDER REF.    ");
			Dsp_saverec ("^^GGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGHGGGGGGGGGGGHGGGGGGGGGGGHGGGGGGGGGGGHGGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGG");
		}
		else
		{
			Dsp_saverec ("^^GGGGGGIGGGGGGGGGGGGGGGGGGIGGGGGGIGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGG");
			Dsp_saverec (" LINE ^E    ITEM NUMBER   ^E UOM. ^E PACKAGE QTY. ^EUNIT QTY REMAINING^E  LOCATION  ^E   LOCATION STATUS    ^E    ORDER REFERENCE       ");
			Dsp_saverec ("^^GGGGGGHGGGGGGGGGGGGGGGGGGHGGGGGGHGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGG");
		}
		sknd_rec.sknh_hash	=	sknh_rec.sknh_hash;
		sknd_rec.line_no	=	0;
		cc = find_rec (sknd, &sknd_rec, GTEQ, "r");
		while (!cc && sknd_rec.sknh_hash == sknh_rec.sknh_hash)
		{
			inmr_rec.hhbr_hash	=	sknd_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (sknd, &sknd_rec, NEXT, "r");
				continue;
			}
			inum_rec.hhum_hash	=	sknd_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (sknd, &sknd_rec, NEXT, "r");
				continue;
			}

			/*
			 * Set quantity remaining from number plate detail.
			 */
			noPlateQtyRemain	=	sknd_rec.qty_rec - sknd_rec.qty_return;

			/*
			 * Subtract the amount(s) already issued.
			 */
			skni_rec.sknd_hash = sknd_rec.sknd_hash;
			cc = find_rec (skni, &skni_rec, GTEQ, "r");
			while (!cc && skni_rec.sknd_hash == sknd_rec.sknd_hash)
			{
				noPlateQtyRemain	-=	skni_rec.qty_issued;
				cc = find_rec (skni, &skni_rec, NEXT, "r");
			}
			if (noPlateQtyRemain <= 0.00)
				noPlateQtyRemain = 0.00;

			inlo_rec.sknd_hash	=	sknd_rec.sknd_hash;
			cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
			while (!cc && inlo_rec.sknd_hash == sknd_rec.sknd_hash)
			{
				strcpy (llst_rec.co_no, comm_rec.co_no);
				strcpy (llst_rec.code,  inlo_rec.loc_status);
				cc = find_rec (llst, &llst_rec, EQUAL, "r");
				if (cc)
					strcpy (llst_rec.desc, " ");

				if (envVarthreePlSystem)
				{
					sprintf 
					(
						workStr, 
						"%04d^E%16.16s^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.10s^E%20.20s^E%-15.15s",
						sknd_rec.line_no,
						inmr_rec.item_no,
						inlo_rec.rec_qty,
						inlo_rec.pack_qty,
						inlo_rec.chg_wgt,
						inlo_rec.gross_wgt,
						inlo_rec.cu_metre,
						inlo_rec.location,
						llst_rec.desc,
						sknd_rec.cus_ord_ref
					);
				}
				else
				{
					sprintf 
					(
						workStr, 
						" %04d ^E %16.16s ^E %4.4s ^E%13.2f ^E%17.2f ^E %10.10s ^E %20.20s ^E %-19.19s ",
						sknd_rec.line_no,
						inmr_rec.item_no,
						inlo_rec.uom,
						sknd_rec.qty_rec - sknd_rec.qty_return / inlo_rec.cnv_fct,
						noPlateQtyRemain,
						inlo_rec.location,
						llst_rec.desc,
						sknd_rec.cus_ord_ref
					);
				}
				Dsp_saverec (workStr);
				intLineCounter++;
				cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			}

			cc = find_rec (sknd, &sknd_rec, NEXT, "r");
		}
		sprintf 
		(
			workStr, 
			"^1 END OF NUMBER PLATE (%s) ^6",
			sknh_rec.plate_no
		);
		intLineCounter++;
		Dsp_saverec (workStr);
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		intLineCounter++;

		lineNo	=	(intLineCounter % INT_MAX_LINES);
		if (lineNo)
		{
			for (i = lineNo; i < INT_MAX_LINES; i++)
				Dsp_saverec (" ");
		}

		cc = find_rec (sknh, &sknh_rec, NEXT, "r");
	}
}
/*====================================
| Number plate processing for Print. |
====================================*/
void
NoPlatePrint (void)
{
	char	formatStr [151];

	strcpy (sknh_rec.co_no,		comm_rec.co_no);
	strcpy (sknh_rec.br_no,		comm_rec.est_no);
	strcpy (sknh_rec.plate_no,	lowerNoPlate);
	cc = find_rec (sknh, &sknh_rec, GTEQ, "r");
	while (	!cc && 
			!strcmp (sknh_rec.co_no, comm_rec.co_no) &&
			!strcmp (sknh_rec.br_no, comm_rec.est_no) &&
			 strncmp (sknh_rec.plate_no,upperNoPlate,15) <= 0)
	{
		strcpy (formatStr, ML ("| NUMBER PLATE (%s)  / RECEIPT DATE %10.10s / DESC       : %-80.80s |\n"));

		fprintf 
		 (
			fout,
			formatStr, 
			sknh_rec.plate_no,
			DateToString (sknh_rec.rec_date),
			sknh_rec.lab_note1
		);
		if (strcmp (sknh_rec.lab_note2, noteBlank))
		{
			fprintf 
			 (
				fout,  
				"|%71.71s : %-80.80s |\n", 
				" ",
				sknh_rec.lab_note2
			);
		}
		if (strcmp (sknh_rec.lab_note3, noteBlank))
		{
			fprintf 
			 (
				fout,  
				"|%71.71s : %-80.80s |\n", 
				" ",
				sknh_rec.lab_note3
			);
		}
		if (strcmp (sknh_rec.lab_note4, noteBlank))
		{
			fprintf 
			 (
				fout, 
				"|%71.71s : %-80.80s |\n", 
				" ",
				sknh_rec.lab_note4
			);
		}
		if (strcmp (sknh_rec.lab_note5, noteBlank))
		{
			fprintf 
			 (
				fout, 
				"|%71.71s : %-80.80s |\n", 
				" ",
					sknh_rec.lab_note5
			);
		}

		fprintf (fout, "|----");
		fprintf (fout, "|----------------");
		fprintf (fout, "|--------------------");
		fprintf (fout, "|----");
		fprintf (fout, "|-----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|----------");
		fprintf (fout, "|--------------------");
		fprintf (fout, "|---------------|\n");

		fprintf (fout, "|LINE");
		fprintf (fout, "|  ITEM NUMBER   ");
		fprintf (fout, "|  ITEM DESCRIPTION  ");
		fprintf (fout, "|UOM.");
		fprintf (fout, "| UNIT QTY. ");
		fprintf (fout, "| PACK QTY. ");
		fprintf (fout, "|CHARGE WGT.");
		fprintf (fout, "|TOTAL GROSS");
		fprintf (fout, "| TOTAL CBM ");
		fprintf (fout, "| LOCATION ");
		fprintf (fout, "|   LOCATION STATUS  ");
		fprintf (fout, "|  CONTAINER NO |\n");

		fprintf (fout, "|----");
		fprintf (fout, "|----------------");
		fprintf (fout, "|--------------------");
		fprintf (fout, "|----");
		fprintf (fout, "|-----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|----------");
		fprintf (fout, "|--------------------");
		fprintf (fout, "|---------------|\n");

		sknd_rec.sknh_hash	=	sknh_rec.sknh_hash;
		sknd_rec.line_no	=	0;
		cc = find_rec (sknd, &sknd_rec, GTEQ, "r");
		while (!cc && sknd_rec.sknh_hash == sknh_rec.sknh_hash)
		{
			inmr_rec.hhbr_hash	=	sknd_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (sknd, &sknd_rec, NEXT, "r");
				continue;
			}
			inum_rec.hhum_hash	=	sknd_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (sknd, &sknd_rec, NEXT, "r");
				continue;
			}
			inlo_rec.sknd_hash	=	sknd_rec.sknd_hash;
			cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
			while (!cc && inlo_rec.sknd_hash == sknd_rec.sknd_hash)
			{
				strcpy (llst_rec.co_no, comm_rec.co_no);
				strcpy (llst_rec.code,  inlo_rec.loc_status);
				cc = find_rec (llst, &llst_rec, EQUAL, "r");
				if (cc)
					strcpy (llst_rec.desc, " ");
				fprintf 
			 	(
					fout,
					"|%04d|%16.16s|%-20.20s|%4.4s|%10.2f |%10.2f |%10.2f |%10.2f |%10.2f |%10.10s|%20.20s|%15.15s|\n",
					sknd_rec.line_no,
					inmr_rec.item_no,
					inmr_rec.description,
					inum_rec.uom, 
					inlo_rec.rec_qty,
					inlo_rec.pack_qty,
					inlo_rec.chg_wgt,
					inlo_rec.gross_wgt,
					inlo_rec.cu_metre,
					inlo_rec.location,
					llst_rec.desc,
					sknd_rec.cus_ord_ref
				);
				cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			}
			cc = find_rec (sknd, &sknd_rec, NEXT, "r");
		}
		fprintf (fout, "|====");
		fprintf (fout, "|================");
		fprintf (fout, "|====================");
		fprintf (fout, "|====");
		fprintf (fout, "|===========");
		fprintf (fout, "|===========");
		fprintf (fout, "|===========");
		fprintf (fout, "|===========");
		fprintf (fout, "|===========");
		fprintf (fout, "|==========");
		fprintf (fout, "|+===================");
		fprintf (fout, "|===============|\n");
		fprintf (fout, ".LRP4\n");
		cc = find_rec (sknh, &sknh_rec, NEXT, "r");
	}
}
void	
RunProgram 
 (
	char	*programName)
{
	sprintf (local_rec.printerString,"%2d",local_rec.printerNumber);
	
	shutdown_prog ();
	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp 
			 (
				"ONIGHT",
				"ONIGHT",
				programName,
				local_rec.printerString,
				local_rec.noPlateFrom,
				local_rec.noPlateTo,
				"Location master file printout", (char *)0
			);
		}
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp 
			 (
				programName,
				programName,
				local_rec.printerString,
				local_rec.noPlateFrom,
				local_rec.noPlateTo,
				 (char *) 0
			);
		}
	}
	else 
	{
		execlp 
		 (
			programName,
			programName,
			local_rec.printerString,
			local_rec.noPlateFrom,
			local_rec.noPlateTo,
			 (char *) 0
		);
	}
}

/*=============================
| Search Number Plate Header. |
=============================*/
void
SrchSknh (
	char    *keyValue)
{
    work_open ();
    save_rec ("#Number Plate   ", "#Number Plate description.");

	/*--------------------------
	| Flush record buffer first |
	---------------------------*/
	memset (&sknh_rec, 0, sizeof (sknh_rec));

	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
    sprintf (sknh_rec.plate_no, "%15.15s", keyValue);
    for (cc = find_rec (sknh, &sknh_rec,  GTEQ,"r");
		 !cc && !strcmp (sknh_rec.co_no, comm_rec.co_no)
		  && !strcmp (sknh_rec.br_no, comm_rec.est_no)
		  && !strncmp (sknh_rec.plate_no, keyValue, strlen (clip (keyValue)));
         cc = find_rec (sknh, &sknh_rec,  NEXT, "r"))
    {
        cc = save_rec (sknh_rec.plate_no, sknh_rec.lab_note1);
        if (cc)
            break;
    }
    cc = disp_srch ();
    work_close ();
    if (cc)
        return;

	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, comm_rec.est_no);
    sprintf (sknh_rec.plate_no, "%15.15s", temp_str);
    cc = find_rec (sknh, &sknh_rec,  COMPARISON, "r");
    if (cc)
       file_err (cc, (char *)sknh, "DBFIND");
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

		rv_pr (ML ("Stock Number Plate Selection"),50,0,1);
		line_at (1,0,130);

		box (0, 3, 131, (DSP_NP) ? 2 : 6);
		line_at (6,1,130);

		line_at (19,0,130);

		print_at (20,0, ML (mlStdMess038), 
					comm_rec.co_no, comm_rec.co_name);

		print_at (21,0, ML (mlStdMess039), 
					comm_rec.est_no, comm_rec.est_name);

		print_at (22,0, ML (mlStdMess099), 
					comm_rec.cc_no, comm_rec.cc_name);

		line_at (23,0,130);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
