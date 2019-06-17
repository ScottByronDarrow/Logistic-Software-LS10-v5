/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : sk_ir_inc.h                                    |
|  Source Desc       : Include file for transfers.  sk_ir_trans.c &   |
|                    :                              sk_ir_mctran.c    |
|                    :                                                |
|                    : ALSO SEE ( sk_ir_code.c )                      |
|---------------------------------------------------------------------|
|  Date Modified     : 01/08/91   | Modified  by  : Campbell Mander.  |
|                    : 06/10/92   | Modified  by  : Anneliese Allen.  |
|                    : 21/10/92   | Modified  by  : Anneliese Allen.  |
|                    : 16/01/1998 | Modified  by  : Campbell Mander.  |
|					 : 06/03/1998 | Modified  by  : Ronnel L. Amanca. |
|                                                                     |
|  Comments          : Added new dot commands for freight details and |
|                    : line spacing.                                  |
|                    :                                                |
|                    : Added new dot command for makers number.       |
|                    : HMQ 7856.                                      |
|                    :                                                |
|                    : Added new dot command for pack size.           |
|                    : DPL 7992.                                      |
|                    :                                                |
|                    : Changes for UOM and LOCATIONS.                 |
|                    :                                                |
|  06/03/98          : Modified to add dot commands in WH Address.    |
|					 :												  |
=====================================================================*/
#define	ISS_CO_NUMB	0
#define	ISS_CO_NAME	1
#define	ISS_CO_SHRT	2
#define	ISS_CO_ADR1	3
#define	ISS_CO_ADR2	4
#define	ISS_CO_ADR3	5
#define	REC_CO_NUMB	6
#define	REC_CO_NAME	7
#define	REC_CO_SHRT	8
#define	REC_CO_ADR1	9
#define	REC_CO_ADR2	10
#define	REC_CO_ADR3	11
#define	ISS_BR_NUMB	12
#define	ISS_BR_NAME	13
#define	ISS_BR_SHRT	14
#define	ISS_BR_ADR1	15
#define	ISS_BR_ADR2	16
#define	ISS_BR_ADR3	17
#define	REC_BR_NUMB	18
#define	REC_BR_NAME	19
#define	REC_BR_SHRT	20
#define	REC_BR_ADR1	21
#define	REC_BR_ADR2	22
#define	REC_BR_ADR3	23
#define	ISS_WH_NUMB	24
#define	ISS_WH_NAME	25
#define	ISS_WH_SHRT	26
#define	REC_WH_NUMB	27
#define	REC_WH_NAME	28
#define	REC_WH_SHRT	29
#define	LN_ITM_NUMB	30
#define	LN_ITM_NAME	31
#define	LN_QT_ORDER	32
#define	LN_QT_BORDR	33
#define	LN_QT_SUPPY	34
#define	LN_INMR_PAK	35
#define	LN_MAK_NUMB	36
#define	LN_SER_NUMB	37
#define	LN_LOCATION	38
#define LN_REF_NUMB	39
#define LN_CUST_STK	40
#define	TRAN_N_DATE	41
#define	TRAN_M_DATE	42
#define	TRAN_C_DATE	43
#define	TRAN_C_TIME	44
#define DOCKET_NUMB	45
#define	PAGE_NUMBER	46
#define	FORMAT_LN_1	47
#define	FORMAT_LN_2	48
#define	TR_TRAN_REF	49
#define	TR_MESSAGES	50
#define	TR_FULL_SUP	51
#define	TR_CAR_CODE	52
#define	FRGHT_VALUE	53
#define	FRT_TOT_WGT	54
#define	FRT_CONS_NO	55
#define	FRT_CARTONS	56
#define	DTE_ENTERED	57
#define	TME_ENTERED	58
#define	OPERATOR_ID	59
#define	LINE_SPACE1	60
#define	LINE_SPACE2	61
#define	LINE_SPACE3	62
#define	FORMAT_LN_3	63
#define	LN_U_O_MEAS	64
#define ISS_WH_ADR1 65
#define ISS_WH_ADR2 66
#define ISS_WH_ADR3 67
#define REC_WH_ADR1 68
#define REC_WH_ADR2 69
#define REC_WH_ADR3 70

#define	N_CMDS 71	

