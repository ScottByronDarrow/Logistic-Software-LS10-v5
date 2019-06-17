/*============================================================================
|  Copyright (C) 1996 - 1999 SOFTWARE ENGINEERING LIMITED.            		 |
|============================================================================|
| Program Name   : ( pos_srvd.c             )                                |
| Program Desc   : ( POS pipe server                                       ) |
|                : ( Handles requests from a POS terminal                  ) |
|----------------------------------------------------------------------------|
| Date written   : ( August 31, 1998 )      | Author    :  Primo O. Esteria  |
|----------------------------------------------------------------------------|
| Modifications  :                                                           |
|                : January 6, 1999          : Primo O. Esteria               | 
|----------------------------------------------------------------------------|
| Comments       :                                                           |
|				 : (January 6, 1999) Modified add_hash to accept the correct |
|				 :                   incc_hash.  Modified abc_selfield to    |
|				 :                   accomodate change in index of the cohr  |
| $Log: pos_srvd.c,v $
| Revision 5.3  2002/11/28 04:09:48  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.2  2001/08/09 09:50:24  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:53  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:26  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:15  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:49  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  2000/07/14 06:26:54  scott
| Updated to fix warnings under linux
|
| Revision 1.11  2000/02/18 02:22:13  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.10  1999/11/19 06:18:32  scott
| Updated for warning errors.
|
| Revision 1.9  1999/10/16 01:48:44  scott
| Updated from ansi
|
| Revision 1.8  1999/06/18 02:05:27  scott
| Updated for log.
|
=============================================================================| 
*/
#ifdef	LINUX
#define	_GNU_SOURCE
#endif	// LINUX

#define CCMAIN
char	*PNAME = "$RCSfile: pos_srvd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/POS/pos_srvd/pos_srvd.c,v 5.3 2002/11/28 04:09:48 scott Exp $";

#define NO_SRCGEN

#include <pslscr.h>

#include <alarm_time.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <std_decs.h>

/* #include <proc_sobg.h> */

/* adopted from so_calpayment.c */
#define SERR_NO_ERROR           0
#define SERR_SOCKCREATE_FAILED  1
#define SERR_BIND_FAILED        2
#define SERR_LISTEN_FAILED      3
#define SERR_ACCEPT_FAILED      4
#define SERR_INVALID_SOCKET     5
#define SERR_CONN_REFUSED       6
#define SERR_HOST_NOT_FOUND     7

#define TERM_OFFLINE                    		0       
#define TERM_ONLINE                             1
#define TERM_RECV                               2
#define TERM_SEND                               3

#define UNSET_LOCK                              0
#define SET_LOCK                                1

#include <messages.h>
#include	"pos_srvd.h"

int                     transmitSocket;
struct sockaddr_in      transmitAddr; 
struct sockaddr_in      drlAddr;
int                     drlSocket;
int                     socketsConn[MAX_TERMINALS];
char                    buffer[1024];
char                    *So_numbers;
long                    hhco_hashx;


FILE                   *OnlineTerm=0;
FILE                   *RecvTerm=0;
/* FILE                  *SendTerm=0; */
FILE                   *pipeLog=0;

char 	 *comm = "comm",
         *data = "data",
         *cohr = "cohr",
         *coln = "coln",
         *sons = "sons",
         *inls = "inls", 
         *cudp = "cudp",
         *esmr = "esmr",
         *ccmr = "ccmr",
         *sobg = "sobg",
         *inmr = "inmr",
         *cumr = "cumr",
         *inpr = "inpr",
         *incp = "incp",
         *inds = "inds",
         *excf = "excf",
         *inum = "inum",
         *insf = "insf",
         *sokt = "sokt",
         *cnch = "cnch",
         *cncd = "cncd",
         *inbm = "inbm",
         *posdtup = "posdtup",
         *posterm = "posterm";

char *incc = "incc";

        /*======+
         | cohr |
         +======*/
 #define        COHR_NO_FIELDS 71       

        struct dbview   cohr_list [COHR_NO_FIELDS] =
        {
                {"cohr_co_no"},
                {"cohr_br_no"},
                {"cohr_dp_no"},
                {"cohr_inv_no"},
                {"cohr_app_inv_no"},
                {"cohr_hhcu_hash"},
                {"cohr_type"},
                {"cohr_cont_no"},
                {"cohr_drop_ship"},
                {"cohr_hhds_hash"},
                {"cohr_cus_ord_ref"},
                {"cohr_ord_ref"},
                {"cohr_grn_no"},
                {"cohr_cons_no"},
                {"cohr_carr_code"},
                {"cohr_carr_area"},
                {"cohr_no_cartons"},
                {"cohr_wgt_per_ctn"},
                {"cohr_no_kgs"},
                {"cohr_hhso_hash"},
                {"cohr_hhco_hash"},
                {"cohr_frei_req"},
                {"cohr_date_raised"},
                {"cohr_date_required"},
                {"cohr_tax_code"},
                {"cohr_tax_no"},
                {"cohr_area_code"},
                {"cohr_sale_code"},
                {"cohr_op_id"},
                {"cohr_time_create"},
                {"cohr_date_create"},
                {"cohr_gross"},
                {"cohr_freight"},
                {"cohr_insurance"},
                {"cohr_other_cost_1"},
                {"cohr_other_cost_2"},
                {"cohr_other_cost_3"},
                {"cohr_tax"},
                {"cohr_gst"},
                {"cohr_disc"},
                {"cohr_deposit"},
                {"cohr_ex_disc"},
                {"cohr_erate_var"},
                {"cohr_sos"},
                {"cohr_exch_rate"},
                {"cohr_fix_exch"},
                {"cohr_batch_no"},
                {"cohr_dl_name"},
                {"cohr_dl_add1"},
                {"cohr_dl_add2"},
                {"cohr_dl_add3"},
                {"cohr_din_1"},
                {"cohr_din_2"},
                {"cohr_din_3"},
                {"cohr_pay_terms"},
                {"cohr_sell_terms"},
                {"cohr_ins_det"},
                {"cohr_pri_type"},
                {"cohr_pri_break"},
                {"cohr_ord_type"},
                {"cohr_prt_price"},
                {"cohr_status"},
                {"cohr_stat_flag"},
                {"cohr_ps_print"},
                {"cohr_inv_print"},
                {"cohr_ccn_print"},
                {"cohr_printing"},
                {"cohr_hhtr_hash"},
                {"cohr_load_flag"},
                {"cohr_pos_inv_no"},
                {"cohr_pos_tran_type"}
        };

        struct tag_cohrRecord
        {
                char    co_no [3];
                char    br_no [3];
                char    dp_no [3];
                char    inv_no [9];
                char    app_inv_no [9];
                long    hhcu_hash;
                char    type [2];
                char    cont_no [7];
                char    drop_ship [2];
                long    hhds_hash;
                char    cus_ord_ref [21];
                char    ord_ref [17];
                char    grn_no [21];
                char    cons_no [17];
                char    carr_code [5];
                char    carr_area [3];
                int             no_cartons;
                double  wgt_per_ctn;
                float   no_kgs;
                long    hhso_hash;
                long    hhco_hash;
                char    frei_req [2];
                Date    date_raised;
                Date    date_required;
                char    tax_code [2];
                char    tax_no [16];
                char    area_code [3];
                char    sale_code [3];
                char    op_id [15];
                char    time_create [6];
                Date    date_create;
                Money   gross;
                Money   freight;
                Money   insurance;
                Money   other_cost_1;
                Money   other_cost_2;
                Money   other_cost_3;
                Money   tax;
                Money   gst;
                Money   disc;
                Money   deposit;
                Money   ex_disc;
                Money   erate_var;
                Money   sos;
                double  exch_rate;
                char    fix_exch [2];
                char    batch_no [6];
                char    dl_name [41];
                char    dl_add1 [41];
                char    dl_add2 [41];
                char    dl_add3 [41];
                char    din_1 [61];
                char    din_2 [61];
                char    din_3 [61];
                char    pay_terms [41];
                char    sell_terms [4];
                char    ins_det [31];
                char    pri_type [2];
                char    pri_break [2];
                char    ord_type [2];
                char    prt_price [2];
                char    status [2];
                char    stat_flag [2];
                char    ps_print [2];
                char    inv_print [2];
                char    ccn_print [2];
                char    printing [2];
                long    hhtr_hash;
                char    load_flag [2];
                char    pos_inv_no[11];
                int     pos_tran_type;

        }       cohr_rec;

