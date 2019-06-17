	/*===============================+
	 | System Bank Transaction File. |
	 +===============================*/
#define	CBBT_NO_FIELDS	11

	struct dbview	cbbt_list [CBBT_NO_FIELDS] =
	{
		{"cbbt_co_no"},
		{"cbbt_bank_id"},
		{"cbbt_tran_no"},
		{"cbbt_tran_date"},
		{"cbbt_tran_desc"},
		{"cbbt_tran_type"},
		{"cbbt_tran_amt"},
		{"cbbt_stat_post"},
		{"cbbt_reconciled"},
		{"cbbt_select"},
		{"cbbt_period"}
	};

	struct tag_cbbtRecord
	{
		char	co_no [3];
		char	bank_id [6];
		long	tran_no;
		Date	tran_date;
		char	tran_desc [41];
		char	tran_type [2];
		Money	tran_amt;
		char	stat_post [2];
		char	reconciled [2];
		char	select [2];
		Date	period;
	}	cbbt_rec;

	
	/*==========================================+
	 | Establishment/Branch Master File Record. |
	 +==========================================*/
#define	_ESMR_NO_FIELDS	4

	struct dbview	_esmr_list [_ESMR_NO_FIELDS] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_nx_csh_trn_no"},
		{"esmr_stat_flag"}
	};

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	br_no [3];
		long	nx_csh_trn_no;
		char	stat_flag [2];
	}	_esmr_rec;

void OpenCashBook (void);
void CloseCashBook (void);
void WriteCashBook (char *,char *,char *,long,char *,char *,double,char *,long);
int		CashBookInstalled	=	FALSE;

void
WriteCashBook
(
	char	*CoNo,
	char	*BrNo,
	char	*BankId,
	long	TranDate,
	char	*TranDesc,
	char	*TranType,
	Money	TranAmt,
	char	*TranStat,
	long	TranPeriod
)
{
	if (!CashBookInstalled)
		return;

	sprintf (_esmr_rec.co_no, "%2.2s", CoNo);
	sprintf (_esmr_rec.br_no, "%2.2s", BrNo);
	cc = find_rec ("_esmr", &_esmr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "esmr", "DBFIND");

	_esmr_rec.nx_csh_trn_no++;

	cc = abc_update ("_esmr", &_esmr_rec);
	if (cc)
		file_err (cc, "esmr", "DBUPDATE");
	
	sprintf (cbbt_rec.co_no, 	"%2.2s", CoNo);
	sprintf (cbbt_rec.bank_id,	"%5.5s", BankId);
	cbbt_rec.tran_no		=	_esmr_rec.nx_csh_trn_no;
	cc = find_rec ("cbbt", &cbbt_rec, COMPARISON, "r");
	while (!cc)
	{
		cbbt_rec.tran_no++;
		cc = find_rec ("cbbt", &cbbt_rec, COMPARISON, "r");
	}
	if (cbbt_rec.tran_no != _esmr_rec.nx_csh_trn_no)
	{
		_esmr_rec.nx_csh_trn_no = cbbt_rec.tran_no;
		cc = abc_update ("_esmr", &_esmr_rec);
		if (cc)
			file_err (cc, "esmr", "DBUPDATE");
	}
	cbbt_rec.tran_date		=	TranDate;	
	cbbt_rec.tran_amt		=	TranAmt;
	cbbt_rec.period			=	TranPeriod;
	sprintf (cbbt_rec.tran_desc, "%-40.40s", TranDesc);
	sprintf (cbbt_rec.tran_type, "%1.1s", 	TranType);
	sprintf (cbbt_rec.stat_post, 	"Y");
	sprintf (cbbt_rec.reconciled, 	"N");
	sprintf (cbbt_rec.select, 		" ");

	cc = abc_add ("cbbt", &cbbt_rec);
	if (cc)
		file_err (cc, "cbbt", "DBADD");
}

void
OpenCashBook (void)
{
	char	*sptr;
	
	sptr = chk_env("CA_INSTALLED");
	CashBookInstalled = ( sptr == (char *)0 ) ? 0 : atoi ( sptr );

	if (!CashBookInstalled)
		return;

	abc_alias ("_esmr", "esmr");
	open_rec ("cbbt", cbbt_list, CBBT_NO_FIELDS, "cbbt_id_no1");
	open_rec ("_esmr", _esmr_list, _ESMR_NO_FIELDS, "esmr_id_no");
}
void
CloseCashBook (void)
{
	if (!CashBookInstalled)
		return;

	abc_fclose ("cbbt");
	abc_fclose ("_esmr");
}
	
