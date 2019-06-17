/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( CustService.c  )                                 |
|  Program Desc  : ( Customer Service Logging.                      ) |
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
| $Log: CustService.c,v $
| Revision 5.1  2001/08/06 22:40:51  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:11  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:16  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:17:12  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.4  1999/09/30 04:35:30  scott
| Updated for datejul etc.
|
| Revision 1.3  1999/09/13 09:36:28  scott
| Updated for Copyright
|
=====================================================================*/
#include	<std_decs.h>
#include	<CustomerService.h>

extern	int	cc;

	 /*===========================
	 | Sales Order Service File. |
	 ===========================*/
#define	_SOSF_NO_FIELDS	30

	struct dbview	_sosf_list [_SOSF_NO_FIELDS] =
	{
		{"sosf_hhco_hash"},
		{"sosf_hhso_hash"},
		{"sosf_hhcu_hash"},
		{"sosf_cus_ord_ref"},
		{"sosf_cons_no"},
		{"sosf_carr_code"},
		{"sosf_del_zone"},
		{"sosf_ocre_op_id"},
		{"sosf_ocre_time"},
		{"sosf_ocre_date"},
		{"sosf_pcre_op_id"},
		{"sosf_pcre_time"},
		{"sosf_pcre_date"},
		{"sosf_oprn_op_id"},
		{"sosf_oprn_time"},
		{"sosf_oprn_date"},
		{"sosf_pprn_op_id"},
		{"sosf_pprn_time"},
		{"sosf_pprn_date"},
		{"sosf_pprn_times"},
		{"sosf_iprn_op_id"},
		{"sosf_iprn_time"},
		{"sosf_iprn_date"},
		{"sosf_iprn_times"},
		{"sosf_odes_op_id"},
		{"sosf_odes_time"},
		{"sosf_odes_date"},
		{"sosf_odel_op_id"},
		{"sosf_odel_time"},
		{"sosf_odel_date"}
	};

	struct _tag_sosfRecord
	{
		long	hhco_hash;
		long	hhso_hash;
		long	hhcu_hash;
		char	cus_ord_ref [21];
		char	cons_no [17];
		char	carr_code [5];
		char	del_zone [7];
		char	ocre_op_id [15];
		long	ocre_time;
		Date	ocre_date;
		char	pcre_op_id [15];
		long	pcre_time;
		Date	pcre_date;
		char	oprn_op_id [15];
		long	oprn_time;
		Date	oprn_date;
		char	pprn_op_id [15];
		long	pprn_time;
		Date	pprn_date;
		int		pprn_times;
		char	iprn_op_id [15];
		long	iprn_time;
		Date	iprn_date;
		int		iprn_times;
		char	odes_op_id [15];
		long	odes_time;
		Date	odes_date;
		char	odel_op_id [15];
		long	odel_time;
		Date	odel_date;
	}	_sosf_rec;


