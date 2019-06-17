/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pformat.c,v 5.4 2002/08/14 06:54:06 scott Exp $
|  Program Name  : (psl_format.c     )                                |
|  Program Desc  : (Print Formatter.                            )     |
|---------------------------------------------------------------------|
|  Date Written  : 15/07/89        |  Author     : Rog Gibbison       |
|---------------------------------------------------------------------|
| $Log: pformat.c,v $
| Revision 5.4  2002/08/14 06:54:06  scott
| .
|
| Revision 5.3  2001/09/14 02:00:05  cha
| Updated to default interpretCaret to false.
|
| Revision 5.2  2001/08/09 09:27:09  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:58:49  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:23:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:44:08  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/22 00:53:33  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pformat.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/pformat/pformat.c,v 5.4 2002/08/14 06:54:06 scott Exp $";

#include	<pslscr.h>
#include	<ml_utils_mess.h>
#include	<stdio.h>
#include	<std_decs.h>
#include	<signal.h>
#include 	<errno.h>

int		AltDevice 			(char *),
		AltSpooler 			(char *),
		BlankLines 			(char *),
		CaretOn 			(char *),
		CaretOff 			(char *),
		CentreText 			(char *),
		CurrentFooter 		(char *),
		DefaultAux 			(char *),
		DefaultFooter 		(char *),
		DefaultHeader 		(char *),
		ExpendCentre 		(char *),
		EndFormat 			(char *),
		OutputDevice 		(char *),
		PrintLine 			(char *),
		MultipleCopies 		(char *),
		JobDesc 			(char *),
		NoFormFeed 			(char *),
		NoHeading 			(char *),
		NoRestart 			(char *),
		PageBreak 			(char *),
		DefineSection 		(char *),
		PageDefine 			(char *),
		FooterPrint 		(char *),
		TenPitch 			(char *),
		TwelvePitch 		(char *),
		SixteenPitch 		(char *),
		FormLength 			(char *),
		NewPage 			(char *),
		PageNumber 			(char *),
		PageingOn 			(char *),
		FormWidth 			(char *),
		MergeFile 			(char *),
		Comment 			(char *),
		SpoolerOnly 		(char *),
		StartWorking 		(char *),
		ExpandText 			(char *),
		RuleOff 			(char *),
		VerticalTab 		(char *),
		PageDesc 			(char *); 

typedef	struct	list_rec	{
	char	*_data;
	struct	list_rec	*_next;
} LIST;

struct	{
	char	*_cmd;			/* dot command lead in		*/
	int		_size;			/* # lines affected		*/
	int		(* _fn) (char *);		/* function to execute		*/
} commands [] =
{
	{"AD",			 0,	AltDevice},		/* alternate device				*/
	{"ALTERNATE",	 0,	AltDevice},		/* alternate device				*/
	{"AS",			 0,	AltSpooler},	/* alternate spooler			*/
	{"BLANK",		-1,	BlankLines},	/* blank lines					*/
	{"B",			-1,	BlankLines},	/* blank lines					*/
	{"CARET_ON",	 0,	CaretOn},		/* caret on						*/
	{"CARET_OFF",	 0,	CaretOff},		/* caret off					*/
	{"CENTRE",		 1,	CentreText},	/* centre						*/
	{"CF",			 0,	CurrentFooter},	/* set current footer			*/
	{"COMMENT",		 0,	Comment},		/* Comment line					*/
	{"COPY",		 0,	MultipleCopies},/* number of copies				*/
	{"CURRENT",		 0,	CurrentFooter},	/* set current footer			*/
	{"C",			 1,	CentreText},	/* centre						*/
	{"DA",			 0,	DefaultAux},	/* define auxillary footer		*/
	{"DEF_AUX",		 0,	DefaultAux},	/* define auxillary footer		*/
	{"DEF_FOOT",	 0,	DefaultFooter},	/* define standard footer		*/
	{"DEF_HEAD",	 0,	DefaultHeader},	/* define header				*/
	{"DEF_SECT",	 0,	DefineSection},	/* section heading				*/
	{"DF",			 0,	DefaultFooter},	/* define standard footer		*/
	{"DH",			 0,	DefaultHeader},	/* define header				*/
	{"DS",			 0,	DefineSection},	/* section heading				*/
	{"EOF",			 0,	EndFormat},		/* end of data					*/
	{"EXPAND",		 1,	ExpandText},	/* expand						*/
	{"EXP_CENTRE",	 1,	ExpendCentre},	/* expand & centre				*/
	{"E",			 1,	ExpendCentre},	/* expand & centre				*/
	{"JD",			 0,	JobDesc},		/* set job description			*/
	{"LENGTH",		 0,	FormLength},	/* set form length				*/
	{"LINE",		 1,	PrintLine},		/* print line					*/
	{"LP",			 0,	OutputDevice},	/* output device				*/
	{"LRP",			 0,	NewPage},		/* page break if required		*/
	{"L",			 0,	FormWidth},		/* set form width				*/
	{"MERGE",		 0,	MergeFile},		/* merge text file				*/
	{"ME",			 0,	MergeFile},		/* merge text file				*/
	{"NC",			 0,	MultipleCopies},/* number of copies				*/
	{"NF",			 0,	NoFormFeed},	/* no formFeed on page throw	*/
	{"NO_FF",		 0,	NoFormFeed},	/* no formFeed on page throw	*/
	{"NO_HEAD",		 0,	NoHeading},		/* turn default heading off		*/
	{"NR",			 0,	NoRestart},		/* don't kick  spooler			*/
	{"OP",			 0,	NoHeading},		/* turn default heading off		*/
	{"PAGE_BREAK",	 0,	PageBreak},		/* page break					*/
	{"PAGE_CHAR",	 0,	PageDefine},	/* define page number character	*/
	{"PAGE_NO",		 0,	PageNumber},	/* set page number				*/
	{"PA",			 0,	PageBreak},		/* page break					*/
	{"PC",			 0,	PageDefine},	/* define page number character	*/
	{"PD",			 0,	PageDesc},		/* page description				*/
	{"PG",			 0,	PageNumber},	/* set page number				*/
	{"PF",			 0,	FooterPrint},	/* print foot after advancing	*/
	{"PN",			 0,	PageingOn},		/* turn default heading on		*/
	{"PITCH10",		 0,	TenPitch},		/* change pitch					*/
	{"PITCH12",		 0,	TwelvePitch},	/* change pitch					*/
	{"PITCH16",		 0,	SixteenPitch},	/* change pitch					*/
	{"PI10",		 0,	TenPitch},		/* change pitch					*/
	{"PI12",		 0,	TwelvePitch},	/* change pitch					*/
	{"PI16",		 0,	SixteenPitch},	/* change pitch					*/
	{"PL",			 0,	FormLength},	/* set form length				*/
	{"PN",			 0,	PageNumber},	/* set page number				*/
	{"PRINTER",		 0,	OutputDevice},	/* output device				*/
	{"PRINT_FOOT",	 0,	FooterPrint},	/* print foot after advancing	*/
	{"RE",			 0,	Comment},		/* Comment line					*/
	{"RULER",		 0,	RuleOff},		/* rule off action				*/
	{"R",			 0,	RuleOff},		/* rule off action				*/
	{"SO",			 0,	SpoolerOnly},	/* spool only					*/
	{"SPOOL",		 0,	SpoolerOnly},	/* spool only					*/
	{"START",		 0,	StartWorking},	/* start of data				*/
	{"VT",			 0,	VerticalTab},	/* vertical tab					*/
	{"WIDTH",		 0,	FormWidth},		/* set form width				*/
	{"e",			 1,	ExpandText},	/* expand						*/
	{NULL}								/* terminator 					*/
};