struct dbview coln_list [] = 
{
        {"coln_hhcl_hash"},
        {"coln_hhco_hash"},
        {"coln_line_no"},
        {"coln_hhbr_hash"},
        {"coln_incc_hash"},
        {"coln_hhum_hash"},
        {"coln_hhsl_hash"},
        {"coln_hhdl_hash"},
        {"coln_crd_type"},
        {"coln_serial_no"},
        {"coln_cont_status"},
        {"coln_qty_org_ord"},
        {"coln_q_order"},
        {"coln_qty_del"},
        {"coln_qty_ret"},
        {"coln_q_backorder"},
        {"coln_gsale_price"},
        {"coln_sale_price"},
        {"coln_cost_price"},
        {"coln_disc_pc"},
        {"coln_reg_pc"},
        {"coln_disc_a"},
        {"coln_disc_b"},
        {"coln_disc_c"},
        {"coln_cumulative"},
        {"coln_tax_pc"},
        {"coln_gst_pc"},
        {"coln_gross"},
        {"coln_freight"},
        {"coln_on_cost"},
        {"coln_amt_disc"},
        {"coln_amt_tax"},
        {"coln_amt_gst"},
        {"coln_erate_var"},
        {"coln_pack_size"},
        {"coln_sman_code"},
        {"coln_cus_ord_ref"},
        {"coln_o_xrate"},
        {"coln_n_xrate"},
        {"coln_item_desc"},
        {"coln_due_date"},
        {"coln_status"},
        {"coln_bonus_flag"}, 
        {"coln_hide_flag"},
        {"coln_hhah_hash"},
        {"coln_stat_flag"}
};

int coln_fields_no = 46;

struct
{
        long hhcl_hash;
        long hhco_hash;
        int  line_no;
        long hhbr_hash;
        long incc_hash;
        long hhum_hash;
        long hhsl_hash;
        long hhdl_hash;
        char crd_type[2];
        char serial_no[26];
        int cont_status;
        float qty_org_ord;
        float q_order;
        float qty_del;
        float qty_ret;
        float q_backorder;
        double gsale_price;
        double sale_price;
        double cost_price;
        float disc_pc;
        float reg_pc;
        float disc_a;
        float disc_b;
        float disc_c;
        int cumulative;
        float tax_pc;
        float gst_pc;
        double gross;
        double freight;
        double on_cost;
        double amt_disc;
        double amt_tax;
        double amt_gst;
        double erate_var;
        char pack_size[6];
        char sman_code[3];
        char cus_ord_ref[21];
        float o_xrate;
        float n_xrate;
        char item_desc[41];
        long due_date;
        char status[2];
        char bonus_flag[2];
        char hide_flag[2];
        long hhah_hash;
        char stat_flag[2];

} coln_rec;

struct dbview sons_list [] = 
{
        { "sons_hhso_hash" },
        { "sons_hhco_hash" },
        { "sons_hhsl_hash" },
        { "sons_hhcl_hash" },
        { "sons_line_no" },
        { "sons_desc" }

};

int sons_fields_no = 6;

struct       
{
   long hhso_hash;
   long hhco_hash;
   long hhsl_hash;
   long hhcl_hash;
   int line_no;
   char desc [41];
} sons_rec;

struct dbview inls_list[] = 
{
        { "inls_co_no" },
        { "inls_est_no" },
        { "inls_date" },
        { "inls_hhbr_hash" },
        { "inls_hhcc_hash" },
        { "inls_hhcu_hash" },
        { "inls_area_code" },
        { "inls_sale_code" },
        { "inls_qty" },
        { "inls_value" },
        { "inls_cost" },
        { "inls_res_code" },
        { "inls_res_desc" },
        { "inls_status" } 
};

int inls_fields_no = 14;

struct 
{
        char co_no [3];
        char est_no [3];
        long date;
        long hhbr_hash;
        long hhcc_hash;
        long hhcu_hash;
        char area_code [3];
        char sale_code [3];
        float qty;
        double value;
        double cost;
        char res_code [3];
        char res_desc [61];
        char status [2];

} inls_rec;

struct dbview cudp_list [] =
{
        { "cudp_co_no" },
        { "cudp_br_no" },
        { "cudp_dp_no" },
        { "cudp_dp_name" },
        { "cudp_csh_pref" },
        { "cudp_chg_pref" },
        { "cudp_crd_pref" },
        { "cudp_man_pref" },
        { "cudp_nx_chg_no" },
        { "cudp_nx_csh_no" },
        { "cudp_nx_crd_no" },
        { "cudp_nx_man_no" },
        { "cudp_nx_sav_no" },
        { "cudp_stat_flag"} 
};

int cudp_fields_no = 14; 

struct   
{
   char co_no [3];
   char br_no [3];
   char dp_no [3];
   char dp_name [41];
   char csh_pref [3];
   char chg_pref [3];
   char crd_pref [3];
   char man_pref [3];
   long nx_chg_no;
   long nx_csh_no;  
   long nx_crd_no;
   long nx_man_no;
   long nx_sav_no;
   char stat_flag [2];
   
} cudp_rec;

struct dbview esmr_list [] = 
{
   { "esmr_co_no" },
   { "esmr_est_no" },
   { "esmr_est_name" },
   { "esmr_chg_pref" },
   { "esmr_csh_pref" },
   { "esmr_crd_pref" },
   { "esmr_man_pref" },
   { "esmr_nx_sav_inv" },
   { "esmr_nx_csh_inv" },
   { "esmr_nx_csh_crd" },
   { "esmr_nx_inv_no" },
   { "esmr_nx_man_no" },
   { "esmr_nx_request_no" },
   { "esmr_nx_slip_no" },
   { "esmr_nx_crd_nte_no" },
   { "esmr_nx_ccn_no"}
};


