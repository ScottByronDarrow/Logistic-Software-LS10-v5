/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: GlFormat.c,v 5.2 2001/08/06 22:40:52 scott Exp $
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : glformat.c                                     |
|  Source Desc       : General ledger format routines.                |
|                                                                     |
|  Library Routines  : set_* ()                                       |
|---------------------------------------------------------------------|
| $Log: GlFormat.c,v $
| Revision 5.2  2001/08/06 22:40:52  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 00:43:27  scott
| New library for 10.5
|
=====================================================================*/
#include	<std_decs.h>
#include	<GlUtils.h>

int		GV_max_level = 0,	/*	Holds max level for account.	*/
		GV_cur_level = 0;	/*	Holds level of current account.	*/

/*
 *	Function declarations
 */
static int	FmtReadAcc 		 (int, int, char *, GLMR_STRUCT *),
			FmtReadGlmr 	 (int, char *, GLMR_STRUCT *),
			FmtBadWidth 	 (int, char *),
			FmtBadLevel 	 (char *),
			FmtTopLevel 	 (void),
			FmtMidLevel 	 (void),
			FmtBotLevel 	 (void),
			FmtEmptyBit 	 (int),
			FmtFormChars 	 (char *);

static void	FmtInitBits (void);

/*
 *	Table of structures to hold each piece of a disassembled account number.
 *	There is also provision to hold the glca description for each piece.
 */

/*
 ****************************************************************************
 *	Structure	:	GV_acc_tab
 ****************************************************************************
 *	Description	:	Holds details of an entire account hierarchy.
 *
 *	Notes		:	The information in this table is used by 
 *					routines that need to know details of accounts
 *					of levels higher than the current account.
 *
 *					e.g. For account 100-100-100 the table
 *					     would hold details for accounts
 *					     100-000-000 and 100-100-000.
 *					
 *	Elements	:	accountBit	- Account part for level.
 *					accObj		- Account data for level.
 *					hhcaHash	- Account chart hash for level.
 */
static	struct	{
	char		accountBit	[MAXLEVEL + 1];
	GLMR_STRUCT accObj;
	long		hhcaHash;
} GV_acc_tab [MAXLEVEL];

#define	ACC_BIT		GV_acc_tab [level].accountBit
#define	ACC_OBJ		GV_acc_tab [level].accObj
#define	ACC_GLCA	GV_acc_tab [level].hhcaHash

/*
	Table to hold each piece of a disassembled account format mask.
*/
static	char	*maskBits [MAXLEVEL],	/*	Mask segments		     */
				PV_co_no [3];			/*	Current company.	     */

/*
 * 	UserCode contain the user code without leding and trailing space
 * 	fUserCode contain the formatted user code without leding and 
 * 	trailing space
 * 	dflt_sUserCode contain the string that used when user press default
 * 	value for GL account number (start GL account number)
 * 	dflt_sfUserCode contain the formatted string that used when user 
 * 	press default value for GL account number (start formatted GL account num)
 * 	dflt_eUserCode contain the string that used when user press default
 * 	value for GL account number (end GL account number)
 * 	dflt_efUserCode contain the formatted string that used when user 
 * 	press default value for GL account number (end formatted GL account number)
 * 
 * 	e.g. If the user code in esmr is "0011-11                        "
 * 		 and the input mask is         "NNNN-NNNN-NNNN-NNNN"
 * 				UserCode is         "001111"
 * 				fUserCode is        "0011-11"
 * 				dflt_sUserCode is    "0011110000000000"
 * 				dflt_sfUserCode is   "0011-1100-0000-0000"
 * 				dflt_eUserCode is    "0011119999999999"
 * 				dflt_efUserCode is   "0011-1199-9999-9999"
 */
				
static  char UserCode			 [MAXLEVEL + 1];
static  char fUserCode			 [FORM_LEN + 1];
static  char dflt_sUserCode		 [MAXLEVEL + 1];
static  char dflt_sfUserCode	 [FORM_LEN + 1];
static  char dflt_eUserCode		 [MAXLEVEL + 1];
static  char dflt_efUserCode	 [FORM_LEN + 1];
static  int accountant 	= FALSE;
static	int	formatted 	= FALSE;	/*	Accounts formatted if TRUE.  */

extern	struct var	vars [];

