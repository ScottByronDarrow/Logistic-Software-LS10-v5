/*=====================================================================
|  Copyright (C) 1988 - 2001 Logistic Software Limited.               |
|=====================================================================|
| $Id: make_doco.c,v 5.2 2001/08/09 09:49:47 scott Exp $
|  Program Name  : (make_doco.c) 
|  Program Desc  : (Create Program Documentation from Program)
|                  (Source File (s)) 
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 31/07/89         |
|---------------------------------------------------------------------|
| $Log: make_doco.c,v $
| Revision 5.2  2001/08/09 09:49:47  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:31:22  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/10 04:41:50  scott
| Updated to fix problem with alias.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "make_doco.c",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/make_doco/make_doco.c,v 5.2 2001/08/09 09:49:47 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/
#include <pslscr.h>
#include <ml_misc_mess.h>

/*==================================
|   Constants, defines and stuff    |
 ==================================*/

/*=======================================
| Table action type definitions	        |
=======================================*/
/*  QUERY   
    renamed these to avoid conflicts with winnt.h
*/
#define	LSL_NONE    00000
#define	LSL_OPEN    00001
#define	LSL_FIND    00010
#define	LSL_ADD     00100
#define	LSL_UPDATE  00200
#define	LSL_DELETE  00400
#define	LSL_ALIAS   01000

#define	CheckAction(x,y)	 ((x & y) == y)
#define	DATA_SIZE	400
#define	NUM_FILES	1000

/*===================================
| Character validation definitions	|
===================================*/
#define	SpecialChar(x)	 (x == '(' || x == ' ' || x == '\t' || \
			 		  	  x == '"' || x == ',' || x == ')')

#define	ValidChar(x)	 (isalpha (x) || isdigit (x) || x == ' ' || \
			 			 x == '(' || x == ')' || x == '/' || x == '-')

/*=======================
|  BTREE definitions    |
=======================*/
/*--------------------------------------
| DATA typedef definition for btree     |
--------------------------------------*/
typedef	struct	
{
	char   *tabName;     /* name of daTabase Table		*/
	char   *tabAlias;    /* "real" name of daTabase Table	*/
	int    tabAction;    /* actions applied to Table		*/
} DATA;

/*-----------------------
|   BTREE definition    |
-----------------------*/
typedef struct _btree	
{
	DATA   *_data;               /* pointer to nodal data */
	struct	_btree	*_left;	     /* left child            */
	struct	_btree	*_right;     /* right child	          */
} BTREE;

#define	TNUL	 ((struct _btree *)0)
#define	TDATA	 (tptr->_data)
#define	TLEFT	 (tptr->_left)
#define	TRIGHT	 (tptr->_right)

/*----------------------
|   BTREE variables     |
 ----------------------*/
int     _offset = 0;    /* BTREE offset (??) */
BTREE   *tree_head;     /* BTREE head (duh)  */

/*=======================================
| "Quick" definitions                   |
=======================================*/
#define	B_SRCH		 (actions [i].search)
#define	ACT_TYPE	 (actions [i].act_type)
#define	ACTION		 (actions [i].action)

#define	TACT_TYPE	 (tabActions [i].act_type)
#define	TACT_DESC	 (tabActions [i].act_desc)

#define	LCL_TAB_NAME	 (tabStruct [i].tabName)
#define	LCL_TAB_DESC	 (tabStruct [i].tabDesc)

/*====================
|   Local variables   |
 ====================*/
#ifndef	LINUX
extern	int	errno;
#endif	// LINUX

FILE	*fin;
FILE	*fout;
FILE	*eout;

int     noAction 		= TRUE,
		SCN_FILE_USED 	= FALSE,
		PFORMAT_USED 	= FALSE,
		PR_FORMAT_USED	= FALSE,
		SRT_FILE_USED 	= FALSE,
		WRK_FILE_USED 	= FALSE,
		DSP_USED 		= FALSE,
		num_inc = 0,
		num_env = 0;

char	program 			[DATA_SIZE + 1],
		daTabase 			[DATA_SIZE + 1],
		description 		[DATA_SIZE + 1],
		dataIn 				[DATA_SIZE + 1],
		tabName 			[20],
		tabAlias 			[20],
		includeFiles 		[30][15],
		environmentVars 	[30][31];

