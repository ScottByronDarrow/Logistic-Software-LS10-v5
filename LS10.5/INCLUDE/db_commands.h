#define	CUR_DAT			0
#define	FUL_DAT			1
#define	CO_NAME			2
#define	CO_ADR1			3
#define	CO_ADR2			4
#define	CO_ADR3			5
#define	DB_NUMB			6
#define	DB_ACRO			7
#define	DB_NAME			8
#define	DB_ADR1			9
#define	DB_ADR2			10
#define	DB_ADR3			11
#define	DB_CNT1			12
#define	CNT_CD1			13
#define	DB_CNT2			14
#define	CNT_CD2			15
#define	SM_NUMB			16
#define	SM_NAME			17
#define	AR_NUMB			18
#define	AR_NAME			19
#define	CNT_TYP			20
#define	PRI_TYP			21
#define	BNK_CDE			22
#define	BNK_BRN			23
#define	DIS_CDE			24
#define	TAX_CDE			25
#define	TAX_NUM			26
#define	DT_LINV			27
#define	DT_LPAY			28
#define	AMT_LPY			29
#define	MTD_SAL			30
#define	YTD_SAL			31
#define	ORD_VAL			32
#define	DB_POST			33
#define	DB_BSEC			34
#define	PHN_NUM			35
#define	FAX_NUM 		36
	
#define	N_CMDS		37

char   *dot_cmds[] = {
	"CUR_DAT",   /* CURRENT DATE FORMAT DD/DD/DD		  	*/
	"FUL_DAT",   /* FULL SYSTEM DATE ( 1st January 1990 ) 	*/
	"CO_NAME",   /* COMPANY NAME.                         	*/
	"CO_ADR1",   /* COMPANY ADDRESS 1.                    	*/
	"CO_ADR2",   /* COMPANY ADDRESS 2.                    	*/
	"CO_ADR3",   /* COMPANY ADDRESS 3.                    	*/
	"DB_NUMB",   /* PROSPECT NUMBER.						*/
	"DB_ACRO",   /* PROSPECT ACRONYM.						*/
	"DB_NAME",   /* PROSPECT NAME.							*/
	"DB_ADR1",   /* PROSPECT ADDRESS PART 1.				*/
	"DB_ADR2",   /* PROSPECT ADDRESS PART 2.				*/
	"DB_ADR3",   /* PROSPECT ADDRESS PART 3.				*/
	"DB_CNT1",   /* PROSPECT CONTACT NAME.					*/
	"CNT_CD1",   /* PROSPECT CONTACT CODE (POSITION).		*/
	"DB_CNT2",   /* PROSPECT CONTACT NAME2.					*/
	"CNT_CD2",   /* PROSPECT CONTACT CODE2 (POSITION).		*/
	"SM_NUMB",   /* SALESMAN NUMBER.						*/
	"SM_NAME",   /* SALESMAN NAME.							*/
	"AR_NUMB",   /* AREA NUMBER.							*/
	"AR_NAME",   /* AREA NAME.								*/
	"CNT_TYP",   /* CONTRACT TYPE							*/
	"PRI_TYP",   /* PRICE TYPE								*/
	"BNK_CDE",   /* BANK CODE								*/
	"BNK_BRN",   /* BANK BRANCH								*/
	"DIS_CDE",   /* DISCOUNT CODE							*/
	"TAX_CDE",   /* TAX CODE								*/
	"TAX_NUM",   /* TAX NUMBER								*/
	"DT_LINV",   /* DATE OF LAST INVOICE					*/
	"DT_LPAY",   /* DATE OF LAST PAYMENT					*/
	"AMT_LPY",   /* AMOUNT OF LAST PAYMENT					*/
	"MTD_SAL",   /* MONTH TO DATE SALES 					*/
	"YTD_SAL",   /* YEAR TO DATE SALES						*/
	"ORD_VAL",   /* VALUE OF CURRENT ORDERS					*/
	"DB_POST",   /* PROSPECT POST CODE.						*/
	"DB_BSEC",   /* PROSPECT BUSINESS SECTOR.				*/
	"PHN_NUM",   /* PROSPECT PHONE NUMBER.					*/
	"FAX_NUM",   /* PROSPECT FAX NUMBER.					*/
	""      /*==============================================*/
};