/*
 ****************************************************************************
 *	Function	:	GL_SetMask ()
 ****************************************************************************
 * 	Description	:	Breaks format mask down into pieces.
 * 
 * 	Notes		:	The separated format strings created by this
 * 					routine are used by other format functions.
 * 
 * 	Globals		:	GV_max_level - Maximum allowable account level.
 * 
 * 	Parameters	:	format - Format to be broken down.
 */
void
GL_SetMask (
	char *format)
{
	static	char	maskChars [32];
	char	*m_ptr, 
			*l_ptr, 
			tmpForm [FORM_LEN + 1];
	int		len;
	int 	i = 0;

	if (!strlen (format))
		format	=	strdup ("XXXXXXXXXXXXXXXX");

	strcpy (maskChars, "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN");
	if (!accountant)
	{
		while ((fUserCode [i] >= '0' && fUserCode [i] <= '9') ||
			   fUserCode [i] == '-')
		{
			maskChars [i] = fUserCode [i];
			i++;
		}
	}

	rtrim (format, tmpForm);
	m_ptr = maskChars;

	l_ptr = strtok (tmpForm, "-");

	for (GV_max_level = 0; l_ptr && GV_max_level < MAXLEVEL; GV_max_level++)
	{
		maskBits [GV_max_level] = m_ptr;
		len 		= strlen (l_ptr);
		m_ptr [len] = (char) NULL;
		m_ptr 		+= (len + 1);
		l_ptr 		= strtok ((char *) 0, "-");
	}
}
/*
 ****************************************************************************
 *  Function	:	GL_SetAccWidth ()
 ****************************************************************************
 * 	Description	:	Account formatting setup routine.
 * 				:	Sets format masks for use in input routines
 * 					and specifies whether account no's are to be
 * 					displayed formatted or not.
 * 
 * 	Parameters	:	f_flag	   -	If TRUE display accounts
 * 									format.
 * 	Parameters	:	a_flag	   -	A (ccountant) or N (on-accountant)
 * 									Accountant need not set up user code.
 */
char *
GL_SetAccWidth (
	char	*companyNo,
	int		f_flag)
{
	int	len, cc;
	static	char	accountMask [FORM_LEN + 1];

	sprintf (accountMask,"%-*.*s", 
						FORM_LEN,FORM_LEN,"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN");

	sprintf (fUserCode, "%-*.*s", MAXLEVEL,MAXLEVEL, GL_GetSecure (companyNo));

	accountant	= glusRec.super_user;
	if (accountant == -1)
	{
		sprintf (accountMask,"%-*.*s", 
					FORM_LEN,FORM_LEN, ML ("*** Security access denied ****"));
		accountant = FALSE;
		return (accountMask);
	}

	strcpy (PV_co_no, companyNo);

	cc 	=	FindGlct ();
	if (cc)
		file_err (cc, glct, "FindGlct");

	GL_SetMask (clip (glctRec.format));

	len = strlen (glctRec.format);
	if (!f_flag)
		len -= FmtFormChars (glctRec.format);
	else
	{
		formatted = TRUE;
		for (cc = 0; cc < len; cc++)
			if (!accountant && 
				 ((fUserCode [cc] >= '0' && fUserCode [cc] <= '9') ||
			       fUserCode [cc] == '-'))
			{
				* (accountMask + cc) = fUserCode [cc];
				dflt_sfUserCode [cc] = fUserCode [cc];
				dflt_efUserCode [cc] = fUserCode [cc];
			}
			else
			{
				fUserCode [cc] = '\0';
				if (glctRec.format [cc] == '-')
				{
					* (accountMask + cc) = '-';
					dflt_sfUserCode [cc] = '-';
					dflt_efUserCode [cc] = '-';
				}
				else
				{
					* (accountMask + cc) = 'N';
					dflt_sfUserCode [cc] = '0';
					dflt_efUserCode [cc] = '9';
				}
			}
	}

	accountMask [len] 		= (char) NULL;
	dflt_sfUserCode [len] 	= (char) NULL;
	dflt_efUserCode [len] 	= (char) NULL;
	GL_StripForm (UserCode, fUserCode);
	GL_StripForm (dflt_sUserCode, dflt_sfUserCode);
	GL_StripForm (dflt_eUserCode, dflt_efUserCode);
	return (accountMask);
}

/*
 ****************************************************************************
 *  Function	:	GL_GetSecure ()
 ****************************************************************************
 */
