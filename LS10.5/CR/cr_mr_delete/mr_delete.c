/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: mr_delete.c,v 5.4 2001/12/06 08:05:39 scott Exp $
|  Program Name  : (cr_mr_delete.c) 
|  Program Desc  : (Delete Suppliers From Suppliers Master File.) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
| $Log: mr_delete.c,v $
| Revision 5.4  2001/12/06 08:05:39  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/06 03:43:19  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mr_delete.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_mr_delete/mr_delete.c,v 5.4 2001/12/06 08:05:39 scott Exp $";

#include 	<pslscr.h>	
#include 	<ml_cr_mess.h>	

#include        "schema"

struct sumrRecord       sumr_rec;
struct srcrRecord       srcr_rec;
struct inisRecord       inis_rec;
struct inldRecord       inld_rec;

	/*
	 * Special fields and flags.
	 */
	int		pidNumber,
			workFileNumber;
					
	char	fileName [100];

	/*
	 * Work file record.
	 */
	struct {
		long	hhsuHash;
	} workRec;

/*
 * Local function prototypes
 */
void	ProcessInis		 (long);
void	ProcessInld		 (long);
void	shutdown_prog	 (void);
int		OpenDB			 (void);
void	CloseDB			 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int 	prog_exit = 0,
			i = 0,
			deleteCount = 0;

	char	displaySupplier [17];

	if (argc < 2) 
		return (EXIT_FAILURE);
	else 
		pidNumber = atoi (argv [1]);

	if (OpenDB () == -1)
		return (EXIT_FAILURE);

	prog_exit = 0;

	init_scr ();
	set_tty	();
	clear ();
	crsr_off ();
	centre_at (2, 80, ML (mlCrMess030));
	line_at (3,12,55);

	box (7, 5, 28, 12);
	box (47, 5, 28, 12);

	print_at (6, 9, "%R %s ", ML ("Suppliers Master File"));
	line_at (7,9,25);

	print_at (6, 51, "%R %s ", ML ("Deleted Suppliers"));
	line_at (7,51,20);
	for (i = 0 ; i < 10 ; i++)
	{
		line_at (i + 8, 35,12);
	}
	i = 1,
	deleteCount = 1;

	cc = RF_READ (workFileNumber, (char *) &workRec);
	while (!cc)
	{
		sumr_rec.hhsu_hash = workRec.hhsuHash;
		if (find_rec (sumr, &sumr_rec, COMPARISON, "u"))
		{
			cc = RF_READ (workFileNumber, (char *) &workRec);
			continue;
		}
		sprintf (displaySupplier, "%-6.6s %-9.9s", 
				sumr_rec.crd_no, sumr_rec.acronym);

		sprintf (err_str, "%02d. %-12.12s", deleteCount, displaySupplier);

		us_pr (err_str, 12, i + 7, 1);

		ProcessInis (sumr_rec.hhsu_hash);

		srcr_rec.hhsu_hash =  workRec.hhsuHash;
		cc = find_rec (srcr, &srcr_rec , EQUAL, "u");
		if (!cc)
		{
			cc = abc_delete (srcr);
			if (cc)
				file_err (cc, srcr, "DBDELETE");
		}

		cc = abc_delete (sumr);
		if (cc)
			file_err (cc, sumr, "DBDELETE");
		
		putchar (BELL);

		us_pr (err_str, 12, i + 7, 0);
		us_pr (err_str, 53, i + 7, 1);

		deleteCount++;
		i++;
		if (i > 10)
		{
			for (i = 0 ; i < 10 ; i++)
			{
				print_at (i + 8, 12, "                 ");
				print_at (i + 8, 53, "                 ");
			}
			i = 1;
		}
		cc = RF_READ (workFileNumber, (char *) &workRec);
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
ProcessInis (
	 long	hhsuHash)
{
	inis_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (inis, &inis_rec, GTEQ, "u");
	while (!cc && inis_rec.hhsu_hash == hhsuHash)
	{
		ProcessInld (inis_rec.hhis_hash);
		abc_delete (inis);
		cc = find_rec (inis, &inis_rec, GTEQ, "u");
	}
	abc_unlock (inis);
}

void
ProcessInld (
	long	hhisHash)
{
	inld_rec.hhis_hash = hhisHash;
	inld_rec.ord_date = 0L;
	cc = find_rec (inld, &inld_rec, GTEQ, "u");
	while (!cc && inld_rec.hhis_hash == hhisHash)
	{
		abc_delete (inld);
		inld_rec.hhis_hash = hhisHash;
		inld_rec.ord_date = 0L;
		cc = find_rec (inld, &inld_rec, GTEQ, "u");
	}
	abc_unlock (inld);
}

void
shutdown_prog (void)
{
	abc_unlock (sumr);
	CloseDB (); 
	FinishProgram ();
}

int
OpenDB (void)
{
	char *	sptr = getenv ("PROG_PATH");

	sprintf (fileName,"%s/WORK/crdl%05d",
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);
	
	cc = RF_OPEN (fileName,sizeof (workRec),"r",&workFileNumber);
	if (cc)
		return (-1);

	abc_dbopen ("data");

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (srcr, srcr_list, SRCR_NO_FIELDS, "srcr_hhsu_hash");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_hhsu_hash");
	open_rec (inld, inld_list, INLD_NO_FIELDS, "inld_id_no");

	return (EXIT_SUCCESS);
}

void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (srcr);
	abc_fclose (inis);
	abc_fclose (inld);
	abc_dbclose ("data");
	if ((cc = RF_DELETE (workFileNumber)))
		sys_err ("Error in work_file During (WKCLOSE)", cc, PNAME);
}
