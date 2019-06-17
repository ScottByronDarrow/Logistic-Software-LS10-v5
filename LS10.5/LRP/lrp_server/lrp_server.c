/*===========================================================================
| Copyright 1999, Logistic Limited. All rights reserved                      |
| ---------------------------------------------------------------------------|
| $Id: lrp_server.c,v 5.2 2001/08/09 09:30:00 scott Exp $
| Program name    : lrp_server.c                                             |
| Description     : Stock forecsating server program                         |
|----------------------------------------------------------------------------|
| Author          : Primo O. Esteria      : Date  June 25, 1999         |
|----------------------------------------------------------------------------|
| Modifications
| $Log: lrp_server.c,v $
| Revision 5.2  2001/08/09 09:30:00  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:47  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:40  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:50  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/01/30 05:48:54  scott
| Updated to add app.schema
|
| Revision 3.1  2000/12/14 08:28:56  scott
| Updated to remove unused fields.
|
| Revision 3.0  2000/10/10 12:15:40  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:49  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.11  2000/06/26 03:45:09  scott
| Updated to remove unused fields.
|
| Revision 1.10  2000/02/18 01:42:16  scott
| Updated for small warning error when compile under Linux
|
| Revision 1.9  1999/12/06 01:34:21  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/25 10:23:57  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.7  1999/10/27 07:33:03  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.6  1999/09/29 10:10:52  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 07:26:43  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/08/25 02:22:05  primo
| Routine update
|
| Revision 1.3  1999/07/01 06:36:59  primo
| Update
|
| Revision 1.2  1999/06/25 02:45:02  primo
| Initial update
|
============================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_server.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_server/lrp_server.c,v 5.2 2001/08/09 09:30:00 scott Exp $";

#include <pslscr.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*=======================
| messages identifier    |
=========================*/
#define LRP_INMR		1
#define LRP_INUM		2
#define LRP_INTR		3
#define LRP_CCMR		4
#define LRP_ESMR		5
#define LRP_EXCF		6
#define LRP_INLS		7
#define LRP_FFDM		8
#define LRP_START		10
#define LRP_MODEL		11
#define LRP_PARAMETERS	12
#define LRP_OVERRIDES	13
#define LRP_RESULTS		14
#define LRP_ERROR		99
#define LRP_QUIT		999


/*==========================================
| maximum number of concurrent connections  |
============================================*/
#define MAX_CONNECTION	100

/*================================
| Maximum send/recv buffer width  |
 ================================*/
#define MAX_BUF			9216

	/*====================================+
	 | Inventory Master File Base Record. |
	 +====================================*/
#define	INMR_NO_FIELDS	8

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_std_uom"},
		{"inmr_stat_flag"}
	};

	struct tag_inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		char	class [2];
		char	description [41];
		char	category [12];
		long	std_uom;
		char	stat_flag [2];
	}	inmr_rec;


	/*=================================+
	 | Inventory Unit of Measure File. |
	 +=================================*/
#define	INUM_NO_FIELDS	5

	struct dbview	inum_list [INUM_NO_FIELDS] =
	{
		{"inum_uom_group"},
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_desc"},
		{"inum_cnv_fct"}
	};

	struct tag_inumRecord
	{
		char	uom_group [21];
		long	hhum_hash;
		char	uom [5];
		char	desc [41];
		float	cnv_fct;
	}	inum_rec;


	/*==============================+
	 | Inventory Transactions File. |
	 +==============================*/
#define	INTR_NO_FIELDS	11

	struct dbview	intr_list [INTR_NO_FIELDS] =
	{
		{"intr_co_no"},
		{"intr_br_no"},
		{"intr_hhbr_hash"},
		{"intr_hhcc_hash"},
		{"intr_hhum_hash"},
		{"intr_type"},
		{"intr_date"},
		{"intr_qty"},
		{"intr_cost_price"},
		{"intr_sale_price"},
		{"intr_stat_flag"}
	};

	struct tag_intrRecord
	{
		char	co_no [3];
		char	br_no [3];
		long	hhbr_hash;
		long	hhcc_hash;
		long	hhum_hash;
		int		type;
		Date	date;
		float	qty;
		Money	cost_price;
		Money	sale_price;
		char	stat_flag [2];
	}	intr_rec;


	/*==========================================+
	 | Establishment/Branch Master File Record. |
	 +==========================================*/