/*=======================================
| Table Action Descriptions	            |
=======================================*/
struct	
{
	int    act_type;       /* action typedef       */
	char   *act_desc;      /* action description   */
} tabActions [] = 
    {
	   {LSL_OPEN,	"Open"},
	   {LSL_FIND,	"Find"},
	   {LSL_ADD,	"Add"},
	   {LSL_UPDATE,	"Update"},
	   {LSL_DELETE,	"Delete"},
	   {LSL_ALIAS,	"Alias"},
	   {0,			""}
    };

/*==============================
|   Local function prototypes   |
 ==============================*/
/*  QUERY
    these functions are needed by the data structure below.
    that is why they are here and not in the usual place.
*/

int		LCL_no_option 		(char *, int),
		ProgramDesc 		(char *, int),
		ProgramName 		(char *, int),
		DataBase 			(char *, int),
		AddTree 			(char *, int),
		MainProcessing 		(char *, int),
		ProcessComment 		(char *, int);

/*=======================================
| Table Action Definitions	             |
=======================================*/
struct	
{
	char   *search;        /* source string	   */
	int    act_type;       /* action typedef   */
	int    (* action) (char *, int);/* action for source string	*/
} actions [] = 
    {
        {"Program Desc",LSL_NONE,	ProgramDesc},/* Program Description		*/
        {"PNAME",		LSL_NONE,	ProgramName},/* Program Name			*/
        {"abc_dbopen",	LSL_NONE,	DataBase},	/* action on abc_dbopen ()	*/
        {"abc_alias",	LSL_ALIAS,	AddTree},	/* action on abc_alias ()	*/
        {"dbalias",		LSL_ALIAS,	AddTree},	/* action on abc_alias ()	*/
        {"abc_add",		LSL_ADD,	AddTree},	/* action on abc_add ()		*/
        {"dbadd",		LSL_ADD,	AddTree},	/* action on dbadd ()		*/
        {"abc_delete",	LSL_DELETE,	AddTree},	/* action on abc_delete ()	*/
        {"dbdelete",	LSL_DELETE,	AddTree},	/* action on dbdelete ()	*/
        {"abc_update",	LSL_UPDATE,	AddTree},	/* action on abc_update ()	*/
        {"dbupdate",	LSL_UPDATE,	AddTree},	/* action on dbupdate ()	*/
        {"find_hash",	LSL_FIND,	AddTree},	/* action on find_hash ()	*/
        {"find_rec",	LSL_FIND,	AddTree},	/* action on find_rec ()	*/
        {"dbfind",		LSL_FIND,	AddTree},	/* action on dbfind ()		*/
        {"open_rec",	LSL_OPEN,	AddTree},	/* action on open_rec ()	*/
        {"/*",			LSL_NONE,	ProcessComment},/* action start comment	*/
        {"//",			LSL_NONE,	ProcessComment},/* action start comment	*/
        {"main",		LSL_NONE,	MainProcessing},/* action on main ()	*/
        {"",			LSL_NONE,	LCL_no_option}
    };

/*==========================================
| Structure For Table Descriptions (sorted)|
==========================================*/
struct	
{
	char	tabName [20];
	char	tabDesc [101];
} tabStruct [NUM_FILES];


/*==============================
|   Local function prototypes   |
 ==============================*/

int     ReadFileNames 			(void);
int     OpenFiles 				(char *, char *);
int     CheckInclude 			(char *);
int     CheckEnviron 			(char *);
int     CheckFLags 				(char *);
int     Compare 				(BTREE *); /* btree comparison routine	*/
int     Initialise 				(BTREE *); /* btree initialisation routine	*/
int     AddTree 				(char *, int);
int     CheckTab 				(char *);
int     PrintNode 				(BTREE *);	/* btree print node routine	*/
void    DescPart 				(char *);
void    Process 				(char *, char *);
void    Output 					(void);
void    DocoString 				(char *);
void    InitBtree 				(void);
void    PrintTree 				(BTREE *, int (*) (BTREE *));
BTREE*  AddNode 				(BTREE *, int (*) (BTREE *), int (*) (BTREE *));
BTREE*  TreeAlloc 				(int (*) (BTREE *));

/*==============================
|   Main Processing Function    |
 ==============================*/
