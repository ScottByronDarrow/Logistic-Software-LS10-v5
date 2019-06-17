/*=============================================================================
|  Copyright (C) 1996 - 1999 SOFTWARE ENGINEERING LIMITED.                    |
|=============================================================================|
| Program name    :  ( pos_upload.c              )                            |
| Program desc    :  ( POS pipe                  )                            |
|-----------------------------------------------------------------------------|
| Author          :  Primo O. Esteria          : Date written  Sept 2, 1998   |
|-----------------------------------------------------------------------------|
| Modified by     :  Primo O. Esteria          :                              |
|-----------------------------------------------------------------------------|
| $Log: pos_upload.c,v $
| Revision 5.2  2002/11/28 04:09:48  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.1  2001/08/06 23:36:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:42  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:50  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/11/19 06:24:24  scott
| Updated for warning errors.
|
| Revision 1.7  1999/10/16 01:49:20  scott
| Updated from ansi
|
| Revision 1.6  1999/06/18 02:05:28  scott
| Updated for log.
|
==============================================================================
*/
#define CCMAIN

char *PNAME        = "pos_upload.c";
char *PROG_VERSION      = "@(#) - (Logistic - 9.62) 98/11/15 16:00:00 ";

#include <pslscr.h>

#include <alarm_time.h> 
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <messages.h>

#define                  MAX_DELAY      2


struct sockaddr_in       localAddr,
                         serverAddr;

int 					 serverSocket;

struct msg_header        mess_head;
char                     com_no[3],
                         bra_no[3],
                         war_no[3];

struct               
{
   int pos_no;
   char filename[11];
   long hash;
} posdt_recx [65536];  /* this may be too little */ 

int                  max_recx;
int                  num_terminals;
FILE                 *SendLog;
char                 SendPath[256];
int                  onlineHandle;


int              terminal_no;
int              port_no;
int              socketHandle;


char            *posterm = "posterm",
                *posdtup = "posdtup",
                *cumr    = "cumr",
                *inmr    = "inmr",
                *incp    = "incp",
                *inpr    = "inpr",
                *inds    = "inds",
                *excf    = "excf",
                *inum    = "inum",
                *insf    = "insf",
                *sokt    = "sokt",
                *cnch    = "cnch",
                *cncd    = "cncd",
                *inbm    = "inbm";

struct dbview posterm_list [] =
{
   { "pos_no" },
   { "ip_address" },
   { "co_no"},
   { "br_no"},
   { "wh_no"}

};

int posterm_fields_no = 5;

struct  
{
   int    pos_no;
   char   ip_address[16];
   char   co_no[3];
   char   br_no[3];
   char   wh_no[3];
} posterm_rec;

struct
{
   int   pos_no;
   char  ip_address[16];
   char  co_no[3];
   char  br_no[3];
   char  wh_no[3];

} terminals [MAX_TERMINALS];

struct dbview posdtup_list [] = 
{
   { "pos_no" },
   { "file_name" },
   { "record_hash" },
   { "action" }
};

int posdtup_fields_no = 4;

struct
{
   int pos_no;
   char file_name[11];
   long record_hash;
   char action[2];

} posdtup_rec;

/* ================================== */

struct dbview cumr_list [] =
{
   {"cumr_dbt_no"},
   {"cumr_hhcu_hash"},
   {"cumr_dbt_name"},
   {"cumr_class_type"},
   {"cumr_price_type"},
   {"cumr_ch_adr1"},
   {"cumr_ch_adr2"},
   {"cumr_ch_adr3"},
   {"cumr_ch_adr4"},
   {"cumr_contact_name"},
   {"cumr_phone_no"},
   {"cumr_ord_value"},
   {"cumr_bo_current"},
   {"cumr_bo_per1"},
   {"cumr_bo_per2"},
   {"cumr_bo_per3"},
   {"cumr_bo_per4"},
   {"cumr_bo_fwd"},
   {"cumr_od_flag"}
};

int cumr_fields_no = 19;


struct 
{
   char   dbt_no[7]; 
   long   hhcu_hash;
   char   dbt_name[41];
   char   class_type[4];
   char   price_type[2];
   char   ch_adr1[41];
   char   ch_adr2[41];
   char   ch_adr3[41];
   char   ch_adr4[41];
   char   contact_name[21];
   char   phone_no[16];
   Money  ord_value;
   Money  bo_current;
   Money  bo_per1;
   Money  bo_per2;
   Money  bo_per3;
   Money  bo_per4;
   Money  bo_fwd;
   int    od_flag;
} cumr_rec;

struct dbview inmr_list[] =
{
  { "inmr_item_no"},
  { "inmr_hhbr_hash"},
  { "inmr_alpha_code"},
  { "inmr_maker_no"},
  { "inmr_alternate"},
  { "inmr_barcode"},
  { "inmr_class"},
  { "inmr_category"},
  { "inmr_description"},
  { "inmr_quick_code"},
  { "inmr_sellgrp"},
  { "inmr_buygrp"},
  { "inmr_disc_pc"},
  { "inmr_gst_pc"},
  { "inmr_tax_pc"},
  { "inmr_std_uom"},
  { "inmr_alt_uom"},
  { "inmr_uom_cfactor"},
  { "inmr_outer_uom"},
  { "inmr_outer_size"},
  { "inmr_min_sell_price"}
};

int inmr_fields_no = 21; 

struct 
{
   char   item_no[17];
   long   hhbr_hash;
   char   alpha_code[17];
   char   maker_no[17];
   char   alternate[17];
   char   barcode[17];
   char   class[2];
   char   category[12];
   char   description[41];
   char   quick_code[9];
   char   sellgrp[7];
   char   buygrp[7];
   float  disc_pc;
   float  gst_pc;
   float  tax_pc;
   long   std_uom;
   long   alt_uom; 
   float  uom_cfactor;
   long   outer_uom;
   float  outer_size;
   Money  min_sell_price;
} inmr_rec;