#define	ESMR_NO_FIELDS	47

	struct dbview	esmr_list [ESMR_NO_FIELDS] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
		{"esmr_adr1"},
		{"esmr_adr2"},
		{"esmr_adr3"},
		{"esmr_area_code"},
		{"esmr_dbt_date"},
		{"esmr_crd_date"},
		{"esmr_inv_date"},
		{"esmr_pay_date"},
		{"esmr_gl_date"},
		{"esmr_stmt_date"},
		{"esmr_date_stmt_prn"},
		{"esmr_status_flags"},
		{"esmr_dflt_bank"},
		{"esmr_chg_pref"},
		{"esmr_csh_pref"},
		{"esmr_crd_pref"},
		{"esmr_man_pref"},
		{"esmr_nx_sav_inv"},
		{"esmr_nx_csh_inv"},
		{"esmr_nx_csh_crd"},
		{"esmr_nx_inv_no"},
		{"esmr_nx_man_no"},
		{"esmr_nx_request_no"},
		{"esmr_nx_slip_no"},
		{"esmr_nx_crd_nte_no"},
		{"esmr_nx_ccn_no"},
		{"esmr_sales_acc"},
		{"esmr_nx_pur_ord_no"},
		{"esmr_nx_pur_fgn"},
		{"esmr_nx_pur_crd_no"},
		{"esmr_nx_gr_no"},
		{"esmr_nx_pack_no"},
		{"esmr_nx_del_dck_no"},
		{"esmr_nx_order_no"},
		{"esmr_nx_requis_no"},
		{"esmr_nx_cheq_no"},
		{"esmr_ls_cheq_no"},
		{"esmr_nx_job_no"},
		{"esmr_nx_dd_order"},
		{"esmr_nx_csh_trn_no"},
		{"esmr_nx_voucher_no"},
		{"esmr_online"},
		{"esmr_stat_flag"}
	};

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	est_name [41];
		char	short_name [16];
		char	adr1 [41];
		char	adr2 [41];
		char	adr3 [41];
		char	area_code [3];
		Date	dbt_date;
		Date	crd_date;
		Date	inv_date;
		Date	pay_date;
		Date	gl_date;
		Date	stmt_date;
		Date	date_stmt_prn;
		char	status_flags [6];
		char	dflt_bank [6];
		char	chg_pref [3];
		char	csh_pref [3];
		char	crd_pref [3];
		char	man_pref [3];
		long	nx_sav_inv;
		long	nx_csh_inv;
		long	nx_csh_crd;
		long	nx_inv_no;
		long	nx_man_no;
		long	nx_request_no;
		long	nx_slip_no;
		long	nx_crd_nte_no;
		long	nx_ccn_no;
		char	sales_acc [7];
		long	nx_pur_ord_no;
		long	nx_pur_fgn;
		long	nx_pur_crd_no;
		long	nx_gr_no;
		long	nx_pack_no;
		long	nx_del_dck_no;
		long	nx_order_no;
		long	nx_requis_no;
		long	nx_cheq_no;
		long	ls_cheq_no;
		long	nx_job_no;
		long	nx_dd_order;
		long	nx_csh_trn_no;
		long	nx_voucher_no;
		int		online;
		char	stat_flag [2];
	}	esmr_rec;


	/*===========================================+
	 | Cost Centre/Warehouse Master File Record. |
	 +===========================================*/
#define	CCMR_NO_FIELDS	23

	struct dbview	ccmr_list [CCMR_NO_FIELDS] =
	{
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_hhlo_hash"},
		{"ccmr_master_wh"},
		{"ccmr_only_loc"},
		{"ccmr_sman_no"},
		{"ccmr_name"},
		{"ccmr_acronym"},
		{"ccmr_whse_add1"},
		{"ccmr_whse_add2"},
		{"ccmr_whse_add3"},
		{"ccmr_whse_add4"},
		{"ccmr_type"},
		{"ccmr_sal_ok"},
		{"ccmr_pur_ok"},
		{"ccmr_issues_ok"},
		{"ccmr_receipts"},
		{"ccmr_reports_ok"},
		{"ccmr_lpno"},
		{"ccmr_nx_wo_num"},
		{"ccmr_stat_flag"}
	};

	struct tag_ccmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		long	hhcc_hash;
		long	hhlo_hash;
		char	master_wh [2];
		char	only_loc [11];
		char	sman_no [3];
		char	name [41];
		char	acronym [10];
		char	whse_add1 [41];
		char	whse_add2 [41];
		char	whse_add3 [41];
		char	whse_add4 [41];
		char	type [3];
		char	sal_ok [2];
		char	pur_ok [2];
		char	issues_ok [2];
		char	receipts [2];
		char	reports_ok [2];
		int		lpno;
		long	nx_wo_num;
		char	stat_flag [2];
	}	ccmr_rec;


	/*============================+
	 | Inventory Lost Sales File. |
	 +============================*/
