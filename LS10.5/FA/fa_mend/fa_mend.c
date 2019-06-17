/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: fa_mend.c,v 5.2 2001/08/09 09:13:12 scott Exp $
|  Program Name  : (fa_mend.c)
|  Program Desc  : (Calculate Disposal & Delete Assets Disposed)
|                : (Calculate Depreciation)
|---------------------------------------------------------------------|
|  Date Written  : (15/05/1997)    | Author       : Scott B Darrow.   |
|---------------------------------------------------------------------|
| $Log: fa_mend.c,v $
| Revision 5.2  2001/08/09 09:13:12  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:25:55  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fa_mend.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FA/fa_mend/fa_mend.c,v 5.2 2001/08/09 09:13:12 scott Exp $";

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct famrRecord	famr_rec;
struct faglRecord	fagl_rec;

	int 	wk_mon;
	char	loc_curr [4];
	char	systemDate [11];

	double	DepThisPeriod	= 0.00;
	double	BookValue		= 0.00;
	double	AccDep			= 0.00;

	int		lpno;

void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Process 		(void);
void 	CalcClosing 	(void);
void 	AssetDispense 	(void);
void 	AddGL 			(char *, char *, double);

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
		printf ("Usage : %s [LPNO]\n", argv [0]);
		return (EXIT_FAILURE);
	}
	lpno	=	atoi (argv [1]);

	OpenDB ();


	strcpy (systemDate, DateToString (TodaysDate ()));

	DateToDMY (comm_rec.gl_date, NULL, &wk_mon, NULL);

	dsp_screen ("Calculating Depreciation on Fixed Assets",comm_rec.co_no, comm_rec.co_name);

	/*====================================
	| Process fixed Assets for Year End. |
	====================================*/
	Process ();

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (loc_curr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	OpenGlmr ();
	open_rec (famr, famr_list, FAMR_NO_FIELDS, "famr_id_no");
	open_rec (fagl, fagl_list, FAGL_NO_FIELDS, "fagl_id_no");
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (famr);
	abc_fclose (fagl);
	GL_CloseBatch (lpno);
	GL_Close ();
	abc_dbclose ("data");
}

/*====================================
| Process fixed Assets for Year End. |
====================================*/
void
Process (void)
{
	strcpy (famr_rec.co_no, comm_rec.co_no);
	strcpy (famr_rec.ass_group,	"     ");
	strcpy (famr_rec.ass_no, 	"     ");
	cc = find_rec (famr, &famr_rec, GTEQ, "u");
	while (!cc && !strcmp (famr_rec.co_no, comm_rec.co_no))
	{
		if (famr_rec.stat_flag [0] == 'D')
		{
			abc_unlock (famr);
			cc = find_rec (famr, &famr_rec, NEXT, "u");
			continue;
		}
		dsp_process ("Asset No", famr_rec.ass_no);
	
		CalcClosing ();

		/*-------------------------------------------------------
		| If Disposal value is non zero then dispose of assett. |
		-------------------------------------------------------*/
		if (famr_rec.disp_price != 0.00)
		{
			AssetDispense ();
			famr_rec.gl_updated = StringToDate (systemDate);

			cc = abc_update (famr,&famr_rec);
			if (cc) 
				file_err (cc, famr, "DBUPDATE");

			cc = find_rec (famr, &famr_rec, NEXT, "u");
			continue;
		}

		if (DepThisPeriod != 0.00)
		{
			AddGL (famr_rec.gl_dbt_acc, "1", DepThisPeriod);
			AddGL (famr_rec.gl_crd_acc, "2", DepThisPeriod);

			famr_rec.gl_updated = StringToDate (systemDate);

			cc = abc_update (famr,&famr_rec);
			if (cc) 
				file_err (cc, famr, "DBUPDATE");
		}
		else
			abc_unlock (famr);

		cc = find_rec (famr, &famr_rec, NEXT, "u");
	}
}