struct dbview inpr_list [] =
{
  { "inpr_br_no" },
  { "inpr_wh_no" },
  { "inpr_hhbr_hash"},
  { "inpr_price_type"},
  { "inpr_curr_code"},
  { "inpr_cust_type"},
  { "inpr_hhgu_hash"},
  { "inpr_price_by"},
  { "inpr_qty_brk1"}, 
  { "inpr_qty_brk2"},
  { "inpr_qty_brk3"},
  { "inpr_qty_brk4"},
  { "inpr_qty_brk5"},
  { "inpr_qty_brk6"},
  { "inpr_qty_brk7"},
  { "inpr_qty_brk8"},
  { "inpr_qty_brk9"},
  { "inpr_base"},
  { "inpr_price1"},
  { "inpr_price2"},
  { "inpr_price3"},
  { "inpr_price4"},
  { "inpr_price5"},
  { "inpr_price6"},
  { "inpr_price7"},
  { "inpr_price8"},
  { "inpr_price9"}
};

int inpr_fields_no = 27;

struct 
{
   char    br_no[3];
   char    wh_no[3];
   long    hhbr_hash;
   int     price_type;
   char    curr_code[4];
   char    cust_type[4];
   long    hhgu_hash;
   char    price_by[2];
   double  qty_brk1;
   double  qty_brk2;
   double  qty_brk3;
   double  qty_brk4;
   double  qty_brk5;
   double  qty_brk6;
   double  qty_brk7;
   double  qty_brk8;
   double  qty_brk9;
   Money   base;
   Money   price1;
   Money   price2;
   Money   price3;
   Money   price4;
   Money   price5;
   Money   price6;
   Money   price7;
   Money   price8;
   Money   price9;
} inpr_rec;

struct dbview incp_list[] = 
{
   { "incp_key"},
   { "incp_hhcu_hash" },
   { "incp_area_code"},
   { "incp_cus_type"},
   { "incp_hhbr_hash"},
   { "incp_curr_code"},
   { "incp_status"},
   { "incp_date_from"},
   { "incp_date_to"},
   { "incp_price1"},
   { "incp_price2"},
   { "incp_price3"},
   { "incp_price4"},
   { "incp_price5"},
   { "incp_price6"},
   { "incp_price7"},
   { "incp_price8"},
   { "incp_price9"},
   { "incp_comment"}, 
   { "incp_dis_allow"},
   { "incp_stat_flag"}
};

int incp_fields_no = 21;
 
struct 
{
  char     key[7];
  long     hhcu_hash;
  char     area_code[3];
  char     cus_type[4];
  long     hhbr_hash;
  char     curr_code[4];
  char     status[2];
  long     date_from;
  long     date_to;
  Money    price1;
  Money    price2;
  Money   price3;
  Money   price4;
  Money   price5;
  Money   price6;
  Money   price7;
  Money   price8;
  Money   price9;
  char     comment[41];
  char     dis_allow[2];
  char     stat_flag[2];
} incp_rec;

struct dbview inds_list[] = 
{
         { "inds_hhcu_hash"},
         { "inds_price_type"},
         { "inds_category"},
         { "inds_sel_group"},
         { "inds_cust_type"},
         { "inds_hhbr_hash"},
         { "inds_disc_by"},
         { "inds_qty_brk1"},
         { "inds_qty_brk2"},
         { "inds_qty_brk3"},
         { "inds_qty_brk4"},
         { "inds_qty_brk5"},
         { "inds_qty_brk6"},
         { "inds_disca_pc1"},
         { "inds_disca_pc2"},
         { "inds_disca_pc3"},
         { "inds_disca_pc4"},
         { "inds_disca_pc5"},
         { "inds_disca_pc6"},
         { "inds_discb_pc1"},
         { "inds_discb_pc2"},
         { "inds_discb_pc3"},
         { "inds_discb_pc4"},
         { "inds_discb_pc5"},
         { "inds_discb_pc6"},
         { "inds_discc_pc1"},
         { "inds_discc_pc2"},
         { "inds_discc_pc3"},
         { "inds_discc_pc4"},
         { "inds_discc_pc5"},
         { "inds_discc_pc6"},
         { "inds_cum_disc"}
};

int inds_fields_no = 32;

struct
{
   long   hhcu_hash;
   int    price_type;
   char   category[12];
   char   sel_group[7];
   char   cust_type[4];
   long   hhbr_hash;
   char   disc_by[2];
   double qty_brk1;
   double qty_brk2;
   double qty_brk3;
   double qty_brk4;
   double qty_brk5;
   double qty_brk6;
   float  disca_pc1;
   float  disca_pc2;
   float  disca_pc3;
   float  disca_pc4;
   float  disca_pc5;
   float  disca_pc6;
   float  discb_pc1;
   float  discb_pc2;
   float  discb_pc3;
   float  discb_pc4;
   float  discb_pc5;
   float  discb_pc6;
   float  discc_pc1;
   float  discc_pc2;
   float  discc_pc3;
   float  discc_pc4;
   float  discc_pc5;
   float  discc_pc6;
   char   cum_disc[2];
} inds_rec;

struct dbview excf_list[] =
{
   { "excf_co_no" },
   { "excf_cat_no"},
   { "excf_cat_desc"},
   { "excf_max_disc"},
   { "excf_min_marg"},
   { "excf_hhcf_hash" }

};

int excf_fields_no = 6;

struct 
{
   char   co_no[3];
   char   cat_no[12];
   char   cat_desc[41];
   float  max_disc;
   float  min_marg;
   long   hhcf_hash;

} excf_rec;

struct dbview inum_list[] = 
{
   { "inum_uom_group"},
   { "inum_hhum_hash"},
   { "inum_uom"},
   { "inum_desc"},
   { "inum_cnv_fct"}
};

