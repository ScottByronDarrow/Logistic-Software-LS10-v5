/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ex_miscinp.c,v 5.7 2002/11/28 04:09:47 scott Exp $
|  Program Name  : (ex_miscinp.c)
|  Program Desc  : (Maintain Area / Class, Salesman, Instruction) 
|---------------------------------------------------------------------|
|  Date Written  : (05/06/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: ex_miscinp.c,v $
| Revision 5.7  2002/11/28 04:09:47  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.6  2002/07/16 08:39:06  scott
| S/C 004152 - Structure error on oracle.
|
| Revision 5.5  2002/04/11 03:42:10  scott
| Updated to add comments to audit files.
|
| Revision 5.4  2001/10/05 02:58:11  cha
| Added code to produce audit files.
|
| Revision 5.3  2001/08/20 23:40:41  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ex_miscinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/ex_miscinp/ex_miscinp.c,v 5.7 2002/11/28 04:09:47 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>
#include <minimenu.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <DBAudit.h>

#define	AREA		(!strcmp (inputType, "EXAF"))
#define	CTYPE		(!strcmp (inputType, "EXCL"))
#define	CNTRCT		(!strcmp (inputType, "EXCT"))
#define	SMAN		(!strcmp (inputType, "EXSF"))
#define	INSTR		(!strcmp (inputType, "EXSI"))
#define	MKT_SS		(!strcmp (inputType, "EXMS"))
#define	MER_AGENCY	(!strcmp (inputType, "EXMA"))
#define	SALE_TYPE	(!strcmp (inputType, "SAST"))
#define	SALE_GRP	(!strcmp (inputType, "SASG"))
#define	SALE_POS	(!strcmp (inputType, "SASP"))
#define	SALE_CA		(!strcmp (inputType, "SACA"))
#define	REC_TYPE	(!strcmp (inputType, "CURT"))

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	SEL_DEFAULT	99

#define	F_EXAF	0
#define	F_EXCL	1
#define	F_EXCT	2
#define	F_EXSF	3
#define	F_EXSI	4
#define	F_EXMS	5
#define	F_EXMA	6
#define	F_SAST	7
#define	F_SASG	8
#define	F_SASP	9
#define	F_SACA	10
#define	F_CURT	11

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
	char	inputType 	[5],
			badFileName [5],
			*BlankDesc	=	"                                        ";

   	int		NewCode = FALSE;

#include	"schema"

struct commRecord	comm_rec;
struct arhrRecord	arhr_rec;
struct arlnRecord	arln_rec;
struct ccmrRecord	ccmr_rec;
struct cfhrRecord	cfhr_rec;
struct cfhsRecord	cfhs_rec;
struct cflnRecord	cfln_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct cudbRecord	cudb_rec;
struct cumrRecord	cumr_rec;
struct cusfRecord	cusf_rec;
struct cuwkRecord	cuwk_rec;
struct esmrRecord	esmr_rec;
struct exafRecord	exaf_rec;
struct exclRecord	excl_rec;
struct exctRecord	exct_rec;
struct exsfRecord	exsf_rec;
struct exsfRecord	exsf2_rec;
struct exsiRecord	exsi_rec;
struct incpRecord	incp_rec;
struct indsRecord	inds_rec;
struct inlsRecord	inls_rec;
struct inmrRecord	inmr_rec;
struct inprRecord	inpr_rec;
struct ithrRecord	ithr_rec;
struct mhdrRecord	mhdr_rec;
struct qthrRecord	qthr_rec;
struct qtphRecord	qtph_rec;
struct sactRecord	sact_rec;
struct sadfRecord	sadf_rec;
struct saleRecord	sale_rec;
struct sohrRecord	sohr_rec;
struct tmopRecord	tmop_rec;
struct tmpmRecord	tmpm_rec;
struct sasgRecord	sasg_rec;
struct saspRecord	sasp_rec;
struct sastRecord	sast_rec;
struct exmsRecord	exms_rec;
struct exmaRecord	exma_rec;
struct sacaRecord	saca_rec;
struct curtRecord	curt_rec;

	char	*data 	= "data",
			*exsf2 	= "exsf2";

	long	*exsf_up_sman	=	&exsf_rec.up_sman1;
	float	*exsf_level		=	&exsf_rec.lev1_com;

	extern	int	TruePosition;

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
		  "" },
		{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
		  "" },
		{ " 3. DELETE RECORD.                     ",
		  "" },
		{ ENDMENU }
	};

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	previousCode [7];
	char	up_sman [3][3];
	char	up_desc [3][41];
	char	up_sman_hash [3];
	char	coverage [3][41];
} local_rec;

