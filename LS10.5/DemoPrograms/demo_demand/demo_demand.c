/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: demo_demand.c,v 5.4 2002/09/16 05:04:44 scott Exp $
|  Program Desc  : (Generate demo demand for lrp.                 )   |
-----------------------------------------------------------------------
| $Log: demo_demand.c,v $
| Revision 5.4  2002/09/16 05:04:44  scott
| Updated to create daily demend for current month
|
| Revision 5.3  2002/07/17 09:42:05  scott
| .
|
| Revision 5.2  2001/08/21 00:04:46  scott
| Updated for development related to bullet proofing
|
| Revision 5.1  2001/08/07 00:04:48  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:47  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 3.1  2000/10/25 03:56:40  scott
| Updated to generate demo data for ffdm;
|
| Revision 3.0  2000/10/10 12:14:30  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 1.1  2000/08/08 08:08:50  scott
| new programs for creation of demo data for lrp.
|
 */
#define	CCMAIN
char	*PNAME = "$RCSfile: demo_demand.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DemoPrograms/demo_demand/demo_demand.c,v 5.4 2002/09/16 05:04:44 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct inccRecord	incc_rec;
struct ffdmRecord	ffdm_rec;


#define	MAX_VALUE	1000
#define	MIN_VALUE	0

	char	*data	= "data";

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void	ProcessCreateData	(void);
void	AddFfdm 			(long);
int		heading 			(int);

long	datesCreate [37];

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();

	OpenDB ();

	ProcessCreateData ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); FinishProgram ();;
	crsr_on ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen(data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	open_rec (ffdm, ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");

}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ffdm);
	abc_fclose (incc);

	abc_dbclose(data);
}

void
ProcessCreateData (void)
{
	long	startDate	=	0L;
	int		i;

	startDate	=	TodaysDate ();
	startDate	= 	MonthStart (startDate) - 1;

	for (i = 0; i < 36; i++)
	{
		datesCreate [i] = MonthStart (startDate);
		startDate		= datesCreate [i] - 1;
	}

	incc_rec.hhcc_hash = 0L;
	incc_rec.hhbr_hash = 0L;
	cc = find_rec ("incc", &incc_rec, GTEQ, "r");
	while (!cc)
	{
		AddFfdm (MAX_VALUE);
		cc = find_rec ("incc", &incc_rec, NEXT, "r");
	}
	startDate	=	TodaysDate ();
	startDate   -=	36;
	for (i = 0; i < 36; i++)
		datesCreate [i] = startDate + i;

	incc_rec.hhcc_hash = 0L;
	incc_rec.hhbr_hash = 0L;
	cc = find_rec ("incc", &incc_rec, GTEQ, "r");
	while (!cc)
	{
		AddFfdm (MAX_VALUE / 36);
		cc = find_rec ("incc", &incc_rec, NEXT, "r");
	}
}


void
AddFfdm (long	maxValue)
{

	long	valueData		=	0L,
			lastDataValue	=	0L,
			upperValue		=	0L,
			lowerValue		=	0L;

	long	lrand48 (void);
	int		i;

	for (i = 0; i < 36; i++)
	{
		valueData =	lrand48 () % maxValue;
		lowerValue	=	lastDataValue * .9;
		upperValue	=	lastDataValue / .9;
		while (i && (valueData < lowerValue || valueData > upperValue))
			valueData =	lrand48 () % maxValue;
		
		while (!valueData)
		{
			valueData =	lrand48 () % maxValue;
			lowerValue	=	lastDataValue * .9;
			upperValue	=	lastDataValue / .9;
			while (i && (valueData < lowerValue || valueData > upperValue))
				valueData =	lrand48 () % maxValue;
		}
		lastDataValue = valueData;

		ffdm_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		ffdm_rec.hhcc_hash	=	incc_rec.hhcc_hash;
		ffdm_rec.date		=	datesCreate [i];
		strcpy (ffdm_rec.type, "1");
		cc = find_rec ("ffdm", &ffdm_rec, COMPARISON, "u");
		if (cc)
		{
			ffdm_rec.qty	=	(float) valueData;
			abc_add ("ffdm", &ffdm_rec);
		}
		else
		{
			ffdm_rec.qty	=	(float) valueData;
			abc_update ("ffdm", &ffdm_rec);
		}
	}
}