#define	DFLT_LPR_PROG	"lp"		/* Use System V "lp" */
#define	DFLT_PRT_FLAGS	"-d"		/* Use -d queuename */

#define	DFLT_WIDTH	80
#define	DFLT_LENGTH	60
#define	MAX_WIDTH	256
#define	MAX_LENGTH	100

#define	HEAD		0
#define	SECT		1
#define	FOOT1		2
#define	FOOT2		3

#define	FALSE		0
#define	TRUE		1
#define	DATA_SIZE	MAX_WIDTH
#define	LNUL		 (LIST *)0

#define	PAGE_BREAK	 (pageLength > 0 && lineNo + footerSize >= pageLength)
#define BREAK_REQD	 (defaultHeading || headerSize + sectionSize > 0 \
			 		 || footerSize > 0)
#define	FIRST_TIME	 (!init_done && lineNo == 0)

#define	TDATA		tptr->_data
#define	TNEXT		tptr->_next

#define	S_DATA		sptr->_data
#define	S_NEXT		sptr->_next

#define	FNEXT		freeList->_next

int	pitch		    = 10;			/* current pitch					*/
int	deviceNumber    = 1;			/* physical output device			*/
int	ncopies		    = 1;			/* number of copies to print		*/
int	headerSize	    = 0;			/* number of lines in header		*/
int	sectionSize	    = 0;			/* number of lines in sect head		*/
int	footerOneSize   = 0;			/* number of lines in main foot		*/
int	footerTwoSize   = 0;			/* number of lines in aux foot		*/
int	footerSize	    = 0;			/* number of lines in footer		*/
int	ruleSize	    = 0;			/* number of lines ruler			*/
int	useRuler	    = FALSE;		/* use ruler as footer				*/
int	formFeed	    = TRUE;			/* formFeed on page break			*/
int	defaultHeading	= TRUE;			/* print default heading			*/
int	pageLength	    = DFLT_LENGTH;	/* length of page					*/
int	pageWidth	    = DFLT_WIDTH;	/* width of page					*/
int	lineNo		    = 0;			/* current line number				*/
int	pageNo		    = 1;			/* current page number				*/
int	spoolerOnly		= FALSE;		/* spool only						*/
int	spooling	    = FALSE;		/* spooling output					*/
int	fromFile	    = FALSE;		/* input from file					*/
int	SpooledPrinter	= FALSE;		/* TRUE if system supports spooled output */
int	outputOpen	    = FALSE;		/* output pipe / file open			*/
int	exFactor	    = 2;			/* EX value from pcap				*/
int	currentFooter	= FOOT1;		/* current footer to use			*/

int	init_done	= FALSE;
int interpretCarets = FALSE;

static int	first_PG = TRUE;

char	buildString [DATA_SIZE * 2] = "";	/* Work area for conversions.	*/
char	finalString [DATA_SIZE * 2] = "";	/* Finished line for printing.	*/
char	dataString [DATA_SIZE];	/* input data			*/
char	dataOutput [DATA_SIZE];	/* output data			*/
char	sendStr [DATA_SIZE * 2];
char	pageChar	= '#';		/* char for page no in head	*/
char	*programPath;			/* from PROG_PATH variable	*/
char	*jobDesc;				/* Job description		*/
char	*spoolFile;				/* path for spool file		*/
char	*pipeFile;				/* string for pipe		*/
char	*outputString;			/* output device (queue / dev)	*/
char	*copyString;			/* #copies string		*/
char	*headingInfo;				/* info on default heading	*/
char	*currUser;				/* current user name		*/
char	*fileName;				/* file name - alternate device	*/
char	*ruler;					/* rule off line action		*/

static char	lprProgram [128];
static char	lprFlags [128];

FILE	*fin;				/* input device			*/
FILE	*fout;				/* output device		*/

LIST	*head [2];			/* page header definition	*/
LIST	*sect [2];			/* page section head definition	*/
LIST	*foot1 [2];			/* std page footer definition	*/
LIST	*foot2 [2];			/* aux page footer definition	*/
LIST	*freeList;			/* pointer to free list		*/

#include	"Pcap.h"

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void	Process 		(void);
void 	ProcessLine 	(char *, int);
int 	DotCommand 		(char *, int);
void 	CloseFormat 	(char *);
void 	PrintInfo 		(void);
void 	PrintHead 		(void);
void 	PrintFoot 		(int, int);
void 	PrintRuler 		(void);
void 	DefinePage 		(char *, int);
void 	PrintPage 		(int);
LIST 	*ListAlloc 		(void);        /* list node allocation */
void 	OpenOutput 		(void);
void 	SetSpool 		(void);
void  	SetPipe 		(void);
void 	SignalTrap 		(int);
void 	CheckFooter 	(void);
void 	_StringParse 	(void);
void 	InitNetPrinter 	(void);


