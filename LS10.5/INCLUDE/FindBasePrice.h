/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( xxxxxxxxx.c  )                                   |
|  Program Desc  : (                                                ) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
	
	/*=======================+
	 | Inventory Price File. |
	 +=======================*/
#define	_INPR_NO_FIELDS		8

	struct dbview	_inpr_list [_INPR_NO_FIELDS] =
	{
		{"inpr_hhbr_hash"},
		{"inpr_price_type"},
		{"inpr_br_no"},
		{"inpr_wh_no"},
		{"inpr_curr_code"},
		{"inpr_cust_type"},
		{"inpr_hhgu_hash"},
		{"inpr_base"},
	};

	struct _tag_inprRecord
	{
		long	hhbr_hash;
		int		price_type;
		char	br_no [3];
		char	wh_no [3];
		char	curr_code [4];
		char	cust_type [4];
		long	hhgu_hash;
		Money	base;
	}	_inpr_rec;

	int		BaseOpened	=	FALSE;
	int		PriceLevel	=	0;

void
OpenBasePrice (void)
{
	char	*sptr;

	sptr = chk_env("SK_CUSPRI_LVL");
	PriceLevel = (sptr == (char *)0) ? 0 : atoi (sptr);

	BaseOpened	=	TRUE;

	abc_alias ("_inpr", "inpr");
	open_rec ("_inpr", _inpr_list, _INPR_NO_FIELDS, "inpr_id_no");
}

void
CloseBasePrice (void)
{
	BaseOpened	=	FALSE;
	abc_fclose ("_inpr");
}

double	FindBasePrice (char *, char *,long, int, char	*);


/*=========================
| Find pricing structure. |
=========================*/
double	
FindBasePrice (
	char	*BrNo,
	char	*WhNo,
	long	HHBR_HASH,
	int		PriceType,
	char	*CurrencyCode)
{
	if (!BaseOpened)
		return (-1.00);

	_inpr_rec.hhbr_hash		=	HHBR_HASH;
	_inpr_rec.price_type	=	PriceType;
	_inpr_rec.hhgu_hash		=	0L;
	sprintf (_inpr_rec.curr_code, "%-3.3s", CurrencyCode);

	if ( !PriceLevel )
	{
		strcpy (_inpr_rec.br_no, "  ");
		strcpy (_inpr_rec.wh_no, "  ");
	}
	else 
	{
		sprintf (_inpr_rec.br_no, "%-2.2s", BrNo);
		sprintf (_inpr_rec.wh_no, "%-2.2s", ( PriceLevel == 2 ) ? WhNo : "  ");
	}
	
	strcpy (_inpr_rec.cust_type, "   ");
	cc = find_rec ("_inpr", &_inpr_rec, EQUAL, "r");
	if (cc)
	{
		sprintf (_inpr_rec.br_no, "%-2.2s", BrNo);
		strcpy (_inpr_rec.wh_no,"  ");
		cc = find_rec ("_inpr", &_inpr_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (_inpr_rec.br_no,"  ");
			strcpy (_inpr_rec.wh_no,"  ");
		}
		cc = find_rec ("_inpr", &_inpr_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (_inpr_rec.br_no, "  ");
			strcpy (_inpr_rec.wh_no, "  ");
			_inpr_rec.base     = 0.00;
		}
	}
	return (_inpr_rec.base);
}