static	struct	var	vars [] =
{
	/*----------------
	| Salesman   	 |
	----------------*/
	{1, LIN, "sman_code",	 2, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Salesperson No         ", "",
		 NE, NO, JUSTRIGHT, "", "", exsf_rec.salesman_no},
	{1, LIN, "sman_name",	 3, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Salesperson No         ", "",
		YES, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{1, LIN, "sal_stat",	 5, 2, CHARTYPE,
		"U", "          ",
		" ", " ", "Salesperson Type       ", "Space = N/A, H(elper), S(alesman), M(arket Rep) or D(elivery Rep)",
		YES, NO,  JUSTLEFT, " HSMD", "", exsf_rec.sale_stat},
	{1, LIN, "pos_code",	 6, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Sales Position Code    ", "Enter Sales Position Code. [SEARCH].",
		YES, NO,  JUSTRIGHT, "", "", exsf_rec.sell_pos},
	{1, LIN, "pos_desc",	 6, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", sasp_rec.pos_desc},
	{1, LIN, "type_code",	 7, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Sales Type Code        ", "Enter Sales Type Code. [SEARCH].",
		YES, NO,  JUSTRIGHT, "", "", exsf_rec.sell_type},
	{1, LIN, "type_desc",	 7, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", sast_rec.desc},
	{1, LIN, "sarea_code",	 8, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Sales Area Code        ", "Enter Area Code. [SEARCH].",
		YES, NO,  JUSTRIGHT, "", "", exsf_rec.area_code},
	{1, LIN, "sarea_desc",	 8, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{1, LIN, "sell_grp",	 9, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Sales Group Code       ", "Enter Sales Group Code. [SEARCH].",
		YES, NO,  JUSTRIGHT, "", "", exsf_rec.sell_grp},
	{1, LIN, "sell_grp_desc",	 9, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", sasg_rec.desc},
	{1, LIN, "com_type",	 11, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Commission Type        ", " G=(% of Gross), M=(% of margin), N=(% of Nett)",
		YES, NO,  JUSTLEFT, "GMN", "", exsf_rec.com_type},
	{1, LIN, "com_status",	 11, 40, CHARTYPE,
		"U", "          ",
		" ", "N", "Commission Status      ", " C = Commissionable, N = Non commissionable), H = Commission on Hold.",
		YES, NO,  JUSTLEFT, "CNH", "", exsf_rec.com_status},
	{1, LIN, "com_pc",	 12, 2, FLOATTYPE,
		"NN.NNN", "          ",
		" ", "0.0", "Standard Commission %  ", "Standard commission if no other commission applies.",
		YES, NO,  JUSTLEFT, "", "", (char *)&exsf_rec.com_pc},
	{1, LIN, "com_min",	 12, 40, MONEYTYPE,
		"NNNNNNN.NNN", "          ",
		" ", "N",   "Commission Minimum       ", "Minumum commission.",
		YES, NO,  JUSTLEFT, "", "", (char *)&exsf_rec.com_min},
	{1, LIN, "sman_com",	 13, 2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "Salesperson Commission ", "Input Commission as a percentage  ",
		YES, NO,  JUSTLEFT, "", "", (char *)&exsf_rec.sman_com},
	{1, LIN, "level1",	 14, 2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "Level 2 Commission       ", "Input Commission as a percentage  ",
		YES, NO,  JUSTLEFT, "", "", (char *)&exsf_rec.lev1_com},
	{1, LIN, "level2",	 14, 40, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "Level 3 Commission       ", "Input Commission as a percentage  ",
		YES, NO,  JUSTLEFT, "", "", (char *)&exsf_rec.lev2_com},
	{1, LIN, "level3",	 15, 2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "Level 4 Commission       ", "Input Commission as a percentage  ",
		YES, NO,  JUSTLEFT, "", "", (char *)&exsf_rec.lev3_com},
	{1, LIN, "level4",	 15, 40, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "Level 5 Commission       ", "Input Commission as a percentage  ",
		YES, NO,  JUSTLEFT, "", "", (char *)&exsf_rec.lev4_com},
	{1, LIN, "up_code1",	 17, 2, CHARTYPE,
		"UU", "          ",
		" ", " ",   "Sales Manager 1.         ", "",
		 YES, NO, JUSTRIGHT, "", "", local_rec.up_sman [0]},
	{1, LIN, "up_desc1",	 17, 31, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.up_desc [0]},
	{1, LIN, "up_code2",	 18, 2, CHARTYPE,
		"UU", "          ",
		" ", " ",   "Sales Manager 2.         ", "",
		 YES, NO, JUSTRIGHT, "", "", local_rec.up_sman [1]},
	{1, LIN, "up_desc2",	 18, 31, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.up_desc [1]},
	{1, LIN, "up_code3",	 19, 2, CHARTYPE,
		"UU", "          ",
		" ", " ",   "Sales Manager 3.         ", "",
		 YES, NO, JUSTRIGHT, "", "", local_rec.up_sman [2]},
	{1, LIN, "up_desc3",	 19, 31, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.up_desc [2]},

	/*----------------
	| Customer  Area |
	----------------*/
	{3, LIN, "area_code",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Area code            ", "",
		 NE, NO, JUSTRIGHT, "", "", exaf_rec.area_code},
	{3, LIN, "area_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Area description.    ", " ",
		YES, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{3, LIN, "rate",	 7, 2, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Freight charge rate  ", " Freight Charge Rate ",
		YES, NO, JUSTRIGHT, "", "", (char *)&exaf_rec.rate},

	/*----------------
	| Customer Class |
	----------------*/
	{3, LIN, "c_code",	 4, 2, CHARTYPE,
		"UUU", "          ",
		" ", "", "Customer Type Code  ", " ",
		 NE, NO,  JUSTLEFT, "", "", excl_rec.class_type},
	{3, LIN, "ctype_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description         ", " ",
		 NO, NO,  JUSTLEFT, "", "", excl_rec.class_desc},

	/*-----------------------
	| Special Instructions	|
	-----------------------*/
	{3, LIN, "i_code",	 4, 2, INTTYPE,
		"NNN", "          ",
		" ", "", "Instruction No.  ", "",
		 NE, NO, JUSTRIGHT, "", "", (char *)&exsi_rec.inst_code},
	{3, LIN, "inst",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.     ", " ",
		YES, NO,  JUSTLEFT, "", "", exsi_rec.inst_text},

	/*----------------------------
	| Market Supply Status File. |
	----------------------------*/
	{3, LIN, "exms_code",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Supply Status Code.  ", " ",
		 NE, NO,  JUSTRIGHT, "", "", exms_rec.stat_code},
	{3, LIN, "exms_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supply Status Desc.  ", " ",
		 NO, NO,  JUSTLEFT, "", "", exms_rec.desc},

	/*---------------------------
	| Merchandiser Agency File. |
	---------------------------*/
	{3, LIN, "exma_code",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Agency Code.         ", " ",
		 NE, NO,  JUSTRIGHT, "", "", exma_rec.code},
	{3, LIN, "exma_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Agency Description.  ", " ",
		 NO, NO,  JUSTLEFT, "", "", exma_rec.desc},

	/*--------------------
	| Sales Type File. |
	--------------------*/
	{3, LIN, "sast_code",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Sales Type Code.        ", " ",
		 NE, NO,  JUSTRIGHT, "", "", sast_rec.code},
	{3, LIN, "sast_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Sales Type Description  ", " ",
		 NO, NO,  JUSTLEFT, "", "", sast_rec.desc},

	/*----------------------
	| Sales position code. |
	----------------------*/
	{3, LIN, "sasp_code",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Sales Position Code.         ", " ",
		 NE, NO,  JUSTRIGHT, "", "", sasp_rec.pos_code},
	{3, LIN, "sasp_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Sales Position Description.  ", " ",
		 NO, NO,  JUSTLEFT, "", "", sasp_rec.pos_desc},

	/*-------------------
	| Sales group code. |
	-------------------*/
	{3, LIN, "sasg_code",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Sales Group Code.         ", " ",
		 NE, NO,  JUSTRIGHT, "", "", sasg_rec.sell_grp},
	{3, LIN, "sasg_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Sales Group Description.  ", " ",
		 NO, NO,  JUSTLEFT, "", "", sasg_rec.desc},

	/*---------------------------
	| Sales Call Activity code. |
	---------------------------*/
	{3, LIN, "saca_code",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Sales Call Activity Code.  ", " ",
		 NE, NO,  JUSTRIGHT, "", "", saca_rec.code},
	{3, LIN, "saca_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Sales Call Activity Desc.  ", " ",
		 NO, NO,  JUSTLEFT, "", "", saca_rec.desc},

	/*------------------------
	| Customer Contract Type |
	------------------------*/
	{3, LIN, "cnt_code",	 4, 2, CHARTYPE,
		"UUU", "          ",
		" ", "", "Contract Type              ", " ",
		 NE, NO,  JUSTRIGHT, "", "", exct_rec.cont_type},
	{3, LIN, "cnt_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contract type description  ", " ",
		 NO, NO,  JUSTLEFT, "", "", exct_rec.cont_desc},

	/*---------------
	| Receipt Type. |
	---------------*/
	{3, LIN, "rtype_code",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Receipt Type.          ", "",
		 NE, NO, JUSTRIGHT, "1", "9", curt_rec.chq_type},
	{3, LIN, "rtype_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.           ", " ",
		YES, NO,  JUSTLEFT, "", "", curt_rec.chq_desc},
	{3, LIN, "fwd_stat",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Forward Cheque (Y/N)   ", " ",
		YES, NO,  JUSTLEFT, "YN", "", curt_rec.fwd_stat},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

int		ChkArhr			(int);
int		ChkCohr			(int);
int		ChkCumr			(int);
int		ChkCuwk			(int);
int		ChkInls			(int);
int		ChkMhdr			(int);
int		ChkSadf			(int);
int		ChkSale			(int);
int		ChkSohr			(int);
int		ExafDelOk		(void);
int		ExclDelOk		(void);
int		ExctDelOk		(void);
int		ExsfDelOk		(void);
int		FindMisc		(void);
int		heading 		(int);
void	CloseDB			(void);
void	OpenDB			(void);
void	shutdown_prog	(void);
void	SrchCfhr		(char *);
void	SrchCurt		(char *);
void	SrchExaf		(char *);
void	SrchExcl		(char *);
void	SrchExct		(char *);
void	SrchExma		(char *);
void	SrchExms		(char *);
void	SrchExsf2		(char *);
void	SrchExsf		(char *);
void	SrchExsi		(char *);
void	SrchSaca		(char *);
void	SrchSasg		(char *);
void	SrchSasp		(char *);
void	SrchSast		(char *);
void	UnlockAll		(void);
void	UpdateCurt		(void);
void	UpdateExaf		(void);
void	UpdateExcl		(void);
void	UpdateExct		(void);
void	UpdateExma		(void);
void	UpdateExms		(void);
void	UpdateExsf		(void);
void	ActiveExsf		(void);
void	UpdateExsi		(void);
void	UpdateSaca		(void);
void	UpdateSasg		(void);
void	UpdateSasp		(void);
void	UpdateSast		(void);
void	Update			(void);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	i;

	if (argc < 2)
	{
		init_scr ();
		clear ();
		print_at (0,0, "Usage : %s [CODE] (see below)", argv [0]);
		print_at (1,0, "EXAF - Area file.");
		print_at (2,0, "EXCL - Customer Class type.");
		print_at (3,0, "EXCT - Contract type code.");
		print_at (4,0, "EXSF - Salesperson code.");
		print_at (5,0, "EXSI - Special instruction code.");
		print_at (6,0, "EXMS - Market Supply Status file.");
		print_at (7,0, "EXMA - Merchandiser Agency file.");
		print_at (8,0, "SAST - Salesperson type file.");
		print_at (9,0, "SASG - Salesperson group file.");
		print_at (10,0,"SASP - Salesperson Position file.");
		print_at (11,0,"SACA - Salesperson Call Activity file.");
		print_at (12,0,"CURT - Customer Receipt type file.\n\r");
		return (argc);
	}

	/*----------------
	| Printer Number |
	----------------*/
	sprintf (inputType,"%-4.4s", argv [1]);

	if (!AREA 		&& !CTYPE 		&& !CNTRCT 		&& !SMAN 		&& 
		 !INSTR 	&& !MKT_SS 		&& !MER_AGENCY	&& !SALE_TYPE 	&& 
		 !SALE_GRP 	&& !SALE_POS 	&& !SALE_CA 	&& !REC_TYPE)
	{
		clear ();
		print_at (0,0, "Usage : %s EXAF - Area file.\n\r", argv [0]);
		print_at (1,0, "\t\t\t EXCL - Customer Class type.\n\r");
		print_at (2,0, "\t\t\t EXCT - Contract type code.\n\r");
		print_at (3,0, "\t\t\t EXSF - Salesperson code.\n\r");
		print_at (4,0, "\t\t\t EXSI - Special instruction code.\n\r");
		print_at (5,0, "\t\t\t EXMS - Market Supply Status file.\n\r");
		print_at (6,0, "\t\t\t EXMA - Merchandiser Agency file.\n\r");
		print_at (7,0, "\t\t\t SAST - Salesperson type file.\n\r");
		print_at (8,0, "\t\t\t SASG - Salesperson group file.\n\r");
		print_at (9,0,"\t\t\t SASP - Salesperson Position file.\n\r");
		print_at (10,0,"\t\t\t SACA - Salesperson Call Activity file.\n\r");
		print_at (11,0,"\t\t\t CURT - Customer Receipt type file.\n\r");
		return (argc);
	}

	TruePosition	=	TRUE;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	for (i = label ("sman_code"); i <= label ("up_desc3"); i++)
		vars [ i ].scn = 3;

	if (AREA)
	{
		for (i = label ("area_code"); i <= label ("rate"); i++)
			vars [ i ].scn = 1;
	}

	if (CTYPE)
	{
		for (i = label ("c_code"); i <= label ("ctype_desc"); i++)
			vars [ i ].scn = 1;
	}

	if (SMAN)
	{
		for (i = label ("sman_code"); i <= label ("up_desc3"); i++)
			vars [ i ].scn = 1;
	}

	if (INSTR)
	{
		for (i = label ("i_code"); i <= label ("inst"); i++)
			vars [ i ].scn = 1;
	}

	if (CNTRCT)
	{
		for (i = label ("cnt_code"); i <= label ("cnt_desc"); i++)
			vars [ i ].scn = 1;
	}

	if (MKT_SS)
	{
		for (i = label ("exms_code"); i <= label ("exms_desc"); i++)
			vars [ i ].scn = 1;
	}
	if (MER_AGENCY)
	{
		for (i = label ("exma_code"); i <= label ("exma_desc"); i++)
			vars [ i ].scn = 1;
	}
	if (SALE_TYPE)
	{
		for (i = label ("sast_code"); i <= label ("sast_desc"); i++)
			vars [ i ].scn = 1;
	}
	if (SALE_GRP)
	{
		for (i = label ("sasg_code"); i <= label ("sasg_desc"); i++)
			vars [ i ].scn = 1;
	}
	if (SALE_POS)
	{
		for (i = label ("sasp_code"); i <= label ("sasp_desc"); i++)
			vars [ i ].scn = 1;
	}
	if (SALE_CA)
	{
		for (i = label ("saca_code"); i <= label ("saca_desc"); i++)
			vars [ i ].scn = 1;
	}
	if (REC_TYPE)
	{
		for (i = label ("rtype_code"); i <= label ("fwd_stat"); i++)
			vars [ i ].scn = 1;
	}

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	no_edit (3);

	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= 0; 
		edit_exit 	= 0;
		prog_exit 	= 0;
		restart 	= 0;
		search_ok 	= 1;

		local_rec.up_sman_hash [0]	=	0L;
		local_rec.up_sman_hash [1]	=	0L;
		local_rec.up_sman_hash [2]	=	0L;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (prog_exit)
			continue;
		
		if (restart)
			UnlockAll ();
		else
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
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (AREA)
	{
		open_rec (sasg, sasg_list, SASG_NO_FIELDS, "sasg_id_no");
		open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
		open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("AreaMaster.txt");
	}
	if (CTYPE)
	{
		open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("CustomerTypeMaster.txt");
	}
	if (SMAN)
	{
		abc_alias (exsf2, exsf);
		open_rec (exsf2, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
		open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
		open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
		open_rec (sasg, sasg_list, SASG_NO_FIELDS, "sasg_id_no");
		open_rec (sasp, sasp_list, SASP_NO_FIELDS, "sasp_id_no");
		open_rec (sast, sast_list, SAST_NO_FIELDS, "sast_id_no");
		open_rec (cfhr, cfhr_list, CFHR_NO_FIELDS, "cfhr_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("SalespersonMaster.txt");
	}

	if (INSTR)
	{
		open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("InstructionMaster.txt");
	}
	if (CNTRCT)
	{
		open_rec (exct, exct_list, EXCT_NO_FIELDS, "exct_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("ContractTypeMaster.txt");
	}
	if (MKT_SS)
	{
		open_rec (exms, exms_list, EXMS_NO_FIELDS, "exms_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("MarketSupplyStatus.txt");
	}

	if (MER_AGENCY)
	{
		open_rec (exma, exma_list, EXMA_NO_FIELDS, "exma_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("MerchandiserAgencyMaster.txt");
	}

	if (SALE_TYPE)
	{
		open_rec (sast, sast_list, SAST_NO_FIELDS, "sast_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("SalesTypeMaster.txt");
	}

	if (SALE_GRP)
	{
		open_rec (sasg, sasg_list, SASG_NO_FIELDS, "sasg_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("SalesGroupMaster.txt");
	}

	if (SALE_POS)
	{
		open_rec (sasp, sasp_list, SASP_NO_FIELDS, "sasp_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("SalesPositionMaster.txt");
	}

	if (SALE_CA)
	{
		open_rec (saca, saca_list, SACA_NO_FIELDS, "saca_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("SalesCallActivity.txt");
	}

	if (REC_TYPE)
	{
		open_rec (curt, curt_list, CURT_NO_FIELDS, "curt_id_no");
		/*
		 * Open audit file.
		 */
		OpenAuditFile ("CustomerReceiptType.txt");
	}
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	if (AREA)
	{
		abc_fclose (exaf);
		abc_fclose (sasg);
		abc_fclose (exsf);
	}
	if (CTYPE)
		abc_fclose (excl);

	if (SMAN)
	{
		abc_fclose (exsf);
		abc_fclose (exsf2);
		abc_fclose (exaf);
		abc_fclose (sasg);
		abc_fclose (sasp);
		abc_fclose (sast);
		abc_fclose (cfhr);
	}

	if (INSTR)
		abc_fclose (exsi);

	if (CNTRCT)
		abc_fclose (exct);

	if (MKT_SS)
		abc_fclose (exms);

	if (MER_AGENCY)
		abc_fclose (exma);

	if (SALE_TYPE)
		abc_fclose (sast);

	if (SALE_GRP)
		abc_fclose (sasg);

	if (SALE_POS)
		abc_fclose (sasp);

	if (SALE_CA)
		abc_fclose (saca);

	if (REC_TYPE)
		abc_fclose (curt);

	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("area_code"))
	{
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (exaf_rec.area_code, "  "))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			NewCode = FALSE;
			entry_exit = 1;
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&exaf_rec, sizeof (exaf_rec));
		}
		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate area description. |
	----------------------------*/
	if (LCHECK ("area_desc"))
	{
		if (!strcmp (exaf_rec.area, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| validate customer type Code . |
	-------------------------------*/
	if (LCHECK ("c_code"))
	{
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (excl_rec.class_type, "   "))
		{
			errmess (ML ("Customer type code cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		NewCode = FALSE;
		strcpy (excl_rec.co_no,comm_rec.co_no);
		cc = find_rec (excl, &excl_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&excl_rec, sizeof (excl_rec));
			entry_exit = 1;
		}

		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate area description. |
	----------------------------*/
	if (LCHECK ("ctype_desc"))
	{
		if (!strcmp (excl_rec.class_desc, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("sman_code"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (exsf_rec.salesman_no, "  "))
		{
			errmess (ML ("Salesperson Number Cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
	
		NewCode = FALSE;
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "w");
		if (cc) 
		{
			NewCode = TRUE;

		}
		else    
		{
			FindMisc ();
			DSP_FLD ("sell_grp_desc");
			DSP_FLD ("pos_desc");
			DSP_FLD ("type_desc");
			DSP_FLD ("sarea_desc");
			DSP_FLD ("sal_stat");
			entry_exit = 1;
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&exsf_rec, sizeof (exsf_rec));
		}

		return (EXIT_SUCCESS);
	}
	/*--------------------------------
	| Validate Salesman description. |
	--------------------------------*/
	if (LCHECK ("sman_name"))
	{
		if (!strcmp (exsf_rec.salesman, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("up_code1"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.up_sman [0], " ");
			strcpy (local_rec.up_desc [0], " ");
			local_rec.up_sman_hash [0]	=	0L;
			DSP_FLD ("up_desc1");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchExsf2 (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsf2_rec.co_no,comm_rec.co_no);
		strcpy (exsf2_rec.salesman_no,local_rec.up_sman [0]);
		cc = find_rec (exsf2, &exsf2_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (!strcmp (exsf_rec.salesman_no, exsf2_rec.salesman_no))
		{
			errmess (ML ("Sales Manager cannot be the same as salesperson"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.up_desc [0], exsf2_rec.salesman);
		local_rec.up_sman_hash [0]	= exsf2_rec.hhsf_hash;
		DSP_FLD ("up_desc1");

		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("up_code2"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.up_sman [1], " ");
			strcpy (local_rec.up_desc [1], " ");
			local_rec.up_sman_hash [1]	=	0L;
			DSP_FLD ("up_desc2");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchExsf2 (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsf2_rec.co_no,comm_rec.co_no);
		strcpy (exsf2_rec.salesman_no,local_rec.up_sman [1]);
		cc = find_rec (exsf2, &exsf2_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (!strcmp (exsf_rec.salesman_no, exsf2_rec.salesman_no))
		{
			errmess (ML ("Sales Manager cannot be the same as salesperson"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.up_desc [1], exsf2_rec.salesman);
		local_rec.up_sman_hash [1]	=	exsf2_rec.hhsf_hash;
		DSP_FLD ("up_desc2");

		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("up_code3"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.up_sman [2], " ");
			strcpy (local_rec.up_desc [2], " ");
			local_rec.up_sman_hash [2]	=	0L;
			DSP_FLD ("up_desc3");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchExsf2 (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsf2_rec.co_no,comm_rec.co_no);
		strcpy (exsf2_rec.salesman_no,local_rec.up_sman [2]);
		cc = find_rec (exsf2, &exsf2_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (!strcmp (exsf_rec.salesman_no, exsf2_rec.salesman_no))
		{
			errmess (ML ("Sales Manager cannot be the same as salesperson"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.up_desc [2], exsf2_rec.salesman);
		local_rec.up_sman_hash [2]	=	exsf2_rec.hhsf_hash;
		DSP_FLD ("up_desc3");

		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("i_code"))
	{
		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsi_rec.co_no,comm_rec.co_no);
		cc = find_rec (exsi, &exsi_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			NewCode = FALSE;
			entry_exit = 1;
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&exsi_rec, sizeof (exsi_rec));
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------------------
	| Validate Special Instruction Code. |
	------------------------------------*/
	if (LCHECK ("inst"))
	{
		if (!strcmp (exsi_rec.inst_text, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Contract Code . |
	--------------------------*/
	if (LCHECK ("cnt_code"))
	{
		if (SRCH_KEY)
		{
			SrchExct (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (exct_rec.cont_type, "   "))
		{
			errmess ("Contract type code cannot be blank");
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		NewCode = FALSE;
		strcpy (exct_rec.co_no,comm_rec.co_no);
		cc = find_rec ("exct", &exct_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&exct_rec, sizeof (exct_rec));
			entry_exit = 1;
		}

		return (EXIT_SUCCESS);
	}
		 
	/*--------------------------------
	| Validate Contract description. |
	--------------------------------*/
	if (LCHECK ("cnt_desc"))
	{
		if (!strcmp (exct_rec.cont_desc, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Sales Group Code. |
	------------------------------*/
	if (LCHECK ("sell_grp"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchSasg (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (exsf_rec.sell_grp, "  "))
		{
			errmess (ML ("Sales group code cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (sasg_rec.co_no, comm_rec.co_no);
		strcpy (sasg_rec.sell_grp, exsf_rec.sell_grp);
		cc = find_rec (sasg, &sasg_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML ("Sales group not found."));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("sell_grp_desc");
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Position Code. |
	-------------------------*/
	if (LCHECK ("pos_code"))
	{
		if (SRCH_KEY)
		{
			SrchSasp (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (exsf_rec.sell_pos, "  "))
		{
			errmess (ML ("Sales position code cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (sasp_rec.co_no, comm_rec.co_no);
		strcpy (sasp_rec.pos_code, exsf_rec.sell_pos);
		cc = find_rec (sasp, &sasp_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML ("Sales position code not found."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pos_desc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Sales Type Code. |
	-----------------------------*/
	if (LCHECK ("type_code"))
	{
		if (SRCH_KEY)
		{
			SrchSast (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (exsf_rec.sell_type, "  "))
		{
			errmess (ML ("Sales type code cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (sast_rec.co_no, comm_rec.co_no);
		strcpy (sast_rec.code, exsf_rec.sell_type);
		cc = find_rec (sast, &sast_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML ("Sales type code not found."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("type_desc");
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Salesman Area Code. |
	------------------------------*/
	if (LCHECK ("sarea_code"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (exsf_rec.area_code, "  "))
		{
			errmess (ML ("Area Code Cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (exaf_rec.co_no, comm_rec.co_no);
		strcpy (exaf_rec.area_code, exsf_rec.area_code);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("sarea_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------------
	| Validate Market Supply Status File. |
	-------------------------------------*/
	if (LCHECK ("exms_code"))
	{
		if (SRCH_KEY)
		{
			SrchExms (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (exms_rec.stat_code, "  "))
		{
			errmess (ML ("Supply status code cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (exms_rec.co_no,comm_rec.co_no);
		cc = find_rec (exms, &exms_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			NewCode = FALSE;
			entry_exit = 1;

			DSP_FLD ("exms_desc");
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&exms_rec, sizeof (exms_rec));
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------------------
	| Validate Supply Status description. |
	--------------------------------------*/
	if (LCHECK ("exms_desc"))
	{
		if (!strcmp (exms_rec.desc, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------------
	| Validate Merchandiser Agency File. |
	------------------------------------*/
	if (LCHECK ("exma_code"))
	{
		if (SRCH_KEY)
		{
			SrchExma (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (exma_rec.code, "  "))
		{
			errmess (ML ("Agency code cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (exma_rec.co_no,comm_rec.co_no);
		cc = find_rec (exma, &exma_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			NewCode = FALSE;
			entry_exit = 1;
			DSP_FLD ("exma_desc");
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&exma_rec, sizeof (exma_rec));
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Agency description. |
	-------------------------------*/
	if (LCHECK ("exma_desc"))
	{
		if (!strcmp (exma_rec.desc, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Sales Type File. |
	-----------------------------*/
	if (LCHECK ("sast_code"))
	{
		if (SRCH_KEY)
		{
			SrchSast (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (sast_rec.code, "  "))
		{
			errmess (ML ("Sales type code cannot be blank"));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (sast_rec.co_no,comm_rec.co_no);
		cc = find_rec (sast, &sast_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			NewCode = FALSE;
			entry_exit = 1;
			DSP_FLD ("sast_desc");
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&sast_rec, sizeof (sast_rec));
		}
		return (EXIT_SUCCESS);
	}
	/*----------------------------------
	| Validate Sales Type description. |
	----------------------------------*/
	if (LCHECK ("sast_desc"))
	{
		if (!strcmp (sast_rec.desc, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| Validate Sales Position File. |
	--------------------------------*/
	if (LCHECK ("sasp_code"))
	{
		if (SRCH_KEY)
		{
			SrchSasp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (sasp_rec.pos_code, "  "))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (sasp_rec.co_no,comm_rec.co_no);
		cc = find_rec (sasp, &sasp_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			NewCode = FALSE;
			entry_exit = 1;
			DSP_FLD ("sasp_desc");
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&sasp_rec, sizeof (sasp_rec));
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------------------
	| Validate Sales Position description. |
	--------------------------------------*/
	if (LCHECK ("sasp_desc"))
	{
		if (!strcmp (sasp_rec.pos_desc, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Sales group File. |
	----------------------------*/
	if (LCHECK ("sasg_code"))
	{
		if (SRCH_KEY)
		{
			SrchSasg (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (sasg_rec.sell_grp, "  "))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (sasg_rec.co_no,comm_rec.co_no);
		cc = find_rec (sasg, &sasg_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			NewCode = FALSE;
			entry_exit = 1;
			DSP_FLD ("sasg_desc");
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&sasg_rec, sizeof (sasg_rec));
		}
		return (EXIT_SUCCESS);
	}
	/*--------------------------------------
	| Validate Sales Position description. |
	--------------------------------------*/
	if (LCHECK ("sasg_desc"))
	{
		if (!strcmp (sasg_rec.desc, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------
	| Validate Sales call activity File. |
	--------------------------------------*/
	if (LCHECK ("saca_code"))
	{
		if (SRCH_KEY)
		{
			SrchSaca (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (saca_rec.code, "  "))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (saca_rec.co_no,comm_rec.co_no);
		cc = find_rec (saca, &saca_rec, COMPARISON, "w");
		if (cc) 
			NewCode = TRUE;
		else    
		{
			NewCode = FALSE;
			entry_exit = 1;
			DSP_FLD ("saca_desc");
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&saca_rec, sizeof (saca_rec));
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------------------------
	| Validate Sales call activity description. |
	-------------------------------------------*/
	if (LCHECK ("saca_desc"))
	{
		if (!strcmp (saca_rec.desc, BlankDesc))
		{
			errmess (ML (mlMenuMess711));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Receipt type . |
	--------------------------*/
	if (LCHECK ("rtype_code"))
	{
		if (SRCH_KEY)
		{
			SrchCurt (temp_str);
			return (EXIT_SUCCESS);
		}

		NewCode = 0;
		strcpy (curt_rec.co_no,comm_rec.co_no);
		cc = find_rec ("curt", &curt_rec, COMPARISON, "u");
		if (cc) 
			NewCode = 1;
		else    
		{
			entry_exit = 1;
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&curt_rec, sizeof (curt_rec));
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
FindMisc (void)
{
	strcpy (sasg_rec.co_no, comm_rec.co_no);
	strcpy (sasg_rec.sell_grp, exsf_rec.sell_grp);
	if (find_rec (sasg, &sasg_rec, COMPARISON, "r"))
		strcpy (sasg_rec.desc, " ");

	strcpy (sasp_rec.co_no, comm_rec.co_no);
	strcpy (sasp_rec.pos_code, exsf_rec.sell_pos);
	if (find_rec (sasp, &sasp_rec, COMPARISON, "r"))
		strcpy (sasp_rec.pos_desc, " ");

	strcpy (sast_rec.co_no, comm_rec.co_no);
	strcpy (sast_rec.code, exsf_rec.sell_type);
	if (find_rec (sast, &sast_rec, COMPARISON, "r"))
		strcpy (sast_rec.desc, " ");

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	strcpy (exaf_rec.area_code, exsf_rec.area_code);
	if (find_rec (exaf, &exaf_rec, COMPARISON, "r"))
		strcpy (exaf_rec.area, " ");
	
	abc_selfield (exsf2, "exsf_hhsf_hash");

	exsf2_rec.hhsf_hash	=	exsf_up_sman [0];
	if (find_rec (exsf2, &exsf2_rec, COMPARISON, "r"))
	{
		strcpy (local_rec.up_sman [0], " ");
		strcpy (local_rec.up_desc [0], " ");
	}
	else
	{
		strcpy (local_rec.up_sman [0], exsf2_rec.salesman_no);
		strcpy (local_rec.up_desc [0], exsf2_rec.salesman);
	}
	exsf2_rec.hhsf_hash	=	exsf_up_sman [1];
	if (find_rec (exsf2, &exsf2_rec, COMPARISON, "r"))
	{
		strcpy (local_rec.up_sman [1], " ");
		strcpy (local_rec.up_desc [1], " ");
	}
	else
	{
		strcpy (local_rec.up_sman [1], exsf2_rec.salesman_no);
		strcpy (local_rec.up_desc [1], exsf2_rec.salesman);
	}
	exsf2_rec.hhsf_hash	=	exsf_up_sman [2];
	if (find_rec (exsf2, &exsf2_rec, COMPARISON, "r"))
	{
		strcpy (local_rec.up_sman [2], " ");
		strcpy (local_rec.up_desc [2], " ");
	}
	else
	{
		strcpy (local_rec.up_sman [2], exsf2_rec.salesman_no);
		strcpy (local_rec.up_desc [2], exsf2_rec.salesman);
	}
	local_rec.up_sman_hash [0]	=	exsf_up_sman [0];
	local_rec.up_sman_hash [1]	=	exsf_up_sman [1];
	local_rec.up_sman_hash [2]	=	exsf_up_sman [2];

	abc_selfield (exsf2, "exsf_id_no");
	return (EXIT_SUCCESS);
}

/*==================
| Updated records. |
==================*/
void
Update (void)
{
	if (AREA)
		UpdateExaf ();

	if (CTYPE)
		UpdateExcl ();

	if (SMAN)
		UpdateExsf ();

	if (INSTR)
		UpdateExsi ();

	if (CNTRCT)
		UpdateExct ();

	if (MKT_SS)
		UpdateExms ();

	if (MER_AGENCY)
		UpdateExma ();

	if (SALE_TYPE)
		UpdateSast ();

	if (SALE_GRP)
		UpdateSasg ();

	if (SALE_POS)
		UpdateSasp ();

	if (SALE_CA)
		UpdateSaca ();

	if (REC_TYPE)
		UpdateCurt ();
}

/*=============================
| Add or update area record . |
=============================*/
void
UpdateCurt (void)
{
	int		exitLoop;

	strcpy (curt_rec.co_no, comm_rec.co_no);
	strcpy (curt_rec.stat_flag, "0");
	if (NewCode)
	{
		cc = abc_add (curt, &curt_rec);
		if (cc) 
			file_err (cc, curt, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (curt, &curt_rec);
				if (cc) 
					file_err (cc, curt, "DBUPDATE");
				/*
				 * Update changes audit record.
				 */

		 		sprintf (err_str, "%s : %s (%s)", ML ("Receipt Type"), curt_rec.chq_type, curt_rec.chq_desc);
				 AuditFileAdd (err_str, &curt_rec, curt_list, CURT_NO_FIELDS);
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (curt);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
					clear_mess ();
					cc = abc_delete (curt);
					if (cc)
						file_err (cc, curt, "DBUPDATE");
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (curt);
}

/*=============================
| Add or update area record . |
=============================*/
void
UpdateExaf (void)
{
	int		exitLoop;

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	strcpy (exaf_rec.stat_flag, "0");
	if (NewCode)
	{
		cc = abc_add (exaf, &exaf_rec);
		if (cc) 
			file_err (cc, exaf, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (exaf, &exaf_rec);
				if (cc) 
					file_err (cc, exaf, "DBUPDATE");
				ActiveExsf ();
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Area"), exaf_rec.area_code, exaf_rec.area);
				 AuditFileAdd (err_str, &exaf_rec, exaf_list, EXAF_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (exaf);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (ExafDelOk ())
				{
					clear_mess ();
					cc = abc_delete (exaf);
					if (cc)
						file_err (cc, exaf, "DBUPDATE");
				}
				else
				{
					sprintf (err_str, ML (mlMenuMess022),badFileName);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (exaf);
	strcpy (local_rec.previousCode, exaf_rec.area_code);

}

/*==============================
| Add or update class record . |
==============================*/
void
UpdateExcl (void)
{
	int		exitLoop;

	strcpy (excl_rec.stat_flag, "0");
	if (NewCode)
	{
		cc = abc_add (excl, &excl_rec);
		if (cc) 
			file_err (cc, excl, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (excl, &excl_rec);
				if (cc) 
					file_err (cc, excl, "DBUPDATE");
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Class"), excl_rec.class_type, excl_rec.class_desc);
				 AuditFileAdd (err_str, &excl_rec, excl_list, EXCL_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (excl);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (ExclDelOk ())
				{
					clear_mess ();
					cc = abc_delete (excl);
					if (cc)
						file_err (cc, excl, "DBUPDATE");
				}
				else
				{
					sprintf (err_str, ML (mlMenuMess022),badFileName);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (excl);
	strcpy (local_rec.previousCode, excl_rec.class_type);

}

/*==========================================
| Add or update Market Supply Status file. |
==========================================*/
void
UpdateExms (void)
{
	int		exitLoop;

	strcpy (exms_rec.co_no, comm_rec.co_no);
	if (NewCode)
	{
		cc = abc_add (exms, &exms_rec);
		if (cc) 
			file_err (cc, exms, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (exms, &exms_rec);
				if (cc) 
					file_err (cc, exms, "DBUPDATE");
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Status"), exms_rec.stat_code, exms_rec.desc);
				 AuditFileAdd (err_str, &exms_rec, exms_list, EXMS_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (exms);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				clear_mess ();
				cc = abc_delete (exms);
				if (cc)
					file_err (cc, exms, "DBUPDATE");
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (exms);
	strcpy (local_rec.previousCode, exms_rec.stat_code);
}

/*=========================================
| Add or update Merchandiser Agency file. |
=========================================*/
void
UpdateExma (void)
{
	int		exitLoop;

	strcpy (exma_rec.co_no, comm_rec.co_no);
	if (NewCode)
	{
		cc = abc_add (exma, &exma_rec);
		if (cc) 
			file_err (cc, exma, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (exma, &exma_rec);
				if (cc) 
					file_err (cc, exma, "DBUPDATE");
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Code"), exma_rec.code, exma_rec.desc);
				 AuditFileAdd (err_str, &exma_rec, exma_list, EXMA_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (exma);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				clear_mess ();
				cc = abc_delete (exma);
				if (cc)
					file_err (cc, exma, "DBUPDATE");
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (exma);
	strcpy (local_rec.previousCode, exma_rec.code);
}

/*==================================
| Add or update Sales Type file. |
==================================*/
void
UpdateSast (void)
{
	int		exitLoop;

	strcpy (sast_rec.co_no, comm_rec.co_no);
	if (NewCode)
	{
		cc = abc_add (sast, &sast_rec);
		if (cc) 
			file_err (cc, sast, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (sast, &sast_rec);
				if (cc) 
					file_err (cc, sast, "DBUPDATE");
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Code"), sast_rec.code, sast_rec.desc);
				 AuditFileAdd (err_str, &sast_rec, sast_list, SAST_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (sast);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				clear_mess ();
				cc = abc_delete (sast);
				if (cc)
					file_err (cc, sast, "DBUPDATE");
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (sast);
	strcpy (local_rec.previousCode, sast_rec.code);
}

/*===================================
| Add or update Sales Group file. |
===================================*/
void
UpdateSasg (void)
{
	int		exitLoop;

	strcpy (sasg_rec.co_no, comm_rec.co_no);
	if (NewCode)
	{
		cc = abc_add (sasg, &sasg_rec);
		if (cc) 
			file_err (cc, sasg, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (sasg, &sasg_rec);
				if (cc) 
					file_err (cc, sasg, "DBUPDATE");
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Group"), sasg_rec.sell_grp, sasg_rec.desc);
				 AuditFileAdd (err_str, &sasg_rec, sasg_list, SASG_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (sasg);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				clear_mess ();
				cc = abc_delete (sasg);
				if (cc)
					file_err (cc, sasg, "DBUPDATE");
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (sasg);
	strcpy (local_rec.previousCode, sasg_rec.sell_grp);
}

/*===========================================
| Add or update Sales call activity file. |
===========================================*/
void
UpdateSaca (void)
{
	int		exitLoop;

	strcpy (saca_rec.co_no, comm_rec.co_no);
	if (NewCode)
	{
		cc = abc_add (saca, &saca_rec);
		if (cc) 
			file_err (cc, saca, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (saca, &saca_rec);
				if (cc) 
					file_err (cc, saca, "DBUPDATE");
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Code"), saca_rec.code, saca_rec.desc);
				 AuditFileAdd (err_str, &saca_rec, saca_list, SACA_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (saca);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				clear_mess ();
				cc = abc_delete (saca);
				if (cc)
					file_err (cc, saca, "DBUPDATE");
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (saca);
	strcpy (local_rec.previousCode, saca_rec.code);
}

/*======================================
| Add or update Sales Position file. |
======================================*/
void
UpdateSasp (void)
{
	int		exitLoop;

	strcpy (sasp_rec.co_no, comm_rec.co_no);
	if (NewCode)
	{
		cc = abc_add (sasp, &sasp_rec);
		if (cc) 
			file_err (cc, sasp, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (sasp, &sasp_rec);
				if (cc) 
					file_err (cc, sasp, "DBUPDATE");
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Code"), sasp_rec.pos_code, sasp_rec.pos_desc);
				 AuditFileAdd (err_str, &sasp_rec, sasp_list, SASP_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (sasp);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				clear_mess ();
				cc = abc_delete (sasp);
				if (cc)
					file_err (cc, sasp, "DBUPDATE");
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (sasp);
	strcpy (local_rec.previousCode, sasp_rec.pos_code);
}

/*======================================
| Add or update contract type record . |
======================================*/
void
UpdateExct (void)
{
	int		exitLoop;

	strcpy (exct_rec.stat_flag,"0");
	if (NewCode)
	{
		cc = abc_add (exct, &exct_rec);
		if (cc) 
			file_err (cc, exct, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (exct, &exct_rec);
				if (cc) 
					file_err (cc, exct, "DBUPDATE");
				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Contract"), exct_rec.cont_type, exct_rec.cont_desc);
				 AuditFileAdd (err_str, &exct_rec, exct_list, EXCT_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (exct);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (ExctDelOk ())
				{
					clear_mess ();
					cc = abc_delete (exct);
					if (cc)
						file_err (cc, exct, "DBUPDATE");
				}
				else
				{
					sprintf (err_str, ML (mlMenuMess022),badFileName);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (exct);
	strcpy (local_rec.previousCode, exct_rec.cont_type);

}

/*=========================================
| Add / Update / Delete salesman record . |
=========================================*/
void
UpdateExsf (void)
{
	int		exitLoop;

	strcpy (exsf_rec.stat_flag, "0");

	exsf_up_sman [0]	=	local_rec.up_sman_hash [0];
	exsf_up_sman [1]	=	local_rec.up_sman_hash [1];
	exsf_up_sman [2]	=	local_rec.up_sman_hash [2];
		
	if (NewCode)
	{
		strcpy (exsf_rec.update,"A");
		cc = abc_add (exsf, &exsf_rec);
		if (cc) 
			file_err (cc, exsf, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .  ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				if (!strcmp (exsf_rec.update,"A"))
					strcpy (exsf_rec.update,"A");
				else
					strcpy (exsf_rec.update,"U");
				cc = abc_update (exsf, &exsf_rec);
				if (cc) 
					file_err (cc, exsf, "DBUPDATE");

				exitLoop = TRUE;
				/*
				 * Update changes audit record.
				 */
		 		sprintf (err_str, "%s : %s (%s)", ML ("Salesperson"), exsf_rec.salesman_no, exsf_rec.salesman);
				 AuditFileAdd (err_str, &exsf_rec, exsf_list, EXSF_NO_FIELDS);
				break;
	
			case SEL_IGNORE :
				abc_unlock (exsf);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (ExsfDelOk ())
				{
					clear_mess ();
					cc = abc_delete (exsf);
					if (cc)
						file_err (cc, exsf, "DBUPDATE");
				}
				else
				{
					sprintf (err_str, ML (mlMenuMess022),badFileName);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	strcpy (local_rec.previousCode, exsf_rec.salesman_no);

}

/*===========================
| Check whether it is OK to |
| delete the exsf record.   |
| Files checked are :       |
| arhr                      |
| arln                      |
| ccmr                      |
| cohr                      |
| coln                      |
| cumr                      |
| cusf                      |
| cuwk                      |
| inls                      |
| qthr                      |
| qtph                      |
| sadf                      |
| sale                      |
| sohr                      |
| tmop                      |
| tmpm                      |
===========================*/
int
ExsfDelOk (void)
{
	/*-------------
	| Check arhr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), arhr);
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkArhr (F_EXSF))
		return (FALSE);

	/*-------------
	| Check ccmr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), ccmr);
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_co_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (ccmr_rec.sman_no, exsf_rec.salesman_no))
		{
			abc_fclose (ccmr);
			strcpy (badFileName, ccmr);
			return (FALSE);
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	abc_fclose (ccmr);

	/*-------------
	| Check cohr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), cohr);
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkCohr (F_EXSF))
		return (FALSE);

	/*-------------
	| Check cumr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), cumr);
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkCumr (F_EXSF))
		return (FALSE);

	/*-------------
	| Check cuwk. |
	-------------*/
/*
	sprintf (err_str, ML (mlMenuMess023), "cuwk");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkCuwk (F_EXSF))
	{
		strcpy (badFileName, cuwk);
		return (FALSE);
	}
*/

	/*-------------
	| Check inls. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "inls");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkInls (F_EXSF))
	{
		strcpy (badFileName, inls);
		return (FALSE);
	}

	/*-------------
	| Check qthr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "qthr");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no");
	strcpy (qthr_rec.co_no, comm_rec.co_no);
	strcpy (qthr_rec.br_no, "  ");
	strcpy (qthr_rec.quote_no, "        ");
	qthr_rec.hhcu_hash	=	0L;
	qthr_rec.hhqt_hash = 0L;
	cc = find_rec (qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strcmp (qthr_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (qthr_rec.sman_code, exsf_rec.salesman_no))
		{
			abc_fclose (qthr);
			strcpy (badFileName, qthr);
			return (FALSE);
		}
		cc = find_rec (qthr, &qthr_rec, NEXT, "r");
	}
	abc_fclose (qthr);

	/*-------------
	| Check qtph. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "qtph");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (qtph, qtph_list, QTPH_NO_FIELDS, "qtph_id_no");
	strcpy (qtph_rec.co_no, comm_rec.co_no);
	qtph_rec.hhph_hash = 0L;
	cc = find_rec (qtph, &qtph_rec, GTEQ, "r");
	while (!cc && !strcmp (qtph_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (qtph_rec.dt_sman, exsf_rec.salesman_no))
		{
			abc_fclose (qtph);
			strcpy (badFileName, qtph);
			return (FALSE);
		}
		cc = find_rec (qtph, &qtph_rec, NEXT, "r");
	}
	abc_fclose (qtph);

	/*-------------
	| Check sadf. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "sadf");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkSadf (F_EXSF))
	{
		strcpy (badFileName, sadf);
		return (FALSE);
	}

	/*-------------
	| Check sale. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "sale");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkSale (F_EXSF))
	{
		strcpy (badFileName, sale);
		return (FALSE);
	}

	/*-------------
	| Check sohr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "sohr");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkSohr (F_EXSF))
	{
		strcpy (badFileName, sohr);
		return (FALSE);
	}

	/*-------------
	| Check tmop. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "tmop");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	strcpy (tmop_rec.co_no, comm_rec.co_no);
	strcpy (tmop_rec.op_id, "              ");
	cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
	while (!cc && !strcmp (tmop_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (tmop_rec.sman_code, exsf_rec.salesman_no))
		{
			abc_fclose (tmop);
			strcpy (badFileName, tmop);
			return (FALSE);
		}
		cc = find_rec (tmop, &tmop_rec, NEXT, "r");
	}
	abc_fclose (tmop);

	return (TRUE);
}

/*===========================
| Check whether it is OK to |
| delete the exct record.   |
| Files checked are :       |
| cumr                      |
===========================*/
int
ExctDelOk (void)
{
	/*-------------
	| Check exct. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), cumr);
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkCumr (F_EXCT))
		return (FALSE);

	return (TRUE);
}

/*===========================
| Check whether it is OK to |
| delete the excl record.   |
| Files checked are :       |
| cudb                      |
| cumr                      |
| incp                      |
| inds                      |
| inpr                      |
| mhdr                      |
| sact                      |
| sale                      |
===========================*/
int
ExclDelOk (void)
{
	/*-------------
	| Check cudb. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "cudb");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (cudb, cudb_list, CUDB_NO_FIELDS, "cudb_id_no");
	strcpy (cudb_rec.co_no,      comm_rec.co_no);
	strcpy (cudb_rec.class_type, excl_rec.class_type);
	cc = find_rec (cudb, &cudb_rec, GTEQ, "r");
	if (!cc &&
		!strcmp (cudb_rec.co_no, comm_rec.co_no) &&
		!strcmp (cudb_rec.class_type, excl_rec.class_type))
	{
		abc_fclose (cudb);
		strcpy (badFileName, cudb);
		return (FALSE);
	}
	abc_fclose (cudb);

	/*-------------
	| Check cumr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), cumr);
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkCumr (F_EXCL))
		return (FALSE);

	/*-------------
	| Check incp. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "incp");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (incp, incp_list, INCP_NO_FIELDS, "incp_id_no");
	sprintf (incp_rec.key,      "%2s    ", comm_rec.co_no);
	strcpy (incp_rec.curr_code, "   ");
	strcpy (incp_rec.status,    " ");
	incp_rec.hhcu_hash = 0L;
	strcpy (incp_rec.area_code,  "  ");
	strcpy (incp_rec.cus_type,  "   ");
	incp_rec.hhbr_hash = 0L;
	incp_rec.date_from = 0L;
	cc = find_rec (incp, &incp_rec, GTEQ, "r");
	while (!cc && !strncmp (incp_rec.key, comm_rec.co_no, 2))
	{
		if (!strcmp (incp_rec.cus_type, excl_rec.class_type))
		{
			abc_fclose (incp);
			strcpy (badFileName, incp);
			return (FALSE);
		}

		cc = find_rec (incp, &incp_rec, NEXT, "r");
	}
	abc_fclose (incp);

	/*-------------
	| Check inds. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "inds");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (inds, inds_list, INDS_NO_FIELDS, "inds_id_no");
	sprintf (inds_rec.co_no, comm_rec.co_no);
	sprintf (inds_rec.br_no, "  ");
	inds_rec.hhcc_hash = 0L;
	inds_rec.hhcu_hash = 0L;
	strcpy (inds_rec.cust_type,  "   ");
	inds_rec.hhbr_hash = 0L;
	strcpy (inds_rec.category,  "           ");
	strcpy (inds_rec.sel_group,  "      ");
	cc = find_rec (inds, &inds_rec, GTEQ, "r");
	while (!cc && !strcmp (inds_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (inds_rec.cust_type, excl_rec.class_type))
		{
			abc_fclose (inds);
			strcpy (badFileName, inds);
			return (FALSE);
		}

		cc = find_rec (inds, &inds_rec, NEXT, "r");
	}
	abc_fclose (inds);

	/*-------------
	| Check inpr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "inpr");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inpr, inpr_list, INPR_NO_FIELDS, "inpr_hhbr_hash");
	sprintf (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "                ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		cc = find_hash (inpr, &inpr_rec, GTEQ, "r", inmr_rec.hhbr_hash);
		while (!cc && inpr_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (!strcmp (inpr_rec.cust_type, excl_rec.class_type))
			{
				abc_fclose (inpr);
				strcpy (badFileName, inpr);
				return (FALSE);
			}

			cc = find_hash (inpr, &inpr_rec, NEXT, "r", inmr_rec.hhbr_hash);
		}

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	abc_fclose (inmr);
	abc_fclose (inpr);

	/*-------------
	| Check mhdr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "mhdr");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkMhdr (F_EXCL))
		return (FALSE);

	/*-------------
	| Check sact. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "sact");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (sact, sact_list, SACT_NO_FIELDS, "sact_id_no");
	strcpy (sact_rec.co_no,      comm_rec.co_no);
	strcpy (sact_rec.class_type, excl_rec.class_type);
	strcpy (sact_rec.year_flag,  " ");
	cc = find_rec (sact, &sact_rec, GTEQ, "r");
	if (!cc &&
		!strcmp (sact_rec.co_no, comm_rec.co_no) &&
		!strcmp (sact_rec.class_type, excl_rec.class_type))
	{
		abc_fclose (sact);
		strcpy (badFileName, sact);
		return (FALSE);
	}
	abc_fclose (sact);

	/*-------------
	| Check sale. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "sale");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkSale (F_EXCL))
	{
		strcpy (badFileName, sale);
		return (FALSE);
	}

	return (TRUE);
}

/*===========================
| Check whether it is OK to |
| delete the exaf record.   |
| Files checked are :       |
| arhr                      |
| cfhs                      |
| cfln                      |
| cohr                      |
| cumr                      |
| cuwk                      |
| esmr                      |
| inls                      |
| ithr                      |
| mhdr                      |
| sadf                      |
| sale                      |
| sohr                      |
| tmpm                      |
===========================*/
int
ExafDelOk (void)
{
	/*-------------
	| Check arhr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), arhr);
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkArhr (F_EXAF))
		return (FALSE);

	/*-------------
	| Check cfhs. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "cfhs");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (cfhs, cfhs_list, CFHS_NO_FIELDS, "cfhs_id_no2");
	strcpy (cfhs_rec.co_no, comm_rec.co_no);
	strcpy (cfhs_rec.br_no, "  ");
	strcpy (cfhs_rec.wh_no, "  ");
	cfhs_rec.date = 0L;
	cc = find_rec (cfhs, &cfhs_rec, GTEQ, "r");
	while (!cc && !strcmp (cfhs_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (cfhs_rec.area_code, exaf_rec.area_code))
		{
			abc_fclose (cfhs);
			strcpy (badFileName, cfhs);
			return (FALSE);
		}

		cc = find_rec (cfhs, &cfhs_rec, NEXT, "r");
	}
	abc_fclose (cfhs);

	/*-------------
	| Check cfln. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "cfln");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (cfhr, cfhr_list, CFHR_NO_FIELDS, "cfhr_id_no");
	open_rec (cfln, cfln_list, CFLN_NO_FIELDS, "cfln_id_no");

	strcpy (cfhr_rec.co_no, comm_rec.co_no);
	strcpy (cfhr_rec.br_no, "  ");
	strcpy (cfhr_rec.carr_code, "    ");
	cc = find_rec (cfhr, &cfhr_rec, GTEQ, "r");
	while (!cc && !strcmp (cfhr_rec.co_no, comm_rec.co_no))
	{
		cfln_rec.cfhh_hash = cfhr_rec.cfhh_hash;
		strcpy (cfln_rec.area_code, exaf_rec.area_code);
		cc = find_rec (cfln, &cfln_rec, GTEQ, "r");
		if (!cc &&
			cfln_rec.cfhh_hash == cfhr_rec.cfhh_hash &&
			!strcmp (cfln_rec.area_code, exaf_rec.area_code))
		{
			abc_fclose (cfhr);
			abc_fclose (cfln);
			strcpy (badFileName, cfln);
			return (FALSE);
		}

		cc = find_rec (cfhr, &cfhr_rec, NEXT, "r");
	}
	abc_fclose (cfhr);
	abc_fclose (cfln);

	/*-------------
	| Check cohr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), cohr);
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkCohr (F_EXAF))
		return (FALSE);

	/*-------------
	| Check cumr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), cumr);
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkCumr (F_EXAF))
		return (FALSE);

	/*-------------
	| Check cuwk. |
	-------------*/
/*
	sprintf (err_str, ML (mlMenuMess023), "cuwk");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkCuwk (F_EXAF))
	{
		strcpy (badFileName, cuwk);
		return (FALSE);
	}
*/

	/*-------------
	| Check esmr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), esmr);
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	strcpy (esmr_rec.co_no,  comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (esmr_rec.area_code, exaf_rec.area_code))
		{
			abc_fclose (esmr);
			strcpy (badFileName, esmr);
			return (FALSE);
		}

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	abc_fclose (esmr);

	/*-------------
	| Check inls. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "inls");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkInls (F_EXAF))
	{
		strcpy (badFileName, inls);
		return (FALSE);
	}

	/*-------------
	| Check ithr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "ithr");
	print_mess (err_str);
	sleep (sleepTime);
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_id_no4");
	strcpy (ithr_rec.co_no,  comm_rec.co_no);
	ithr_rec.del_no = 0L;
	cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
	while (!cc && !strcmp (ithr_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (ithr_rec.carr_area, exaf_rec.area_code))
		{
			abc_fclose (ithr);
			strcpy (badFileName, ithr);
			return (FALSE);
		}

		cc = find_rec (ithr, &ithr_rec, NEXT, "r");
	}
	abc_fclose (ithr);

	/*-------------
	| Check mhdr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "mhdr");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkMhdr (F_EXAF))
		return (FALSE);

	/*-------------
	| Check sadf. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "sadf");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkSadf (F_EXAF))
	{
		strcpy (badFileName, sadf);
		return (FALSE);
	}

	/*-------------
	| Check sale. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "sale");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkSale (F_EXAF))
	{
		strcpy (badFileName, sale);
		return (FALSE);
	}

	/*-------------
	| Check sohr. |
	-------------*/
	sprintf (err_str, ML (mlMenuMess023), "sohr");
	print_mess (err_str);
	sleep (sleepTime);
	if (!ChkSohr (F_EXAF))
	{
		strcpy (badFileName, sohr);
		return (FALSE);
	}

	return (TRUE);
}

int
ChkArhr (
 int	chkType)
{
	open_rec (arhr, arhr_list, ARHR_NO_FIELDS, "arhr_id_no");
	open_rec (arln, arln_list, ARLN_NO_FIELDS, "arln_id_no");

	strcpy (arhr_rec.co_no,   comm_rec.co_no);
	strcpy (arhr_rec.br_no,   "  ");
	strcpy (arhr_rec.type,    " ");
	strcpy (arhr_rec.inv_no,  "          ");
	cc = find_rec (arhr, &arhr_rec, GTEQ, "r");
	while (!cc && !strcmp (arhr_rec.co_no, comm_rec.co_no))
	{
		switch (chkType)
		{
		case F_EXSF:
			if (!strcmp (arhr_rec.sale_code, exsf_rec.salesman_no))
			{
				abc_fclose (arhr);
				abc_fclose (arln);
				strcpy (badFileName, arhr);
				return (FALSE);
			}
	
			/*--------------------------
			| Check arln records also. |
			--------------------------*/
			arln_rec.hhco_hash = arhr_rec.hhco_hash;
			arln_rec.line_no = 0;
			cc = find_rec (arln, &arln_rec, GTEQ, "r");
			while (!cc && arln_rec.hhco_hash == arhr_rec.hhco_hash)
			{
				if (!strcmp (arln_rec.sman_code, exsf_rec.salesman_no))
				{
					abc_fclose (arhr);
					abc_fclose (arln);
					strcpy (badFileName, arln);
					return (FALSE);
				}

				cc = find_rec (arln, &arln_rec, NEXT, "r");
			}

			break;

		case F_EXAF:
			if (!strcmp (arhr_rec.area_code, exaf_rec.area_code) ||
				!strcmp (arhr_rec.carr_area, exaf_rec.area_code))
			{
				abc_fclose (arhr);
				abc_fclose (arln);
				strcpy (badFileName, arhr);
				return (FALSE);
			}

			break;
		}
		cc = find_rec (arhr, &arhr_rec, NEXT, "r");
	}

	abc_fclose (arhr);
	abc_fclose (arln);

	return (TRUE);
}

int
ChkCohr (
 int	chkType)
{
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");

	strcpy (cohr_rec.co_no,   comm_rec.co_no);
	strcpy (cohr_rec.br_no,   "  ");
	strcpy (cohr_rec.type,    " ");
	strcpy (cohr_rec.inv_no,  "          ");
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no))
	{
		switch (chkType)
		{
		case F_EXSF:
			if (!strcmp (cohr_rec.sale_code, exsf_rec.salesman_no))
			{
				abc_fclose (cohr);
				abc_fclose (coln);
				strcpy (badFileName, cohr);
				return (FALSE);
			}
	
			/*--------------------------
			| Check coln records also. |
			--------------------------*/
			coln_rec.hhco_hash = cohr_rec.hhco_hash;
			coln_rec.line_no = 0;
			cc = find_rec (coln, &coln_rec, GTEQ, "r");
			while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
			{
				if (!strcmp (coln_rec.sman_code, exsf_rec.salesman_no))
			{
					abc_fclose (cohr);
					abc_fclose (coln);
					strcpy (badFileName, coln);
					return (FALSE);
				}

				cc = find_rec (coln, &coln_rec, NEXT, "r");
			}

			break;

		case F_EXAF:
			if (!strcmp (cohr_rec.area_code, exaf_rec.area_code) ||
				!strcmp (cohr_rec.carr_area, exaf_rec.area_code))
			{
				abc_fclose (cohr);
				abc_fclose (coln);
				strcpy (badFileName, cohr);
				return (FALSE);
			}

			break;
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}

	abc_fclose (cohr);
	abc_fclose (coln);

	return (TRUE);
}

int
ChkCumr (
 int	chkType)
{
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_co_no");
	open_rec (cusf, cusf_list, CUSF_NO_FIELDS, "cusf_id_no");
	open_rec (tmpm, tmpm_list, TMPM_NO_FIELDS, "tmpm_hhcu_hash");

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		switch (chkType)
		{
		case F_EXSF:
			if (!strcmp (cumr_rec.sman_code, exsf_rec.salesman_no))
			{
				abc_fclose (cumr);
				abc_fclose (cusf);
				abc_fclose (tmpm);
				strcpy (badFileName, cumr);
				return (FALSE);
			}

			/*------------------
			| Check cusf also. |
			------------------*/
			cusf_rec.hhcu_hash = cumr_rec.hhcu_hash;
			cusf_rec.line_no = 0;
			cc = find_rec (cusf, &cusf_rec, GTEQ, "r");
			while (!cc && cusf_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
				if (!strcmp (cusf_rec.sman, exsf_rec.salesman_no))
				{
					abc_fclose (cumr);
					abc_fclose (cusf);
					abc_fclose (tmpm);
					strcpy (badFileName, cusf);
					return (FALSE);
				}

				cc = find_rec (cusf, &cusf_rec, NEXT, "r");
			}

			/*------------------
			| Check tmpm also. |
			------------------*/
			cc = find_hash (tmpm, &tmpm_rec, GTEQ, "r", cumr_rec.hhcu_hash);
			while (!cc && tmpm_rec.hhcu_hash == cumr_rec.hhcu_hash)
			{
				if (!strcmp (tmpm_rec.sman_code, exsf_rec.salesman_no))
				{
					abc_fclose (cumr);
					abc_fclose (cusf);
					abc_fclose (tmpm);
					strcpy (badFileName, tmpm);
					return (FALSE);
				}

				cc = find_hash (tmpm, &tmpm_rec, NEXT, "r", cumr_rec.hhcu_hash);
			}
	
			break;

		case F_EXCT:
			if (!strcmp (cumr_rec.cont_type, exct_rec.cont_type))
			{
				abc_fclose (cumr);
				abc_fclose (cusf);
				abc_fclose (tmpm);
				strcpy (badFileName, cumr);
				return (FALSE);
			}

			break;

		case F_EXCL:
			if (!strcmp (cumr_rec.class_type, excl_rec.class_type))
			{
				abc_fclose (cumr);
				abc_fclose (cusf);
				abc_fclose (tmpm);
				strcpy (badFileName, cumr);
				return (FALSE);
			}

			break;

		case F_EXAF:
			if (!strcmp (cumr_rec.area_code, exaf_rec.area_code))
			{
				abc_fclose (cumr);
				abc_fclose (cusf);
				abc_fclose (tmpm);
				strcpy (badFileName, cumr);
				return (FALSE);
			}

			/*------------------
			| Check tmpm also. |
			------------------*/
			cc = find_hash (tmpm, &tmpm_rec, GTEQ, "r", cumr_rec.hhcu_hash);
			while (!cc && tmpm_rec.hhcu_hash == cumr_rec.hhcu_hash)
			{
				if (!strcmp (tmpm_rec.area_code, exaf_rec.area_code))
				{
					abc_fclose (cumr);
					abc_fclose (cusf);
					abc_fclose (tmpm);
					strcpy (badFileName, tmpm);
					return (FALSE);
				}

				cc = find_hash (tmpm, &tmpm_rec, NEXT, "r", cumr_rec.hhcu_hash);
			}

			break;
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	abc_fclose (cumr);
	abc_fclose (cusf);
	abc_fclose (tmpm);

	return (TRUE);
}

int
ChkCuwk (
 int	chkType)
{
	open_rec (cuwk, cuwk_list, CUWK_NO_FIELDS, "cuwk_co_no");

/*
	strcpy (cuwk_rec.co_no, comm_rec.co_no);
	cc = find_rec (cuwk, &cuwk_rec, GTEQ, "r");
	while (!cc && !strcmp (cuwk_rec.co_no, comm_rec.co_no))
	{
		switch (chkType)
		{
		case F_EXSF:
			if (!strcmp (cuwk_rec.sman, exsf_rec.salesman_no))
			{
				abc_fclose (cuwk);
				return (FALSE);
			}

			break;

		case F_EXAF:
			if (!strcmp (cuwk_rec.area, exaf_rec.area_code))
			{
				abc_fclose (cuwk);
				return (FALSE);
			}

			break;
		}
		cc = find_rec (cuwk, &cuwk_rec, NEXT, "r");
	}
*/
	abc_fclose (cuwk);

	return (TRUE);
}

int
ChkInls (
 int	chkType)
{
	open_rec (inls, inls_list, INLS_NO_FIELDS, "inls_id_no");

	strcpy (inls_rec.co_no, comm_rec.co_no);
	strcpy (inls_rec.est_no, "  ");
	cc = find_rec (inls, &inls_rec, GTEQ, "r");
	while (!cc && !strcmp (inls_rec.co_no, comm_rec.co_no))
	{
		switch (chkType)
		{
		case F_EXSF:
			if (!strcmp (inls_rec.sale_code, exsf_rec.salesman_no))
			{
				abc_fclose (inls);
				return (FALSE);
			}

			break;

		case F_EXAF:
			if (!strcmp (inls_rec.area_code, exaf_rec.area_code))
			{
				abc_fclose (inls);
				return (FALSE);
			}

			break;
		}
		cc = find_rec (inls, &inls_rec, NEXT, "r");
	}

	abc_fclose (inls);

	return (TRUE);
}

int
ChkMhdr (
 int	chkType)
{
	open_rec (mhdr, mhdr_list, MHDR_NO_FIELDS, "mhdr_co_no");

	strcpy (mhdr_rec.co_no,    comm_rec.co_no);
	cc = find_rec (mhdr, &mhdr_rec, GTEQ, "r");
	while (!cc && !strcmp (mhdr_rec.co_no, comm_rec.co_no))
	{
		switch (chkType)
		{
		case F_EXCL:
			if (!strcmp (mhdr_rec.cust_type, excl_rec.class_type))
			{
				abc_fclose (mhdr);
				strcpy (badFileName, mhdr);
				return (FALSE);
			}
	
			break;

		case F_EXAF:
			if (!strcmp (mhdr_rec.cust_area, exaf_rec.area_code))
			{
				abc_fclose (mhdr);
				strcpy (badFileName, mhdr);
				return (FALSE);
			}
	
			break;
		}
		cc = find_rec (mhdr, &mhdr_rec, NEXT, "r");
	}

	abc_fclose (mhdr);

	return (TRUE);
}

int
ChkSadf (
 int	chkType)
{
	open_rec (sadf, sadf_list, SADF_NO_FIELDS, "sadf_id_no");

	strcpy (sadf_rec.co_no, comm_rec.co_no);
	strcpy (sadf_rec.br_no, "  ");
	strcpy (sadf_rec.year,  " ");
	sadf_rec.hhbr_hash = 0L;
	sadf_rec.hhcu_hash = 0L;
	strcpy (sadf_rec.sman,  "  ");
	strcpy (sadf_rec.area,  "  ");
	cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
	while (!cc && !strcmp (sadf_rec.co_no, comm_rec.co_no))
	{
		switch (chkType)
		{
		case F_EXSF:
			if (!strcmp (sadf_rec.sman, exsf_rec.salesman_no))
			{
				abc_fclose (sadf);
				return (FALSE);
			}

			break;

		case F_EXAF:
			if (!strcmp (sadf_rec.area, exaf_rec.area_code))
			{
				abc_fclose (sadf);
				return (FALSE);
			}

			break;
		}
		cc = find_rec (sadf, &sadf_rec, NEXT, "r");
	}

	abc_fclose (sadf);

	return (TRUE);
}

int
ChkSale (
 int	chkType)
{
	open_rec (sale, sale_list, SALE_NO_FIELDS, "sale_id_no");

	sprintf (sale_rec.key,      "%2s      ", comm_rec.co_no);
	strcpy (sale_rec.category,  "           ");
	strcpy (sale_rec.sman,      "  ");
	strcpy (sale_rec.area,      "  ");
	strcpy (sale_rec.ctype,     "   ");
	strcpy (sale_rec.dbt_no,    "      ");
	strcpy (sale_rec.year_flag, " ");
	strcpy (sale_rec.period,    " ");
	cc = find_rec (sale, &sale_rec, GTEQ, "r");
	while (!cc && !strncmp (sale_rec.key, comm_rec.co_no, 2))
	{
		switch (chkType)
		{
		case F_EXSF:
			if (!strcmp (sale_rec.sman, exsf_rec.salesman_no))
			{
				abc_fclose (sale);
				return (FALSE);
			}

			break;

		case F_EXCL:
			if (!strcmp (sale_rec.ctype, excl_rec.class_type))
			{
				abc_fclose (sale);
				return (FALSE);
			}

			break;

		case F_EXAF:
			if (!strcmp (sale_rec.area, exaf_rec.area_code))
			{
				abc_fclose (sale);
				return (FALSE);
			}

			break;
		}
		cc = find_rec (sale, &sale_rec, NEXT, "r");
	}

	abc_fclose (sale);

	return (TRUE);
}

int
ChkSohr (
 int	chkType)
{
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");

	strcpy (sohr_rec.co_no,    comm_rec.co_no);
	strcpy (sohr_rec.br_no,    "  ");
	strcpy (sohr_rec.order_no, "          ");
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && !strcmp (sohr_rec.co_no, comm_rec.co_no))
	{
		switch (chkType)
		{
		case F_EXSF:
			if (!strcmp (sohr_rec.sman_code, exsf_rec.salesman_no))
			{
				abc_fclose (sohr);
				strcpy (badFileName, sohr);
				return (FALSE);
			}
	
			break;

		case F_EXAF:
			if (!strcmp (sohr_rec.area_code, exaf_rec.area_code) ||
				!strcmp (sohr_rec.carr_area, exaf_rec.area_code))
			{
				abc_fclose (sohr);
				strcpy (badFileName, sohr);
				return (FALSE);
			}
	
			break;
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}

	abc_fclose (sohr);

	return (TRUE);
}

/*=============================================
| Add or update special instructions record . |
=============================================*/
void
UpdateExsi (void)
{
	strcpy (exsi_rec.co_no,comm_rec.co_no);
	strcpy (exsi_rec.stat_flag,"0");
	if (NewCode)
	{
		cc = abc_add (exsi,&exsi_rec);
		if (cc) 
			file_err (cc, exsi, "DBADD");
	}
	else
	{
		cc = abc_update (exsi,&exsi_rec);
		if (cc) 
			file_err (cc, exsi, "DBUPDATE");
		/*
		 * Update changes audit record.
		 */
		 sprintf (err_str, "%s : %d (%s)", ML ("Instruction"), exsi_rec.inst_code, exsi_rec.inst_text);
		 AuditFileAdd (err_str, &exsi_rec, exsi_list, EXSI_NO_FIELDS);
	}
        abc_unlock (exsi);

}

void
SrchCurt (
 char *	key_val)
{
	_work_open (1,0,40);
	save_rec ("#T","#Receipt type Description");
	strcpy (curt_rec.co_no,comm_rec.co_no);
	sprintf (curt_rec.chq_type,"%-1.1s",key_val);
	cc = find_rec ("curt",&curt_rec,GTEQ,"r");

	while (!cc && !strcmp (curt_rec.co_no,comm_rec.co_no) &&
		      !strncmp (curt_rec.chq_type,key_val,strlen (key_val)))
	{
		cc = save_rec (curt_rec.chq_type,curt_rec.chq_desc);
		if (cc)
			break;

		cc = find_rec ("curt",&curt_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (curt_rec.co_no,comm_rec.co_no);
	sprintf (curt_rec.chq_type,"%-1.1s",temp_str);
	cc = find_rec ("curt",&curt_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "curt", "DBFIND");
}

void
SrchExaf (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#AC","#Area Code description");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf,&exaf_rec,GTEQ,"r");

	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) &&
		      !strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code,exaf_rec.area);
		if (cc)
			break;

		cc = find_rec (exaf,&exaf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}
			
/*========================
| Search on Sales group. |
========================*/
void
SrchSasg (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#SG","#Sales Group code description");
	strcpy (sasg_rec.co_no,comm_rec.co_no);
	sprintf (sasg_rec.sell_grp,"%-2.2s",key_val);
	cc = find_rec (sasg,&sasg_rec,GTEQ,"r");

	while (!cc && !strcmp (sasg_rec.co_no,comm_rec.co_no) &&
		      !strncmp (sasg_rec.sell_grp,key_val,strlen (key_val)))
	{
		cc = save_rec (sasg_rec.sell_grp,sasg_rec.desc);
		if (cc)
			break;

		cc = find_rec (sasg,&sasg_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sasg_rec.co_no,comm_rec.co_no);
	sprintf (sasg_rec.sell_grp,"%-2.2s",temp_str);
	cc = find_rec (sasg,&sasg_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, sasg, "DBFIND");
}

/*=======================
| Search on Sales Type. |
=======================*/
void
SrchSast (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#ST","#Sales Type code description");
	strcpy (sast_rec.co_no,comm_rec.co_no);
	sprintf (sast_rec.code,"%-2.2s",key_val);
	cc = find_rec (sast,&sast_rec,GTEQ,"r");

	while (!cc && !strcmp (sast_rec.co_no,comm_rec.co_no) &&
		      !strncmp (sast_rec.code,key_val,strlen (key_val)))
	{
		cc = save_rec (sast_rec.code,sast_rec.desc);
		if (cc)
			break;

		cc = find_rec (sast,&sast_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sast_rec.co_no,comm_rec.co_no);
	sprintf (sast_rec.code,"%-2.2s",temp_str);
	cc = find_rec (sast,&sast_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, sast, "DBFIND");
}

/*================================
| Search on Sales Position Code. |
================================*/
void
SrchSasp (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#SP","#Sales Position code description");
	strcpy (sasp_rec.co_no,comm_rec.co_no);
	sprintf (sasp_rec.pos_code,"%-2.2s",key_val);
	cc = find_rec (sasp,&sasp_rec,GTEQ,"r");

	while (!cc && !strcmp (sasp_rec.co_no,comm_rec.co_no) &&
		      !strncmp (sasp_rec.pos_code,key_val,strlen (key_val)))
	{
		cc = save_rec (sasp_rec.pos_code,sasp_rec.pos_desc);
		if (cc)
			break;

		cc = find_rec (sasp,&sasp_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sasp_rec.co_no,comm_rec.co_no);
	sprintf (sasp_rec.pos_code,"%-2.2s",temp_str);
	cc = find_rec (sasp,&sasp_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, sasp, "DBFIND");
}

/*=========================
| Search on Carrier Code. |
=========================*/
void
SrchCfhr (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#Code","#Carrier Code Description");
	strcpy (cfhr_rec.co_no,comm_rec.co_no);
	sprintf (cfhr_rec.carr_code,"%-4.4s",key_val);
	cc = find_rec (cfhr,&cfhr_rec,GTEQ,"r");

	while (!cc && !strcmp (cfhr_rec.co_no,comm_rec.co_no) &&
		      !strncmp (cfhr_rec.carr_code,key_val,strlen (key_val)))
	{
		cc = save_rec (cfhr_rec.carr_code,cfhr_rec.carr_desc);
		if (cc)
			break;

		cc = find_rec (cfhr,&cfhr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cfhr_rec.co_no,comm_rec.co_no);
	sprintf (cfhr_rec.carr_code,"%-4.4s",temp_str);
	cc = find_rec (cfhr,&cfhr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, cfhr, "DBFIND");
}

void
SrchExcl (
 char *	key_val)
{
	_work_open (3,0,40);
	save_rec ("#CL ","#Class code description");
	strcpy (excl_rec.co_no,comm_rec.co_no);
	sprintf (excl_rec.class_type,"%-3.3s",key_val);
	cc = find_rec (excl,&excl_rec,GTEQ,"r");

	while (!cc && !strcmp (excl_rec.co_no,comm_rec.co_no) &&
				  !strncmp (excl_rec.class_type,key_val,strlen (key_val)))
	{
		cc = save_rec (excl_rec.class_type,excl_rec.class_desc);
		if (cc)
			break;

		cc = find_rec (excl,&excl_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excl_rec.co_no,comm_rec.co_no);
	sprintf (excl_rec.class_type,"%-3.3s",temp_str);
	cc = find_rec (excl,&excl_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, excl, "DBFIND");
}

void
SrchExct (
 char *	key_val)
{
	_work_open (3,0,40);
	save_rec ("#CT ", "#Contract Type Description");

	strcpy (exct_rec.co_no, comm_rec.co_no);
	sprintf (exct_rec.cont_type, "%-3.3s", key_val);
	cc = find_rec (exct, &exct_rec, GTEQ, "r");

	while (!cc && 
		   !strcmp (exct_rec.co_no,comm_rec.co_no) &&
		   !strncmp (exct_rec.cont_type,key_val,strlen (key_val)))
	{
		cc = save_rec (exct_rec.cont_type,exct_rec.cont_desc);
		if (cc)
			break;

		cc = find_rec (exct, &exct_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exct_rec.co_no, comm_rec.co_no);
	sprintf (exct_rec.cont_type, "%-3.3s", temp_str);
	cc = find_rec (exct, &exct_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exct, "DBFIND");
}

void
SrchExsf (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#SM","#Salespersons Name");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec (exsf,&exsf_rec,GTEQ,"r");

	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) &&
				  !strncmp (exsf_rec.salesman_no,key_val,strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;

		cc = find_rec (exsf,&exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

/*=========================
| Search user for helper. |
=========================*/
void
SrchExsf2 (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#SM","#Salespersons Name");
	strcpy (exsf2_rec.co_no,comm_rec.co_no);
	sprintf (exsf2_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec (exsf2,&exsf2_rec,GTEQ,"r");

	while (!cc && !strcmp (exsf2_rec.co_no,comm_rec.co_no) &&
				  !strncmp (exsf2_rec.salesman_no,key_val,strlen (key_val)))
	{
		cc = save_rec (exsf2_rec.salesman_no,exsf2_rec.salesman);
		if (cc)
			break;

		cc = find_rec (exsf2,&exsf2_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsf2_rec.co_no,comm_rec.co_no);
	sprintf (exsf2_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec (exsf2,&exsf2_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exsf2, "DBFIND");
}

void
SrchExsi (
 char *	key_val)
{
	char	wk_code [4];

	_work_open (3,0,60);
	save_rec ("#No ","#Special instruction description");
	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = atoi (key_val);
	cc = find_rec (exsi,&exsi_rec,GTEQ,"r");

	while (!cc && !strcmp (exsi_rec.co_no,comm_rec.co_no))
	{
		sprintf (wk_code, "%3d", exsi_rec.inst_code); 
		cc = save_rec (wk_code,exsi_rec.inst_text);
		if (cc)
			break;

		cc = find_rec (exsi,&exsi_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code = atoi (temp_str);
	cc = find_rec (exsi,&exsi_rec,GTEQ,"r");
	if (cc)
		file_err (cc, exsi, "DBFIND");
}

/*======================================
| Search for External Market Supplier. |
======================================*/
void
SrchExms (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#MS","#Market Supply status description");
	strcpy (exms_rec.co_no,comm_rec.co_no);
	sprintf (exms_rec.stat_code,"%-2.2s",key_val);
	cc = find_rec (exms,&exms_rec,GTEQ,"r");

	while (!cc && !strcmp (exms_rec.co_no,comm_rec.co_no) &&
		      	  !strncmp (exms_rec.stat_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exms_rec.stat_code,exms_rec.desc);
		if (cc)
			break;

		cc = find_rec (exms,&exms_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exms_rec.co_no,comm_rec.co_no);
	sprintf (exms_rec.stat_code,"%-2.2s",temp_str);
	cc = find_rec (exms,&exms_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exms, "DBFIND");
}

/*======================================
| Search for Merchandiser Agency File. |
======================================*/
void
SrchExma (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#MA","#Merchandiser Agency Description");
	strcpy (exma_rec.co_no,comm_rec.co_no);
	sprintf (exma_rec.code,"%-2.2s",key_val);
	cc = find_rec (exma,&exma_rec,GTEQ,"r");

	while (!cc && !strcmp (exma_rec.co_no,comm_rec.co_no) &&
		      	  !strncmp (exma_rec.code,key_val,strlen (key_val)))
	{
		cc = save_rec (exma_rec.code,exma_rec.desc);
		if (cc)
			break;

		cc = find_rec (exma,&exma_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exma_rec.co_no,comm_rec.co_no);
	sprintf (exma_rec.code,"%-2.2s",temp_str);
	cc = find_rec (exma,&exma_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exma, "DBFIND");
}

/*=============================
| Search for Sales Type File. |
=============================*/
void
SrchSaca (
 char *	key_val)
{
	_work_open (2,0,40);
	save_rec ("#SA","#Sales Call Activity Description");
	strcpy (saca_rec.co_no,comm_rec.co_no);
	sprintf (saca_rec.code,"%-2.2s",key_val);
	cc = find_rec (saca,&saca_rec,GTEQ,"r");

	while (!cc && !strcmp (saca_rec.co_no,comm_rec.co_no) &&
		      	  !strncmp (saca_rec.code,key_val,strlen (key_val)))
	{
		cc = save_rec (saca_rec.code,saca_rec.desc);
		if (cc)
			break;

		cc = find_rec (saca,&saca_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (saca_rec.co_no,comm_rec.co_no);
	sprintf (saca_rec.code,"%-2.2s",temp_str);
	cc = find_rec (saca,&saca_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, saca, "DBFIND");
}

void
UnlockAll (void)
{
	if (AREA)
		abc_unlock (exaf);

	if (CTYPE)
		abc_unlock (excl);

	if (CNTRCT)
		abc_unlock (exct);

	if (SMAN)
		abc_unlock (exsf);

	if (INSTR)
		abc_unlock (exsi);

	if (MKT_SS	)
		abc_unlock (exms);

	if (MER_AGENCY)
		abc_unlock (exma);

	if (SALE_TYPE)
		abc_unlock (sast);

	if (SALE_GRP)
		abc_unlock (sasg);

	if (SALE_POS)
		abc_unlock (sasp);

	if (SALE_CA)
		abc_unlock (saca);
}

void
ActiveExsf (void)
{
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", " ");
	cc = find_rec (exsf, &exsf_rec, GTEQ, "u");
	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.co_no))
	{
		if (AREA)
		{
			if (!strcmp (exsf_rec.area_code, exaf_rec.area_code))
			{
				if (!strcmp (exsf_rec.update,"A"))
					strcpy (exsf_rec.update,"A");
				else
					strcpy (exsf_rec.update,"U");
				cc = abc_update (exsf, &exsf_rec);
				if (cc) 
					file_err (cc, exsf, "DBUPDATE");
			}
			else
				abc_unlock (exsf);
		}
		cc = find_rec (exsf, &exsf_rec, NEXT, "u");
	}
	return;
}

int
heading (
 int	scn)
{
	if (restart) 
	{
		UnlockAll ();
		return (EXIT_SUCCESS);
	}
	if (scn != cur_screen)
		scn_set (scn);

	snorm ();
	clear ();
	if (AREA)
	{
		rv_pr (ML (mlMenuMess027), 25,0,1);
		print_at (0,66, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);

		box (0,3,80,4);
		line_at (6,1,79);
	}

	if (CTYPE)
	{
		rv_pr (ML (mlMenuMess029), 20,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);

		line_at (1,0,80);

		box (0,3,80,2);
	}

	if (CNTRCT)
	{
		rv_pr (ML (mlMenuMess031), 20,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);
		box (0,3,80,2);
	}

	if (SMAN)
	{
		rv_pr (ML (mlMenuMess033), 25,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);

		box (0,1,80,18);

		line_at (4,1,79);
		line_at (10,1,79);
		line_at (16,1,79);
	}

	if (INSTR)
	{
		rv_pr (ML (mlMenuMess035), 24,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);

		box (0,3,80,2);
	}

	if (MKT_SS)
	{
		rv_pr (ML (" Market Supply Status File Maintenance "), 20,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);

		box (0,3,80,2);
	}
	if (MER_AGENCY)
	{
		rv_pr (ML (" Merchandiser Agency Maintenance "), 20,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);

		box (0,3,80,2);
	}
	if (SALE_TYPE)
	{
		rv_pr (ML (" Sales Type file Maintenance "), 20,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);

		box (0,3,80,2);
	}
	if (SALE_GRP)
	{
		rv_pr (ML (" Sales Group File Maintenance "), 20,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);

		box (0,3,80,2);
	}
	if (SALE_POS)
	{
		rv_pr (ML (" Sales Position File Maintenance "), 20,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);
		box (0,3,80,2);
	}
	if (SALE_CA)
	{
		rv_pr (ML (" Sales Call Activity code Maintenance "), 20,0,1);
		print_at (0,64, ML (mlMenuMess217),local_rec.previousCode);
		line_at (1,0,80);
		box (0,3,80,2);
	}

	if (REC_TYPE)
	{
		rv_pr (ML (mlMenuMess037) , 25,0,1);
		line_at (1,0,80);
		box (0,3,80,3);
	}
	line_at (22,0,80);
	print_at (23,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);

	line_cnt = 0;
		scn_write (scn);

	return (EXIT_SUCCESS);
}


