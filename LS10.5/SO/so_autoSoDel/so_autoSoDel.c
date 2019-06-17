/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: so_autoSoDel.c,v 5.4 2002/04/30 07:56:45 scott Exp $
|  Program Name  : (so_autoSoDel.c)
|  Program Desc  : (Automatic Delete of Sales orders.)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 21 April 2001    |
|---------------------------------------------------------------------|
| $Log: so_autoSoDel.c,v $
| Revision 5.4  2002/04/30 07:56:45  scott
| Update for new Archive modifications;
|
| Revision 5.3  2002/04/29 07:47:13  scott
| Update for new Archive modifications;
|
| Revision 5.2  2001/08/09 09:20:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:50:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:38  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 1.1  2001/04/21 03:50:53  scott
| New Program for XML delete of sales orders
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_autoSoDel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_autoSoDel/so_autoSoDel.c,v 5.4 2002/04/30 07:56:45 scott Exp $";

#include 	<pslscr.h>
#include 	<so_autoSoDel.h>
#include 	<proc_sobg.h>
#include 	<Archive.h>

#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct inlaRecord	inla_rec;

/*
 * Function Declarations 
 */
void 	shutdown_prog 			(void);
void	StartProgram 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
int 	DeleteFunc 				(long);
int		PRError 				(int);

/*
 * Main Processing Routine
 */
int
main (
 int  argc, 
 char *argv [])
{
	long	hhsoHash	=	0L;

	StartProgram ();
	
	while (scanf ("%ld", &hhsoHash) != EOF)
	{
		cc = DeleteFunc (hhsoHash);
		if (cc)
			return (cc);
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Start up program, Open database files, read default accounts, open audit. 
 */
void
StartProgram (void)
{
	/*
	 * Open database files. 
	 */
	OpenDB ();
}
/*
 * Program exit sequence	
 */
void
shutdown_prog (void)
{
	recalc_sobg ();
	ArchiveClose ();
	CloseDB (); 
	FinishProgram ();
}
/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (inla,  inla_list, INLA_NO_FIELDS, "inla_hhsl_id");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (inla);
	abc_dbclose ("data");
}

/*
 * Process deleted invoices. 
 */
int
DeleteFunc (
	long	hhsoHash)
{
	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_SOD_NO_ORDER));

	/*
	 * Process all lines and delete as required. 
	 */
	soln_rec.hhso_hash 	= sohr_rec.hhso_hash;
	soln_rec.line_no 	= -1;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		if (soln_rec.status [0] != 'P')
		{
			add_hash
			(
				comm_rec.co_no,
				comm_rec.est_no,
				"RC",
				0,
				soln_rec.hhbr_hash,
				soln_rec.hhcc_hash,
				0L,
				(double) 0
			);
			inla_rec.hhsl_hash	=	soln_rec.hhsl_hash;
			inla_rec.inlo_hash	=	0L;
			cc = find_rec (inla, &inla_rec, GTEQ, "u");
			while (!cc && inla_rec.hhsl_hash ==	soln_rec.hhsl_hash)
			{
				cc = abc_delete (inla);
				if (cc)
					file_err (cc, inla, "DBDELETE");

				inla_rec.hhsl_hash	=	soln_rec.hhsl_hash;
				inla_rec.inlo_hash	=	0L;
				cc = find_rec (inla, &inla_rec, GTEQ, "u");
			}
			abc_unlock (inla);

			cc = ArchiveSoln (soln_rec.hhsl_hash);
			if (cc)
				file_err (cc, soln, "ARCHIVE");

			cc = abc_delete (soln);
			if (cc)
				file_err (cc, soln, "DBDELETE");

			soln_rec.hhso_hash 	= sohr_rec.hhso_hash;
			soln_rec.line_no 	= -1;
			cc = find_rec (soln, &soln_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
		}
	}
	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = -1;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	if (cc || soln_rec.hhso_hash != sohr_rec.hhso_hash)
	{
		sohr_rec.hhso_hash	=	hhsoHash;
		cc = find_rec (sohr, &sohr_rec , COMPARISON, "u");
		if (cc)
		{
			abc_unlock (sohr);
			return (PRError (ERR_SOD_DELETE));
		}
		else
		{
			cc = ArchiveSohr (sohr_rec.hhso_hash);
			if (cc)
				file_err (cc, sohr, "ARCHIVE");

			cc = abc_delete (sohr);
			if (cc)
				return (PRError (ERR_SOD_DELETE));
		}
	}
	return (EXIT_SUCCESS);
}
/*
 * Process Errors 
 */
int
PRError (
	int	errCode)
{
	if (ERR_SOD_NO_ORDER == errCode)
	{
		fprintf (stderr, "Sales order hash (sohr_hhso_hash) must be valid\n");
		fprintf (stderr, "sohr_hhso_hash = [%ld]\n", sohr_rec.hhso_hash);
		return (errCode);
	}
	if (ERR_SOD_STATUS == errCode)
	{
		fprintf (stderr, "Sales order status not valid for delete\n");
		fprintf (stderr, "sohr_status = [%s]\n", sohr_rec.status);
		return (errCode);
	}
	if (ERR_SOD_DELETE == errCode)
	{
		fprintf (stderr, "Sales order Delete failed.\n");
		fprintf (stderr, "sohr_hhso_hash = [%ld]\n", sohr_rec.hhso_hash);
		return (errCode);
	}
	return (errCode);
}
