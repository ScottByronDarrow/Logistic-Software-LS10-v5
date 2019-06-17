/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: rg_resmnt.c,v 5.3 2002/07/08 08:04:46 scott Exp $
|  Program Name  : (rg_resmnt.c)
|  Program Desc  : (Routing system Resource Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : 30/10/91        | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
| $Log: rg_resmnt.c,v $
| Revision 5.3  2002/07/08 08:04:46  scott
| S/C 004059 - Updated for display error
|
| Revision 5.2  2001/08/09 09:16:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:39:44  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: rg_resmnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/RG/rg_resmnt/rg_resmnt.c,v 5.3 2002/07/08 08:04:46 scott Exp $";

/*--------------------------
| Already defined somewhere |
 --------------------------*/
#define		MAXSCNS	6
#include	<pslscr.h>
#include	<GlUtils.h>
#include	<ml_std_mess.h>
#include	<ml_rg_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct rgrsRecord	rgrs_rec;

	char	*accPad = "                ";
	char	*data	= "data",
			*glmr2	= "glmr2";

	int		new_code;
	int		scn_no;

int 	heading 		(int);
int 	spec_valid 		(int);
void 	shutdownProg 	(void);
void	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	SrchRgrs 		(char *);

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	long	dir_hash,
			dir_rec_hash,
			fix_hash,
			fix_rec_hash,
			mfg_dir_hash,
			mfg_fix_hash;
	char	dir_acc [MAXLEVEL + 1],
			dir_desc [26],
			dir_rec_acc [MAXLEVEL + 1],
			dir_rec_desc [26],
			fix_acc [MAXLEVEL + 1],
			fix_desc [26],
			fix_rec_acc [MAXLEVEL + 1],
			fix_rec_desc [26],
			mfg_dir_acc [MAXLEVEL + 1],
			mfg_dir_desc [26],
			mfg_fix_acc [MAXLEVEL + 1],
			mfg_fix_desc [26];
	char	cal_sel [11];
	char	dummy [11];
} local_rec;

#define	MAIN	1
#define	LABOUR	2
#define	MACHINE	3
#define	OTHER	4
#define	QC		5
#define	SPECIAL	6


static	struct	var	vars [] =
{
	{MAIN, LIN, "code",	 2, 20, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Resource Code        ", " ",
		YES, NO,  JUSTLEFT, "", "", rgrs_rec.code},
	{MAIN, LIN, "desc",	 3, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Resource Description ", " ",
		 NO, NO,  JUSTLEFT, "", "", rgrs_rec.desc},
	{MAIN, LIN, "type",	 4, 20, CHARTYPE,
		"U", "          ",
		" ", "", "Type                 ", "L(abour) M(achine) Q(C-check) S(pecial) O(ther)",
		YES, NO,  JUSTLEFT, "LMOQS", "", rgrs_rec.type},
	{MAIN, LIN, "type_name",	 5, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "Type Name            ", " ",
		 NA, NO,  JUSTLEFT, "", "", rgrs_rec.type_name},

