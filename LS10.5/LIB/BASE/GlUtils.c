/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: GlUtils.c,v 5.5 2002/06/25 01:37:57 scott Exp $
|  Program Name  : (GlUtils.c)  
|  Program Desc  : (General Ledger Library Functions)
|---------------------------------------------------------------------|
| $Log: GlUtils.c,v $
| Revision 5.5  2002/06/25 01:37:57  scott
| Updated to remove debug warning.
|
| Revision 5.4  2001/08/26 22:35:05  scott
| Updated for checkes on possible 0 hash on gl.
|
| Revision 5.3  2001/08/20 23:05:53  scott
| Updated from testing.
|
| Revision 5.2  2001/08/06 22:40:52  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 00:43:27  scott
| New library for 10.5
|
=====================================================================*/
#include	<std_decs.h>
#include	<GlUtils.h>
#include	<math.h>

#define	RECOVER	 (PV_rec_flag == TRUE)
#define	MAX_DIFF	3

/*
 * 	Variables that will be external.
 */
int		GV_link_cnt 	= 0;
long	GV_post_time 	= 0L;

/*
 * 	External Functions / variables required.
 */
extern	int		GV_cur_level;
extern	long	TodaysDate	(void);
extern	int		cc;
extern	int		_wide;
extern	char	temp_str [];
extern	int	_SpecialRedraw;

	static	const	char 	*glln2	=	"_glln2_costing";

	GLLN_STRUCT	glln2Rec;

	static	int		glbd_openDone	=	FALSE,
					glbh_openDone	=	FALSE,
					glbl_openDone	=	FALSE,
					glca_openDone	=	FALSE,
					glct_openDone	=	FALSE,
					glih_openDone	=	FALSE,
					glid_openDone	=	FALSE,
					gljc_openDone	=	FALSE,
					gllg_openDone	=	FALSE,
					glln_openDone	=	FALSE,
					glln2_openDone	=	FALSE,
					glmr_openDone	=	FALSE,
					glna_openDone	=	FALSE,
					glpd_openDone	=	FALSE,
					gltc_openDone	=	FALSE,
					gltr_openDone	=	FALSE,
					glus_openDone	=	FALSE,
					glwk_openDone	=	FALSE,
					pocr_openDone	=	FALSE,
					badGLI			=	FALSE;


/* 
 *	Internal (static) prototypes 
 */
static	void 	AddBatchHeader 	(void),
				AddBatchDetail 	(void),
				GL_initClass 	(char *),
				CheckBatch 		(long),
				GLI_Error 		(char *,char *,char *,char *,char *,char *);

static	int		GL_ReadAccountNo 	(char *, GLMR_STRUCT *),
				GL_UpdateLinks 		(long, int, int, char *, double, double),
				GL_UpdatePeriod 	(long, int, int, char *, double, double),
				GL_ClassOK 			(GLMR_STRUCT *),
				ReadGlid 			(long, char *, char	*, char	*),
				ReadGlih 			(char *,char *,char *,char *);

static	int		PV_rec_flag 	= FALSE,
				PV_budget 		= 0,
				glblLineNo 		= 0,
				maxLineValue	= -1,
				GL_SubCat 		= 0,
				GLI_messageNo	= 0,
				GL_GLI_Error	= 0,
				GL_WideMode		= 0;

static	char	glWorkStr			[256],
				PV_co_no 			[3],
				PV_br_no 			[3],
				PV_acc_curr 		[4],
				GL_InterfaceCode 	[11];

static	long	PV_rec_time 	= 0L,
				batchNo 		= -1;

static	Money	checkBalance	= 0.00,
				totalDebit		= 0.00,
				totalCredit		= 0.00,
				lastAmount		= 0.00;

int				InterfaceSelect		(char *);

/*
 * Function GL_AddBatch ()
 * Should be called by other programs to fill batch header and detail
 * information to glbh and glbl.
 * For each instance of the program, there should exist only one
 * batch header.  After create that, we keep on adding batch detail
 * to that batch header.
 */
char *GL_AddBatch (void)
{
	/*
	 * batchNo = -1 means there is no batch created for this instance.
	 * Thus, add a new batch header for it.
	 */
	if (batchNo == -1)
		AddBatchHeader ();

	AddBatchDetail ();
	return (glbhRec.batch_no);
}

/*
 * Open Datebase Files.
 */
void
GL_OpenBatch (
	char	*companyNo,
	char	*branchNo)
{
	sprintf (PV_co_no, "%-2.2s", companyNo);
	sprintf (PV_br_no, "%-2.2s", branchNo);

	glblLineNo 		= 0;
	batchNo 		= -1;
	maxLineValue	= -1;
	totalCredit		= 0.00;
	totalDebit		= 0.00;
}

/*
 * Close Datebase Files.
 */
void
GL_CloseBatch (
	int		printerNo)
{
	if (batchNo != -1)
	{
		CheckBatch (glbhRec.hhbh_hash);

		if (printerNo)
		{
			sprintf 
		 	(
				glWorkStr, 
				"gl_batch_list %d \"%-2.2s\" \"%-10.10s\"", 
				printerNo, 
				glbhRec.jnl_type, 
				glbhRec.batch_no
			);
			sys_exec (glWorkStr);
		}
	}
}

/*
 * Add information to glbh
 */
static void 
AddBatchHeader (void)
{
	OpenGlbh ();

	batchNo	=	GL_NextBatchNo (glwkRec.co_no, glwkRec.tran_type);

	strcpy (glbhRec.co_no, glwkRec.co_no);
	if (strlen (clip (glwkRec.est_no)))
		strcpy (glbhRec.br_no, glwkRec.est_no);	
	else
		sprintf (glbhRec.br_no, PV_br_no);

	strcpy (glbhRec.jnl_type, glwkRec.tran_type);
    sprintf (glbhRec.batch_no, "%010ld", batchNo);
	sprintf (glbhRec.user, "%-8.8s", getenv ("LOGNAME")); 
	glbhRec.glbh_date = TodaysDate ();
	strcpy (glbhRec.glbh_time, TimeHHMM ());
	strcpy (glbhRec.stat_flag, "N"); 
	glbhRec.mth = atoi (glwkRec.period_no);
	glbhRec.other_module = TRUE;

	cc = abc_add (glbh,&glbhRec);
	if (cc)
		file_err (cc, glbh, "DBADD");

	cc	=	FindGlbh 
			(
				glbhRec.co_no, 
				glbhRec.br_no, 
				glbhRec.jnl_type, 
				glbhRec.batch_no, 
				"r"
			);
	if (cc)
		file_err (cc, glbh, "DBFIND");
}

/*
 * Add information to glbl
 */