int inum_fields_no = 5;

struct 
{
   char   uom_group[21];
   long   hhum_hash;
   char   uom[5];
   char   desc[41];
   float  cnv_fct;
} inum_rec; 

struct dbview insf_list [] =
{
  { "insf_hhwh_hash"},
  { "insf_hhbr_hash"},
  { "insf_serial_no"},
  { "insf_status"}
};  

int insf_fields_no = 4;

struct 
{
  long hhwh_hash;
  long hhbr_hash;
  char serial_no[26];
  char status[2];
} insf_rec;

struct dbview sokt_list [] =
{
   { "sokt_hhbr_hash" },
   { "sokt_line_no"},
   { "sokt_mabr_hash"},
   { "sokt_matl_qty"},
   { "sokt_due_date"},
   { "sokt_bonus"}
};

int sokt_fields_no = 6;

struct
{
  long  hhbr_hash;
  int   line_no;
  long  mabr_hash;
  float matl_qty;
  long  due_date;
  char  bonus[2];
} sokt_rec;

struct dbview cnch_list [] =
{
  { "cnch_cont_no" },
  { "cnch_hhch_hash" },
  { "cnch_desc"},
  { "cnch_contact" },
  { "cnch_date_wef"},
  { "cnch_date_rev"},
  { "cnch_date_ren"},
  { "cnch_exch_type"}
};

int cnch_fields_no = 8;

struct
{
  char cont_no[7];
  long hhch_hash;
  char desc[41];
  char contact[21];
  long date_wef;
  long date_rev;
  long date_ren;
  char exch_type[2];
} cnch_rec;


 struct dbview cncd_list [] =
 {
   { "cncd_hhch_hash" },
   { "cncd_line_no"},
   { "cncd_hhbr_hash"},
   { "cncd_hhcc_hash"},
   { "cncd_hhsu_hash"},
   { "cncd_hhcl_hash"},
   { "cncd_price"},
   { "cncd_curr_code"},
   { "cncd_disc_ok"},
   { "cncd_cost"}
 };

 int cncd_fields_no = 10;

 struct
 {
        long hhch_hash;
        int line_no;
        long hhbr_hash;
        long hhcc_hash;
        long hhsu_hash;
        long hhcl_hash;
        double price;
        char curr_code[4];
        char disc_ok[2];
        double cost;
 } cncd_rec;

struct dbview inbm_list [] =
{
  { "inbm_co_no"},
  { "inbm_barcode"},
  { "inbm_item_no"},
  { "inbm_uom"}
};

int inbm_fields_no = 4;

struct
{
   char co_no[3];
   char barcode[17];
   char item_no[17];
   char uom[5];
} inbm_rec;

/* ================================ */
int    main (int, char *argc[]);
int    InitiateConnection (int ,const char *);
int    CheckTables (void);
void   open_db (void);
void   close_db (void);
int    GetRecord (char *,long, int);
int    GetCumr (long, int);
int    GetInmr (long, int);
int    GetIncp (long, int);
int    GetInpr (long, int);
int    GetInds (long, int);
int    GetExcf (long, int);
int    GetInum (long, int);
int    GetInsf (long, int);
int    GetSokt (long, int);
int    GetCnch (long, int);
int    GetCncd (long, int);
int    SendHeader (char *, int);
int    UpdatePosdtup(long hash, char *file, int );
void   ChildEnds(int);


int 
main(
 int  argc, 
 char *argv[])
{
    char ip_address [20];
    int i;
    int lockHandle; 
    char temp[20];

    if (argc < 3)
    {
        printf ("This program is not run on command line\n");
        return 0;
    }
    
    /* Initialize some globals passed from the parent process
       for this POS terminal
    */
 
    terminal_no = atoi (argv [1]);
    strcpy (ip_address, argv [2]);
    strcpy (com_no, argv [3]);
    strcpy (bra_no, argv [4]);
    strcpy (war_no, argv [5]);
    
    sprintf (temp,"/tmp/PosStatus/POSlock%d",terminal_no);
    lockHandle = open (temp, O_RDWR);
    
    lockf (lockHandle, F_LOCK, 0);
     
    open_db ();

    while (1)
    {
         /* branchNolish connection */
         if (InitiateConnection (terminal_no, ip_address) == 1)
         {
             while (1)
             {
                /* Retrieve record for terminal_no */
                if (CheckTables () == 0)
                {
                    time_out();
                    continue;
                }


                for (i = 0; i < max_recx ; i++)
                {
                     print_at (terminal_no,45, "%s-%d-%d",
                          posdt_recx [i].filename,
                          posdt_recx [i].pos_no,
                          posdt_recx [i].hash);
 
                     if (GetRecord (posdt_recx[i].filename,
                                posdt_recx[i].hash,terminal_no) == -1)
                     {
                           break;
                     };
                }
             }

             close (socketHandle );
         }
         else
         {
            sleep (sleepTime);
         }
    }
     
    close (lockHandle );

    close_db();

	return EXIT_SUCCESS;

}