/*===============
| Main routine. |
===============*/
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;

	/*------------------
	| Check arguments. |
	------------------*/
	if (argc != 1 && argc != 2)
	{
		print_at (0,0,ML ("Usage : %s <filename>"), argv [0]);
		return (EXIT_FAILURE);
	}

	/*---------------
	| Trap DEL key. |
	---------------*/
	signal (SIGINT, SignalTrap);

	/*------------------------------------
	| Check if Spooled output Supported. |
	-------------------------------------*/
	if ((sptr = chk_env ("LPR_ONLY")))
		SpooledPrinter = (*sptr == 'Y' || *sptr == 'y');
	else
		SpooledPrinter = FALSE;

	/*------------------
	| Filename passed. |
	------------------*/
	if (argc == 2)
	{
		fromFile = TRUE;
		if ((fin = fopen (argv [1], "r")) == 0)
		{
			sprintf (dataString, "Error in %s during (FOPEN)", argv [1]);
			sys_err (dataString, errno, PNAME);
		}
	}
	else
	{
		fin = stdin;
		fromFile = FALSE;
	}

	/*-------------------
	| Initialise lists. |
	-------------------*/
	head [0] 	= LNUL;
	head [1] 	= LNUL;
	sect [0] 	= LNUL;
	sect [1] 	= LNUL;
	foot1 [0] 	= LNUL;
	foot1 [1] 	= LNUL;
	foot2 [0] 	= LNUL;
	foot2 [1] 	= LNUL;
	freeList 	= LNUL;

	/*-----------------------
	| Initialise variables. |
	-----------------------*/
	fout       		= (FILE *)0;
	jobDesc    		= (char *)0;
	spoolFile 		= (char *)0;
	pipeFile  		= (char *)0;
	outputString    = (char *)0;
	headingInfo  	= (char *)0;
	ruler      		= (char *)0;
	copyString   	= strdup (" ");

	/*----------------------------
	| Get print spooler program. |
	----------------------------*/
	strcpy (lprProgram,(sptr = getenv ("LPR_PROGRAM")) ? sptr : DFLT_LPR_PROG);
	strcpy (lprFlags,  (sptr = getenv ("LPR_FLAGS")) ? sptr : DFLT_PRT_FLAGS);

	sptr = getenv ("LOGNAME");
	currUser = (sptr == (char *)0) ? strdup ("root") : sptr;
	sptr = getenv ("PROG_PATH");
	programPath = (sptr == (char *)0) ? strdup ("/usr/LS10.5") : sptr;
	fileName = (char *)0;

	/*-------------------------------
	| Process text to be formatted. |
	-------------------------------*/
	Process ();

	EndFormat (NULL);

	return (EXIT_SUCCESS);
}

/*================
| Process input. |
================*/
void
Process (void)
{
	char	*data;

	/*---------------------------------------
	| Process until ".EOF" or end of input. |
	---------------------------------------*/
	while (1)
	{
		/*-------------------------------------
		| Read line from input & remove "\n". |
		-------------------------------------*/
		data = fgets (dataString, DATA_SIZE, fin);
		if (data == (char *)0)
			break;
		* (data + strlen (data) - 1) = '\0';

		/*------------------------------
		| Check for valid dot command. |
		------------------------------*/
		if (*data == '.')
		{
			if (DotCommand (data, TRUE) < 0)
				ProcessLine (data, TRUE);
		}
		else
			ProcessLine (data, TRUE);
	}
}

/*=======================================
| output line, handle header & width	|
=======================================*/
void
ProcessLine (
 char*              data,
 int                new_line)
{
	char	*data_save;

	/*---------------------------------------------
	| Ignore any input if output not defined yet. |
	---------------------------------------------*/
	if (outputString == (char *) 0)
		return;

	/*--------------------------
	| Open output if required. |
	--------------------------*/
	data_save = (char *)0;
	if (!outputOpen)
	{
		data_save = strdup (data);
		OpenOutput ();
		data = data_save;
	}

	/*----------------------
	| Page break required. |
	----------------------*/
	if (new_line && (PAGE_BREAK || (FIRST_TIME && BREAK_REQD)))
	{
		if (data_save == (char *)0)
			data_save = strdup (data);
		if (PAGE_BREAK)
			PrintFoot (currentFooter, FALSE);
		lineNo = -1;
		init_done = TRUE;

		PrintHead ();
		data = data_save;
	}
	if (!strchr (data, '^') && strlen (data) > (unsigned int) pageWidth)
		* (data + pageWidth) = 0;
	strcat (buildString, data);

	/*-----------------------------
	| Print new_line if required. |
	-----------------------------*/
	if (new_line)
	{
		_StringParse ();						/* Convert buildString->finalStrinfinalString	*/
		fprintf (fout, "%s\n", finalString);
		strcpy (buildString, "");
		lineNo++;
	}

	if (data_save != (char *)0)
		free (data_save);
}

/*====================================
| Check for valid dot command        |
| return :  -1	- invalid command    |
|          else	- offset of command  |
====================================*/
int
DotCommand (
 char*              data,
 int                proc)
{
	int	cmd;

	/*-------------------------
	| Check for dot commands. |
	-------------------------*/
	for (cmd = 0; commands [cmd]._cmd; cmd++)
	{
		int	len = strlen (commands [cmd]._cmd);

		if (!strncmp (commands [cmd]._cmd, data + 1, len))
		{
			/*--------------------
			| Dot command found. |
			---------------------*/
			if (proc)
				 (*commands [cmd]._fn) (data + len + 1);
			return (cmd);
		}
	}
	/*--------------------
	| Old style heading. |
	--------------------*/
	if (* (data + 1) >= '0' && * (data + 1) <= '9')
	{
		for (cmd = 0; commands [cmd]._cmd; cmd++)
		{
			if (!strcmp (commands [cmd]._cmd, "DH"))
			{
				/*---------------------
				| Heading definition. |
				---------------------*/
				if (proc)
					 (*commands [cmd]._fn) (data + 1);
				return (cmd);
			}
		}
	}
	return (-1);
}

/*=========================
| ".EOF" or end of input. |
=========================*/
int
EndFormat (
 char*              data)
{
	CloseFormat (data);
	exit (0);
	return (0);
}

/*======================================
| Close output file in preparation for |
| either new destination or exit.      |
======================================*/
void
CloseFormat (
 char*              data)
{
	int	i;

	/*-------------------------
	| Nothing output yet ...  |
	-------------------------*/
	if (!outputOpen)
		return;

	/*---------------
	| Print footer. |
	---------------*/
	if (pageLength > 0)
	{
		if (useRuler && pageNo == 1 && lineNo == 0)
		{
			ProcessLine (ruler, TRUE);

			/*--------------------------
			| Form feed to eject page. |
			--------------------------*/
/*			if (formFeed)
			{
				fprintf (fout,"%s",TOF);
				fflush (fout);
			}
			else
			{
				curr_lno = lineNo;
				while (curr_lno < pageLength)
				{
					fprintf (fout, "\n");
					curr_lno++;
				}
			}
*/		}
		else
			PrintFoot (currentFooter, TRUE);
	}

	/*--------------------------------------------
	| Print deinitialisation string.             |
	| Only if not printing to a network printer. | 
	--------------------------------------------*/
	if (fileName == (char *) 0 && !networkPrinter)
		ProcessLine (DEINIT, FALSE);

	_StringParse ();
	fprintf (fout, "%s", finalString);
	strcpy (buildString, "");
	fflush (fout);

	/*-----------------------------------
	| Put out .EOF for network printer. |
	-----------------------------------*/
	if (networkPrinter)
		fprintf (fout, ".EOF\n");

	/*---------------
	| Close output. |
	---------------*/
	if (spooling)
	{
		fclose (fout);

		/*------------------
		| Multiple copies. |
		------------------*/
		if (spoolerOnly || ncopies > 1)
		{
			for (i = 0;i < ncopies;i++)
			{
				if (lockName [0] == 0)
				{
					/*----------------------------------------
					| Here the spooled file (save file) gets |
					| printed to the printer.                |
					----------------------------------------*/
					if (fork () == 0)
					{
						char	dest [128];
	
						if (strcmp (queue, "local"))
						{
							sprintf (dest, "%s%s", lprFlags, queue);
							execlp 
							 (
								lprProgram,
								lprProgram,
								dest,
								spoolFile,
								 (char *) 0
							);
						}
						else
						{
							execlp 
							 (
								lprProgram,
								lprProgram,
								spoolFile,
								 (char *) 0
							);
						}
						return;
					}
					else
						wait ((int *) 0);
				}
				else
				{
					char	dest [128];

					sprintf (err_str, "/bin/cat %s > %s",
							spoolFile,
							queue);
					system (err_str);
	
					if (strcmp (queue, "local"))
					{
						sprintf (dest, "%s%s", lprFlags, queue);
						execlp (lprProgram, lprProgram, dest,
								spoolFile, (char *) 0);
					}
					else
						execlp (lprProgram, lprProgram, 
								spoolFile, (char *) 0);
				}
			}
		}
	}
	else
	{
		if (lockName [0] == 0)
			pclose (fout);
		else
			fclose (fout);
	}

	/*----------------------------
	| Get rid of the device	lock |
	| if we created it.          |
	----------------------------*/
	if (lockName [0] != 0)
		unlink (lockName);

}

