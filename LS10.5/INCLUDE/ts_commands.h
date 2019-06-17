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
#define	FAX_NUM			36
#define	PH_FREQ			37
#define	NX_PHDT			38
#define	NX_PHTM			39
#define	VS_FREQ			40
#define	NX_VSDT			41
#define	NX_VSTM			42
#define	OPERATR			43
#define	LST_OPR			44
#define	LST_PHN			45
#define	BST_PHN			46
#define	DT_CREA			47
#define	LBL_HZi			48
#define	LBL_VTi			49
#define	LBL_HDi			50
#define	LBL_TLi			51
#define	LBL_HSi			52
#define	LBL_VSi			53
	
#define	N_CMDS		54

/*---------------------------------------------
| Following structure hold valid dot commands |
| for telesales mailers and which format they |
| may be used in : N - Normal                 |
|                  F - Follow Up              |
|                  L - Label                  |
---------------------------------------------*/
struct
{
	char	command[8];
	char	format[4];
} dot_cmds[] = {
	{ "CUR_DAT", "NFL" },  /* CURRENT DATE FORMAT DD/DD/DD		*/
	{ "FUL_DAT", "NFL" },  /* FULL SYSTEM DATE ( 1st January 1990 ) */
	{ "CO_NAME", "NFL" },  /* COMPANY NAME.                       	*/
	{ "CO_ADR1", "NFL" },  /* COMPANY ADDRESS 1.                    */
	{ "CO_ADR2", "NFL" },  /* COMPANY ADDRESS 2.                    */
	{ "CO_ADR3", "NFL" },  /* COMPANY ADDRESS 3.                    */
	{ "DB_NUMB", "NFL" },  /* PROSPECT NUMBER.			*/
	{ "DB_ACRO", "NFL" },  /* PROSPECT ACRONYM.			*/
	{ "DB_NAME", "NFL" },  /* PROSPECT NAME.			*/
	{ "DB_ADR1", "NFL" },  /* PROSPECT ADDRESS PART 1.		*/
	{ "DB_ADR2", "NFL" },  /* PROSPECT ADDRESS PART 2.		*/
	{ "DB_ADR3", "NFL" },  /* PROSPECT ADDRESS PART 3.		*/
	{ "DB_CNT1", "NFL" },  /* PROSPECT CONTACT NAME.		*/
	{ "CNT_CD1", "NF " },  /* PROSPECT CONTACT CODE (POSITION).	*/
	{ "DB_CNT2", "NF " },  /* PROSPECT CONTACT NAME2.		*/
	{ "CNT_CD2", "NF " },  /* PROSPECT CONTACT CODE2 (POSITION).	*/
	{ "SM_NUMB", "NF " },  /* SALESMAN NUMBER.			*/
	{ "SM_NAME", "NF " },  /* SALESMAN NAME.			*/
	{ "AR_NUMB", "NFL" },  /* AREA NUMBER.				*/
	{ "AR_NAME", "NFL" },  /* AREA NAME.				*/
	{ "CNT_TYP", "NF " },  /* CONTRACT TYPE				*/
	{ "PRI_TYP", "NF " },  /* PRICE TYPE				*/
	{ "BNK_CDE", "NF " },  /* BANK CODE				*/
	{ "BNK_BRN", "NF " },  /* BANK BRANCH				*/
	{ "DIS_CDE", "NF " },  /* DISCOUNT CODE				*/
	{ "TAX_CDE", "NF " },  /* TAX CODE				*/
	{ "TAX_NUM", "NF " },  /* TAX NUMBER				*/
	{ "DT_LINV", "NF " },  /* DATE OF LAST INVOICE			*/
	{ "DT_LPAY", "NF " },  /* DATE OF LAST PAYMENT			*/
	{ "AMT_LPY", "NF " },  /* AMOUNT OF LAST PAYMENT		*/
	{ "MTD_SAL", "NF " },  /* MONTH TO DATE SALES 			*/
	{ "YTD_SAL", "NF " },  /* YEAR TO DATE SALES			*/
	{ "ORD_VAL", "NF " },  /* VALUE OF CURRENT ORDERS		*/
	{ "DB_POST", "NFL" },  /* PROSPECT POST CODE.			*/
	{ "DB_BSEC", "NF " },  /* PROSPECT BUSINESS SECTOR.		*/
	{ "PHN_NUM", "NFL" },  /* PROSPECT PHONE NUMBER.		*/
	{ "FAX_NUM", "NFL" },  /* PROSPECT FAX NUMBER.			*/
	{ "PH_FREQ", "NF " },  /* PHONE FREQUENCY.			*/
	{ "NX_PHDT", "NF " },  /* NEXT PHONE DATE.			*/
	{ "NX_PHTM", "NF " },  /* NEXT PHONE TIME.			*/
	{ "VS_FREQ", "NF " },  /* VISIT FREQUENCY.			*/
	{ "NX_VSDT", "NF " },  /* NEXT VISIT DATE.			*/
	{ "NX_VSTM", "NF " },  /* NEXT VISIT TIME.			*/
	{ "OPERATR", "NF " },  /* OPERATOR.          			*/
	{ "LST_OPR", "NF " },  /* LAST OPERATOR.       			*/
	{ "LST_PHN", "NF " },  /* LAST PHONE DATE.			*/
	{ "BST_PHN", "NF " },  /* BEST PHONE TIME.			*/
	{ "DT_CREA", "NF " },  /* DATE CREATED.				*/
	{ "LBL_HZi", "L  " },  /* LABELS HORIZONTALLY i = integer 1-9	*/
	{ "LBL_VTi", "L  " },  /* LABELS VERTICALLY   i = integer 1-9	*/
	{ "LBL_HDi", "L  " },  /* LINES BEFORE FIRST LABEL.          	*/
	{ "LBL_TLi", "L  " },  /* LINES AFTER LAST LABEL.          	*/
	{ "LBL_HSi", "L  " },  /* HORIZONTAL SPACING BETWEEN LABELS  	*/
	{ "LBL_VSi", "L  " },  /* VERTICAL SPACING BETWEEN LABELS  	*/
	{ "", 	    "" },      /*=======================================*/
};