int esmr_fields_no = 16;

struct
{
   char co_no [3];
   char est_no [3];
   char est_name [41];
   char chg_pref [3];
   char csh_pref [3];
   char crd_pref [3];
   char man_pref [3];
   long nx_sav_inv;
   long nx_csh_inv;
   long nx_csh_crd;
   long nx_inv_no;
   long nx_man_no;
   long nx_request_no;
   long nx_slip_no;
   long nx_crd_nte_no;
   long nx_ccn_no;
} esmr_rec;

struct dbview posterm_list [] =
{
   { "pos_no"},
   { "ip_address" },
   { "co_no" },
   { "br_no" },
   { "wh_no" },
   { "last_login"},
   { "last_logoff"},
   { "last_user"}

};

int posterm_fields_no = 8;

struct
{
   int  pos_no;
   char ip_address [16];
   char co_no [3];
   char br_no [3];
   char wh_no [3];
   long last_login;
   long last_logoff;
   char last_user [15];

} posterm_rec;

struct dbview ccmr_list [] = 
{
  { "ccmr_co_no" },
  { "ccmr_est_no"},
  { "ccmr_cc_no" },
  { "ccmr_hhcc_hash"}
};

int ccmr_fields_no = 4;

struct
{
   char co_no [3];
   char est_no [3];
   char cc_no [3];
   long hhcc_hash;
} ccmr_rec;

struct dbview incc_list [] = 
{
        { "incc_hhcc_hash"},
        { "incc_hhbr_hash"},
        { "incc_committed"},
        { "incc_sales" },
        { "incc_closing_stock"},
        { "incc_ytd_sales"}
};

int incc_fields_no = 6;

struct
{
   long    hhcc_hash;
   long    hhbr_hash;
   float   committed;
   float   sales;
   float   closing_stock;
   float   ytd_sales;
} incc_rec;

struct dbview sobg_list [] = 
{
   { "sobg_co_no" },
   { "sobg_br_no" },
   { "sobg_type"  },
   { "sobg_lpno" },
   { "sobg_hash" },
   { "sobg_hash2" },
   { "sobg_pid" },
   { "sobg_value"},
   { "sobg_last_line"}
};


int sobg_fields_no = 9;

struct 
{
   char   co_no [3];
   char   br_no [3];
   char   type [3];
   int    lpno;
   long   hash;
   long   hash2;
   long   pid;
   double value;
   int    last_line;
} sobg_rec;

struct dbview inmr_list [] = 
{
  {"inmr_co_no"},
  {"inmr_hhbr_hash"}
};
int inmr_fields_no = 2;
struct 
{
   char co_no [3];
   long hhbr_hash;
} inmr_rec;

struct dbview cumr_list [] = 
{
   {"cumr_co_no"},
   {"cumr_hhcu_hash"},
   {"cumr_dbt_no"},
   {"cumr_dbt_name"},
   {"cumr_ch_adr1"},
   {"cumr_ch_adr2"},
   {"cumr_ch_adr3"},
   {"cumr_ch_adr4"}

};
int cumr_fields_no = 8;
struct 
{
   char co_no [3];
   long hhcu_hash;
   char dbt_no [7];
   char dbt_name [41];
   char ch_adr1 [41];
   char ch_adr2 [41];
   char ch_adr3 [41];
   char ch_adr4 [41];
} cumr_rec;

struct dbview inpr_list [] =
{
 { "inpr_hhbr_hash"}
};
int inpr_fields_no = 1;
struct
{
  long hhbr_hash;
} inpr_rec;
  
struct dbview incp_list [] =
{
  { "incp_key"},
  { "incp_curr_code"},
  { "incp_status"},
  { "incp_hhcu_hash"},
  { "incp_area_code"},
  { "incp_cus_type"},
  { "incp_hhbr_hash"},
  { "incp_date_from"}
};
int incp_fields_no = 8;

struct 
{
   char key [7];
   char curr_code [4];
   char status [2];
   long hhcu_hash;
   char area_code [3];
   char cus_type [4];
   long hhbr_hash;
   long date_from;
} incp_rec;

struct dbview inds_list [] =
{
  { "inds_hhcu_hash" },
  { "inds_hhbr_hash" }
};
int inds_fields_no = 2;
struct
{
  long hhcu_hash; 
  long hhbr_hash;
} inds_rec;

struct dbview excf_list [] = 
{
  { "excf_hhcf_hash"},
  { "excf_co_no"},
  { "excf_cat_no" }
};
int excf_fields_no = 3;
struct
{
  long hhcf_hash;
  char co_no [3];
  char cat_no [12];
} excf_rec;

struct dbview inum_list [] = 
{
  { "inum_hhum_hash"}
};
int inum_fields_no = 1;
struct
{
  long hhum_hash;
} inum_rec;

struct dbview insf_list [] =
{
  {"insf_hhwh_hash"},
  {"insf_hhbr_hash"},
  {"insf_serial_no"},
  {"insf_status"}
};
int insf_fields_no = 4;
struct
{
  long hhwh_hash;
  long hhbr_hash;
  char serial_no [26];
  char status [2]; 
} insf_rec;

struct dbview sokt_list [] =
{
  { "sokt_hhbr_hash"},
  { "sokt_line_no"},
  { "sokt_mabr_hash"},
  { "sokt_due_date"},
  { "sokt_bonus"}
};
int sokt_fields_no = 5;
struct 
{
   long  hhbr_hash;
   int   line_no;
   long  mabr_hash;
   long  due_date;
   char  bonus [2];

} sokt_rec;
 
struct dbview cnch_list [] = 
{
   { "cnch_hhch_hash"}
};
int cnch_fields_no = 1;
struct
{
  long hhch_hash;
} cnch_rec;

struct dbview cncd_list [] =
{
  { "cncd_hhch_hash" }
};
int cncd_fields_no = 1;
struct
{
  long hhch_hash;
} cncd_rec;

struct dbview posdtup_list [] =
{
  { "pos_no" },
  { "file_name"},
  { "record_hash"},
  { "action"}
};
int posdtup_fields_no = 4;
struct 
{
   int  pos_no;
   char file_name [11];
   long record_hash;
   char action [2];
} posdtup_rec;

struct dbview inbm_list[] =
{
  { "inbm_co_no" },
  { "inbm_barcode" },
  { "inbm_item_no"}
};

int inbm_fields_no = 3;

struct
{
  char co_no [3];
  char barcode [17];
  char item_no [17];
} inbm_rec;

/* ==================================== */

char com_no [3];
char bra_no [3];
char war_no [3];
int  terminal_no;
int  terminated = 0;

void open_db (void);
void close_db (void);

void OpenLogFiles (void);
void CloseLogFiles (void);

int SetListen (void);
int AcceptConnection (void);
void StartProcessing (void);
char * GetMessage (int);

int GetCommsInit (int);
int GetTransHeader (int);
int GetTransLines (int);
int GetNotesLines (int);
int SendAll (void);
int GetLostLines (int);

char *GetAppInv (char *);

