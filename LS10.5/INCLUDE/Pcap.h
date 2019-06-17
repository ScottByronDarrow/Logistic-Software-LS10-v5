/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: Pcap.h,v 5.0 2001/06/19 06:51:19 cha Exp $
|  Program Name  : ( pcap.h     	)                                 |
|  Program Desc  : ( Printcap include file.                       )   |
|---------------------------------------------------------------------|
|  Access files  :  prntype                                           |
|---------------------------------------------------------------------|
| $Log: Pcap.h,v $
| Revision 5.0  2001/06/19 06:51:19  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:21  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.4  2001/01/12 01:28:10  scott
| Re-added as found program that used this include.
|
| Revision 3.2  2000/11/10 02:50:49  scott
| Updated to replace pcap.h, required for new cleaned pformat.
|
=====================================================================*/
/*======================================================================
|																		|
| NB: When making ANY changes to the _pa array within this module,		|
|	PLEASE ripple the same changes into the pcap_maint.c program.		|
|	Thanks....... TvB. 15/01/91.										|
|																		|
=======================================================================*/
#define TOF	_pa [0]._value	/* Top of form.						*/
#define EXON	_pa [1]._value	/* Expanded print on.			*/
#define EXOFF	_pa [2]._value	/* Expanded print off.			*/
#define INIT	_pa [3]._value	/* Initialisation string		*/
#define DEINIT	_pa [4]._value	/* Deinitialisation string		*/
#define PITCH10	_pa [5]._value	/* Switch to '10' pitch.		*/
#define PITCH12	_pa [6]._value	/* Switch to '12' pitch.		*/
#define PITCH16	_pa [7]._value	/* Switch to '16' pitch.		*/
#define USON	_pa [8]._value	/* Underscore on				*/
#define USOFF	_pa [9]._value	/* Underscore off				*/
#define FONT1	_pa [10]._value	/* Switch to font number 1		*/
#define FONT2	_pa [11]._value	/* Switch to font number 2		*/
#define FONT3	_pa [12]._value	/* Switch to font number 3		*/
#define OSON	_pa [13]._value	/* Switch on Overstrike.		*/
#define OSOFF	_pa [14]._value	/* Switch off Overstrike.		*/
#define EXSPACE	_pa [15]._value	/* Divisor ( 2 or 4 ) use with .E	*/
#define	BOLDON	_pa [16]._value	/* Turn on bold print.			*/
#define	BOLDOFF	_pa [17]._value	/* Turn off bold print.			*/
#define	GPXON	_pa [18]._value	/* Turn on graphics mode.		*/
#define	GPXOFF	_pa [19]._value	/* Turn off graphics mode.		*/
#define	BOXCHRS	_pa [20]._value	/* Box chars (see termcap.cprogs)	*/

#define	CODE	_pa [i]._code
#define	VALUE	_pa [i]._value

#define	MENUSYS_DIR	"BIN/MENUSYS"
#define	PRTCAP_DIR	"BIN/MENUSYS/PRINT"

char	*deflt_box  = "----||-+--||#*x ";
char	*deflt_ex   = "2";
char	*blank_attr = "\0";
int		InitPrinter 	(int);
int		PrintLineLoad 	(char *);
int		GetPrintType 	(int, char *);
int		PrintAtoo 		(char *);
void	PrintConvert 	(char *);

struct	{
	char	*_code;
	char	*_value;
} _pa [] = {
	{ "TF=",	},
	{ "XN=",	},
	{ "XF=",	},
	{ "I1=",	},
	{ "I2=",	},
	{ "P0=",	},
	{ "P2=",	},
	{ "P6=",	},
	{ "us=",	},
	{ "ue=",	},
	{ "F1=",	},
	{ "F2=",	},
	{ "F3=",	},
	{ "SN=",	},
	{ "SF=",	},
	{ "EX=",	},
	{ "so=",	},
	{ "se=",	},
	{ "go=",	},
	{ "ge=",	},
	{ "bx=",	},
	{ "",		},
};