	{LABOUR, LIN, "descL",	 3, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Resource Description ", " ",
		 NO, NO,  JUSTLEFT, "", "", rgrs_rec.desc},
	{LABOUR, LIN, "wdaccL",	 7, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Direct G/L       ", "WIP Direct/Variable Resource G/L Account",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_acc},
	{LABOUR, LIN, "wddscL",	 7, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_desc},
	{LABOUR, LIN, "rdaccL",	 8, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Direct G/L       ", "Recovery Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_rec_acc},
	{LABOUR, LIN, "rddscL",	 8, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_rec_desc},
	{LABOUR, LIN, "wfaccL",	 9, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Fix O/H G/L      ", "WIP Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_acc},
	{LABOUR, LIN, "wfdscL",	 9, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_desc},
	{LABOUR, LIN, "rfaccL",	 10, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Fix O/H G/L      ", "Recovery Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_rec_acc},
	{LABOUR, LIN, "rfdscL",	 10, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_rec_desc},
	{LABOUR, LIN, "mfgdiraccL",	 11, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Direct G/L   ", "Manufacturing Variance Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_dir_acc},
	{LABOUR, LIN, "mfgdirdscL",	 11, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_dir_desc},
	{LABOUR, LIN, "mfgfixaccL",	 12, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Fix O/H G/L  ", "Manufacturing Variance Fixed Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_fix_acc},
	{LABOUR, LIN, "mfgfixdscL",	 12, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_fix_desc},
	{LABOUR, LIN, "rateL",	 13, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Labour Rate          ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.rate},
	{LABOUR, LIN, "vovhL",	14, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Varbl)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_var},
	{LABOUR, LIN, "fovhL",	15, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Fixed)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_fix},
	{LABOUR, LIN, "qavlL",	16, 20, INTTYPE,
		"NN", "          ",
		" ", "1", "Qty Available        ", " ",
		 YES, NO, JUSTRIGHT, "1", "99", (char *) &rgrs_rec.qty_avl},
	{LABOUR, LIN, "clslL",	17, 20, CHARTYPE,
		"U", "          ",
		" ", "G", "Cal. Used            ", "Please enter either G(lobal) OR S(pecific)",
		YES, NO, JUSTLEFT, "GS", "", rgrs_rec.cal_sel},
	{LABOUR, LIN, "cl_dscL",	17, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.cal_sel},

	{MACHINE, LIN, "descM",	 3, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Resource Description ", " ",
		 NO, NO,  JUSTLEFT, "", "", rgrs_rec.desc},
	{MACHINE, LIN, "wdaccM",	 7, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Direct G/L       ", "WIP Direct/Variable Resource G/L Account",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_acc},
	{MACHINE, LIN, "wddscM",	 7, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_desc},
	{MACHINE, LIN, "rdaccM",	 8, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Direct G/L       ", "Recovery Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_rec_acc},
	{MACHINE, LIN, "rddscM",	 8, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_rec_desc},
	{MACHINE, LIN, "wfaccM",	 9, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Fix O/H G/L      ", "WIP Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_acc},
	{MACHINE, LIN, "wfdscM",	 9, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_desc},
	{MACHINE, LIN, "rfaccM",	 10, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Fix O/H G/L      ", "Recovery Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_rec_acc},
	{MACHINE, LIN, "rfdscM",	 10, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_rec_desc},
	{MACHINE, LIN, "mfgdiraccM",	 11, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Direct G/L   ", "Manufacturing Variance Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_dir_acc},
	{MACHINE, LIN, "mfgdirdscM",	 11, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_dir_desc},
	{MACHINE, LIN, "mfgfixaccM",	 12, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Fix O/H G/L  ", "Manufacturing Variance Fixed Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_fix_acc},
	{MACHINE, LIN, "mfgfixdscM",	 12, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_fix_desc},
	{MACHINE, LIN, "rateM",	 13, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Machine Rate         ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.rate},
	{MACHINE, LIN, "vovhM",	14, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Varbl)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_var},
	{MACHINE, LIN, "fovhM",	15, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Fixed)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_fix},
	{MACHINE, LIN, "qavlM",	16, 20, INTTYPE,
		"NN", "          ",
		" ", "1", "Qty Available        ", " ",
		 YES, NO, JUSTRIGHT, "1", "99", (char *) &rgrs_rec.qty_avl},
	{MACHINE, LIN, "clslM",	17, 20, CHARTYPE,
		"U", "          ",
		" ", "G", "Cal. Used            ", "Please enter either G(lobal) OR S(pecific)",
		YES, NO, JUSTLEFT, "GS", "", rgrs_rec.cal_sel},
	{MACHINE, LIN, "cl_dscM",17, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.cal_sel},

	{OTHER, LIN, "descO",	 3, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Resource Description ", " ",
		 NO, NO,  JUSTLEFT, "", "", rgrs_rec.desc},
	{OTHER, LIN, "type_nameO",5, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "Type Name            ", " ",
		 NO, NO,  JUSTLEFT, "", "", rgrs_rec.type_name},
	{OTHER, LIN, "wdaccO",	 7, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Direct G/L       ", "WIP Direct/Variable Resource G/L Account",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_acc},
	{OTHER, LIN, "wddscO",	 7, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_desc},
	{OTHER, LIN, "rdaccO",	 8, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Direct G/L       ", "Recovery Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_rec_acc},
	{OTHER, LIN, "rddscO",	 8, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_rec_desc},
	{OTHER, LIN, "wfaccO",	 9, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Fix O/H G/L      ", "WIP Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_acc},
	{OTHER, LIN, "wfdscO",	 9, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_desc},
	{OTHER, LIN, "rfaccO",	 10, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Fix O/H G/L      ", "Recovery Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_rec_acc},
	{OTHER, LIN, "rfdscO",	 10, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_rec_desc},
	{OTHER, LIN, "mfgdiraccO",	 11, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Direct G/L   ", "Manufacturing Variance Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_dir_acc},
	{OTHER, LIN, "mfgdirdscO",	 11, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_dir_desc},
	{OTHER, LIN, "mfgfixaccO",	 12, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Fix O/H G/L  ", "Manufacturing Variance Fixed Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_fix_acc},
	{OTHER, LIN, "mfgfixdscO",	 12, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_fix_desc},
	{OTHER, LIN, "typeO",	 13, 20, CHARTYPE,
		"UUU", "          ",
		" ", "IFC", "Costing Type         ", "DVC=Dir Var, DFC=Dir Fixed, IVC=Ind Var, IFC=Ind Fixed",
		YES, NO,  JUSTLEFT, "CDFIV", "", rgrs_rec.cost_type},
	{OTHER, LIN, "rateO",	14, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Other Rate           ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.rate},
	{OTHER, LIN, "vovhO",	15, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Varbl)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_var},
	{OTHER, LIN, "fovhO",	16, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Fixed)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_fix},
	{OTHER, LIN, "qavlO",	17, 20, INTTYPE,
		"NN", "          ",
		" ", "1", "Qty Available        ", " ",
		 YES, NO, JUSTRIGHT, "1", "99", (char *) &rgrs_rec.qty_avl},
	{OTHER, LIN, "clslO",	18, 20, CHARTYPE,
		"U", "          ",
		" ", "G", "Cal. Used            ", "Please enter either G(lobal) OR S(pecific)",
		YES, NO, JUSTLEFT, "GS", "", rgrs_rec.cal_sel},
	{OTHER, LIN, "cl_dscO",	18, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.cal_sel},

	{QC, LIN, "descQ",	 3, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Resource Description ", " ",
		 NO, NO,  JUSTLEFT, "", "", rgrs_rec.desc},
	{QC, LIN, "wdaccQ",	 7, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Direct G/L       ", "WIP Direct/Variable Resource G/L Account",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_acc},
	{QC, LIN, "wddscQ",	 7, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_desc},
	{QC, LIN, "rdaccQ",	 8, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Direct G/L       ", "Recovery Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_rec_acc},
	{QC, LIN, "rddscQ",	 8, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_rec_desc},
	{QC, LIN, "wfaccQ",	 9, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Fix O/H G/L      ", "WIP Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_acc},
	{QC, LIN, "wfdscQ",	 9, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_desc},
	{QC, LIN, "rfaccQ",	 10, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Fix O/H G/L      ", "Recovery Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_rec_acc},
	{QC, LIN, "rfdscQ",	 10, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_rec_desc},
	{QC, LIN, "mfgdiraccQ",	 11, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Direct G/L   ", "Manufacturing Variance Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_dir_acc},
	{QC, LIN, "mfgdirdscQ",	 11, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_dir_desc},
	{QC, LIN, "mfgfixaccQ",	 12, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Fix O/H G/L  ", "Manufacturing Variance Fixed Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_fix_acc},
	{QC, LIN, "mfgfixdscQ",	 12, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_fix_desc},
	{QC, LIN, "rateQ",	 13, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Q/C Rate             ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.rate},
	{QC, LIN, "vovhQ",	14, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Varbl)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_var},
	{QC, LIN, "fovhQ",	15, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Fixed)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_fix},
	{QC, LIN, "qavlQ",	16, 20, INTTYPE,
		"NN", "          ",
		" ", "1", "Qty Available        ", " ",
		 YES, NO, JUSTRIGHT, "1", "99", (char *) &rgrs_rec.qty_avl},
	{QC, LIN, "clslQ",	17, 20, CHARTYPE,
		"U", "          ",
		" ", "G", "Cal. Used            ", "Please enter either G(lobal) OR S(pecific)",
		YES, NO, JUSTLEFT, "GS", "", rgrs_rec.cal_sel},
	{QC, LIN, "cl_dscQ",	17, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.cal_sel},

	{SPECIAL, LIN, "descS",	 3, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Resource Description ", " ",
		 NO, NO,  JUSTLEFT, "", "", rgrs_rec.desc},
	{SPECIAL, LIN, "wdaccS",	 7, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Direct G/L       ", "WIP Direct/Variable Resource G/L Account",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_acc},
	{SPECIAL, LIN, "wddscS",	 7, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_desc},
	{SPECIAL, LIN, "rdaccS",	 8, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Direct G/L       ", "Recovery Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.dir_rec_acc},
	{SPECIAL, LIN, "rddscS",	 8, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dir_rec_desc},
	{SPECIAL, LIN, "wfaccS",	 9, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "WIP Fix O/H G/L      ", "WIP Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_acc},
	{SPECIAL, LIN, "wfdscS",	 9, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_desc},
	{SPECIAL, LIN, "rfaccS",	 10, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Rec Fix O/H G/L      ", "Recovery Fixed Overheads Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.fix_rec_acc},
	{SPECIAL, LIN, "rfdscS",	 10, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fix_rec_desc},
	{SPECIAL, LIN, "mfgdiraccS",	 11, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Direct G/L   ", "Manufacturing Variance Direct/Variable Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_dir_acc},
	{SPECIAL, LIN, "mfgdirdscS",	 11, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_dir_desc},
	{SPECIAL, LIN, "mfgfixaccS",	 12, 20, CHARTYPE,
		GlMask, "          ",
		"0", "", "Mfg Var Fix O/H G/L  ", "Manufacturing Variance Fixed Resource G/L Account.",
		YES, NO,  JUSTLEFT, "0123456789-*", "", local_rec.mfg_fix_acc},
	{SPECIAL, LIN, "mfgfixdscS",	 12, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mfg_fix_desc},
	{SPECIAL, LIN, "rateS",	 13, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Special Rate         ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.rate},
	{SPECIAL, LIN, "vovhS",	14, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Varbl)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_var},
	{SPECIAL, LIN, "fovhS",	15, 20, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Overhd (Fixed)       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &rgrs_rec.ovhd_fix},
	{SPECIAL, LIN, "qavlS",	16, 20, INTTYPE,
		"NN", "          ",
		" ", "1", "Qty Available        ", " ",
		 YES, NO, JUSTRIGHT, "1", "99", (char *) &rgrs_rec.qty_avl},
	{SPECIAL, LIN, "clslS",	17, 20, CHARTYPE,
		"U", "          ",
		" ", "G", "Cal. Used            ", "Please enter either G(lobal) OR S(pecific)",
		YES, NO, JUSTLEFT, "GS", "", rgrs_rec.cal_sel},
	{SPECIAL, LIN, "cl_dscS",17, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.cal_sel},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	SETUP_SCR (vars);

	OpenDB ();

	GL_SetMask (GlFormat);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (MAIN);
	
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
		heading (MAIN);
		entry (MAIN);
		if (prog_exit)
		{
			shutdownProg ();
			return EXIT_FAILURE;
		}

		switch (rgrs_rec.type [0])
		{
		case	'L':
			scn_no = LABOUR;
			break;

		case	'M':
			scn_no = MACHINE;
			break;

		case	'O':
			scn_no = OTHER;
			break;

		case	'Q':
			scn_no = QC;
			break;

		case	'S':
			scn_no = SPECIAL;
			break;

		default:
			scn_no = 0;
		}

		if (scn_no == 0)
			continue;

		heading (scn_no);
		scn_display (scn_no);
		edit (scn_no);

		if (restart)
			continue;

		Update ();
	}	/* end of input control loop	*/

	shutdownProg ();

	return EXIT_SUCCESS;
}