int
main (
 int   argc, 
 char *argv [])
{
	/*---------------------------------------
	| check parameter count	                |
	---------------------------------------*/
	if (argc != 3)
	{
		print_at (0,0,ML (mlMiscMess702),argv [0]);
		return (EXIT_FAILURE);
	}

	/*---------------------------------------
	| Load list of Logistic files into array
	---------------------------------------*/
	ReadFileNames ();

	/*---------------------------------------
	| check if in & output files are same	|
	---------------------------------------*/
	if (!strcmp (argv [1],argv [2]))
	{
		print_at (1,0,ML (mlMiscMess004));
		return  (EXIT_FAILURE);
	}
	/*---------------------------------------
	| perform initialisation                |
	---------------------------------------*/
	InitBtree ();
	strcpy (program,"");
	strcpy (description,"");
	strcpy (daTabase,"");
	/*---------------------------------------
	| process input file to output file     |
	---------------------------------------*/
	Process (argv [1],argv [2]);

	return (EXIT_SUCCESS);
}

int
ReadFileNames (
 void)
{
	char   *sptr;
	char   *tptr;
	char   tmp_char;
	char   filename [100];
	char   lineBuff [DATA_SIZE + 1];
	int    i = 0;

	sptr = getenv ("PROG_PATH");
	sprintf (filename, "%s/SCHEMA/PFILE_DESC", (sptr) ? sptr : "/usr/LS10.5");

	/*---------------------------------------
	|  open source file                    	|
	---------------------------------------*/
	fin = fopen (filename,"r");
	if (fin == 0)
	{
		sprintf (err_str,"Error in %s during (FOPEN)",filename);
		sys_err (err_str,errno,PNAME);
	}

	while ((sptr = fgets (lineBuff,DATA_SIZE,fin)))
	{
		/*------------------------------------
		| Lines starting with # are comments |
		------------------------------------*/
		if (*sptr == '#')
		{
			continue;
		}

		* (sptr + strlen (sptr) - 1) = '\0';

		/*------------------------
		| Separate name and desc |
		------------------------*/
		tptr = sptr;
		while (*tptr && *tptr != '\t')
		{
			tptr++;
		}

		tmp_char = *tptr;
		*tptr = '\0';

		sprintf (tabStruct [i].tabName, "%-10.10s", sptr + 4);

		if (tmp_char)
			sprintf (tabStruct [i++].tabDesc,"%-100.100s",tptr + 1);
		else
			sprintf (tabStruct [i++].tabDesc, "%-100.100s", " ");
	}
	return (EXIT_SUCCESS);
}

void
Process (
 char   *in_file, 
 char   *out_file)
{
	char	*data;
	/*---------------------------------------
	|  open files for input & output	     |
	---------------------------------------*/
	if (!OpenFiles (in_file,out_file))
    {
		return;
    }
	/*--------------------------------------
	| process source file                  |
	---------------------------------------*/
	data = fgets (dataIn,DATA_SIZE,fin);
	while (data != (char *)0)
	{
		if (!ProcessComment (data,LSL_NONE))
		{
			DocoString (data);
		}
		data = fgets (dataIn,DATA_SIZE,fin);
	}
	/*---------------------------------------
	| output to output file	                |
	---------------------------------------*/
	Output ();
	fclose (fin);
	fclose (fout);
	fclose (eout);
}

/*  QUERY
    this function originally did not return anything yet its use requires a
    return value of TRUE for success and FALSE for failure.
    the return values are based on the Program flow. 
*/
int
OpenFiles (
 char   *in_file, 
 char   *out_file)
{
	char	EnvFile [100];
	char	*sptr;

	/*--------------------------------------
	|  open source file                    |
	---------------------------------------*/
	fin = fopen (in_file,"r");
	if (fin == 0)
	{
		sprintf (err_str,"Error in %s during (FOPEN)",in_file);
		sys_err (err_str,errno,PNAME);
        return (FALSE);
	}
	/*--------------------------------------
	| open documentation file              |
	---------------------------------------*/
	fout = fopen (out_file,"w");
	if (fout == 0)
	{
		sprintf (err_str,"Error in %s during (FOPEN)",out_file);
		sys_err (err_str,errno,PNAME);
        return (FALSE);
	}

	sptr = getenv ("ENVOUT");
	sprintf (EnvFile, "%s", (sptr) ? sptr : "/usr/tmp/env.out");
	/*--------------------------------------
	| open documentation file              |
	---------------------------------------*/
	eout = fopen (EnvFile,"a");
	if (eout == 0)
	{
		file_err (errno, EnvFile, "fopen");
        return (FALSE);
	}

    return (TRUE);
}

