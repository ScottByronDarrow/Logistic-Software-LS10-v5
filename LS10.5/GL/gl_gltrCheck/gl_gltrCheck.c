/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_gltrCheck.c,v 5.0 2002/05/08 01:29:41 scott Exp $
|  Program Name  : (gltr_check.c)                          
|  Program Desc  : (Global integrity check of the gltr file)
|                 (against the posting level glpd) 
|---------------------------------------------------------------------|
|  Author        : Trevor van Bremen       | Date Written : 14/01/91  |
|---------------------------------------------------------------------|
| $Log: gl_gltrCheck.c,v $
| Revision 5.0  2002/05/08 01:29:41  scott
| CVS administration
|
| Revision 1.4  2001/08/20 23:12:49  scott
| Updated for development related to bullet proofing
|
| Revision 1.3  2001/08/09 09:13:47  scott
| Updated to add FinishProgram () function
|
| Revision 1.2  2001/08/06 23:27:22  scott
| RELEASE 5.0
|
| Revision 1.1  2001/07/25 02:46:15  scott
| New LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_gltrCheck.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_gltrCheck/gl_gltrCheck.c,v 5.0 2002/05/08 01:29:41 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/

#include <pslscr.h>
#include <GlUtils.h>
#include <twodec.h>
#include <ml_gl_mess.h>

#include	"schema"

struct	commRecord	comm_rec;
struct	comrRecord	comr_rec;

#define		CF(x)	comma_fmt (DOLLARS (x), "NNN,NNN,NNN,NNN.NN")

/*==================================
|   Constants, defines and stuff    |
 ==================================*/

#define	ACT_LIST	0
#define	CHK_LIST	1

/*  QUERY
    these should be declared as const char*
    to minimize potential problems.
*/
char	*glmr2 = "glmr2",
        *data  = "data";

/*====================
|   Local variables   |
 ====================*/

char    acc_bit [16][17];
char	dispStr [256];

int     PV_max_len;
int     PV_max_lvl;
int     lvl_len [16];
int     tot_len [17];

double  GLPD_LC_BALANCE,
        GLPD_FX_BALANCE,
        GLTR_LC_BALANCE,
        GLTR_FX_BALANCE;

GLMR_STRUCT glmr2Rec;

int	    GL_YEAR,
        GL_PERIOD;
long    TR_DATE;


/*===============================
|   Local function prototypes   |
===============================*/

void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	InitDisplay		(void);
void 	Process 		(void);
void 	CheckBals 		(void);
void 	PrintError 		(double, double);
int  	MoneyZero 		(double);
int	 	UpdateGlpd;

/*=======================
| Main Processing Loop. |
=======================*/
int
main (
 int    argc, 
 char  *argv [])
{
	UpdateGlpd = FALSE;

	OpenDB ();

	GL_YEAR		=	fisc_year (comm_rec.gl_date);

	init_scr ();		/*  sets terminal from termcap	  */
	set_tty ();

	clear ();

	InitDisplay ();
	Process ();

	shutdown_prog ();

    return (EXIT_SUCCESS);
}

/*
 *   Standard program exit sequence     
 */
void
shutdown_prog (void)
{
	Dsp_srch ();
	Dsp_close ();
	CloseDB (); 
	FinishProgram ();
}
/*=======================
| Open data base files	|
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);			/* Get base currency */
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_alias (glmr2, glmr);
	OpenGltr (); abc_selfield (gltr, "gltr_id_no2");
	OpenGlmr ();
	OpenGlpd ();
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (void)
{
	abc_fclose (comr);
	GL_Close ();
	abc_fclose (glmr2);
	abc_dbclose (data);
}

/*
 *   Initialise screen output.
 */
void
InitDisplay (void)
{
	clear ();
	swide ();
	rv_pr ("General ledger period versus transaction integrity check", 25, 0,1);
	Dsp_prn_open (0, 1, 17, dispStr, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0);
	
	Dsp_saverec (" CO |     ACCOUNT      | YEAR |PERIOD|   PERIOD VALUE    |   TRANSACTION     |     DIFFERENCE    | STATUS ");

	Dsp_saverec (" NO |     NUMBER       |      |NUMBER|      LOCAL        |   VALUE LOCAL     |                   |        ");
	Dsp_saverec (" [FN03]  [FN05]  [FN14]  [FN15]  [FN16] ");
}