int UpdateStatus (int, int, int);
int UpdateStatusLog (FILE *,int);

int UpdateCohr (struct TransHeader *);
int UpdateColn (struct TransLine *);
int UpdateSons (struct TransNotes *);
int UpdateIlns (struct TransLostSales *);
void GetCompany (int,char *, char *,char *, char *);
char *GetLastInvoiceNo (int);
long GetIncc (void);
void UpdateInventory (long, long);
void term_srvd (int);
void  ChildEnds (int);
void  SendInmr (void);
void SendCumr (void);
void SendInpr (void);
void SendInum (void);
void SendInds (void);
void SetSignals (void);
static void EndProcess (int);
void	daemonize (void);

#include <calypso.h>


int 
main(
 int argc, 
 char *argv [])
{

   SetSignals ();

   daemonize ();

   So_numbers = getenv ("SO_NUMBERS");
   SetListen ();

   return EXIT_SUCCESS;
}
 

void
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
}

void 
SetSignals (
 void)
{
	signal (SIGTERM, EndProcess);
	signal (SIGHUP,  EndProcess);
	signal (SIGUSR1, EndProcess);
	signal (SIGCHLD, SIG_IGN);

}

void
EndProcess (
 int sig)
{
	signal (sig, SIG_IGN);

	terminated = 1;

	signal (sig, EndProcess);

}


int 
SetListen (
 void)
{
    int bindOK;
    int listenStatus;

    transmitAddr.sin_family = AF_INET;
    transmitAddr.sin_addr.s_addr = 0;

    transmitAddr.sin_port = ResolveService ("sel-posPipe","tcp");  
    
    transmitSocket = socket (AF_INET, SOCK_STREAM, 0); /* tcp */
    
    if (transmitSocket == -1)
    {
         printf ("\nCreate failed ...");
         return SERR_SOCKCREATE_FAILED;
    }
    
    bindOK = bind (transmitSocket,
                  (struct sockaddr *)&transmitAddr,
                   sizeof (transmitAddr));

    if (bindOK != 0)
    {
          printf ("bind failed ...");
          return SERR_BIND_FAILED;

    }

	terminated = 0;

    while (terminated == 0)
    {
       listenStatus = listen (transmitSocket, 10);
   
       if (listenStatus != 0)
       {
           printf ("Listen failed ...");
		   sleep (sleepTime);
		   continue;
           /* return SERR_LISTEN_FAILED; */
       }

       AcceptConnection();

     }

     return SERR_NO_ERROR;
}

int 
AcceptConnection (
 void)
{
   int   addrLen;

   addrLen =  sizeof (drlAddr);

   drlSocket = accept (transmitSocket,
                      (struct sockaddr *)&drlAddr,
                      &addrLen);

   if (drlSocket == -1 )
   {
        printf ("Accept failed ...");
		close (drlSocket );

        return (SERR_ACCEPT_FAILED );
   }

   if (fork () == 0)
   {
		 StartProcessing ();
		 exit (0);
   }
   else
   {
		 sleep (sleepTime);
   }
   
   close (drlSocket );

   return 0;
}
 
/*===============================
| Processes incoming connection |   
 ================================*/
void 
StartProcessing (
 void)
{ 
        char *mess;
        terminated = 0;

        while (terminated == 0)
        {
                mess = GetMessage (drlSocket) ;
                if (!strcmp (mess,POS_INIT_COMMS))
                {
                    if ((terminal_no = GetCommsInit (drlSocket)) == -1)
                    {
                         CloseLogFiles ();
                         close (drlSocket);
                         printf ("Error on terminal initialization\n");
                         return;
                     }

                     OpenLogFiles ();
                     UpdateStatusLog (pipeLog, UNSET_LOCK);
                }
                else if (!strcmp (mess,POS_TRANS_HEADER))
                {
                     GetTransHeader (drlSocket); 
                }
                else if (!strcmp (mess, POS_TRANS_LINE))
                {
                     GetTransLines (drlSocket);
                }
                else if (!strcmp (mess, POS_TRANS_NOTES))
                {
                     GetNotesLines (drlSocket);
                }
                else if (!strcmp (mess, POS_LOST_SALES_LINE))
                {
                     GetLostLines (drlSocket);
                }
                else if (!strcmp (mess, POS_SEND_ALL))
                {
                     SendAll ();
                }
                else if (!strcmp (mess, POS_GEN_STATUS))
                {
                      /* what does this do */ 
                }
                else if (!strcmp (mess, POS_CUMR))
                {
                     SendCumr ();
                }
                else if (!strcmp (mess, POS_INMR))
                {
                     SendInmr ();
                }
                else if (!strcmp (mess, POS_INPR))
                {
                     SendInpr ();     
                }
                else if (!strcmp (mess, POS_INUM))
                {
                     SendInum ();
                }
                else if (!strcmp (mess, POS_INDS))
                {
                     SendInds ();
                }
                else if (!strcmp (mess, " "))
                {
                     CloseLogFiles ();
                     close (drlSocket);
                     exit (0);
                     return;
                }
                else if (!strcmp (mess,POS_SERV_EXIT))
                {
                     CloseLogFiles ();
                     close (drlSocket);
                     exit (0);
                     return ;
                }
        }

}

int 
GetCommsInit (
 int sockHandle)
{
      struct msg_init *buf;
      int  term = 0;
         
      memset (buffer,0,sizeof (buffer));

      if (recv (sockHandle,buffer,sizeof (struct msg_init),0) != -1)
      {
          buf  = (struct msg_init *)buffer;
          term =  atoi ((char *)buf->terminal);
           
          printf ("POS terminal %s connected\n",buf->terminal);
                  GetCompany (term,com_no,bra_no,war_no, buf->user);
      }
      else
      {
         /* return -1; */
      }
      
      send (sockHandle, "eeeee", 5, 0);
 
     return term;
}

int 
GetTransHeader (
 int sockHandle)
{
    struct TransHeader *buf;

    memset (buffer,0,sizeof (buffer));

    if (recv (sockHandle,buffer,sizeof (struct TransHeader),0) != -1 )
    {
           buf = (struct TransHeader *)buffer;
           UpdateCohr (buf); 
    }
    else
    {
          /*  return -1; */
    }
    
    send (sockHandle, "eeeee", 5, 0);

    return 0; 
}

int 
GetTransLines (
 int sockHandle)
{
    struct TransLine *buf;

    memset (buffer,0,sizeof(buffer));

    if (recv (sockHandle,buffer,sizeof (struct TransLine),0) != -1)
    {
        buf = (struct TransLine *)buffer;
        UpdateColn (buf);     
    }
    else
    {
        /* return -1; */
    }

    send (sockHandle, "eeeee", 5, 0);
    
	return 0;
}


int
GetNotesLines (
 int sockHandle)
{
    struct TransNotes *buf;

    memset (buffer,0,sizeof(buffer));

    if (recv (sockHandle,buffer,sizeof (struct TransNotes),0) != -1)
    {
         buf = (struct TransNotes *)buffer;
         UpdateSons (buf);
    }
    else
    {
         /* return -1; */
    }
    
    send (sockHandle, "eeeee", 5, 0);

    return 0;
}

