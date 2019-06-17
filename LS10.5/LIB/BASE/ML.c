/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ML.c,v 5.0 2001/06/19 06:59:13 cha Exp $
|  Program Name  : ( ML.c                           )                 |
|  Program Desc  : ( Multi Lingual Routine.                         ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 01/09/1997       |
|---------------------------------------------------------------------|
| $Log: ML.c,v $
| Revision 5.0  2001/06/19 06:59:13  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.5  2001/02/07 07:43:23  scott
| Updated to add strdup as per LS10-GUI
|
| Revision 3.4  2001/02/02 03:06:18  scott
| Changed SYS_LANG_ADD environment to chk_env
|
| Revision 3.3  2001/02/02 03:05:22  scott
| Updated to go back, you guessed it to option one.
| Added SYS_LANG_ADD environment
|
| Revision 3.1  2001/01/24 02:11:27  scott
| Updated to fix problem with translated strings being clipped to origional
| string length. This caused format characters (e.g. "%s") to be chopped
| causing runtime core dumps.
| Solutions are
| (1) return origional string if translated string > origional string OR
| (2) return translated sting with new length if new length > origional string
|     length. Otherwise return translated string with length = origional string.
| At this stage I have selected option (1) for better or worse.
|
| Revision 3.0  2000/10/12 13:34:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:17:12  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.6  2000/06/26 00:18:12  scott
| Updated to make ML routine use unix environment instead of LS10
|
| Revision 1.5  1999/12/22 05:09:25  scott
| Updated to make SYS_LANG a Logistic environment rather than system one.
|
| Revision 1.4  1999/09/13 09:36:28  scott
| Updated for Copyright
|
|                :                                                    |
=====================================================================*/
#include	<std_decs.h>

extern	int	cc;

	 /*====================================
	 | Screen generator field label text. |
	 ====================================*/
#define	MLDB_NO_FIELDS	9

	static	struct dbview	mldb_list [MLDB_NO_FIELDS] =
	{
		{"mldb_lu_prmpt"},
		{"mldb_text1"},
		{"mldb_text2"},
		{"mldb_text3"},
		{"mldb_text4"},
		{"mldb_text5"},
		{"mldb_pname"},
		{"mldb_org_len"},
		{"mldb_hide"}
	};

	struct mldbRecord
	{
		char	lu_prmpt [121];
		char	text[5] [131];
		char	pname[21];
		int		org_len;
		int		hide;
	};

	extern	int		SYS_LANG;

	/*---------------------------------------------------------
	| Defines multi-lingual Clip function and Find Functions. |
	---------------------------------------------------------*/
	char	*ML_Clip (char	*),
			*ML_Find (char	*);

	void	ML_Open (void);

	extern	char	*PNAME;

	char	ml_WorkString [512];
	int		ml_DbaseOpen		=	FALSE,
			ml_Selected			=	FALSE,
			MlHide				=	0,
			envVarSysLangAdd 	= 	FALSE;

/*====================================
| Multi Lingual convershion routine. |
====================================*/
char *
ML (
	char	*string)
{
	char	*ml_WrkStr;

	if (!ml_Selected)
		SYS_LANG	=	lang_select();

	if (!SYS_LANG)
		return (strdup (string));

	ml_WrkStr	=	string;

	if (ml_DbaseOpen == FALSE)
		ML_Open ();
	
	return (ML_Find (ml_WrkStr));
}


/*============================================
| Open Database and read Multi Lingual file. |
============================================*/
void
ML_Open (void)
{
	abc_dbopen ("data");
	open_rec ("mldb", mldb_list, MLDB_NO_FIELDS, "mldb_lu_prmpt");
	ml_DbaseOpen	=	TRUE;
}

/*======================================
| Find Multi Lingual database record . |
======================================*/
char	*
ML_Find (
	char	*mlString)
{
	int		orgLength	=	(int) strlen (mlString),
			newLength	=	0,
			strLength	=	0,
			validString	=	FALSE,
			i			=	0;

	struct	mldbRecord	mldbRec;

	memset (&mldbRec, 0, sizeof (mldbRec)); 

	sprintf (mldbRec.lu_prmpt, "%-120.120s", ML_Clip (mlString));
	cc = find_rec ("mldb", &mldbRec, COMPARISON, "r");
	if (cc)
	{
		sprintf (mldbRec.text [SYS_LANG - 1], "%-130.130s", mlString);
		sprintf (mldbRec.text [0], "%-130.130s", mlString);

		strcpy (ml_WorkString, mldbRec.text [0]);
		for (i = 0; i < (int) strlen (ml_WorkString); i++)
		{
			if (ml_WorkString[i] >= 'A' && ml_WorkString[i] <= 'z')
			{
				validString	=	TRUE;
				break;
			}
		}
		if (strlen (clip (ml_WorkString)) && validString)
		{
			if (envVarSysLangAdd)
			{
				sprintf (mldbRec.pname, "%-20.20s", PNAME);
				mldbRec.org_len = orgLength;
			
				mldbRec.hide = (MlHide) ? 1 : 0;
	
				cc = abc_add ("mldb", &mldbRec);
				if (cc)
					file_err (cc, "mldb", "DBADD");
			}
		}
		strcpy (ml_WorkString, mlString);
		return (ml_WorkString);
	}
	strcpy (ml_WorkString, mldbRec.text [SYS_LANG -1]);
	if (!strlen (clip (ml_WorkString)))
	{
		strcpy (ml_WorkString, mlString);
		return (ml_WorkString);
	}
	newLength =	(int) strlen (ml_WorkString);
	strLength = (orgLength > newLength) ? orgLength : newLength;
	sprintf 
	(
		ml_WorkString, 
		"%*.*s", 
		strLength, 
		strLength, 
		mldbRec.text [SYS_LANG -1]
	);
	return (ml_WorkString);
}


/*==========================
| Select Language routine. |
==========================*/
int
lang_select(void)
{
	char	*sptr;
	int		envVarSysLang;

	sptr = getenv ("SYS_LANG");
	envVarSysLang = (sptr == (char *)0) ? 0 : atoi (sptr);
	if (envVarSysLang < 0 || envVarSysLang > 5)
		envVarSysLang = 0;

	sptr = chk_env ("SYS_LANG_ADD");
	envVarSysLangAdd = (sptr == (char *)0) ? 0 : atoi(sptr);

	ml_Selected	=	TRUE;

	return (envVarSysLang);
}

/*======================================
| Special Multi Lingual clip function. |
======================================*/
char	*
ML_Clip (
	char	*ml_ClipString)
{
	static char	tmpBuf [512];
	char *	sptr;
	char *	tptr;

	memset (tmpBuf, '\0', sizeof (tmpBuf));

	tptr = tmpBuf;
	sptr = ml_ClipString;
	while (*sptr)
	{
		if (*sptr == ' ' ||
			*sptr == ':' ||
			*sptr == '.')
		{
			sptr++;
			continue;
		}

		*tptr++ = toupper (*sptr++);
	}
	return (tmpBuf);
}