char	*
GL_GetSecure (
	char	*companyNo)
{
	int		errCode;
	OpenGlus ();

	sprintf (glusRec.co_no , "%-2.2s", companyNo);
	sprintf	 (glusRec.user_name, "%-14.14s", getenv ("LOGNAME"));
	errCode = find_rec (glus, &glusRec, COMPARISON, "r");
	if (errCode)
	{
		sprintf (glusRec.co_no , "%-2.2s", companyNo);
		sprintf	 (glusRec.user_name, "%-14.14s", "SYSTEM WIDE   ");
		errCode = find_rec (glus, &glusRec, COMPARISON, "r");
		if (errCode)
		{
			glusRec.super_user	=	1;
			sprintf (glusRec.acc_hdr_code, "%30.30s", " ");
			return (clip (glusRec.acc_hdr_code));
		}
	}
	return (clip (glusRec.acc_hdr_code));
}

/*
 ************************************************************************
 * 	Function	:	GL_SetBitWidth ()
 ************************************************************************
 * 	Description	:	Return format to be used for account part.
 * 
 * 	Notes		:	Set_bit_width returns a format suitable for
 * 					use in input routines to format account parts.
 * 
 * 	Parameters	:	level	   	  -	Level for which width is to be set.
 * 					previousWidth - Pointer to variable to hold width last set.
 *				 	accountWidth  -	Pointer to variable to hold width set.
 * 
 * 	Globals		:	GV_max_level - Maximum allowable account level.
 * 
 * 	Returns		:	0 - if function succeeds.
 * 					1 - if function fails.
 */
char	*GL_SetBitWidth (
	int 	level, 
	int 	*previousWidth, 
	int 	*accountWidth)
{
	if (level > GV_max_level || level < 1)
	{
		print_err ("Level must be within range 1 to %d", GV_max_level);
		return ((char *) NULL);
	}

	*previousWidth = *accountWidth;
	*accountWidth = strlen (maskBits [level - 1]);
	return (maskBits [level - 1]);
}

/*
 ************************************************************************
 * 	Function	:	GL_FormAccNo ()
 ************************************************************************
 * 	Description	:	Check account format.
 * 
 * 	Notes		:	Account string format is checked and parts of
 * 				the broken down account are stored in
 * 				GV_acc_tab.
 * 
 * 	Parameters	:	form_acc     -	String holding account to be formatted.
 * 					accountNo	 -	String to hold account WITHOUT format chars.
 * 					v_level	     -	0 -	Any level is valid.
 * 									1 -	Top level only.
 * 									2 -	Mid level only.
 * 									3 -	Detail level only.
 * 
 * 	Globals		:	GV_cur_level - Level of current account.
 * 					GV_max_level - Maximum allowable account level.
 * 					GV_acc_tab   - Structures holding account part informatin.
 * 	
 * 	Returns		:	0	     - 	if all checks succeed.
 * 					1	     -	if any check fails.
 */
