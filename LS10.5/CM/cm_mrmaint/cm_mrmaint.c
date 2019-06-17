/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_mrmaint.c,v 5.4 2002/01/10 07:08:40 scott Exp $
|  Program Name  : (cm_mrmaint.c)                                   |
|  Program Desc  : (Contract Management Header Input/Maint. )     |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 19/02/93         |
|---------------------------------------------------------------------|
| $Log: cm_mrmaint.c,v $
| Revision 5.4  2002/01/10 07:08:40  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
| Revision 5.3  2001/10/09 22:34:26  scott
| Updated from Scotts machine.
|
| Revision 5.2  2001/08/09 08:57:35  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:22  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_mrmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_mrmaint/cm_mrmaint.c,v 5.4 2002/01/10 07:08:40 scott Exp $";

#define	TXT_REQD
#include <pslscr.h>
#include <GlUtils.h>
#include <getnum.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>

#define	CM_SCN1		1
#define	CM_SCN2		2
#define	CM_SCN3		3

#define	COMPANY	2
#define	BRANCH	1
#define	USER	0
#define DIFF_JOB (exists &&  strcmp (local_rec.old_job, cmjt_rec.job_type))

	char	*cumr2 = "cumr2",
			*cmhr2 = "cmhr2",
			*data  = "data";

#include	"schema"

struct cmjdRecord	cmjd_rec;
struct cmcbRecord	cmcb_rec;
struct cmtrRecord	cmtr_rec;
struct cmtsRecord	cmts_rec;
struct commRecord	comm_rec;
struct cmitRecord	cmit_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cmcdRecord	cmcd_rec;
struct cmhrRecord	cmhr_rec;
struct cmhrRecord	cmhr2_rec;
struct cmjtRecord	cmjt_rec;
struct cmwsRecord	cmws_rec;
struct comrRecord	comr_rec;
struct cudiRecord	cudi_rec;
struct esmrRecord	esmr_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	int 	envDbFind	= 0, 
			envDbCo		= 0, 
			byWhat		= 0,
			heldOrder 	= FALSE,
			exists 		= FALSE,	
			dbStopCrd 	= TRUE,
			dbCrdTerm 	= TRUE,
			dbCrdOver 	= TRUE;

	long	plusDays = -1;

	double	c_left = 0.00;

	char	branchNo [3],
			cbranchNo [3];

	static char *GlAccName [] = 
	{
		"Work In Progress.",
		"Labour Recovery.",
		"Overhead Recovery.",
		"Sales.",
		"Cost Of Sales.",
		"Variance.",
		"Internal."
	};

	extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
/*
		char	desc [7][71];
*/
		long	lsystemDate;
		char	systemDate [11];
		char	accDesc [7][41];
		char	quote_desc [9];
		char 	progress [4];
		char	int_desc [4];
		char	masterContractDesc [61];
		char	dummy [10];
		char	dbt_no [7];
		char	old_job [5];
		char	contDesc [71];
} local_rec;