/*========================
| ".DH"	- define header. |
========================*/
int
DefaultHeader (
 char*              data)
{
	DefinePage (data, HEAD);
    return (EXIT_SUCCESS);
}

/*====================================
| ".DS"		- define section header. |
====================================*/
int
DefineSection (
 char*              data)
{
	DefinePage (data, SECT);
    return (EXIT_SUCCESS);
}

/*================================
| ".DA" - define auxillary foot. |
================================*/
int
DefaultAux (
 char*              data)
{
	DefinePage (data, FOOT2);
	footerSize = (currentFooter == FOOT1) ? footerOneSize : footerTwoSize;
	useRuler = FALSE;
	CheckFooter ();
    return (EXIT_SUCCESS);
}

/*========================
| ".DF" - define footer. |
========================*/
int
DefaultFooter (
 char*              data)
{
	DefinePage (data, FOOT1);
	footerSize = (currentFooter == FOOT1) ? footerOneSize : footerTwoSize;
	useRuler = FALSE;
	CheckFooter ();
    return (EXIT_SUCCESS);
}

/*=====================================
| Print default header - if required. |
=====================================*/
void
PrintInfo (void)
{
	int	middle;

	/*---------------------------------------
	| info on heading ie date, program name	|
	---------------------------------------*/
	switch (pitch)
	{
		case 10 : ProcessLine (PITCH10, FALSE);
				  break;
		case 12 : ProcessLine (PITCH12, FALSE);
				  break;
		case 16 : ProcessLine (PITCH16, FALSE);
				  break;
		default : ProcessLine (PITCH12, FALSE);
	}
	/*----------------------------------------
	| Info on heading ie date, program name. |
	----------------------------------------*/
	if (headingInfo != (char *)0)
	{
		middle = pageWidth - strlen (headingInfo) - strlen (currUser) - 16;
		if (middle < 0)
			middle = 0;
		sprintf (finalString, "Page %3d User (%s)%*.*s%s",
				 pageNo,
				 currUser,
				 middle,
				 middle,
				 " ",
				 headingInfo);
	}
	else
		sprintf (finalString, "PAGE %3d User (%s)", pageNo, currUser);

	ProcessLine (finalString, TRUE);
}

/*===============
| Print header. |
===============*/
void
PrintHead (void)
{
	lineNo = -1;
	if (defaultHeading)
		PrintInfo ();

	PrintPage (HEAD);
	PrintPage (SECT);

	lineNo = headerSize + sectionSize;
	if (defaultHeading)
		lineNo++;

	pageNo++;
}

/*===============
| Print footer. |
===============*/
void
PrintFoot (
 int                footer,
 int                end)
{
	int	curr_lno = lineNo;

	lineNo = -1;

	if (fileName != (char *) 0)
		return;

	/*-------------------
	| Can apply footer. |
	-------------------*/
	if (pageLength > 0)
	{
		/*--------------------------
		| Footer has been defined. |
		--------------------------*/
		if (useRuler)
			PrintRuler ();
		else
		{
			if (networkPrinter)
				fprintf (fout, ".B%d\n", (pageLength - footerSize) - curr_lno);
			else
			{
				while (curr_lno < (pageLength - footerSize))
				{
					fprintf (fout, "\n");
					curr_lno++;
				}
			}
			PrintPage (footer);
		}

		/*--------------------------
		| Form feed to eject page. |
		--------------------------*/
		if (!end)
		{
			if (formFeed)
			{
				if (networkPrinter)
					fprintf (fout, ".PA\n");
				else
				{
					fprintf (fout, "%s", TOF);
					fflush (fout);
				}
			}
			else
			{
				curr_lno += lineNo;
				curr_lno++;
				if (networkPrinter)
					fprintf (fout, ".B%d\n", pageLength - curr_lno);
				else
				{
					while (curr_lno < pageLength)
					{
						fprintf (fout, "\n");
						curr_lno++;
					}
				}
			}
		}
	}
	lineNo = -1;
}

/*===================
| Print ruler line. |
===================*/
void
PrintRuler (void)
{
	if (ruler != (char *)0)
	{
		if (*ruler == '.')
		{
			if (DotCommand (ruler, TRUE) < 0)
				ProcessLine (ruler, TRUE);
		}
		else
			ProcessLine (ruler, TRUE);
	}
}