void
CalcClosing (void)
{
	BookValue 		= famr_rec.int_open_val;
	AccDep			= 0.00;
	DepThisPeriod	= 0.00;

	fagl_rec.famr_hash 	=	famr_rec.famr_hash;
	cc = find_rec (fagl, &fagl_rec, GTEQ, "u");
	while (!cc && fagl_rec.famr_hash 	==	famr_rec.famr_hash)
	{
		/*-------------------------------------
		| Ensure is not a future transaction. |
		-------------------------------------*/
		if (fagl_rec.tran_date <= MonthEnd (comm_rec.gl_date))
		{
			/*-----------------------------------------------
			| Check if transaction is within current month. |
			-----------------------------------------------*/
			if (MonthStart (fagl_rec.tran_date) == MonthStart (comm_rec.gl_date))
			{
				/*-----------------------------------------------
				| Check if transaction has already been posted. |
				-----------------------------------------------*/
				if (fagl_rec.posted [0] == 'Y')
				{
					abc_unlock (fagl);
					cc = find_rec (fagl, &fagl_rec, NEXT, "u");
					continue;
				}
				DepThisPeriod = fagl_rec.int_amt;

				strcpy (fagl_rec.posted, "Y");
				cc = abc_update (fagl, &fagl_rec);
				if (cc)
					file_err (cc, fagl, "DBUPDATE");
			}
			BookValue	-= fagl_rec.int_amt;
			AccDep		+= fagl_rec.int_amt;
		}
		cc = find_rec (fagl, &fagl_rec, NEXT, "u");
	}
	if (BookValue < 0.00)
		BookValue = 0.00;
}

/*=======================
| Dispensing with Asset	|
=======================*/
void
AssetDispense (void)
{
	double	PostValue1	=	0.00;
	double	PostValue2	=	0.00;
	double	PostValue3	=	0.00;

	PostValue1 = BookValue - famr_rec.disp_price;
	PostValue2 = famr_rec.cost_price - BookValue;
	PostValue3 = famr_rec.cost_price - famr_rec.disp_price;

	if (PostValue1 < 0.00)
		AddGL (famr_rec.gl_dbt_acc, "2", PostValue1 * -1);
	else
		AddGL (famr_rec.gl_dbt_acc, "1", PostValue1);

	if (PostValue2 < 0.00)
		AddGL (famr_rec.gl_crd_acc, "2", PostValue2 * -1);
	else
		AddGL (famr_rec.gl_crd_acc, "1", PostValue2);
	
	if (PostValue3 < 0.00)
		AddGL (famr_rec.gl_ass_acc, "1", PostValue3 * -1);
	else
		AddGL (famr_rec.gl_ass_acc, "2", PostValue3);

	/*-----------------------------------
	| Flag famr record as been deleted. |
	-----------------------------------*/
	strcpy (famr_rec.stat_flag, "D");
	cc = abc_update (famr, &famr_rec);
	if (cc) 
		file_err (cc, famr, "DBUPDATE");
}

void
AddGL (
	char	*Account,
	char	*DC_Flag,
	double	DepAmount)
{
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.acc_no, Account);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no , 	comm_rec.co_no);
	strcpy (glwkRec.acc_no, 	glmrRec.acc_no);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	strcpy (glwkRec.tran_type, " 1");
	sprintf (glwkRec.sys_ref , "%010ld", (long) comm_rec.term);
	glwkRec.tran_date = comm_rec.gl_date;
	sprintf (glwkRec.period_no, "%02d", wk_mon);
	glwkRec.post_date = StringToDate (systemDate);
 
	sprintf (glwkRec.narrative, "F/A NO : %s-%s",
							famr_rec.ass_group,famr_rec.ass_no);
	sprintf (glwkRec.user_ref, "F/A Period End.");
	sprintf (glwkRec.alt_desc1, famr_rec.ass_desc1);
	sprintf (glwkRec.alt_desc2, famr_rec.ass_desc2);
	sprintf (glwkRec.alt_desc3, famr_rec.ass_desc3);
	sprintf (glwkRec.batch_no, " ");
	glwkRec.amount = DepAmount;
	strcpy (glwkRec.jnl_type, DC_Flag);
	strcpy (glwkRec.stat_flag, "2");
	glwkRec.loc_amount = glwkRec.amount;
	glwkRec.exch_rate = 1.00;
	strcpy (glwkRec.currency, loc_curr);

	GL_AddBatch ();
}
