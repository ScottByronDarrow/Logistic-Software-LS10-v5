/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: wjnlprnt.c,v 5.3 2001/08/09 09:14:02 scott Exp $
|  Program Name  : (gl_jnlprnt.c  )
|  Program Desc  : (Standard / General / Accrual Journal)
|                 (Transactions Print)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: wjnlprnt.c,v $
| Revision 5.3  2001/08/09 09:14:02  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:40  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:06  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: wjnlprnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_wjnlprnt/wjnlprnt.c,v 5.3 2001/08/09 09:14:02 scott Exp $";

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_gl_mess.h>

#include	"schema"

struct commRecord	comm_rec;

	/*
	 * Special fields and flags  ##################################.
	 */
   	int		processID		= 0,
			glwkNo			= 0,	
			printerNo 		= 1,
			headingFlag 	= 0,
			dataFound 		= FALSE;
	long	pageNo			= 0,
			oldPageNo		= 0;

	char 	pageStart [11];

   	double 	totalAmount [5]	= {0.0,0.0,0.0,0.0,0.0},
			totalCredit 	= 0.0,
			totalDebit 		= 0.0;

   FILE 	*ftmp;

/*
 * Local Function Prototypes.
 */
int 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadGljc 		(void);
int 	PrintDetails 	(void);
void 	PrintTotal 		(void);
int 	HeadingFunc 	(void);
void 	shutdown_prog 	(void);
void	PrintExtraDesc 	(char *);
char 	*GetAccount 	(char *);

/*
 * Main processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	init_scr ();

	if (argc < 4)
	{
		print_at (0,0,mlGlMess145, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNo	= atoi (argv [2]);
	processID   = atoi (argv [3]);

	sprintf (gljcRec.journ_type,"%-2.2s",argv [1]);

	if (OpenDB ())
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }

	print_mess ("Printing General Ledger Journals.");

	ReadGljc ();
	
	cc = RF_READ (glwkNo, (char *) &glwkRec);
	while (!cc)
	{
		/*
		 * Print journal lines.
		 */
		if (PrintDetails () == 1)
        {
            return (EXIT_FAILURE);
        } 
		cc = RF_READ (glwkNo, (char *) &glwkRec);
	}
	/*
	 * Print  journal control records.
	 */
	if (dataFound)
    {
		PrintTotal ();
    }

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Open Database Files.
 */
int
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	sprintf (filename,"%s/WORK/gl_work%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);

	OpenGljc ();
	OpenGlmr ();

	return (RF_OPEN (filename,sizeof (glwkRec),"r",&glwkNo));
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	GL_Close ();
	RF_CLOSE (glwkNo);
		
	abc_dbclose ("data");
}

/*
 * Get journal control records for jnl types 1, 2 & 3.
 */
void
ReadGljc (void)
{
	strcpy (gljcRec.co_no,comm_rec.co_no);
	cc = find_rec (gljc, &gljcRec, COMPARISON, "r");
	if (cc)
		file_err (cc, gljc, "DBFIND");

	totalAmount [0] = gljcRec.tot_1;
	totalAmount [1] = gljcRec.tot_2;
	totalAmount [2] = gljcRec.tot_3;
	totalAmount [3] = gljcRec.tot_4;
	totalAmount [4] = gljcRec.tot_5;
}

int
PrintDetails (void)
{
	if (strcmp (glwkRec.co_no,comm_rec.co_no))
    	return (EXIT_SUCCESS);

	if (headingFlag != 1)
    {
		if (HeadingFunc () == 1)
        {
        	return (EXIT_FAILURE);
        }
    }

	strcpy (pageStart,glwkRec.sys_ref);
	pageNo = atol (pageStart);
	if (pageNo != oldPageNo)
	{
		fprintf (ftmp,".PG%ld\n",pageNo);
		oldPageNo = pageNo;
	}

	if (strncmp (glwkRec.acc_no,"                ",MAXLEVEL))
	{
		fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
		fprintf (ftmp,"| %-25.25s ",GetAccount (glwkRec.acc_no));
		fprintf (ftmp,"|   %20.20s  ",glwkRec.narrative);
		fprintf (ftmp,"|%-15.15s",glwkRec.user_ref);
		if (glwkRec.jnl_type [0] == '1')
		{
			fprintf 
			(
				ftmp,
				"| %14.14s ",
				comma_fmt (DOLLARS (glwkRec.loc_amount), "NNN,NNN,NNN.NN")
			);
			fprintf (ftmp,"|                ");
			totalDebit += glwkRec.loc_amount;
		}
		else
		{
			fprintf (ftmp,"|                ");
			fprintf 
			(
				ftmp,
				"| %14.14s ", 
				comma_fmt (DOLLARS (glwkRec.loc_amount), "NNN,NNN,NNN.NN")
			);
			totalCredit += glwkRec.loc_amount;
		}
		fprintf 
		(
			ftmp,
			"| %2.2d",
			mth2fisc (atoi (glwkRec.period_no), comm_rec.fiscal)
		);
		fprintf (ftmp,"  |\n");
		dataFound = TRUE;

		if (strlen (clip (glwkRec.alt_desc1)))
			PrintExtraDesc (glwkRec.alt_desc1);

		if (strlen (clip (glwkRec.alt_desc2)))
			PrintExtraDesc (glwkRec.alt_desc2);

		if (strlen (clip (glwkRec.alt_desc3)))
			PrintExtraDesc (glwkRec.alt_desc3);
	
	}
	else
	{
		fprintf (ftmp, "|                |                           ");
		fprintf (ftmp, "| %-20.20s    |               ", glwkRec.narrative);
		fprintf (ftmp, "|                |                |     |\n");
	}
    return (EXIT_SUCCESS);
}

