/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_glpdCheck.c,v 5.0 2002/05/08 01:21:33 scott Exp $
|  Program Name  : (glpd_check.c) 
|  Program Desc  : (Global integrity check of the glpd file)
|---------------------------------------------------------------------|
|  Author        : Trevor van Bremen       | Date Written : 14/01/91  |
|---------------------------------------------------------------------|
| $Log: gl_glpdCheck.c,v $
| Revision 5.0  2002/05/08 01:21:33  scott
| CVS administration
|
| Revision 1.6  2002/02/26 06:52:20  cha
| Updated to fix S/C 761. ensure that null structures are handled correclty
|
| Revision 1.5  2002/02/22 04:37:06  scott
| S/C 00761 - DBUPDATE error due to file being updated when only "r" specified.
|
| Revision 1.4  2001/08/20 23:12:49  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_glpdCheck.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_glpdCheck/gl_glpdCheck.c,v 5.0 2002/05/08 01:21:33 scott Exp $";

/*
 *   Include file dependencies  
 */

#define		CF(x)	comma_fmt (DOLLARS (x), "NNN,NNN,NNN,NNN.NN")

#include <pslscr.h>
#include <GlUtils.h>
#include <twodec.h>

/*
 *   Constants, defines and stuff   
 */
char	*glmr2 	= "glmr2",
        *data 	= "data";

struct GLPD_LIST
{
	int		year;
	int		periodNo;
	double	locValue;
	double	fgnValue;
	struct	GLPD_LIST *_next;
};

#define	GLPD_NULL	(struct GLPD_LIST *) NULL
#define	ACT_LIST	0
#define	CHK_LIST	1

/*
 *   Local variables  
 */

struct	GLPD_LIST	*act_head = GLPD_NULL;
struct	GLPD_LIST	*chk_head = GLPD_NULL;
struct	GLPD_LIST	*tmp_list;

GLMR_STRUCT glmr2Rec;
FILE        *fout;

char    acc_bit [16][17];
char	dispStr	[256];
int     PV_max_len;
int     PV_max_lvl;
int     lvl_len [16];
int     tot_len [17];
int	    updateGlpd = FALSE;

/*
 *   Local function prototypes  
 */

void    OpenDB 			(void);
void    CloseDB 		(void);
void    shutdown_prog 	(void);
void    InitDisplay		(void);
void    Process 		(void);
void    CheckBalances 	(void);
void    CheckNFAccounts (void);
void    CheckFAccounts 	(void);
void    ProcessGlpd 	(long, char *, int);
void    ProcessError 	(void);
void    PrintDetails	(int, int, double, double, double, double);
void    ClearLists 		(void);
void    InsertAdditions (int, char *);
int     MoneyZero 		(double);
int     GetLevel 		(char *);
int     CheckCoControl 	(void);

struct GLPD_LIST *list_alloc (void);

/*
 *   Main Processing Routine    
 */