/*===========================================================
| Send all data needed by the POS terminal.  This is         |    
| usually done upon installation of the POS terminal, once.  |
|                                                            |
| pos_upload will do the actual sending of data              |
============================================================*/
int
SendAll (
 void)
{
   int cc;

   /* send inmr */
   inmr_rec.hhbr_hash = 0L;
   cc = find_rec (inmr,&inmr_rec,GTEQ,"r");
   while (!cc)
   {
          posdtup_rec.pos_no = terminal_no;
          posdtup_rec.record_hash = inmr_rec.hhbr_hash;
          strcpy (posdtup_rec.file_name,"inmr");

          abc_add (posdtup,&posdtup_rec);
     
          cc = find_rec (inmr,&inmr_rec,NEXT,"r");
   }
  
   /* send cumr */
   cumr_rec.hhcu_hash = 0;
   cc = find_rec (cumr,&cumr_rec,GTEQ,"r");
   while (!cc)
   {
           posdtup_rec.pos_no = terminal_no;
           posdtup_rec.record_hash = cumr_rec.hhcu_hash;
           strcpy (posdtup_rec.file_name,"cumr");

           abc_add (posdtup,&posdtup_rec);
 
           cc = find_rec (cumr,&cumr_rec,NEXT, "r");
   }

   /* send inpr */
   inpr_rec.hhbr_hash = 0;
   cc = find_rec (inpr,&inpr_rec,GTEQ, "r");
   while (!cc)
   {
          posdtup_rec.pos_no = terminal_no;
          posdtup_rec.record_hash = inpr_rec.hhbr_hash;
          strcpy (posdtup_rec.file_name,"inpr");

          abc_add (posdtup,&posdtup_rec);

          cc = find_rec (inpr,&inpr_rec,NEXT,"r");

   }
   
   /* contract price */
   /* send incp      */
   strcpy (incp_rec.key,"");
   strcpy (incp_rec.curr_code,"");
   strcpy (incp_rec.area_code,"");
   strcpy (incp_rec.status,"");
   incp_rec.hhbr_hash = 0;
   incp_rec.hhcu_hash = 0;
   incp_rec.date_from = 0;

   cc = find_rec (incp,&incp_rec, GTEQ, "r");

   while (!cc)
   {
      posdtup_rec.pos_no = terminal_no;
      strcpy (posdtup_rec.file_name,"incp");
      posdtup_rec.record_hash = incp_rec.hhbr_hash;
     
      abc_add (posdtup,&posdtup_rec);
 
      cc = find_rec (incp,&incp_rec, NEXT, "r");
   }
   
   
   /* send inds */
   inds_rec.hhcu_hash = 0;
   cc = find_rec (inds,&inds_rec,GTEQ,"r");
   while (!cc)
   {
          posdtup_rec.pos_no = terminal_no;
          posdtup_rec.record_hash = inds_rec.hhcu_hash;
          strcpy (posdtup_rec.file_name,"inds");

          abc_add (posdtup,&posdtup_rec);

          cc = find_rec (inds,&inds_rec,NEXT, "r");
   }

   /* send excf */
   excf_rec.hhcf_hash = 0;

   cc = find_rec (excf,&excf_rec,GTEQ,"r");
   while (cc == 0 && strcmp (excf_rec.co_no,com_no) == 0)
   {
          posdtup_rec.pos_no      = terminal_no;
          posdtup_rec.record_hash = excf_rec.hhcf_hash;
          strcpy (posdtup_rec.file_name,"excf");

          abc_add (posdtup,&posdtup_rec);

          cc = find_rec (excf,&excf_rec,NEXT, "r");
   }

   /* send inum */
   inum_rec.hhum_hash = 0;
   cc = find_rec (inum,&inum_rec,GTEQ,"r");
   while (!cc)
   {
           posdtup_rec.pos_no = terminal_no;
           posdtup_rec.record_hash = inum_rec.hhum_hash;
           strcpy (posdtup_rec.file_name,"inum");

           abc_add (posdtup,&posdtup_rec);
       
           cc = find_rec (inum,&inum_rec,NEXT, "r");

   }

   /* send insf */
   insf_rec.hhwh_hash = 0;
   cc = find_rec (insf,&insf_rec,GTEQ,"r");
   while (!cc)
   {
           posdtup_rec.pos_no = terminal_no;
           posdtup_rec.record_hash = insf_rec.hhbr_hash;
           strcpy (posdtup_rec.file_name,"insf");

           abc_add (posdtup,&posdtup_rec);

           cc = find_rec (insf,&insf_rec,NEXT, "r");
   }

  /* send sokt */
   sokt_rec.hhbr_hash = 0;
   cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
   while (!cc)
   {
          posdtup_rec.pos_no = terminal_no;
          posdtup_rec.record_hash = sokt_rec.hhbr_hash;
          strcpy (posdtup_rec.file_name,"sokt");

          abc_add (posdtup,&posdtup_rec);

          cc = find_rec (sokt,&sokt_rec,NEXT,"r");
   }
  
   cnch_rec.hhch_hash = 0;
   cc = find_rec (cnch, &cnch_rec,GTEQ, "r");
   
   while (!cc)
   {
          posdtup_rec.pos_no = terminal_no;
          posdtup_rec.record_hash = cnch_rec.hhch_hash;
          strcpy (posdtup_rec.file_name,"cnch");

          abc_add (posdtup,&posdtup_rec);
          
          cc = find_rec (cnch, &cnch_rec,NEXT, "r");
   }

   cncd_rec.hhch_hash = 0;
   cc = find_rec (cncd, &cncd_rec, GTEQ, "r");

   while (!cc)
   {
          posdtup_rec.pos_no = terminal_no;
          posdtup_rec.record_hash = cncd_rec.hhch_hash;
          strcpy (posdtup_rec.file_name,"cncd");

          abc_add (posdtup,&posdtup_rec);

          cc = find_rec (cncd, &cncd_rec,NEXT, "r");

   }
   
   return 0;
}

int
GetLostLines ( 
 int sockHandle)
{
        struct TransLostSales *buf;

        memset (buffer,0,sizeof (buffer));

        if (recv (sockHandle,buffer,sizeof (struct TransLostSales),0) != -1)
        {
                buf = (struct TransLostSales *)buffer;
                UpdateInls (buf);
        }
        else
        {
                /*  return -1; */
        }
        send (sockHandle, "eeeee", 5, 0);

		return 0; 
}

/*============================================
| Retrieves message type from message header |
| Each message being preceeded by a message  |
| header.
*===========================================*/
char * 
GetMessage (
 int sockHandle)
{
   int count;
   struct msg_header *head; 

   if (sockHandle < 1)
   {
           return  " ";
   }

   memset (buffer,0,sizeof (buffer));

   count = recv (sockHandle,buffer,sizeof (struct msg_header),0);

   if (count == -1 )
   {
          /* return  " "; */
   }
  
   send (sockHandle, "eeeee", 5, 0);
 
   head = (struct msg_header *)buffer;
   return head->message; 
}