void
Output (
 void)
{
	char   *sptr;
	int    i;

	/*--------------------------------------
	| set program name, description etc    |
	---------------------------------------*/
	if (!strlen (program))
	{
		strcpy (program," ");
	}
	if (!strlen (description))
	{
		strcpy (description," ");
	}
	if (!strlen (daTabase))
	{
		strcpy (daTabase," ");
	}
	/*--------------------------------------
	| print documentation file header      |
	---------------------------------------*/
	fprintf (fout,"=====================");
	fprintf (fout,"============");
	fprintf (fout,"==============================================\n");

	fprintf (fout,"| Program Name    : %-57.57s |\n",program);
	if (strlen (description) <= 57)
		fprintf (fout,"| Program Desc    : %-57.57s |\n",description);
	else
	{
		sptr = description + 57;
		while (sptr > description && *sptr == ' ')
		{
			sptr++;
		}
		if (*sptr == ' ')
		{
			*sptr = '\0';
			fprintf (fout,"| Program Desc    : %-57.57s |\n",description);
		}
		else
		{
			fprintf (fout,"| Program Desc    : %-57.57s |\n",description);
		}
	}
	fprintf (fout,"| Database        : %-57.57s |\n",daTabase);

	fprintf (fout,"|-----------");
	fprintf (fout,"------------");
	fprintf (fout,"------------------------------------------------------|\n");

	fprintf (fout,"| File Name ");
	fprintf (fout,"|  Action   ");
	fprintf (fout,"|                  F i l e   N a m e                  |\n");

	fprintf (fout,"|-----------");
	fprintf (fout,"|-----------");
	fprintf (fout,"|-----------------------------------------------------|\n");

	/*--------------------------------------
	| print Table details		            |
	---------------------------------------*/
	if (tree_head != TNUL)
	{
		PrintTree (tree_head,PrintNode);
	}
	
	/*---------------------------------------
	| print list of include files		|
	---------------------------------------*/
	fprintf (fout,"|--------------------------------------");
	fprintf (fout,"---------------------------------------|\n");
	fprintf (fout,"| Files Included                                                              |\n");
	fprintf (fout,"|--------------------------------------");
	fprintf (fout,"---------------------------------------|\n");
	for (i = 0; i < num_inc; i++)
	{
		fprintf (fout,
		         "|      <%-14.14s>                                                       |\n",
		         includeFiles [i]);
	}

	/*---------------------------------------
	| print list of environment variables	|
	---------------------------------------*/
	fprintf (fout,"|--------------------------------------");
	fprintf (fout,"---------------------------------------|\n");
	fprintf (fout,"| Environment Variables Used                                                  |\n");
	fprintf (fout,"|--------------------------------------");
	fprintf (fout,"---------------------------------------|\n");
	for (i = 0; i < num_env; i++)
	{
		fprintf (fout,
		         "|      <%-30.30s>                                       |\n",
		         environmentVars [i]);

		fprintf (eout, "%s|%-30.30s|\n", program, environmentVars [i]);

	}

	fprintf (fout,"|--------------------------------------");
	fprintf (fout,"---------------------------------------|\n");
	fprintf (fout,"| Misc Notes:                                                                 |\n");
	fprintf (fout,"|--------------------------------------");
	fprintf (fout,"---------------------------------------|\n");

	fprintf (fout,
	         "|        Screen file used      :  %-3.3s                                         |\n", 
	        (SCN_FILE_USED) ? "Yes" : "No ");
	fprintf (fout,
	         "|        pformat used          :  %-3.3s                                         |\n", 
	         (PFORMAT_USED) ? "Yes" : "No ");
	fprintf (fout,
	         "|        pr_format used        :  %-3.3s                                         |\n", 
	         (PR_FORMAT_USED) ? "Yes" : "No ");
	fprintf (fout,
	         "|        sort files used       :  %-3.3s                                         |\n", 
	         (SRT_FILE_USED) ? "Yes" : "No ");
	fprintf (fout,
	         "|        work files used       :  %-3.3s                                         |\n",
	         (WRK_FILE_USED) ? "Yes" : "No ");
	fprintf (fout,
	         "|        Display utils used    :  %-3.3s                                         |\n",
	         (DSP_USED) ? "Yes" : "No ");

	fprintf (fout,"=====================");
	fprintf (fout,"============");
	fprintf (fout,"==============================================\n");

}