char	*dot_cmds[] = {
			/*================================================*/
	"ISS_CO_NUMB",	/* Issuing company number.                        */
	"ISS_CO_NAME",	/* Issuing company name.                          */
	"ISS_CO_SHRT",	/* Issuing company short name.                    */
	"ISS_CO_ADR1",	/* Issuing company address part one.              */
	"ISS_CO_ADR2",	/* Issuing company address part two.              */
	"ISS_CO_ADR3",	/* Issuing company address part three.            */
	"REC_CO_NUMB",	/* Receiving company number.                      */
	"REC_CO_NAME",	/* Receiving company name.                        */
	"REC_CO_SHRT",	/* Receiving company short name.                  */
	"REC_CO_ADR1",	/* Receiving company address part one.            */
	"REC_CO_ADR2",	/* Receiving company address part two.            */
	"REC_CO_ADR3",	/* Receiving company address part three.          */
	"ISS_BR_NUMB",	/* Issuing branch number.                         */
	"ISS_BR_NAME",	/* Issuing branch name.                           */
	"ISS_BR_SHRT",	/* Issuing branch short name.                     */
	"ISS_BR_ADR1",	/* Issuing branch address part one.               */
	"ISS_BR_ADR2",	/* Issuing branch address part two.               */
	"ISS_BR_ADR3",	/* Issuing branch address part three.             */
	"REC_BR_NUMB",	/* Receiving branch number.                       */
	"REC_BR_NAME",	/* Receiving branch name.                         */
	"REC_BR_SHRT",	/* Receiving branch short name.                   */
	"REC_BR_ADR1",	/* Receiving branch address part one.             */
	"REC_BR_ADR2",	/* Receiving branch address part two.             */
	"REC_BR_ADR3",	/* Receiving branch address part three.           */
	"ISS_WH_NUMB",	/* Issuing warehouse number                       */
	"ISS_WH_NAME",	/* Issuing warehouse name.                        */
	"ISS_WH_SHRT",	/* Issuing warehouse short name.                  */
	"REC_WH_NUMB",	/* Receiving warehouse number.                    */
	"REC_WH_NAME",	/* Receiving warehouse name.                      */
	"REC_WH_SHRT",	/* Receiving warehouse short name.                */
	"LN_ITM_NUMB",	/* Line item number.                              */
	"LN_ITM_NAME",	/* Line item description.                         */
	"LN_QT_ORDER",	/* Line quantity ordered.                         */
	"LN_QT_BORDR",	/* Line quantity back ordered.                    */
	"LN_QT_SUPPY",	/* Line quantity supplied.                        */
	"LN_INMR_PAK",	/* Pack size.                                     */
	"LN_MAK_NUMB",	/* Line maker number.                             */
	"LN_SER_NUMB",	/* Line item serial number.                       */
	"LN_LOCATION",	/* Line item location.                            */
	"LN_REF_NUMB",	/* Line Level Reference Number                    */
	"LN_CUST_STK",	/* Customer / Stock Transfer.                     */
	"TRAN_N_DATE",	/* Transfer Date.  ( dd/mm/yy )                   */
	"TRAN_M_DATE",	/* Current module date ( STOCK ) ( dd/mm/yy )     */
	"TRAN_C_DATE",	/* Current system date ( dd/mm/yy )               */
	"TRAN_C_TIME",	/* Current system time ( hh:mm )                  */
	"DOCKET_NUMB",	/* Delivery docket number.                        */
	"PAGE_NUMBER",	/* Page number.                                   */
	"FORMAT_LN_1",	/* Line item format line Number 1                 */
	"FORMAT_LN_2",	/* Line item format line Number 2 (Used for serno)*/
	"TR_TRAN_REF",	/* Transfer Reference at header level             */
	"TR_MESSAGES",	/* Transfer message lines used to print "reprint" */
	"TR_FULL_SUP",	/* Full Supply message.                           */
	"TR_CAR_CODE",	/* Carrier Code                                   */
	"FRGHT_VALUE",	/* Value Of Freight                               */
	"FRT_TOT_WGT",	/* Total Weight Of Freight (in kgs)               */
	"FRT_CONS_NO",	/* Consignment Number                             */
	"FRT_CARTONS",	/* Number Of Cartons                              */
	"DTE_ENTERED",	/* Date Transfer Entered                          */
	"TME_ENTERED",	/* Time Transfer Entered                          */
	"OPERATOR_ID",	/* Entered By Operator ID ...                     */
	"LINE_SPACE1",	/* Set single line spacing                        */
	"LINE_SPACE2",	/* Set double line spacing                        */
	"LINE_SPACE3",	/* Set triple line spacing                        */
	"FORMAT_LN_3",	/* Line item format line Number 3 (Used for location)*/
	"LN_U_O_MEAS",	/* Unit of measure	*/
	"ISS_WH_ADR1", /* Issuing Warehouse Address 1 */
	"ISS_WH_ADR2", /* Issuing Warehouse Address 2 */
	"ISS_WH_ADR3", /* Issuing Warehouse Address 3 */
	"REC_WH_ADR1", /* Receiving Warehouse Address 1 */
	"REC_WH_ADR2", /* Receiving Warehouse Address 2 */
	"REC_WH_ADR3", /* Receiving Warehouse Address 3 */
			/*================================================*/
	""
};