/*==================================
| Define page headers and footers. |
==================================*/
void
DefinePage (
 char*              data,
 int                def_type)
{
	register	int	i;
	int		clen;
	int		lines;
	int		cmd;
	int		len = 0;
	LIST	*tptr;
	LIST	*sptr = LNUL;
	LIST	*page [2];

	/*--------------------
	| Free current list. |
	--------------------*/
	switch (def_type)
	{
	case	HEAD:
		tptr = head [0];
		break;

	case	SECT:
		tptr = sect [0];
		break;

	case	FOOT1:
		tptr = foot1 [0];
		break;

	case	FOOT2:
		tptr = foot2 [0];
		break;

	default:
		return;
	}

	while (tptr)
	{
		sptr = tptr;
		tptr = TNEXT;
		free (S_DATA);
		if (freeList)
			S_NEXT = freeList;
		else
			S_NEXT = LNUL;
		freeList = sptr;
	}

	/*--------------------------------
	| Get number of lines in header. |
	--------------------------------*/
	lines = atoi (data);
	page [0] = LNUL;
	page [1] = LNUL;

	/*------------------
	| Create new list. |
	------------------*/
	for (i = 0;i < lines;i++)
	{
		data = fgets (dataString, DATA_SIZE, fin);
		if (data == (char *)0)
			break;
		* (data + strlen (data) - 1) = '\0';

		/*----------------------------
		| Find dot command position. |
		----------------------------*/
		if (*data == '.')
			cmd = DotCommand (data, FALSE);
		else
			cmd = -1;

		/*--------------------
		| Not a dot command. |
		--------------------*/
		if (cmd < 0)
			len++;
		else
		{
			if (!commands [cmd]._size)
			{
				clen = strlen (commands [cmd]._cmd);
				 (*commands [cmd]._fn) (data + clen + 1);
				continue;
			}

			/*----------------
			| Variable size. |
			----------------*/
			if (commands [cmd]._size < 0)
				len += atoi (data + strlen (commands [cmd]._cmd) + 1);
			else
				len += commands [cmd]._size;
		}
		sptr = ListAlloc ();
		if (sptr == LNUL)
			break;

		S_DATA = strdup (data);
		S_NEXT = LNUL;
		if (page [0] == LNUL)
		{
			page [0] = sptr;
			page [1] = sptr;
		}
		else
		{
			page [1]->_next = sptr;
			page [1] = page [1]->_next;
		}
	}

	/*--------------------------------------
	| Set head, section head or foot size. |
	--------------------------------------*/
	switch (def_type)
	{
	case	HEAD:
		headerSize = len;
		head [0] = page [0];
		head [1] = page [1];
		break;

	case	SECT:
		sectionSize = len;
		sect [0] = page [0];
		sect [1] = page [1];
		break;

	case	FOOT1:
		footerOneSize = len;
		foot1 [0] = page [0];
		foot1 [1] = page [1];
		break;

	case	FOOT2:
		footerTwoSize = len;
		foot2 [0] = page [0];
		foot2 [1] = page [1];
		break;
	}
}

/*===============================
| Print page header and footer. |
===============================*/
void
PrintPage (
 int                prt_type)
{
	char	*data;
	char	*sptr;
	char	*lptr;
	LIST	*tptr	=	LNUL;

	/*-----------------------
	| Process header lines. |
	-----------------------*/
	switch (prt_type)
	{
	case	HEAD:
		tptr = head [0];
		break;

	case	SECT:
		tptr = sect [0];
		break;

	case	FOOT1:
		tptr = foot1 [0];
		break;

	case	FOOT2:
		tptr = foot2 [0];
		break;

	default:
		break;
	}

	/*---------------
	| Process list. |
	---------------*/
	while (tptr != LNUL)
	{
		data = dataString;
		*data = (char) NULL;

		/*----------------------------------------
		| Substitute page numbers for page char. |
		----------------------------------------*/
		sptr = TDATA;
		lptr = strchr (sptr, pageChar);
		while (lptr != (char *)0)
		{
			/*----------------------
			| Concatenate lead in. |
			----------------------*/
			data = strncat (data, sptr, lptr - sptr);
			sptr = lptr;

			/*--------------------
			| Move down pageNo. |
			--------------------*/
			while (strlen (sptr) && *sptr == pageChar)
				sptr++;

			/*----------------------
			| Concatenate pageNo. |
			----------------------*/
			sprintf (err_str, "%*d", sptr - lptr, pageNo);
			data = strcat (data, err_str);
			lptr = strchr (sptr, pageChar);
		}
		data = strcat (data, sptr);

		/*-----------------------
		| Possible dot command. |
		-----------------------*/
		if (*data == '.')
		{
			if (DotCommand (data, TRUE) < 0)
				ProcessLine (data, TRUE);
		}
		else
			ProcessLine (data, TRUE);
		tptr = TNEXT;
	}
}

/*=======================================
| Alloc list node for header or footer. |
=======================================*/
LIST*
ListAlloc (void)
{
	LIST	*tptr;

	/*---------------------
	| Nodes in free list. |
	---------------------*/
	if (freeList)
	{
		tptr = freeList;
		freeList = (FNEXT) ? FNEXT : LNUL;
	}
	else
		tptr = (LIST *) malloc ((unsigned) sizeof (LIST));

	return (tptr);
}

/*===========================
| ".PD" - Page description. |
===========================*/
int
PageDesc (
 char*              data)
{
	LIST	*tptr = sect [0];
	LIST	*sptr = LNUL;

	/*-------------------------------
	| Free existing section header. |
	-------------------------------*/
	while (tptr)
	{
		sptr = tptr;
		tptr = TNEXT;
		free (S_DATA);
		if (freeList)
			S_NEXT = freeList;
		freeList = sptr;
	}

	/*------------------
	| Create new list. |
	------------------*/
	sectionSize = 0;
	sect [0] = LNUL;
	sect [1] = LNUL;
	sptr = ListAlloc ();
	if (sptr != LNUL)
	{
		S_DATA = strdup (data);
		S_NEXT = LNUL;
		sectionSize = 1;
		sect [0] = sptr;
		sect [1] = sptr;
	}
/*
	data = fgets (dataString,DATA_SIZE,fin);
*/
    return (EXIT_SUCCESS);
}

/*===========================
| ".AD" - Alternate device. |
===========================*/
int
AltDevice (
 char*              data)
{
	if (fileName != (char *)0)
		free (fileName);
	fileName = strdup (data);
	outputString = strdup (data);
    return (EXIT_SUCCESS);
}

/*=====================
| ".B" - Blank lines. |
=====================*/
int
BlankLines (
 char*              data)
{
	int		n_blank = atoi (data);
	int		i;

	/*----------------------------
	| Print n_blank blank lines. |
	----------------------------*/
	for (i = 0; i < n_blank; i++)
		ProcessLine ("", TRUE);
    return (EXIT_SUCCESS);
}

/*======================
| ".NF"	- no formFeed. |
======================*/
int
NoFormFeed (
 char*              data)
{
	formFeed = FALSE;
    return (EXIT_SUCCESS);
}

/*========================================
| ".CARET_ON" - Caret interpretation ON. |
========================================*/
int
CaretOn (
 char*              data)
{
    interpretCarets = TRUE;
    return (EXIT_SUCCESS);
}

/*==========================================
| ".CARET_OFF" - Caret interpretation OFF. |
==========================================*/
int
CaretOff (
 char*              data)
{
    interpretCarets = FALSE;
    return (EXIT_SUCCESS);
}

/*=====================
| ".C" - Centre text. |
=====================*/
int
CentreText (
 char*              data)
{
	int	left;

	/*---------------------------------
	| Calculate # characters to left. |
	---------------------------------*/
	left = pageWidth - strlen (data);
	if (left > 0)
	{
		left /= 2;
		sprintf (dataOutput, "%*.*s%s", left, left, " ", data);
		ProcessLine (dataOutput, TRUE);
	}
	else
		ProcessLine (data, TRUE);

    return (EXIT_SUCCESS);
}

