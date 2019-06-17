/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: FindSerial.h,v 5.0 2001/06/19 06:51:17 cha Exp $
-----------------------------------------------------------------------
| $Log: FindSerial.h,v $
| Revision 5.0  2001/06/19 06:51:17  cha
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
*/

int		FindInsf (long, char *, char *, char *);

/*=======================================
|	if hhwhHash != 0L					|
|		load hhwhHash,serialNo,status	|
|										|
|	if serialNo != NULL					|
|		find COMPARISON					|
|	else								|
|		if hhwhHash != 0L				|
|			find GTEQ					|
|		else							|
|			find NEXT					|
=======================================*/
int
FindInsf (
	long	hhwhHash, 
	char	*serialNo, 
	char	*serialStatus, 
	char	*findType)
{
	int	noSeral;

	noSeral = (serialNo == (char *)0 || strlen (serialNo) == 0);

	if (hhwhHash != 0L)
	{
		insf_rec.hhwh_hash = hhwhHash;
		sprintf (insf_rec.status,	"%-1.1s",serialStatus);
		sprintf (insf_rec.serial_no,"%-25.25s", (!noSeral) ? serialNo : " ");
	}

	/*---------------------------------------
	|	if serialNo != NULL					|
	|		find COMPARISON					|
	|	else								|
	|		find NEXT or GTEQ (on hhwhHash)	|
	---------------------------------------*/

	if (noSeral)
		cc = find_rec ("insf", &insf_rec, (hhwhHash) ? GTEQ : NEXT, findType);
	else
		cc = find_rec ("insf", &insf_rec, COMPARISON, findType);

	/*-------------------
	| if error in find	|
	-------------------*/
	if (cc)
		return (cc);

	/*---------------------------------------------------
	| return FALSE if status matches status required	|
	---------------------------------------------------*/
	return ((insf_rec.status[0] != serialStatus[0]) ? -1 : 0);
}