int
main (
 int	argc,
 char *	argv [])
{
    if (argc != 2)
    {
        printf ("Usage : %s <R(eport | U(pdate>\n", argv [0]);
        return (EXIT_FAILURE);
    }

    if (argv [1][0] == 'U' || argv [1][0] == 'u')
    {
        updateGlpd = TRUE;
    }

	OpenDB ();

	init_scr ();		/*  sets terminal from termcap	  */
	set_tty ();

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

/*
 * Open data base files
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (glmr2, glmr);

	OpenGlca ();
	OpenGlct ();
	OpenGlln ();
	OpenGlmr ();
	OpenGlpd ();
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (
 void)
{
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
	rv_pr ("General ledger period integrity check", 30, 0,1);
	Dsp_prn_open (0, 1, 17, dispStr, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0);
	
	Dsp_saverec (" CO |     ACCOUNT      | YEAR |PERIOD|   LOCAL  VALUE    |   CALCULATED      |   LOCAL  VALUE    |STATUS");

	Dsp_saverec (" NO |     NUMBER       |      |NUMBER|       ACTUAL      |   LOCAL  VALUE    |    DIFFERENCE     |      ");
	Dsp_saverec (" [FN03]  [FN05]  [FN14]  [FN15]  [FN16] ");
}
/*
 * Process the glmr records
 */
void
Process (void)
{
	cc = find_rec (glct, &glctRec, FIRST, "r");
	if (cc)
		file_err (cc, glct, "DBFIND");
    
	if (CheckCoControl ())
		file_err (cc, glct, "Invalid Format");

	strcpy (glmrRec.co_no, "  ");
	strcpy (glmrRec.acc_no, "                ");
	cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (!cc)
	{
		CheckBalances ();
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

/*
 * Validate glct_format
 */
int
CheckCoControl (void)
{
	int		lvl_no,
			len_tot = 0;
	char	*sptr,
			*tptr;

	for (lvl_no = 0; lvl_no < 16; lvl_no++)
        tot_len [lvl_no] = lvl_len [lvl_no] = 0;

	/*
	 * Format MUST consist only of a	combination of '-' and 'X'.
	 */
	tptr = sptr = clip (glctRec.format);
	while (*tptr)
	{
		if (*tptr == 'X' || *tptr == '-')
            tptr++;
		else
            return (EXIT_FAILURE);
	}

	/*
	 * Each level MUST be > 0 chars
	 */
	lvl_no = 0;
	while (TRUE)
	{
		tptr = strchr (sptr, '-');
		if (tptr)
		{
			*tptr = 0;
			lvl_len [lvl_no] = strlen (sptr);
			if (! (lvl_len [lvl_no]))
                return (EXIT_FAILURE);
            
			len_tot+= strlen (sptr);
			lvl_no++;
			tot_len [lvl_no] = len_tot;
			sptr = tptr + 1;
		}
		else
		{
			lvl_len [lvl_no] = strlen (sptr);
			len_tot	+= strlen (sptr);
			tot_len [lvl_no + 1] = len_tot;
			PV_max_len = len_tot;
			PV_max_lvl = lvl_no;
			break;
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Make sure the balance of this account is   
 * the same as the sum of the children accounts. 
 */
void
CheckBalances (void)
{
	/*
	 * Impossible to check posting accounts
	 */
	if (glmrRec.glmr_class [2][0] == 'P')
        return;

	if (glmrRec.glmr_class [0][0] == 'N')
        CheckNFAccounts ();
	else
        CheckFAccounts ();
}

void
CheckNFAccounts (void)
{
	abc_selfield (glmr2, "glmr_hhmr_hash");

	/*
	 * Produce total for MAIN acct
	 */
	ProcessGlpd (glmrRec.hhmr_hash, "   ", ACT_LIST);

	gllnRec.parent_hash = glmrRec.hhmr_hash;
	gllnRec.child_hash 	= 0L;

	cc = find_rec (glln, &gllnRec, GTEQ, "r");
	while (!cc && gllnRec.parent_hash == glmrRec.hhmr_hash)
	{
		glmr2Rec.hhmr_hash = gllnRec.child_hash;
		cc = find_rec (glmr2, &glmr2Rec, EQUAL, "r");
		if (cc)
            file_err (cc, glmr, "DBFIND");

		ProcessGlpd (glmr2Rec.hhmr_hash, glmrRec.curr_code, CHK_LIST);
		cc = find_rec (glln, &gllnRec, NEXT, "r");
	}

	ProcessError ();
	ClearLists ();
}

void
CheckFAccounts (void)
{
	char    acc_no [17];
	int     acc_len = 0,
            acc_lvl;

	abc_selfield (glmr2, "glmr_id_no");

	/*
	 * Produce total for MAIN acct
	 */
	ProcessGlpd (glmrRec.hhmr_hash, "   ", ACT_LIST);

	strcpy (acc_no, glmrRec.acc_no);
	clip (acc_no);

	acc_lvl = GetLevel (glmrRec.acc_no);
	acc_len = tot_len [acc_lvl];

	strcpy (glmr2Rec.co_no, glmrRec.co_no);
	sprintf (glmr2Rec.acc_no, "%-16.16s", glmrRec.acc_no);

    cc = find_rec (glmr2, &glmr2Rec, GREATER, "r");
	while (!cc)
	{
		if (strcmp (glmrRec.co_no, glmr2Rec.co_no))
            break;
        
		if (acc_len)
        {
			if (strncmp (glmrRec.acc_no, glmr2Rec.acc_no, acc_len))
                break;
        }
		if (glmr2Rec.glmr_class [0][0] == 'F')
		{
			if (GetLevel (glmr2Rec.acc_no) == acc_lvl + 1)
			{
				ProcessGlpd 
				(
					glmr2Rec.hhmr_hash, 
					glmrRec.curr_code, 
					CHK_LIST
				);
			}
		}
		cc = find_rec (glmr2, &glmr2Rec, NEXT, "r");
	}

	ProcessError ();

	ClearLists ();
}

/*
 * Ascertain which level this account is      
 */
int
GetLevel (
	char	*lcl_accno)
{
	char    acc_no [17];
	int     acc_lvl;

	strcpy (acc_no, lcl_accno);
	clip (acc_no);

	/*
	 * Split account number into level-sized bits
	 */
	for (acc_lvl = 0; acc_lvl < 16; acc_lvl++)
	{
		sprintf 
		(
			acc_bit [acc_lvl], 
			"%-*.*s",
			lvl_len [acc_lvl],
			lvl_len [acc_lvl],
			&acc_no [tot_len [acc_lvl]]
		);
	}

	acc_lvl = 16;
	while (acc_lvl > 0)
	{
		if (atol (acc_bit [acc_lvl - 1]) == 0L)
            acc_lvl--;
		else
            break;
	}
	return (acc_lvl);
}

void
ProcessGlpd (
	long	hhmrHash,
	char 	*currCode,
	int		list_no)
{
	glpdRec.hhmr_hash 	= hhmrHash;
	glpdRec.budg_no 	= 0;
	glpdRec.year 		= 0;
	glpdRec.prd_no 		= 0;

    cc = find_rec (glpd, &glpdRec, GTEQ, "u");
	while (!cc && glpdRec.budg_no == 0 && glpdRec.hhmr_hash == hhmrHash)
	{
		if (twodec (glpdRec.balance) 	!= glpdRec.balance ||
			twodec (glpdRec.fx_balance) != glpdRec.fx_balance)
		{
			glpdRec.balance 	= twodec (glpdRec.balance);
			glpdRec.fx_balance 	= twodec (glpdRec.fx_balance);
			cc = abc_update (glpd, &glpdRec);
			if (cc)
                file_err (cc, glpd, "DBUPDATE");
		}
		else
			abc_unlock (glpd);

		InsertAdditions (list_no, currCode);
		cc = find_rec (glpd, &glpdRec, NEXT, "u");
	}
}

void
ProcessError (void)
{
	struct	GLPD_LIST	*act_curr = act_head;
	struct	GLPD_LIST	*chk_curr = chk_head;

	while (1)
	{
		if (act_curr == GLPD_NULL)
		{
			if (chk_curr == GLPD_NULL)
            {
                break;
            }

			if (!MoneyZero (chk_curr->locValue))
			{
				PrintDetails 
				(
					chk_curr->year, 
					chk_curr->periodNo, 
					0.00, 
					chk_curr->locValue,
					0.00,
					chk_curr->fgnValue
				);
			}
			chk_curr = chk_curr->_next;
			continue;
		}

		if (chk_curr == GLPD_NULL)
		{
			if (act_curr == GLPD_NULL) 
			{
    			break;
			}
			if (!MoneyZero (act_curr->locValue))
			{
				PrintDetails 
				(
					act_curr->year, 
					act_curr->periodNo, 
					act_curr->locValue, 
					0.00,
					act_curr->fgnValue,
					0.00	
				);
			}
			act_curr = act_curr->_next;
			continue;
		}

		if (chk_curr->year < act_curr->year ||
		(chk_curr->year == act_curr->year &&
		     chk_curr->periodNo < act_curr->periodNo))
		{
			if (!MoneyZero (chk_curr->locValue))
			{
				PrintDetails 
				(
					chk_curr->year, 
					chk_curr->periodNo, 
					act_curr->locValue, 
					chk_curr->locValue,
					act_curr->fgnValue,
					chk_curr->fgnValue
				);
			}
			chk_curr = chk_curr->_next;
			continue;
		}

		if (act_curr->year < chk_curr->year ||
		(act_curr->year == chk_curr->year &&
		     act_curr->periodNo < chk_curr->periodNo))
		{
			if (!MoneyZero (act_curr->locValue))
			{
				PrintDetails 
				(
					act_curr->year, 
					act_curr->periodNo, 
					act_curr->locValue, 
					chk_curr->locValue,
					act_curr->fgnValue,
					chk_curr->fgnValue
				);
			}
			act_curr = act_curr->_next;
			continue;
		}
		PrintDetails 
		(
			act_curr->year, 
			act_curr->periodNo, 
			act_curr->locValue, 
			chk_curr->locValue,
			act_curr->fgnValue,
			chk_curr->fgnValue
		);
		act_curr = act_curr->_next;
		chk_curr = chk_curr->_next;
	}
	return;
}

void
PrintDetails (
	int    	year,
	int    	periodNo,
	double	locActValue,
	double	locCheckValue,
	double	fgnActValue,
	double	fgnCheckValue)
{
	char	fgnWork	[17],
			locWork [17];

	char	workStr [3] [21];

	int 	foundRecord		=	FALSE,
			updatedRecord	=	FALSE;
	if (updateGlpd)
	{
		/*
		 * Find glpd and update
		 */
		glpdRec.hhmr_hash   = glmrRec.hhmr_hash;
		glpdRec.budg_no     = 0;
		glpdRec.prd_no      = periodNo;
		glpdRec.year        = year;
		cc = find_rec (glpd, &glpdRec, EQUAL, "u");
		if (!cc)
		{
			foundRecord = TRUE;
	
			glpdRec.balance 	= locCheckValue;
			glpdRec.fx_balance 	= fgnCheckValue;
			cc = abc_update (glpd, &glpdRec);
			if (cc)
				file_err (cc, glpd, "DBUPDATE");

			updatedRecord = TRUE;
		}
		else
    		abc_unlock (glpd);
	}
    abc_unlock (glpd);

	sprintf (workStr [0], "%18.18s", CF (locActValue));
	sprintf (workStr [1], "%18.18s", CF (locCheckValue));
	sprintf (workStr [2], "%18.18s", CF (locActValue - locCheckValue));

	if (MoneyZero (locActValue - locCheckValue))
		strcpy (locWork, "    none     ");
	else
		sprintf (locWork, "%13.2f", locActValue - locCheckValue);

	if (MoneyZero (fgnActValue - fgnCheckValue))
		strcpy (fgnWork, "    none     ");
	else
		sprintf (fgnWork, "%13.2f", fgnActValue - fgnCheckValue);

	sprintf 
	(
		dispStr, 
		" %-2.2s ^E %-16.16s ^E %4d ^E  %02d  ^E%18.18s ^E%18.18s ^E%18.18s ^E%s",
		glmrRec.co_no,
		glmrRec.acc_no,
		year,
		periodNo,
		workStr [0],
		workStr [1],
		workStr [2],
		(MoneyZero (locActValue - locCheckValue)) ? "  OK" : "ERROR"
	);
	Dsp_saverec (dispStr);
}

void
ClearLists (void)
{
	struct	GLPD_LIST	*next_ptr;

	for (tmp_list = act_head; tmp_list != GLPD_NULL; tmp_list = next_ptr)
	{
		next_ptr = tmp_list->_next;
		free (tmp_list);
	}

	for (tmp_list = chk_head; tmp_list != GLPD_NULL; tmp_list = next_ptr)
	{
		next_ptr = tmp_list->_next;
		free (tmp_list);
	}

	act_head = chk_head = GLPD_NULL;
}

/*
 * Routine to insert or accumulate glpd values onto their respective entries
 */
void
InsertAdditions (
	int		list_no,
	char 	*currCode)
{
	struct	GLPD_LIST	*list_alloc (void);
	struct	GLPD_LIST	*head_ptr;
	struct	GLPD_LIST	*prev_ptr = GLPD_NULL;
	struct	GLPD_LIST	*temp_ptr = GLPD_NULL;

	/*
	 * Convert to the currency of the parent account.
	 */
	if (list_no == CHK_LIST)
	{
/*	Don't know why this existed but it is wrong as it converts fgn amounts back to local. 
		FindPocr (glmrRec.co_no, currCode);

		glpdRec.fx_balance = curr_fx_amt (glpdRec.balance);
*/
	}

	head_ptr = (list_no == ACT_LIST) ? act_head : chk_head;

	if (head_ptr == GLPD_NULL)
	{
		temp_ptr 			= list_alloc ();
		temp_ptr->year 		= glpdRec.year;
		temp_ptr->periodNo 	= glpdRec.prd_no;
		temp_ptr->locValue 	= glpdRec.balance;
		temp_ptr->fgnValue 	= glpdRec.fx_balance;

		temp_ptr->_next 	= GLPD_NULL;
		if (list_no == ACT_LIST)
            act_head = temp_ptr;
		else
            chk_head = temp_ptr;

		return;
	}

	for (tmp_list = head_ptr; tmp_list != GLPD_NULL; prev_ptr = tmp_list, tmp_list = tmp_list->_next)
	{
		if (tmp_list->year < glpdRec.year)
        {
            continue;
        }

		if (tmp_list->year == glpdRec.year &&
		    tmp_list->periodNo < glpdRec.prd_no)
        {
            continue;
        }

		if (tmp_list->year == glpdRec.year &&
		    tmp_list->periodNo == glpdRec.prd_no)
		{
			tmp_list->locValue 	+= glpdRec.balance;
			tmp_list->fgnValue 	+= glpdRec.fx_balance;
			return;
		}

		/*
		 * Break-out if this point reached.
		 */
		break;
	}

	temp_ptr 			= list_alloc ();
	temp_ptr->year 		= glpdRec.year;
	temp_ptr->periodNo 	= glpdRec.prd_no;
	temp_ptr->locValue 	= glpdRec.balance;
	temp_ptr->fgnValue 	= glpdRec.fx_balance;
	if (prev_ptr == GLPD_NULL)
	{
		temp_ptr->_next = head_ptr;
		if (list_no == ACT_LIST)
			act_head = temp_ptr;
		else
			chk_head = temp_ptr;
	}
	else
	{
		prev_ptr->_next = temp_ptr;
		temp_ptr->_next = tmp_list;
	}

	return;
}

struct	GLPD_LIST*
list_alloc (
 void)
{
	struct	GLPD_LIST	*alloc_ptr;

	alloc_ptr = (struct GLPD_LIST *) malloc (sizeof (struct GLPD_LIST));
	if (alloc_ptr == GLPD_NULL)
    {
		sys_err ("Error in list_alloc () During (MALLOC)", errno, PNAME);
    }

	return (alloc_ptr);
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