/*=============================
| ".CF" - Set current footer. |
=============================*/
int
CurrentFooter (
 char*              data)
{
	int	footer = atoi (data);

	/*---------------------
	| Set current footer. |
	---------------------*/
	currentFooter = (footer <= 1) ? FOOT1 : FOOT2;
	footerSize = (footer <= 1) ? footerOneSize : footerTwoSize;
	useRuler = FALSE;
	CheckFooter ();
    return (EXIT_SUCCESS);
}

/*==============================
| ".E" - Expand & centre text. |
==============================*/
int
ExpendCentre (
 char*              data)
{
	int	left;
	int	len;

	/*----------------------------
	| Calculate # chars to left. |
	----------------------------*/
	len = strlen (data);
	left = pageWidth / exFactor;
	left -= len;
	left /= 2;
	if (left < 0)
		sprintf (dataOutput, "%.*s", pageWidth / exFactor, data);
	else
		sprintf (dataOutput, "%*.*s%s", left, left, " ", data);

	if (strlen (EXON))
	{
		switch (pitch)
		{
			case 10 : sprintf (finalString, "%s%s%s%s", 
							   EXON, dataOutput, EXOFF, PITCH10);
					  break;
			case 12 : sprintf (finalString, "%s%s%s%s", 
							   EXON, dataOutput, EXOFF, PITCH12);
					  break;
			case 16 : sprintf (finalString, "%s%s%s%s", 
							   EXON, dataOutput, EXOFF, PITCH16);
					  break;
			default : sprintf (finalString, "%s%s%s%s", 
							   EXON, dataOutput, EXOFF, PITCH12);
		}
	}
	else
		expand (finalString, dataOutput);

	ProcessLine (finalString, TRUE);
    return (EXIT_SUCCESS);
}

/*============================
| ".LP" - Set output device. |
============================*/
int
OutputDevice (
 char*              data)
{
	int		printerNumber;
	int		tmpEx;

	if (outputString != (char *)0)
	{
		CloseFormat (NULL);
		outputOpen = FALSE;
		free (outputString);
		outputString = (char *) 0;

		if (pipeFile != (char *) 0)
			free (pipeFile);

		pipeFile = (char *) 0;
		lineNo   = 0;
		pageNo   = 1;
		init_done = FALSE;
	}

	/*---------------------------------
	| Read printer capabilities file. |
	---------------------------------*/
	printerNumber = atoi (data);
	deviceNumber = InitPrinter (printerNumber);
	tmpEx = atoi (EXSPACE);
	if (tmpEx > 0)
		exFactor = tmpEx;

	/*---------------------------
	| output to queue			|
	---------------------------*/
	if (!strncmp (queue, "local", 5))
	{
		strcpy (lprProgram, get_env ("LCL_QUEUE"));
		sprintf (err_str, "exec %s", lprProgram);
		spoolerOnly = TRUE;
	}
	else if (deviceNumber == 0)
			sprintf (err_str, "exec %s %s%s", lprProgram, lprFlags, queue);
	else
	{
		/*---------------------------
		| default printer			|
		----------------------------*/
		sprintf (err_str, "exec %s", lprProgram);
	}
	outputString = strdup (err_str);
    return (EXIT_SUCCESS);
}

/*==========================
| ".NC" - Multiple copies. |
==========================*/
int
MultipleCopies (
 char*              data)
{
	if (copyString != (char *)0)
		free (copyString);
	ncopies = atoi (data);

	/*------------------
	| Multiple copies. |
	------------------*/
	if (ncopies > 1)
	{
		sprintf (err_str, "-n%d", ncopies);
		copyString = strdup (err_str);
	}
	else
		copyString = strdup (" ");
    return (EXIT_SUCCESS);
}

/*==========================
| ".JD" - Job Description. |
==========================*/
int
JobDesc (
 char*              data)
{
	jobDesc = strdup (data);
    return (EXIT_SUCCESS);
}

/*===============================
| ".OP" - Inhibit default head. |
===============================*/
int
NoHeading (
 char*              data)
{
	defaultHeading = FALSE;
    return (EXIT_SUCCESS);
}

/*================================
| ".NR" - Don't restart spooler. |
================================*/
int
NoRestart (
 char*              data)
{
    return (EXIT_SUCCESS);
}

/*============================
| ".AS" - Alternate spooler. |
============================*/
int
AltSpooler (
 char*              data)
{
    return (EXIT_SUCCESS);
}

/*=======================
| ".VT" - Vertical tab. |
=======================*/
int
VerticalTab (
 char*              data)
{
	int		i;
	int		target = atoi (data);

	/*---------------------------------------
	| Advance to target line if on this pg. | 
	---------------------------------------*/
	for (i = lineNo; i < target; i++)
		ProcessLine ("", TRUE);
    return (EXIT_SUCCESS);
}

