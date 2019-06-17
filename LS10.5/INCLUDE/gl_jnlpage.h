#ifndef  _GL_JNLPAGE_H_
#define  _GL_JNLPAGE_H_

#ifndef	MAXLINES
#define	MAXLINES	27
#endif	/* MAXLINES */

/*------------------------------------------------------------------
| This structure must be changed if MAXLINES or TABLINES increased |
------------------------------------------------------------------*/
/* Renamed 'coz of conflict with std math.h */
char	*jrnl[] = {
	"General        ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Standing       ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Accrual        ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Sales          ", "db_winvjnl~LPNO~PID~I~", 		" ",
	"Sales Returns  ", "db_winvjnl~LPNO~PID~C~", 		" ",
	"Receipts       ", "db_crcjnl~LPNO~PID~", 			"db_crcaud~LPNO~PID~",
	"Payables       ", "cr_wicjnl~LPNO~PID~I~", 		" ",
	"Creditors C/N  ", "cr_wicjnl~LPNO~PID~C~", 		" ",
	"Disbursements  ", "cr_payjnl~LPNO~PID~", 			" ",
	"Stock Iss/Rec  ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Purchases      ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Stock Adj      ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Stock Cost Sale", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Stock Take     ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Exch Variance. ", "cr_wevjnl~LPNO~PID~", 			" ",
	"Customers Jnls.", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Creditors Jnls.", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Bank Transfers.", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Prod. Iss./Rec.", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Bills Reversed ", "db_bill_aud~LPNO~PID~", 		" ",
	"Contract Lab.  ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Contract Mat.  ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Contract Comp. ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Contract Adj.  ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"Forward Receipt", "db_fwd_aud~LPNO~PID~", 			" ",
	"Sundry Receipts", "db_crc2jnl~LPNO~PID~",			" ", 
	"POS - Invoices ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"POS - Credits  ", "gl_wjnlprnt~TYPE~LPNO~PID~", 	" ",
	"", "", ""
};
#endif