void
LogCustService (
	long	hhcoHash,
	long	hhsoHash,
	long	hhcuHash,
	char	*customerOrderRef,
	char	*consignmentNo,
	char	*carrierCode,
	char	*deliveryZone,
	int		serviceType)
{
	int		newSosf;
	long	logDate	=	0L,
			logTime	=	0L;
	char	tempTime [9];
	char	*currentUser	=	getenv ("LOGNAME");

	logDate	=	TodaysDate ();

	strcpy (tempTime, TimeHHMMSS ());
	logTime	=	atot (tempTime);

	if (hhcoHash == 0L && hhsoHash == 0L)
		return;

	if (hhcuHash == 0L)
		return;

	memset (&_sosf_rec, 0, sizeof (_sosf_rec));

	open_rec ("sosf", _sosf_list, _SOSF_NO_FIELDS, "sosf_id_no");

	_sosf_rec.hhco_hash		=	hhcoHash;
	_sosf_rec.hhso_hash		=	hhsoHash;
	_sosf_rec.hhcu_hash		=	hhcuHash;
	newSosf = find_rec ("sosf", &_sosf_rec, COMPARISON, "u");
	if (newSosf)
	{
		if (customerOrderRef == (char *)0)
			strcpy (_sosf_rec.cus_ord_ref, " ");
		else
			sprintf (_sosf_rec.cus_ord_ref, "%-20.20s", customerOrderRef);

		if (consignmentNo == (char *)0)
			strcpy (_sosf_rec.cons_no, " ");
		else
			sprintf (_sosf_rec.cons_no, "%-16.16s", consignmentNo);

		if (carrierCode == (char *)0)
			strcpy (_sosf_rec.carr_code, " ");
		else
			sprintf (_sosf_rec.carr_code, "%-4.4s", carrierCode);

		if (deliveryZone == (char *)0)
			strcpy (_sosf_rec.carr_code, " ");
		else
			sprintf (_sosf_rec.del_zone, "%-6.6s", deliveryZone);
	}
	else
	{
		if (customerOrderRef != (char *)0)
			sprintf (_sosf_rec.cus_ord_ref, "%-20.20s", customerOrderRef);

		if (consignmentNo != (char *)0)
			sprintf (_sosf_rec.cons_no, "%-16.16s", consignmentNo);

		if (carrierCode != (char *)0)
			sprintf (_sosf_rec.carr_code, "%-4.4s", carrierCode);

		if (deliveryZone != (char *)0)
			sprintf (_sosf_rec.del_zone, "%-6.6s", deliveryZone);
	}

	switch (serviceType)
	{
		case	LOG_CREATE:	
				sprintf (_sosf_rec.ocre_op_id, "%-14.14s", currentUser);
				_sosf_rec.ocre_time	=	logTime;
				_sosf_rec.ocre_date	=	logDate;
			break;

		case	LOG_PCCREATE:	
				sprintf (_sosf_rec.pcre_op_id, "%-14.14s", currentUser);
				_sosf_rec.pcre_time	=	logTime;
				_sosf_rec.pcre_date	=	logDate;
			break;

		case	LOG_ORD_PRINT:	
				sprintf (_sosf_rec.oprn_op_id, "%-14.14s", currentUser);
				_sosf_rec.oprn_time	=	logTime;
				_sosf_rec.oprn_date	=	logDate;
			break;

		case	LOG_PS_PRINT:	
				sprintf (_sosf_rec.pprn_op_id, "%-14.14s", currentUser);
				_sosf_rec.pprn_time	=	logTime;
				_sosf_rec.pprn_date	=	logDate;
				_sosf_rec.pprn_times++;
			break;

		case	LOG_INV_PRINT:	
				sprintf (_sosf_rec.iprn_op_id, "%-14.14s", currentUser);
				_sosf_rec.iprn_time	=	logTime;
				_sosf_rec.iprn_date	=	logDate;
				_sosf_rec.iprn_times++;
			break;

		case	LOG_DISPATCH:	
				sprintf (_sosf_rec.odes_op_id, "%-14.14s", currentUser);
				_sosf_rec.odes_time	=	logTime;
				_sosf_rec.odes_date	=	logDate;
			break;

		case	LOG_DELIVERY:	
				sprintf (_sosf_rec.odel_op_id, "%-14.14s", currentUser);
				_sosf_rec.odel_time	=	logTime;
				_sosf_rec.odel_date	=	logDate;
			break;

		default:
				break;
	}

	if (newSosf)
		cc = abc_add ("sosf", &_sosf_rec);
	else
		cc = abc_update ("sosf", &_sosf_rec);

	if (cc)
		file_err (cc, "sosf", "DBADD/DBUPDATE");

	abc_fclose ("sosf");

	return;
}
	
/*======================================================
| Updated Customer P-slip/ Invoice/Credit Header File. |
======================================================*/
void	
UpdateSosf ( long	hhcuHash,
	long	hhsoHash,
	long	hhcoHash)
{
	open_rec ("sosf", _sosf_list, _SOSF_NO_FIELDS, "sosf_id_no2");

	_sosf_rec.hhcu_hash 	= hhcuHash;
	_sosf_rec.hhso_hash 	= hhsoHash;
	_sosf_rec.hhco_hash 	= 0L;
	cc = find_rec ("sosf", &_sosf_rec, GTEQ, "u");
	if (!cc && _sosf_rec.hhcu_hash == hhcuHash && 
			   _sosf_rec.hhso_hash == hhsoHash)
	{
		_sosf_rec.hhco_hash	=	hhcoHash;
		cc = abc_update ("sosf", &_sosf_rec);
		if (cc)
			file_err (cc, "sosf", "DBUPDATE");
	}
	else
		abc_unlock ("sosf");

	abc_fclose ("sosf");
}
