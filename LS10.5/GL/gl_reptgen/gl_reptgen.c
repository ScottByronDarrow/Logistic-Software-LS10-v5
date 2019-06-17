/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_reptgen.c,v 5.7 2001/11/13 02:56:57 scott Exp $
|  Program Name  : (gl_reptgen.c)
|  Program Desc  : (General Ledger Report Generator)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: gl_reptgen.c,v $
| Revision 5.7  2001/11/13 02:56:57  scott
| Updated to add ".FULLDATE"
|
| Revision 5.6  2001/11/13 02:44:43  scott
| Updated to remove program name
|
| Revision 5.5  2001/11/13 02:42:59  scott
| Updated to remove .E before dates as that may be printed after start of line
|
| Revision 5.4  2001/08/28 08:46:05  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.3  2001/08/09 09:13:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:33  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:00  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_reptgen.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_reptgen/gl_reptgen.c,v 5.7 2001/11/13 02:56:57 scott Exp $";

#include <pslscr.h>
#include <GlUtils.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_gl_mess.h>
#include <DateToString.h>
#include <pDate.h>

#define USEACCS	16	/* Number of useable accumulators		*/
#define	V_ACCS	13	/* V1 -V12 + VT (Current year values)		*/
#define	B1_ACCS	13	/* B1 -B12, BT1 (Budget 1 values Curr Yr)	*/
#define	B2_ACCS	13	/* B13-B24, BT2 (Budget 1 values Next Yr)	*/
#define	F1_ACCS	13	/* F1 -F12, FT1 (Budget 2 values Curr Yr)	*/
#define	F2_ACCS	13	/* F13-F24, FT2 (Budget 2 values Next Yr)	*/
#define	O_ACCS	2	/* Others BF, R12 (Brought Forward, Rolling 12)	*/
#define	NOACCS	 (USEACCS + V_ACCS + B1_ACCS + B2_ACCS + F1_ACCS + F2_ACCS + O_ACCS)
			/* Number of accumulators to each register	*/
#define	VARLEN	20	/* Maximum Length of Variables			*/
#define MAXLINE	255	/* Max. length of lines				*/

#define	is_vble(x)	 ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') ||\
			 		 (x >= '0' && x <= '9') || x == '_')

#define	V_BIAS	 (USEACCS)
#define	VT		 (V_BIAS + V_ACCS - 1)
#define	B1_BIAS	 (V_BIAS + V_ACCS)
#define	BT1		 (B1_BIAS + B1_ACCS - 1)
#define	B2_BIAS	 (B1_BIAS + B1_ACCS)
#define	BT2		 (B2_BIAS + B2_ACCS - 1)
#define	F1_BIAS	 (B2_BIAS + B2_ACCS)
#define	FT1		 (F1_BIAS + F1_ACCS - 1)
#define	F2_BIAS	 (F1_BIAS + F1_ACCS)
#define	FT2		 (F2_BIAS + F2_ACCS - 1)
#define	R12		 (F2_BIAS + F2_ACCS)
#define	BF		 (R12 + 1)

typedef	struct	tnode
{
	char	*_var_name;
	double	_acc [NOACCS];
	struct	tnode	*_lptr;
	struct	tnode	*_rptr;
} TNODE, *TPTR;

/*#include	"gl_reptgen.h"*/

TPTR	var_head;		/* pointer to root of tree	*/
TPTR	currentReg;		/* pointer to current register	*/
TPTR	defaultReg;		/* current default register	*/
TPTR	defaultPointer [2];
TPTR	TreeAllocate (void);

#define	NUL_TREE	 (struct tnode *) 0

	/*
	 * The Following are needed for branding Routines.
	 */
	char	*specialRegisters [100];
	int		lastSpecialRegister	= 0,/* Stores last Register Number			*/
			printerNumber,			/* Printer number.  					*/
			noDecimalPoints = FALSE,/* Set if no decimal points needed.		*/
			lastAccount,			/* Last accumulator accessed.       	*/
			accountTab,				/* Amount of tab's needed for acc nos	*/
			currentPosition = 0,	/* Current position.                	*/
			newLine	= FALSE;		/* TRUE if ok to print newline & eol    */

	char	generalLedgerDate [11],	/* General ledger date in string form.  */
			printFormatMask [20] = "%12.2f ",/* Format for figures as print	*/
			printAccounts [2] = "Y",/* Indicates whether to print acc no    */
			ioline [MAXLINE];

	char	*null = "",
			companyNumber [3],
			printMonth [13];

	/*
	 * Set up Array to hold Months of Year used with mon in time struct.
	 */
	static char *mth [] = {
		"January",	
		"February",	
		"March",	
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};

	double	workValue = 0.00;

#include	"schema"

struct commRecord	comm_rec;


	FILE	*fin, *pout;

int		currentPeriod;		/* Current period number		*/
int		currentYear;		/* Current year no + century	*/

int		selectBudgetOne = 0;		/* User selected budget number	*/
int		selectBudgetTwo = 0;		/* User selected budget number	*/
long	runDate 		= 0L;		/* User specified gl date	*/

/*
 * Local Function Prototypes.
 */
void	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	ProcessFile 		 (void);
int 	PrintData 			 (char *);
void 	DotCommand 			 (char *);
int 	MakeTabs 			 (char *);
int 	PrintValue 			 (TPTR, char *);
int 	PrintBudgetHistory 	 (TPTR, char *);
void 	PrintAccounts 		 (char *);
void 	PrintAccountInfo 	 (char *);
void 	ParseRoutine 		 (char *);
void 	PrintRegister 		 (TPTR);
void 	ReportStart 		 (void);
int 	FindGlmr 			 (int);
void 	RegisterConstant 	 (TPTR, double, char);
void 	RegisterOperation 	 (TPTR, TPTR, char);
void 	ReportError 		 (char *, int, char *);
TPTR 	FindTree 			 (TPTR, char *);
TPTR 	FindVariable 		 (char *, int *);
int 	AddTree 			 (char *);
void 	InOrder 			 (TPTR, int);
int 	GlobalUpShift 		 (char *);
TPTR 	TreeAllocate 		 (void);
double 	CalculateSpecialReg (int);
char 	*NumberConvert 		 (char *, double *);

/*
 * Main Processing Routine.
 */