int
ProcessComment (
 char   *data, 
 int    act_type)   /* unreferenced formal parameter */
{
	char   *sptr;
	/*--------------------------------------
	| ignore comments                       |
	---------------------------------------*/
	if (noAction)
	{
		return (FALSE);
	}
	/*--------------------------------------
	| check for comment start               |
	---------------------------------------*/
	sptr = strstr (data, "/*");
	if (sptr != (char *)0)
	{
		/*--------------------------------------
		| process string prior to comment       |
		---------------------------------------*/
		if (sptr > data)
		{
			* (sptr - 1) = '\0';
			DocoString (data);
		}
		/*--------------------------------------
		| check for comment end if              |
		---------------------------------------*/
		sptr = strstr (data, "*/");
		while (sptr == (char *)0)
		{
			data = fgets (dataIn,DATA_SIZE,fin);
		
			if (data != (char *)0)
			{
				return (TRUE);
			}

			sptr = strstr (data, "*/");
		}
		/*--------------------------------------
		| process string after comment          |
		---------------------------------------*/
		DocoString (sptr);
		return (TRUE);
	}
	return (FALSE);
}

void
DocoString (
 char   *data)
{
	int    i;
	char   *sptr;

	CheckInclude (data);
	CheckEnviron (data);
	CheckFLags (data);

	/*---------------------------------------
	| check data string against patterns	|
	---------------------------------------*/
	for (i = 0; strlen (B_SRCH); i++)
	{
		sptr = strstr (data, B_SRCH);
		/*--------------------------------------
		| pattern matches                       |
		---------------------------------------*/
		if (sptr != (char *)0)
		{
			 (* ACTION) (sptr + strlen (B_SRCH),ACT_TYPE);
		}

	}
}


int
CheckInclude (
 char   *data)
{
	char   *sptr;
	char   *tptr;

	sptr = strstr (data, "#include");
	if (sptr)
	{
		tptr = sptr;
		while (*tptr && 
		       *tptr != '<')
		{
			tptr++;
		}

		if (*tptr)
		{
			sptr = tptr + 1;
			tptr = sptr;
			while (*tptr && 
			       *tptr != '>')
			{
				tptr++;
			}

			*tptr = '\0';

			sprintf (includeFiles [num_inc++], "%-14.14s", sptr);
		}
	}
	return (EXIT_SUCCESS);
}

int
CheckEnviron (
 char *data)
{
	char   *sptr;
	char   *tptr;

	sptr = strstr (data, "getenv");
	if (!sptr)
	{
		sptr = strstr (data, "get_env");
	}

	if (!sptr)
	{
		sptr = strstr (data, "chk_env");
	}

	if (!sptr)
	{
		sptr = strstr (data, "chk_vble");
	}

	if (sptr)
	{
		tptr = sptr;
		while (*tptr && 
		       *tptr != '"')
		{
			tptr++;
		}

		if (*tptr)
		{
			sptr = tptr + 1;
			tptr = sptr;
			while (*tptr && 
			       *tptr != '"')
			{
				tptr++;
			}

			*tptr = '\0';

			sprintf (environmentVars [num_env++], "%-30.30s", sptr);
		}
	}
	return (EXIT_SUCCESS);
}

int
CheckFLags (
 char   *data)
{
	char	*sptr;

	if (!SCN_FILE_USED)
	{
		sptr = strstr (data, "_set_masks");
		if (sptr)
		{
			SCN_FILE_USED = TRUE;
		}
	}

	if (!PFORMAT_USED)
	{
		sptr = strstr (data, "pformat");
		if (sptr)
		{
			PFORMAT_USED = TRUE;
		}
	}

	if (!PR_FORMAT_USED)
	{
		sptr = strstr (data, "pr_format");
		if (sptr)
		{
			PR_FORMAT_USED = TRUE;
		}
	}

	if (!SRT_FILE_USED)
	{
		sptr = strstr (data, "sort_open");
		if (!sptr)
		{
			sptr = strstr (data, "sort_read");
		}

		if (!sptr)
		{
			sptr = strstr (data, "sort_sort");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "sort_delete");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "sort_save");
		}
	
		if (sptr)
		{
			SRT_FILE_USED = TRUE;
		}
	}

	if (!WRK_FILE_USED)
	{
		sptr = strstr (data, "RF_ADD");
		if (!sptr)
		{
			sptr = strstr (data, "RF_CLOSE");
		}

		if (!sptr)
		{
			sptr = strstr (data, "RF_DELETE");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "RF_DISPLAY");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "RF_OPEN");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "RF_READ");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "RF_REWIND");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "wkrm");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "RF_SEEK");
		}
	
		if (!sptr)
		{
			sptr = strstr (data, "RF_UPDATE");
		}
	
		if (sptr)
		{
			WRK_FILE_USED = TRUE;
		}
	}

	if (!DSP_USED)
	{
		sptr = strstr (data, "Dsp");
		if (sptr)
		{
			DSP_USED = TRUE;
		}
	}

	return (EXIT_SUCCESS);
}