#define	INLS_NO_FIELDS	14

	struct dbview	inls_list [INLS_NO_FIELDS] =
	{
		{"inls_co_no"},
		{"inls_est_no"},
		{"inls_date"},
		{"inls_hhbr_hash"},
		{"inls_hhcc_hash"},
		{"inls_hhcu_hash"},
		{"inls_area_code"},
		{"inls_sale_code"},
		{"inls_qty"},
		{"inls_value"},
		{"inls_cost"},
		{"inls_res_code"},
		{"inls_res_desc"},
		{"inls_status"}
	};

	struct tag_inlsRecord
	{
		char	co_no [3];
		char	est_no [3];
		Date	date;
		long	hhbr_hash;
		long	hhcc_hash;
		long	hhcu_hash;
		char	area_code [3];
		char	sale_code [3];
		float	qty;
		Money	value;
		Money	cost;
		char	res_code [3];
		char	res_desc [61];
		char	status [2];
	}	inls_rec;


	/*================================+
	 | External Category File Record. |
	 +================================*/
#define	EXCF_NO_FIELDS	17

	struct dbview	excf_list [EXCF_NO_FIELDS] =
	{
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_hhcf_hash"},
		{"excf_ex_rate"},
		{"excf_cat_desc"},
		{"excf_max_disc"},
		{"excf_min_marg"},
		{"excf_ol_min_marg"},
		{"excf_ol_max_marg"},
		{"excf_gp_mkup"},
		{"excf_item_alloc"},
		{"excf_no_trans"},
		{"excf_no_days"},
		{"excf_review_prd"},
		{"excf_cont_drugs"},
		{"excf_ib_marg"},
		{"excf_stat_flag"}
	};

	struct tag_excfRecord
	{
		char	co_no [3];
		char	cat_no [12];
		long	hhcf_hash;
		float	ex_rate;
		char	cat_desc [41];
		float	max_disc;
		float	min_marg;
		float	ol_min_marg;
		float	ol_max_marg;
		float	gp_mkup;
		char	item_alloc [2];
		int		no_trans;
		int		no_days;
		float	review_prd;
		char	cont_drugs [2];
		float	ib_marg;
		char	stat_flag [2];
	}	excf_rec;

/*=========================
| database and table names |
===========================*/
char *data = "data",
	 *inmr = "inmr",
	 *excf = "excf",
	 *inls = "inls",
	 *inum = "inum",
	 *intr = "intr",
	 *ccmr = "ccmr",
	 *esmr = "esmr";

/*===============================
| Number of current connections  |
================================*/
int currentConnections = 0;

/*============================
| Transmit/receive buffer     |
=============================*/
char 					buffer [MAX_BUF];

/*==========================
| Socket specific variable  |
===========================*/
int                     transmitSocket;
struct sockaddr_in      transmitAddr;
struct sockaddr_in      drlAddr;
int                     drlSocket;

/*=================
| Termination flag |
==================*/
int                     terminated =0;

/*=====================================================
| Length of incoming data, from GetMessage/SendMessage |
======================================================*/
int                     msgLen;

/*=================
| signal variables |
 =================*/
struct                  sigaction newsa, oldsa;

/*=======================
| Log file stream handle |
========================*/
FILE 					*fp_log;

/*========================================================================
| SETUP = 1 if the system work on existing inmr_class/inmr_category setup |
|         2 if system is on other group setup.                            |
=========================================================================*/
int					    SETUP;	

/*=================================================
| CO, BR, WH  comes in every data sent from clients |
 ==================================================*/
char					CO [3];
char					BR [3];
char					WH [3];

/*==========================
| Main Processing Routine. |
==========================*/
int main ( int	argc, char	*argv []);