/*===================
| Global Variables. |
===================*/
char	lockName [21];
char	queue [21];
char	spool_name [21];
int		networkPrinter;
char	netPrtHostName [100];
char	netPrtServiceName [25];
char	netPrtPrinterName [100];

/*==================================================================
| Routine to get specific details for the LOGICAL printer number   |
| passed as an argument.                                           |
| If the number is less than 1 then printer number 1 is assumed.   |
|                                                                  |
| Returns: -1 if unable to find printer entry or open termcap.     |
==================================================================*/
int
InitPrinter (int pno)
{
	char 	_pline [80];
	char 	filename [200];
	char	ptype [9];
	int 	i;
	int		pdevno;
	int		found;
	char	*sptr;
	FILE 	*termcap;

	/*---------------------------
	| Initialise printer queue. |
	---------------------------*/
	strcpy (queue, "  ");
	if (pno < 1)
		pno = 1;

	/*-----------------------------------
	| Get device number & printer type. |
	-----------------------------------*/
	pdevno = GetPrintType (pno, ptype);
	if (pdevno < 0)
	{
		sprintf (filename, "Error in finding Logical Printer %d", pno);
		sys_err (filename, pdevno, PNAME);
	}

	/*-----------------------------
	| Initialise array of values. |
	-----------------------------*/
	for (i = 0; strlen (CODE); i++)
		VALUE = strdup (blank_attr);
	BOXCHRS = deflt_box;
	EXSPACE = deflt_ex;

	/*------------------------------
	| Don't try to read a printcap |
	| entry for a network printer. |
	------------------------------*/
	if (networkPrinter)
		return (pdevno);

	/*--------------------------------------
	| Check if definition in special file. |
	--------------------------------------*/
	sprintf (filename, "%s/%s/%s", programPath, PRTCAP_DIR, ptype);
	if (access (filename, 00))
		sprintf(filename, "%s/%s/termcap.cprogs", programPath, MENUSYS_DIR);

	/*-----------------------
	| Open definition file. |
	-----------------------*/
	if ((termcap = fopen (filename, "r")) == NULL)
	{
		sprintf (err_str,
				 "Error %s during (FOPEN)",
				 filename);
		sys_err (err_str, errno, PNAME);
	}

	/*----------------------
	| Find printers entry. |
	----------------------*/
	found = 0;
	sptr = fgets (_pline, 80, termcap);
	while (!found && sptr != (char	*)0)
	{
		if (strncmp (_pline, ptype, strlen (ptype)) == 0)
			found = 1;
		else
			sptr = fgets (_pline, 80, termcap);
	}

	/*--------------
	| Find failed. |
	--------------*/
	if (!found)
	{
		sprintf (filename, "Error in finding Print Type %s", ptype);
		sys_err (filename, -1, PNAME);
	}

	/*---------------------------------
	| Read entry into array elements. |
	---------------------------------*/
	while (sptr != (char *)0 && PrintLineLoad (_pline) != 1)
		sptr = fgets (_pline, 80, termcap);

	fclose (termcap);

	/*------------------------------
	| Convert array to 'C' format. |
	------------------------------*/
	for (i = 0; strlen (CODE); i++) 
	{
		if (VALUE != (char *) 0)
			PrintConvert (VALUE);
	}
	return (pdevno);
}

/*=============================
|                             |
=============================*/
int
PrintLineLoad (char *line)
{
	int	flag = 0;
	int	i;
	int	len = strlen(line);

	/*---------------------------------
	| Return 1 if last line in entry. |
	---------------------------------*/
	if (*(line + len - 2) == ':' || len == 0)
		flag = 1;
	while (*(line + len) != ':')
		--len;

	/*---------------------
	| Remove end of line. |
	---------------------*/
	*(line + len) = '\0';
	while (len)
	{
		if (*(line + len) == ':' )
		{
			*(line + len) = '\0';
			for (i = 0; strlen (CODE); i++)
			{
				if (strncmp (CODE, line + len + 1, 3) == 0)
				{
					VALUE = p_strsave (line + len + 4);
					break;
				}
			}
		}
		--len;
	}
	return (flag);
}