int 
InitiateConnection(
 int   terminal,
 const char  *ipad)
{
    struct servent    *serv_ent;
    char   buf[15];
	long	inet_addr (const char *);

    socketHandle = socket(AF_INET,SOCK_STREAM,0);
    
    memset(&localAddr,0,sizeof(struct sockaddr));

    localAddr.sin_family = AF_INET;
    localAddr.sin_port   = 0;
    localAddr.sin_addr.s_addr = 0;

    /* Retrieve /etc/services entry */

    sprintf (buf,"sel-term%d",terminal);  
    serv_ent    = getservbyname(buf,"tcp");
    if (serv_ent == 0)
    {
          close (socketHandle);
          return 0;
    }

    memset (&serverAddr,0,sizeof(struct sockaddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = serv_ent->s_port;
    serverAddr.sin_addr.s_addr = inet_addr(ipad); 
   
    if (connect (serverSocket, 
                (struct sockaddr *)&serverAddr ,
                 sizeof (struct sockaddr)) != 0)
    {
        /* print_at (terminal,1,"Term %d connect failed",terminal); */ 
        close (serverSocket);
        serverSocket  = 0;
        return 0;
    }

    return 1;
}


int
CheckTables (
 void)
{
    int cc;
    int i=0;
    int j;

    posdtup_rec.pos_no = 0;

    strcpy (posdtup_rec.file_name,"      ");
           posdtup_rec.record_hash = 0; 

    cc = find_rec (posdtup,&posdtup_rec,GTEQ ,"r");
    
    while (!cc)
    {
           if (posdtup_rec.pos_no == 0)
           {
                abc_delete (posdtup);
                
                /* add record to all terminals */
                for (j = 0; j < 4; j++)  /* fixed for sprint cass only */
                {
                     posdtup_rec.pos_no = j+1;
                     abc_add (posdtup, &posdtup_rec);
                }

           }
           /* Only add record belonging to this terminal */
           else if (posdtup_rec.pos_no == terminal_no)
           {
               if (posdtup_rec.record_hash > 0)
               {
              	  strcpy (posdt_recx[i].filename,posdtup_rec.file_name);
               	  posdt_recx [i].hash   = posdtup_rec.record_hash;
                  posdt_recx [i].pos_no = posdtup_rec.pos_no;

                  i++;
               }
               /* delete invalid records */
               else
  			   { 
                   abc_delete (posdtup);   
  			   }
           }      

           /* Process this much record at a time */
           if ( i >= 2500)
           {
                break;
           }

           cc = find_rec (posdtup,&posdtup_rec, NEXT, "r"); 
    }
        
    max_recx = i;

    /* return 0 if no record belong to this terminal_no */
    if (i == 0)
    {
          return 0;
    }

    /* record found */
    return 1;

}

void 
open_db (
 void)
{
   abc_dbopen ("data");

   open_rec (posdtup,posdtup_list,posdtup_fields_no,"posterm_id_no") ;
   open_rec (cumr,cumr_list, cumr_fields_no, "cumr_hhcu_hash");
   open_rec (inmr,inmr_list, inmr_fields_no, "inmr_hhbr_hash");  
   open_rec (inpr,inpr_list, inpr_fields_no, "inpr_id_no");
   open_rec (incp,incp_list, incp_fields_no, "incp_id_no");
   open_rec (inds,inds_list, inds_fields_no, "inds_hhcu_hash");   
   open_rec (excf,excf_list, excf_fields_no, "excf_hhcf_hash");
   open_rec (inum,inum_list, inum_fields_no, "inum_hhum_hash");
   open_rec (insf,insf_list, insf_fields_no, "insf_id_no");
   open_rec (sokt,sokt_list, sokt_fields_no, "sokt_hhbr_hash");
   open_rec (cnch,cnch_list, cnch_fields_no, "cnch_hhch_hash");
   open_rec (cncd,cncd_list, cncd_fields_no, "cncd_id_no");
   open_rec (inbm,inbm_list, inbm_fields_no, "inbm_id_no2");
}

void 
close_db (
 void)
{
   abc_fclose (posdtup);
   abc_fclose (cncd);
   abc_fclose (cnch);
   abc_fclose (sokt);
   abc_fclose (insf);
   abc_fclose (inum); 
   abc_fclose (excf);
   abc_fclose (inds);
   abc_fclose (incp);
   abc_fclose (inpr);
   abc_fclose (inmr);
   abc_fclose (cumr);
   abc_fclose (inbm);
   abc_fclose ("data");
}

int
GetRecord (
 char *file,
 long hash,
 int terminal)
{
     clip (file);

     if (!strcmp (file,"inmr"))
     { 
          return GetInmr (hash,terminal);
     }
     else if (!strcmp (file,"cumr"))
     {
          return GetCumr (hash,terminal);
     }
     else if (!strcmp (file,"inpr"))
     {
          return GetInpr (hash, terminal);
     }
     else if (!strcmp (file,"incp"))
     {
          return GetIncp (hash, terminal);
     }
     else if (!strcmp (file,"inds"))
     {
          return GetInds (hash, terminal );
     }
     else if (!strcmp (file,"excf"))
     {
          return GetExcf (hash, terminal);
     }
     else if (!strcmp (file,"inum"))
     {
          return GetInum (hash, terminal);
     }
     else if (!strcmp (file,"insf"))
     {
          return GetInsf (hash, terminal);
     }
     else if (!strcmp (file,"sokt"))
     {
          return GetSokt (hash, terminal);
     }
     else if (!strcmp (file ,"cnch"))
     {
          return GetCnch (hash, terminal);
     }
     else if (!strcmp (file, "cncd"))
     {
          return GetCncd (hash, terminal);
     }

	 return 0;
}

int
GetInmr (
 long hash,
 int terminal)
{
        int cc;
        struct inmr_record rec;
        struct inbm_record rec2;
        char   temp[6];
    
        inmr_rec.hhbr_hash = hash;

        cc = find_rec (inmr,&inmr_rec,EQUAL,"r");

        if (!cc)
        {
                strcpy (rec.item_no,      inmr_rec.item_no);
                sprintf (rec.hhbr_hash,   "%ld",inmr_rec.hhbr_hash);
                strcpy (rec.alpha_code ,  inmr_rec.alpha_code);
                strcpy (rec.maker_no   ,  inmr_rec.maker_no);
                strcpy (rec.alternate  ,  inmr_rec.alternate);
                strcpy (rec.barcode    ,  inmr_rec.barcode);
                strcpy (rec._class     ,  inmr_rec.class);
                strcpy (rec.category   ,  inmr_rec.category);
                strcpy (rec.description,  inmr_rec.description);
                strcpy (rec.quick_code ,  inmr_rec.quick_code);
                strcpy (rec.sellgrp    ,  inmr_rec.sellgrp);
                strcpy (rec.buygrp     ,  inmr_rec.buygrp);
                sprintf (rec.disc_pc,    "%12.2f",inmr_rec.disc_pc);
                sprintf (rec.gst_pc ,    "%12.2f",inmr_rec.gst_pc);
                sprintf (rec.tax_pc ,    "%12.2f",inmr_rec.tax_pc);
                sprintf (rec.std_uom,    "%ld",inmr_rec.std_uom);
                sprintf (rec.alt_uom,    "%ld", inmr_rec.alt_uom);
                sprintf (rec.uom_cfactor,"%12.2f",inmr_rec.uom_cfactor);
                sprintf (rec.outer_size, "%12.2f" ,inmr_rec.outer_size);
                sprintf (rec.min_sell_price,"%12.2f",inmr_rec.min_sell_price);
        
                if (SendHeader (SERV_INMR, terminal) == -1)
                {
                       print_at (terminal_no,1,"Inmr failed to send");
                       return -1;
                }
                 
                if (send (socketHandle, (char *)&rec,sizeof(rec),0) == -1)
                {
                    print_at (terminal_no,1,"Failed inmr record\n");
                    /* return -1; */
                }
              
                UpdatePosdtup (hash, "inmr" , 1);

                /*
                    Also send inbm record;
                */
                strcpy (inbm_rec.co_no,com_no);
                strcpy (inbm_rec.item_no, inmr_rec.item_no);
                strcpy (inbm_rec.barcode,"                ");
                cc = find_rec (inbm, &inbm_rec,GTEQ, "r");

                while (cc == 0 && strcmp (inbm_rec.item_no, inmr_rec.item_no) == 0)
                {
                    strcpy (rec2.co_no,inbm_rec.co_no);
                    strcpy (rec2.barcode, inbm_rec.barcode);
                    strcpy (rec2.item_no, inbm_rec.item_no);
                    strcpy (rec2.uom    , inbm_rec.uom);

                    if (SendHeader (SERV_INBM, terminal) == -1)
                    {
                          return -1;
                    }
                        
                    if (send (socketHandle,(char *)&rec2,sizeof (rec2),0) == -1)
                    {
                            /* return -1; */
                    }
                    recv (socketHandle, temp,5,0);
                    if (!strcmp (temp,"ERROR"))
                    {
                    }
                    cc = find_rec (inbm, &inbm_rec, NEXT, "r");

                }
        }
        else
        {
           UpdatePosdtup (hash, "inmr" , 0);
           return -1;
        }

		return 1;
}

int
GetCumr (
 long hash,
 int terminal)
{
        int cc;
        struct cumr_record rec;

        cumr_rec.hhcu_hash = hash;
    
        cc = find_rec (cumr,&cumr_rec,EQUAL,"r");

        if (!cc)
        {
                strcpy (rec.dbt_no        , cumr_rec.dbt_no);
                strcpy (rec.dbt_name      , cumr_rec.dbt_name);
                sprintf (rec.hhcu_hash, "%ld",cumr_rec.hhcu_hash);
                strcpy (rec.class_type    , cumr_rec.class_type);
                strcpy (rec.price_type    , cumr_rec.price_type);
                strcpy (rec.ch_adr1       , cumr_rec.ch_adr1);
                strcpy (rec.ch_adr2       , cumr_rec.ch_adr2);
                strcpy (rec.ch_adr3       , cumr_rec.ch_adr3);
                strcpy (rec.ch_adr4       , cumr_rec.ch_adr4);
                strcpy (rec.contact_name  , cumr_rec.contact_name);
                strcpy (rec.phone_no      , cumr_rec.phone_no);
                sprintf (rec.ord_value, "%12.2f",cumr_rec.ord_value);
                sprintf (rec.bo_current,"%12.2f", cumr_rec.bo_current);
                sprintf (rec.bo_per1, "%12.2f", cumr_rec.bo_per1);
                sprintf (rec.bo_per2, "%12.2f",cumr_rec.bo_per2);
                sprintf (rec.bo_per3, "%12.2f", cumr_rec.bo_per3);
                sprintf (rec.bo_per4, "%12.2f", cumr_rec.bo_per4);
                sprintf (rec.bo_fwd,  "%12.2f", cumr_rec.bo_fwd);
                sprintf (rec.od_flag, "%d", cumr_rec.od_flag);
   
                if (SendHeader (SERV_CUMR, terminal) == -1)
                {
                   /* printf("\nFailed to send cumr message");*/
                   return -1;
                }

                if (send (socketHandle,
                        (char *)&rec,sizeof (rec),0) == -1)
                {
                   /* printf("\nSend record failed on cumr"); */
                }

                UpdatePosdtup (hash, "cumr" , 1); 
        }
        else
        {
                printf ("\nCustomer record hash invalid");
                UpdatePosdtup (hash, "cumr", 0);

                return -1;
        }

        return 1;
}

int
GetInpr(
 long hash,
 int terminal)
{
   int cc;
   int pLevel;
   struct  inpr_record rec;
   /*
   pLevel = 0, company;
            1  branch;
            2  warehouse

   */

   pLevel = atoi(get_env("SK_CUSPRI_LVL"));


   inpr_rec.hhbr_hash = hash;
   inpr_rec.hhgu_hash = 0;
   inpr_rec.price_type = 0;
   strcpy (inpr_rec.curr_code, "   ");
   strcpy (inpr_rec.cust_type, "   ");
   
   if (pLevel == 0)
   {
          strcpy (inpr_rec.br_no, "  ");
          strcpy (inpr_rec.wh_no, "  ");
   }
   else if (pLevel == 1)
   {
          strcpy (inpr_rec.br_no,bra_no);
          strcpy (inpr_rec.wh_no,"  ");
   }
   else
   {
         strcpy (inpr_rec.br_no, bra_no);
         strcpy (inpr_rec.wh_no, war_no);
   }

   cc = find_rec (inpr,&inpr_rec,GTEQ, "r");

   memset (&rec, 0, sizeof (rec));

   while  (cc == 0 && hash == inpr_rec.hhbr_hash)
   {  
           if (pLevel == 0)
              strcpy (rec.priceLevel        ,"C");
           else if (pLevel == 1)
                  strcpy (rec.priceLevel        ,"B");
           else     
                  strcpy (rec.priceLevel        ,"W");

           sprintf (rec.hhbr_hash, "%ld",inpr_rec.hhbr_hash);
           sprintf (rec.price_type,"%d", inpr_rec.price_type);
           strcpy (rec.curr_code         , inpr_rec.curr_code);
           strcpy (rec.cust_type         , inpr_rec.cust_type);
           sprintf (rec.hhgu_hash, "%ld",inpr_rec.hhgu_hash);
           strcpy (rec.price_by     , inpr_rec.price_by);
           sprintf (rec.qty_brk1, "%12.2f"  , inpr_rec.qty_brk1); 
           sprintf (rec.qty_brk2 , "%12.2f" ,inpr_rec.qty_brk2); 
           sprintf (rec.qty_brk3 , "%12.2f" ,inpr_rec.qty_brk3); 
           sprintf (rec.qty_brk4 , "%12.2f" , inpr_rec.qty_brk4); 
           sprintf (rec.qty_brk5 , "%12.2f" ,  inpr_rec.qty_brk5); 
           sprintf (rec.qty_brk6 , "%12.2f" ,  inpr_rec.qty_brk6); 
           sprintf (rec.qty_brk7 , "%12.2f" ,  inpr_rec.qty_brk7); 
           sprintf (rec.qty_brk8 , "%12.2f" ,  inpr_rec.qty_brk8); 
           sprintf (rec.qty_brk9 , "%12.2f" ,  inpr_rec.qty_brk9);
           sprintf (rec.base     , "%12.2f" , inpr_rec.base);
           sprintf (rec.price1   , "%12.2f" , inpr_rec.price1);
           sprintf (rec.price2   , "%12.2f" , inpr_rec.price2);
           sprintf (rec.price3   , "%12.2f"           , inpr_rec.price3);
           sprintf (rec.price4  ,  "%12.2f"           , inpr_rec.price4);
           sprintf (rec.price5 , "%12.2f"             , inpr_rec.price5);
           sprintf (rec.price6 , "%12.2f"             , inpr_rec.price6);
           sprintf (rec.price7 , "%12.2f"             , inpr_rec.price7);
           sprintf (rec.price8 , "%12.2f"             , inpr_rec.price8);
           sprintf (rec.price9 , "%12.2f"             , inpr_rec.price9);

           if (SendHeader (SERV_INPR, terminal) == -1)
           {
                   return -1;
           }
           
           if (send (socketHandle,(char *)&rec,sizeof(rec),0) == -1)
           {
           }
       
           UpdatePosdtup (hash, "inpr" , 1); 
      
           cc = find_rec (inpr,&inpr_rec,NEXT , "r");
       
           memset (&rec, 0, sizeof (rec));
  
   }
   return 1;
}

int
GetIncp (
 long hash,
 int terminal)
{
   int cc;
   int pLevel; 
   struct incp_record rec;

   pLevel = atoi (get_env ("SK_CUSPRI_LVL"));

   incp_rec.hhbr_hash = hash;
   
   cc = find_rec (incp,&incp_rec,GTEQ,"r");

   while (cc == 0 && incp_rec.hhbr_hash == hash)
   {
           if (pLevel == 0)
           {
                  strcpy (rec.priceLevel,"C");
           }
           else if (pLevel == 1)
           {
                  strcpy (rec.priceLevel,"B");
           }
           else
           {
                  strcpy (rec.priceLevel,"W");
           }
        
           sprintf (rec.hhcu_hash, "%ld", incp_rec.hhcu_hash);
           strcpy (rec.cus_type,	incp_rec.cus_type);
           strcpy (rec.area_code,	incp_rec.area_code);
           sprintf (rec.hhbr_hash, "%ld", incp_rec.hhbr_hash);
           strcpy (rec.curr_code,incp_rec.curr_code);
           strcpy (rec.status, incp_rec.status);
           sprintf (rec.date_from, "%ld",   incp_rec.date_from);
           sprintf (rec.date_to  , "%ld",   incp_rec.date_to);
           sprintf (rec.price1, "%12.2f", incp_rec.price1);
           sprintf (rec.price2, "%12.2f", incp_rec.price2);
           sprintf (rec.price3, "%12.2f", incp_rec.price3);
           sprintf (rec.price4, "%12.2f", incp_rec.price4);
           sprintf (rec.price5, "%12.2f", incp_rec.price5);
           sprintf (rec.price6, "%12.2f", incp_rec.price6);
           sprintf (rec.price7, "%12.2f", incp_rec.price7);
           sprintf (rec.price8, "%12.2f", incp_rec.price8);
           sprintf (rec.price9 , "%12.2f",incp_rec.price9);
           strcpy (rec.comment,incp_rec.comment);
           strcpy (rec.dis_allow, incp_rec.dis_allow);
           strcpy (rec.stat_flag, incp_rec.stat_flag);

                
           if (SendHeader (SERV_INCP, terminal) == -1)
           {
                  return -1;
           }

           if (send (socketHandle, (char *)&rec,sizeof(rec),0) == -1)
           {
           }
           
           UpdatePosdtup (hash, "incp" , 1); 
           
           cc = find_rec (incp, &incp_rec, NEXT, "r");
   }
	
   return 1;
}

/*==============================================
| This send multiple record for each hhcu_hash |
==============================================*/
int    
GetInds (
 long hhcu_hash,
 int terminal)
{
   int cc;
   struct inds_record rec;

   inds_rec.hhcu_hash = hhcu_hash;

   cc = find_rec (inds,&inds_rec,EQUAL,"r");
   while (!cc)
   {
           strcpy (rec.discLevel    , "W");
           sprintf (rec.hhcu_hash, "%ld",inds_rec.hhcu_hash);
           sprintf (rec.price_type, "%d",inds_rec.price_type);
           strcpy (rec.category     , inds_rec.category);
           strcpy (rec.sel_group    , inds_rec.sel_group);
           strcpy (rec.cust_type    , inds_rec.cust_type);
           sprintf (rec.hhbr_hash , "%ld", inds_rec.hhbr_hash);
           strcpy (rec.disc_by,inds_rec.disc_by);
           sprintf (rec.qty_brk1 ,"%12.2f", inds_rec.qty_brk1);
           sprintf (rec.qty_brk2 ,"%12.2f",            inds_rec.qty_brk2);
           sprintf (rec.qty_brk3 ,"%12.2f",            inds_rec.qty_brk3);
           sprintf (rec.qty_brk4 ,"%12.2f",            inds_rec.qty_brk4);
           sprintf (rec.qty_brk5 ,"%12.2f",            inds_rec.qty_brk5);
           sprintf (rec.qty_brk6 ,"%12.2f",            inds_rec.qty_brk6);

           sprintf (rec.disca_pc1, "%12.2f",           inds_rec.disca_pc1);
           sprintf (rec.disca_pc2, "%12.2f",           inds_rec.disca_pc2);
           sprintf (rec.disca_pc3, "%12.2f",           inds_rec.disca_pc3);
           sprintf (rec.disca_pc4, "%12.2f",           inds_rec.disca_pc4);
           sprintf (rec.disca_pc5, "%12.2f",           inds_rec.disca_pc5);
           sprintf (rec.disca_pc6, "%12.2f",           inds_rec.disca_pc6);

           sprintf (rec.discb_pc1, "%12.2f",           inds_rec.discb_pc1);
           sprintf (rec.discb_pc2, "%12.2f",           inds_rec.discb_pc2);
           sprintf (rec.discb_pc3, "%12.2f",           inds_rec.discb_pc3);
           sprintf (rec.discb_pc4, "%12.2f",           inds_rec.discb_pc4);
           sprintf (rec.discb_pc5, "%12.2f",           inds_rec.discb_pc5);
           sprintf (rec.discb_pc6, "%12.2f",           inds_rec.discb_pc6);

           sprintf (rec.discc_pc1, "%12.2f",           inds_rec.discc_pc1);
           sprintf (rec.discc_pc2, "%12.2f",           inds_rec.discc_pc2);
           sprintf (rec.discc_pc3, "%12.2f",           inds_rec.discc_pc3);
           sprintf (rec.discc_pc4, "%12.2f",           inds_rec.discc_pc4);
           sprintf (rec.discc_pc5, "%12.2f",           inds_rec.discc_pc5);
           sprintf (rec.discc_pc6, "%12.2f",           inds_rec.discc_pc6);
   
           if (SendHeader (SERV_INDS, terminal) == -1)
           {
                   return -1;
           }

           if (send (socketHandle, (char *)&rec,sizeof(rec),0) == -1)
           {
           }
       
           UpdatePosdtup (hhcu_hash, "inds" , 1); 
           
           cc = find_rec (inds,&inds_rec,NEXT,"r");
   }

   

   return 1;
}

/*
  the source of the hash it first to be converted to long
*/

int    
GetExcf (
 long hash,
 int terminal)
{
        int cc;
        struct  excf_record rec;
        char cat[12];

        sprintf (cat,"%ld",hash);
    
        /*
        strcpy(excf_rec.cat_no,cat);
        strcpy(excf_rec.co_no,com_no);
        */

        excf_rec.hhcf_hash = hash;

        cc = find_rec (excf,&excf_rec,EQUAL,"r");

        if (!cc)
        {
                 strcpy (rec.cat_no , excf_rec.cat_no);
                 strcpy (rec.cat_desc, excf_rec.cat_desc);
                 sprintf (rec.max_disc, "%12.2f",   excf_rec.max_disc);
                 sprintf (rec.min_marg  , "%12.2f", excf_rec.min_marg);
         
                 if (SendHeader (SERV_EXCF, terminal) == -1)
                 {
                         return -1;
                 }

                 if (send (socketHandle,(char *)&rec,sizeof (rec),0) == -1)
                 {
                 }

                 UpdatePosdtup (hash, "excf" , 1); 
        }
        else
        {
           return 0;
        }

    return 1;
}

int    
GetInum (
 long hhum_hash,
 int terminal)
{
        int cc;
        struct inum_record rec;

        inum_rec.hhum_hash = hhum_hash;

        cc = find_rec (inum,&inum_rec,EQUAL,"r");

        if (!cc)
        {
                strcpy (rec.uom_group          , inum_rec.uom_group);
                sprintf (rec.hhum_hash, "%ld",inum_rec.hhum_hash);
                strcpy (rec.uom                , inum_rec.uom);
                strcpy (rec.desc               , inum_rec.desc);
                sprintf (rec.cnv_fct, "%12.2f" , inum_rec.cnv_fct);

                if (SendHeader (SERV_INUM, terminal) == -1)
                {
                   return -1;
                }

                if (send (socketHandle,(char *)&rec, sizeof (rec),0) == -1)
                {
                }

                UpdatePosdtup (hhum_hash, "inum" , 1); 
        }
        else
        {
                return 0;
        }

        return 1;
}

int    
GetInsf (
 long hhbr_hash,
 int terminal)
{
        int cc;
        struct insf_record rec;

        insf_rec.hhbr_hash = hhbr_hash;

        cc = find_rec (insf,&insf_rec,EQUAL, "r");

        if (!cc)
        {
                sprintf (rec.hhbr_hash, "%ld", insf_rec.hhbr_hash);
                strcpy (rec.serial_no , insf_rec.serial_no);

                if (SendHeader (SERV_INSF, terminal) == -1)
                {
                   return -1;
                }

                if (send (socketHandle,(char *)&rec,sizeof(rec),0) == -1)
                {
                }
        
                UpdatePosdtup (hhbr_hash, "insf" , 1); 
        }
        else
        {
           return 0;
        }

        return 1;
}

int    
GetSokt (
 long hhbr_hash,
 int terminal)
{
   int cc;
   struct sokt_record rec;

   sokt_rec.hhbr_hash = hhbr_hash; 

   cc = find_rec (sokt, &sokt_rec,EQUAL, "r");

   if (!cc)
   {
          sprintf (rec.hhbr_hash, "%ld",sokt_rec.hhbr_hash);
          sprintf (rec.line_no, "%d", sokt_rec.line_no);
          sprintf (rec.mabr_hash, "%ld", sokt_rec.mabr_hash);
          sprintf (rec.due_date , "%ld", sokt_rec.due_date);
          strcpy (rec.bonus       , sokt_rec.bonus);

          if (SendHeader (SERV_SOKT, terminal) == -1)
          {
                  return -1;
          }

          if (send (socketHandle,(char *)&rec,sizeof(rec),0) == -1)
          {
          }
         
          UpdatePosdtup (hhbr_hash, "sokt" , 1); 

   }
   else
   {
          return 0;
   }

   return 1;
}

int
GetCnch (
 long hash,
 int terminal)
{
   int cc;
   struct cnch_record rec;

   cnch_rec.hhch_hash = hash; 

   cc = find_rec (cnch, &cnch_rec,EQUAL, "r");

   if (!cc)
   {
          sprintf (rec.hhch_hash, "%ld",cnch_rec.hhch_hash);
          strcpy (rec.cont_no     , cnch_rec.cont_no);
          strcpy (rec.desc        , cnch_rec.desc);
          strcpy (rec.contact     , cnch_rec.contact);
          sprintf (rec.date_wef  , "%ld", cnch_rec.date_wef);
          sprintf (rec.date_rev  , "%ld", cnch_rec.date_rev);
          sprintf (rec.date_ren  , "%ld", cnch_rec.date_ren);
          strcpy (rec.exch_type   , cnch_rec.exch_type);

          if (SendHeader (SERV_CNCH, terminal) == -1)
          {
                  return -1;
          }

          if (send (socketHandle,(char *)&rec,sizeof (rec),0) == -1)
          {
          }
         
          UpdatePosdtup (hash, "cnch" , 1); 
   }

   return 1;
}

int
GetCncd (
 long hash,
 int terminal)
{
        int cc;
        struct cncd_record rec;
   
        cncd_rec.hhch_hash = hash;
        cncd_rec.line_no   = 0;

        cc = find_rec (cncd,&cncd_rec,GTEQ, "r");

        while (cc == 0 && cncd_rec.hhch_hash == hash)
        {
             sprintf (rec.hhch_hash, "%ld",cncd_rec.hhch_hash);
             sprintf (rec.line_no, "%d",   cncd_rec.line_no);
             sprintf (rec.hhbr_hash , "%ld", cncd_rec.hhbr_hash);
             sprintf (rec.hhcc_hash , "%ld", cncd_rec.hhcc_hash);
             sprintf (rec.hhsu_hash , "%ld", cncd_rec.hhsu_hash);
             sprintf (rec.hhcl_hash , "%ld", cncd_rec.hhcl_hash);
             sprintf (rec.price     , "%12.2f", cncd_rec.price);
             strcpy (rec.curr_code, cncd_rec.curr_code);
             strcpy (rec.disc_ok  , cncd_rec.disc_ok);
             sprintf (rec.cost, "%12.2f",       cncd_rec.cost);


             if (SendHeader (SERV_CNCD, terminal) == -1)
             {
                     return -1;
             }

             if (send (socketHandle,(char *)&rec,sizeof (rec),0) == -1)
             {
             }
        
             UpdatePosdtup (hash, "cncd" , 1);
             
             cc = find_rec (cncd,&cncd_rec,NEXT,"r");
        }
		
		return 1;
}

int
SendHeader(
 char *mtype,
 int term)
{
   char temp [6];
   
   memset (&mess_head,0, sizeof (mess_head));

   strcpy (mess_head.message, mtype);
   strcpy (mess_head.type, "SRV_POS");
   /* mess_head.len = 1; */  /* sizeof(mess_head); */
   
   if (send (socketHandle ,
            (char *)&mess_head,sizeof (mess_head),0) == -1)
   {
         /* return -1; */
   }
   recv (socketHandle, temp, 5,0);
   if (!strcmp (temp,"ERROR"))
   {
   }
   return 1;
}

int
UpdatePosdtup(
 long hash,
 char *file,
 int normal)
{
   int cc;
   char temp [6];

   if (normal )
   {  
      recv (socketHandle, temp, 5,0);
      if (!strcmp (temp,"ERROR"))
      {
      }
   }
   
    
   strcpy (posdtup_rec.file_name,file);
   posdtup_rec.record_hash = hash;
   posdtup_rec.pos_no = terminal_no;
   
   cc = find_rec (posdtup,&posdtup_rec, EQUAL, "r");

   if (!cc)
   {
           abc_delete (posdtup);
   }
   else
   {
   }
    
   return 1;

}

void
ChildEnds(
 int sig)
{
   
   printf("\nA child process died");
}

/**eof**/
