/*=====================================================================
|  Copyright (C) 1999 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_BatchRoundFix.c,v 5.0 2002/05/08 01:22:11 scott Exp $
|  Program Name  : (gl_BatchRoundFix.c) 
|  Program Desc  : (Fix local amount rounding)
|---------------------------------------------------------------------|
|  Date Written  : 29th March 2001 | Author       : Scott B Darrow    |
|---------------------------------------------------------------------|
| $Log: gl_BatchRoundFix.c,v $
| Revision 5.0  2002/05/08 01:22:11  scott
| CVS administration
|
| Revision 1.2  2001/08/09 09:49:47  scott
| Updated to add FinishProgram () function
|
| Revision 1.1  2001/06/22 06:27:49  scott
| New program
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_BatchRoundFix.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/gl_BatchRoundFix/gl_BatchRoundFix.c,v 5.0 2002/05/08 01:22:11 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include	<number.h>

#include	"schema"

struct commRecord	comm_rec;
struct glbhRecord	glbh_rec;
struct glblRecord	glbl_rec;

	char	*data	= "data";


static	int		maxLineValue	= -1;
static	Money	checkBalance	= 0.00,
				lastAmount		= 0.00;

/*=====================
| function prototypes |
=====================*/
void 	CloseDB 			(void);
void 	OpenDB 				(void);
void 	shutdown_prog 		(void);
void	ProcessBatch 		(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	/*----------------------
	| Open database files. |
	----------------------*/
	OpenDB ();

	glbh_rec.hhbh_hash	=	0L;
	cc = find_rec (glbh, &glbh_rec, GTEQ, "r");
	while (!cc)
	{
		ProcessBatch (glbh_rec.hhbh_hash);
		cc = find_rec (glbh, &glbh_rec, NEXT, "r");
	}
		
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (glbh, glbh_list, GLBH_NO_FIELDS, "glbh_hhbh_hash");
	open_rec (glbl, glbl_list, GLBL_NO_FIELDS, "glbl_id_no");
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (glbh);
	abc_fclose (glbl);
	abc_dbclose (data);
}
/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
ProcessBatch (
	long	hhbhHash)
{
	maxLineValue	=	-1;
	checkBalance	=	0.00;
	lastAmount		=	0.00;

	glbl_rec.hhbh_hash	=	hhbhHash;
	glbl_rec.line_no	=	0;
	cc = find_rec (glbl, &glbl_rec, GTEQ, "r");
	while (!cc && glbl_rec.hhbh_hash == hhbhHash)
	{
		if (glbl_rec.local_amt > lastAmount)
		{
			maxLineValue = glbl_rec.line_no;
			lastAmount	=	glbl_rec.local_amt;
		}
		if ((atoi (glbl_rec.dc_flag) % 2) == 0)
			checkBalance	-= no_dec (glbl_rec.local_amt);
		else
			checkBalance	+= no_dec (glbl_rec.local_amt);

		cc = find_rec (glbl, &glbl_rec, NEXT, "r");
	}
	if (checkBalance == 0.0)
		return;

	glbl_rec.hhbh_hash 	= glbh_rec.hhbh_hash;
	glbl_rec.line_no   	= maxLineValue;
	cc = find_rec (glbl, &glbl_rec, COMPARISON, "r");
	if (!cc)
	{
		printf ("BATCH [%s][%s][%s][%s]\n\r",
				glbh_rec.co_no, 	glbh_rec.br_no,
				glbh_rec.jnl_type, 	glbh_rec.batch_no);

		printf ("Diff [%.2f] Value Before [%.2f]\n\r", checkBalance, glbl_rec.local_amt);
		glbl_rec.local_amt	-=	checkBalance;

		printf ("Value After [%.2f]\n\r", glbl_rec.local_amt);
	}
	/*
		cc = abc_update (glbl, &glbl_rec);
		if (cc)
			file_err (cc, glbl, "DBUPDATE");
	}
	else
		abc_unlock (glbl);
	*/
}
