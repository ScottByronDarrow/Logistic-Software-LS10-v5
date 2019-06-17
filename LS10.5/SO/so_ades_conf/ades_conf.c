/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ades_conf.c,v 5.5 2002/04/30 07:56:37 scott Exp $
|  Program Name  : ( so_ades_conf.c )                                 |
|  Program Desc  : ( New Automatic / Manual despatch confirmation.  ) |
|---------------------------------------------------------------------|
|  Date Written  : (20/02/1997)  | Author       : Scott B Darrow.     |
|---------------------------------------------------------------------|
| $Log: ades_conf.c,v $
| Revision 5.5  2002/04/30 07:56:37  scott
| Update for new Archive modifications;
|
| Revision 5.4  2002/04/29 07:47:05  scott
| Update for new Archive modifications;
|
| Revision 5.3  2001/10/25 08:14:01  scott
| Updated from changes made to transport
|
| Revision 5.2  2001/08/09 09:20:31  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:50:43  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/23 04:06:02  scott
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ades_conf.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_ades_conf/ades_conf.c,v 5.5 2002/04/30 07:56:37 scott Exp $";

#include	<pslscr.h>
#include	<hot_keys.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>
#include	<ml_tr_mess.h>
#include	<CustomerService.h>
#include	<Archive.h>

#include    <tabdisp.h>

#define		SERIAL_ITEM	 (inmr_rec.serial_item [0] == 'Y')
#define		NON_STOCK	 (inmr_rec.inmr_class [0] == 'Z')
#define		SUR_CHARGE	 (sohr_rec.sohr_new [0] == 'Y' && \
		                 (cumr_rec.sur_flag [0] == 'Y' || \
		                  cumr_rec.sur_flag [0] == 'y'))

#define 	AUTO_SK_UP	 (createStatusFlag [0] == automaticStockUpdate [0])
#define		TRANSPORT	 (findStatusFlag [0] == 'T')

#include 	<twodec.h>
#include 	<proc_sobg.h>

	extern	int	EnvScreenOK;
	extern	int	TruePosition;


	char	get_buf [200];

	int		printerNumber 		= 1,
			nuumberTabLines 	= 0,
			PipeOpen 			= FALSE,
			firstTime 			= TRUE,	 /* first time in main while	*/
			notax				= FALSE, /* notax for this debtor		*/
			envCombInvPack 		= FALSE,
			audit_done 			= FALSE,
			envSoRtDelete  		= TRUE,
			envSoDoi  			= FALSE,
			envSoFreightBord	= TRUE,
			envDbNettUsed 		= TRUE;

	double	taxTotal 			= 0.00,
			lineTotal 			= 0.00,
			lineDiscount 		= 0.00,
			lineTax 			= 0.00,
			lineGst 			= 0.00;

	char	createStatusFlag [2],
			findStatusFlag [2],
			automaticStockUpdate [2],
			dbt_str [7],
			dbt_name [41],
			order_str [9],
			reason [61];

	long	des_date = 0L;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct solnRecord	soln2_rec;
struct exsfRecord	exsf_rec;
struct trzmRecord	trzm_rec;

	char 	*soln2 = "soln2",
			*data = "data";

	FILE	*pout;


static	int	tag_func 	(int, KEY_TAB *);
static	int	abort_func 	(int, KEY_TAB *);

#ifdef	GVISION
static KEY_TAB ind_keys [] =
{
    { NULL,		FN1, abort_func,
	"",								"A" },
    { " TAG/UNTAG ",	'T', tag_func,
	"Tag/Untag current line.",					"A" },
    { " ACCEPT ALL ",	CTRL ('A'), tag_func,
	"Tag/Untag All Lines.",						"A" },
    END_KEYS
};
#else
static	KEY_TAB ind_keys [] = 
{
    { NULL,		FN1, abort_func,
	"",								"A" },
    { " [T]AG/UNTAG ",	'T', tag_func,
	"Tag/Untag current line.",					"A" },
    { " [^A]ccept All ",	CTRL ('A'), tag_func,
	"Tag/Untag All Lines.",						"A" },
    END_KEYS
};
#endif

/*
 * Local & Screen Structures 
 */
struct {
	char	dummy [11];
	char	systemDate [11];
	char	StartInv [9],
			EndInv [9],
			trueInv [9];
	char	StartSalesman [3],
			DStartSalesman [41],
			EndSalesman [3],
			DEndSalesman [41],
			trueSalesman [3];
	char	StartZone [7],
			DStartZone [41],
			EndZone [7],
			DEndZone [41],
			trueZone [7];
	long	StartDate,
			EndDate;
	long	inv_date;
} local_rec;            

