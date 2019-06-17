/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: FindInmr.h,v 5.0 2001/06/19 06:51:16 cha Exp $
-----------------------------------------------------------------------
| $Log: FindInmr.h,v $
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
| Revision 1.4  2000/06/08 00:43:16  scott
| Updated to include FindSupercession in header to external programs can use it.
|
| Revision 1.3  2000/05/30 01:03:27  scott
| Updated to add check_search define.
|
| Revision 1.2  2000/05/29 09:25:48  scott
| Updated to add new functions to move include files into library. Process will allow GVision to have more processes in the back end server.
|
*/

#ifndef	_FindInmr_h
#define	_FindInmr_h

/*----------------------------------------------------------
| SearchFindClose () should be called in CloseDB () routine. |
----------------------------------------------------------*/
void		SearchFindClose		(void);
extern int check_search (char *, char *, int *);
int			FindSupercession	(char *, char *, int);

/*----------------------------------------------------------------------------
  FindInmr (companyNumber, itemNumber, customerHhcuHash, CustomerCodeFlag )   
  e.g. cc = 	FindInmr
				(
					comm_rec.co_no,
					inmr_rec.item_no,
					cumr_rec.hhcu_hash,
					cumr_rec.item_codes
				);
		cc =	FindInmr
				{
					comm_rec.co_no,
					inmr_rec.item_no,
					0L,
					"N"
				};
----------------------------------------------------------------------------*/
int			FindInmr			(char *,char *, long, char *);

/*----------------------------------------------------------------------------
  InmrSearch (companyNumber, itemNumber, customerHhcuHash, CustomerCodeFlag )   
  e.g. InmrSearch
		(
			comm_rec.co_no,
			inmr_rec.item_no,
			cumr_rec.hhcu_hash,
			cumr_rec.item_codes
		);
		InmrSearch
		{
			comm_rec.co_no,
			inmr_rec.item_no,
			0L,
			"N"
		};
----------------------------------------------------------------------------*/
void		InmrSearch			(char *,char *, long, char *);

void		SuperSynonymError	(void);

#endif	/*	_FindInmr_h	*/