int
LCL_no_option (
 char   *data,      /* unreferenced formal parameter */
 int    act_type)   /* unreferenced formal parameter */
{
    /* QUERY
     * this function is empty
     */

    return (EXIT_SUCCESS); 
}

int
MainProcessing (
 char   *data,      /* unreferenced formal parameter */
 int    act_type)   /* unreferenced formal parameter */
{
	noAction = FALSE;
    return (EXIT_SUCCESS);
}


int 
DataBase (
 char   *data, 
 int    act_type)   /* unreferenced formal parameter */
{
	static	int	called;
	char	*sptr = data;
	char	*tptr;
	/*---------------------------------------
	| ignore until valid                    |
	---------------------------------------*/
	if (noAction)
	{
		return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| ignore if already called              |
	---------------------------------------*/
	if (called)
	{
		return (EXIT_SUCCESS);
	}
	called = TRUE;
	/*---------------------------------------
	| find start of program name            |
	---------------------------------------*/
	while (*sptr && !isalpha (*sptr))
	{
		sptr++;
	}
	tptr = sptr;
	/*---------------------------------------
	| find end of program name              |
	---------------------------------------*/
	while (*sptr && isalpha (*sptr))
	{
		sptr++;
	}
	if (*sptr)
	{
		*sptr = '\0';
	}
	strcpy (daTabase,tptr);

	return (EXIT_SUCCESS); /* QUERY */
}

int
ProgramName (
 char   *data, 
 int    act_type)   /* unreferenced formal parameter */
{
	static	int	called;
	char	*sptr = data;
	char	*tptr;
	/*---------------------------------------
	| ignore if already called              |
	---------------------------------------*/
	if (called)
	{
		return (EXIT_SUCCESS);
	}
	called = TRUE;
	/*---------------------------------------
	| find start of program name            |
	---------------------------------------*/
	while (*sptr && !isalpha (*sptr))
	{
		sptr++;
	}
	tptr = sptr;
	/*---------------------------------------
	| find end of program name              |
	---------------------------------------*/
	while (*sptr && *sptr != '"')
	{
		sptr++;
	}
	if (*sptr)
	{
		*sptr = '\0';
	}
	strcpy (program,tptr);

	return (EXIT_SUCCESS);
}


/* QUERY
 * originally this did not return anything. however the prototype
 *    requires that it return an integer. so, we assumed the 
 *    return values based on the program flow.
 */
int
ProgramDesc (
 char   *data, 
 int    act_type)   /* unreferenced formal parameter */
{
	static	int	called;

	/*---------------------------------------
	| ignore if already called	             |
	---------------------------------------*/
	if (called)
	{
		return (EXIT_SUCCESS); /* QUERY assumed return value */
	}
	called = TRUE;
	/*---------------------------------------
	| find start of description		|
	---------------------------------------*/
	DescPart (data);
	data = fgets (dataIn,DATA_SIZE,fin);
	if (data == (char *)0)
	{
		return (EXIT_FAILURE); /* QUERY assumed return value */
	}

	DescPart (data);
	return (EXIT_SUCCESS);  /* QUERY assumed return value */
}

void
DescPart (
 char   *data)
{
	int    in_bracket = FALSE;
	char   *sptr = data;
	char   *tptr;
	/*---------------------------------------
	| find start of description             |
	---------------------------------------*/
	while (*sptr && !isalpha (*sptr))
	{
		sptr++;
	}
	/*---------------------------------------
	| find end of description	            |
	---------------------------------------*/
	if (*sptr)
	{
		tptr = sptr;

		while (*sptr && ValidChar (*sptr))
		{
			if (*sptr == ')')
			{
				if (in_bracket)
				{
					in_bracket = FALSE;
				}
				else
				{
					break;
				}
			}

			if (*sptr == '(')
			{
				in_bracket = TRUE;
			}
			sptr++;
		}
		*sptr = '\0';
		tptr = clip (tptr);
		sptr = strcat (description,tptr);
		sptr = strcat (description," ");
	}
}

int
Compare (
 BTREE  *tptr)
{
	return (strcmp (tabName,TDATA->tabName));
}

int
Initialise (
 BTREE *tptr)
{
	TDATA->tabName 		= p_strsave (tabName);
	TDATA->tabAlias 	= p_strsave (tabAlias);
	TDATA->tabAction 	= LSL_NONE;

    return (EXIT_SUCCESS);
}


/* QUERY
 * the original code contained return (FALSE); and return; (ie no return value)
 *      that is sloppy coding! 
 * 
 * the prototype requires that this return an integer. i am replacing
 *      them with return (EXIT_SUCCESS) etc. based on program flow.
 */
int
AddTree (
 char   *data, 
 int    tabAct)
{
	char	*sptr = data;
	char	*lptr;
	BTREE	*tptr;

	/*---------------------------------------
	| ignore until valid                    |
	---------------------------------------*/
	if (noAction)
	{
		return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| isolate Table name                    |
	---------------------------------------*/
	while (*sptr && SpecialChar (*sptr))
	{
		sptr++;
	}

	if (!*sptr)
	{
		return (EXIT_SUCCESS);
	}
	lptr = sptr;

	while (*sptr && !SpecialChar (*sptr))
	{
		sptr++;
	}

	*sptr = '\0';
	strcpy (tabName,lptr);
	/*---------------------------------------
	| isolate "real" Table name		        |
	---------------------------------------*/
	if (tabAct == LSL_ALIAS)
	{
		sptr++;
		while (*sptr && SpecialChar (*sptr))
		{
			sptr++;
		}

		if (!*sptr)
		{
			return (EXIT_SUCCESS);
		}

		lptr = sptr;
		while (*sptr && !SpecialChar (*sptr))
		{
			sptr++;
		}

		*sptr = '\0';
		strcpy (tabAlias, lptr);
	}
	else
	{
		strcpy (tabAlias, tabName);
	}

	if (strlen (tabAlias) != 4 && strlen (tabAlias) != 5 && !CheckTab (tabAlias))
	{
		return (EXIT_SUCCESS);
	}


	/*---------------------------------------
	| attempt to add node to tree           |
	---------------------------------------*/
	tptr = AddNode (tree_head,Compare,Initialise);
	if (tptr != TNUL)
	{
		if (tree_head == TNUL)
		{
			tree_head = tptr;
		}
		TDATA->tabAction |= tabAct;
	}
	/*-----------------------------------
	| apply action to "real" Table also	|
	-----------------------------------*/
	if (tabAct != LSL_ALIAS && CheckAction (TDATA->tabAction,LSL_ALIAS))
	{
		strcpy (tabName,TDATA->tabAlias);
		/*---------------------------------------
		| attempt to add node to tree           |
		---------------------------------------*/
		tptr = AddNode (tree_head,Compare,Initialise);
		if (tptr != TNUL)
		{
			if (tree_head == TNUL)
			{
				tree_head = tptr;
			}
			TDATA->tabAction |= tabAct;
		}
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------
| Check to see if tableName is |
| a valid Table from PFILE_DESC |
-------------------------------*/
int
CheckTab (
 char   *tableName)
{
	int	j;

	for (j = 0; j < NUM_FILES; j++)
	{
		if (!strcmp (tabStruct [j].tabName, tableName))
		{
			return (TRUE);
		}
	}

	return (FALSE);
}

int
PrintNode (
 BTREE  *tptr)
{
	static  int last;
	int i;
	int cnt;
	int cmp;
	
	/*---------------------------------------
	| ignore alias entries                  |
	---------------------------------------*/
	if (CheckAction (TDATA->tabAction,LSL_ALIAS))
	{
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| print Table name & actions            |
	---------------------------------------*/
	fprintf (fout,"| %-10.10s",TDATA->tabName);
	fprintf (fout,"| ");
	/*---------------------------------------
	| print actions                         |
	---------------------------------------*/
	for (i = 0,cnt = 0;tabActions [i].act_type > 0;i++)
	{
		/*---------------------------------------
		| action applies to current Table       |
		---------------------------------------*/
		if (CheckAction (TDATA->tabAction,TACT_TYPE))
		{
			if (cnt != 0)
			{
				fprintf (fout,"-%-1.1s",TACT_DESC);
				cnt += 2;
			}
			else
			{
				fprintf (fout,"%-1.1s",TACT_DESC);
				cnt++;
			}
		}
	}
	fprintf (fout,"%-*.*s|",10 - cnt,10 - cnt," ");
	/*---------------------------------------
	| get & print Table description         |
	---------------------------------------*/
	for (i = last;strlen (LCL_TAB_NAME);i++)
	{
		/*---------------------------------------
		| perform comparison                    |
		---------------------------------------*/
		cmp = strncmp (LCL_TAB_NAME,TDATA->tabName,4);
		if (cmp > 0)
			break;

		/*---------------------------------------
		| exact match                           |
		---------------------------------------*/
		if (cmp == 0)
		{
			last = i;
			strcpy (err_str, LCL_TAB_DESC);
			fprintf (fout,"%-53.53s|\n",err_str);
			return (EXIT_SUCCESS);;
		}
	}
	fprintf (fout,"%53.53s|\n"," ");

    return (EXIT_SUCCESS);
}


/*=====================
|   BTREE functions   |
=====================*/

/*--------------------------
| Initialise btree			|
 --------------------------*/
void
InitBtree (
 void)
{
	tree_head = TNUL;
}

/*--------------------------
| Print btree				|
 --------------------------*/
void
PrintTree (
 BTREE *tptr, 
 int (*node) (BTREE *))
{
	_offset++;
	/*-------------------------------
	| Search Left Subtree First		|
	-------------------------------*/
	if (TLEFT != TNUL)
	{
		PrintTree (TLEFT,node);
	}
	/*---------------------------
	| Print Node				|
	---------------------------*/
	 (*node) (tptr);
	/*-----------------------------
	| Search Right Subtree         |
	-------------------------------*/
	if (TRIGHT != TNUL)
	{
		PrintTree (TRIGHT,node);
	}
	_offset--;
}
/*=======================================
| Recursively search tree from tptr     |
| if tree is empty add node at root.    |
| else add node appropriately.          |
=======================================*/
BTREE*
AddNode (
 BTREE *tptr, 
 int (*comp) (BTREE *), 
 int (*init) (BTREE *))
{
	int    i;
	BTREE	*xptr;
	/*---------------------------------------
	| Tree is Null	                        |
	---------------------------------------*/
	if (tptr == TNUL)
	{
		xptr = TreeAlloc (init);
		errno = (xptr == TNUL) ? 12 : 0;
		return (xptr);
	}
	/*---------------------------------------
	| Use Comparison Routine.               |
	---------------------------------------*/
	i = (*comp) (tptr);
	/*---------------------------------------
	| Exact Match                           |
	---------------------------------------*/
	if (i == 0)
	{
		return (tptr);
	}
	/*---------------------------------------
	| Check Left Subtree                    |
	---------------------------------------*/
	if (i < 0)
	{
		/*---------------------------------------
		| At edge of Tree.                     |
		---------------------------------------*/
		if (TLEFT == TNUL)
		{
			TLEFT = TreeAlloc (init);
			errno = (TLEFT == TNUL) ? 12 : 0;
			return (TLEFT);
		}
		/*---------------------------------------
		| Continue Recursing                   |
		---------------------------------------*/
		return (AddNode (TLEFT,comp,init));
	}
	else
	{
		if (TRIGHT == TNUL)
		{
			TRIGHT = TreeAlloc (init);
			errno = (TRIGHT == TNUL) ? 12 : 0;
			return (TRIGHT);
		}
		return (AddNode (TRIGHT,comp,init));
	}
}
/*---------------------------------------
| Allocate tree node an initialise it	|
---------------------------------------*/
BTREE*
TreeAlloc (
 int (*init) (BTREE *))
{
	BTREE	*tptr;
	/*---------------------------------------
	| allocate node                         |
	---------------------------------------*/
	tptr = (BTREE *) malloc ((unsigned) sizeof (BTREE));
	if (tptr != TNUL)
	{
		/*---------------------------------------
		| allocate data section                 |
		---------------------------------------*/
		TDATA = (DATA *) malloc ((unsigned) sizeof (DATA));
		if (TDATA == (DATA *)0)
		{
			return (TNUL);
		}
		/*---------------------------------------
		| initialise tree node                  |
		---------------------------------------*/
		 (*init) (tptr);
		TLEFT = TNUL;
		TRIGHT = TNUL;
	}
	return (tptr);
}

/* [end of file] */