/* system routines */
int 	run (void);
int 	daemonize (void);
void 	setsignals (void);
void 	endprocess (int);
int		resolveservice (char *, char *);
int		processclient (void);
int		GetMessage (void);
int		getsetup (void);
void	GetData (void);
int		SendMessage (int, int);
void	SendData (void);
void	ChildLoop (void);
void	EndMessage (void);
void	SetConnections (int);

/* application routines */
void 	OpenDB (void);
void	CloseDB (void);
void	GetINMR (void);
void 	GetINUM (void);
void	GetCCMR (void);
void	GetESMR (void);
void	GetINTR (void);
void	GetEXCF (void);
void	GetInls (long, long, long);
char	*getservice (void);
void	GetGroup (char *);
void	GetSubgroup (char *);
void 	GetItem (char *);
void	GetAllItems (void);
long	GetIncc (void);


/*================== 
| utility routines  |
 ==================*/
void	sendthisinmr (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	setsignals ();
	
	/*
	daemonize ();
	*/

	/*-------------------------------------------------\
	| Check ../BIN/LRP/lrp_server/setup/lrp.setup      |
	| for entries of groups = 1, standard              |
	|                       = 2, other or user defined | 
	 -------------------------------------------------*/
	SETUP = getsetup ();
    if (SETUP == 0)
	{
		SETUP = 1;
	}

	printf ("Running ...\n");

	/*----------------------
	| set connections to 0  |
	 ----------------------*/
	SetConnections (-1);
	run ();
	return (EXIT_SUCCESS);

}

int 
daemonize (void)
{
	pid_t pid;
						
	umask (0);
											 
 	if ((pid = fork ()) < 0)
	{
		 printf ("\nCannot fork");
	}
	else if (pid != 0)
	{       
	 	exit (0);
	}

	setsid ();

    sigset (SIGHUP, SIG_IGN);
	
	if ((pid = fork ()) < 0)
	{
		printf ("\nCannot fork");
	}
	else if (pid !=0)
	{
		exit (0);
	}

	chdir ("/");
	
	return 1;
}

int
getsetup (void)
{
	char buf [256];
	char *bufx;
    FILE *fp;
	char setup [256];
	int i=0;

	sprintf (setup, "%s/BIN/LRP/setup/lrp.server", 
				getenv ("PROG_PATH"));
	
	fp = fopen (setup, "r");
	if (fp == NULL)
	{
		return 0; 
	}

    /*------------------------- 
	| disregard the first line |
	 -------------------------*/
    fgets (buf, 256, fp);
	/*-------------------------------
	| this is what we are interested |
	--------------------------------*/
    fgets (buf, 256, fp);

    fclose (fp);
	
	bufx = buf;
    
	i=0;   
	while (*bufx != '=')
	{
		bufx++;
		i++; 

		if (i==256)
		{
			return 0;
		}
	}

	return atoi (++bufx);
}

int 
run (void)
{
    int bindOK;
    int listenStatus;
	int port;
	
	/*
    service = getservice ();
    if (service == NULL)
	{
		printf ("Unable to read lrp_server\n");
		return -1;
	}
	*/

	transmitAddr.sin_family = AF_INET;
    transmitAddr.sin_addr.s_addr = 0;

    port = resolveservice ("sel-term1", "tcp");
    if (port == -1)
	{
		 printf ("\nInvalid tcp service");
		 return 0;
	}
	
	transmitAddr.sin_port = port;

    transmitSocket = socket (AF_INET, SOCK_STREAM, 0); /* tcp */

	if (transmitSocket == -1)
	{
		 printf ("\nCreate failed ...");
		 return -1;
	}

	bindOK = bind (transmitSocket,
				  (struct sockaddr *)&transmitAddr,
				   sizeof (transmitAddr));
 

    if (bindOK != 0)
    {
		 printf ("bind failed ...");
 	     return  0;
    }
 
    while (1)
	{ 
		if (currentConnections < MAX_CONNECTION)
		{
			printf ("Waiting for connection ...\n");

			listenStatus = listen (transmitSocket, 10);

			if (listenStatus != 0)
			{
				 printf ("Listen failed ...");
			 	return 0; /* SERR_LISTEN_FAILED;  */
	    	}
			
			/*----------------------------------------
			| Increase number of connections if child |
			| process successfully created            |
			-----------------------------------------*/
	    	if (processclient () == 1)
			{
				/*----------------------------
				| add 1 to connection numbers |
				 ----------------------------*/
				SetConnections (0);
				printf ("Current connections=%d\n", currentConnections);
			}
		}
		else
		{
			sleep (15);
		}

	}

}

