#define	DB_ACRO		0
#define	DB_NUMB		1
#define	DB_NAME		2
#define	DB_ADR1		3
#define	DB_ADR2		4
#define	DB_ADR3		5
#define	DB_CONT		6
#define	SM_NUMB		7
#define	SM_NAME		8
#define	QT_NUMB		9
#define	QT_SALU		10
#define	QT_ORDE		11
#define	QT_QDAT		12
#define	QT_EDAT		13
#define	QT_CNAM		14
#define	QT_COM1		15
#define	QT_COM2		16
#define	QT_COM3		17
#define	LN_ITEM		18
#define	LN_DESC		19
#define	LN_QUTY		20
#define	LN_NETT		21
#define	LN_GROS		22
#define	LN_DIS1		23
#define	LN_DIS2		24
#define	LN_SERL		25
#define	TOT_DIS		26
#define	TOT_TAX		27
#define	TOT_NET		28
#define	TOT_GRO		29
#define	TOT_ALL		30
#define	MOD_DAT		31
#define	FUL_DAT		32
#define	CUR_DAT		33
#define	CO_NAME		34
#define	CO_ADR1		35
#define	CO_ADR2		36
#define	CO_ADR3		37
#define	ST_NETT		38
#define	ST_GROS		39

#define	N_CMDS	40

char	*dot_cmds[] = {
			/*==============================================*/
	"DB_ACRO",	/*	DEBTORS ACRONYM.			*/
	"DB_NUMB",	/*	DEBTORS NUMBER.				*/
	"DB_NAME",	/*	DEBTORS NAME.				*/
	"DB_ADR1",	/*	DEBTORS ADDRESS PART 1.			*/
	"DB_ADR2",	/*	DEBTORS ADDRESS PART 2.			*/
	"DB_ADR3",	/*	DEBTORS ADDRESS PART 3.			*/
	"DB_CONT",	/*	DEBTORS CONTACT NAME.			*/
	"SM_NUMB",	/*	SALESMAN NUMBER.			*/
	"SM_NAME",	/*	SALESMAN NAME.				*/
	"QT_NUMB",	/*	QUOTATION NUMBER.			*/
	"QT_SALU",	/*	QUOTATION SALUTATION.			*/
	"QT_ORDE",	/*	QUOTATION CUSTOMER ORDER NUMBER.	*/
	"QT_QDAT",	/*	QUOTATION DATE.				*/
	"QT_EDAT",	/*	QUOTATION EXPIRY DATE.			*/
	"QT_CNAM",	/*	QUOTATION CONTACT NAME.			*/
	"QT_COM1",	/*	QUOTATION COMMENT LINE 1.		*/
	"QT_COM2",	/*	QUOTATION COMMENT LINE 2.		*/
	"QT_COM3",	/*	QUOTATION COMMENT LINE 3.		*/
	"LN_ITEM",	/*	LINE ITEM ( ITEM NUMBER ) 		*/
	"LN_DESC",	/*	LINE ITEM ( ITEM DESCRIPTION ) 		*/
	"LN_QUTY",	/*	LINE ITEM ( QUANTITY. ) 		*/
	"LN_NETT",	/*	LINE ITEM ( NETT PRICE  ) 		*/
	"LN_GROS",	/*	LINE ITEM ( GROSS PRICE ) 		*/
	"LN_DIS1",	/*	LINE ITEM ( DISCOUNT PC ) 		*/
	"LN_DIS2",	/*	LINE ITEM ( DISCOUNT AMOUNT ) 		*/
	"LN_SERL",	/*	LINE ITEM ( SERIAL NUMBER ) 		*/
	"TOT_DIS",	/*	TOTAL QUOTE DISCOUNT AMOUNT.		*/
	"TOT_TAX",	/*	TOTAL QUOTE TAX AMOUNT.			*/
	"TOT_NET",	/*	TOTAL QUOTE NETT AMOUNT.		*/
	"TOT_GRO",	/*	TOTAL QUOTE GROSS AMOUNT.		*/
	"TOT_ALL",	/*	TOTAL QUOTE INCLUDING GROSS + TAX 	*/
	"MOD_DAT",	/*	MODULE DATE FORMAT DD/MM/YY.		*/
	"FUL_DAT",	/*	FULL SYSTEM DATE ( 1st January 1990 )   */
	"CUR_DAT",	/*	CURRENT DATE FORMAT DD/DD/DD		*/
	"CO_NAME",	/*	COMPANY NAME.                       	*/
	"CO_ADR1",	/*	COMPANY ADDRESS 1.                      */
	"CO_ADR2",	/*	COMPANY ADDRESS 2.                      */
	"CO_ADR3",	/*	COMPANY ADDRESS 3.                      */
	"ST_NETT",	/*	LINE ITEM ( SUB_TOTAL NETT PRICE  )		*/
	"ST_GROS",	/*	LINE ITEM ( SUB_TOTAL GROSS PRICE  		*/
			/*==============================================*/
	""
};