void 
open_db (
 void)
{
   abc_dbopen (data);
   
   open_rec (cohr,cohr_list,COHR_NO_FIELDS, "cohr_id_no2");
   open_rec (coln,coln_list,coln_fields_no,"coln_id_no");
   open_rec (sons,sons_list,sons_fields_no,"sons_id_no4");
   open_rec (inls,inls_list,inls_fields_no,"inls_id_no2");
   open_rec (cudp,cudp_list,cudp_fields_no,"cudp_id_no");
   open_rec (esmr,esmr_list,esmr_fields_no,"esmr_id_no");
   open_rec (ccmr,ccmr_list,ccmr_fields_no,"ccmr_id_no");
   open_rec (incc,incc_list,incc_fields_no,"incc_id_no");
   open_rec (posterm, posterm_list, posterm_fields_no,"pos_no");
   open_rec (sobg,sobg_list,sobg_fields_no,"sobg_id_no");

   open_rec (posdtup,posdtup_list,posdtup_fields_no,"pos_no1");
   open_rec (cumr,cumr_list, cumr_fields_no, "cumr_hhcu_hash");
   open_rec (inmr,inmr_list, inmr_fields_no, "inmr_hhbr_hash");
   open_rec (inpr,inpr_list, inpr_fields_no, "inpr_hhbr_hash");
   open_rec (incp,incp_list, incp_fields_no, "incp_id_no");
   open_rec (inds,inds_list, inds_fields_no, "inds_hhcu_hash");
   open_rec (excf,excf_list, excf_fields_no, "excf_hhcf_hash");
   open_rec (inum,inum_list, inum_fields_no, "inum_hhum_hash");
   open_rec (insf,insf_list, insf_fields_no, "insf_id_no");
   open_rec (sokt,sokt_list, sokt_fields_no, "sokt_hhbr_hash");
   open_rec (cnch,cnch_list, cnch_fields_no, "cnch_hhch_hash");
   open_rec (cncd,cncd_list, cncd_fields_no, "cncd_hhch_hash");
   /*
   open_rec(inbm,inbm_list, inbm_fields_no, "inbm_id_no");
   */
}

void 
close_db (
 void)
{
   abc_fclose (sobg);
   abc_fclose (posterm);
   abc_fclose (incc);
   abc_fclose (ccmr);
   abc_fclose (inls);
   abc_fclose (sons);
   abc_fclose (coln);
   abc_fclose (cohr);
   abc_fclose (cudp);
   abc_fclose (esmr);
   abc_fclose (posdtup);
   abc_fclose (cumr);
   abc_fclose (inmr);
   abc_fclose (inpr);
   abc_fclose (incp);
   abc_fclose (inds);
   abc_fclose (excf);
   abc_fclose (inum);
   abc_fclose (insf);
   abc_fclose (sokt);
   abc_fclose (cnch);
   abc_fclose (cncd);
   /*
   abc_fclose(inbm);
   */
   abc_fclose (data);
}

void 
OpenLogFiles (
 void)
{
   char pos_online [256],
        pos_recv [256],
        /* pos_send [256], */
        pos_log [256];

   sprintf (pos_online,"/tmp/PosStatus/Pos%03d.online",terminal_no);
   sprintf (pos_recv,"/tmp/PosStatus/Pos%03d.recv",terminal_no);
   /* sprintf(pos_send,"/tmp/PosStatus/Pos%03d.send",terminal_no); */
   sprintf (pos_log,
                   "%-s/BIN/LOG/POS/srvdTrans%03d.log",
                   pos_log,
                   terminal_no);

   OnlineTerm = fopen (pos_online,"w");
   RecvTerm   = fopen (pos_recv, "w");
   /* SendTerm   = fopen (pos_send,"w"); */
   
   pipeLog    = fopen (pos_log,"a");

}

void 
CloseLogFiles (
 void)
{
        if (OnlineTerm)
        {
           fclose (OnlineTerm);
        }

        if (RecvTerm)
        {
           fclose (RecvTerm);
        }
    
        /*
        if (SendTerm) 
        {
           fclose (SendTerm);
        }
        */

        if (pipeLog)
        {
           fclose (pipeLog);
        }
}

int 
UpdateStatusLog (
 FILE *fil,
 int set)
{
   if (set)
   {
      /* flockfile (fil); */
   }
   else
   {
          /* funlockfile (fil); */
   }

   return 0;
}

int 
UpdateStatus (
 int term,
 int status,
 int  set)
{
	return 0;
}

int 
UpdateCohr (
 struct TransHeader *header)
{
        int  type_t  = atoi (header->tran_type); 
        char inv_no [9];
        int cc;
    
        strcpy (cohr_rec.pos_inv_no,header->inv_no);
        cohr_rec.pos_tran_type = type_t; /* atoi (header->tran_type); */

        strcpy (inv_no, GetLastInvoiceNo (type_t));
        
        strcpy (cohr_rec.co_no,com_no);
        strcpy (cohr_rec.br_no,bra_no);
        strcpy (cohr_rec.dp_no, " 1");

        if (type_t > 20)
        {
              strcpy (cohr_rec.app_inv_no, GetAppInv (header->hhco_hash));
        }
        else
        {
              strcpy (cohr_rec.app_inv_no, "        ");              
        }

        cohr_rec.hhcu_hash = atol (header->hhcu_hash);

        if (type_t < 20)
        {
           strcpy (cohr_rec.type,"I");
        }
        else
        {
           strcpy (cohr_rec.type, "C");
        }

        strcpy (cohr_rec.inv_no, inv_no);
        /* char cont_no[7]; */
        /* char drop_ship[2]; */
        cohr_rec.hhds_hash = 0L; 
        strcpy (cohr_rec.cus_ord_ref,header->cus_ord_ref); 
        /* char ord_ref[17]; */
        /* char grn_no[21];  */
        /* char cons_no[17]; */
        /* char carr_code[5];*/
        /* char carr_area[3];*/
        /* int no_cartons;   */
        /* double wgt_per_ctn; */
        /* float no_kgs;       */
        /* long hhso_hash;     */
        /* long hhco_hash;     */
        strcpy (cohr_rec.frei_req,"N");
        cohr_rec.date_raised = StringToDate (header->date_raised);
        cohr_rec.date_required = StringToDate (header->date_required);
        /* char tax_code[2]; */
        /* char tax_no[16];  */
        /* char area_code[3];*/
        /* char sale_code[3];*/
        strcpy (cohr_rec.sale_code, header->sale_code);
        strcpy (cohr_rec.area_code, header->area_code);

        strcpy (cohr_rec.op_id,header->op_id);
        strcpy (cohr_rec.time_create,header->time_create);
        cohr_rec.date_create = StringToDate (header->date_create);
        cohr_rec.gross = atof (header->gross);
        /* double freight;      */
        /* double insurance;    */
        /* double other_cost_1; */
        /* double other_cost_2; */
        /* double other_cost_3; */
        cohr_rec.tax  = atof (header->tax);
        cohr_rec.gst  = atof (header->gst);
        cohr_rec.disc = atof (header->disc);
        /* double deposit;   */
        /* double ex_disc;   */
        /* double erate_var; */
        /* double sos;       */
        cohr_rec.exch_rate = 1.0;     
        strcpy (cohr_rec.fix_exch,"Y" );     
        strcpy (cohr_rec.batch_no,"0000");    