int
processclient (void)
{
	int addrLen;
	
	addrLen =  sizeof(drlAddr);

 	drlSocket = accept (transmitSocket,
				(struct sockaddr *)&drlAddr,
			     &addrLen);

    if (drlSocket == -1 )
    {                                                                           
	   printf("Accept failed ...\n");
       return 0; /* (SERR_ACCEPT_FAILED ); */
    }       

	/*----------------------  
	| create child process  |
	-----------------------*/ 
	if (fork () == 0)
	{
		OpenDB ();
		/*----------------------
		| main processing loop  |
		 ----------------------*/
		ChildLoop ();
	    close (drlSocket);
	    CloseDB (); 
		FinishProgram ();
	    exit (0);
	}
	else
    {	
		sleep (sleepTime);
	}

	return 1;
}

/*=============================
| main child, proccessing loop | 
 =============================*/
void
ChildLoop (void)
{
	terminated = 0;

	while (!terminated)
	{
		/*--------------------------
		| get incoming data header  |
		 --------------------------*/
	 	switch (GetMessage ())
		{
			case LRP_INMR:
				GetINMR ();
				break;
			case LRP_INUM:
				GetINUM ();
				break;
			case LRP_INTR:
				GetINTR ();
				break;
			case LRP_CCMR:
				GetCCMR ();
				break;
			case LRP_ESMR:
				GetESMR ();
				break;
			case LRP_EXCF:
				GetEXCF ();
				break;
			case LRP_FFDM:
			case LRP_START:
			case LRP_MODEL:
			case LRP_PARAMETERS:
			case LRP_OVERRIDES:
			case LRP_RESULTS:
			case LRP_ERROR:
			case LRP_QUIT:
				exit;		
		}
	}
	
}


/*========================================\
| lrp_server gets service name from       | 
|../BIN/LRP/lrp_server/setup/lrp.server   |
=========================================*/
char *
getservice (void)
{
	static char buf [256];
    FILE *service;
	char setup [256];
	
	sprintf (setup, "%s/BIN/LRP/setup/lrp.server", 
				getenv ("PROG_PATH"));
	
	service = fopen (setup, "r");
	if (service == NULL)
	{
		return NULL;
	}

    fgets (buf, 256, service);
	
	fclose (service);

	return buf;
}

/*===================================
| mode = 0 add, mode == 1, subtract  |
 ===================================*/
void
SetConnections (
 int mode)
{

	char buf [81];
    FILE *fp;
	char setup [256];
	int i=0;

	sprintf (setup, "%s/BIN/LRP/setup/lrp.connections", 
				getenv ("PROG_PATH"));
	
	fp = fopen (setup, "r");
	if (fp == NULL)
	{
		return; 
	}

    /*------------------------- 
	| disregard the first line |
	 -------------------------*/
    fgets (buf, 80, fp);

    i = atoi (buf);

	rewind (fp);

	if (mode == 0)
	{
		sprintf (buf,"%d", ++i);
	}
	else if (mode == 1)
	{
		sprintf (buf, "%d", i > 0 ? --i: 0);
	}
	else 
	{
		sprintf (buf,"%d", 0);
	}

    currentConnections = i;

	fputs (buf, fp);

    fclose (fp);
	
}

void
setsignals (void)
{
    signal (SIGTERM, endprocess);
	signal (SIGHUP,  endprocess);
	signal (SIGUSR1, endprocess);
	signal (SIGCHLD, SIG_IGN);    
}                                 

void                       
endprocess (
 int sig)
{           
	 signal (sig, SIG_IGN);

	 signal (sig, endprocess);
     
	 /*--------------------------------------
	 | decrease number of connections.       |
	 | This is the death of a child process. |
	 ---------------------------------------*/
	 if (currentConnections >= 0)
	 {
		SetConnections (1);
	 }

	 terminated = 1;
}

/*============================================
| resolve /etc/services entry for serviceName |
 ============================================*/