/*========================
| Program exit sequence. |
========================*/
void
shutdownProg (
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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	abc_alias (glmr2, glmr);

	OpenGlmr ();
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_id_no");
}

/*=========================
| Close data base files . |
=========================*/

void 
CloseDB (
 void)
{
	abc_fclose (glmr2);
	abc_fclose (rgrs);
	GL_Close ();
	abc_dbclose (data);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		scn_set (MAIN);
		clear ();
		rv_pr (ML (mlRgMess003), 29, 0, 1);

		box (0, 1, 80, 4);

		line_at (20,0,80);
		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);
		scn_write (MAIN);
		if (scn != MAIN)
			scn_display (MAIN);
		else
			return 0;

		if (scn == OTHER)
			box (0, 1, 80, 17);
		else
			box (0, 1, 80, 16);
		scn_set (scn);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

	return 0;
}

int
spec_valid (
 int field)
{
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "u");
		if (!cc)
		{
			new_code = FALSE;
			entry_exit = TRUE;
			/* Read for WIP Direct Resource Account */
			cc = find_hash (glmr2, &glmrRec, EQUAL, "r", rgrs_rec.dir_hash);
			if (cc)
			{
				sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL, accPad);
				sprintf (glmrRec.desc, "%25.25s", " ");
			}
			local_rec.dir_hash = rgrs_rec.dir_hash;
			strcpy (local_rec.dir_acc, glmrRec.acc_no);
			strcpy (local_rec.dir_desc, glmrRec.desc);

			/* Read for Recovery Direct Resource Account */
			cc = find_hash (glmr2, &glmrRec, EQUAL, "r", rgrs_rec.dir_rec_hash);
			if (cc)
			{
				sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL, accPad);
				sprintf (glmrRec.desc, "%25.25s", " ");
			}
			local_rec.dir_rec_hash = rgrs_rec.dir_rec_hash;
			strcpy (local_rec.dir_rec_acc, glmrRec.acc_no);
			strcpy (local_rec.dir_rec_desc, glmrRec.desc);

			/* Read for WIP Fixed Overheads Resource Account */
			cc = find_hash (glmr2, &glmrRec, EQUAL, "r", rgrs_rec.fix_hash);
			if (cc)
			{
				sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL, accPad);
				sprintf (glmrRec.desc, "%25.25s", " ");
			}
			local_rec.fix_hash = rgrs_rec.fix_hash;
			strcpy (local_rec.fix_acc, glmrRec.acc_no);
			strcpy (local_rec.fix_desc, glmrRec.desc);

			/* Read for Recovery Fixed Overheads Resource Account */
			cc = find_hash (glmr2, &glmrRec, EQUAL, "r", rgrs_rec.fix_rec_hash);
			if (cc)
			{
				sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL, accPad);
				sprintf (glmrRec.desc, "%25.25s", " ");
			}
			local_rec.fix_rec_hash = rgrs_rec.fix_rec_hash;
			strcpy (local_rec.fix_rec_acc, glmrRec.acc_no);
			strcpy (local_rec.fix_rec_desc, glmrRec.desc);

			/* Read for Manufacturing Variance Direct Resource Account */
			cc = find_hash (glmr2, &glmrRec, EQUAL, "r", rgrs_rec.mfg_dir_hash);
			if (cc)
			{
				sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL, accPad);
				sprintf (glmrRec.desc, "%25.25s", " ");
			}
			local_rec.mfg_dir_hash = rgrs_rec.mfg_dir_hash;
			strcpy (local_rec.mfg_dir_acc, glmrRec.acc_no);
			strcpy (local_rec.mfg_dir_desc, glmrRec.desc);

			/* Read for Manufacturing Variance Fixed Resource Account */
			cc = find_hash (glmr2, &glmrRec, EQUAL, "r", rgrs_rec.mfg_fix_hash);
			if (cc)
			{
				sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL, accPad);
				sprintf (glmrRec.desc, "%25.25s", " ");
			}
			local_rec.mfg_fix_hash = rgrs_rec.mfg_fix_hash;
			strcpy (local_rec.mfg_fix_acc, glmrRec.acc_no);
			strcpy (local_rec.mfg_fix_desc, glmrRec.desc);

			DSP_FLD ("desc");
			DSP_FLD ("type");
			DSP_FLD ("type_name");

			switch (rgrs_rec.cal_sel [0])
			{
			case	'G':
				sprintf (local_rec.cal_sel, "%-10.10s", "G(lobal)");
				break;

			case	'S':
				sprintf (local_rec.cal_sel, "%-10.10s", "S(pecific)");
				break;
			}
		}
		else
		{
			new_code = TRUE;
			sprintf (rgrs_rec.desc, "%-25.25s", " ");
			sprintf (rgrs_rec.type, "%-1.1s", " ");
			sprintf (rgrs_rec.type_name, "%-10.10s", " ");
			sprintf (rgrs_rec.cost_type, "%-3.3s", " ");
			sprintf (rgrs_rec.cost_type, "%-3.3s", " ");
			local_rec.dir_hash		 = 0L;
			local_rec.dir_rec_hash	 = 0L;
			local_rec.fix_hash		 = 0L;
			local_rec.fix_rec_hash	 = 0L;
			local_rec.mfg_dir_hash	 = 0L;
			local_rec.mfg_fix_hash	 = 0L;
			sprintf (local_rec.dir_acc,			"%*.*s", MAXLEVEL,MAXLEVEL," ");
			sprintf (local_rec.dir_rec_acc,		"%*.*s", MAXLEVEL,MAXLEVEL," ");
			sprintf (local_rec.fix_acc,			"%*.*s", MAXLEVEL,MAXLEVEL," ");
			sprintf (local_rec.fix_rec_acc,		"%*.*s", MAXLEVEL,MAXLEVEL," ");
			sprintf (local_rec.mfg_dir_acc,		"%*.*s", MAXLEVEL,MAXLEVEL," ");
			sprintf (local_rec.mfg_fix_acc,		"%*.*s", MAXLEVEL,MAXLEVEL," ");
			sprintf (local_rec.dir_desc,		"%-25.25s", " ");
			sprintf (local_rec.dir_rec_desc,	"%-25.25s", " ");
			sprintf (local_rec.fix_desc,		"%-25.25s", " ");
			sprintf (local_rec.fix_rec_desc,	"%-25.25s", " ");
			sprintf (local_rec.mfg_dir_desc,	"%-25.25s", " ");
			sprintf (local_rec.mfg_fix_desc,	"%-25.25s", " ");
			rgrs_rec.qty_avl = 1;
			rgrs_rec.rate = 0.00;
			rgrs_rec.ovhd_var = 0.00;
			rgrs_rec.ovhd_fix = 0.00;
			strcpy (rgrs_rec.cal_sel, "G");
			sprintf (local_rec.cal_sel, "%-10.10s", "G(lobal)");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("type"))
	{
		switch (rgrs_rec.type [0])
		{
		case	'L':
			sprintf (rgrs_rec.type_name, "%-10.10s", "Labour");
			break;

		case	'M':
			sprintf (rgrs_rec.type_name, "%-10.10s", "Machine");
			break;

		case	'Q':
			sprintf (rgrs_rec.type_name, "%-10.10s", "QC Check");
			break;

		case	'S':
			sprintf (rgrs_rec.type_name, "%-10.10s", "Special");
			break;

		case	'O':
			if (prog_status == ENTRY)
				sprintf (rgrs_rec.type_name, "%-10.10s", "Other");
			break;
		}

		DSP_FLD ("type_name");

		return (EXIT_SUCCESS);
	}
	
	if (LNCHECK ("wdacc", 5))
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, local_rec.dir_acc);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (glmrRec.glmr_class [0][0] != 'F' ||
			glmrRec.glmr_class [2][0] != 'P')
		{
			errmess (ML (mlStdMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.dir_hash = glmrRec.hhmr_hash;
		strcpy (local_rec.dir_acc, glmrRec.acc_no);
		strcpy (local_rec.dir_desc, glmrRec.desc);
		/* display correct variable type */
		if (scn_no == LABOUR) DSP_FLD ("wddscL");
		else if (scn_no == MACHINE) DSP_FLD ("wddscM");
		else if (scn_no == OTHER) DSP_FLD ("wddscO");
		else if (scn_no == QC) DSP_FLD ("wddscQ");
		else if (scn_no == SPECIAL) DSP_FLD ("wddscS");
	}

	if (LNCHECK ("rdacc", 5))
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, local_rec.dir_rec_acc);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (glmrRec.glmr_class [0][0] != 'F' ||
			glmrRec.glmr_class [2][0] != 'P')
		{
			errmess (ML (mlStdMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.dir_rec_hash = glmrRec.hhmr_hash;
		strcpy (local_rec.dir_rec_acc, glmrRec.acc_no);
		strcpy (local_rec.dir_rec_desc, glmrRec.desc);
		/* display correct variable type */
		if (scn_no == LABOUR) DSP_FLD ("rddscL");
		else if (scn_no == MACHINE) DSP_FLD ("rddscM");
		else if (scn_no == OTHER) DSP_FLD ("rddscO");
		else if (scn_no == QC) DSP_FLD ("rddscQ");
		else if (scn_no == SPECIAL) DSP_FLD ("rddscS");
	}

	if (LNCHECK ("wfacc", 5))
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, local_rec.fix_acc);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (glmrRec.glmr_class [0][0] != 'F' ||
			glmrRec.glmr_class [2][0] != 'P')
		{
			errmess (ML (mlStdMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.fix_hash = glmrRec.hhmr_hash;
		strcpy (local_rec.fix_acc, glmrRec.acc_no);
		strcpy (local_rec.fix_desc, glmrRec.desc);
		/* display correct variable type */
		if (scn_no == LABOUR) DSP_FLD ("wfdscL");
		else if (scn_no == MACHINE) DSP_FLD ("wfdscM");
		else if (scn_no == OTHER) DSP_FLD ("wfdscO");
		else if (scn_no == QC) DSP_FLD ("wfdscQ");
		else if (scn_no == SPECIAL) DSP_FLD ("wfdscS");
	}

	if (LNCHECK ("rfacc", 5))
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, local_rec.fix_rec_acc);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (glmrRec.glmr_class [0][0] != 'F' ||
			glmrRec.glmr_class [2][0] != 'P')
		{
			errmess (ML (mlStdMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.fix_rec_hash = glmrRec.hhmr_hash;
		strcpy (local_rec.fix_rec_acc, glmrRec.acc_no);
		strcpy (local_rec.fix_rec_desc, glmrRec.desc);
		/* display correct variable type */
		if (scn_no == LABOUR) DSP_FLD ("rfdscL");
		else if (scn_no == MACHINE) DSP_FLD ("rfdscM");
		else if (scn_no == OTHER) DSP_FLD ("rfdscO");
		else if (scn_no == QC) DSP_FLD ("rfdscQ");
		else if (scn_no == SPECIAL) DSP_FLD ("rfdscS");
	}

	if (LNCHECK ("mfgdiracc", 5))
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, local_rec.mfg_dir_acc);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (glmrRec.glmr_class [0][0] != 'F' ||
			glmrRec.glmr_class [2][0] != 'P')
		{
			errmess (ML (mlStdMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.mfg_dir_hash = glmrRec.hhmr_hash;
		strcpy (local_rec.mfg_dir_acc, glmrRec.acc_no);
		strcpy (local_rec.mfg_dir_desc, glmrRec.desc);
		/* display correct variable type */
		if (scn_no == LABOUR) DSP_FLD ("mfgdirdscL");
		else if (scn_no == MACHINE) DSP_FLD ("mfgdirdscM");
		else if (scn_no == OTHER) DSP_FLD ("mfgdirdscO");
		else if (scn_no == QC) DSP_FLD ("mfgdirdscQ");
		else if (scn_no == SPECIAL) DSP_FLD ("mfgdirdscS");
	}

	if (LNCHECK ("mfgfixacc", 5))
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, local_rec.mfg_fix_acc);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (glmrRec.glmr_class [0][0] != 'F' ||
			glmrRec.glmr_class [2][0] != 'P')
		{
			errmess (ML (mlStdMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.mfg_fix_hash = glmrRec.hhmr_hash;
		strcpy (local_rec.mfg_fix_acc, glmrRec.acc_no);
		strcpy (local_rec.mfg_fix_desc, glmrRec.desc);
		/* display correct variable type */
		if (scn_no == LABOUR) DSP_FLD ("mfgfixdscL");
		else if (scn_no == MACHINE) DSP_FLD ("mfgfixdscM");
		else if (scn_no == OTHER) DSP_FLD ("mfgfixdscO");
		else if (scn_no == QC) DSP_FLD ("mfgfixdscQ");
		else if (scn_no == SPECIAL) DSP_FLD ("mfgfixdscS");
	}

	if (LNCHECK ("clsl", 4))
	{
		switch (rgrs_rec.cal_sel [0])
		{
		case	'G':
			sprintf (local_rec.cal_sel, "%-10.10s", "G(lobal)");
			break;

		case	'S':
			sprintf (local_rec.cal_sel, "%-10.10s", "S(pecific)");
			break;
		}
		/* display correct variable type */
		if (scn_no == LABOUR) DSP_FLD ("cl_dscL");
		else if (scn_no == MACHINE) DSP_FLD ("cl_dscM");
		else if (scn_no == OTHER) DSP_FLD ("cl_dscO");
		else if (scn_no == QC) DSP_FLD ("cl_dscQ");
		else if (scn_no == SPECIAL) DSP_FLD ("cl_dscS");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("typeO"))
	{
		if (!strcmp (rgrs_rec.cost_type, "IFC"))
			return (EXIT_SUCCESS);
		if (!strcmp (rgrs_rec.cost_type, "IVC"))
			return (EXIT_SUCCESS);
		if (!strcmp (rgrs_rec.cost_type, "DFC"))
			return (EXIT_SUCCESS);
		if (!strcmp (rgrs_rec.cost_type, "DVC"))
			return (EXIT_SUCCESS);
		errmess (ML (mlRgMess004));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void 
Update (
 void)
{
	rgrs_rec.dir_hash	 = local_rec.dir_hash;
	rgrs_rec.dir_rec_hash = local_rec.dir_rec_hash;
	rgrs_rec.fix_hash	 = local_rec.fix_hash;
	rgrs_rec.fix_rec_hash = local_rec.fix_rec_hash;
	rgrs_rec.mfg_dir_hash = local_rec.mfg_dir_hash;
	rgrs_rec.mfg_fix_hash = local_rec.mfg_fix_hash;
	if (new_code)
	{
		cc = abc_add (rgrs, &rgrs_rec);
		if (cc)
			file_err (cc, rgrs, "DBADD");
	}
	else
	{
		cc = abc_update (rgrs, &rgrs_rec);
		if (cc)
			file_err (cc, rgrs, "DBUPDATE");
	}
}

/*=======================
| Search for accs_code  |
=======================*/
void 
SrchRgrs (
 char *key_val)
{
	_work_open (8,0,40);
	save_rec ("#Code", "#Description");
	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", key_val);
	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (rgrs_rec.co_no, comm_rec.co_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.est_no) &&
		!strncmp (rgrs_rec.code, key_val, strlen (key_val))
	)
	{
		cc = save_rec (rgrs_rec.code, rgrs_rec.desc);
		if (cc)
			break;
		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", temp_str);
	cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "rgrs", "DBFIND");
}