static void 
AddBatchDetail (void)
{
	OpenGlbl ();

	glblRec.hhbh_hash 	= glbhRec.hhbh_hash;
	glblRec.line_no   	= glblLineNo++;
	glblRec.ci_amt 		= glwkRec.ci_amt;
	glblRec.o1_amt 		= glwkRec.o1_amt;
	glblRec.o2_amt 		= glwkRec.o2_amt;
	glblRec.o3_amt 		= glwkRec.o3_amt;
	glblRec.o4_amt 		= glwkRec.o4_amt;
	glblRec.hhgl_hash 	= glwkRec.hhgl_hash;
	glblRec.tran_date 	= glwkRec.tran_date;
	glblRec.fx_amt 		= glwkRec.amount;
	glblRec.local_amt 	= glwkRec.loc_amount;
	glblRec.exch_rate	= glwkRec.exch_rate;
	strcpy (glblRec.acc_no, 	glwkRec.acc_no);
	strcpy (glblRec.acronym, 	glwkRec.acronym);
	strcpy (glblRec.name, 		glwkRec.name);
	strcpy (glblRec.chq_inv_no, glwkRec.chq_inv_no);
	strcpy (glblRec.tran_type, 	glwkRec.tran_type);
	strcpy (glblRec.sys_ref,    glwkRec.sys_ref);
	strcpy (glblRec.period_no,  glwkRec.period_no);
	strcpy (glblRec.narrative,  glwkRec.narrative);
	strcpy (glblRec.alt_desc1,  glwkRec.alt_desc1);
	strcpy (glblRec.alt_desc2,  glwkRec.alt_desc2);
	strcpy (glblRec.alt_desc3,  glwkRec.alt_desc3);
	strcpy (glblRec.user_ref,   glwkRec.user_ref);
	strcpy (glblRec.dc_flag,    glwkRec.jnl_type);
	strcpy (glblRec.currency,   glwkRec.currency);
	strcpy (glblRec.stat_flag, "N");

	if (glblRec.local_amt > lastAmount)
	{
		maxLineValue 	= glblRec.line_no;
		lastAmount		= glblRec.local_amt;
	}
	if ((atoi (glblRec.dc_flag) % 2) == 0)
		checkBalance	-= no_dec (glblRec.local_amt);
	else
		checkBalance	+= no_dec (glblRec.local_amt);

	cc = abc_add (glbl,&glblRec);
	if (cc)
		file_err (cc, glbl, "DBADD");
}

static	void
CheckBatch (
	long	hhblHash)
{
	if (checkBalance == 0.0)
		return;

	/* 
	 * To big to be a exchange conversion.
	 */
	if (fabs (checkBalance) > MAX_DIFF)
		return;

	glblRec.hhbh_hash 	= glbhRec.hhbh_hash;
	glblRec.line_no   	= maxLineValue;
	cc = find_rec (glbl, &glblRec, COMPARISON, "u");
	if (!cc)
	{
		glblRec.local_amt	-= checkBalance;
		cc = abc_update (glbl, &glblRec);
		if (cc)
			file_err (cc, glbl, "DBUPDATE");
	}
	else
		abc_unlock (glbl);
}

/*
 * Open glbd file
 */
void
OpenGlbd (void)
{
	if (glbd_openDone == FALSE)
	{
		open_rec (glbd, glbd_list, GLBD_NO_FIELDS, "glbd_id_no");
		glbd_openDone = TRUE;
	}
}
/*
 * Open glbh file
 */
void
OpenGlbh (void)
{
	if (glbh_openDone == FALSE)
	{
		open_rec (glbh, glbh_list, GLBH_NO_FIELDS, "glbh_id_no");
		glbh_openDone = TRUE;
	}
}
/*
 * Open glbl file
 */
void
OpenGlbl (void)
{
	if (glbl_openDone == FALSE)
	{
		open_rec (glbl, glbl_list, GLBL_NO_FIELDS, "glbl_id_no");
		glbl_openDone = TRUE;
	}
}
/*
 * Open glca file
 */
void
OpenGlca (void)
{
	if (glca_openDone == FALSE)
	{
		open_rec (glca, glca_list, GLCA_NO_FIELDS, "glca_id_no");
		glca_openDone = TRUE;
	}
}
/*
 * Open glct file
 */
void
OpenGlct (void)
{
	if (glct_openDone == FALSE)
	{
		open_rec (glct, glct_list, GLCT_NO_FIELDS, "glct_mod_date");
		glct_openDone = TRUE;
	}
}
/*
 * Open glih file
 */
void
OpenGlih (void)
{
	if (glih_openDone == FALSE)
	{
		open_rec (glih, glih_list, GLIH_NO_FIELDS, "glih_id_no");
		glih_openDone = TRUE;
	}
}
/*
 * Open glid file
 */
void
OpenGlid (void)
{
	if (glid_openDone == FALSE)
	{
		open_rec (glid, glid_list, GLID_NO_FIELDS, "glid_id_no");
		glid_openDone = TRUE;
	}
}
/*
 * Open gljc file
 */
void
OpenGljc (void)
{
	if (gljc_openDone == FALSE)
	{
		open_rec (gljc, gljc_list, GLJC_NO_FIELDS, "gljc_id_no");
		gljc_openDone = TRUE;
	}
}
/*
 * Open gllg file
 */
void
OpenGllg (void)
{
	if (gllg_openDone == FALSE)
	{
		open_rec (gllg, gllg_list, GLLG_NO_FIELDS, "gllg_id_no");
		gllg_openDone = TRUE;
	}
}

/*
 * Open Glln file. 
 */
void
OpenGlln2 (void)
{
	static	int	alias_done_before	=	FALSE;

	if (alias_done_before == FALSE)
	{
		alias_done_before = TRUE;
		abc_alias (glln2, glln);
	}
	if (glln2_openDone == FALSE)
	{
		open_rec (glln2,glln_list, GLLN_NO_FIELDS, "glln_id_no2"); 
		glln2_openDone = TRUE;
	}
}
/*
 * Open glln file
 */
void
OpenGlln (void)
{
	if (glln_openDone == FALSE)
	{
		open_rec (glln, glln_list, GLLN_NO_FIELDS, "glln_id_no");
		glln_openDone = TRUE;
	}
}
/*
 * Open glmr file
 */