        cumr_rec.hhcu_hash = atol (header->hhcu_hash);
 
        cc = find_rec (cumr,&cumr_rec,EQUAL,"r");

        if (!cc)
        {
                strcpy (cohr_rec.dl_name, cumr_rec.dbt_name);
                strcpy (cohr_rec.dl_add1, cumr_rec.ch_adr1);
                strcpy (cohr_rec.dl_add2, cumr_rec.ch_adr2);
                strcpy (cohr_rec.dl_add3, cumr_rec.ch_adr3);
        }

        /* char din_1[61];      */ 
        /* char din_2[61];      */ 
        /* char din_3[61];      */ 
        strcpy (cohr_rec.pay_terms,"0 ");
        /* char sell_terms[4];  */ 
        /* char ins_det[31];    */ 
        strcpy (cohr_rec.pri_type, header->pri_type) ;     
        /* char pri_break[2];   */ 
        strcpy (cohr_rec.ord_type,"D"); 
        /* char prt_price[2];   */ 
        /* char status[2];      */
        strcpy (cohr_rec.stat_flag,"X");   
        strcpy (cohr_rec.ps_print,"N");    
        strcpy (cohr_rec.inv_print,"Y");   
        /* char ccn_print[2];   */ 
        /* char printing[2];    */
        /* long hhtr_hash;      */
        /* char load_flag[2];   */

        cc = abc_add (cohr,&cohr_rec);

        if (cc)
        {
           printf ("\nCohr add failed on invoice= %s",inv_no);   
        }
        else
        {
                /*-----------------------------------------------------------
                | this will cause the hhco_hash to be updated into cohr_rec |
                | for use with UpdateColn and others                        |
                -----------------------------------------------------------*/
                cc = find_rec (cohr,&cohr_rec,EQUAL,"r");
                if (!cc)
                {
                     printf ("\nCohr add with hhco_hash = %ld",
                             cohr_rec.hhco_hash); 
                }

        }

		return 0;
}

int 
UpdateColn(
 struct TransLine  *lines)
{
        int cc;

        coln_rec.hhcl_hash = atol (lines->hhcl_hash);
        coln_rec.hhco_hash = cohr_rec.hhco_hash;
        printf ("\n%ld",cohr_rec.hhco_hash);
        coln_rec.line_no   = atoi (lines->line_no) -1;
        coln_rec.hhbr_hash = atol (lines->hhbr_hash)   ;
        coln_rec.incc_hash = GetIncc();
        coln_rec.hhum_hash = atol (lines->hhum_hash );
        /* long hhsl_hash; */
        /* long hhdl_hash; */
        strcpy (coln_rec.crd_type, lines->crd_type);
        strcpy (coln_rec.serial_no,lines->serial_no);
        /* int cont_status; */
        coln_rec.qty_org_ord = atof (lines->qty_org_ord);
        coln_rec.q_order = atof (lines->q_order);
        /* float qty_del; */
        /* float qty_ret; */
        /* float q_backorder; */
        coln_rec.gsale_price = atof (lines->gsale_price);
        coln_rec.sale_price  = atof (lines->sale_price);
        /* cost_price; */
        coln_rec.disc_pc    = atof (lines->disc_pc);
        coln_rec.reg_pc     = atof (lines->reg_pc);
        coln_rec.disc_a     = atof (lines->disc_a);
        coln_rec.disc_b     = atof (lines->disc_b);
        coln_rec.disc_c     = atof (lines->disc_c);
        coln_rec.cumulative = atoi (lines->cumulative);
        coln_rec.tax_pc     = atof (lines->tax_pc );
        coln_rec.gst_pc     = atof (lines->gst_pc );
        coln_rec.gross      = atof (lines->gross );
        /* double freight;                   */
        /* double on_cost;                   */
        coln_rec.amt_disc   = atof (lines->amt_disc);
        coln_rec.amt_tax    = atof (lines->amt_tax );
        coln_rec.amt_gst    = atof (lines->amt_gst);
        /* double erate_var;   */
        /* char pack_size[6];  */
        /* char sman_code[3];  */
        strcpy (coln_rec.sman_code,cohr_rec.sale_code);

        strcpy (coln_rec.cus_ord_ref,lines->cus_ord_ref);
        /* float o_xrate;      */
        /* float n_xrate;      */
        strcpy (coln_rec.item_desc,lines->item_desc);
        /* long due_date;      */
        coln_rec.status[0] = '7';     
        strcpy(coln_rec.bonus_flag, lines->bonus_flag);
        /* char hide_flag[2];  */
        /* long hhah_hash;     */
        coln_rec.stat_flag[0] = 'X';
        
        cc = abc_add (coln,&coln_rec);
   
        if (cc)
        {
           printf ("\nColn add failed\n");
        }
        else
        {
           printf ("\nColn added\n");
           UpdateInventory (coln_rec.hhbr_hash, coln_rec.incc_hash);
        }

 	return 0;
}

int 
UpdateSons( 
 struct TransNotes *notes)
{
   int cc;

   sons_rec.hhco_hash = cohr_rec.hhco_hash;
   sons_rec.hhcl_hash = atol (notes->hhcl_hash);
   sons_rec.line_no   = atoi (notes->line_no);
   strcpy (sons_rec.desc, notes->desc) ;

   cc = abc_add (sons,&sons_rec);

   if (cc)
   {
          printf ("\nSons add failed\n");  
   }
   else
   {
          printf ("\nSons add success");
   }

   return 0;
}

int 
UpdateInls (
 struct TransLostSales *lost)
{
    int cc;
    
    strcpy (inls_rec.co_no, com_no);
    strcpy (inls_rec.est_no, bra_no);
    inls_rec.date = atol (lost->date);
    inls_rec.hhbr_hash = atol (lost->hhbr_hash);
    inls_rec.hhcu_hash = atol (lost->hhcu_hash);
    inls_rec.qty = atof (lost->qty);
    inls_rec.value = atof (lost->value);
    inls_rec.hhcc_hash = GetIncc ();
    strcpy (inls_rec.res_code, lost->res_code);
    strcpy (inls_rec.res_desc, lost->res_desc);

    cc = abc_add (inls, &inls_rec);

    if (!cc)
    {
       printf ("\nInls OK");
    }
    else
    {
    }

	return 0;
}

void 
GetCompany (
 int term, 
 char *co, 
 char *br,
 char *wr,
 char *usr)
{
    int cc;
    long now = TodaysDate ();

    now = time (&now);

    posterm_rec.pos_no = term;

    cc = find_rec (posterm,&posterm_rec,EQUAL,"u");

    if (!cc)
    {
       
           posterm_rec.last_login = now;
           strcpy (posterm_rec.last_user  , usr);
           
           abc_update (posterm,&posterm_rec);

           strcpy (co,posterm_rec.co_no);
           strcpy (br,posterm_rec.br_no);
           strcpy (wr,posterm_rec.wh_no);
    }
    else
    {
           strcpy (co,"  ");
           strcpy (br,"  ");
           strcpy (wr,"  ");
    }
}