int
GL_FormAccNo (
	char	*formAccount, 
	char	*accountNo, 
	int		v_level)
{
	int		level, len, max_len, a_len;
	char	*l_ptr, *f_ptr, acctStr [FORM_LEN + 1];

	strcpy (acctStr, formAccount);
	l_ptr = clip (acctStr);
	max_len = strlen (acctStr);
	GV_cur_level = 0;
	FmtInitBits ();

	for (level = 0; l_ptr < acctStr + max_len && level < GV_max_level; level++)
	{
		if ((f_ptr = strchr (l_ptr, '-')))
		{
			len = f_ptr - l_ptr;
			if (len != strlen (maskBits [level]))
				return (FmtBadWidth (level, l_ptr));
		}
		else
			len = strlen (maskBits [level]);

		a_len = strlen (l_ptr);
		strncpy (ACC_BIT, l_ptr, (len > a_len) ? a_len : len);
		ACC_BIT [len] = (char) NULL;

		if (!FmtEmptyBit (level) && GV_cur_level)
			GV_cur_level = 0;

		if (FmtEmptyBit (level) && !GV_cur_level)
			GV_cur_level = level;

		l_ptr += (f_ptr) ? len + 1 : len;
	}

	if (!GV_cur_level)
		GV_cur_level = level;

	if (GL_CheckLevel (v_level))
		return (EXIT_FAILURE);
	
	level = 0;
	for (*formAccount = (char) NULL; level < GV_max_level; level++)
	{
		strcat (formAccount, ACC_BIT);
		if (formatted && level < (GV_max_level - 1))
			strcat (formAccount, "-");
	}
	GL_StripForm (accountNo, formAccount);

	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * 	Function	:	GL_CheckAccNo ()
 ************************************************************************
 * 	Description	:	Load specified account and related accounts.
 * 
 * 	Notes		:	Check_accno loads the specified account and
 * 					all accounts above it whose numbers form 
 * 					the parts of the specified number.
 * 
 * 					e.g. For account 100-100-100 details for
 * 					     accounts 100-000-000 and
 * 					     100-100-000 would also be loaded.
 * 			
 * 	Parameters	:	t_flag	    - If TRUE load all details.
 * 					accountNo	- Account number to be checked.
 * 					accObj	    - Pointer to object to hold entire record.
 * 
 * 	Globals		:	GV_max_level - Maximum allowable account level.
 * 					GV_acc_tab   - Structures holding account part
 * 					       informatin.
 * 
 * 	Returns		:	1	     - If check fails.
 * 					0	     - If check succeeds.
 */
int
GL_CheckAccNo (
	int			t_flag, 
	char		*accountNo, 
	GLMR_STRUCT *accObj)
{
	int	len, level;
	char	*a_ptr, tmp_acc [FORM_LEN + 1];

	strcpy (tmp_acc, accountNo);
	pin_bfill (accountNo, '0', MAXLEVEL);
	
	OpenGlca ();

	for (a_ptr = accountNo, level = 0; level < GV_max_level; level++)
	{
		len = strlen (ACC_BIT);
		strncpy (a_ptr, ACC_BIT, len);
		if (FmtReadAcc (t_flag, level, accountNo, accObj))
		{
			strcpy (accountNo, tmp_acc);
			return (EXIT_FAILURE);
		}
		a_ptr += len;
	}
	strcpy (accountNo, tmp_acc);

	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * 	Function	:	GL_CheckLevel ()
 ************************************************************************
 * 	Description	:	Check account is correct level.
 * 
 * 	Parameters	:	level	 -	0 -	Any level is valid.
 * 								1 -	Top level only.
 * 								2 -	Mid level only.
 * 								3 -	Detail level only.
 * 	
 * 	Returns		:	TRUE	if correct level.
 * 					FALSE	if incorrect level.
 */
int
GL_CheckLevel (int level)
{
	return	 (level == 0) ? 0 :				/*	Any level		*/
			 (level == 1) ? FmtTopLevel () :	/*	First level		*/
			 (level == 2) ? FmtMidLevel () :	/*	Middle level	*/
			 (level == 3) ? FmtBotLevel () : /*	Detail level	*/
			       1 ;						/*	ERROR			*/
}

/*
 ************************************************************************
 * 	Function	:	GL_GetHhca ()
 ************************************************************************
 * 	Description	:	Return the glca_hash of the specified level.
 * 
 * 	Parameters	:	level	      - Level for which the hash should be returned.
 * 
 * 	Globals		:	GV_max_level  - Maximum allowable account level.
 * 					GV_acc_tab    - Structures holding account part information.
 * 
 * 	Returns		:	hhcaHash      - glca hash for specified level.
 */
long	
GL_GetHhca (
	int		level)
{
	if (level > GV_max_level || level < 0)
		return (0L);

	return (GV_acc_tab [level - 1].hhcaHash);
}

/*
 ************************************************************************
 * 	Function	:	GL_GetBit ()
 ************************************************************************
 * 	Description	:	Return the number part of the specified level.
 * 
 * 	Parameters	:	level	      - Level for which the part should be returned.
 * 
 * 	Globals		:	GV_max_level  - Maximum allowable account level.
 * 					GV_acc_tab    - Structures holding account part
 * 					        information.
 * 
 * 	Returns		:	accountBit	  - Part for specified level.
 */
char *
GL_GetBit (
	int		level)
{
	if (level > GV_max_level || level < 0)
		return ((char *) NULL);

	return (GV_acc_tab [level - 1].accountBit);
}
/*
 ************************************************************************
 * 	Function	:	GL_GetAccount ()
 ************************************************************************
 * 	Description	:	Return account record for the specified level.
 * 
 * 	Parameters	:	level	      - Level for which the data should be returned.
 * 
 * 	Globals		:	GV_max_level  - Maximum allowable account level.
 * 					GV_acc_tab    - Structures holding account part information.
 * 
 * 	Returns		:	accObj	      - Pointer to data for specified level.
 */
char *
GL_GetAccount (
	int		level)
{
	if (level > GV_max_level)
		return ((char *) &GV_acc_tab [GV_max_level - 1].accObj);

	if (level < 1)
		return ((char *) &GV_acc_tab [0].accObj);

	return ((char *) &GV_acc_tab [level - 1].accObj);
}

/*
 ************************************************************************
 * 	Function	:	GL_GetDesc ()
 ************************************************************************
 * 	Description	:	Return default account description.
 * 
 * Notes		:	Build default account description from the
 *					descriptions of the accounts above it and the
 *					glca description for the current level.
 * 
 * 	Parameters	:	level	- Level for which the description should be built.
 * 					workDesc- Pointer to string into which the
 * 							  description should be placed.
 * 					len		- Maximum length of default string.
 * 
 * 	Globals		:	GV_max_level  - Maximum allowable account level.
 * 					GV_acc_tab    - Structures holding account part information.
 */
void
GL_GetDesc (
	int 	level, 
	char 	*workDesc, 
	int		len)
{
	char	*t_ptr, tmp_str [81];
	int	desc_len, cnt;

	if (level < GV_max_level)
	{
		if (!FmtEmptyBit (level))
			sprintf (workDesc, "%-*.*s", len, len, ACC_OBJ.desc);
		else
			sprintf (workDesc, "%*.*s", len, len, " ");
		return ;
	}
	
	t_ptr = tmp_str;
	*t_ptr = (char) NULL;

	level = (GV_cur_level - 2 < 0) ? 0 : GV_cur_level - 2;

	for (cnt = 0; level < GV_max_level; cnt++, level++)
	{
		clip (ACC_OBJ.desc);
		desc_len = strlen (ACC_OBJ.desc);
		if (!FmtEmptyBit (level) && desc_len)
		{
			if ((int) (strlen (t_ptr) + (cnt ? 2 : 0) + desc_len) > len)
				*t_ptr = (char) NULL;
			
			else if (cnt)
				strcat (t_ptr, ", ");

			strcat (t_ptr, ACC_OBJ.desc);
		}
	}
	sprintf (workDesc, "%-*.*s", len, len, tmp_str);
}

/*
 ************************************************************************
 * 	Function	:	GL_GetAccountNo ()
 ************************************************************************
 * 	Description	:	Get account number for specified level.
 * 
 * 	Notes		:	Get_accno builds an account number for the
 * 					specified level in the current hierarchy using
 * 					information held in GV_acc_tab.
 * 			
 * 	Parameters	:	level	- Level for which the description should be built.
 * 							- Pointer to string into which the description 
 *							  should be placed.
 * 
 * 	Globals		:	GV_max_level  - Maximum allowable account level.
 * 					GV_acc_tab    - Structures holding account part information.
 * 	Returns		:
 */
char *
GL_GetAccountNo (
	int 	level, 
	char 	*workAccount)
{
	int	lvl_cnt;

	if (level > GV_max_level || level < 0)
		return ((char *) NULL);
	
	for (lvl_cnt = 0, *workAccount = (char) NULL; lvl_cnt < level; lvl_cnt++)
		strcat (workAccount, GV_acc_tab [lvl_cnt].accountBit);

	level = strlen (workAccount);
	if ((int) (strlen (workAccount)) < MAXLEVEL)
		sprintf (workAccount + level, "%-*.*s", MAXLEVEL - level,
					     MAXLEVEL - level, "0000000000000000");
	
	return (workAccount);
}
/*
 ************************************************************************
 * 	Function	:	GL_ValidUserCode ()
 ************************************************************************
 * 	Description :   Compare the prefix of the GL account code with the
 * 					stored user code
 * 	Parameters  :   accountNo Account Code
 * 	Globals     :   UserCode 	 (Stored when calling GL_SetAccWidth
 * 					accountant  (accountant user or not)
 * 	Return      :   TRUE or FALSE
 */
int
GL_ValidUserCode (
	char	*accountNo)
{
	if (accountant)
		return TRUE;
	return (!strncmp (accountNo, UserCode, strlen (UserCode)));
}
/*
 ************************************************************************
 * 	Function	:	GL_ValidfUserCode ()
 ************************************************************************
 * 	Description :   Compare the prefix of the GL account code with the
 * 					stored formatted user code
 * 	Parameters  :   accountNo Account Code
 * 	Globals     :   fUserCode (Stored when calling GL_SetAccWidth
 * 					accountant  (accountant user or not, stored when
 * 								 calling GL_SetAccWidth)
 * 	Return      :   TRUE or FALSE
 */
int
GL_ValidfUserCode (
	char *accountNo)
{
	if (accountant)
		return TRUE;
	return (!strncmp (accountNo, fUserCode, strlen (fUserCode)));
}

/*
 ************************************************************************
 * 	Function : GL_GetUserCode ()
 ************************************************************************
 * 	Description : return User Code.
 */
char *GL_GetUserCode (void)
{
	return (UserCode);
}

/*
 ************************************************************************
 * 	Function : GL_GetfUserCode ()
 ************************************************************************
 * 	Description : return formatted user code
 */
char *GL_GetfUserCode (void)
{
	return (fUserCode);
}
/*
 ************************************************************************
 * 	Function : GL_GetDfltSfaccCode ()
 ************************************************************************
 * 	Description : return default start account code
 */
char *GL_GetDfltSaccCode (void)
{
	return (dflt_sUserCode);
}

/*
 ************************************************************************
 * 	Function : GL_GetDfltSfaccCode ()
 ************************************************************************
 * 	Description : return default start formatted account code
 */
char *GL_GetDfltSfaccCode (void)
{
	return (dflt_sfUserCode);
}
	
/*
 ************************************************************************
 * 	Function : GL_GetDfltEaccCode ()
 ************************************************************************
 * 	Description : return default end account code
 */
char *GL_GetDfltEaccCode (void)
{
	return (dflt_eUserCode);
}

/*
 ************************************************************************
 * 	Function : GL_GetDfltEfaccCode ()
 ************************************************************************
 * 	Description : return default end formatted account code
 */
char *GL_GetDfltEfaccCode (void)
{
	return (dflt_efUserCode);
}

/*
 ************************************************************************
 * 	Function	:	FmtReadAcc ()
 ************************************************************************
 * 	Function	:	FmtReadAcc ()
 * 
 * 	Description	:	Read account and chart information.
 * 
 * 	Notes		:	This function reads the account and related
 * 					glca information for the specified level.
 * 
 * 	Parameters	:	t_flag	 - If TRUE read the glmr record.
 * 					level	 - Level of glca to read.
 * 					accountNo	 - Account number to read.
 * 					accObj	 - Pointer to object to hold account data.
 * 	
 * 	Returns		:	1	 - If call fails.
 * 					0	 - If call succeeds.
 */
static int
FmtReadAcc (
	int			t_flag,
	int			level,
	char		*accountNo,
	GLMR_STRUCT	*accObj)
{
	if (FmtEmptyBit (level) && (level + 1 < GV_cur_level))
	{
		print_err ("Invalid chart of account at level [%d]", level + 1);
		return (EXIT_FAILURE);
	}
	if (!FmtEmptyBit (level) && GV_cur_level > level)
	{
		if (ReadGlca (PV_co_no, level + 1, ACC_BIT, "r", &glcaRec))
			return (EXIT_FAILURE);

		ACC_GLCA = glcaRec.hhca_hash;
		strcpy (ACC_OBJ.desc, glcaRec.acc_desc);

		if (t_flag)
			return (FmtReadGlmr (level, accountNo, accObj));
		else
			return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * 	Function	:	FmtReadGlmr ()
 ************************************************************************
 * 	Description	:	Read account information.
 * 
 * 	Notes		:	This function reads the account master record.
 * 
 * 	Parameters	:	level	 - Level for validation.
 * 					accountNo	 - Account number to read.
 * 					accObj	 - Pointer to object to hold account data.
 * 	
 * 	Returns		:	1	 - If call fails.
 * 					0	 - If call succeeds.
 */
static int
FmtReadGlmr (
	int			level,
	char 		*accountNo,
	GLMR_STRUCT	*accObj)
{
	if (find_rec (glmr, (char *) accObj, COMPARISON, "r"))
	{
		if (level + 1 <= GV_max_level && !FmtEmptyBit (level + 1))
			return print_err ("Account %s not on file.", accountNo);
		else
			return (EXIT_SUCCESS);
	}
	pin_bcopy ((char *) &ACC_OBJ, (char *) accObj, sizeof (GLMR_STRUCT));
	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * 	Function	:	FmtBadWidth ()
 ************************************************************************
 * 	Description	:	Print bad width error message.
 * 
 * 	Parameters	:	level -	Level number of bad width.
 * 					s     - Pointer to message to print.
 * 	
 * 	Returns		:	1
 */
static int
FmtBadWidth (
	int		level,
	char	*s)
{
	return print_err ("%s invalid width for level %d.", s, level + 1);
}

/*
 ************************************************************************
 * 	Function	:	IsTopLevel ()
 ************************************************************************
 * 	Description	:	Validate account is top level.
 * 					Exactly the same as FmtTopLevel () but is accessible by
 * 					outside world and will not prompt user for error. So, 
 * 					program will check level queity.
 * 
 * 	Returns		:	TRUE	- If account is top level.
 * 					FALSE	- If account is not top level.
 */
int
IsTopLevel (void)
{
	int	level;

	if (FmtEmptyBit (0))
		return (FALSE);

	for (level = 1; level < GV_max_level; level++)
		if (!FmtEmptyBit (level))
			return (FALSE);

	return (TRUE);
}

/*
 ************************************************************************
 * 	Function	:	FmtTopLevel ()
 ************************************************************************
 * 	Description	:	Validate account is top level.
 * 
 * 	Returns		:	0	- If account is top level.
 * 					1	- If account is not top level.
 */
static int
FmtTopLevel (void)
{
	int	level;

	if (FmtEmptyBit (0))
		return (FmtBadLevel ("top"));

	for (level = 1; level < GV_max_level; level++)
		if (!FmtEmptyBit (level))
			return (FmtBadLevel ("top"));

	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * 	Function	:	GL_IsMidLevel ()
 ************************************************************************
 * 	Description	:	Validate account is mid level.
 * 					Exactly the same as FmtMidLevel () but is accessible by
 * 					outside world and will not prompt user for error. So, 
 * 					program will check level queity.
 * 
 * 	Returns		:	TRUE	- If account is middle level.
 * 					FALSE	- If account is not middle level.
 */
int
GL_IsMidLevel (void)
{
	if (!FmtEmptyBit (GV_max_level - 1))
		return (FALSE);

	return (TRUE);
}

/*
 ************************************************************************
 * 	Function	:	FmtMidLevel ()
 ************************************************************************
 * 	Description	:	Validate account is middle level.
 * 
 * 	Returns		:	0	- If account is middle level.
 * 					1	- If account is not middle level.
 */
static int
FmtMidLevel (void)
{
	if (!FmtEmptyBit (GV_max_level - 1))
		return (FmtBadLevel ("middle"));

	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * 	Function	:	GL_IsBotLevel ()
 ************************************************************************
 * 	Description	:	Validate account is bot level.
 * 					Exactly the same as FmtBotLevel () but is accessible by
 * 					outside world and will not prompt user for error. So, 
 * 					program will check level queity.
 * 
 * 	Returns		:	FALSE	- If account is posting level.
 * 					TRUE	- If account is not posting level.
 */
int
GL_IsBotLevel (void)
{
	if (FmtEmptyBit (GV_max_level - 1))
		return (FALSE);

	return (TRUE);
}

/*
 ************************************************************************
 * 	Function	:	FmtBotLevel ()
 ************************************************************************
 *	Description	:	Validate account is posting level.
 *
 *	Returns		:	0	- If account is posting level.
 *					1	- If account is not posting level.
 */
static int
FmtBotLevel (void)
{
	if (FmtEmptyBit (GV_max_level - 1))
		return (FmtBadLevel ("posting"));

	return (EXIT_SUCCESS);
}

/*
 ************************************************************************
 * 	Function	:	FmtEmptyBit ()
 ************************************************************************
 * 	Description	:	Returns TRUE if the specified level is 'empty'.
 * 
 * 	Parameters	:	l     - Level to check.
 * 	
 * 	Returns		:	TRUE  -	If level is empty.
 * 					FALSE -	If level is not empty.
 */
static int
FmtEmptyBit (
	int		l)
{
	char	*t_ptr;

	t_ptr = GV_acc_tab [l].accountBit;
	while (*t_ptr)
	{
		if (*t_ptr++ != '0')
			return (FALSE);
	}
	return (TRUE);
}
	
/*
 ************************************************************************
 * 	Function	:	FmtBadLevel ()
 ************************************************************************
 * 	Description	:	Print bad level error message.
 * 	Parameters	:	msg - Pointer to message to print.
 * 	Returns		:	1
 */
static int
FmtBadLevel (
	char	*msg)
{
	return print_err ("Must be a %s level account.", msg);
}

/*
 ************************************************************************
 * 	Function	:	FmtInitBits ()
 ************************************************************************
 * 	Description	:	Initialise account part table.
 */
static void
FmtInitBits (void)
{
	static	char	*initChars = "0000000000000000000000000000000";
	char	*i_ptr;
	int		len		= 0, 
			level	= 0;

	for (level = 0, i_ptr = initChars; level < GV_max_level; level++)
	{
		len = strlen (maskBits [level]);
		sprintf (ACC_BIT, "%-*.*s", len, len, i_ptr);
		i_ptr += (len + 1);
	}
}

/*
 ************************************************************************
 * 	Function	:	FmtFormChars ()
 ************************************************************************
 * 	Description	:	Return number of '-' characters in format.
 * 
 * 	Parameters	:	format	- Format to be scanned.
 * 				:	mask	- Print/Display mask to update.
 * 	
 * 	Returns		:	f_cnt - No of '-' chars.
 */
static	int
FmtFormChars (
	char	*format)
{
	int	formatCnt = 0;

	while (*format++)
		if (*format == '-')
			formatCnt++;

	return (formatCnt);
}
/*
 ************************************************************************
 * 	Function	:	GL_StripForm ()
 ************************************************************************
 * 	Description	:	Strip format characters out of an account code.
 * 
 * 	Parameters	:	s1 - Pointer to string to hold stripped code.
 * 					s2 - Pointer to string holding unstripped code.
 */
void
GL_StripForm (
	char *s1, 
	char *s2)
{
	while (*s2)
	{
		while (*s2 == '-')
			s2++;

		*s1++ = *s2++;
	}
	*s1 = '\0';
}

/*
 ******************************************************************************
 * 	Function	:	ReadGlca ()
 ******************************************************************************
 * 
 * 	Description	:	Read chart of accounts record.
 * 
 * 	Notes		:	Read General Ledger chart of accounts record
 * 					for the specified level of the current account number.
 * 			
 *	Parameters	:	companyNo	   	- Company number.
 * 					level	   		- Level of glca record to read.
 * 					accountBit    	- Account number.
 * 					glca_obj   		- Pointer to GLCA_STRUCT to hold data.
 * 
 * 	Globals		:	GV_acc_tab - Structures holding account part information.
 * 
 * 	Returns		:	1	   - If call fails.
 * 					0	   - If call succeeds.
 */
int
ReadGlca (
	char 		*companyNo, 
	int 		level, 
	char 		*accountBit, 
	char 		*rtype, 
	GLCA_STRUCT *glca_obj)
{
	strcpy (glca_obj->co_no, companyNo);
	glca_obj->level_no = level;
	glca_obj->acc_no = atol (accountBit);

	if (find_rec (glca, glca_obj, COMPARISON, rtype))
	{
		if (*rtype != 'u')
			return print_err ("%s doesn't exist for level %d.\007",
							accountBit, level);
		else
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
/*
 ******************************************************************************
 * 	Function	:	ReadGlmr ()
 ******************************************************************************
 * 
 * 	Description	:	Read General Ledger Account.  
 * 
 * 	Notes		:	
 * 			
 *	Parameters	:	companyNo	   	- Company number.
 * 					glmr_obj   		- Pointer to GLMR_STRUCT to hold data.
 * 					lockFlag    	- lock flag.
 * 					readType   		- Read type
 * 
 * 	Returns		:	1	   - If call fails.
 * 					0	   - If call succeeds.
 */
int
ReadGlmr (
	char 		*companyNo, 
	GLMR_STRUCT *glmr_obj,
	char 		*lockFlag,
	int 		readType)
{
	strcpy (glmr_obj->co_no, companyNo);
	return (find_rec (glmr, glmr_obj, readType, lockFlag));
}

/*
 ******************************************************************************
 * 	Function	:	GL_FormAccBit ()
 ******************************************************************************
 * 	Description	:	Format account part to specified width.
 * 
 * 	Notes		:	Used to format the separate parts of an
 * 				:	account number to a specified width.
 * 
 * 	Parameters	:	accountStr	- String to be formated.
 * 				:	width		- Formatted width of account string.
 * 
 * 	Returns		:	acc_str formatted and converted to a long integer.
 */

long 
GL_FormAccBit (
	char	*accountStr, 
	int 	width)
{
	long		atol (const char *);
	register	int	cnt;
	int		len;

	len = strlen (accountStr);
	if (len < width)
	{
		for (cnt = len; cnt < width; cnt++)
			accountStr [cnt] = '0';

		accountStr [cnt] = (char) NULL;
	}
	return (atol (accountStr));
}