int
resolveservice (
 char   *serviceName,
 char   *protocolName)
{
		u_short portNumber;
		struct  servent *pServiceEntry;
								 
	    /*-----------------------------------
		| Get service port number from name. |
		 -----------------------------------*/
		if ((pServiceEntry = getservbyname (serviceName, protocolName)))
		{
			   portNumber = pServiceEntry->s_port;
		}
		else if ((portNumber = htons ((u_short)atoi (serviceName))) == 0)
		{
			   portNumber = -1;
        }

		return (portNumber);
}		

/*==========================================
| Read socket for incoming 8 byte header.   |
| buffer, drlSocket are globals             |
| RETURNS : message received on success     |
|           LRP_QUIT if socket is invalid   |
			LRP_EXIT on recv error          |
 ==========================================*/
int  
GetMessage (void)
{
    char mess [5];
    int count;
    char *buf;

    if (drlSocket < 1)
    {
		 return  LRP_QUIT;
	}

	memset(buffer,0,  MAX_BUF);

	count = recv(drlSocket, buffer,8, 0);
    if (count == -1 )
	{
		close (drlSocket);
	    exit (0);
		/* return  LRP_ERROR; */
	}

	strncpy (mess,buffer, 4);
	mess [4] = '\0';

	buf = buffer;
	buf += 4;

    /*-----------------------------------
	| set the length of the message      |
	| for the succeding GetData () call  |  
	------------------------------------*/
	msgLen = atoi (buf);

    return atoi (mess);
}

void 
GetData (void)
{
     char *data;
	 char bufx [MAX_BUF];
	 int i; 

	 memset (buffer, 0, MAX_BUF);
	 memset (bufx, 0, MAX_BUF);
	 strcpy (buffer, "");                                     

	 while (1)
	 {
		  memset (bufx,0, MAX_BUF);
		  i =     recv (drlSocket, bufx, msgLen, 0);
		  if (i < msgLen)
		  {
			  strcat (buffer, bufx);
			  msgLen -= i;
		  }
		  else
		  {
			  strcat (buffer, bufx);
			  break;
		  }
	 }

	 data = buffer;

     /*------------------------------------------------------
	 | each data sent from client are prefixed by a 6 bytes  |   
	 | co/br/wh data                                         |
	 -------------------------------------------------------*/
	 strncpy (CO, data, 2);
	 CO [2] = '\0';
     data += 2;

	 strncpy (BR, data, 2);
	 BR [2] = '\0';
     data += 2;

	 strncpy (WH, data, 2);
	 WH [2] = '\0';
     data += 2;

	 strcpy (buffer, data);
}

/*=================================
| Returns 1 on success; 0 on error |
=================================*/
int 
SendMessage (
 int mss, 
 int len)
{
   char mess [9];
   sprintf (mess, "%04d%04d", mss, len);
 
   if (send (drlSocket, mess, 8, 0) == -1)
   {
		return 0;
   }
   else
   {
		return 1;
   }
}

void
SendData (void)
{
    int s;
	int len = strlen (buffer);
	char *buf = buffer;
									  
	 while (1)
	 {
		  s = send (drlSocket, buf, len, 0);

	      if (s < len)
		  {
				buf += s;
				len -= s;
		  }
		  else
		  {
				break;
		  }
	 }
}

void
EndMessage (void)
{
	SendMessage (LRP_ERROR, 0);
}

/*===============================
| Application specific routines  | 
================================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");
	open_rec (inls, inls_list, INLS_NO_FIELDS, "inls_id_no2");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (intr);
	abc_fclose (inls);
	abc_fclose (excf);
	abc_fclose (ccmr);

	abc_fclose (data);
}

void
GetINMR (void)
{
	char mode [2];
    char key [17];
    char *buf;

	GetData ();
    buf = buffer;
    
	/*------------------
	| mode is sent as   | 
	| 1 - group         |
	| 2 - sub group     |
	| 3 - item          |
	| 4 - getall        |
	-------------------*/
	strncpy (mode,buf, 1);
    mode [1] = '\0';

	buf += 1;
	
	strcpy (key, buf);
    
   	switch (mode [0])
	{
		case '1':
			if (SETUP == 1)
			{
				GetGroup (buf);
			}
			else
			{
			}
			break;
		case '2':
			if (SETUP == 1)
			{
				GetSubgroup (buf);
			}
			else
			{
			}
			break; 
		case '3':
			GetItem (buf);
			break;
		case '4':
			GetAllItems ();
			break;
	}
}

/*==============================================================
| Group with this version is taken as inmr_class, june 27, 1999 |
 ==============================================================*/