int
main (
 int                argc, 
 char*              argv [])
{
	if (argc != 8) 
	{
		print_at (0,0, mlGlMess704, argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);

	if ((fin = fopen (argv [2], "r")) == 0) 
		file_err (errno, argv [2], "FOPEN");

	sprintf (printAccounts, "%-1.1s", argv [3]);

	if (argv [4][0] == 'Y')
	{
		noDecimalPoints = TRUE;
		strcpy (printFormatMask,"%10.0f ");
	}

	selectBudgetOne = atoi (argv [5]);
	selectBudgetTwo = atoi (argv [6]);

	runDate = StringToDate (argv [7]);

	OpenDB ();

	if ((pout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);

	/*
	 * Work out current month 
	 */
	if (runDate == 0L)
		runDate = comm_rec.gl_date;

	strcpy (generalLedgerDate, DateToString (runDate));
	
	DateToDMY (runDate, NULL, &currentPeriod, &currentYear);

	currentYear++;
	if (currentPeriod <= comm_rec.fiscal)
		currentYear--;

	currentPeriod -= comm_rec.fiscal;
	if (currentPeriod < 1)
		currentPeriod += 12;

	sprintf (printMonth, "%-s", mth [ (comm_rec.fiscal + currentPeriod + 11) % 12]);

	ReportStart ();

	sprintf (err_str,"Processing General Ledger Report %s", argv [2]);
	dsp_screen (err_str, comm_rec.co_no,comm_rec.co_name);

	var_head = NUL_TREE;

	/*
	 * Add the default node to the tree	this is the variable that data is		
	 * read into from glmr etc.			
	 */
	if (AddTree (strdup ("__SYSTEM")) == 0)
		RegisterConstant (currentReg,0.00,'=');	/* Set to 0 to start with */

	if (AddTree (strdup ("DEFAULT")) == 0)
		RegisterConstant (currentReg,0.00,'=');	/* Set to 0 to start with */

	defaultPointer [0] = currentReg;

	if (AddTree (strdup ("A")) == 0)
		RegisterConstant (currentReg,0.00,'=');	/* Set to 0 to start with */

	defaultPointer [1] = currentReg;

	ProcessFile ();
	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	fprintf (pout,"\n.EOF\n");
	pclose (pout);
	fclose (fin);
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open database files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	OpenGlmr ();
	OpenGlpd ();

	strcpy (companyNumber, comm_rec.co_no);
}

/*
 * Close database files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_dbclose ("data");
}
/*
 * Routine that reads in and parses the complete report file.
 * Returns: 0 if ok, non-zero if not ok.                    
 */
void
ProcessFile (void)
{
	int		dotCommandFound = FALSE;
	int		done;
	int		last;
	int		workCounter = 0;
	int		cp;
	int		len;
	char	*sptr;
	char	wk_mask [11];

	while (TRUE) 
	{
		sptr = fgets (ioline,MAXLINE,fin);

		if (ioline [strlen (ioline) - 1] == '\n') 
			ioline [strlen (ioline) - 1] = '\0';

		if (feof (fin) || ferror (fin)) 
			return;

		if (strncmp (ioline,"END",3) == 0) 
			return;

		/*
		 * Ignore All lines until first dot command	
		 * generally only a Uniplex ruler.			
		 */
		if (ioline [0] == '.')
			dotCommandFound = TRUE;

		if (dotCommandFound != TRUE)
		{
			specialRegisters [lastSpecialRegister] = strdup (ioline);
			if (specialRegisters [lastSpecialRegister] == (char *) 0)
				file_err (12, "ProcessFile ()", "MALLOC ()");
			lastSpecialRegister++;
			if (lastSpecialRegister > 98)
			{
				free (specialRegisters [99]);
				lastSpecialRegister = 98;
			}
			continue;
		}

		sprintf (wk_mask, "%06d", ++workCounter);

		dsp_process ("Line : ",wk_mask);

		last = strlen (ioline);

		if (last < 1)
			continue;

		/*
		 * Remove trailing spaces.
		 */
		clip (ioline);
		last = strlen (ioline);
		if (ioline [last - 1] == ';')
			newLine = FALSE;
		else 
		{
			newLine = TRUE;
			last--;
		}

		cp = done = 0;
		defaultReg = var_head;

		while (!done && cp < last) 
		{
			switch (ioline [cp]) 
			{
			case ' ':
				while (ioline [cp] == ' ')
					cp++;
				break;

			case '"':
				cp++;
				cp += PrintData (ioline + cp);
				break;

			case '.':
				DotCommand (ioline + cp);
				done = 1;
				newLine = FALSE;
				break;

			case '@':
				cp++;
				cp += MakeTabs (ioline + cp);
				break;

			case '+':
			case '-':
			case '*':
			case '^':
			case '#':
				PrintAccounts (ioline + cp);
				done = 1;
				break;

			case '$':
				cp++;
				len = PrintValue (defaultReg,ioline + cp);
				if (len == 0)
					ReportError (ioline + cp,0,"Register Used Before Initialisation");
				else
					cp += len;
				break;

			/*
			 * "\"
			 */
			case 92:
				cp++;
				cp += PrintBudgetHistory (defaultReg, ioline + cp);
				break;

			case ';':
				newLine = FALSE;
				cp++;
				break;

			default :
				if (isalpha (ioline [cp])) 
					ParseRoutine (ioline + cp);
				else 
				{
					ReportError (ioline,cp,"");
					return;
				}
				done = 1;
				newLine = FALSE;
			}
		}

		if (newLine) 
		{
			fprintf (pout,"\n");
			fflush (pout);
			currentPosition = 0;
		}
	}
}

/*
 * Routine which prints string until matching quote is found.
 * Allows standard 'escapes' (backslash)                    
 */
int
PrintData (
	char	*s)
{
	int	i;
	int	end;

	end = strlen (s);
	i = 0;

	while (s [i] != '"' && i < end) 
	{
		if (s [i] == '\\')
			i += 2;
		else
			i++;
	}
	/*
	 * Double quotes (Blank line).
	 */
	if (i == 0) 
	{
		fprintf (pout,"\n");
		fflush (pout);
		currentPosition = 0;
		newLine = FALSE;
	}
	else 
	{
		fprintf (pout,"%*.*s",i,i,s);
		fflush (pout);
		currentPosition += i;
		return (i + 1);
	}
	return (EXIT_SUCCESS);
}

/*
 * Routine to print line passed to it as a dot command.
 * Prints it literally.                               
 */
void
DotCommand (
 char*              s)
{
	int	i;
		
	if (strncmp (s, ".MONTH",6) == 0)
	{
		fprintf (pout, "%s %s\n",s+6, printMonth);
		currentPosition = 0;
		return;
	}

	if (strncmp (s, ".STIME",6) == 0)
	{
		fprintf (pout, "%-8.8s %s\n", s+6, TimeHHMMSS ());
		currentPosition = 0;
		return;
	}

	if (strncmp (s, ".DATE",5) == 0)
	{
		fprintf (pout, "%s %s\n", s+5, generalLedgerDate);
		currentPosition = 0;
		return;
	}
	if (strncmp (s, ".FULLDATE", 9) == 0)
	{
		fprintf (pout, "%s %s\n", s+9, DateToFmtString (runDate, "%e %B %Y", err_str));
		currentPosition = 0;
		return;
	}

	if (strncmp (s, ".REM",4) == 0)
	{
		currentPosition = 0;
		return;
	}
	
	if (strncmp (s, ".CO", 3) == 0)
	{
		i = atoi (s+3);
		sprintf (companyNumber, "%2d", i);
		currentPosition = 0;		
		return;
	}
	fprintf (pout,"%s\n",s);
	fflush (pout);
	currentPosition = 0;
	return;
}

/*
 * Routine to MakeTabs to a given location in line, if location already 
 * passed then a new line is printed. Position is set to that location.  
 * Returns width of MakeTabs literal in above char.                    
 */
int
MakeTabs (
 char*              s)
{
	int		i,
			j,
			p;

	i = 0;
	while (isdigit (s [i]))
		i++;

	p = atoi (s);
	if (p < currentPosition) 
	{
		fputc ('\n',pout);
		currentPosition = 0;
	}

	j = 0;
	while (s [i + j] == ' ')
		j++;

	/*
	 * Amount of tab required for account format.
	 */
	if (s [i + j] == '#' || s [i + j] == '*') 
	{
		accountTab = p;
	}

	fprintf (pout,"%*s",p - currentPosition," ");
	fflush (pout);
	currentPosition = p;
	return (i + j);
}

/*
 * Routine to print value from specified register (r).                
 * Uses 10.2 format for number, may change later.                      
 * Also as all fields are really money fields they will be converted to 
 * double before they are printed. If there is a mixture of fields then  
 * the register structure will have to be changed.                      
 */
int
PrintValue (
 TPTR               use_reg,
 char*              s)
{
	int	i;
	int	len;
	char	junk [30];
	TPTR	tptr;

	if (use_reg == NUL_TREE)
		return (EXIT_SUCCESS);

	workValue = 0.00;

	/*
	 * Change default register.
	 */
	if (isalpha (s [0])) 
	{
		tptr = FindVariable (s,&len);
		if (tptr != NUL_TREE)
			defaultReg = tptr;
		else
			len = 0;
		return (len);
	}

	/*
	 * Assume accumulator instead.
	 */
	i = atoi (s);
	len = 0;
	if (i >= 10000)
		len = 5;
	else if (i >= 1000)
		len = 4;
	else if (i >= 100)
		len = 3;
	else if (i >= 10)
		len = 2;
	else if (i >= 1)
		len = 1;

	/*
	 * Only Use first 10 accumulators		
	 * the rest are special purpose			
	 *										
	 *	if (i < 1 || i > NOACCS)			
	 * Default to last used accumulator. 
	 */
	if (i < 1 || i > USEACCS)
		i = lastAccount;

	lastAccount = i;

	workValue = use_reg->_acc [i - 1];


	if (workValue < 0.00)
	{
		workValue *= -1;
		if (noDecimalPoints)
		{
			sprintf (printFormatMask,"(%.0f)",workValue);
			sprintf (junk,"%11.11s",printFormatMask);
		}
		else
		{
			sprintf (printFormatMask,"(%.2f)",workValue);
			sprintf (junk,"%13.13s",printFormatMask);
		}
	}
	else
	{
		if (noDecimalPoints)
			strcpy (printFormatMask,"%10.0f ");
		else
			strcpy (printFormatMask,"%12.2f ");
	
		sprintf (junk,printFormatMask,workValue);
	}

	if (workValue == 0.00)
	{
		if (noDecimalPoints)
			sprintf (junk,"%11.11s-%1.1s"," "," ");
		else
			sprintf (junk,"%8.8s-%4.4s"," "," ");
	}
	fprintf (pout,junk);
	fflush (pout);
	currentPosition += strlen (junk);
	return (len);
}

/*
 * Routine to print value from specified register (r).                 
 * Uses 10.2 format for number, may change later.                       
 * Also as all fields are really money fields they will be converted to 
 * double before they are printed. If there is a mixture of fields then
 * the register structure will have to be changed.                    
 */
int
PrintBudgetHistory (
 TPTR               use_reg,
 char*              s)
{
	int	i;
	int	len;
	char	junk [30];

	if (use_reg == NUL_TREE)
		return (EXIT_SUCCESS);

	workValue = 0.00;

	/*
	 * \V1	- Value for 1st Month of Current Year	
	 *  :											
	 * \V12	- Value for 12th Month of Current Year
	 * \VT	- Total of Values for Current Year	
	 *												
	 * \B1	- Total for 1st Month of Current Year	
	 *  :										
	 * \B12	- Total for 12th Month of Current Year	
	 * \BT1	- Total of Budgets for Current Year	
	 *										
	 * \B13	- Total for 1st Month of Next Year	
	 *  :									
	 * \B24	- Total for 12th Month of Next Year
	 * \BT2	- Total of Budgets for Next Year
	 */
	switch (s [0])
	{
	case	'V':
		if (s [1] == 'T')
		{
			i = VT;
			len = 2;
		}
		else
		{
			i = atoi (s + 1);
			len = (i >= 10) ? 3 : 2;
			i += (V_BIAS - 1);
		}
		break;

	case	'B':
		/*
		 * Brought Forward
		 */
		if (s [1] == 'F')
		{
			i =  BF;
			len = 2;
			break;
		}

		if (s [1] == 'T')
		{
			i = (s [2] == '1') ? BT1 : BT2;
			len = 3;
		}
		else
		{
			/*
			 * B1 = use_reg->_acc [B1_BIAS]		
			 * :							
			 * B12 = use_reg->_acc [B1_BIAS+11]
			 *								
			 * B13 = use_reg->_acc [B2_BIAS]
			 * :							
			 * B24 = use_reg->_acc [B2_BIAS+11]
			 */
			i = atoi (s + 1);
			len = (i >= 10) ? 3 : 2;
			i += (i < 13) ? (B1_BIAS - 1) : (B2_BIAS - 13);
		}
		break;

	case	'F':
		if (s [1] == 'T')
		{
			i = (s [2] == '1') ? FT1 : FT2;
			len = 3;
		}
		else
		{
			/*
			 * F1 = use_reg->_acc [F1_BIAS]	
			 * :								
			 * F12 = use_reg->_acc [F1_BIAS+11]
			 *								
			 * F13 = use_reg->_acc [F2_BIAS]
			 * :							
			 * F24 = use_reg->_acc [F2_BIAS+11]
			 */
			i = atoi (s + 1);
			len = (i >= 10) ? 3 : 2;
			i += (i < 13) ? (F1_BIAS - 1) : (F2_BIAS - 13);
		}
		break;

	case	'R':
		i = R12;
		len = 3;
		break;

	case	'S':
		i = atoi (s + 1);
		len = (i > 9) ? 3 : 2;
		i = 0 - i;
		break;

	default:
		return (EXIT_SUCCESS);
	}

	if (i < 0)
		workValue = CalculateSpecialReg (0 - i);
	else
		workValue = use_reg->_acc [i];

	if (workValue < 0.00)
	{
		workValue *= -1;
		if (noDecimalPoints)
		{
			sprintf (printFormatMask,"(%.0f)",workValue);
			sprintf (junk,"%11.11s",printFormatMask);
		}
		else
		{
			sprintf (printFormatMask,"(%.2f)",workValue);
			sprintf (junk,"%13.13s",printFormatMask);
		}
	}
	else
	{
		if (noDecimalPoints)
			strcpy (printFormatMask,"%10.0f ");
		else
			strcpy (printFormatMask,"%12.2f ");
	
		sprintf (junk,printFormatMask,workValue);
	}

	if (workValue == 0.00)
	{
		if (noDecimalPoints)
			sprintf (junk,"%16.14s-%1.1s"," "," ");
		else
			sprintf (junk,"%8.8s-%4.4s"," "," ");
	}
	fprintf (pout,junk);
	fflush (pout);
	currentPosition += strlen (junk);
	return (len);
}
	
/*
 * Routine to print account figures for nominated account.
 */
void
PrintAccounts (
 char*              s)
{
	char	acct_no [MAXLEVEL + 1];
	char	to_acc [MAXLEVEL + 1];
	char	from_acc [MAXLEVEL + 1];
	char	mult_acc [3];
	int		mult_flag = 0;
	int		off_set = 17;	/* Was 10 */
	int		find_type = GTEQ;
	int		ok = TRUE;
	int		len;
	TPTR	tptr;

	tptr = FindVariable (strdup ("DEFAULT"),&len);

	/*
	 * Invalid format.
	 */
	if (strlen (s) < MAXLEVEL + 1)
		return;

	sprintf (mult_acc, "%2.2s", s + (MAXLEVEL + 1));
	if (strcmp (mult_acc, "TO") == 0 || strcmp (mult_acc, "to") == 0)
	{
		mult_flag = 1;
		off_set = MAXLEVEL + MAXLEVEL + 4;	/* 2 x account + 2 x # + TO */
		find_type = GTEQ;
		sprintf (from_acc,"%-16.16s", s + 1);
		sprintf (to_acc,  "%-16.16s", s + (MAXLEVEL + 4));
		strcpy (glmrRec.acc_no,from_acc);
	}

	while (ok)
	{
		if (mult_flag)
		{
			cc = FindGlmr (find_type);
			if (cc) 
			{
				currentPosition = 0;
				return;
			}
			find_type = NEXT;

			if (strcmp (glmrRec.acc_no, to_acc) > 0)
			{
				ok = FALSE;
				return;
			}
		}
		else
		{
			sprintf (acct_no,"%-16.16s", s + 1);
			acct_no [MAXLEVEL] = '\0';
			strcpy (glmrRec.acc_no,acct_no);
			cc = FindGlmr (GTEQ);
			if (cc || strcmp (glmrRec.acc_no, acct_no)) 
			{
				currentPosition = 0;
				fprintf (pout,"%s :Record not found\n",acct_no);
				fflush (pout);
				return;
			}
			ok = FALSE;
		}

		switch (s [0]) 
		{
		case '-':
		case '+':
			RegisterOperation (tptr,var_head,s [0]);
			newLine = FALSE;
			break;

		/*
		 * Subtract from register A.
		 */
		case '*':
			RegisterConstant (var_head,-1.00,'*');
			RegisterOperation (tptr,var_head,'-');
			PrintAccountInfo (s + off_set);
			break;

		/*
		 * Add from register A.
		 */
		case '#':
			RegisterOperation (tptr,var_head,'+');
		  	PrintAccountInfo (s + off_set);
		  	break;
	
		/*
		 * Print Account only.
		 */
		case '^':
			PrintAccountInfo (s + off_set);
		  	break;

		default:
			break;
		}

		if (mult_flag) 
		{
			if (s [0] != '-' && s [0] != '+')
			{
				fprintf (pout,"\n");
				fflush (pout);
			}
			currentPosition = 0;
			newLine = FALSE;
		}
	}
	return;
}

/*
 * Routine to print account no. and name.                            
 * Also will print any values as requested, if no format exists then  
 * the last format will be used.                                     
 */
void
PrintAccountInfo (
	char	*s)
{
	int	i;
	int	len;
	int	l;
	char	*ptr;
	char	junk [100];

	defaultReg = var_head;

	l = strlen (s);
	if (l == 0)
		ptr = null;
	else 
		ptr = s;

	if (printAccounts [0] == 'Y')
		sprintf (junk,"%-*.*s - %-25.25s",
				MAXLEVEL, MAXLEVEL, glmrRec.acc_no,glmrRec.desc);
	else
		sprintf (junk,"%-25.25s",glmrRec.desc);

	l = strlen (junk);
	if (accountTab && currentPosition < accountTab) 
	{
		fprintf (pout,"%*s",accountTab - currentPosition," ");
		fflush (pout);
		currentPosition = accountTab;
	}
	fprintf (pout,junk);
	fflush (pout);
	currentPosition += l;

	l = strlen (ptr);
	i = 0;
	while (i < l) 
	{
		while (ptr [i] == ' ')
			i++;

		switch (ptr [i]) 
		{
		case '@':
			i++;
			i += MakeTabs (ptr + i);
			break;

		case '$':
			i++;
			len = PrintValue (defaultReg,ptr + i);
			if (len == 0)
				ReportError (ptr + i,0,"Register Used Before Initialisation");
			else
				i += len;
			break;

		/*
		 * "\"
		 */
		case 92:
			i++;
			i += PrintBudgetHistory (defaultReg,ptr + i);
			break;

		case '"':
			i++;
			i += PrintData (ptr + i);
			break;

		default :
			ReportError (ptr + i,0,"");
			i = l;
			break;
		}
	}
	return;
}

/*
 * Routine to parse a register expression into bits and act accordingly
 */
void
ParseRoutine (
	char	*s)
{
	TPTR	tptr;
	int		i;
	int		len;
	int		n;
	int		cs;				/* Start position of constant 				*/
	int		op_rqrd;		/* TRUE if operator required				*/
	int		err = 0;		/* TRUE if error in parsing has occurred   	*/
	char	*ops = "/-+\\*";/* Available operands 						*/
	char	cur_op = '=';	/* Current operator for expression			*/
	double	conval;			/* Value of constant						*/

	/*
	 * New Variable
	 */
	i = AddTree (s);

	switch (i)
	{
	case	-1:
		return;

	case	0:
		RegisterConstant (currentReg,0.00,'=');	/* Set to 0 to start with */

	default:
		break;
	}

	tptr = FindVariable (s,&i);
	n = strlen (s);
	while (s [i] != '=' && i < n)	/* Look for '=' sign	*/
		i++;

	if (i == n && s [i] != '=') 
	{
		err = 1;
		ReportError (s,i,"'=' Expected");
		return;
	}
	i++;

	op_rqrd = 0;
	while (i < n && !err) 
	{
		/*
		 * Remove embedded spaces.
		 */
		if (s [i] == ' ') 
		{
			i++;
			while (s [i] == ' ' && i < n)
				i++;
			continue;
		}

		/*
		 * Trap out of sequence expressions.
		 */
		if (op_rqrd && (isdigit (s [i]) || isalpha (s [i]))) 
		{
			err = 1;
			ReportError (s,i,"Operator Expected");
			break;
		}

		/*
		 * Start of constant - look for end.
		 */
		if (isdigit (s [i])) 
		{
			cs = i;
			while (isdigit (s [i]) || s [i] == '.')
				i++;	
			conval = atof (s + cs);
			op_rqrd = 1;
			RegisterConstant (currentReg,conval,cur_op);
			continue;
		}

		/*
		 * Register name.
		 */
		if (isalpha (s [i])) 
		{
			tptr = FindVariable (s + i,&len);
			if (tptr == NUL_TREE)
			{
				sprintf (err_str,"Register name %s not defined",s + i);
				ReportError (s,i,err_str);
				err = 1;
				break;
			}
			else
			{
				RegisterOperation (currentReg,tptr,cur_op);
				i += len;
			}
			op_rqrd = 1;
			continue;
		}
		/*
		 * Operator.
		 */
		if (strchr (ops,s [i]) != (char *) 0) 
		{
			if (!op_rqrd) 
			{
				ReportError (s,i,"Register name/Constant expected");
				err = 1;
				break;
			}
			else 
			{
				op_rqrd = 0;
				cur_op = s [i];
				i++;
			}
			continue;
		}

		ReportError (s,i,"");
		err = 1;
	}	
}

/*
 * Print register values on standard error. Debugging only.
 */
void
PrintRegister (
	TPTR	rega)
{
	int	i;

	if (rega == NUL_TREE)
	{
		fprintf (stderr,"Register not available\n\r");
		return;
	}
	else
		fprintf (stderr,"Register [%s]\n\r",rega->_var_name);

	for (i = 0;i < NOACCS;i++) 
		fprintf (stderr,"Result acc [%d] - %.2f\n\r",i,rega->_acc [i]);
	fflush (stderr);
}

/*
 * Start report output - specify printer number.
 */
void
ReportStart (void)
{

	fprintf (pout,".START%s\n", generalLedgerDate);
	fprintf (pout,".LP%d\n",printerNumber);
	fflush (pout);
}

/*
 * Routine which looks for glmr record according to searchtype passed as
 * an argument. Also stuffs the _acc array with data read from the glpd 
 * records as appropraiate. Returns 0 if found, 1 if not found.         
 */
int
FindGlmr (
	int		stype)
{
	strcpy (glmrRec.co_no ,companyNumber);
	cc = find_rec (glmr, &glmrRec, stype, "r");
	if (cc || strcmp (glmrRec.co_no, companyNumber))
		return (EXIT_FAILURE);

	/*
	 * Place values into register 0 (current record values).
	 */
	RegisterConstant (var_head, (double) 0.00,'=');

	/*
	 * Zero registers holding totals
	 */
	var_head->_acc [VT]		= 0.00;
	var_head->_acc [BT1]	= 0.00;
	var_head->_acc [BT2]	= 0.00;
	var_head->_acc [FT1]	= 0.00;
	var_head->_acc [FT2]	= 0.00;
	var_head->_acc [R12]	= 0.00;
	var_head->_acc [BF]		= 0.00;

	glpdRec.hhmr_hash = glmrRec.hhmr_hash;
	glpdRec.budg_no = glpdRec.year = glpdRec.prd_no = 0;
	cc = find_rec (glpd, &glpdRec, GTEQ, "r");
	while (!cc && glpdRec.hhmr_hash == glmrRec.hhmr_hash)
	{
		glpdRec.balance = DOLLARS (glpdRec.balance);
		if (glpdRec.budg_no == 0)
		{
			if (glpdRec.year == currentYear)
			{
				if (glpdRec.prd_no == currentPeriod)
				{
					var_head->_acc [0] 		+= glpdRec.balance;
					var_head->_acc [1] 		+= glpdRec.balance;
					var_head->_acc [R12] 	+= glpdRec.balance;
				}

				if (glpdRec.prd_no < currentPeriod)
				{
					var_head->_acc [1] 		+= glpdRec.balance;
					var_head->_acc [R12] 	+= glpdRec.balance;
					var_head->_acc [BF] 	+= glpdRec.balance;
				}
				var_head->_acc [V_BIAS + glpdRec.prd_no - 1] += glpdRec.balance;
				var_head->_acc [VT] += glpdRec.balance;
			}
			if (glpdRec.year == (currentYear - 1))
			{
				if (glpdRec.prd_no == currentPeriod)
					var_head->_acc [2] += glpdRec.balance;
				if (glpdRec.prd_no <= currentPeriod)
					var_head->_acc [3] += glpdRec.balance;
				else
					var_head->_acc [R12] += glpdRec.balance;
			}
		}
		if (glpdRec.budg_no == selectBudgetOne)
		{
			if (glpdRec.year == currentYear)
			{
				if (glpdRec.prd_no == currentPeriod)
					var_head->_acc [4] += glpdRec.balance;
				if (glpdRec.prd_no <= currentPeriod)
					var_head->_acc [5] += glpdRec.balance;
				var_head->_acc [B1_BIAS + glpdRec.prd_no - 1] += glpdRec.balance;
				var_head->_acc [BT1] += glpdRec.balance;
			}
			if (glpdRec.year == (currentYear + 1))
			{
				var_head->_acc [B2_BIAS + glpdRec.prd_no - 1] += glpdRec.balance;
				var_head->_acc [BT2] += glpdRec.balance;
			}
		}
		if (glpdRec.budg_no == selectBudgetTwo)
		{
			if (glpdRec.year == currentYear)
			{
				if (glpdRec.prd_no == currentPeriod)
					var_head->_acc [10] += glpdRec.balance;
				if (glpdRec.prd_no <= currentPeriod)
					var_head->_acc [11] += glpdRec.balance;
				var_head->_acc [F1_BIAS + glpdRec.prd_no - 1] += glpdRec.balance;
				var_head->_acc [FT1] += glpdRec.balance;
			}
			if (glpdRec.year == (currentYear + 1))
			{
				var_head->_acc [F2_BIAS + glpdRec.prd_no - 1] += glpdRec.balance;
				var_head->_acc [FT2] += glpdRec.balance;
			}
		}
		cc = find_rec (glpd, &glpdRec, NEXT, "r");
	}
	
	var_head->_acc [6] 		= var_head->_acc [2] 	- var_head->_acc [0];
	var_head->_acc [7] 		= var_head->_acc [3] 	- var_head->_acc [1];
	var_head->_acc [8] 		= var_head->_acc [4] 	- var_head->_acc [0];
	var_head->_acc [9] 		= var_head->_acc [5] 	- var_head->_acc [1];
	var_head->_acc [12] 	= var_head->_acc [10] 	- var_head->_acc [0];
	var_head->_acc [13] 	= var_head->_acc [11] 	- var_head->_acc [1];
	var_head->_acc [14] 	= var_head->_acc [1] 	- var_head->_acc [5];
	var_head->_acc [15] 	= var_head->_acc [0] 	- var_head->_acc [4];
	return (EXIT_SUCCESS);
}

/*
 * Routine to add,subtract conval to/from rega or multiply or divide rega by
 * conval according to op (1 of +,-,*,/). (If op = '/' and conval = 0 then  
 * the operation will not be carried out). Also if op = '=' then rega will  
 * be set to conval.                                                       
 */
void
RegisterConstant (
	TPTR	rega,
	double	conval,
	char	op)
{
	int	i;

	if (rega == NUL_TREE)
		return;

	switch (op) 
	{
	case '+': 
		for (i = 0;i < NOACCS;i++)
			if (rega == defaultPointer [0] || rega == defaultPointer [1])
			{
				defaultPointer [0]->_acc [i] += conval;
				defaultPointer [1]->_acc [i] += conval;
			}
			else
				rega->_acc [i] += conval;
		break;

	case '-': 
		for (i = 0;i < NOACCS;i++)
			if (rega == defaultPointer [0] || rega == defaultPointer [1])
			{
				defaultPointer [0]->_acc [i] -= conval;
				defaultPointer [1]->_acc [i] -= conval;
			}
			else
				rega->_acc [i] -= conval;
		break;

	case '*':
		for (i = 0;i < NOACCS;i++)
			if (rega == defaultPointer [0] || rega == defaultPointer [1])
			{
				defaultPointer [0]->_acc [i] *= conval;
				defaultPointer [1]->_acc [i] *= conval;
			}
			else
				rega->_acc [i] *= conval;
		break;

	case '/':
		if (conval != 0.0) 
		{
			for (i = 0;i < NOACCS;i++)
				if (rega == defaultPointer [0] || rega == defaultPointer [1])
				{
					defaultPointer [0]->_acc [i] /= conval;
					defaultPointer [1]->_acc [i] /= conval;
				}
				else
					rega->_acc [i] /= conval;
		}
		else
		{
			for (i = 0;i < NOACCS;i++)
				if (rega == defaultPointer [0] || rega == defaultPointer [1])
				{
					defaultPointer [0]->_acc [i] = 0.00;
					defaultPointer [1]->_acc [i] = 0.00;
				}
				else
					rega->_acc [i] = 0.00;
		}
		break;

	case '=':
		for (i = 0;i < NOACCS;i++)
			if (rega == defaultPointer [0] || rega == defaultPointer [1])
			{
				defaultPointer [0]->_acc [i] = conval;
				defaultPointer [1]->_acc [i] = conval;
			}
			else
				rega->_acc [i] = conval;
		break;

	default:
		break;
	}
}

/*
 * Routine to add,subtract,multiply or divide rega by regb according to  
 * op (1 of +,-,*,/). (If op = '/' and regb = 0 then the operation will   
 * not be carried out). Op can also be a '=' in which case rega is copied 
 * to regb.                                                             
 */
void
RegisterOperation (
	TPTR	rega,
	TPTR	regb,
	char	op)
{
	int	i;

	if (rega == NUL_TREE || regb == NUL_TREE)
		return;

	switch (op) 
	{
	case '+': 
		for (i = 0;i < NOACCS;i++)
			if (rega == defaultPointer [0] || rega == defaultPointer [1])
			{
				defaultPointer [0]->_acc [i] += regb->_acc [i];
				defaultPointer [1]->_acc [i] += regb->_acc [i];
			}
			else
				rega->_acc [i] += regb->_acc [i];
		break;

	case '-': 
		for (i = 0;i < NOACCS;i++)
			if (rega == defaultPointer [0] || rega == defaultPointer [1])
			{
				defaultPointer [0]->_acc [i] -= regb->_acc [i];
				defaultPointer [1]->_acc [i] -= regb->_acc [i];
			}
			else
				rega->_acc [i] -= regb->_acc [i];
		break;

	case '*':
		for (i = 0;i < NOACCS;i++)
			if (rega == defaultPointer [0] || rega == defaultPointer [1])
			{
				defaultPointer [0]->_acc [i] *= regb->_acc [i];
				defaultPointer [1]->_acc [i] *= regb->_acc [i];
			}
			else
				rega->_acc [i] *= regb->_acc [i];
		break;

	case '/':
		for (i = 0;i < NOACCS;i++) 
		{
			if (regb->_acc [i] != 0.0)
				if (rega == defaultPointer [0] || rega == defaultPointer [1])
				{
					defaultPointer [0]->_acc [i] /= regb->_acc [i];
					defaultPointer [1]->_acc [i] /= regb->_acc [i];
				}
				else
					rega->_acc [i] /= regb->_acc [i];
			else
				if (rega == defaultPointer [0] || rega == defaultPointer [1])
				{
					defaultPointer [0]->_acc [i] = 0.00;
					defaultPointer [1]->_acc [i] = 0.00;
				}
				else
					rega->_acc [i] = 0.00;
		}
		break;

	case '=':
		for (i = 0;i < NOACCS;i++)
			if (rega == defaultPointer [0] || rega == defaultPointer [1])
			{
				defaultPointer [0]->_acc [i] = regb->_acc [i];
				defaultPointer [1]->_acc [i] = regb->_acc [i];
			}
			else
				rega->_acc [i] = regb->_acc [i];
		break;

	default:
		break;
	}
}

/*
 * Routine which prints a report error.
 */
void
ReportError (
	char	*s,
	int      pos,
	char	*msg)
{
	if (msg != (char *)0 && strlen (msg) != 0) 
		fprintf (pout,"\n%s\n%*s^ %s\n",s,pos," ",msg);
	else 
		fprintf (pout,"\n%s\n%*s^ Construct not understood\n",s,pos," ");

	currentPosition = 0;
	fflush (pout);
}

/*
 * Return pointer to node that var_name is held
 * or pointer to node above if var_name is not	
 * in tree yet ...				|
 */
TPTR	
FindTree (
	TPTR	tptr,
	char	*var_name)
{
	/*
	 * Should ONLY happen for root of tree
	 */
	if (tptr == NUL_TREE)
		return (NUL_TREE);

	/*
	 * Found variable
	 */
	if (strcmp (var_name,tptr->_var_name) == 0)
		return (tptr);

	/*
	 * Left Sub Tree ??
	 */
	if (strcmp (var_name,tptr->_var_name) < 0)
	{
		if (tptr->_lptr == NUL_TREE)
			return (tptr);
		else
			return (FindTree (tptr->_lptr,var_name));
	}

	/*
	 * Right Sub Tree.
	 */
	if (strcmp (var_name,tptr->_var_name) > 0)
	{
		if (tptr->_rptr == NUL_TREE)
			return (tptr);
		else
			return (FindTree (tptr->_rptr,var_name));
	}
	return (NUL_TREE);
}

/*
 * check if variable 'var_name' is in tree return pointer to it if it 
 * was else return NUL_TREE.					               
 */
TPTR	
FindVariable (
	char	*var_name,
	int		*len)
{
	char	vble_name [VARLEN + 1];
	TPTR	tptr;

	*len = GlobalUpShift (var_name);

	if (*len > VARLEN)
		*len = VARLEN;

	sprintf (vble_name,"%-*.*s",*len,*len,var_name);

	tptr = FindTree (var_head,vble_name);

	if (tptr != NUL_TREE &&
        strncmp (vble_name,tptr->_var_name,*len) == 0 && 
        strlen (tptr->_var_name) == (unsigned int) *len)
		return (tptr);
	else
		return (NUL_TREE);
}

/*
 * Add var_name onto tree. 
 * return 1 if already there 
 * return 0 if not already there. 
 * return -1 if out of memory.
 */
int
AddTree (
	char	*var_name)
{
	int		len;
	char	vble_name [VARLEN + 1];
	TPTR	tptr;

	len = GlobalUpShift (var_name);

	if (len > VARLEN)
		len = VARLEN;

	sprintf (vble_name,"%-*.*s", len, len, var_name);

	tptr = FindTree (var_head,vble_name);

	/*
	 * If var_name already in the tree
	 */
	if (tptr != NUL_TREE &&
        strncmp (vble_name,tptr->_var_name,len) == 0 &&
        strlen (tptr->_var_name) == len)
	{
		currentReg = tptr;
		return (EXIT_FAILURE);
	}

	/*
	 * Left sub tree
	 */
	if (tptr != NUL_TREE && strcmp (vble_name,tptr->_var_name) < 0)
	{
		tptr->_lptr = TreeAllocate ();
		/*
		 * Out of Memory
		 */
		if (tptr->_lptr == NUL_TREE)
		{
			ReportError (vble_name,0,"Too many Variables Used.");
			return (-1);
		}
		tptr = tptr->_lptr;
	}
	else
	{
		/*
		 * Right sub tree
		 */
		if (tptr != NUL_TREE && strcmp (vble_name,tptr->_var_name) > 0)
		{
			tptr->_rptr = TreeAllocate ();
			/*
			 * Out of Memory
			 */
			if (tptr->_rptr == NUL_TREE)
			{
				ReportError (vble_name,0,"Too many Variables Used.");
				return (-1);
			}
			tptr = tptr->_rptr;
		}
		else
		{
			if (tptr == NUL_TREE)
			{
				var_head = TreeAllocate ();
				/*
				 * Out of Memory
				 */
				if (var_head == NUL_TREE)
				{
					ReportError (vble_name,0,"Too many Variables Used.");
					return (-1);
				}
				tptr = var_head;
			}
		}
	}

	tptr->_var_name = strdup (vble_name);
	/*
	 * Out of Memory
	 */
	if (tptr->_var_name == (char *)0)
	{
		ReportError (vble_name,0,"Too many Variables Used.");
		return (-1);
	}
	tptr->_lptr = NUL_TREE;
	tptr->_rptr = NUL_TREE;
	currentReg = tptr;
	return (EXIT_SUCCESS);
}

void
InOrder (
	TPTR	tptr,
	int     indent)
{
	if (indent == 0)
		fprintf (stderr,"Inorder \n\n");

	if (tptr == NUL_TREE)
		return;

	InOrder (tptr->_lptr,indent + 3);
	fprintf (stderr,"%*.*s- [%s]\n\n",indent,indent," ",tptr->_var_name);
	InOrder (tptr->_rptr,indent + 3);
}

/*
 * Change ALL lower case to upper case in str
 */
int
GlobalUpShift (
	char	*str)
{
	char	*sptr = str;

	while (sptr != (char *)0 && *sptr && is_vble (*sptr))
	{
		*sptr = toupper (*sptr);
		sptr++;
	}
	return (sptr - str);
}

/*
 * Allocate memory for tree node
 */
TPTR	
TreeAllocate (void)
{
	return ((struct tnode *) malloc (sizeof (struct tnode)));
}

/*
 * Calculate the actual value of special register n.	
 */
double 
CalculateSpecialReg (
	int	n)
{
	char	*sptr,
			last_op = '+';
	double	answer = 0.00,
			tmp_val = 0.00;
	int	i;

	if (n > (lastSpecialRegister + 1))
	{
		ReportError (ioline, 0, "Special Register Used Without Declaration");
		return (answer);
	}
	sptr = specialRegisters [n - 1];
	while (*sptr)
	{
		if (*sptr == ' ')
		{
			sptr++;
			continue;
		}
		switch (*sptr)
		{
		case	'+':
		case	'-':
		case	'*':
		case	'/':
			switch (last_op)
			{
			case	'+':
				answer += tmp_val;
				break;

			case	'-':
				answer -= tmp_val;
				break;

			case	'*':
				answer *= tmp_val;
				break;

			case	'/':
				if (tmp_val != 0.00)
					answer /= tmp_val;
				else
					answer = 0.00;
				break;
			}
			tmp_val = 0.00;
			last_op = *sptr;
			sptr++;
			break;

		case	'\\':
			sptr++;
			switch (*sptr)
			{
			case	'V':
				sptr++;
				if (*sptr == 'T')
				{
					tmp_val = defaultReg->_acc [VT];
					sptr++;
				}
				else
				{
					i = atoi (sptr);
					sptr += (i >= 10) ? 2 : 1;
					i += (V_BIAS - 1);
					tmp_val = defaultReg->_acc [i];
				}
				break;

			case	'B':
				/*
				 * Brought Forward	
				 */
				sptr++;
				if (*sptr == 'F')
				{
					tmp_val = defaultReg->_acc [BF];
					sptr++;
					break;
				}
				if (*sptr == 'T')
				{
					i = (* (sptr + 1) == '1') ? BT1 : BT2;
					tmp_val = defaultReg->_acc [i];
					sptr += 2;
				}
				else
				{
					i = atoi (sptr);
					sptr += (i >= 10) ? 2 : 1;
					i += (i < 13) ? (B1_BIAS - 1) : (B2_BIAS - 13);
					tmp_val = defaultReg->_acc [i];
				}
				break;

			case	'F':
				sptr++;
				if (*sptr == 'T')
				{
					i = (* (sptr + 1) == '1') ? FT1 : FT2;
					tmp_val = defaultReg->_acc [i];
					sptr += 2;
				}
				else
				{
					i = atoi (sptr);
					sptr += (i >= 10) ? 2 : 1;
					i += (i < 13) ? (F1_BIAS - 1) : (F2_BIAS - 13);
					tmp_val = defaultReg->_acc [i];
				}
				break;

			case	'R':
				tmp_val = defaultReg->_acc [R12];
				sptr += 3;
				break;

			case	'S':
				i = atoi (sptr + 1);
				sptr += (i > 9) ? 3 : 2;
				if (i >= n)
				{
					ReportError (ioline, 0, "Circular special register reference");
					tmp_val = 0.00;
				}
				else
					tmp_val = CalculateSpecialReg (i);
				break;

			default:
				ReportError (ioline, 0, "Illegal \\ register usage");
				tmp_val = 0;
				break;
			}
			break;

		case	'$':
			sptr++;
			i = atoi (sptr);
			if (i < 1 || i > USEACCS)
			{
				ReportError (ioline, 0, "Illegal register usage");
				return (answer);
			}
			sptr++;
			if (i > 9)
				sptr++;
			tmp_val = defaultReg->_acc [i - 1];
			break;

		default:
			if (isdigit (*sptr))
				sptr = NumberConvert (sptr, &tmp_val);
			else
			{
				ReportError (ioline, 0, "Illegal calculation");
				*sptr = 0;
			}
		}
	}

	switch (last_op)
	{
	case	'+':
		answer += tmp_val;
		break;

	case	'-':
		answer -= tmp_val;
		break;

	case	'*':
		answer *= tmp_val;
		break;

	case	'/':
		if (tmp_val != 0.0)
			answer /= tmp_val;
		else
			answer = 0.0;

		break;
	}

	return (answer);
}

char *
NumberConvert (
	char	*sptr,
	double	*value)
{
	double	hold_val = 0.00;
	int	past_dp = 0;

	while (*sptr)
	{
		if (isdigit (*sptr))
		{
			hold_val *= 10;
			hold_val += (*sptr - '0');
			if (past_dp)
				past_dp++;
			sptr++;
			continue;
		}
		if (*sptr == '.' && past_dp == 0)
		{
			past_dp = 1;
			sptr++;
			continue;
		}
		while (past_dp > 1)
		{
			hold_val /= 10;
			past_dp--;
		}
		*value = hold_val;
		return (sptr);
	}
	while (past_dp > 1)
	{
		hold_val /= 10;
		past_dp--;
	}
	*value = hold_val;
	return (sptr);
}
