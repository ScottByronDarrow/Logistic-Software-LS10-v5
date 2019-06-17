/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: FindInsfCost.h,v 5.0 2001/06/19 06:51:16 cha Exp $
|----------------------------------------------------------------------
| $Log: FindInsfCost.h,v $
| Revision 5.0  2001/06/19 06:51:16  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:28:50  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:15:34  gerry
| Force revision no. to 2.0 - Rel-15072000
|
| Revision 1.2  2000/05/29 09:25:48  scott
| Updated to add new functions to move include files into library. Process will allow GVision to have more processes in the back end server.
|
| Revision 1.1  2000/05/29 01:48:18  scott
| Updated include files to make things like searches and item lookup standard.
| All include files will work with standard app.schema defines.
| -------------------------------------------------------------------------------
| | New Inc file name	| Old Inc File Name |  Old Func. Name  | New Func. Name   |
| |-------------------|-------------------|------------------|------------------|
| | FindFifo.h        | find_fifo.h       | find_incf ()     | FindFifo ()      |
| |-------------------|-------------------|------------------|------------------|
| | FindFifoCost.h    | find_f_cost.h     | find_f_cost ()   | FindFifoCost ()  |
| |-------------------|-------------------|------------------|------------------|
| | FindInmr.h        | find_inmr.h       | find_inmr ()     | FindInmrC ()     |
| |                   | find_inmr2.h      | sup_alt_err ()   | FindInmr ()      |
| |                   | find_inmr3.h      |                  | SuperSynonymError|
| |                   | find_inmr_bc.h    |                  |                  |
| |-------------------|-------------------|------------------|------------------|
| | FindInsfCost.h    | find_s_cost.h     | find_s_cost ()   | FindSerialCost() |
| |-------------------|-------------------|------------------|------------------|
| | FindSerial.h      | find_serial.h     | find_serial ()   | FindInsf ()      |
| |-------------------|-------------------|------------------|------------------|
| | UpdateInsf.h      | up_serial.h       | up_serial ()     | UpdateInsf ()    |
| |-------------------|-------------------|------------------|------------------|
| | LRPFunctions.h    | LRP_LSA.c         | No changes       | No changes       |
| |-------------------|-------------------|------------------|------------------|
| | SearchInmr.h      | inmr_search.h     | srck_search ()   | InmrSearchC ()   |
| |                   | inmr_search2.h    |                  | InmrSearch ()    |
| |                   | inmr_search3.h    |                  |                  |
| |                   | inmr_search4.h    |                  |                  |
|
=====================================================================*/
#include <UpdateInsf.h>

int		serialEnvironmentOpen	=	FALSE;
char	envVarSerialValue[2];
void	OpenSerialEnv	(void);
double	FindSerialCost 	(long, int);

/*===========================
| Find Serial Item Costing. |
===========================*/
double	
FindSerialCost (
	long	hhwhHash, 
	int		serialAverage)
{
	int	rc = 0;
	double	serialNo	=	0.00,
			serialCost	=	0.00;


	/*-------------------------------
	| Read serial environment once. |
	-------------------------------*/
	OpenSerialEnv ();

	rc = FindInsf (hhwhHash, (char *) NULL, "F", "r");

	while (!rc && hhwhHash == insf_rec.hhwh_hash)
	{
		if (envVarSerialValue[0] == 'E')
			serialCost += insf_rec.est_cost;
		else
			serialCost += (insf_rec.act_cost != 0.00) 
							? insf_rec.act_cost : insf_rec.est_cost;
		serialNo++;
		rc = FindInsf (0L, (char *) NULL, "F", "r");
	}
	rc = FindInsf (hhwhHash, (char *) NULL, "C", "r");

	while (!rc && hhwhHash == insf_rec.hhwh_hash)
	{
		if (envVarSerialValue[0] == 'E')
			serialCost += insf_rec.est_cost;
		else
			serialCost += (insf_rec.act_cost != 0.00) ? insf_rec.act_cost : insf_rec.est_cost;
		serialNo++;
		rc = FindInsf (0L, (char *) NULL, "C", "r");
	}

	if (serialAverage && serialNo != 0.00)
		serialCost /= serialNo;

	return ((serialNo) ? serialCost : 0.00);
}

/*===============================
| Read serial environment once. |
===============================*/
void
OpenSerialEnv (void)
{
	char	*sptr;

	if (serialEnvironmentOpen	==	TRUE)
		return;
	
	/*---------------------------------------
	| Get Serial Value Costing Environment. |
	---------------------------------------*/
	sptr	=	chk_env ("SER_VALUE");
	if (sptr == (char *)0)
		strcpy (envVarSerialValue, "E");
	else
		sprintf (envVarSerialValue, "%-1.1s", sptr);

	serialEnvironmentOpen	=	TRUE;
}