/*=============================
|                             |
=============================*/
void
PrintConvert (char *str)
{
	int	i;
	int	len = strlen(str);
	int	c = 0;

	/*-----------------
	| Convert string. |
	-----------------*/
	for (i = 0; i < len; i++)
	{
		switch (*(str + i))
		{
		/*--------------------
		| Control character. |
		--------------------*/
		case	'^':
			*(str + c) = *(str + i + 1) - 64;
			++c;
			++i;
			break;

		/*--------------------
		| Special character. |
		--------------------*/
		case	92:
			/*--------
			| ESCAPE |
			--------*/
			if (*(str + i + 1) == 'E')
			{
				*(str + c) = 27;
				++c;
				++i;
			}
			else
			{
				/*--------
				| Octal. |
				--------*/
				if (*(str + i + 1) >= '0' || *(str + i + 1) <= '3')
				{
					*(str + c) = PrintAtoo (str + i + 1);
					++c;
					i += 3;
				} 
			}
			break;

		/*-------------------
		| Normal character. |
		-------------------*/
		default	:
			*(str + c) = *(str + i);
			++c;
			break;
		}
	}
	*(str + c) = '\0';
}

/*=============================
|                             |
=============================*/
int
PrintAtoo (char *str)
{
	char	*sptr = str;
	int		oct = 0;
	int		cnt;

	for (cnt = 0; *sptr && cnt < 3; cnt++)
	{
		oct *= 8;
		oct += (*sptr++ - '0');
		oct &= 0377;
	}
	return (oct);
}