static	struct	var	vars []	=	
{
	{1, LIN, "s_pslip_no", 3, 2, CHARTYPE, 
		"UUUUUUUU", "        ", 
		"0", "        ", " Start Packing Slip No  ", "Enter start packing slip number <default start> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.StartInv}, 
	{1, LIN, "e_pslip_no", 3, 65, CHARTYPE, 
		"UUUUUUUU", "        ", 
		"0", "~~~~~~~~", " End Packing Slip No    ", "Enter End packing slip number <default End> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.EndInv}, 
	{1, LIN, "StartDate", 4, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", " Start Delivery Date    ", "Enter Start date. <default First date> ", 
		YES, NO, JUSTLEFT, " ", "", (char *)&local_rec.StartDate}, 
	{1, LIN, "EndDate", 4, 65, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, " End Delivery Date      ", "Enter End date. <default Today> ", 
		YES, NO, JUSTLEFT, " ", "", (char *)&local_rec.EndDate}, 
	{1, LIN, "StartSalesman", 5, 2, CHARTYPE, 
		"UU", "          ", 
		" ", " ", " Start Salesman code    ", "Enter start salesman code. <default = spaces> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.StartSalesman}, 
	{1, LIN, "DStartSalesman", 5, 65, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "  ", " Salesman Description   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.DStartSalesman}, 
	{1, LIN, "EndSalesman", 6, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "~~", " End Salesman code      ", "Enter end salesman code. <default = end of range> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.EndSalesman}, 
	{1, LIN, "DEndSalesman", 6, 65, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " Salesman Description   ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.DEndSalesman}, 
	{1, LIN, "StartZone", 7, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", " Start Delivery Zone    ", "Enter start delivery Zone. <default = spaces> ", 
		YES, NO, JUSTLEFT, "", "", local_rec.StartZone}, 
	{1, LIN, "DStartZone", 7, 65, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " Zone Description       ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.DStartZone}, 
	{1, LIN, "EndZone", 8, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "~", " End Delivery Zone      ", "Enter end delivery Zone. <default = end of range> ", 
		YES, NO, JUSTLEFT, "", "", local_rec.EndZone}, 
	{1, LIN, "DEndZone", 8, 65, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " Zone Description       ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.DEndZone}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};


/*
 * Function Declarations 
 */
char 	*CheckVariable 		(char *, char *);
char 	*_CheckVariable 	(char *, char *, char *, char *);
int 	ProcessCustomer 	(void);
int 	heading 			(int);
int 	spec_valid 			(int);
void 	CheckSalesman 		(void);
void 	CloseDB 			(void);
void 	DeleteSohr 			(long);
void 	HeadingOutput 		(void);
void 	OpenDB 				(void);
void 	PrintAudit 			(void);
void 	ProcPackingSlips 	(void);
void 	ProcessCohr 		(void);
void 	ProcessSelected 	(void);
void 	ReadMisc 			(void);
void 	SrchCohrPS 			(char *);
void 	SrchExsf 			(char *);
void 	SrchTrzm 			(char *);
void 	UpdatePackingSlip 	(void);
void 	shutdown_prog 		(void);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	sprintf (automaticStockUpdate, "%-1.1s",get_env ("AUTO_SK_UP"));

	if (argc != 4)
	{
		print_at (0,0, mlSoMess753 ,argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sprintf (createStatusFlag,"%-1.1s",argv [1]);
	sprintf (findStatusFlag,"%-1.1s",argv [2]);

	printerNumber = atoi (argv [3]);

	init_scr (); 
	set_tty (); 
	set_masks (); 

	sptr = chk_env ("SO_RT_DELETE");
	envSoRtDelete = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("SO_DOI");
	envSoDoi = (sptr == (char *)0 || sptr [1] == 'S') ? TRUE : FALSE;

	sptr = chk_env ("SO_FREIGHT_BORD");
	envSoFreightBord = (sptr == (char *)0) ? 1 : atoi (sptr);

	/*
	 * open main database files. 
	 */
	OpenDB ();

	strcpy (err_str,CheckVariable ("COMB_INV_PAC","N"));
	if (err_str [0] == 'Y' || err_str [0] == 'y')
		envCombInvPack = TRUE;

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	des_date = TodaysDate ();

	while (prog_exit == 0)
	{
		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		heading (1);
		scn_display (1);
		ProcPackingSlips ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence 
 */
void
shutdown_prog (void)
{
	if (PipeOpen)
	{
		fprintf (pout,".EOF\n");
		pclose (pout);
	}
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Database Files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	ReadMisc ();

	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, "trzm_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	abc_alias (soln2, soln);
	open_rec (soln2,soln_list, SOLN_NO_FIELDS, "soln_id_no");
}

/*
 * Close Database Files 
 */
void
CloseDB (void)
{
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (trzm);
	abc_fclose (exsf);
	abc_fclose (soln2);
	ArchiveClose ();
	abc_dbclose (data);
}

/*
 * Get common info from commom database file. 
 */
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc,"comr","DBFIND");

	abc_fclose (comr);

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc,"ccmr","DBFIND");

	abc_fclose (ccmr);
}

int
spec_valid (
 int field)
{
	/*
	 * Validate pack Slip Number.	
	 */
	if (LCHECK ("s_pslip_no")) 
	{
		if (SRCH_KEY)
		{
			SrchCohrPS (temp_str);
			return (EXIT_SUCCESS);
		}
		/*
		 * Check if order is on file. 
		 */
		strcpy (cohr_rec.co_no,comm_rec.co_no);
		strcpy (cohr_rec.br_no,comm_rec.est_no);
		strcpy (cohr_rec.type,"P");
		sprintf (cohr_rec.inv_no, local_rec.StartInv);

		if (!dflt_used)
		{
			cc = find_rec ("cohr",&cohr_rec,COMPARISON,"r");
			if (cc) 
			{
				errmess (ML (mlSoMess227));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
			/*
			 * Check if packing slip printed, if not then have to ignore. 
			 */
			if (cohr_rec.ps_print [0] != 'Y')
			{
				errmess (ML ("Packing Slip has not been printed."));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		else
			sprintf (local_rec.StartInv,"%8.8s","        ");
		if (prog_status != ENTRY && strcmp (local_rec.StartInv,local_rec.EndInv) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_SUCCESS); 
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate pack Slip Number.	
	 */
	if (LCHECK ("e_pslip_no")) 
	{
		if (SRCH_KEY)
		{
			SrchCohrPS (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Check if order is on file. 
		 */
		strcpy (cohr_rec.co_no,comm_rec.co_no);
		strcpy (cohr_rec.br_no,comm_rec.est_no);
		strcpy (cohr_rec.type,"P");
		sprintf (cohr_rec.inv_no, local_rec.EndInv);

		if (dflt_used)
		{
			sprintf (local_rec.EndInv,"%8.8s","~~~~~~~~");
			memset ((char *)local_rec.trueInv,0xff,sizeof (local_rec.trueInv));
			return (EXIT_SUCCESS);
		}
		
		cc = find_rec ("cohr",&cohr_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlSoMess227));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		/*
		 * Check if packing slip printed, if not then have to ignore. 
		 */
		if (cohr_rec.ps_print [0] != 'Y')
		{
			errmess (ML ("Packing Slip has not been printed."));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (local_rec.StartInv,local_rec.EndInv) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.trueInv, local_rec.EndInv);
		return (EXIT_SUCCESS);
	}
    /*
     * Validate Start Date. 
     */
    if (LCHECK ("StartDate"))
    {
        if (dflt_used)
        {
            local_rec.StartDate = 0L;
            return (EXIT_SUCCESS);
        }
		if (local_rec.StartDate > StringToDate (local_rec.systemDate))
		{
			print_mess (ML (mlStdMess086));
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
		}
        if (prog_status != ENTRY &&
            local_rec.StartDate > local_rec.EndDate)
        {
            print_mess (ML (mlStdMess057));
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }
        return (EXIT_SUCCESS);
    }
    /*
	 * Validate End Date. 
     */
    if (LCHECK ("EndDate"))
    {
        if (dflt_used)
        {
            local_rec.EndDate = StringToDate (local_rec.systemDate);
			if (local_rec.StartDate > local_rec.EndDate)
			{
				print_mess (ML (mlStdMess058));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
        	}

            return (EXIT_SUCCESS);
        }
        if (local_rec.StartDate > local_rec.EndDate)
        {
			print_mess (ML (mlStdMess058));
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }
        return (EXIT_SUCCESS);
	}
    /*
	 * Validate start group 
	 */
	if (LCHECK ("StartSalesman"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, local_rec.StartSalesman);

		if (dflt_used)
		{
			sprintf (local_rec.StartSalesman,"%-2.2s","  ");
			sprintf (local_rec.DStartSalesman,"%-40.40s",ML ("START OF RANGE"));
			DSP_FLD ("DStartSalesman");
			return (EXIT_SUCCESS);
		}
		
		cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (prog_status != ENTRY && strcmp (local_rec.StartSalesman,local_rec.EndSalesman) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_SUCCESS); 
		}
		strcpy (local_rec.DStartSalesman, exsf_rec.salesman);
		DSP_FLD ("DStartSalesman");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate end group 
	 */
	if (LCHECK ("EndSalesman"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,local_rec.EndSalesman);
		
		if (dflt_used)
		{
			sprintf (local_rec.EndSalesman,"%-2.2s","~~");
			sprintf (local_rec.DEndSalesman,"%-40.40s",ML ("END OF RANGE"));
			memset ((char *)local_rec.trueSalesman,0xff,sizeof (local_rec.trueSalesman));
			DSP_FLD ("DEndSalesman");
			return (EXIT_SUCCESS);
		}
		cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (local_rec.StartSalesman,local_rec.EndSalesman) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.DEndSalesman,exsf_rec.salesman);
		strcpy (local_rec.trueSalesman, local_rec.EndSalesman);
		DSP_FLD ("DEndSalesman");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Start Zone. 
	 */
	if (LCHECK ("StartZone"))
	{
		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (trzm_rec.co_no,comm_rec.co_no);
		strcpy (trzm_rec.br_no,comm_rec.est_no);
		strcpy (trzm_rec.del_zone, local_rec.StartZone);

		if (dflt_used)
		{
			sprintf (local_rec.StartZone,"%-6.6s","  ");
			sprintf (local_rec.DStartZone,"%-40.40s",ML ("START OF RANGE"));
			DSP_FLD ("DStartZone");
			return (EXIT_SUCCESS);
		}

		cc = find_rec (trzm, &trzm_rec, COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (prog_status != ENTRY && strcmp (local_rec.StartZone,local_rec.EndZone) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_SUCCESS); 
		}
		sprintf (local_rec.DStartZone,"%-40.40s",trzm_rec.desc);
		DSP_FLD ("DStartZone");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate End Zone. 
	 */
	if (LCHECK ("EndZone"))
	{
		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (trzm_rec.co_no,comm_rec.co_no);
		strcpy (trzm_rec.br_no,comm_rec.est_no);
		strcpy (trzm_rec.del_zone, local_rec.EndZone);
		
		if (dflt_used)
		{
			sprintf (local_rec.EndZone,"%-6.6s","~~~~~~");
			sprintf (local_rec.DEndZone,"%-40.40s",ML ("END OF RANGE"));
			memset ((char *)local_rec.trueZone,0xff,sizeof (local_rec.trueZone));
			DSP_FLD ("DEndZone");
			return (EXIT_SUCCESS);
		}
		cc = find_rec (trzm,&trzm_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (local_rec.StartZone,local_rec.EndZone) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.DEndZone,trzm_rec.desc);
		DSP_FLD ("DEndZone");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
ProcessCohr (
 void)
{
	sprintf (dbt_str,	"%6.6s",	" ");
	sprintf (dbt_name,	"%40.40s",	" ");
	sprintf (order_str,	"%8.8s",	" ");
	sprintf (reason,	"%60.60s",	" ");

	if (!ProcessCustomer ())
		UpdatePackingSlip ();

	if (!audit_done)
		PrintAudit ();

}

int
ProcessCustomer (
 void)
{
	cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
	cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
	if (cc)
	{
		sprintf (reason,"Customer %06ld not on file",cohr_rec.hhcu_hash);
		return (cc);
	}
	strcpy (dbt_str,cumr_rec.dbt_no);
	strcpy (dbt_name,cumr_rec.dbt_name);
	strcpy (order_str,"        ");
	return (EXIT_SUCCESS);
}

/*
 * Validate various fields at line level	
 */
void
CheckSalesman (void)
{
	char	old_sman [3];
	char	new_sman [3];

	strcpy (old_sman,coln_rec.sman_code);
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,coln_rec.sman_code);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
	{
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,cohr_rec.sale_code);
		cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy (exsf_rec.co_no,comm_rec.co_no);
			strcpy (exsf_rec.salesman_no,cumr_rec.sman_code);
			cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
		}
		strcpy (new_sman, (cc) ? "99" : exsf_rec.salesman_no);
		strcpy (coln_rec.sman_code,new_sman);
		sprintf (reason,"Salesman %s not on file - updated to %s",old_sman,new_sman);
	}
}

/*
 * Update all files. 
 */
void
UpdatePackingSlip (void)
{
	int	ns_ok = FALSE;

	double	value = 0.00;
	double	wk_value = 0.00;
	char	dbt_day [3];
	char	dbt_mth [3];

	int		dmy [3];

	int		no_lines = FALSE;

	audit_done = FALSE;

	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		notax = 1;
	else
		notax = 0;

	cohr_rec.gross 	= 0.00;
	cohr_rec.disc 	= 0.00;
	cohr_rec.tax 	= 0.00;
	cohr_rec.gst 	= 0.00;

	coln_rec.hhco_hash = cohr_rec.hhco_hash;
	coln_rec.line_no = 0L;
	cc = find_rec (coln,&coln_rec,GTEQ,"u");
	if (cc || coln_rec.hhco_hash != cohr_rec.hhco_hash)
	{
		strcpy (reason,"Packing Slip has no lines");
		cohr_rec.gross = 0.00;
		cohr_rec.disc  = 0.00;
		cohr_rec.tax   = 0.00;
		cohr_rec.gst   = 0.00;
		no_lines = TRUE;
		abc_unlock (coln);
	}
	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		if (sohr_rec.sohr_new [0] == 'N' && envSoFreightBord)
			cohr_rec.freight = 0.00;

		inmr_rec.hhbr_hash 	=	coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (reason,"Item (Hash = %06ld) not on file",coln_rec.hhbr_hash);
			PrintAudit ();
			abc_unlock (coln);
			cc = find_rec (coln,&coln_rec,NEXT,"u");
			continue;
		}
		/*
		 * Find Original soln record		
		 */
		soln_rec.hhsl_hash	=	coln_rec.hhsl_hash;
		cc = find_rec (soln,&soln_rec,EQUAL,"u");
		if (cc)
		{
			abc_unlock (coln);
			cc = find_rec (coln,&coln_rec,NEXT,"u");
			continue;
		}

		sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
		cc = find_rec (sohr,&sohr_rec,EQUAL,"r");
		if (!cc)
		{
			strcpy (dbt_str,cumr_rec.dbt_no);
			strcpy (dbt_name,cumr_rec.dbt_name);
			strcpy (order_str,sohr_rec.order_no);
		}
		if (SERIAL_ITEM && !strcmp (coln_rec.serial_no,"                         "))
		{
			abc_unlock (coln);
			sprintf (reason,"Item %s requires Valid Serial Number",inmr_rec.item_no);
			return;
		}

		CheckSalesman ();

		/*
		 * Non Stock Item, But Item it belongs to has been deleted.
		 */
		if (NON_STOCK && !ns_ok)
		{
			if (!envSoRtDelete)
			{
				soln_rec.qty_order = 0.00;
				soln_rec.qty_bord  = 0.00;
				strcpy (soln_rec.status, "D");
				strcpy (soln_rec.stat_flag, "D");
				cc = abc_update (soln, &soln_rec);
				if (cc)
					file_err (cc,soln,"DBUPDATE");
			}
			else
			{
				cc = ArchiveSoln (soln_rec.hhsl_hash);
				if (cc)
					file_err (cc,soln,"ARCHIVE");

				cc = abc_delete (soln);
				if (cc)
					file_err (cc,soln,"DBDELETE");
			}
			abc_unlock (coln);

			/*
			 * Check if header needs to be deleted/updated. 
			 */
			DeleteSohr (soln_rec.hhso_hash);

			cc = find_rec (coln,&coln_rec,NEXT,"u");
			continue;
		}
		else
		{
			/*
			 * Item is non stock and parent placed on B/O. 
			 */
			if (NON_STOCK)
			{
				strcpy (soln_rec.stat_flag,	"B");
				strcpy (soln_rec.status,	"B");
				cc = abc_update (soln,&soln_rec);
				if (cc)
				{
					strcpy (reason,"Error in updating soln for a non-stock item");
					return;
				}
				abc_unlock (soln);
			}
			ns_ok = TRUE;
		}
		/*
		 * Packing slip from so_pscreat + so_ps_cons are now created with 	   
		 * status stat flag of "P". Basically if status is not 'P' then 	
		 * something else has updated is leave as is to prevent double posting
		 */
		if (coln_rec.stat_flag [0] == 'P')
		{
			strcpy (coln_rec.stat_flag,	createStatusFlag);
			strcpy (coln_rec.status,	createStatusFlag);
		}
		if (notax)
		{
			coln_rec.tax_pc = 0.00;
			coln_rec.gst_pc = 0.00;
		}

		/*
		 * Perform Invoice Calculations	
		 */
		lineTotal = coln_rec.q_order * 
					  out_cost (coln_rec.sale_price, inmr_rec.outer_size);

		lineTotal = no_dec (lineTotal);

		taxTotal = coln_rec.q_order * 
					out_cost (inmr_rec.tax_amount, inmr_rec.outer_size);

		taxTotal = no_dec (taxTotal);

		lineDiscount = (double) (coln_rec.disc_pc / 100.00);
		lineDiscount *= lineTotal;
		lineDiscount = no_dec (lineDiscount);

		if (notax)
			lineTax = 0.00;
		else
		{
			lineTax = (double) inmr_rec.tax_pc;
			if (cumr_rec.tax_code [0] == 'D')
				lineTax *= taxTotal;
			else
			{
				if (envDbNettUsed)
					lineTax *= (lineTotal - lineDiscount);
				else
					lineTax *= lineTotal;
			}
		
			lineTax = DOLLARS (lineTax);
			lineTax = no_dec (lineTax);
		}
		if (notax)
			lineGst = 0.00;
		else
		{
			lineGst = (double) inmr_rec.gst_pc;
			if (envDbNettUsed)
				lineGst *= ((lineTotal - lineDiscount) + lineTax);
			else
				lineGst *= (lineTotal + lineTax);

			lineGst = DOLLARS (lineGst);
		}

		coln_rec.gross    = lineTotal;
		coln_rec.amt_disc = lineDiscount;
		coln_rec.amt_tax  = lineTax;
		coln_rec.amt_gst  = lineGst;

		cohr_rec.gross += lineTotal;
		cohr_rec.disc  += lineDiscount;
		cohr_rec.tax   += lineTax;
		cohr_rec.gst   += lineGst;

		cc = abc_update (coln,&coln_rec);
		if (cc)
		{
			strcpy (reason,"Error in updating coln ");
			return;
		}
		abc_unlock (coln);

		if (!NON_STOCK)
		{
			/*
			 * backorder qty = 0 so delete soln	
			 */
			if (coln_rec.q_backorder == 0.00)
			{
				if (!envSoRtDelete)
				{
					soln_rec.qty_order = 0.00;
					soln_rec.qty_bord  = 0.00;
					strcpy (soln_rec.status, "D");
					strcpy (soln_rec.stat_flag, "D");
					cc = abc_update (soln, &soln_rec);
					if (cc)
						file_err (cc,soln,"DBUPDATE");
				}
				else
				{
					cc = ArchiveSoln (soln_rec.hhsl_hash);
					if (cc)
						file_err (cc,soln,"ARCHIVE");

					cc = abc_delete (soln);
					if (cc)
						file_err (cc,soln,"DBUPDATE");
				}
				ns_ok = FALSE;
			}
			else
			{
				strcpy (soln_rec.status,	"B");
				strcpy (soln_rec.stat_flag,	"B");
				soln_rec.qty_bord	= coln_rec.q_backorder;
				soln_rec.qty_order	= 0.00;
				cc = abc_update (soln,&soln_rec);
				if (cc)
				{
					strcpy (reason,"Error in updating soln");
					return;
				}
				abc_unlock (soln);
				ns_ok = TRUE;
			}
		}
		/*
		 * Check if header needs to be deleted/updated. 
		 */
		DeleteSohr (soln_rec.hhso_hash);

		abc_unlock (coln);
		cc = find_rec (coln,&coln_rec,NEXT,"u");
	}
	if (notax)
		wk_value = 0.00;
	else
		wk_value = (double) (comm_rec.gst_rate / 100.00);



	value = cohr_rec.freight + 
			cohr_rec.insurance -
			cohr_rec.ex_disc + 
			cohr_rec.other_cost_1 +
			cohr_rec.other_cost_2 + 
			cohr_rec.other_cost_3 +
			cohr_rec.sos;

	wk_value *= value;
	wk_value = no_dec (wk_value);

	cohr_rec.gst += wk_value;
	cohr_rec.gst = no_dec (cohr_rec.gst);
	
	/*
	 * Set the invoice batch no	
	 */
	DateToDMY (comm_rec.dbt_date, &dmy [0],&dmy [1],&dmy [2]);
	sprintf (dbt_day,"%02d", dmy [0]);
	sprintf (dbt_mth,"%02d", dmy [1]);

	sprintf (cohr_rec.batch_no,"A%s%s",dbt_day,dbt_mth);

	/*
	 * Packing slip/invoice combined so invoice date must 
	 * be what was on printed Doco.                      
	 */
	if (envSoDoi)
	{
		local_rec.inv_date = (envCombInvPack) ? cohr_rec.date_raised	
											  : StringToDate (local_rec.systemDate);
	}
	else
	{
		local_rec.inv_date = (envCombInvPack) ? cohr_rec.date_raised	
											  : comm_rec.dbt_date;
	}

	cohr_rec.date_raised 	= local_rec.inv_date;
	cohr_rec.date_required 	= des_date;

	strcpy (cohr_rec.tax_code,sohr_rec.tax_code);
	strcpy (cohr_rec.tax_no,cumr_rec.tax_no);

	if (findStatusFlag [0] == 'I' && cohr_rec.type [0] == 'P')
		strcpy (cohr_rec.type,"I");

	if (findStatusFlag [0] == 'T' && cohr_rec.type [0] == 'P')
		strcpy (cohr_rec.type,"T");

	/*
	 * Packing slip from so_pscreat + so_ps_cons are now created with status 
	 * stat flag of "P". Basically if status is not 'P' then something else 
	 * has updated is leave as is to prevent double posting.               
	 */
	if (cohr_rec.stat_flag [0] == 'P')
	{
		strcpy (cohr_rec.stat_flag,	createStatusFlag);
		strcpy (cohr_rec.status,	createStatusFlag);
	}
	strcpy (cohr_rec.inv_print, (envCombInvPack) ? "Y" : "N");

	cc = abc_update (cohr,&cohr_rec);
	if (cc)
	{
		strcpy (reason,"Error in updating cohr");
		return;
	}

	abc_unlock (cohr);

	/*
	 * Create a log file record for sales Order. 
	 */
	LogCustService 
	(
		cohr_rec.hhco_hash,
		cohr_rec.hhso_hash,
		cumr_rec.hhcu_hash,
		cohr_rec.cus_ord_ref,
		cohr_rec.cons_no,
		cohr_rec.carr_code,
		cohr_rec.del_zone,
		LOG_DISPATCH
	);
	if (!TRANSPORT)
	{
		/*
		 * Create a log file record for sales Order. 
		 */
		LogCustService 
		(
			cohr_rec.hhco_hash,
			cohr_rec.hhso_hash,
			cumr_rec.hhcu_hash,
			cohr_rec.cus_ord_ref,
			cohr_rec.cons_no,
			cohr_rec.carr_code,
			cohr_rec.del_zone,
			LOG_DELIVERY
		);
	}
	add_hash
	 (	
		comm_rec.co_no,
		comm_rec.est_no,
		"RO",
		0,	
		cohr_rec.hhcu_hash,
		0L,
		0L,
		 (double) 0.00
	);

	if (AUTO_SK_UP && !no_lines)
	{
		add_hash
		 (
			comm_rec.co_no,
			comm_rec.est_no,
			"SU",
			0,
			cohr_rec.hhco_hash,
			0L,
			0L,
			 (double) 0.00
		);
	}
	recalc_sobg ();

	/*
	 * If P/Slip has been updated correctly,print "Confirmed" on Audit
	 */
	strcpy (reason,"Complete Packing Slip Confirmed without Error.");
	return;
}

/*
 * Delete or update sohr record. 
 */
void
DeleteSohr (
	long	hhsoHash)
{
	int		lines_found;

	sohr_rec.hhso_hash	=	hhsoHash;
	if (find_rec (sohr, &sohr_rec, COMPARISON, "u"))
	{
		abc_unlock (sohr);
		return;
	}

	/*
	 * Check if sohr needs to be deleted. ie no soln records remaining for sohr
	 */
	if (!envSoRtDelete)
	{
		lines_found = 0;

		soln2_rec.hhso_hash = hhsoHash;
		soln2_rec.line_no 	= 0;
		cc = find_rec (soln2, &soln2_rec, GTEQ, "r");
		while (!cc && soln2_rec.hhso_hash == hhsoHash)
		{
			if (soln2_rec.status [0] != 'D')
				lines_found++;

			cc = find_rec (soln2, &soln2_rec, NEXT, "r");
		}
		if (!lines_found)
		{
			strcpy (sohr_rec.stat_flag, "D");
			strcpy (sohr_rec.status, "D");
			strcpy (sohr_rec.sohr_new,"N");
			cc = abc_update (sohr, &sohr_rec);
			if (cc)
				file_err (cc, "sohr", "DBUPDATE");

			return;
		}
	}
	else
	{
		soln2_rec.hhso_hash = hhsoHash;
		soln2_rec.line_no = 0;
		cc = find_rec (soln2,&soln2_rec,GTEQ,"r");
		if (cc || soln2_rec.hhso_hash != hhsoHash)
		{
			cc = ArchiveSohr (soln_rec.hhso_hash);
			if (cc)
				strcpy (reason,"Error in Archive sohr");

			cc = abc_delete (sohr);
			if (cc) 
			{
				strcpy (reason,"Error in deleting sohr");
				return;
			}
		}
	}
	/*
	 * Check if status and invoice number/packing slip number
	 * is the same as order may be on multiple packing slips.
	 */
	if (!strcmp (sohr_rec.inv_no, cohr_rec.inv_no) && 
		  sohr_rec.status [0] == 'P')
	{
		strcpy (sohr_rec.inv_no,"        ");
		strcpy (sohr_rec.stat_flag,	"B");
		strcpy (sohr_rec.status,	"B");
	}
	strcpy (sohr_rec.sohr_new,"N");
	cc = abc_update (sohr,&sohr_rec);
	if (cc)
	{
		strcpy (reason,"Error in updating sohr");
		return;
	}
	abc_unlock (sohr);

	return;
}


void
PrintAudit (
 void)
{
	if (firstTime)
		HeadingOutput ();

	fprintf (pout, "|  %-6.6s  ",dbt_str);
	fprintf (pout, "| %-40.40s ",dbt_name);
	fprintf (pout, "| %-8.8s ",cohr_rec.inv_no);
	fprintf (pout, "| %-8.8s ",order_str);
	fprintf (pout, "|   %-60.60s    |\n",reason);
	
	/*
	 * Reset audit lines	
	 */
	sprintf (dbt_str,	"%6.6s",	" ");
	sprintf (dbt_name,	"%40.40s",	" ");
	sprintf (order_str,	"%8.8s",	" ");
	sprintf (reason,	"%60.60s",	" ");

	audit_done = TRUE;
}

void
HeadingOutput (void)
{
	char	DateFrom [11],
			DateTo [11];

	/*
	 * Start Out Put To Standard Print 
	 */

	strcpy (DateFrom, DateToString (local_rec.StartDate));
	strcpy (DateTo, DateToString (local_rec.EndDate));

	if ((pout = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*
	 * Start output to file. 
	 */
	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pout, ".LP%d\n",printerNumber);
	fprintf (pout, ".14\n");
	fprintf (pout, ".PI12\n");
	fprintf (pout, ".L151\n");
	fprintf (pout, ".B1\n");
	if (findStatusFlag [0] == 'T')
		fprintf (pout, ".ETRANSPORT DESPATCH CONFIRMATION AUDIT\n");
	else
		fprintf (pout, ".EDESPATCH CONFIRMATION AUDIT\n");
	fprintf (pout, ".E%s \n",clip (comm_rec.co_name));
	fprintf (pout, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (pout, ".B1\n");
	fprintf (pout, ".EAS AT : %s\n",SystemTime ());
	fprintf (pout, ".C P/SLIP FROM (%s) TO (%s)  / DATE FROM (%s) TO (%s)\n",
						local_rec.StartInv, local_rec.EndInv,DateFrom,DateTo);

	fprintf (pout, ".C SALESMAN FROM (%s) TO (%s) / ZONE FROM (%s) TO (%s)\n",
						local_rec.StartSalesman, local_rec.EndSalesman,
						local_rec.StartZone, local_rec.EndZone);

	fprintf (pout, ".R==================================================");
	fprintf (pout, "==================================================");
	fprintf (pout, "=============================================\n");
	fprintf (pout, "==================================================");
	fprintf (pout, "==================================================");
	fprintf (pout, "=============================================\n");

	fprintf (pout, "|CUSTOMER #");
	fprintf (pout, "|  CUSTOMER NAME                           ");
	fprintf (pout, "|  P/SLIP# ");
	fprintf (pout, "|  ORDER#  ");
	fprintf (pout, "|  DESCRIPTION                                                      |\n");
	fprintf (pout, "|----------");
	fprintf (pout, "|------------------------------------------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|-------------------------------------------------------------------|\n");
	firstTime = FALSE;

	PipeOpen = TRUE;
}


/*
 * Allow tagging of all packing slips within selected range. 
 */
void
ProcPackingSlips (void)
{
	char	deliveryDate [11],
			raisedDate [11];

	nuumberTabLines = 0;
	/*
	 * Open tab display and print heading line. i.e the one with the '#' 
	 */
	tab_open ("PSlipTagFile", ind_keys, 9, 0, 7, FALSE);
	tab_add
	 (
		"PSlipTagFile", 
		"# Customer No. |               Customer Name            | Delivery Zone | Salesman No | P/Slip No | Required Date | Delivery Date "
	);

	/*
	 * Read all packing slips within selected range. 
	 */
	abc_selfield (cohr, "cohr_id_no2");

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type,  "P");
	strcpy (cohr_rec.inv_no, local_rec.StartInv);
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");

	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (cohr_rec.br_no, comm_rec.est_no) &&
				  cohr_rec.type [0] == 'P' &&
				  strcmp (cohr_rec.inv_no, local_rec.trueInv) <= 0)
	{
		/*
		 * Check if packing slip printed, if not then have to ignore. 
		 */
		if (cohr_rec.ps_print [0] != 'Y')
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Read customer. 
		 */
		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cumr, "DBFIND");

		if (!strcmp (cohr_rec.del_zone, "      "))
			strcpy (cohr_rec.del_zone, cumr_rec.del_zone);

		/*
		 * Check salesman start and end range. 
		 */
		if (strcmp (cohr_rec.sale_code, local_rec.StartSalesman) < 0 ||
			strcmp (cohr_rec.sale_code, local_rec.trueSalesman) > 0)
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Check area start and end range. 
		 */
		if (strcmp (cohr_rec.del_zone, local_rec.StartZone) < 0 ||
			strcmp (cohr_rec.del_zone, local_rec.trueZone) > 0)
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Check date start and end range. 
		 */
		if (cohr_rec.del_date > 0L && TRANSPORT)
		{
			if (cohr_rec.del_date < local_rec.StartDate ||
		    	cohr_rec.del_date > local_rec.EndDate)
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;
			}
		}
		else
		{
			if (cohr_rec.date_raised < local_rec.StartDate ||
		    	cohr_rec.date_raised > local_rec.EndDate)
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;
			}
		}
		if (TRANSPORT && cohr_rec.del_req [0] != 'Y')
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}

		if (cohr_rec.del_date > 0L)
			sprintf (deliveryDate, "%10.10s", DateToString (cohr_rec.del_date));
		else
			strcpy (deliveryDate, " No Date. ");

		if (cohr_rec.date_raised > 0L)
			sprintf (raisedDate, "%10.10s", DateToString (cohr_rec.date_raised));
		else
			strcpy (raisedDate, "          ");

		/*
		 * Write record to tab screen. 
		 */
		cc = tab_add
		 (
			"PSlipTagFile", 
			"    %6.6s    |%40.40s|    %6.6s     |      %2.2s     | %8.8s  |  %10.10s   |  %10.10s   |  %010ld",
			cumr_rec.dbt_no,
			cumr_rec.dbt_name,
			cohr_rec.del_zone,
			cohr_rec.sale_code,
			cohr_rec.inv_no,
			raisedDate,
			deliveryDate,
			cohr_rec.hhco_hash
		);
		
		if (cc)
			break;

		nuumberTabLines++;

		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}

	if (nuumberTabLines > 0)
		tab_scan ("PSlipTagFile");
	else
	{
		/*
		 * No packing slips found. 
		 */
		tab_add ("PSlipTagFile", "  ************  NO VALID LINES CAN BE LOADED  ************");
		tab_display ("PSlipTagFile", TRUE);
		putchar (BELL);
		fflush (stdout);
		sleep (sleepTime);
		tab_close ("PSlipTagFile", TRUE);
		return;
	}

	if (!prog_exit)
		ProcessSelected ();

	tab_close ("PSlipTagFile", TRUE);
}       


/*
 * updates details 
 */
void
ProcessSelected (void)
{
	int	i;

	abc_selfield (cohr, "cohr_hhco_hash");

	/*
	 * Process all tagged lines 
	 */
	for (i = 0; i < nuumberTabLines; i++)
	{
		tab_get ("PSlipTagFile", get_buf, EQUAL, i);
	   	if (!tagged (get_buf))
			continue;
		
		redraw_line ("PSlipTagFile", TRUE);
		tag_unset ("PSlipTagFile");

		/*
		 * Find cuhd record. 
		 */
		cohr_rec.hhco_hash	= atol (get_buf + 132);
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "cohr", "DBFIND");
		
		ProcessCohr ();
		redraw_line ("PSlipTagFile", FALSE);
	}
	return;
}

/*
 * Search for p_slip number 
 */
void
SrchCohrPS  (
 char	*keyValue)
{
	work_open ();
	strcpy (cohr_rec.co_no,comm_rec.co_no);
	strcpy (cohr_rec.br_no,comm_rec.est_no);
	sprintf (cohr_rec.inv_no,"%-8.8s",keyValue);
	strcpy (cohr_rec.type,"P");

	save_rec ("#P/Slip","#Customer Name ");
	cc = find_rec (cohr,&cohr_rec,GTEQ,"r");
	while (!cc && !strncmp (cohr_rec.inv_no,keyValue,strlen (keyValue)) && 
				  !strcmp (cohr_rec.co_no,comm_rec.co_no) && 
				  !strcmp (cohr_rec.br_no,comm_rec.est_no) &&
				  cohr_rec.type [0] == 'P')
	{
		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cumr, "DBFIND");

		/*
		 * Check if packing slip printed, if not then have to ignore. 
		 */
		if (cohr_rec.ps_print [0] != 'Y')
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (cohr_rec.inv_no,cumr_rec.dbt_no);
		if (cc)
			break;
		
		cc = find_rec (cohr,&cohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cohr_rec.co_no,comm_rec.co_no);
	strcpy (cohr_rec.br_no,comm_rec.est_no);
	sprintf (cohr_rec.inv_no,"%-8.8s",temp_str);
	strcpy (cohr_rec.type,"P");
	cc = find_rec (cohr,&cohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "cohr", "DBFIND");
}
/*
 * Search for salesman. 
 */
void
SrchExsf (
 char	*keyValue)
{
	work_open ();
	save_rec ("#Sm","#Salesman.");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",keyValue);
	cc = find_rec (exsf,&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exsf_rec.salesman_no,keyValue,strlen (keyValue)))
	{
		cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec (exsf,&exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}


/*
 * Search for Zome Master. 
 */
void
SrchTrzm (
	char *keyValue)
{
	work_open();
	save_rec("#Zone. ","#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", keyValue);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trzm_rec.br_no, comm_rec.est_no) &&
				  !strncmp (trzm_rec.del_zone, keyValue, strlen (keyValue)))
	{
		cc = save_rec (trzm_rec.del_zone, trzm_rec.desc);
		if (cc)
			break;

		cc = find_rec (trzm, &trzm_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trzm, "DBFIND");

	return;
}

static	int 
tag_func (
 int c, 
 KEY_TAB *psUnused)
{
	if (c == 'T')
		tag_toggle ("PSlipTagFile");
	else
		tag_all ("PSlipTagFile");

	return (c);
}

static	int 
abort_func (
 int c, 
 KEY_TAB *psUnused)
{
	prog_exit = TRUE;

	return (c);
}

/*
 * check the existence of the environment variable first with company & branch,
 * then company, and then by itself. If no vble found then return 'prg'
 */
char *
CheckVariable (
	char*	environmentName,
	char*	programName)
{
	return
	(
		_CheckVariable
		(
			environmentName,
			programName,
			comm_rec.co_no,
			comm_rec.est_no
		)
	);
}

char *
_CheckVariable (
	char*  environmentName,
	char*  programName,
	char*  coNo,
	char*  brNo)
{
	char	*sptr;
	char	runPrintProg [41];

	/*
	 * Check Company & Branch	
	 */
	sprintf  (runPrintProg ,"%s%s%s",environmentName,coNo,brNo);
	sptr = chk_env (runPrintProg );
	if (sptr == (char *)0)
	{
		/*
		 * Check Company	
		 */
		sprintf (runPrintProg ,"%s%s",environmentName,coNo);
		sptr = chk_env (runPrintProg );
		if (sptr == (char *)0)
		{
			sprintf (runPrintProg ,"%s",environmentName);
			sptr = chk_env (runPrintProg );
			return ((sptr == (char *)0) ? programName : sptr);
		}
		else
			return (sptr);
	}
	else
		return (sptr);
}
/*
 * Program heading function. 
 */
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	line_at (1,0,132);

	box (0,2,132,6);

	line_at (20,0,132);

	if (findStatusFlag [0] == 'T')
		rv_pr (ML (mlSoMess128),40,0,1);
	else
		rv_pr (ML (mlSoMess129),45,0,1);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_short);

	/*  reset this variable for _new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
