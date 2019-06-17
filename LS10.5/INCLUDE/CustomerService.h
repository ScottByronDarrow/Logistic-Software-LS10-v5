/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( CustomerService.h )                              |
|  Program Desc  : ( Customer Service Header File.                  ) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (06/04/1999)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#ifndef	_CustomerService_h
#define	_CustomerService_h
								/*----------------------------------*/
#define	LOG_CREATE		1		/* Define for order creation.		*/
#define	LOG_PCCREATE	2		/* Define for PS creation.			*/
#define	LOG_ORD_PRINT	3		/* Define for order printing.		*/
#define	LOG_PS_PRINT	4		/* Define for Packing Slip Print.	*/
#define	LOG_INV_PRINT	5		/* Define for Invoice Print.		*/
#define	LOG_DISPATCH	6		/* Define for Dispatch.				*/
#define	LOG_DELIVERY	7		/* Define for Delivery Date.		*/
								/*----------------------------------*/

/*-----------------------------------------------------------------------
|	(1)		hhco_hash	-	Invoice / Packing slip / Credit note hash. 	|
|	(2)		hhso_hash	-	Sales Order hash							|
|	(3)		hhcu_hash	-	Customer Hash								|
|	(4)		Customer Order Ref											|
|	(5)		Consignment Number											|
|	(6)		Carrier Code												|
|	(7)		Delivery Zone												|
|	(8)		Delivery Type Code. See Define Above.                       |
-----------------------------------------------------------------------*/
void	LogCustService	(long, long, long, char *, char *,char *, char *, int);
void	UpdateSosf	(long, long, long);

#endif	/* _CustomerService_h */