/*=========================================================================
| Routine to get entry associated with a LOGICAL printer number passed    |
| to it. Returns printer type in second argument and returns physical     |
| printer number associated with this printer. The above entries are      |
| obtained from the file './MENUSYS/prntype',  which has the following    |
| structure on each line:                                                 |
|                                                                         |
| printer-type<TAB>physical printer number<TAB>printer description<LF>	  |
|                                                                         |
| Or                                                                      |
|                                                                         |
| printer-type<TAB>queue name<TAB>printer description<LF>		  |
|                                                                         |
| The logical printer entry is the nth line in the file for printer n.    |
|                                                                         |
| Returns: physical printer number or -1 if printer entry not found or    |
| unable to open './MENUSYS/prntype'.                                     |
|                                                                         |
| If the queue-name begins with a '/', then create a 'lock' file in the   |
| /usr/spool/locks directory containing the current pid.                  |
|                                                                         |
| If the queue-name begins with a '*', then the printer is on a networked |
| PC (running 'netprt').  The file (specified by queue name) contains     |
| information regarding which PC, TCP service and Printer to use.         |
|                                                                         |
=========================================================================*/
int
GetPrintType 
(
	int 	pno,
	char 	*ptype)
{
	int		i;
	int		printerNumber;
	char	*sptr;
	char	*tptr;
	char	record [80];
	char	filename [200];
	char	netQFilename [200];
	FILE	*prntype;
	FILE	*netQueue;
	FILE	*lockFile;

	lockName [0] = 0;
	/*--------------------
	| Open prntype file. |
	--------------------*/
	sprintf (filename, "%s/%s/prntype", programPath, MENUSYS_DIR);
	if ((prntype = fopen (filename, "r")) == NULL)
	{
		sprintf (err_str, "Error %s during (FOPEN)", filename);
		sys_err (err_str, errno, PNAME);
	}

	/*---------------------------
	| Get to appropriate entry. |
	---------------------------*/
	sptr = fgets (record, 79, prntype);
	for (i = 1; sptr != (char *)0 && i < pno; i++)
		sptr = fgets (record, 79, prntype);

	/*---------
	| Failed. |
	---------*/
	fclose (prntype);
	if (sptr == (char *)0)
		return (-1);

	/*---------------------------------
	| Copy type into second argument. |
	---------------------------------*/
	tptr = strchr (sptr, '\t');
	if (tptr != (char *)0)
	{
		*tptr = '\0';
		tptr++;
		strcpy (ptype, sptr);
	}
	else
		return (-2);

	/*-------------------------
	| Logical printer number. |
	-------------------------*/
	printerNumber = atoi (tptr);

	/*------------------------
	| Might be a Queue Name. |
	------------------------*/
	if (printerNumber == 0)
	{
		sptr = tptr;
		while (*sptr && *sptr != '\t' && *sptr != '\n' && *sptr != ' ')
			sptr++;
		*sptr = '\0';

		sprintf (queue, "%.20s", tptr);

		sptr = strrchr (&queue [0], '/');
		if (sptr != (char *) 0)
			strcpy (spool_name, sptr + 1);
		else
		{
			sptr = strrchr (&queue [0], '*');
			if (sptr != (char *) 0)
				strcpy (spool_name, sptr + 1);
			else
				strcpy (spool_name, queue);
		}

		for (sptr = spool_name; *sptr; sptr++)
			*sptr = toupper (*sptr);
		if (queue [0] == '/')
		{
			sprintf (lockName, "/usr/spool/locks/%02d", pno);

			/*------------------------------
			| Wait for exclusive access to |
			| the print device.            |
			------------------------------*/
			while (access (lockName, 0) == 0);

			/*------------------------------
			| Make SURE that we create the |
			| lock file.                   |
			------------------------------*/
			while ((lockFile = fopen (lockName, "w")) == NULL);

			fprintf (lockFile, "%05d\n", getpid ());
			fclose (lockFile);
		}
		else if (queue [0] == '*')
		{
			/*------------------
			| Network printer. |
			------------------*/
			networkPrinter = TRUE;
			strcpy (queue, &queue [1]);

			/*--------------------------------------
			| Open network queue information file. |
			--------------------------------------*/
			sprintf (netQFilename, 
					 "%s/%s/%s", 
					 programPath,
					 PRTCAP_DIR,
					 queue);
			if ((netQueue = fopen (netQFilename, "r")) == NULL)
			{
				sprintf (err_str, "Error %s During (FOPEN)", netQFilename);
				sys_err (err_str, errno, PNAME);
			}

			/*---------------------------------------------------------
			| Read network printer information.                       |
			|  line#1 : Network host name of machine that has printer |
			|           attached.                                     |
			|                                                         |
			|  line#2 : Service name to connect to on printer host.   |
			|            eg. netprt                                   |
			|                                                         |
			|  line#3 : Printer name.                                 |
			---------------------------------------------------------*/
			strcpy (netPrtHostName,    "");
			strcpy (netPrtServiceName, "");
			strcpy (netPrtPrinterName, "");
			sptr = fgets (record, 79, netQueue);
			for (i = 1; sptr != (char *)0 && i <= 3; i++)
			{
				/*--------------------------
				| Remove trailing newline. |
				--------------------------*/
				*(sptr + strlen (sptr) - 1) = '\0';

				switch (i)
				{
				/*------------
				| Host name. |
				------------*/
				case 1:
				strcpy (netPrtHostName, sptr);
					break;

				/*---------------
				| Service name. |
				---------------*/
				case 2:
				strcpy (netPrtServiceName, sptr);
					break;

				/*---------------
				| Printer name. |
				---------------*/
				case 3:
				strcpy (netPrtPrinterName, sptr);
					break;
				}

				sptr = fgets (record, 79, prntype);
			}
		}
	}
	else
		sprintf (spool_name, "PRINTER%d", printerNumber);

	return (printerNumber);
}