/*=====================
| ".PA" - Page break. |
=====================*/
int
PageBreak (
 char*              data)
{
	if (!outputOpen)
		OpenOutput ();

	if (fileName != (char *) 0)
	{
		fprintf (fout, ".PA\n");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| No footing & formFeed allowed. |
	--------------------------------*/
	if (footerSize <= 0 && formFeed)
	{
		lineNo = pageLength;
		return (EXIT_SUCCESS);
	}

	/*--------------
	| Print ruler. |
	--------------*/
	PrintFoot (currentFooter, FALSE);

	PrintHead ();

	ProcessLine ("", FALSE);
    return (EXIT_SUCCESS);
}

/*==============================
| ".PC" - Define page no char. |
==============================*/
int
PageDefine (
 char*              data)
{
	pageChar = *data;
    return (EXIT_SUCCESS);
}

/*=======================
| ".PF" - Print footer. |
=======================*/
int
FooterPrint (
 char*              data)
{
	int		i;
	int		footer = atoi (data);
	int		n_blank;

	/*----------------------------
	| Print "\n" to end of page. |
	----------------------------*/
	currentFooter = (footer <= 1) ? FOOT1 : FOOT2;
	footerSize = (footer <= 1) ? footerOneSize : footerTwoSize;
	useRuler = FALSE;
	CheckFooter ();
	n_blank = (pageLength - footerSize - lineNo);
	for (i = 0;i < n_blank;i++)
		ProcessLine ("", TRUE);

	/*---------------------------
	| Print appropriate footer. |
	---------------------------*/
	PrintFoot (currentFooter, FALSE);
    return (EXIT_SUCCESS);
}

/*==========================
| ".ME" - Merge text file. |
==========================*/
int
MergeFile (
 char*              data)
{
	FILE	*ftext;

	/*-----------------
	| Open text file. |
	-----------------*/
	ftext = fin;
	if ((fin = fopen (data, "r")) == 0)
	{
		sprintf (err_str, "Error in %s during (FOPEN)", data);
		sys_err (err_str, errno, PNAME);
	}
	Process ();
	fclose (fin);
	fin = ftext;
	return (EXIT_SUCCESS);
}

/*=====================
| ".PI10" - 10 pitch. |
=====================*/
int
TenPitch (
 char*              data)
{
	/*-------------------------
	| Output has been opened. |
	-------------------------*/
	if (outputString)
	{	
		if (networkPrinter)
			ProcessLine (".PI10\n", FALSE);
		else
			ProcessLine (PITCH10, FALSE);
	}

	pitch = 10;
    return (EXIT_SUCCESS);
}

/*=====================
| ".PI12" - 12 pitch. |
=====================*/
int
TwelvePitch (
 char*              data)
{
	/*-------------------------
	| Output has been opened. |
	-------------------------*/
	if (outputString)
	{	
		if (networkPrinter)
			ProcessLine (".PI12\n", FALSE);
		else
			ProcessLine (PITCH12, FALSE);
	}

	pitch = 12;
    return (EXIT_SUCCESS);
}

/*=====================
| ".PI16" - 16 pitch. |
=====================*/
int
SixteenPitch (
 char*              data)
{
	/*-------------------------
	| Output has been opened. |
	-------------------------*/
	if (outputString)
	{	
		if (networkPrinter)
			ProcessLine (".PI16\n", FALSE);
		else
			ProcessLine (PITCH16, FALSE);
	}
	pitch = 16;
    return (EXIT_SUCCESS);
}

/*======================
| ".PL" - page length. |
======================*/
int
FormLength (
 char*              data)
{
	pageLength = atoi (data);

	if (pageLength > MAX_LENGTH || pageLength < 0)
		pageLength = DFLT_LENGTH;
    return (EXIT_SUCCESS);
}

/*=======================
| ".LINE" - Print line. |
=======================*/
int
PrintLine (
 char*              data)
{
	int		i;
	int		number;
	int		line_char;

	/*-------------------------------
	| Calc length of line to print. |
	-------------------------------*/
	number = atoi (data);
	if (number <= 0)
		number = pageWidth;

	/*------------------------
	| Get line drawing char. |
	------------------------*/
	if (strlen (data))
		line_char = * (data + strlen (data) - 1);
	else
		line_char = '=';

	/*---------------------
	| Build & print line. |
	---------------------*/
	for (i = 0;i < number && i < pageWidth;i++)
		dataOutput [i] = line_char;
	dataOutput [i] = '\0';
	ProcessLine (dataOutput, TRUE);
    return (EXIT_SUCCESS);
}

/*=============================
| ".LRP" - Optional new page. |
=============================*/
int
NewPage (
 char*              data)
{
	int	number = atoi (data);

	/*------------------------------
	| Calc if PageBreak required. |
	------------------------------*/
	if (lineNo + footerSize + number > pageLength)
		PageBreak (data);
    return (EXIT_SUCCESS);
}

/*==========================
| ".PG" - Set page number. |
==========================*/
int
PageNumber (
 char*              data)
{
	pageNo = atoi (data);
	if (!first_PG)
		PageBreak (data);

	first_PG = FALSE;
    return (EXIT_SUCCESS);
}

/*===============================
| ".PN" - Turn default head on. |
===============================*/
int
PageingOn (
 char*              data)
{
	defaultHeading = TRUE;
    return (EXIT_SUCCESS);
}

/*====================
| ".L" - Page width. |
====================*/
int
FormWidth (
 char*              data)
{
	pageWidth = atoi (data);
	if (pageWidth > MAX_WIDTH || pageWidth <= 0)
		pageWidth = DFLT_WIDTH;
    return (EXIT_SUCCESS);
}

/*=======================
| ".RE" - Comment line. |
=======================*/
int
Comment (
 char*              data)
{
    return (EXIT_SUCCESS);
}

/*=====================
| ".SO" - Spool only. |
=====================*/
int
SpoolerOnly (
 char*              data)
{
	spoolerOnly = TRUE;
    return (EXIT_SUCCESS);
}

/*===================================
| ".START" - Define default header. |
===================================*/
int
StartWorking (
 char*              data)
{
	if (headingInfo != (char *)0)
		free (headingInfo);
	if (strlen (data))
		headingInfo = strdup (data);
	else
		headingInfo = (char *)0;

    return (EXIT_SUCCESS);
}

/*=====================
| ".e" - Expand text. |
=====================*/
int
ExpandText (
 char*              data)
{
	int		left;
	int		len;

	/*------------------------
	| Calculate # chrs/line. |
	------------------------*/
	len = strlen (data);
	left = pageWidth / exFactor;

	if (left < len)
		* (data + left) = 0;

	if (strlen (EXON))
	{
		switch (pitch)
		{
			case 10 : sprintf (finalString, "%s%s%s%s", 
							   EXON, dataOutput, EXOFF, PITCH10);
					  break;
			case 12 : sprintf (finalString, "%s%s%s%s", 
							   EXON, dataOutput, EXOFF, PITCH12);
					  break;
			case 16 : sprintf (finalString, "%s%s%s%s", 
							   EXON, dataOutput, EXOFF, PITCH16);
					  break;
			default : sprintf (finalString, "%s%s%s%s", 
							   EXON, dataOutput, EXOFF, PITCH12);
		}
	}
	else
		expand (finalString, data);

	ProcessLine (finalString, TRUE);
    return (EXIT_SUCCESS);
}

/*=======================
| ".R" - Rule off line. |
=======================*/
int
RuleOff (
 char*              data)
{
	int		cmd;

	/*-----------------
	| Free old ruler. |
	-----------------*/
	if (ruler != (char *)0)
		free (ruler);
	ruler = strdup (data);

	/*----------------------------
	| Find dot command position. |
	----------------------------*/
	if (*data == '.')
		cmd = DotCommand (data,FALSE);
	else
		cmd = -1;

	/*--------------------
	| Not a dot command. |
	--------------------*/
	if (cmd < 0)
		ruleSize = 1;
	else
	{
		/*----------------
		| Variable size. |
		----------------*/
		if (commands [cmd]._size < 0)
			ruleSize = atoi (data + strlen (commands [cmd]._cmd) + 1);
		else
			ruleSize = commands [cmd]._size;
	}
	if (useRuler)
		footerSize = ruleSize;
	else
		CheckFooter ();

    return (EXIT_SUCCESS);
}

/*===================================
| Open  & initialise output device. |
===================================*/
void
OpenOutput (void)
{
	/*--------------------------------
	| Spool only or multiple copies. |
	--------------------------------*/
	if (spoolerOnly || ncopies > 1 || !SpooledPrinter || fileName)
	{
		SetSpool ();
		if ((fout = fopen (spoolFile, "w")) == 0)
		{
			sprintf (err_str, "Error in %s during (FOPEN)", spoolFile);
			sys_err (err_str, errno, PNAME);
		}
		spooling = TRUE;
	}
	else
	{
		SetPipe ();
		if (lockName [0] == 0)
			fout = popen (pipeFile, "w");
		else
			fout = fopen (queue, "w");
		if (fout == (FILE *) 0)
		{
			sprintf (err_str, "Error in %s during (FOPEN)", spoolFile);
			sys_err (err_str, errno, PNAME);
		}
		spooling = FALSE;

		/*------------------------------------------
		| If we are printing to a network printer  |
		| then output necessary information.       |
		------------------------------------------*/
		if (networkPrinter)
			InitNetPrinter ();
	}
	outputOpen = TRUE;

	/*--------------------------------------------
	| Print initialisation string & header.      |
	| Only if not printing to a network printer. | 
	--------------------------------------------*/
	if (fileName == (char *) 0 && !networkPrinter)
		ProcessLine (INIT, FALSE);
}

/*==========================
| Set spoolFile variable. |
==========================*/
void
SetSpool (void)
{
	int		dmy [3];
	long	createDate;
	char	workDate [11];
	char	workTime [6];
	char	saveFilePrefix [100];

	/*-----------------
	| Already called. |
	-----------------*/
	if (spoolFile != (char *)0)
	{
		/*-----------------------------
		| Output device already open. |
		-----------------------------*/
		if (fout != (FILE *)0)
		{
			/*--------------------------------------------
			| Print deinitialisation string.             |
			| Only if not printing to a network printer. | 
			--------------------------------------------*/
			if (fileName == (char *) 0 && !networkPrinter)
				ProcessLine (DEINIT, FALSE);

			_StringParse ();
			fprintf (fout, "%s", finalString);
			strcpy (buildString, "");
			if (spooling || lockName [0] != 0)
				fclose (fout);
			else
				pclose (fout);
		}
		free (spoolFile);
	}

	/*-----------------------------
	| Output to alternate device. |
	-----------------------------*/
	if (fileName != (char *)0)
		strcpy (err_str, fileName);
	else
	{
		sprintf (workDate , "%-10.10s", DateToString (TodaysDate ()));
		strcpy (workTime, TimeHHMM ());
		createDate	=	TodaysDate ();
		DateToDMY (createDate, &dmy [0], &dmy [1], &dmy [2]);
	
		sprintf (saveFilePrefix, "%d%d%d-%s-%s", 
								dmy [2], dmy [1], dmy [0], workTime, currUser);
	
		sprintf (err_str, "%s/%s/%s%06d.spl", 
							programPath, spool_name, saveFilePrefix, getpid ());
	}

	spoolFile = strdup (err_str);
}

/*=========================
| Set pipeFile variable. |
=========================*/
void
SetPipe (void)
{
	/*-----------------
	| Already called. |
	-----------------*/
	if (pipeFile)
	{
		/*-----------------------------
		| Output device already open. |
		-----------------------------*/
		if (fout != (FILE *)0)
		{
			/*--------------------------------------------
			| Print deinitialisation string.             |
			| Only if not printing to a network printer. | 
			--------------------------------------------*/
			if (fileName == (char *) 0 && !networkPrinter)
				ProcessLine (DEINIT, FALSE);

			_StringParse ();
			fprintf (fout, "%s", finalString);
			strcpy (buildString, "");
			if (spooling || lockName [0] != 0)
				fclose (fout);
			else
				pclose (fout);
		}
		free (pipeFile);
	}

	if (SpooledPrinter)
		sprintf (err_str, "%s %s", outputString, copyString);
	else
		strcpy (err_str, outputString);
	pipeFile = strdup (err_str);
}

void	
SignalTrap (
 int                x)
{
	signal (SIGINT, SIG_IGN);
	ProcessLine ("", TRUE);
	ProcessLine (" Terminated due to a users request ... ", TRUE);
	ProcessLine ("", TRUE);
	EndFormat (NULL);
}

void
CheckFooter (void)
{
	if (!useRuler && footerSize <= 0)
	{
		footerSize = ruleSize;
		useRuler = TRUE;
	}
	else
		useRuler = FALSE;
}

/*=======================================================
| Convert buildString into finalString by searching through |
| buildString and replacing information after caret       |
| symbols '^' with corresponding escape sequences.      |
=======================================================*/
void
_StringParse (void)
{
	int		indx;
	int		g_set = FALSE;
	int		so_set = FALSE;
	int		us_set = FALSE;
	char	*sptr = buildString;
	char	*tptr;

	strcpy (finalString, "");
    if (!interpretCarets)
    {
        strcpy (finalString, buildString);
        return;
    }

	while (*sptr)
	{
		tptr = finalString + strlen (finalString);
		if (!g_set)
		{
			while (*sptr != '^' && *sptr)
				*tptr++ = *sptr++;
			*tptr = 0;
		}
		else
		{
			while (*sptr != '^' && *sptr)
			{
				indx = *sptr - 'A';
				sptr++;
				if (indx >= 0 && indx < 16)
				{
					*tptr = * (BOXCHRS + indx);
					tptr++;
				}
			}
			*tptr = 0;
		}

		if (! (*sptr))
			break;
		sptr++;
		if (! (*sptr))
			break;

		switch (*sptr)
		{
		case	'^':
			g_set = !g_set;
			sptr++;
			strcat (finalString, (g_set) ? GPXON : GPXOFF);
			break;

		case	'1':
		case	'3':
		case	'4':
			so_set = TRUE;
			strcat (finalString, BOLDON);
			sptr++;
			break;

		case	'6':
		case	'8':
		case	'9':
			so_set = FALSE;
			strcat (finalString, BOLDOFF);
			sptr++;
			break;

		case	'2':
			us_set = TRUE;
			strcat (finalString, USON);
			sptr++;
			break;

		case	'7':
			us_set = FALSE;
			strcat (finalString, USOFF);
			sptr++;
			break;

		default:
			indx = *sptr - 'A';
			if (indx >= 0 && indx < 16)
			{
				if (!g_set)
					strcat (finalString, GPXON);
				finalString [strlen (finalString) + 1] = 0;
				finalString [strlen (finalString)] = * (BOXCHRS+indx);
				if (!g_set)
					strcat (finalString, GPXOFF);
				sptr++;
			}
		}
	}

	if (g_set)
		strcat (finalString, GPXOFF);

	if (so_set)
		strcat (finalString, BOLDOFF);

	if (us_set)
		strcat (finalString, USOFF);
}

/*==================================
| Output necessary information for |
| network printer communication.   |
==================================*/
void
InitNetPrinter (void)
{
	fprintf (fout, "%s\n", netPrtHostName);
	fprintf (fout, "%s\n", netPrtServiceName);
	fprintf (fout, "%s\n", netPrtPrinterName);

	fflush (fout);
}