void
PrintExtraDesc (
	char	*extraDesc)
{
	fprintf (ftmp, "|                |                           ");
	fprintf (ftmp, "| %-20.20s    |               ", extraDesc);
	fprintf (ftmp, "|                |                |     |\n");
}
void
PrintTotal (void)
{
	char	tot1 [15],
			tot2 [15];

	strcpy (tot1, comma_fmt (DOLLARS (totalDebit), "NNN,NNN,NNN.NN"));
	strcpy (tot2, comma_fmt (DOLLARS (totalCredit), "NNN,NNN,NNN.NN"));

	fprintf (ftmp, "|----------------|---------------------------");
	fprintf (ftmp, "|-------------------------|---------------");
	fprintf (ftmp, "|----------------|----------------|-----|\n");

	fprintf (ftmp, "| %-14.14s ", ML ("TODAYS TOTALS"));
	fprintf (ftmp, "|                           ");
	fprintf (ftmp, "|                         |               ");
	fprintf (ftmp, "| %14.14s | %14.14s |     |\n", tot1, tot2);

	totalAmount [0] -= totalDebit;
	totalAmount [1] -= totalCredit;

	strcpy (tot1, comma_fmt (DOLLARS (totalAmount [0]), "NNN,NNN,NNN.NN"));
	strcpy (tot2, comma_fmt (DOLLARS (totalAmount [1]), "NNN,NNN,NNN.NN"));

	fprintf (ftmp, "| %-14.14s ", ML ("PROIR BALANCES"));
	fprintf (ftmp, "|                           ");
	fprintf (ftmp, "|                         |               ");
	fprintf (ftmp, "| %14.14s | %14.14s |     |\n", tot1, tot2);

	strcpy (tot1,comma_fmt (DOLLARS (gljcRec.tot_1),"NNN,NNN,NNN.NN"));
	strcpy (tot2,comma_fmt (DOLLARS (gljcRec.tot_2),"NNN,NNN,NNN.NN"));

	fprintf (ftmp, "| %-14.14s ", ML ("MONTH TO DATE "));
	fprintf (ftmp, "|                           ");
	fprintf (ftmp, "|                         |               ");
	fprintf (ftmp, "| %14.14s | %14.14s |     |\n", tot1, tot2);

	fprintf (ftmp,".EOF\n");

	pclose (ftmp);
}

/*
 * Print journal headings.
 */

int
HeadingFunc (void)
{
	/*
	 * Open pipe work file to pformat.
	 */
	strcpy (pageStart,glwkRec.sys_ref);
	pageNo 		= atol (pageStart);
	oldPageNo 	= pageNo ;
	headingFlag = 1;

	if ( (ftmp = popen ("pformat","w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)",cc, PNAME);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	/*
	 * Start output to file.
	 */
	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.gl_date), PNAME);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".LP%d\n",printerNo);
	fprintf (ftmp,".PI12\n");
	fprintf (ftmp,".11\n");
	fprintf (ftmp,".L158\n");
	fprintf (ftmp,".PG%ld\n",pageNo);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s JOURNAL (RUN NUMBER : %s)\n",
				clip (gljcRec.jnl_desc), glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT %s: \n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".B1\n");

	fprintf (ftmp, ".R=============================================");
	fprintf (ftmp, "==========================================");
	fprintf (ftmp, "=========================================\n");

	fprintf (ftmp, "=============================================");
	fprintf (ftmp, "==========================================");
	fprintf (ftmp, "=========================================\n");

	fprintf (ftmp, "|    ACCOUNT     |                           ");
	fprintf (ftmp, "|   DESCRIPTION OF ENTRY  |USER REFERENCE ");
	fprintf (ftmp, "| DEBIT AMOUNT   | CREDIT AMOUNT  |PER. |\n");

	fprintf (ftmp, "|----------------|---------------------------");
	fprintf (ftmp, "|-------------------------|---------------");
	fprintf (ftmp, "|----------------|----------------|-----|\n");

    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Get General Ledger Account No.
 */
char*
GetAccount (
	char	*accno)
{
	strcpy (glmrRec.co_no, comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,accno);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		return (" ");

	return (glmrRec.desc);
}