void
OpenGlmr (void)
{
	if (glmr_openDone == FALSE)
	{
		strcpy (GlMask, 	"AAAAAAAAAAAAAAAA");
		sprintf (GlFormat,	"XXXXXXXXXXXXXXXX");
		sprintf (GlDesc,  	"%-16.16s", ML ("Account Number"));
		sprintf (GlfDesc, 	"%-16.16s", ML ("Account Number"));
		open_rec (glmr, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
		glmr_openDone = TRUE;
	}
}
/*
 * Open glna file
 */
void
OpenGlna (void)
{
	if (glna_openDone == FALSE)
	{
		open_rec (glna, glna_list, GLNA_NO_FIELDS, "glna_id_no");
		glna_openDone = TRUE;
	}
}
/*
 * Open glpd file
 */
void
OpenGlpd (void)
{
	if (glpd_openDone == FALSE)
	{
		open_rec (glpd, glpd_list, GLPD_NO_FIELDS, "glpd_id_no");
		glpd_openDone = TRUE;
	}
}
/*
 * Open gltr file
 */
void
OpenGltc (void)
{
	if (gltc_openDone == FALSE)
	{
		open_rec (gltc, gltc_list, GLTC_NO_FIELDS, "gltc_id_no");
		gltc_openDone = TRUE;
	}
}
/*
 * Open gltr file
 */
void
OpenGltr (void)
{
	if (gltr_openDone == FALSE)
	{
		open_rec (gltr, gltr_list, GLTR_NO_FIELDS, "gltr_id_no");
		gltr_openDone = TRUE;
	}
}
/*
 * Open glus file
 */
void
OpenGlus (void)
{
	if (glus_openDone == FALSE)
	{
		open_rec (glus, glus_list, GLUS_NO_FIELDS, "glus_id_no");
		glus_openDone = TRUE;
	}
}
/*
 * Open glwk file
 */
void
OpenGlwk (void)
{
	if (glwk_openDone == FALSE)
	{
		open_rec (glwk, glwk_list, GLWK_NO_FIELDS, "glwk_id_no");
		glwk_openDone = TRUE;
	}
}
/*
 * Open pocr file
 */
void
OpenPocr (void)
{
	if (pocr_openDone == FALSE)
	{
		open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
		pocr_openDone = TRUE;
	}
}
/*
 *************************************************************************
 * FindPocr (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	companyNo		-	comm_co_no
 *	currencyCode	-	Currency Code.
 *	updFlag			=	UpdateFlag. "r"=Read, "u"=Update, "w"=Write.
 */
 int
 FindPocr (
 	char	*companyNo,
	char	*currencyCode,
	char	*updFlag)
{
	OpenPocr ();

	sprintf (pocrRec.co_no, "%-2.2s", companyNo);
	sprintf (pocrRec.code,  "%-3.3s", currencyCode);
	return (find_rec (pocr, &pocrRec, COMPARISON, updFlag));
}

/*
 *************************************************************************
 * CurrencyFgnAmt (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	localAmount		-	local amount to convert.
 */
double 
CurrencyFgnAmt (
	double	localAmount)
{
	if (pocr_openDone == FALSE)
		return((double) -1.00);

	switch(pocrRec.pocr_operator[0])
	{
		case '*':
			return (localAmount / pocrRec.ex1_factor);
		case '/':
			return (localAmount * pocrRec.ex1_factor);
	}
	return (localAmount * pocrRec.ex1_factor);
}

/*
 *************************************************************************
 * CurrencyLocAmt (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	FgnAmount		-	Fgn amount to convert.
 */
double 
CurrencyLocAmt (
	double	fgnAmount)
{
	if (pocr_openDone == FALSE)
		return((double) -1.00);
	
	switch (pocrRec.pocr_operator[0])
	{
		case '*':
			return (fgnAmount * pocrRec.ex1_factor);
		case '/':
			return (fgnAmount / pocrRec.ex1_factor);
		default:
			return (fgnAmount / pocrRec.ex1_factor);
	}
	return (fgnAmount / pocrRec.ex1_factor);
}

/*
 *************************************************************************
 * SearchPocr (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	companyNo		-	comm_co_no
 *	keyedValue  	-	Search value.
 */
void
SearchPocr (
	char	*companyNo,
	char 	*keyedValue)
{
	OpenPocr ();
	
	_work_open (3,0,40);

	save_rec("#No.", "#Currency Description");

	sprintf (pocrRec.co_no, "%-2.2s", companyNo);
	sprintf (pocrRec.code,  "%-3.3s", keyedValue);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && !strcmp (pocrRec.co_no, companyNo) &&
			!strncmp (pocrRec.code, keyedValue, strlen (keyedValue)))
	{
		cc = save_rec (pocrRec.code, pocrRec.description);
		if (cc)
			break;

		cc = find_rec (pocr, &pocrRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (pocrRec.co_no, "%-2.2s", companyNo);
	sprintf (pocrRec.code,  "%-3.3s", temp_str);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	if (cc)
		strcpy (temp_str, "");
}
/*
 *************************************************************************
 * FindGljc (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	companyNo		-	comm_co_no
 *	journalType		-	Journal Type
 *	updFlag			=	UpdateFlag. "r"=Read, "u"=Update, "w"=Write.
 */
 int
 FindGljc (
 	char	*companyNo,
	char	*journalType,
	char	*updFlag)
{
	OpenGljc ();

	sprintf (gljcRec.co_no,      "%-2.2s", companyNo);
	sprintf (gljcRec.journ_type, "%-2.2s", journalType);
	return (find_rec (gljc, &gljcRec, COMPARISON, updFlag));
}
/*
 *************************************************************************
 * FindGlct (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 */
 int
 FindGlct (void)
{
	OpenGlct ();

	glctRec.mod_date	=	0L;
	return (find_rec (glct, &glctRec, GTEQ, "r"));
}
/*
 *************************************************************************
 * GL_NextBatchNo (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	companyNo		-	comm_co_no
 *	journalType		-	Journal Type
 */
long
GL_NextBatchNo (
 	char	*companyNo,
	char	*journalType)
{
	OpenGljc ();

 	cc = FindGljc (companyNo, journalType, "u");
	if (cc)
		file_err (cc, gljc, "DBFIND");

	gljcRec.run_no++;

	cc = abc_update (gljc, &gljcRec);
	if (cc)
		file_err (cc,gljc,"DBUPDATE");

	return (gljcRec.run_no);
}
/*
 *************************************************************************
 * FindGlbh (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	companyNo		-	comm_co_no
 *	branchNo		-	comm_est_no
 *	journalType		-	Journal Type
 *	batchNo			-	Batch Number
 *	updFlag			=	UpdateFlag. "r"=Read, "u"=Update, "w"=Write.
 */
 int
 FindGlbh (
 	char	*companyNo,
 	char	*branchNo,
	char	*journalType,
	char	*batchNo,
	char	*updFlag)
{
	OpenGlbh ();

	sprintf (glbhRec.co_no,    "%-2.2s", 	companyNo);
	sprintf (glbhRec.br_no,    "%-2.2s", 	branchNo);
	sprintf (glbhRec.jnl_type, "%-2.2s", 	journalType);
	sprintf (glbhRec.batch_no, "%-10.10s", 	batchNo);
	return (find_rec (glbh, &glbhRec, COMPARISON, updFlag));
}
	
void
GL_Close (void)
{
	if (glbd_openDone == TRUE)
		abc_fclose (glbd);

	if (glbh_openDone == TRUE)
		abc_fclose (glbh);

	if (glbl_openDone == TRUE)
		abc_fclose (glbl);

	if (glca_openDone == TRUE)
		abc_fclose (glca);

	if (glct_openDone == TRUE)
		abc_fclose (glct);

	if (glid_openDone == TRUE)
		abc_fclose (glid);

	if (glih_openDone == TRUE)
		abc_fclose (glih);

	if (gljc_openDone == TRUE)
		abc_fclose (gljc);

	if (gllg_openDone == TRUE)
		abc_fclose (gllg);

	if (glln_openDone == TRUE)
		abc_fclose (glln);

	if (glln2_openDone == TRUE)
		abc_fclose (glln2);

	if (glmr_openDone == TRUE)
		abc_fclose (glmr);

	if (glna_openDone == TRUE)
		abc_fclose (glna);

	if (glpd_openDone == TRUE)
		abc_fclose (glpd);

	if (gltc_openDone == TRUE)
		abc_fclose (gltc);

	if (gltr_openDone == TRUE)
		abc_fclose (gltr);

	if (glwk_openDone == TRUE)
		abc_fclose (glwk);

	if (pocr_openDone == TRUE)
		abc_fclose (pocr);
}

/*
 ************************************************************************
 * GL_PostSetup (S)
 ************************************************************************
 *	Description	:	General Ledger posting setup routine.
 *
 *	Notes		:	Post_setup sets the variables and opens the 
 *					files needed during a general ledger posting
 *					session.
 *			
 *	Parameters	:	co_no	    - Company number.
 *					budgetNo	- Budget number.
 *
 *	Globals		:	PV_co_no    - Company to be posted to.
 *					PV_rec_time - Recovery time variable.
 *					PV_rec_flag - Recovery ON/OFF flag.
 */
void
GL_PostSetup (
	char	*companyNo, 
	int		budgetNo)
{
	strcpy (PV_co_no, companyNo);
	GL_PostBudget (budgetNo);
	PV_rec_time = 0L;
	PV_rec_flag = FALSE;

	OpenGlmr ();
	OpenGlln2 ();
	OpenGlpd ();
}

/*
 ************************************************************************
 * GL_PostBudget (S)
 ************************************************************************
 * 	Description	:	Set budget used during posting session.
 * 
 * 	Parameters	:	budgetNo   - Budget number.
 * 
 * 	Globals		:	PV_budget - Global budget no.
 */
void
GL_PostBudget (
	int		budgetNo)
{
	PV_budget = budgetNo;
}

/*
 ************************************************************************
 * GL_PostStamp (S)
 ************************************************************************
 * 	Description	:	Set time stamp used during posting session.
 * 
 * 	Globals		:	GV_post_time - Global posting time.
 * 
 * 	Returns		:	GV_post_time - Stamped posting time.
 */
int
GL_PostStamp (void)
{
	GV_post_time	= time (NULL);
	GV_link_cnt		= 0;
	return (GV_post_time);
}

/*
 ************************************************************************
 * GL_PostRecover (S)
 ************************************************************************
 * 	Description	:	Set time used during recovery session.
 * 
 * 	Note		:	The recovery time should be set only during
 * 					a posting recovery run when the consolidated 
 * 					audit trail is being used to recover from a
 * 					crash during a normal posting run.
 * 
 * 	Globals		:	PV_rec_time - Recovery time.
 */
void
GL_PostRecover (
	long	postTime)
{
	PV_rec_time = postTime;
}

/*
 ************************************************************************
 * GL_PostTerminate (S)
 ************************************************************************
 * 	Description	:	End posting session.
 * 
 * 	Notes		:	Post_terminate closes the files used by the
 * 				posting routines.
 */
void
GL_PostTerminate (void)
{
}

/*
 ************************************************************************
 * GL_PostAccount (S)
 ************************************************************************
 * 	Description	:	Post amount to specified account hierarchy.
 * 
 * 	Note		:	Post_account posts the specified amount into
 * 					all the accounts in the hierarchy above the
 * 					account number passed to it.
 * 
 * 					e.g. The parameters :
 * 
 * 					     100-100-100 01/04/89 100000.00
 * 					     and a fiscal year end of 3,
 * 					     GV_fiscal = 3,
 * 
 * 					     would post $100000.00 into 
 * 					     period 01, year 1990 of accounts
 * 					     100-000-000, 100-100-100 and
 * 					     100-100-100 and any statistical
 * 					     trees any of the accounts in the
 * 					     hierarchy participated in.
 * 
 * 	Parameters	:	accountNo 	- Account number of lowest account in hierarchy.
 * 					postDate 	- Calendar date to post into.
 * 					currCode	- Currency code of foreign amount.
 * 					fgnAmount 	- Foreign Amount to post.
 * 					locAmount   - Amount to post.
 * 
 * 	Returns		:	1      - If posting fails.
 * 					0      - If posting succeeds.
 */
int
GL_PostAccount (
	char	*accountNo, 
	Date	 postDate, 
	char 	*currCode, 
	double 	fgnAmount, 
	double 	locAmount)
{
	int		errCode	=	0;

	short	fdmy [3];

	get_fdmy (fdmy, postDate);

	errCode	=	_PostAccount 
				(
					accountNo, 
					fdmy [2], 
					fdmy [1], 
					currCode, 
					fgnAmount, 
					locAmount
				);
	return (errCode);
}

/*
 ************************************************************************
 * _PostAccount (S)
 ************************************************************************
 * 	Description	:	Post amount to specified account hierarchy.
 * 
 * 	Note		:	This function posts the specified amount into
 * 					all the accounts in the hierarchy above the
 * 					account number passed to it.
 * 
 * 					e.g. The parameters :
 * 
 * 					     100-100-100 90 01 100000.00
 * 					     and a fiscal year end of 3,
 * 					     GV_fiscal = 3,
 * 
 * 					     would post $100000.00 into 
 * 					     period 01, year 1990 of accounts
 * 					     100-000-000, 100-100-100 and
 * 					     100-100-100 and any statistical
 * 					     trees any of the accounts in the
 * 					     hierarchy participated in.
 * 
 * 	Parameters	:	accountNo 	- Account number of lowest account in hierarchy.
 * 					year     	- Finacial Year to post into.
 * 					period    	- Financial period to post into.
 * 					currCode	- Currency code of foreign amount.
 * 					fgnAmount   - Foreign Amount to post.
 * 					locAmount   - Amount to post.
 * 
 * 	Returns		:	1      - If posting fails.
 * 					0      - If posting succeeds.
 */
int
_PostAccount (
	char	*accountNo, 
	int		year, 
	int		period, 
	char	*currCode, 
	double	fgnAmount, 
	double	locAmount)
{
	GLMR_STRUCT	glmrPost;

	int			i		=	0, 
				level	=	0,
				errCode	=	0;
	char		tempAccount [19];

	GV_link_cnt = 0;

	i = GL_ReadAccountNo (accountNo, &glmrPost);
	strcpy (PV_acc_curr, glmrPost.curr_code);

	errCode	=	GL_PostLinks
				(
					glmrPost.hhmr_hash, 
					year, 
					period, 
					currCode, 
					fgnAmount, 
					locAmount
				);
	if (errCode)
		return (EXIT_FAILURE);

	if (i)
	{
		GL_UpdatePeriod 
		(
			glmrPost.hhmr_hash, 
			year, 
			period, 
			currCode, 
			-fgnAmount, 
			-locAmount
		);
	}

	if (glmrPost.glmr_class [0][0] != 'N')
	{
		for (level = GV_cur_level - 1; level; level--)
		{
			GL_GetAccountNo (level, tempAccount);
			i = GL_ReadAccountNo (tempAccount, &glmrPost);
			strcpy (PV_acc_curr, glmrPost.curr_code);

			errCode	=	GL_PostLinks
						(
							glmrPost.hhmr_hash, 
							year, 
							period, 
							currCode, 
							fgnAmount, 
							locAmount
						);
			if (errCode)
				return (EXIT_FAILURE);

			if (i)
			{
				GL_UpdatePeriod 
				(
					glmrPost.hhmr_hash, 
					year, 
					period, 
					currCode, 
					-fgnAmount, 
					-locAmount
				);
			}
		}
		return GL_PostControl (year, period, currCode, fgnAmount, locAmount);
	}
	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * GL_PostControl (S)
 ************************************************************************
 * 	Description	:	Post amounts into the company control account.
 * 
 * 	Parameters	:	year   		- Finacial year to post into.
 * 					period  	- Financial period to post into.
 * 					currCode  	- Currency code of foreign amount.
 * 					fgnAmount 	- Foreign Amount to post.
 * 					locAmount 	- Local Amount to post.
 * 
 * 	Returns		:	1    - If posting fails.
 * 					0    - If posting succeeds.
 */
int
GL_PostControl (
	int		year, 
	int 	period, 
	char 	*currCode, 
	double 	fgnAmount, 
	double	locAmount)
{
	int		errCode	=	0;

	GLMR_STRUCT	glmrContRec;

	if (GL_ReadAccountNo ("00000000000000000000000000000", &glmrContRec))
	{
		fgnAmount	= 0.00;
		locAmount	= 0.00;
	}
	strcpy (PV_acc_curr, glmrContRec.curr_code);
	errCode	=	GL_UpdatePeriod 
				(
					glmrContRec.hhmr_hash, 
					year, 
					period, 
					currCode, 
					fgnAmount, 
					locAmount
				);
	return (errCode);
}

/*
 ************************************************************************
 * GL_PostLinks (S)
 ************************************************************************
 * 	Description	:	Post into accounts with specified hash.
 * 
 * 	Notes		:	The routine post_links doesn't need to read
 * 				the account record itself as it directly loads
 * 				and updates the periods containing the correct
 * 				hash, year and period number.
 * 
 * 				N.B. This routine recurses via GL_UpdateLinks.
 * 			
 * 	Parameters	:	hhmrHash	- Serial number of account to post into.
 * 					year   		- Financial year to post into.
 * 					period 		- Financial period to post into.
 * 					currCode  	- Currency code of foreign amount.
 * 					fgnAmount 	- Foreign Amount to post.
 * 					locAmount 	- Amount to post.
 * 
 * 	Returns		:	1    - If posting succeeds.
 * 					0    - If posting fails.
 */
int
GL_PostLinks (
	long	hhmrHash, 
	int		year, 
	int 	period, 
	char 	*currCode, 
	double 	fgnAmount, 
	double 	locAmount)
{
	int		errCode	=	0;

	errCode	=	GL_UpdatePeriod 
				(
					hhmrHash, 
					year, 
					period, 
					currCode, 
					fgnAmount, 
					locAmount
				);
	if (errCode)
		return (errCode);

	errCode	=	GL_UpdateLinks 
				(
					hhmrHash, 
					year, 
					period, 
					currCode, 
					fgnAmount, 
					locAmount
				);
	return (errCode);
}

/*
 ************************************************************************
 * GL_ReadAccountNo (S)
 ************************************************************************
 * 	Description	:	Read General Ledger master record.
 * 
 * 	Globals		:	PV_co_no - Company number.
 * 
 * 	Parameters	:	acc_no   - Account number to be read.
 * 					glmr_obj - Object into which to place data.
 */
static	int
GL_ReadAccountNo (
	char 		*accountNo, 
	GLMR_STRUCT *glmr_obj)
{
	char	formAccount [FORM_LEN + 1];

	strcpy (glmr_obj->co_no, PV_co_no);
	strcpy (formAccount, accountNo);

	if (GL_FormAccNo (formAccount, glmr_obj->acc_no, 0))
	{
		sprintf (glWorkStr, "GL_ReadAccountNo (%s) Cannot post to level",
														glmr_obj->acc_no);
		file_err (-1, glWorkStr, "DBFIND");
	}
	return (find_rec (glmr, glmr_obj, COMPARISON, "r"));
}

/*
 ************************************************************************
 * GL_UpdateLinks (S)
 ************************************************************************
 * 	Description	:	Post into accounts with specified hash.
 * 
 * 	Notes		:	Update_links reads through an account hierarchy
 * 					and updates the periods containing the correct
 * 					hash, year and period number by calling
 * 					post_links.
 * 
 * 				N.B. This routine recurses via post_links.
 * 			
 * 	Parameters	:	hhmrHash	- Serial number of account to post into.
 * 					year   		- Financial year to post into.
 * 					period  	- Financial period to post into.
 * 					currCode   	- Currency code of foreign amount.
 * 					fgnAmount 	- Foreign Amount to post.
 * 					locAmount 	- Amount to post.
 * 
 * 	Returns		:	1    - If posting succeeds.
 * 					0    - If posting fails.
 */
static int
GL_UpdateLinks (
	long	hhmrHash, 
	int		year, 
	int		period,
	char 	*currCode,
	double	fgnAmount,
	double 	locAmount)
{
	int		errCode	=	0,
			cc		=	0;

	long	tmpParent	=	0L,
			tmpChild	=	0L;

	GLLN_STRUCT	gllnPost;

	gllnPost.child_hash	= hhmrHash;
	gllnPost.parent_hash = 0L;

	cc = find_rec (glln2, &gllnPost, GTEQ,"r");
	while (!cc && gllnPost.child_hash == hhmrHash)
	{
		tmpParent 	= gllnPost.parent_hash;
		tmpChild 	= gllnPost.child_hash;

		errCode	=	GL_PostLinks 
					(
						gllnPost.parent_hash, 
						year, 
						period, 
						currCode, 
						fgnAmount, 
						locAmount
					);
		if (errCode)
			return (errCode);

		gllnPost.parent_hash	= tmpParent + 1;
		gllnPost.child_hash	= tmpChild;
		cc = find_rec (glln2, &gllnPost, GTEQ,"r");
	}

	/*
		Reset Pointers.
	*/
	gllnPost.child_hash	= hhmrHash;
	gllnPost.parent_hash = 0L;
	cc = find_rec (glln2, &gllnPost, GTEQ,"r");
	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * GL_UpdatePeriod (S)
 ************************************************************************
 * 	Description	:	Update / Create General Ledger period records.
 * 
 * 	Notes		:	Update_period either finds and adds to existing
 * 				period balances or if the period doesn't already
 * 				exist it is created.
 * 			
 * 	Parameters	:	hhmrHash 	- Serial number of account to post into.
 * 					year		- Financial year to post into.
 * 					period		- Financial period to post into.
 * 					currCode	- Currency code of foreign amount.
 * 					fgnAmount	- Foreign Amount to post.
 * 					locAmount	- Local Amount to post.
 * 
 * 	Globals		:	GV_link_cnt  - Count of links updated.
 * 					PV_budget    - Budget to update.
 * 					PV_rec_time  - Check time for recovery posting.
 * 					GV_post_time - Stamp time for posting session.
 * 
 * 	Returns		:	1    - If posting succeeds.
 * 					0    - If posting fails.
 */
static int
GL_UpdatePeriod (
	long	hhmrHash,
	int		year,
	int		period,
	char 	*currCode,
	double	fgnAmount,
	double 	locAmount)
{
	GLPD_STRUCT	glpdPost;
	int			glpdError;

	GV_link_cnt++;

	/*
	 * The currency of the foreign amount passed is not the same as the 
	 * currency of the account. Therefore we must convert the LOCAL amount  
	 * to the currency of the account. 
	 */
	if (strcmp (PV_acc_curr, currCode))
	{
		cc = FindPocr (PV_co_no, PV_acc_curr, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		fgnAmount = CurrencyFgnAmt (locAmount);
	}

	glpdPost.hhmr_hash 	= hhmrHash;
	glpdPost.budg_no 	= PV_budget;
	glpdPost.year 		= (year < 1000) ? year + 1900 : year;
	glpdPost.prd_no 		= period;
	glpdPost.hhgp_hash	=	0L;

	glpdError = find_rec (glpd, &glpdPost, COMPARISON, "u");
	if (PV_rec_time && !RECOVER)
	{
		if (glpdPost.mod_time != PV_rec_time)
			PV_rec_flag = TRUE;
		else
		{
			abc_unlock (glpd);
			return (EXIT_SUCCESS);
		}
	}
	glpdPost.mod_time = RECOVER ? PV_rec_time : GV_post_time;
	sprintf (glpdPost.user_id, "%-8.8s", getlogin ());

	/*	
	 * Period doesn't exist - create. 
	 */
	if (glpdError)	
	{
		glpdPost.fx_balance	= fgnAmount;
		glpdPost.balance 	= locAmount;
		glpdError = abc_add (glpd, &glpdPost);
		if (glpdError)
		{	
			abc_unlock (glpd);

			/*
			 * Cannot live with this not posting so try again.
			 */
			glpdPost.hhmr_hash 	= hhmrHash;
			glpdPost.budg_no	= PV_budget;
			glpdPost.year 		= (year < 1000) ? year + 1900 : year;
			glpdPost.prd_no 	= period;
			glpdError = find_rec (glpd, &glpdPost, COMPARISON, "u");
			if (glpdError)
			{
				glpdPost.fx_balance = fgnAmount;
				glpdPost.balance 	= locAmount;
				glpdPost.mod_time 	= RECOVER ? PV_rec_time : GV_post_time;
				sprintf (glpdPost.user_id, "%-8.8s", getlogin ());
				glpdError = abc_add (glpd, &glpdPost);

				/*
			 	 * Tried our best but an error it is.
			 	 */
				if (glpdError)
					file_err (glpdError, glpd, "DBADD");
			}
			else
			{
				glpdPost.fx_balance 	+= fgnAmount;
				glpdPost.balance 	+= locAmount;
				glpdPost.mod_time = RECOVER ? PV_rec_time : GV_post_time;
				sprintf (glpdPost.user_id, "%-8.8s", getlogin ());
				glpdError = abc_update (glpd, &glpdPost);
				/*
			 	 * Tried our best but an error it is.
			 	 */
				if (glpdError)
					file_err (glpdError, glpd, "DBUPDATE");
			}
		}
	}
	else
	{
		glpdPost.fx_balance 	+= fgnAmount;
		glpdPost.balance		+= locAmount;
		glpdError = abc_update (glpd, &glpdPost);
		/*
		 * Tried our best but an error it is.
		 */
		if (glpdError)
			file_err (glpdError, glpd, "DBUPDATE");
	}
	return (EXIT_SUCCESS);
}



/*
 * This routine returns a formatted account number with currency limitation
 */
int
SearchGlmr_CF (
	char 	*companyNo, 
	char 	*keyVal, 
	char 	*accClass, 
	char 	*currCode)
{
	int			err,
				tmp_len;
	char		accountNo [MAXLEVEL + 1];
	GLMR_STRUCT	s_glmr;
	char		desc[41];

	GL_initClass (accClass);

	GL_StripForm (accountNo, keyVal);	/* remove format info for raw accno */

	work_open ();
	save_rec ("# G/L Account No.", "# G/L Account Description   ");

	memset (&s_glmr, 0, sizeof (s_glmr));
	strcpy (s_glmr.co_no, companyNo);
	strcpy (s_glmr.acc_no, accountNo);

	tmp_len = strlen (accountNo);
	err = find_rec (glmr, (char *) &s_glmr, GTEQ, "r");
	while (!err &&
		!strcmp (s_glmr.co_no, companyNo) &&
		!strncmp (s_glmr.acc_no, accountNo, tmp_len))
	{
		char	formAccount [FORM_LEN + 1];

		if (strcmp(currCode, "") &&
			strncmp(s_glmr.curr_code, currCode, 3))
		{
			err = find_rec (glmr, (char *) &s_glmr, NEXT, "r");
			continue;
		}
		if (GL_ClassOK (&s_glmr))
		{
			strcpy (formAccount, s_glmr.acc_no);
			GL_FormAccNo (formAccount, s_glmr.acc_no, 0);
			sprintf(desc, "%25.25s - (%3.3s)",
					s_glmr.desc, s_glmr.curr_code);
			if (strcmp(currCode, ""))
			{
				if (save_rec (formAccount, desc))
					break;
			}
			else
			{
				if (save_rec (formAccount, s_glmr.desc))
					break;
			}
		}

		err = find_rec (glmr, (char *) &s_glmr, NEXT, "r");
	}
	disp_srch ();
	work_close ();

	return (EXIT_SUCCESS);
}

/*
 * This routine returns a formatted account number
 */
int
SearchGlmr_F (	
	char	*companyNo, 
	char	*keyVal, 
	char	*accClass)
{
	int			err,
				tmp_len;
	char		accountNo [MAXLEVEL + 1];
	GLMR_STRUCT	s_glmr;

	GL_initClass (accClass);

	GL_StripForm (accountNo, keyVal);	/* remove format info for raw accno */

	work_open ();
	save_rec ("# G/L Account No.", "# G/L Account Description   ");

	memset (&s_glmr, 0, sizeof (s_glmr));
	strcpy (s_glmr.co_no, companyNo);
	strcpy (s_glmr.acc_no, accountNo);

	tmp_len = strlen (accountNo);
	err = find_rec (glmr, (char *) &s_glmr, GTEQ, "r");
	while (!err &&
		!strcmp (s_glmr.co_no, companyNo) &&
		!strncmp (s_glmr.acc_no, accountNo, tmp_len))
	{
		char	formAccount [FORM_LEN + 1];

		if (GL_ClassOK (&s_glmr))
		{
			strcpy (formAccount, s_glmr.acc_no);
			GL_FormAccNo (formAccount, s_glmr.acc_no, 0);
			if (save_rec (formAccount, s_glmr.desc))
				break;
		}
		err = find_rec (glmr, (char *) &s_glmr, NEXT, "r");
	}
	disp_srch ();
	work_close ();

	return (EXIT_SUCCESS);
}

/*
 * This routine returns a stripped account number with currency limitation
 */
int
SearchGlmr_C (
	char 	*companyNo, 
	char 	*keyVal, 
	char 	*accClass, 
	char 	*currCode)
{
	char	accountNo [MAXLEVEL + 1];

	int	retval = SearchGlmr_CF (companyNo, keyVal, accClass, currCode);

	GL_StripForm (accountNo, temp_str);
	strcpy (temp_str, accountNo);

	return (retval);
}

/*
 * This routine returns a stripped account number.
 */
int
SearchGlmr (
	char	*companyNo, 
	char 	*keyVal, 
	char 	*accClass)
{
	char	accountNo [MAXLEVEL + 1];

	int	retval = SearchGlmr_F (companyNo, keyVal, accClass);

	GL_StripForm (accountNo, temp_str);
	strcpy (temp_str, accountNo);

	return (retval);
}

static	char	PV_class [4];

static	void
GL_initClass (
	char	*accClass)
{
	int	cnt;

	for (cnt = 0; cnt < 3 && *accClass; accClass++, cnt++)
		PV_class [cnt] = *accClass;

	while (cnt < 3)
		PV_class [cnt++] = '*';

	PV_class [3] = '\0';
}

static	int
GL_ClassOK (
	GLMR_STRUCT *glmr_ptr)
{
	int	cnt;

	for (cnt = 0; cnt < 3; cnt++)
		if (PV_class [cnt] != '*' &&
			glmr_ptr -> glmr_class [cnt] [0] != PV_class [cnt])
			return (FALSE);
	
	return (TRUE);
}

/*
 *****************************************************************************
 * 	Function	:	GL_FgnTotGlpd ()
 *****************************************************************************
 * 	Description	:	Total General Ledger periods.
 * 
 * 	Notes		:	The GL_FgnTotGlpd function is used to return the
 * 					total balance for a range of periods in a
 * 					particular budget and year.
 * 			
 * 	Parameters	:	hhmrHash	- 	Serial number of the glmr record
 * 					      			whose periods are to be totaled.
 * 					budgetNo	    - 	Budget number.
 * 					year	    - 	Year.
 * 					startPeriod	- 	Starting period.
 * 					endPeriod	- 	Ending period.
 * 
 * 	Returns		:	prdFgnBalance - Total for the period range.
 */
double
GL_FgnTotGlpd (
	long	hhmrHash, 
	int		budgetNo, 
	int		year, 
	int		startPeriod, 
	int		endPeriod)
{
	int	cc;
	double	prdFgnBalance = 0.0;
	GLPD_STRUCT	prdRec;

	year += (year < 1900) ? 1900 : 0;
	prdRec.hhmr_hash 	= hhmrHash;
	prdRec.budg_no 		= budgetNo;
	prdRec.year 		= year;
	prdRec.prd_no 		= startPeriod;

	cc = find_rec (glpd, &prdRec, GTEQ,"r");
	while (!cc && prdRec.hhmr_hash == hhmrHash &&
				  prdRec.budg_no 	== budgetNo &&
				  prdRec.year 		== year && 
				  prdRec.prd_no 	<= endPeriod)
	{
		prdFgnBalance += prdRec.fx_balance;
		
		cc = find_rec (glpd, &prdRec, NEXT,"r");
	}
	return (prdFgnBalance);
}

/*
 *****************************************************************************
 * 	Function	:	GL_LocTotGlpd ()
 *****************************************************************************
 * 
 * 	Description	:	Total General Ledger periods.
 * 
 * 	Notes		:	The GL_LocTotGlpd function is used to return the
 * 					total balance for a range of periods in a
 * 					particular budget and year.
 * 			
 * 	Parameters	:	hhmrHash	- 	Serial number of the glmr record
 * 					     			whose periods are to be totaled.
 * 				budgetNo	    - 	Budget number.
 * 				year			- 	Year.
 * 				startPeriod	    - 	Starting period.
 * 				endPeriod	    - 	Ending period.
 * 
 * 	Returns		:	prdLocBalance - Total for the period range.
 */
double 
GL_LocTotGlpd (
	long	hhmrHash, 
	int		budgetNo, 
	int		year, 
	int		startPeriod, 
	int		endPeriod)
{
	int	cc;
	double	prdLocBalance = 0.0;
	GLPD_STRUCT	prdRec;

	year += (year < 1900) ? 1900 : 0;
	prdRec.hhmr_hash 	= hhmrHash;
	prdRec.budg_no 	= budgetNo;
	prdRec.year 		= year;
	prdRec.prd_no 		= startPeriod;

	cc = find_rec (glpd, &prdRec, GTEQ,"r");
	while (!cc && prdRec.hhmr_hash == hhmrHash &&
				  prdRec.budg_no 	== budgetNo &&
				  prdRec.year 		== year && 
				  prdRec.prd_no 	<= endPeriod)
	{
		prdLocBalance += prdRec.balance;
		cc = find_rec (glpd, &prdRec, NEXT,"r");
	}

	return (prdLocBalance);
}

/*
 * Open General Ledger interface files.
 */
void
OpenGlInterface (void)
{
	char	*sptr;

	OpenGlih ();
	OpenGlid ();
	OpenGlmr ();

	sptr = chk_env ("DIS_FIND");
	GL_SubCat = (sptr == (char *)0) ? 11 : atoi (sptr);
}
/*
 *****************************************************************************
 * 	Function	:	GL_GLI ()
 *****************************************************************************
 * 	Description	:	Gets general ledger account from interface tables.
 * 
 * 	Notes		:	
 * 			
 * 	Parameters	:	coNo			- 	Company Number.
 * 					brNo    		- 	Branch number.
 * 					whNo			- 	Warehouse number.
 * 					interfaceCode 	- 	Interface code.
 * 					CustSaleType	-	Customer type or salesperson.
 * 					Category		-	Product category
 * 
 * 	Returns		:	N/A
 */
void	
GL_GLI (
	char	*coNo,  
	char	*brNo,   
	char	*whNo,   
	char	*interfaceCode,   
	char	*custSaleType,   
	char	*category)
{
	char	workString [12];

	GL_WideMode		= _wide;
	GL_GLI_Error 	= FALSE;
	GLI_messageNo 	= 0;

	OpenGlInterface ();

	sprintf (GL_InterfaceCode, "%-10.10s", interfaceCode);

	_SpecialRedraw = FALSE;
	/*
	 * Check for interface by Company
	 *          Interface code
	 *          Customer Type
	 *          Category
	 */
	sprintf 
	(
		workString, 
		"%-*.*s%*.*s", 
		GL_SubCat,
		GL_SubCat,
		category,
		11 - GL_SubCat,
		11 - GL_SubCat,
		" "
	);
	badGLI	=	TRUE;
	while (badGLI)
	{
		/*
		 * Find interface by coNo,interfaceCode,custSaleType, category
		 */
		badGLI	=	ReadGlih 
					(
						coNo,
						GL_InterfaceCode,   
						custSaleType,   
						category  
					);
		/*
		 * Find interface by coNo,interfaceCode,custSaleType, sub-category
		 */
		if (badGLI)
		{
			badGLI	=	ReadGlih 
						(
							coNo,
							GL_InterfaceCode,   
							custSaleType,   
							workString  
						);
		}
		/*
		 * Find interface by coNo,interfaceCode,custSaleType
		 */
		if (badGLI)
		{
			badGLI	=	ReadGlih 
						(
							coNo,
							GL_InterfaceCode,   
							custSaleType,   
							"           "
						);
		}
		/*
		 * Find interface by coNo,interfaceCode, category
		 */
		if (badGLI)
		{
			badGLI	=	ReadGlih 
						(
							coNo,
							GL_InterfaceCode,   
							"    ",
							category
						);
		}
		/*
		 * Find interface by coNo,interfaceCode, sub-category
		 */
		if (badGLI)
		{
			badGLI	=	ReadGlih 
						(
							coNo,
							GL_InterfaceCode,   
							"    ",
							workString
						);
		}
		/*
		 * Find interface by coNo,interfaceCode
		 */
		if (badGLI)
		{
			badGLI	=	ReadGlih 
						(
							coNo,
							GL_InterfaceCode,   
							"    ",
							"           "
						);
		}
		/*
		 * No interface found in glih.
		 */
		if (badGLI)
		{
			GLI_messageNo = 1;
			GLI_Error (coNo, brNo, whNo, GL_InterfaceCode, custSaleType, category);
			continue;
		}
		/*
		 * Interface found. 
		 * Try by Branch and Warehouse then by Branch then blank
		 */
		badGLI = ReadGlid (glihRec.hhih_hash, coNo, brNo, whNo);
		if (badGLI)
		{
			badGLI = ReadGlid (glihRec.hhih_hash, coNo, brNo, "  ");
		}
		if (badGLI)
		{
			badGLI = ReadGlid (glihRec.hhih_hash, coNo, "  ", "  ");
		}

		if (badGLI)
		{
			GLI_messageNo = 2;
			GLI_Error (coNo, brNo, whNo, GL_InterfaceCode, custSaleType, category);
		}
		/*
		 * Check if valid account.
		 */
		strcpy (glmrRec.co_no, coNo);
		strcpy (glmrRec.acc_no, glidRec.acct_no);
		badGLI = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (badGLI)
		{
			GLI_messageNo = 3;
			GLI_Error (coNo, brNo, whNo, GL_InterfaceCode, custSaleType, category);
		}

		/*
		 * Check if posting account.
		 */
		if (glmrRec.glmr_class [2][0] != 'P')
		{
			GLI_messageNo = 4;
			badGLI	=	TRUE;
			GLI_Error (coNo, brNo, whNo, GL_InterfaceCode, custSaleType, category);
		}
	}
	sprintf (GL_Account, "%-16.16s", glidRec.acct_no);
	if (GL_GLI_Error == TRUE)
	{
		if (GL_WideMode)
			swide ();
		else
			snorm ();
		_SpecialRedraw = TRUE;
		ungetc (' ', stdin);
	}
	return;
}

/*
 * Find General ledger interface header. 
 */
static	int
ReadGlih (
	char	*coNo,
	char	*interfaceCode,
	char	*custSaleType,
	char	*category)
{
	memset (&glihRec, 0, sizeof (glihRec));
	sprintf (glihRec.co_no,		"%-2.2s",	coNo);
	sprintf (glihRec.int_code,	"%-10.10s",	interfaceCode);
	sprintf (glihRec.class_type,"%-3.3s",	custSaleType);
	sprintf (glihRec.cat_no,	"%-11.11s",	category);
	return (find_rec (glih, &glihRec, COMPARISON, "r"));
}

/*
 * Find General ledger interface details. 
 */
static	int
ReadGlid (
	long	hhihHash,
	char	*coNo,
	char	*brNo,
	char	*whNo)
{
	memset (&glidRec, 0, sizeof (glidRec));
		
	glidRec.hhih_hash	=	hhihHash;
	sprintf (glidRec.br_no, "%-2.2s", brNo);
	sprintf (glidRec.wh_no, "%-2.2s", whNo);
	cc = find_rec (glid, &glidRec, COMPARISON, "r");
	if (cc)
		return (cc);

	strcpy (glmrRec.co_no, coNo);
	strcpy (glmrRec.acc_no, glidRec.acct_no);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		return (cc);

	if (glmrRec.glmr_class [2][0] != 'P')
		return (1);

	return (0);
}

/*
 * Process General Ledger Interface error.
 */
static	void
GLI_Error (
	char	*coNo,  
	char	*brNo,   
	char	*whNo,   
	char	*interfaceCode,   
	char	*custSaleType,   
	char	*category)
{
	char	workString [131];

	GL_GLI_Error	=	TRUE;

	if (!foreground ())
	{
		strcpy (GL_InterfaceCode, "SUSPENSE");
		return;
	}
	Dsp_open (0,0,16);

	sprintf (workString, "%5.5s%s - [%s]%5.5s"," ", ML ("LS10 General Ledger interface warning"), interfaceCode," ");
	Dsp_saverec (workString);
	Dsp_saverec ("");
	Dsp_saverec ("Make Selection.");

	if (GLI_messageNo == 1)
	{
		Dsp_saverec (ML ("ERROR : Interface is not defined."));
		Dsp_saverec (" ");
	}
	else if (GLI_messageNo == 2)
	{
		Dsp_saverec (ML ("ERROR : Interface details are not defined."));
		Dsp_saverec (" ");
	}
	else if (GLI_messageNo == 3)
	{
		Dsp_saverec (ML ("ERROR : Interface contains an invalid account."));
		sprintf (workString,"      : [%s]", glidRec.acct_no);
		Dsp_saverec (workString);
	}
	else if (GLI_messageNo == 4)
	{
		Dsp_saverec (ML ("ERROR : Interface contains an non posting account."));
		sprintf (workString,"      : [%s]", glidRec.acct_no);
		Dsp_saverec (workString);
	}

	sprintf (workString,"%s",ML("Please contract your systems administrator."));
	Dsp_saverec (workString);
	Dsp_saverec (" ");

	sprintf (workString,"%s", ML ("Interface Information "));
	Dsp_saverec (workString);
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGG ");
	sprintf (workString,"%7.7s%s[%s]", " ", ML ("Branch No      : "), brNo);
	Dsp_saverec (workString);
	sprintf (workString,"%7.7s%s[%s]", " ", ML ("Warehouse No.  : "), whNo);
	Dsp_saverec (workString);
	sprintf (workString,"%7.7s%s[%s]", " ", ML ("Cust Interface : "), custSaleType);
	Dsp_saverec (workString);
	sprintf (workString,"%7.7s%s[%s]", " ", ML ("Stock Category : "), category);
	Dsp_saverec (workString);
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	Dsp_saverec (ML ("Select one option listed below."));
	Dsp_saverec (ML ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"));
    Dsp_save_fn (ML (" Run interface maintenance now "), "R");
	Dsp_save_fn (ML (" Post to suspense account and continue "), "S");
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	Dsp_srch_fn (InterfaceSelect);
	Dsp_close ();
}

/*
 * Selection of available options from user.
 */
int
InterfaceSelect	(
	char	*selectKey)
{
	char	systemExecString [31];

	/*
	 * Run the interface program to setup missing interface.
	 */
	if (selectKey [0] == 'R')
	{
		sprintf (systemExecString, "gl_ariface I \"%s\"", GL_InterfaceCode);
		sys_exec (systemExecString);
	}
	/*
	 * Select the suspense account.
	 */
	if (selectKey [0] == 'S')
		strcpy (GL_InterfaceCode, "SUSPENSE");
	
	return (TRUE);
}


/*
 *****************************************************************************
 * 	Function	:	GL_ReadGlmr ()
 *****************************************************************************
 * 	Description	:	Gets general ledger account
 * 
 * 	Notes		:	
 * 			
 * 	Parameters	:	coNo	 - 	Company Number.
 *               	acc_no   - Account number to be read.
 * 					glmr_obj - Object into which to place data.
 *					findFlag - COMPARISON, GTEQ .....
 *					updFlag	 - 	UpdateFlag. "r"=Read, "u"=Update, "w"=Write.
 */
int
GL_ReadGlmr (
	char			*coNo,
	char			*acctNo,
	GLMR_STRUCT		*glmr_obj,
	int				findFlag,
	char			*updFlag)
{
	sprintf (glmr_obj->co_no,"%-2.2s", coNo);
	GL_StripForm (glmr_obj->acc_no,acctNo);
	return (find_rec (glmr, glmr_obj, findFlag, updFlag));
}
