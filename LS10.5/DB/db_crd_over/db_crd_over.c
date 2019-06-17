/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crd_over.c,v 5.2 2001/11/22 06:39:26 scott Exp $
|  Program Name  : (db_crd_over.c)
|  Program Desc  : (Display Customer Over Credit Limit)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_crd_over.c,v $
| Revision 5.2  2001/11/22 06:39:26  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.1  2001/11/22 06:38:09  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crd_over.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crd_over/db_crd_over.c,v 5.2 2001/11/22 06:39:26 scott Exp $";

#include <pslscr.h>

	char	branchNo [3];

	int		envDbCo = 0;
	int		envDbFind = 0;

	double	totalOwing = 0.00;
	double	cumr_bo [6];

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;

char	*cumr2	=	"cumr2",
		*data	=	"data";
/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 	(void);
void 	ProcessFile 	(void);
void 	SaveData 		(double);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	CheckCumr 		(void);

int
main (
 int                argc,
 char*              argv [])
{
	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	init_scr 	();
	swide 		();
	clear 		();
	crsr_off 	();
	set_tty 	();
	ProcessFile ();
	snorm 		();
	rset_tty 	();
	shutdown_prog ();

    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB ();
	FinishProgram ();
}

void
ProcessFile (void)
{
	int	i;

	Dsp_open (0,1,16);
	Dsp_saverec (" CUST. |            NAME                    |      CONTACT NAME   |     PHONE      |   CREDIT  | BALANCE   |  CREDIT REFERENCE   ");
	Dsp_saverec ("NUMBER |                                    |                     |     NUMBER     |   LIMIT   |           |                     ");
	Dsp_saverec (" [NEXT] [PREV] [EDIT/END]");

	strcpy (cumr_rec.co_no,comm_rec.co_no);
	strcpy (cumr_rec.est_no,"   ");
	strcpy (cumr_rec.dbt_acronym,"         ");
	cc = find_rec (cumr,&cumr_rec,GTEQ,"r");
	while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no))
	{
		if (cumr_rec.ho_dbt_hash > 0L)
		{
			cc = find_rec (cumr,&cumr_rec,NEXT,"r");
			continue;
		}
		/*---------------------------
		| Total head office debtor. |
		---------------------------*/
		for (i = 0; i < 6; i++)
			cumr_bo [i] = cumr_balance [i];

		/*--------------------------
		| Total all child debtors. |
		--------------------------*/
		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			for (i = 0; i < 6; i++)
				cumr_bo [i] += cumr2_balance [i];

			cc = find_rec (cumr2,&cumr2_rec, NEXT, "r");
		}
		if (CheckCumr ())
		    	SaveData (totalOwing);

		cc = find_rec (cumr,&cumr_rec,NEXT,"r");
	}
	Dsp_srch ();
	Dsp_close ();
}

void
SaveData (
 double             limit)
{
	char	wk_str [200];

	sprintf (wk_str,"%6.6s ^E %-35.35s^E %20.20s^E %15.15s^E %10.2f^E %10.2f^E %20.20s",
			cumr_rec.dbt_no,
			cumr_rec.dbt_name,
			cumr_rec.contact_name,
			cumr_rec.phone_no,
			DOLLARS (cumr_rec.credit_limit),
			DOLLARS (limit),
			cumr_rec.credit_ref);
	Dsp_saverec (wk_str);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	open_rec (cumr,cumr_list,CUMR_NO_FIELDS, (!envDbFind) 
									? "cumr_id_no" : "cumr_id_no3");

	open_rec (cumr2,cumr_list,CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_dbclose (data);
}


/*==========================================
| Validate credit period and credit limit. |
==========================================*/
int
CheckCumr (void)
{
	totalOwing = cumr_bo [0] + 
		      	 cumr_bo [1] +
	             cumr_bo [2] + 
	  	      	 cumr_bo [3] +
	  	         cumr_bo [4] +
	  	         cumr_bo [5];

	/*
	 * Check if customer is over his credit limit.
	 */
	if (cumr_rec.credit_limit <= totalOwing && cumr_rec.credit_limit != 0.00 && 
	     cumr_rec.crd_flag [0] != 'Y')
		return (EXIT_FAILURE);

	if (cumr_rec.od_flag && cumr_rec.crd_flag [0] != 'Y')
		return (2);

	return (FALSE);
}