char *
GetLastInvoiceNo (
 int type_t)
{
   int cc;
   static char rTemp[9];
   int numb  = atoi (getenv ("SO_NUMBERS")); 
   
   if (numb == 1)
   {
           strcpy (esmr_rec.co_no,com_no);
           strcpy (esmr_rec.est_no,bra_no);
           cc = find_rec (esmr,&esmr_rec,EQUAL,"u");    
           if (!cc)
           {
                   if (type_t < 20)
                   {
                       lclip (clip (esmr_rec.csh_pref));
                       sprintf (rTemp,
                           "%s%06ld",
                           esmr_rec.csh_pref,
                           esmr_rec.nx_csh_inv+1);
                   
                        esmr_rec.nx_csh_inv++;
                   }
                   else
                   {
                      lclip (clip (esmr_rec.crd_pref));
                      sprintf (rTemp, "%s%06ld",
                          esmr_rec.crd_pref,
                          esmr_rec.nx_csh_crd+1);

                      esmr_rec.nx_csh_crd++;
                   }

                   abc_update (esmr,&esmr_rec);
           }
           else
           {
                   sprintf (rTemp,"%s"," ");
           }
   }
   else
   {
           strcpy (cudp_rec.co_no,com_no);
           strcpy (cudp_rec.br_no,bra_no);
           strcpy (cudp_rec.dp_no,war_no);

           cc = find_rec (cudp,&cudp_rec,EQUAL,"r");
           if (!cc)
           {
                   
                   if (type_t < 20)
                   {
                       lclip (clip (cudp_rec.csh_pref));
                       sprintf (rTemp,
                           "%s%06ld",
                           cudp_rec.csh_pref,
                           cudp_rec.nx_csh_no+1);
                   
                        cudp_rec.nx_csh_no++;
                   }
                   else
                   {
                      lclip (clip (cudp_rec.crd_pref));
                      sprintf (rTemp, "%s%06ld",
                          cudp_rec.crd_pref,
                          cudp_rec.nx_crd_no+1);

                      cudp_rec.nx_crd_no++;
                   }

                   abc_update (cudp,&cudp_rec);
           }
           else
           {
              sprintf (rTemp,"%s"," ");
           }
   }

   return rTemp;

}

long 
GetIncc (
 void)
{
    int cc;
    long hash;

    strcpy (ccmr_rec.co_no,com_no);
    strcpy (ccmr_rec.est_no,bra_no);
    strcpy (ccmr_rec.cc_no,war_no);
    
    cc = find_rec (ccmr,&ccmr_rec,EQUAL,"r");
    if (!cc)
    {
        hash = ccmr_rec.hhcc_hash;
    }
    else
    {
        hash = 0l;
    }
    
    return hash; 
}


void 
UpdateInventory (
 long hhbr,
 long incc)
{
    add_hash (com_no,bra_no,"SU", 0,hhbr,incc,0l, 0.00);
    recalc_sobg ();

}


void 
term_srvd ( 
 int sig)
{
   printf ("\nProgram interrupted...  aborted.");
   close (transmitSocket);
   close (drlSocket);

   exit (0);
}

void 
ChildEnds (
 int sig)
{
  printf ("\nChild ends");
}

void 
SendInmr (
 void)
{
   int cc;

   /* send inmr */
   inmr_rec.hhbr_hash = 0L;
   cc = find_rec (inmr,&inmr_rec,GTEQ,"r");
   while (!cc)
   {
            if (strcmp (inmr_rec.co_no, com_no) == 0)
            {
                 posdtup_rec.pos_no = terminal_no;
                 posdtup_rec.record_hash = inmr_rec.hhbr_hash;
                 strcpy (posdtup_rec.file_name,"inmr");
                 
                 abc_add (posdtup,&posdtup_rec);
            }
  
     
            cc = find_rec (inmr,&inmr_rec,NEXT,"r");
   }
  
   /* send cumr */
}

void 
SendInpr (
 void)
{
   int cc;

   /* send inpr */
   inpr_rec.hhbr_hash = 0;
   cc = find_rec (inpr,&inpr_rec,GTEQ, "r");
   while (!cc)
   {
          posdtup_rec.pos_no = terminal_no;
          posdtup_rec.record_hash = inpr_rec.hhbr_hash;
          strcpy (posdtup_rec.file_name,"inpr");

          abc_add (posdtup,&posdtup_rec);

          cc = find_rec (inpr,&inpr_rec,NEXT,"r");

   }
   
}

void 
SendCumr (
 void)
{
   int cc;

   /* send cumr */
   cumr_rec.hhcu_hash = 0;
   cc = find_rec (cumr,&cumr_rec,GTEQ,"r");
   while (!cc)
   {
           if (strcmp (cumr_rec.co_no, com_no) == 0)
           {
	           posdtup_rec.pos_no = terminal_no;
    	       posdtup_rec.record_hash = cumr_rec.hhcu_hash;
        	   strcpy (posdtup_rec.file_name,"cumr");

 	          abc_add (posdtup,&posdtup_rec);
           }
           cc = find_rec (cumr,&cumr_rec,NEXT, "r");
   }

   /* send inpr */
}

void 
SendInum (
 void)
{
   int cc;

   /* send inum */
   inum_rec.hhum_hash = 0;
   cc = find_rec(inum,&inum_rec,GTEQ,"r");
   while (!cc)
   {
           posdtup_rec.pos_no = terminal_no;
           posdtup_rec.record_hash = inum_rec.hhum_hash;
           strcpy (posdtup_rec.file_name,"inum");

           abc_add (posdtup,&posdtup_rec);
       
           cc = find_rec (inum,&inum_rec,NEXT, "r");

   }

}


void 
SendInds (
 void)
{
   int cc;

   /* send inds */
   inds_rec.hhcu_hash = 0;
   cc = find_rec (inds,&inds_rec,GTEQ,"r");
   while (!cc)
   {
          posdtup_rec.pos_no = terminal_no;
          posdtup_rec.record_hash = inds_rec.hhbr_hash;
          strcpy (posdtup_rec.file_name,"inds");

          abc_add (posdtup,&posdtup_rec);

          cc = find_rec (inds,&inds_rec,NEXT, "r");
   }

}

char *
GetAppInv (
 char *oldInv)
{
    static char prevInv[9];
    char *inv = oldInv;
    int  cc;

    inv[0] = '0';

    abc_selfield (cohr, "cohr_pos_id");
    
	strcpy (cohr_rec.co_no, com_no);
    strcpy (cohr_rec.pos_inv_no, inv);

    cc = find_rec (cohr, &cohr_rec, EQUAL, "r");

    if (!cc)
    {
       strcpy (prevInv, cohr_rec.inv_no);
    }
    else
    {
       strcpy (prevInv, " ");
    }

    abc_selfield (cohr, "cohr_id_no2");

    return prevInv;

}