static	struct	var	vars [] =
{
	{CM_SCN1, LIN, "cont_no",	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Contract No.          ", "Enter Contract Number For Maintenance else <ENTER> , Full Search Available. ",
		 NE, NO,  JUSTLEFT, "", "", cmhr_rec.cont_no},
	{CM_SCN1, LIN, "customerNo",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  "Customer No.          ", "Enter Customer Number , Full Search Available. ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dbt_no},
	{CM_SCN1, LIN, "name",	 	 4, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{CM_SCN1, LIN, "mcust_no",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  "Master Cust No.       ", "",
		 NA, NO,  JUSTLEFT, "", "", cumr2_rec.dbt_no},
	{CM_SCN1, LIN, "mname", 	 5, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", cumr2_rec.dbt_name},
	{CM_SCN1, LIN, "mcont_no",	 6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", "Master Contract No.   ", "Enter Master Contract Number , Full Search Available. ",
		 YES, NO,  JUSTLEFT, "", "", cmhr2_rec.cont_no},
	{CM_SCN1, LIN, "masterContractDesc",	 6, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "      ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.masterContractDesc},
	{CM_SCN1, LIN, "cus_ord_ref",	 8, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Customer Order Ref.   ", " ",
		YES, NO,  JUSTLEFT, "", "", cmhr_rec.cus_ref},
	{CM_SCN1, LIN, "contact",	8, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.contact_name, "Contact Name          ", "Default - Customer Master File Contact Name",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.contact},
	{CM_SCN1, LIN, "cus_addr1",	9, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr1, "Delivery Address #1   ", "Default - Customer Master File Address ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.adr1},
	{CM_SCN1, LIN, "cus_addr2",	10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr2, "Delivery Address #2   ", " Default - Customer Master File Address ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.adr2},
	{CM_SCN1, LIN, "cus_addr3",	11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cumr_rec.dl_adr3, "Delivery Address #3   ", " Default - Customer Master File Address ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.adr3},
	{CM_SCN1, LIN, "startDate",	9, 65, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Start Date            ", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&cmhr_rec.st_date},
	{CM_SCN1, LIN, "dueDate",	10, 65, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Due Date              ", "Please Search to select Calandar",
		YES, NO,  JUSTLEFT, " ", "", (char *)&cmhr_rec.due_date},
	{CM_SCN1, LIN, "quote_type",	11, 65, CHARTYPE,
		"U", "          ",
		" ", "F", "Contract Type        ", "F)ixed V)ariable",
		 YES, NO,  JUSTLEFT, "FV", "", cmhr_rec.quote_type},
	{CM_SCN1, LIN, "quote_desc",	11, 90, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.quote_desc},
	{CM_SCN2, TXT, "contDesc", 13, 30, 0, "","          ", " "," ",
	"                         CONTRACT DESCRIPTION                         ", " ",
	6,70,7, "", "", local_rec.contDesc},
	{CM_SCN3, LIN, "issueTo",	3, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Issue To             ", "Enter Issue to code or <return> for none.",
		 YES, NO,  JUSTLEFT, "", "", cmit_rec.issto},
	{CM_SCN3, LIN, "issueName",	3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " - ", " ",
		 NA, NO,  JUSTLEFT, "", "", cmit_rec.iss_name},
	{CM_SCN3, LIN, "issueDate",	3, 90, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Issue To Date        ", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&cmhr_rec.it_date},
	{CM_SCN3, LIN, "wipStatus",	4, 2, CHARTYPE,
		"UUUU", "          ",
		" ", "", "WIP Status           ", "Enter WIP status code [SEARCH] ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.wip_status},
	{CM_SCN3, LIN, "wip_desc",	4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " - ", " ",
		 NA, NO,  JUSTLEFT, "", "", cmws_rec.desc},
	{CM_SCN3, LIN, "wip_date",	4, 90, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "WIP Date             ", "Default - Today's Date",
		YES, NO,  JUSTLEFT, " ", "", (char *)&cmhr_rec.wip_date},
	{CM_SCN3, LIN, "progress",	5, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Progress Billing     ", "Enter Y(es or N(o",
		 YES, NO,  JUSTLEFT, "YN", "", cmhr_rec.progress},
	{CM_SCN3, LIN, "prog_desc",	5, 25, CHARTYPE,
		"AAA", "          ",
		" ", "", "(", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.progress},
	{CM_SCN3, LIN, "anniversaryDay",	5, 90, CHARTYPE,
		"AA", "          ",
		" ", "", "Anniversary Day      ", " ",
		 ND, NO,  JUSTRIGHT, "0123456789", "", cmhr_rec.anni_day},
	{CM_SCN3, LIN, "quoteValue",	6, 2, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", " ", "Contract Value       ", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999999.99", (char *) &cmhr_rec.quote_val},
	{CM_SCN3, LIN, "estimatedCost",	6, 50, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", " ", "Estimated Costs      ", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999999.99", (char *) &cmhr_rec.est_costs},
	{CM_SCN3, LIN, "expectedProfit",	6, 90, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "Expected Profit%     ", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &cmhr_rec.est_prof},
	{CM_SCN3, LIN, "int_ext",	7, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Internal Contract    ", "Y)es N)o ",
		 YES, NO,  JUSTLEFT, "YN", "", cmhr_rec.internal},
	{CM_SCN3, LIN, "int_desc",	7, 25, CHARTYPE,
		"AAA", "          ",
		" ", "", "(", "",
		 NA, NO,  JUSTLEFT, "YN", "", local_rec.int_desc},
	{CM_SCN3, LIN, "lab_rate",	7, 50, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0.00", "Override Labour      ", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999999.99", (char *) &cmhr_rec.lab_rate},
	{CM_SCN3, LIN, "o_h_rate",	7, 90, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0.00", "Override O/H         ", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999999.99", (char *) &cmhr_rec.oh_rate},
	{CM_SCN3, LIN, "usr_desc1",	9, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "Analysis Codes #1    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.usr_ref1},
	{CM_SCN3, LIN, "usr_desc2",	9, 50, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "Analysis Codes #2    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.usr_ref2},
	{CM_SCN3, LIN, "usr_desc3",	10, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "Analysis Codes #3    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.usr_ref3},
	{CM_SCN3, LIN, "usr_desc4",	10, 50, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "Analysis Codes #4    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.usr_ref4},
	{CM_SCN3, LIN, "usr_desc5",	10, 90, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "Analysis Codes #5    ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.usr_ref5},
	{CM_SCN3, LIN, "job_type",	12, 2, CHARTYPE,
		"UUUU", "          ",
		" ", "", "Job Type             ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmjt_rec.job_type},
	{CM_SCN3, LIN, "job_desc",	12, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", cmjt_rec.desc},
	{CM_SCN3, LIN, "acc_no0",	13, 2, CHARTYPE,
		GlMask, "                          ",
		" ", " ", "WIP G/L Account      ", " ",
		 NI, NO,  JUSTLEFT, "0123456789", "", cmhr_rec.wip_glacc},
	{CM_SCN3, LIN, "wipacc_desc",	13, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [0]},
	{CM_SCN3, LIN, "acc_no1",	14, 2, CHARTYPE,
		GlMask, "                          ",
		"0", " ", "Lab G/L Account      ", " ",
		 NI, NO,  JUSTLEFT, "0123456789", "", cmhr_rec.lab_glacc},
	{CM_SCN3, LIN, "labacc_desc",	14, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [1]},
	{CM_SCN3, LIN, "acc_no2",	15, 2, CHARTYPE,
		GlMask, "                          ",
		"0", " ", "O/H G/L Account      ", " ",
		 NI, NO,  JUSTLEFT, "0123456789", "", cmhr_rec.o_h_glacc},
	{CM_SCN3, LIN, "o_hacc_desc",	15, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [2]},
	{CM_SCN3, LIN, "acc_no3",	16, 2, CHARTYPE,
		GlMask, "                          ",
		"0", " ", "Sales G/L Account    ", " ",
		 NI, NO,  JUSTLEFT, "0123456789", "", cmhr_rec.sal_glacc},
	{CM_SCN3, LIN, "salacc_desc",	16, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [3]},
	{CM_SCN3, LIN, "acc_no4",	17, 2, CHARTYPE,
		GlMask, "                          ",
		"0", " ", "COG G/L Account      ", " ",
		 NI, NO,  JUSTLEFT, "0123456789", "", cmhr_rec.cog_glacc},
	{CM_SCN3, LIN, "cogacc_desc",	17, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [4]},
	{CM_SCN3, LIN, "acc_no5",	 18, 2, CHARTYPE,
		GlMask, "                          ",
		"0", " ", "Variance G/L Account ", " ",
		 NI, NO,  JUSTLEFT, "0123456789", "", cmhr_rec.var_glacc},
	{CM_SCN3, LIN, "varacc_desc",	 18, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [5]},
	{CM_SCN3, LIN, "acc_no6",	 19, 2, CHARTYPE,
		GlMask, "                          ",
		"0", " ", "Internal G/L Account ", " ",
		 NI, NO,  JUSTLEFT, "0123456789", "", cmhr_rec.int_glacc},
	{CM_SCN3, LIN, "intacc_desc",	 19, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [6]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>

/*
 * Local function prototypes
 */
int		CheckCmtr			(void);
int		CheckCmts			(void);
int		GetGlmr				(char *, char *);
int		heading				(int);
int		LoadFields			(void);
int		LoadGlInfo			(void);
int		spec_valid			(int);
int		SrchCudi 			(int);
int		WarnUser			(char *, int);
long	ValidateContractNo	(long);
void	AddNewCmcbs			(void);
void	CloseDB			 	(void);
void	DeleteOldCmcbs		(void);
void	InitML				(void);
void	OpenDB				(void);
void	shutdown_prog		(void);
void	SrchCmhr2			(char *);
void	SrchCmhr			(char *);
void	SrchCmit			(char *);
void	SrchCmjt			(char *);
void	SrchCmws			(char *);
void	Update				(void);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char *	argv [])
{
	char *sptr = chk_env ("CM_AUTO_CON");

	if (sptr)
		byWhat = atoi (sptr);
	else
		byWhat = COMPANY;

	TruePosition	=	TRUE;
	/*
	 * Check and Get Credit terms.
	 */
	sptr = get_env ("SO_CRD_TERMS");
	dbStopCrd = (* (sptr + 0) == 'S');
	dbCrdTerm = (* (sptr + 1) == 'S');
	dbCrdOver = (* (sptr + 2) == 'S');

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	if (!envDbCo)
		envDbFind = TRUE;

	/*
	 * Read common terminal record.
	 */
	OpenDB ();

	GL_SetAccWidth (comm_rec.co_no, FALSE);

	init_scr ();
	set_tty (); 
	set_masks ();

	InitML ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");
	strcpy (cbranchNo, (byWhat != COMPANY) ? comm_rec.est_no : " 0");

	clear ();
	swide ();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
		exists		= FALSE;
	
		lcount [CM_SCN2]  = 0;
		init_vars (CM_SCN1);
		init_vars (CM_SCN2);
		init_vars (CM_SCN3);
		/*
		 * Enter screen 1 linear input. 
		 */
		heading (CM_SCN1);
		entry (CM_SCN1);
		if (prog_exit || restart)
			continue;


		scn_write (CM_SCN1);
		scn_display (CM_SCN1);

		if (exists)
			edit_all ();
		else
		{
			scn_display (CM_SCN2);
			entry (CM_SCN2);			
			if (prog_exit || restart)
				continue;

			heading (CM_SCN3);
			entry (CM_SCN3);			
			if (prog_exit || restart)
				continue;

			edit_all ();
		}
		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Database Files. 
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	abc_alias (cmhr2, cmhr);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, 
							(!envDbFind) ? "cumr_id_no" : "cumr_id_no3");

	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");

	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cudi, cudi_list, CUDI_NO_FIELDS, "cudi_id_no");
	open_rec (cmjt, cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmit, cmit_list, CMIT_NO_FIELDS, "cmit_id_no");
	open_rec (cmws, cmws_list, CMWS_NO_FIELDS, "cmws_id_no");
	open_rec (cmts, cmts_list, CMTS_NO_FIELDS, "cmts_hhhr_hash");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_hhhr_hash");
	open_rec (cmcb, cmcb_list, CMCB_NO_FIELDS, "cmcb_hhhr_hash");

	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmhr2,cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");

	if (byWhat == COMPANY)
		open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");
	else
		open_rec (esmr,esmr_list,ESMR_NO_FIELDS,"esmr_id_no");

	OpenGlmr ();
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cmhr);
	abc_fclose (cmhr2);
	abc_fclose (cmcd);
	abc_fclose (cudi);
	abc_fclose (cmjt);
	abc_fclose (cmit);
	abc_fclose (cmws);
	abc_fclose (cmts);
	abc_fclose (cmtr);
	abc_fclose (cmcb);

	if (byWhat == COMPANY)
		abc_fclose (comr);	
	if (byWhat == BRANCH)
		abc_fclose (esmr);	

	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int		i;
	double	total_owing = 0.00;

	if (LCHECK ("startDate"))
	{
		if (prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (cmhr_rec.st_date > cmhr_rec.due_date)
		{
			print_mess (ML (mlStdMess057));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
	}
	if (LCHECK ("dueDate"))
	{
		if (cmhr_rec.st_date > cmhr_rec.due_date)
		{
			print_mess (ML (mlStdMess058));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK ("wipStatus"))
	{
		if (SRCH_KEY)
		{
			SrchCmws (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmws_rec.co_no, comm_rec.co_no);
		strcpy (cmws_rec.wp_stat, cmhr_rec.wip_status);

		cc = find_rec (cmws, &cmws_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess059));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		DSP_FLD ("wip_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("expectedProfit"))
	{
		if (dflt_used)
		{
			if (cmhr_rec.quote_val != 0.00 && cmhr_rec.est_costs != 0.00)
			{
				cmhr_rec.est_prof = (float) ((cmhr_rec.quote_val -
										cmhr_rec.est_costs) /
										cmhr_rec.est_costs);

				cmhr_rec.est_prof *= 100.00;
			}
			else
				cmhr_rec.est_prof = 0.00;

			if (cmhr_rec.est_prof > 999.99 ||
			     cmhr_rec.est_prof < -99.99)
			{
				print_mess (ML (mlCmMess127));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			
			DSP_FLD ("expectedProfit");
			return (EXIT_SUCCESS);
		}
	}

	if (LCHECK ("issueTo"))
	{
		if (dflt_used)
		{
			sprintf (cmit_rec.issto, "%-10.10s", " ");
			sprintf (cmit_rec.iss_name, "%-40.40s", " ");
			cmhr_rec.hhit_hash = 0L;
			FLD ("issueDate")	=	NA;
			DSP_FLD ("issueTo");
			DSP_FLD ("issueName");
			return (EXIT_SUCCESS);
		}
		FLD ("issueDate")	=	YES;

		if (SRCH_KEY)
		{
			SrchCmit (temp_str);
			return (EXIT_SUCCESS);
		}
		else
		{
			strcpy (cmit_rec.co_no, comm_rec.co_no);
			cc = find_rec (cmit, &cmit_rec, EQUAL, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess054));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cmhr_rec.hhit_hash = cmit_rec.hhit_hash;

		DSP_FLD ("issueName");
		return (EXIT_SUCCESS);

	}

	if (LCHECK ("anniversaryDay"))
	{
		if (atoi (cmhr_rec.anni_day) > 31 || atoi (cmhr_rec.anni_day) < 1)
		{
			print_mess (ML (mlCmMess128));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK ("progress"))
	{
		if (cmhr_rec.progress [0] == 'Y')
		{
			strcpy (local_rec.progress, ML ("Yes"));
			FLD ("anniversaryDay") = YES;
		}
		else
		{
			strcpy (local_rec.progress, ML ("No "));
			sprintf (cmhr_rec.anni_day, "%-2.2s", " 1");
			FLD ("anniversaryDay") = ND;
		}

		DSP_FLD ("prog_desc");
		scn_write (CM_SCN3);
		DSP_FLD ("anniversaryDay");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("int_ext"))
	{
		if (cmhr_rec.internal [0] == 'Y')
			strcpy (local_rec.int_desc, ML ("Yes"));
		else
			strcpy (local_rec.int_desc, ML ("No "));

		DSP_FLD ("int_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("quote_type"))
	{
		if (cmhr_rec.quote_type [0] == 'F')
		{
			strcpy (local_rec.quote_desc, ML ("Fixed"));
			FLD ("quoteValue") = YES;
		}
		else
		{
			strcpy (local_rec.quote_desc, ML ("Variable"));
			FLD ("quoteValue") = NA;
			cmhr_rec.quote_val = 0.00;
			DSP_FLD ("quoteValue");
		}

		DSP_FLD ("quote_desc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate General Ledger Account Numbers.
	 */
	if (LNCHECK ("acc_no", 6))
	{
		i = atoi (FIELD.label + 6);

		if (dflt_used)
		{
			switch (i)
			{
				case	0:
					cc = GetGlmr (cmjt_rec.wip_glacc, local_rec.accDesc [i]);
					break;

				case	1:
					cc = GetGlmr (cmjt_rec.lab_glacc, local_rec.accDesc [i]);
					break;
			
				case	2:
					cc = GetGlmr (cmjt_rec.o_h_glacc, local_rec.accDesc [i]);
					break;
			
				case	3:
					cc = GetGlmr (cmjt_rec.sal_glacc, local_rec.accDesc [i]);
					break;
			
				case	4:
					cc = GetGlmr (cmjt_rec.cog_glacc, local_rec.accDesc [i]);
					break;
			
				case	5:
					cc = GetGlmr (cmjt_rec.var_glacc, local_rec.accDesc [i]);
					break;
			
				case	6:
					cc = GetGlmr (cmjt_rec.int_glacc, local_rec.accDesc [i]);
					break;
			}
		}

		if (SRCH_KEY)
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
		else
		{
			switch (i)
			{
				case	0:
					cc = GetGlmr (cmhr_rec.wip_glacc, local_rec.accDesc [i]);
					break;

				case	1:
					cc = GetGlmr (cmhr_rec.lab_glacc, local_rec.accDesc [i]);
					break;
			
				case	2:
					cc = GetGlmr (cmhr_rec.o_h_glacc, local_rec.accDesc [i]);
					break;
			
				case	3:
					cc = GetGlmr (cmhr_rec.sal_glacc, local_rec.accDesc [i]);
					break;
			
				case	4:
					cc = GetGlmr (cmhr_rec.cog_glacc, local_rec.accDesc [i]);
					break;
			
				case	5:
					cc = GetGlmr (cmhr_rec.var_glacc, local_rec.accDesc [i]);
					break;
			
				case	6:
					cc = GetGlmr (cmhr_rec.int_glacc, local_rec.accDesc [i]);
					break;
			}
			if (cc)
				return (EXIT_FAILURE);
		}
		display_field (field + 1);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("job_type"))
	{
		/*
		 * can not change job type if transactions exist or timesheets silly.
		 */
		if (DIFF_JOB && CheckCmtr ())
		{
			print_mess (ML (mlCmMess129));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (DIFF_JOB && CheckCmts ())
		{
			print_mess (ML (mlCmMess130));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			SrchCmjt (temp_str);
			return (EXIT_SUCCESS);
		}
		else
		{
			strcpy (cmjt_rec.co_no, comm_rec.co_no);
			strcpy (cmjt_rec.br_no, comm_rec.est_no);
			sprintf (cmjt_rec.job_type, "%-4.4s", temp_str);

			cc = find_rec (cmjt, &cmjt_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess056));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (DIFF_JOB)
		{
			i = prmptmsg (ML (mlCmMess138), "YNyn", 10, 2);

			print_at (2,10, "%-100.100s", " ");

			if (i == 'N' || i == 'n')
				return (EXIT_FAILURE);
		}

		if (LoadGlInfo ())
		{
			print_mess (ML (mlCmMess131));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cmhr_rec.hhjt_hash = cmjt_rec.hhjt_hash;
		scn_display (CM_SCN3);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cus_addr1"))
	{
		if (SRCH_KEY)
		{
			cc = SrchCudi (0);
			if (cc < 0)
				return (cc);
			sprintf (temp_str,  "%-40.40s", cudi_rec.adr1);
			sprintf (cmhr_rec.adr1, "%-40.40s", cudi_rec.adr2);
			sprintf (cmhr_rec.adr2, "%-40.40s", cudi_rec.adr3);
			if (prog_status != ENTRY)
				scn_display (CM_SCN1);
			else
			{
				display_field (field + 1);
				display_field (field + 2);
				skip_entry = 2;
			}
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("mcont_no"))
	{
		if (SRCH_KEY)
		{
			SrchCmhr2 (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			cmhr_rec.mast_hhhr = 0L;
			DSP_FLD ("mcont_no");
			return (EXIT_SUCCESS);
		}

		pad_num (cmhr2_rec.cont_no);

		if (!strcmp (cmhr_rec.cont_no, cmhr2_rec.cont_no))
		{
			print_mess (ML (mlCmMess132));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (cmhr2_rec.co_no, comm_rec.co_no);
		if (byWhat == BRANCH || byWhat == USER)
			strcpy (cmhr2_rec.br_no, comm_rec.est_no);

		cc = find_rec (cmhr2, &cmhr2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess133));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cmhr_rec.mast_hhhr = cmhr2_rec.hhhr_hash;
		if (cmhr_rec.mast_hhhr)
		{
			cmcd_rec.hhhr_hash = cmhr_rec.mast_hhhr;
			strcpy (cmcd_rec.stat_flag, "D");
			cmcd_rec.line_no = 0;
			cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cmcd, "DBFIND");
			
			sprintf (local_rec.masterContractDesc, "%-60.60s", cmcd_rec.text);
			DSP_FLD ("masterContractDesc");
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer Number.
	 */
	if (LCHECK ("customerNo"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.dbt_no));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		while (cc)
		{
			i = prmptmsg (ML (mlCmMess139), "YNyn", 20,2);

			if (i == 'n' || i == 'N')
				return (EXIT_FAILURE);

			snorm ();
			* (arg) = "db_mr_inpt";
			* (arg+ (1)) = (char *)0;
			shell_prog (2);
			swide ();
			strcpy (cumr_rec.co_no,comm_rec.co_no);
			strcpy (cumr_rec.est_no,branchNo);
			strcpy (cumr_rec.dbt_no,pad_num (local_rec.dbt_no));
			cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
			
			heading (CM_SCN1);
			scn_display (CM_SCN1);
		}

		/*
		 * Check if customer is on stop credit.
		 */
		if (cumr_rec.stop_credit [0] == 'Y')
		{
			if (dbStopCrd)
			{
				print_mess (ML (mlStdMess060));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				cc = WarnUser (ML (mlStdMess060),0);
				if (cc)
					return (cc);
				heldOrder = TRUE;
			}
		}

		total_owing = cumr_balance [0] + 
					  cumr_balance [1] +
		              cumr_balance [2] + 
					  cumr_balance [3] +
					  cumr_balance [4] +
					  cumr_balance [5];

		c_left = total_owing - cumr_rec.credit_limit;

		/*
		 * Check if customer is over his credit limit.
		 */
		if (cumr_rec.credit_limit <= total_owing && 
			cumr_rec.credit_limit != 0.00 &&
			cumr_rec.crd_flag [0] != 'Y')
		{
			if (dbCrdOver)
			{
				print_mess (ML (mlStdMess061));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				cc = WarnUser (ML (mlStdMess061),0);
				if (cc)
					return (EXIT_FAILURE);
				heldOrder = TRUE;
			}
		}
		/*
		 * Check Credit Terms
		 */
		if (cumr_rec.od_flag && cumr_rec.crd_flag [0] != 'Y')
		{
			if (dbCrdTerm)
			{
				sprintf (err_str,ML (mlStdMess062), cumr_rec.od_flag);
				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				sprintf (err_str,ML (mlStdMess062), cumr_rec.od_flag);
				cc = WarnUser (err_str,0);
				if (cc)
					return (EXIT_FAILURE);

				heldOrder = TRUE;
			}
			
		}

		if (cumr_rec.ho_dbt_hash != 0L)
		{
			cumr2_rec.hhcu_hash	= cumr_rec.ho_dbt_hash;
		    cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
		}
		else
		{
			strcpy (cumr2_rec.dbt_no, "000000");
			strcpy (cumr2_rec.dbt_name, ML ("No Master Account"));
		}

		cmhr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		sprintf (cmhr_rec.contact,	"%-40.40s", cumr_rec.contact_name);
		sprintf (cmhr_rec.adr1, 	"%-40.40s", cumr_rec.dl_adr1);
		sprintf (cmhr_rec.adr2, 	"%-40.40s", cumr_rec.dl_adr2);
		sprintf (cmhr_rec.adr3, 	"%-40.40s", cumr_rec.dl_adr3);

		if (heldOrder)
			cmhr_rec.status [0] = 'X';

		DSP_FLD ("mcust_no");
		DSP_FLD ("name");
		DSP_FLD ("mname");

		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("cont_no"))
	{
		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, cbranchNo);

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/*
			 * means not Maint. 
			 */
			if (byWhat == USER)
			{
				print_mess (ML (mlCmMess134));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}	

			sprintf (cmhr_rec.cont_no,"%-6.6s", "NEW");
		}
		else
		{
			pad_num (cmhr_rec.cont_no);
			strcpy (cmhr_rec.co_no, comm_rec.co_no);	
			strcpy (cmhr_rec.br_no, cbranchNo);	
			cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
			if (!cc)
			{
				/*
				 * make sure not closed
				 */
				if (cmhr_rec.status [0] == 'B')
				{
					print_mess (ML (mlCmMess135));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				if (cmhr_rec.status [0] == 'H' ||
				     cmhr_rec.status [0] == 'C')
				{
					print_mess (ML (mlCmMess136));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}

				if (cmhr_rec.status [0] == 'X')
				{
					print_mess (ML (mlCmMess137));
					sleep (sleepTime);
					clear_mess ();
				}
				/*
				 * read again with proper locks
				 */
				cc = find_rec (cmhr, &cmhr_rec, EQUAL, "u");
				if (cc)
					file_err (cc, cmhr, "DBFIND");

				exists = TRUE;
				cc = LoadFields ();
				if (cc)
					return (cc);

				scn_display (CM_SCN1);
				entry_exit = TRUE;
			}
			else
				exists = FALSE;
		}
		
		if (!exists)
		{
			/*
			 * read again with proper locks
			 */
			cc = find_rec (cmhr, &cmhr_rec, EQUAL, "w");

			cmhr_rec.status [0] = 'O';
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlCmMess142), 50, 0, 1);

	if (scn == CM_SCN1 || scn == CM_SCN2)
	{
		box (0, 2, 131, 9);
		line_at (7,1,130);
		if (scn == CM_SCN1)
		{
			scn_set (CM_SCN2);
			scn_write (CM_SCN2);
			scn_display (CM_SCN2);
		}
		else
		{
			scn_set (CM_SCN1);
			scn_write (CM_SCN1);
			scn_display (CM_SCN1);
		}
	}

	if (scn == CM_SCN3)
	{
		box (0, 2, 131, 17);
		line_at (8,1,130);
		line_at (11,1,130);
	}

	line_at (21,0,132);

	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 1, err_str, comm_rec.co_no, comm_rec.co_name);

	line_at (1,0,132);
	
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

long
ValidateContractNo (
 long	job)
{
	strcpy (cmhr2_rec.co_no, comm_rec.co_no);
	strcpy (cmhr2_rec.br_no, cbranchNo);
	sprintf (cmhr2_rec.cont_no, "%06ld", job);

	cc = find_rec (cmhr2, &cmhr2_rec, COMPARISON, "r");
		
	while (!cc)
	{
		sprintf (cmhr2_rec.cont_no, "%06ld", ++job);
		cc = find_rec (cmhr2, &cmhr2_rec, COMPARISON, "r");
	}
	return (job);
}

void
SrchCmhr (
 char *	key_val)
{
	_work_open (6,0,40);
	save_rec ("#No.", "#Customer Order No.");
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	sprintf (cmhr_rec.cont_no, "%-6.6s", key_val);

	strcpy (cmhr_rec.br_no, cbranchNo);

	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
			!strncmp (cmhr_rec.cont_no, key_val,strlen (key_val)))
	{
		if (byWhat == BRANCH || byWhat == USER)
		{
			if (strcmp (cmhr_rec.br_no, cbranchNo) > 0)
				break;
		}

		if (cmhr_rec.status [0] != 'O' && cmhr_rec.status [0] != 'X')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, cbranchNo);
	sprintf (cmhr_rec.cont_no, "%6.6s", temp_str);

	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

void
SrchCmhr2 (
 char *	key_val)
{
	_work_open (6,0,40);
	save_rec ("#No.", "#Customer Order No.");
	strcpy (cmhr2_rec.co_no, comm_rec.co_no);
	strcpy (cmhr2_rec.br_no, cbranchNo);
	sprintf (cmhr2_rec.cont_no, "%6.6s", pad_num (key_val));

	cc = find_rec (cmhr2, &cmhr2_rec, GTEQ, "r");
	while (!cc && !strcmp (cmhr2_rec.co_no, comm_rec.co_no) &&
			strncmp (cmhr2_rec.cont_no, key_val,strlen (key_val)) > 0)
	{
		if (strcmp (cmhr2_rec.br_no, cbranchNo))
				break;

		if (cmhr2_rec.status [0] != 'O' && cmhr2_rec.status [0] != 'X')
		{
			cc = find_rec (cmhr2, &cmhr2_rec, NEXT, "r");
			continue;
		}

		save_rec (cmhr2_rec.cont_no, cmhr2_rec.cus_ref);
		cc = find_rec (cmhr2, &cmhr2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (cmhr2_rec.cont_no, "%6.6s", temp_str);

}
/*
 * Warn user about something.
 */
int
WarnUser (
 char *	wn_mess,
 int	wn_flip)
{
	int		i;

	clear_mess ();
	print_mess (wn_mess);

	if (!wn_flip)
	{
		i = prmptmsg (ML (mlCmMess140),"YyNnMm",20,2);
		
		heading (CM_SCN1);
		scn_display (CM_SCN1);

		if (i == 'Y' || i == 'y') 
			return (EXIT_SUCCESS);

		if (i == 'M' || i == 'm') 
		{
			DbBalWin (cumr_rec.hhcu_hash,comm_rec.fiscal, comm_rec.dbt_date);
			i = prmptmsg (ML (mlCmMess141), "YyNn",20,2);
			
			heading (CM_SCN1);
			scn_display (CM_SCN1);
			if (i == 'Y' || i == 'y') 
				return (EXIT_SUCCESS);
		}
		return (EXIT_FAILURE);
	}

	if (wn_flip == 9)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
Update (void)
{
	long	nx_job;

	if (DIFF_JOB)
		DeleteOldCmcbs ();

	if (!strncmp (cmhr_rec.cont_no, "NEW", 3))
	{
		if (byWhat == COMPANY)
		{
			strcpy (comr_rec.co_no, comm_rec.co_no);
			cc = find_rec (comr,&comr_rec,COMPARISON,"u");
			if (cc)
				file_err (cc, comr, "DBFIND");
			
			nx_job = comr_rec.nx_job_no;

			/*
			 * make sure no number not already used.
			 */

			comr_rec.nx_job_no = ValidateContractNo (nx_job + 1);

			cc = abc_update (comr, &comr_rec);
			if (cc)
				file_err (cc, comr, "DBUPDATE");

			sprintf (cmhr_rec.cont_no,"%06ld", comr_rec.nx_job_no);
			
		}
		else
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no,comm_rec.est_no);
			cc = find_rec (esmr,&esmr_rec,COMPARISON,"u");
			if (cc)
				file_err (cc, esmr, "DBFIND");
			
			nx_job = esmr_rec.nx_job_no;

			/*
			 * make sure no number not
			 */
			esmr_rec.nx_job_no = ValidateContractNo (nx_job);

			cc = abc_update (esmr, &esmr_rec);
			if (cc)
				file_err (cc, esmr, "DBUPDATE");

			sprintf (cmhr_rec.cont_no,"%06ld", esmr_rec.nx_job_no);
		}
		clear ();
		print_at (1, 10,ML (mlCmMess143), cmhr_rec.cont_no);
		PauseForKey (2, 10, ML (mlStdMess042), 0);
	}

	if (exists)
		cc = abc_update (cmhr, &cmhr_rec);
	else
		cc = abc_add (cmhr, &cmhr_rec);

	if (cc)
		file_err (cc, cmhr, (exists) ? "DBUPDATE" : "DBADD");

	if (!exists)
	{
		/*
		 * read again to get hash 
		 */
		strcpy (cmhr_rec.co_no, comm_rec.co_no);	
		strcpy (cmhr_rec.br_no, cbranchNo);	
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cmhr, "DBFIND");
	}

	scn_display (CM_SCN2);

	for (line_cnt = 0;line_cnt < lcount [CM_SCN2] ;line_cnt++)
	{
		getval (line_cnt);

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = line_cnt;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (cmcd_rec.text, "%-70.70s", local_rec.contDesc);

			cc = abc_add (cmcd, &cmcd_rec);
			if (cc)
				file_err (cc, cmcd, "DBFIND");
		}
		else
		{
			sprintf (cmcd_rec.text, "%-70.70s", local_rec.contDesc);

			cc = abc_update (cmcd, &cmcd_rec);
			if (cc)
				file_err (cc, cmcd, "DBUPDATE");
		}
	}

	/*
	 * Delete old lines
	 */
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = line_cnt;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "u");
	while (!cc && cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
				  cmcd_rec.stat_flag [0] == 'D')
	{
		abc_delete (cmcd);

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = line_cnt;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "u");
	}
	abc_unlock (cmcd);

	if (DIFF_JOB || !exists)
		AddNewCmcbs ();
}

int
LoadFields (void)
{
	int		i,
			currentScreen	=	cur_screen;

	abc_selfield (cumr, "cumr_hhcu_hash");
	abc_selfield (cmjt, "cmjt_hhjt_hash");

	cumr_rec.hhcu_hash = cmhr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");
	
	if (cumr_rec.ho_dbt_hash)
	{
		cumr2_rec.hhcu_hash = cumr_rec.ho_dbt_hash;
		cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
	}
	else
	{
		strcpy (cumr2_rec.dbt_no, "000000");
		strcpy (cumr2_rec.dbt_name, ML ("No Master Account"));
	}

	if (cc)
		file_err (cc, cumr2, "DBFIND");

	strcpy (local_rec.dbt_no, cumr_rec.dbt_no);

	if (cmhr_rec.mast_hhhr)
	{
		abc_selfield (cmhr2, "cmhr_hhhr_hash");

		cmhr2_rec.hhhr_hash	= cmhr_rec.mast_hhhr;
		cc = find_rec (cmhr2, &cmhr2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cmhr2, "DBFIND");
		else 
			abc_selfield (cmhr2, "cmhr_id_no");

		cmcd_rec.hhhr_hash = cmhr_rec.mast_hhhr;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cmcd, "DBFIND");
		
		sprintf (local_rec.masterContractDesc, "%-60.60s", cmcd_rec.text);
	}
	else
	{
		strcpy (cmhr2_rec.cont_no, "      ");
		sprintf (local_rec.masterContractDesc, "%60.60s", " ");
	}

	scn_set (CM_SCN2);
	lcount [CM_SCN2] = 0;

	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
	while (!cc && cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
				  cmcd_rec.stat_flag [0] == 'D')
  	{
		strcpy (local_rec.contDesc, cmcd_rec.text);
		putval (lcount [CM_SCN2]++);
		cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
		
	}
	scn_set (currentScreen);

	cmjt_rec.hhjt_hash = cmhr_rec.hhjt_hash;
	cc = find_rec (cmjt, &cmjt_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cmjt, "DBFIND");

	strcpy (local_rec.old_job, cmjt_rec.job_type);

	for (i = 0; i < 7; i++)
	{
		switch (i)
		{
			case	0:
				cc = GetGlmr (cmhr_rec.wip_glacc, local_rec.accDesc [i]);
				break;

			case	1:
				cc = GetGlmr (cmhr_rec.lab_glacc, local_rec.accDesc [i]);
				break;
		
			case	2:
				cc = GetGlmr (cmhr_rec.o_h_glacc, local_rec.accDesc [i]);
				break;
		
			case	3:
				cc = GetGlmr (cmhr_rec.sal_glacc, local_rec.accDesc [i]);
				break;
		
			case	4:
				cc = GetGlmr (cmhr_rec.cog_glacc, local_rec.accDesc [i]);
				break;
		
			case	5:
				cc = GetGlmr (cmhr_rec.var_glacc, local_rec.accDesc [i]);
				break;
		
			case	6:
				cc = GetGlmr (cmhr_rec.int_glacc, local_rec.accDesc [i]);
				break;
		}
	}

	if (cmhr_rec.quote_type [0] == 'F')
	{
		strcpy (local_rec.quote_desc, ML ("Fixed"));
		FLD ("quoteValue") = YES;
	}
	else
	{
		strcpy (local_rec.quote_desc, ML ("Variable"));
		FLD ("quoteValue") = NA;
	}
	
	if (cmhr_rec.internal [0] == 'Y')
		strcpy (local_rec.int_desc, ML ("Yes"));
	else
		strcpy (local_rec.int_desc, ML ("No"));

	if (cmhr_rec.progress [0] == 'Y')
	{
		strcpy (local_rec.progress, ML ("Yes"));
		FLD ("anniversaryDay") = YES;
	}
	else
	{
		strcpy (local_rec.progress, ML ("No"));
		sprintf (cmhr_rec.anni_day, "%-2.2s", " 1");
		FLD ("anniversaryDay") = ND;
	}

	if (cmhr_rec.hhit_hash != 0L)
	{
		abc_selfield (cmit, "cmit_hhit_hash");

		cmit_rec.hhit_hash = cmhr_rec.hhit_hash;
		cc = find_rec (cmit, &cmit_rec, EQUAL, "r");

		abc_selfield (cmit, "cmit_id_no");
	}

	if (strcmp (cmhr_rec.wip_status, "    "))
	{
		strcpy (cmws_rec.co_no, comm_rec.co_no);
		strcpy (cmws_rec.wp_stat, cmhr_rec.wip_status);

		cc = find_rec (cmws, &cmws_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cmws, "DBFIND");
	}
	
	heading (CM_SCN1);
	scn_display (CM_SCN1);

	abc_selfield (cmjt, "cmjt_id_no");
	abc_selfield (cumr, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
	return (EXIT_SUCCESS);
}

void
SrchCmjt (
 char *	key_val)
{

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", key_val);

	_work_open (4,0,40);
	save_rec ("#Type", "#Job Type Description");

	cc = find_rec (cmjt, &cmjt_rec, GTEQ, "r");

	while (!cc && !strcmp (cmjt_rec.co_no, comm_rec.co_no))
	{
		if (byWhat != COMPANY && strcmp (cmjt_rec.br_no,
							comm_rec.est_no))
			break;

		if (strncmp (cmjt_rec.job_type, key_val, strlen (key_val)))
		{
			cc = find_rec (cmjt, &cmjt_rec, NEXT, "r");
			continue;
		}

		save_rec (cmjt_rec.job_type, cmjt_rec.desc);
		cc = find_rec (cmjt, &cmjt_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", temp_str);

	cc = find_rec (cmjt, &cmjt_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cmjt, "DBFIND");
}

int
LoadGlInfo (
 void)
{
	int i;

	/*
	 * Lookup GL account descriptions.
	 */
	for (i = 0; i < 7; i++)
	{
		switch (i)
		{
			case	0:
				strcpy (cmhr_rec.wip_glacc, cmjt_rec.wip_glacc);
				cc = GetGlmr (cmhr_rec.wip_glacc, local_rec.accDesc [i]);
				break;

			case	1:
				strcpy (cmhr_rec.lab_glacc, cmjt_rec.lab_glacc);
				cc = GetGlmr (cmhr_rec.lab_glacc, local_rec.accDesc [i]);
				break;
		
			case	2:
				strcpy (cmhr_rec.o_h_glacc, cmjt_rec.o_h_glacc);
				cc = GetGlmr (cmhr_rec.o_h_glacc, local_rec.accDesc [i]);
				break;
		
			case	3:
				strcpy (cmhr_rec.sal_glacc, cmjt_rec.sal_glacc);
				cc = GetGlmr (cmhr_rec.sal_glacc, local_rec.accDesc [i]);
				break;
		
			case	4:
				strcpy (cmhr_rec.cog_glacc, cmjt_rec.cog_glacc);
				cc = GetGlmr (cmhr_rec.cog_glacc, local_rec.accDesc [i]);
				break;
		
			case	5:
				strcpy (cmhr_rec.var_glacc, cmjt_rec.var_glacc);
				cc = GetGlmr (cmhr_rec.var_glacc, local_rec.accDesc [i]);
				break;
		
			case	6:
				strcpy (cmhr_rec.int_glacc, cmjt_rec.int_glacc);
				cc = GetGlmr (cmhr_rec.int_glacc, local_rec.accDesc [i]);
				break;
		}
		if (cc)
		{
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Find account number, account description etc. 
 */
int
GetGlmr (
 char *	acc_no,
 char *	acc_desc)
{
	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (acc_no, glmrRec.acc_no, 0);
		
	if ((cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
	{
		print_mess (ML (mlStdMess024));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0] != 'P')
	{
		print_mess (ML (mlStdMess025));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	sprintf (acc_desc, "%-40.40s", glmrRec.desc);
	
	return (EXIT_SUCCESS);
}

void
SrchCmit (
 char *	key_val)
{
	_work_open (10,0,40);
	save_rec ("#Issue To", "#Issue To Name");

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", key_val);
	cc = find_rec (cmit, &cmit_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmit_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmit_rec.issto, key_val, strlen (key_val)))
	{
		cc = save_rec (cmit_rec.issto, cmit_rec.iss_name);
		if (cc)
			break;

		cc = find_rec (cmit, &cmit_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", temp_str);
	cc = find_rec (cmit, &cmit_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmit, "DBFIND");
}

void
SrchCmws (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#No.", "#WIP Status Description ");

	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%-4.4s", key_val);
	cc = find_rec (cmws, &cmws_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmws_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmws_rec.wp_stat, key_val, strlen (key_val)))
	{
		cc = save_rec (cmws_rec.wp_stat, cmws_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmws, &cmws_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%-4.4s", temp_str);
	cc = find_rec (cmws, &cmws_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmws, "DBFIND");
}


int
CheckCmts (void)
{
	cmts_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	return (!find_rec (cmts, &cmts_rec, EQUAL, "r"));
}

int
CheckCmtr (void)
{
	cmtr_rec.hhhr_hash	= cmhr_rec.hhhr_hash;
	return (!find_rec (cmtr, &cmtr_rec, EQUAL, "r"));
}

void
DeleteOldCmcbs (void)
{
	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cc = find_rec (cmcb, &cmcb_rec, EQUAL, "u");
	while (!cc)
	{
		abc_delete (cmcb);
		cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cc = find_rec (cmcb, &cmcb_rec, EQUAL, "u");
	}
	abc_unlock (cmcb);
}

void
AddNewCmcbs (void)
{
	open_rec (cmjd,cmjd_list,CMJD_NO_FIELDS,"cmjd_id_no");
	cmjd_rec.line_no = 0;
	cmjd_rec.hhjt_hash = cmjt_rec.hhjt_hash;

	cc = find_rec (cmjd, &cmjd_rec, GTEQ, "r");

	while (!cc && cmjd_rec.hhjt_hash == cmjt_rec.hhjt_hash)
	{
		cmcb_rec.hhcm_hash = cmjd_rec.hhcm_hash;
		cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcb_rec.dtl_lvl, cmjd_rec.dtl_lvl);

		cc = abc_add (cmcb, &cmcb_rec);
		if (cc)
			file_err (cc, cmcb, "DBADD");

		cc = find_rec (cmjd, &cmjd_rec, NEXT, "r");
	}

	abc_fclose (cmjd);
}

void
InitML (void)
{
	GlAccName [0] = strdup (ML (mlCmMess199));
	GlAccName [1] = strdup (ML (mlCmMess200));
	GlAccName [2] = strdup (ML (mlCmMess201));
	GlAccName [3] = strdup (ML (mlCmMess202));
	GlAccName [4] = strdup (ML (mlCmMess203));
	GlAccName [5] = strdup (ML (mlCmMess204));
	GlAccName [6] = strdup (ML (mlCmMess205));
}
	
int
SrchCudi (
	int		indx)
{
	char	workString [170];

	_work_open (5,0,80);
	save_rec ("#DelNo","#Delivery Details");
	cudi_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= 0;
	cc = find_rec (cudi, &cudi_rec, GTEQ, "r");
	while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{                        
		sprintf 
		(
			workString,"%s, %s, %s, %s",
			clip (cudi_rec.name),
			clip (cudi_rec.adr1),
			clip (cudi_rec.adr2),
			clip (cudi_rec.adr3)
		);
		sprintf (err_str, "%05d", cudi_rec.del_no);
		cc = save_rec (err_str, workString); 
		if (cc)
			break;

		cc = find_rec (cudi, &cudi_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (-1);

	cudi_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= atoi (temp_str);
	cc = find_rec (cudi,&cudi_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, cudi, "DBFIND");

	switch (indx)
	{
	case	0:
		sprintf (temp_str,"%-40.40s",cudi_rec.name);
		break;

	case	1:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr1);
		break;

	case	2:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr2);
		break;

	case	3:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr3);
		break;

	default:
		break;
	}
	return (cudi_rec.del_no);
}
