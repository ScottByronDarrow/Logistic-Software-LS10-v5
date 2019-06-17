typedef struct	
	{
	  	char	accountNo [32];
		int		year;
		int		period;
		double	fgnAmount;
		double	locAmount;
		char    currCode [4];
		double  exchRate;
		long	theTime;
		char	status [2];
	} AUD_STRUCT, *AUD_PTR;

#define	AUD_ACCOUNT_NO	aud_buf.accountNo
#define	AUD_YEAR		aud_buf.year
#define	AUD_PERIOD		aud_buf.period
#define	AUD_FGN_AMOUNT	aud_buf.fgnAmount
#define AUD_LOC_AMOUNT	aud_buf.locAmount
#define AUD_CURR_CODE	aud_buf.currCode
#define AUD_EXCH_RATE	aud_buf.exchRate
#define	AUD_TIME		aud_buf.theTime
#define	AUD_STATUS		aud_buf.status

#define	IGLAUDIT