/*===============================
|   Process the glmr records    |
===============================*/
void
Process (void)
{
	strcpy (glmrRec.co_no, "  ");
	strcpy (glmrRec.acc_no, "                ");

    cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (!cc)
	{
		if (glmrRec.glmr_class [2][0] != 'P')
		{
			cc = find_rec (glmr, &glmrRec, NEXT, "r");
			continue;
		}
		CheckBals ();
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

/*===============================================
|   Make sure the balance of this account is    |
| the same as the sum of the children accounts. |
===============================================*/
void
CheckBals (void)
{
	double	GLPD_LC_BALANCE	=	0.00;
	double	GLTR_LC_BALANCE	=	0.00;
	int		glYear			=	0;
	int 	fmonth			= 0,
			fyear			= 0,
			i,
    		continueLoop = 0;

	glYear	=	fisc_year (comm_rec.gl_date);
	for (i = 1; i < 13; i++)
	{
		GLPD_LC_BALANCE		=	0.00;
		GLTR_LC_BALANCE		=	0.00;

		glpdRec.hhmr_hash 	= glmrRec.hhmr_hash;
		glpdRec.budg_no 	= 0;
		glpdRec.year 		= glYear;
		glpdRec.prd_no 		= i;

    	cc = find_rec (glpd, &glpdRec, GTEQ, "r");
		while (!cc && 
           	glpdRec.budg_no == 0 && 
           	glpdRec.hhmr_hash == glmrRec.hhmr_hash &&
           	glpdRec.year 	== glYear &&
           	glpdRec.prd_no 	== i)
    	{
			GLPD_LC_BALANCE		+=	no_dec (glpdRec.balance);

		
			gltrRec.hhmr_hash	=	glmrRec.hhmr_hash;
			gltrRec.tran_date	=	FinDMYToDate 
									(
										comr_rec.fiscal,
										1,
										(glpdRec.prd_no == 99) ? 1 : glpdRec.prd_no,
										glpdRec.year
									);
			cc = find_rec (gltr, &gltrRec, GTEQ, "r");
			if (cc)
				continue;

			DateToFinDMY 
			(
				gltrRec.tran_date, 
				comr_rec.fiscal, 
				NULL, 
				&fmonth, 
				&fyear
			);
	
			continueLoop = TRUE;
			while (continueLoop == TRUE)
			{
				if (!cc && gltrRec.hhmr_hash == glmrRec.hhmr_hash &&	
					fmonth <= glpdRec.prd_no && fyear == glpdRec.year)
				{
					continueLoop = TRUE;
				}
					else
				{
					continueLoop = FALSE;
					break;
				}
				GLTR_LC_BALANCE		+=	no_dec (gltrRec.amount);
				cc = find_rec (gltr, &gltrRec, NEXT,"r");
				DateToFinDMY 
				(
					gltrRec.tran_date, 
					comr_rec.fiscal, 
					NULL, 
					&fmonth, 
					&fyear
				);
			}
			PrintError (GLPD_LC_BALANCE, GLTR_LC_BALANCE);
			cc = find_rec (glpd, &glpdRec, NEXT, "r");
		}   /* end of while loop */
	}
}

void
PrintError (
	double	periodValue,
	double	tranValue)
{
	double	diffValue	=	0.00;
	char	tranCharStr	[21],
			diffCharStr	[21],
			prdCharStr	[21];

	periodValue	= 	no_dec (periodValue);
	tranValue	= 	no_dec (tranValue);
	diffValue	=	no_dec (periodValue - tranValue);

	sprintf (tranCharStr, "%18.18s", CF (tranValue));
	sprintf (diffCharStr, "%18.18s", CF (diffValue));
	sprintf (prdCharStr,  "%18.18s", CF (periodValue));
	
	sprintf (dispStr, 
		" %-2.2s ^E %-16.16s ^E %4d ^E  %02d  ^E%18.18s ^E%18.18s ^E%18.18s ^E %s ",
		glmrRec.co_no,
		glmrRec.acc_no,
		glpdRec.year,
		glpdRec.prd_no,
		prdCharStr,
		tranCharStr,
		diffCharStr,
		(!MoneyZero (diffValue)) ? "ERROR" : "  OK"
	);
	Dsp_saverec (dispStr);
	if (!UpdateGlpd)
		return;
}


/*
 *	Minor support functions
 */
int
MoneyZero (
 double	m)
{
	return (fabs (m) < 0.0001);
}

/* [ end of file ] */