void
GetGroup (
 char *group)
{
	int cc;

	abc_selfield (inmr, "inmr_id_no_6");
	
	inmr_rec.class [0] = group [0];
	strcpy (inmr_rec.item_no, "                ");
	strcpy (inmr_rec.co_no, CO);

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	while (!cc && 
		   inmr_rec.class [0] == group [0] && 
		   strcmp (inmr_rec.co_no, CO) == 0)
	{
		/*---------------------
		| send the inmr record |
		----------------------*/
		sendthisinmr ();

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
   
	/*--------------------------------------------
	| terminate each loop with an EndMessage (),  |
	| the other end of the pipe wait for this to  |
	| complete the task.                          |
	 --------------------------------------------*/
	EndMessage ();
	
	abc_selfield (inmr, "inmr_hhbr_hash");
}

/*=======================
| inmr transmit routine. | 
=========================*/
void
sendthisinmr (void)
{
	memset (buffer, 0, MAX_BUF);
	
	sprintf (buffer,
				 "%-2.2s"
				 "%-10ld"
				 "%-16.16s"
				 "%-1.1s"
				 "%-11.11s"
				 "%-40.40s"
				 "%-10ld",
				 inmr_rec.co_no,
				 inmr_rec.hhbr_hash,
				 inmr_rec.item_no,
				 inmr_rec.class,
				 inmr_rec.category,
				 inmr_rec.description,
				 inmr_rec.std_uom);

    printf ("%s\n", buffer);

	if (SendMessage (LRP_INMR, strlen (buffer)))
	{
		SendData ();
	}
}

/*====================================================
| subgroup is taken as inmr_category, june 27, 1999   | 
=====================================================*/
void 
GetSubgroup (
 char *subgroup)
{
	int cc;
    int len;

	abc_selfield (inmr, "inmr_id_cat");
	
	clip (subgroup );
    
	len = strlen (subgroup);

	strcpy (inmr_rec.co_no, CO);
	strcpy (inmr_rec.category, subgroup);

    cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
    
	while (!cc && 
			strcmp (inmr_rec.co_no, CO) == 0 &&
			strncmp (inmr_rec.category, subgroup, len)== 0)
	{
	    sendthisinmr ();	
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

	EndMessage ();

    abc_selfield (inmr, "inmr_hhbr_hash");
}

void
GetItem (
 char *item)
{
	int cc;

	abc_selfield (inmr, "inmr_id_no");
	
	strcpy (inmr_rec.item_no, item);
	strcpy (inmr_rec.co_no, CO);
    
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	while (!cc && strcmp (inmr_rec.item_no, item) == 0 &&
			strcmp (inmr_rec.co_no, CO) == 0)
	{
		sendthisinmr ();
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
    
	EndMessage ();

	abc_selfield (inmr, "inmr_hhbr_hash");
}

void
GetAllItems (void)
{
	int cc;

	abc_selfield (inmr, "inmr_id_no");
	
	strcpy (inmr_rec.item_no, "                ");
	strcpy (inmr_rec.co_no, CO);
    
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	while (!cc && 
			strcmp (inmr_rec.co_no, CO) == 0)
	{
		sendthisinmr ();
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
    
	EndMessage ();

	abc_selfield (inmr, "inmr_hhbr_hash");
}

void
GetINUM (void)
{
	int cc;
    
	GetData ();
	
	inum_rec.hhum_hash = 0;

	cc = find_rec (inum, &inum_rec, GTEQ, "r");
	
	while (!cc)
	{
		memset (buffer, 0, MAX_BUF);
		
		sprintf (buffer, "%-10ld"
						 "%-4.4s"
						 "%-40.40s"
						 "%-10.2f",
						 inum_rec.hhum_hash,
						 inum_rec.uom,
						 inum_rec.desc,
						 inum_rec.cnv_fct);

		SendMessage (LRP_INUM, strlen (buffer));

		SendData ();

		cc = find_rec (inum, &inum_rec, NEXT, "r");

	}

	EndMessage ();

}

void
GetCCMR (void)
{
}

void 
GetESMR (void)
{
}

/*===================================================================\
| Get and send inventory transaction record for a specific hhbr_hash |
| for a date range                                                   |
====================================================================*/
void
GetINTR (void)
{
	int  	cc;
    char 	*buf;
	char	chhbr [11];
	long	incc, hhbr;
    char	from_date [11], to_date [11];
	long    lfrom_date, lto_date;

    GetData ();

	buf = buffer;
	
	/*-------------------
    | extract hhbr_hash  |
	 -------------------*/
	strncpy (chhbr, buf, 10);
	chhbr [10] = '\0';
	buf += 10;

	strncpy (from_date, buf, 10);
    from_date [10] = '\0';
	buf += 10;

	strncpy (to_date, buf, 10);
	to_date [10] = '\0';

    /*---------
	| converts | 
    ----------*/
	hhbr = atol(chhbr);
    intr_rec.hhbr_hash = hhbr;

	lfrom_date = StringToDate (from_date);
	lto_date   = StringToDate (to_date);

	incc = GetIncc ();

	cc = find_rec (intr, &intr_rec, GTEQ, "r");
	while (!cc && 
		   hhbr == intr_rec.hhbr_hash)
	{
		/*----------------------------------
		| send only the following intr_type |
		 ----------------------------------*/
		if (intr_rec.hhcc_hash == incc && 
			lfrom_date >= intr_rec.date &&
			lto_date <= intr_rec.date && 
			(intr_rec.type == 3  || 	/* stock issue      */ 
			 intr_rec.type == 6  || 	/* invoice          */
			 intr_rec.type == 7  || 	/* credit           */
			 intr_rec.type == 9  || 	/* stock transfer   */
			 intr_rec.type == 10 || 	/* production order */ 
			 intr_rec.type == 13    	/* drop ship sales  */
			 ))
		{
			memset (buffer, 0, MAX_BUF);
			sprintf (buffer,
					 "%-12.4f"
					 "%-10.2f"
					 "%-10.2f"
					 "%1s",
					 intr_rec.qty,
					 intr_rec.cost_price,
					 intr_rec.sale_price,
					 intr_rec.stat_flag); 
		
			SendMessage (LRP_INTR, strlen (buffer));

			SendData ();
		}

 
	}
	
	EndMessage ();
}

void
GetEXCF (void)
{
	int cc;
	
	GetData ();

	strcpy (excf_rec.co_no, CO);
	strcpy (excf_rec.cat_no, "           ");
 
	cc = find_rec (excf, &excf_rec, GTEQ, "r");

	while (cc == 0 ) 
	{
		memset (buffer, MAX_BUF, 0);
		sprintf (buffer,"%-2s" 
						"%-11s"
						"%-40s",
						excf_rec.co_no,
						excf_rec.cat_no,
						excf_rec.cat_desc);

		SendMessage (LRP_EXCF, strlen (buffer));

		SendData ();

		cc = find_rec (excf, &excf_rec, NEXT, "r");	
	}

	EndMessage ();
}

long
GetIncc (void)
{
	int cc;

	strcpy (ccmr_rec.co_no, CO);
	strcpy (ccmr_rec.est_no, BR);
	strcpy (ccmr_rec.cc_no, WH);

	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (!cc)
	{
		return ccmr_rec.hhcc_hash;
	}
	
	return 0l;
}

void
GetInls (
 long hhbr, 
 long from, 
 long to)
{
	int cc;
	long hhcc = GetIncc ();
	double qty   = 0,
		   value = 0,
		   cost  = 0;
	
	inls_rec.hhbr_hash = hhbr;

	cc = find_rec (inls, &inls_rec, GTEQ, "r");

	while (!cc && inls_rec.hhbr_hash == hhbr)
	{
		if (from >= inls_rec.date && 
			to <= inls_rec.date &&
			hhcc == inls_rec.hhcc_hash)
		{
			qty   += inls_rec.qty;
			value += inls_rec.value;
			cost  += inls_rec.cost;
		} 

		cc = find_rec (inls, &inls_rec, NEXT, "r");
	}

	memset (buffer, 0, MAX_BUF);
	
	sprintf (buffer,
			"%-10ld"
			"%-10ld"
			"%-12.2f"
			"%-12.2f"
			"%-12.2f",
			hhcc,
			hhbr,
			qty,
			value,
			cost);
	
	SendMessage (LRP_INLS, strlen (buffer));

	SendData ();
	/*----------------------------------------------
	| No need for an EndMessage () call as this is |
	| part of another message                      |  
	 ---------------------------------------------*/
	/*
	EndMessage ();
	*/
}

/* END OF FILE */
