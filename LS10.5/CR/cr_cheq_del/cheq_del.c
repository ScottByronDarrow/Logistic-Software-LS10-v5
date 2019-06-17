/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cheq_del.c,v 5.2 2001/08/09 08:51:34 scott Exp $
|  Program Name  : (cr_cheq_del.c)
|  Program Desc  : (Suppliers Cheque Listings Delete)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 27/01/87         |
|---------------------------------------------------------------------|
| $Log: cheq_del.c,v $
| Revision 5.2  2001/08/09 08:51:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:11  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cheq_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_cheq_del/cheq_del.c,v 5.2 2001/08/09 08:51:34 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_cr_mess.h>

#define	DEL_OK	 (((comm_rec.crd_date - suhp_rec.date_payment) > 90L) && \
                    suhp_rec.presented [0] == 'Y' && \
                    suhp_rec.stat_flag [0] == 'D')

#include	"schema"

struct commRecord	comm_rec;
struct suhpRecord	suhp_rec;
struct suhtRecord	suht_rec;

char	*fifteenSpaces	=	"               ";
	

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			 (void);
void	CloseDB			 (void);
void	shutdown_prog	 (void);
void	proc_file		 (void);


/*==========================
| Main processing routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	DspString [21];
	
	OpenDB ();

	dsp_screen (mlCrMess705, comm_rec.co_no, comm_rec.co_name);
	strcpy (DspString, ML (mlCrMess706));

	strcpy (suhp_rec.co_no, comm_rec.co_no);
	strcpy (suhp_rec.cheq_no, fifteenSpaces);

	cc = find_rec (suhp, &suhp_rec, GTEQ, "u");
	while (!cc && !strcmp (suhp_rec.co_no, comm_rec.co_no))
	{
		strcpy (suhp_rec.stat_flag, "D");
		cc = abc_update (suhp, &suhp_rec);
		if (cc)
			file_err (cc, suhp, "DBUPDATE");
	
		dsp_process (DspString,suhp_rec.cheq_no);

		cc = find_rec (suhp, &suhp_rec, NEXT, "u");
	}

	strcpy (suhp_rec.co_no, comm_rec.co_no);
	strcpy (suhp_rec.cheq_no, fifteenSpaces);
	cc = find_rec (suhp, &suhp_rec, GTEQ, "u");
	while (!cc && !strcmp (suhp_rec.co_no, comm_rec.co_no))
	{
	   	dsp_process (DspString,suhp_rec.cheq_no);

		if (DEL_OK)
		{
	     	abc_delete (suhp);
	    	proc_file ();
			abc_unlock (suhp);
	   		cc = find_rec (suhp, &suhp_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (suhp);
	   		cc = find_rec (suhp, &suhp_rec, NEXT, "u");
		}
	}
	abc_unlock (suhp);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (suhp, suhp_list, SUHP_NO_FIELDS, "suhp_id_no");
	open_rec (suht, suht_list, SUHT_NO_FIELDS, "suht_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (suhp);
	abc_fclose (suht);
	abc_dbclose ("data");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*===========================
| Validate and print lines. |
===========================*/
void
proc_file (
 void)
{
	suht_rec.hhsq_hash = suhp_rec.hhsq_hash;
	strcpy (suht_rec.est_no, "  ");

    cc = find_rec (suht, &suht_rec, GTEQ, "r");
    while (!cc && suht_rec.hhsq_hash == suhp_rec.hhsq_hash)
    {
		abc_delete (suht);
    	cc = find_rec (suht, &suht_rec, GTEQ, "r");
	}
}
