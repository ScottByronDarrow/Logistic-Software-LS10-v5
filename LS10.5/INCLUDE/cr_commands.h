#define	CUR_DAT			0
#define	FUL_DAT			1
#define	CO_NAME			2
#define	CO_ADR1			3
#define	CO_ADR2			4
#define	CO_ADR3			5
#define	CR_NUMB			6
#define	CR_ACRO			7
#define	CR_NAME			8
#define	CR_ADR1			9
#define	CR_ADR2			10
#define	CR_ADR3			11
#define	CR_CNT1			12
#define	CR_CNT2			13
#define	ACT_TYP			14
#define	BNK_CDE			15
#define	BNK_BRN			16
#define	DIS_PER			17
#define	TAX_CDE			18
#define	TAX_NUM			19
#define	PHN_NUM			20
#define	FAX_NUM 		21
	
#define	N_CMDS		22
 
char   *dot_cmds[] = {
	"CUR_DAT",   /* CURRENT DATE FORMAT DD/DD/DD		  	*/
	"FUL_DAT",   /* FULL SYSTEM DATE ( 1st January 1990 ) 	*/
	"CO_NAME",   /* COMPANY NAME.                         	*/
	"CO_ADR1",   /* COMPANY ADDRESS 1.                    	*/
	"CO_ADR2",   /* COMPANY ADDRESS 2.                    	*/
	"CO_ADR3",   /* COMPANY ADDRESS 3.                    	*/
	"CR_NUMB",   /* PROSPECT NUMBER.						*/
	"CR_ACRO",   /* PROSPECT ACRONYM.						*/
	"CR_NAME",   /* PROSPECT NAME.							*/
	"CR_ADR1",   /* PROSPECT ADDRESS PART 1.				*/
	"CR_ADR2",   /* PROSPECT ADDRESS PART 2.				*/
	"CR_ADR3",   /* PROSPECT ADDRESS PART 3.				*/
	"CR_CNT1",   /* PROSPECT CONTACT NAME.					*/
	"CR_CNT2",   /* PROSPECT CONTACT NAME2.					*/
	"ACT_TYP",   /* ACCOUNT TYPE							*/
	"BNK_CDE",   /* BANK CODE								*/
	"BNK_BRN",   /* BANK BRANCH								*/
	"DIS_PER",   /* DISCOUNT PERCENT						*/
	"TAX_CDE",   /* TAX CODE								*/
	"DBT_NUM",   /* DEBTOR NUMBER							*/
	"PHN_NUM",   /* CONTACT PHONE NUMBER.					*/
	"FAX_NUM",   /* PROSPECT FAX NUMBER.					*/
	""      /*==============================================*/
};
