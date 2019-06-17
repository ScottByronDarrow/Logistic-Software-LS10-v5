/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: tr_delmani.c,v 5.4 2002/08/01 01:46:43 scott Exp $
|  Program Name  : (tr_delmani.c)
|  Program Desc  : (Delivery manifest)
|---------------------------------------------------------------------|
|  Author        : Alan Rivera .   | Date Written  : 29/05/96         |
|---------------------------------------------------------------------|
| $Log: tr_delmani.c,v $
| Revision 5.4  2002/08/01 01:46:43  scott
| .
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tr_delmani.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_delmani/tr_delmani.c,v 5.4 2002/08/01 01:46:43 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_tr_mess.h>
#include	<get_lpno.h>
#include 	<dsp_process2.h>
#include 	<dsp_screen.h>

static char	*data	= "data";

#include	"schema"

struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct commRecord	comm_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct trhrRecord	trhr_rec;
struct trlnRecord	trln_rec;
struct trveRecord	trve_rec;
struct extfRecord	extf_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;


/*
 * Local & Screen Structures. 
 */
struct {
	char	vehi_ref [11];
	char	vehi_desc [41];
	long	del_date; 
	char	date_desc [20];
	char	trip_name [13];
    char    trip_desc [13];
	char	sequence [2];
	int		printerNo;
    long    hhveHash;
	char	lp_str [3];
	char	dummy [11];
} local_rec;


/*======================
Special fields and flags
=======================*/

FILE 	*fout,
		*fsort;

char    *srt_offset [256];

char    tmp_line [300],	
        curr_co [3],
		driver_code [7], 
		driver_name [41], 
		cust_no [7], 
		cust_name [41], 
        cust_add1 [41],
        cust_add2 [41],
        cust_add3 [41],
        cust_add4 [41],
        sman_code [3],
        sman_name [40],
		itemno [17],
		itemdesc [41],  
        uom [5],
        areacode [3],
        areadesc [41],
        trucker [41],
        paymode [2],
        vehicle [11],
        tripname [13],
        payment [7],
        prev_vehi [11],
        prev_trip [13],
        prev_cust [7],
        prev_area [3],
        prev_trucker [41],
        curr_vehi [11],
        curr_trip [13],
        curr_cust [7],
        curr_area [3],
        curr_trucker [41];

long    hhcu_hash,
        curr_hhcu_hash,
        prev_hhcu_hash;

double   price,
        amount,
        tamount;

float   disc,
 		loadqty,
        retqty;

long    deldate,
        curr_deldate,
        prev_deldate;	/*Date*/

/*=======================
| Function declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	EndReport 			(void);
void 	HeadingOutput 		(void);
void 	ProcessFile 		(void);
void 	storedata 			(void);
void 	DisplayReport 		(void);
void 	head_display 		(void);
void 	head_display2 		(void);
void 	SrchTrve 			(char *);
void 	srch_trhr_date 		(char *);
void 	srch_trhr_trip 		(char *);
int	 	heading 			(int);
char 	*_sort_read 		(FILE *srt_fil);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 1 && argc != 6)
	{
		print_at (0,0,mlStdMess036, argv [0]);
        return (EXIT_FAILURE);
	}

	OpenDB ();

	sprintf (local_rec.vehi_ref,"%-10.10s",argv [1]);
	local_rec.del_date = atol (argv [2]);
	sprintf (local_rec.trip_name,"%-12.12s",argv [3]);
	sprintf (local_rec.sequence,"%-1.1s",argv [4]);
	local_rec.printerNo = atoi (argv [5]);

	dsp_screen ("Printing Delivery Manifest",
			comm_rec.co_no,comm_rec.co_name);
	HeadingOutput ();
	ProcessFile (); 	
	fprintf (fout, ".EOF\n");
	EndReport ();

	/*=============================
	| Set up required parameters  |
	=============================*/
    shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (trhr, trhr_list, TRHR_NO_FIELDS, "trhr_id_no");
	open_rec (trln, trln_list, TRLN_NO_FIELDS, "trln_hhtr_hash");
	open_rec (trve, trve_list, TRVE_NO_FIELDS, "trve_id_no");
	open_rec (extf, extf_list, EXTF_NO_FIELDS, "extf_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
 	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
 }

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (trhr);
	abc_fclose (trln);
	abc_fclose (trve);
	abc_fclose (extf);
    abc_fclose (exaf);
    abc_fclose (inum);
	abc_dbclose (data);
}

void
EndReport (void)
{
	pclose (fout);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	if ((fout = popen ("pformat","w")) == (FILE *) NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNo);
	fprintf (fout,".PI16\n");
	fprintf (fout,".8\n");
	fprintf (fout,".L132\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".C%s\n", clip (comm_rec.co_name));
	fprintf (fout,".CDELIVERY MANIFEST \n\n");
	fprintf (fout,".CAS AT %s\n", SystemTime ());			
	fprintf (fout,".R-------------");
	fprintf (fout,"--------------");
	fprintf (fout,"-------------------");
	fprintf (fout,"----------------------------------"); 
	fprintf (fout,"-------------");  
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------\n");

	fflush (fout); 
}

void
ProcessFile (void)
{
	float	std_cnv_fct = (float) 0,
			cnv_fct = (float) 0;

	fsort=sort_open ("tr_delmani");
	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	strcpy (trve_rec.ref, local_rec.vehi_ref);
	cc = find_rec (trve,&trve_rec,GTEQ,"r");
	while (!cc 
		&& !strcmp (trve_rec.co_no, comm_rec.co_no) 
		&& !strcmp (trve_rec.br_no, comm_rec.est_no) 
		&& (!strcmp (trve_rec.ref, local_rec.vehi_ref) 
				|| !strcmp (local_rec.vehi_ref, "          ")))
	{
            sprintf (vehicle, "%-10.10s", trve_rec.ref);
			trhr_rec.hhve_hash = trve_rec.hhve_hash;
			trhr_rec.del_date = local_rec.del_date;
            strcpy (trhr_rec.trip_name, local_rec.trip_name);
			cc = find_rec (trhr, &trhr_rec, GTEQ,"r");
			while (!cc 
				&& trhr_rec.hhve_hash == trve_rec.hhve_hash 
				&& (trhr_rec.del_date == local_rec.del_date 
					|| local_rec.del_date == 0l) 
				&& (!strcmp (local_rec.trip_name, "All Trips   ") 
					|| !strcmp (trhr_rec.trip_name,local_rec.trip_name)))	
			{
				strcpy (tripname, trhr_rec.trip_name);
                deldate = trhr_rec.del_date;
				strcpy (extf_rec.co_no, comm_rec.co_no);
                sprintf (extf_rec.code,"%-6.6s", trhr_rec.driver);
				cc = find_rec (extf, &extf_rec, EQUAL,"r");
  				if (!cc)
                     strcpy (trucker, extf_rec.name);
				trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
				cc = find_rec (trln, &trln_rec, GTEQ, "r");
				while (!cc && trln_rec.hhtr_hash == trhr_rec.hhtr_hash)
				{
					cohr_rec.hhco_hash = trln_rec.hhco_hash;
					cc = find_rec (cohr, &cohr_rec, EQUAL, "r");
					if (!cc)  	
					{
						sprintf (areacode,"%-2.2s", cohr_rec.area_code);
						cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
						cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
						if (!cc)
						{
							sprintf (cust_no, "%-6.6s", cumr_rec.dbt_no);
							sprintf (cust_name, "%-40.40s", cumr_rec.dbt_name);
  							sprintf (sman_code, "%-2.2s",cumr_rec.sman_code);	
                            sprintf (cust_add1, "%-40.40s",cumr_rec.dl_adr1);
                            sprintf (cust_add2, "%-40.40s",cumr_rec.dl_adr2);
                            sprintf (cust_add3, "%-40.40s",cumr_rec.dl_adr3);
                            sprintf (cust_add4, "%-40.40s",cumr_rec.dl_adr4);
							sprintf (paymode, "%-1.1s",cumr_rec.pay_method);
                            hhcu_hash = cumr_rec.hhcu_hash;
						}
					}
					coln_rec.hhco_hash = trln_rec.hhco_hash;
					coln_rec.line_no = 0;	
					cc = find_rec (coln, &coln_rec, GTEQ, "r");
					while (!cc && coln_rec.hhco_hash == trln_rec.hhco_hash)
					{
						inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
						cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
						if (!cc)
						{
   							sprintf (itemno, "%-16.16s", inmr_rec.item_no);
   							sprintf (itemdesc, "%-40.40s", inmr_rec.description);
 							inum_rec.hhum_hash = inmr_rec.std_uom;
							cc = find_rec (inum, &inum_rec, EQUAL, "r");
							if (!cc)
							{
								std_cnv_fct =	inum_rec.cnv_fct;
							}
						}
						inum_rec.hhum_hash = coln_rec.hhum_hash;
						cc = find_rec (inum,&inum_rec,EQUAL,"r");
						if (!cc)
						{
							sprintf (uom, "%-4.4s",inum_rec.uom);
							cnv_fct	=	inum_rec.cnv_fct/std_cnv_fct;
						}
					    price   = coln_rec.sale_price * inum_rec.cnv_fct;
                        disc    = coln_rec.disc_pc;	
                        loadqty = coln_rec.q_order/cnv_fct;
                        retqty  = coln_rec.qty_ret/cnv_fct;
                        amount  = coln_rec.gross;
                        storedata ();   
						cc = find_rec (coln, &coln_rec, NEXT, "r");
					}
					cc = find_rec (trln, &trln_rec, NEXT, "r");
				}
				cc = find_rec (trhr, &trhr_rec, NEXT,"r");
	    	}
		    cc = find_rec (trve, &trve_rec, NEXT,"r");
   	}
	fsort = sort_sort (fsort, "tr_delmani");
    DisplayReport ();
    fflush (fout);
    sort_delete (fsort,"tr_delmani");
}

void
storedata (
 void)
{
	/*---------------------------------
	| Add transaction to sort file  . |
	---------------------------------*/
    sprintf (tmp_line, "%s%c%ld%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%f%c%f%c%f%c%s%c%f%c%ld%c%f%c%s\n", 
                      vehicle,               	1,	/* Offset #0  	*/
                      deldate,                  1,  /* Offset #1    */
                      tripname,               	1,	/* Offset #2  	*/
                      areacode,           		1,	/* Offset #3  	*/
                      trucker,               	1,	/* Offset #4  	*/
	                  cust_no, 					1,	/* Offset #5  	*/
                      sman_code,    			1,	/* Offset #6 	*/
                      paymode,					1,	/* Offset #7 	*/
                      itemno,					1,	/* Offset #8 	*/
                      itemdesc,					1,	/* Offset #9	*/
			          price,  					1,  /* Offset #10   */	
                      disc,   					1,  /* Offset #11   */
                      loadqty,                  1,  /* Offset #12   */
   				      uom,                      1,  /* Offset #13   */		
                      amount,                   1,  /* Offset *14   */
                      hhcu_hash,                1,  /* Offset *15   */
                      retqty,                   1,  /* Offset *16   */
					  " ");	

    sort_save (fsort,tmp_line);
}

void
DisplayReport (void)
{
	char	temp_item [17];
	char 	*out_tmp_line;
	int 	first_time = TRUE;
	out_tmp_line =  _sort_read (fsort);   
    if (out_tmp_line == (char *)0)
    {
        /*print_mess ("No record found in the specified range, press <Enter> to exit");*/
        print_mess (ML (mlTrMess027));
        getchar ();
		clear_mess ();
    }
	
    sprintf (prev_vehi, "%-10.10s", "          ");
    sprintf (prev_trip, "%-12.12s", "            ");
    sprintf (prev_cust, "%-6.6s", "      ");
	memset (itemno,0,sizeof (itemno));
	memset (temp_item,0,sizeof (temp_item));
	prev_hhcu_hash = 0;
	while (out_tmp_line != (char *)0)
	{

    	sprintf (curr_vehi, "%-10.10s", srt_offset [0]);
        curr_deldate = atol (srt_offset [1]);
    	sprintf (curr_trip, "%-12.12s", srt_offset [2]);
        sprintf (curr_area, "%-2.2s", srt_offset [3]);
        sprintf (curr_trucker, "%-40.40s", srt_offset [4]);
    	sprintf (curr_cust, "%-6.6s", srt_offset [5]);
		curr_hhcu_hash = atof (srt_offset [15]);

        sprintf (vehicle, "%-10.10s", srt_offset [0]);         	
        deldate = atol (srt_offset [1]);
        sprintf (tripname, "%-12.12s", srt_offset [2]);
        sprintf (areacode, "%-2.2s", srt_offset [3]);
        sprintf (trucker, "%-40.40s", srt_offset [4]);
	    sprintf (cust_no, "%-6.6s", srt_offset [5]);
        sprintf (sman_code, "%-2.2s", srt_offset [6]);		
        sprintf (paymode, "%-1.1s", srt_offset [7]);			
        sprintf (itemno, "%-16.16s", srt_offset [8]);		
        sprintf (itemdesc, "%-40.40s", srt_offset [9]);	
		price      =  atof	 (srt_offset [10]);				
        disc       =  atof	 (srt_offset [11]);		
        loadqty    =  atof (srt_offset [12]);          
        sprintf (uom, "%-4.4s", srt_offset [13]);
        amount     =  atol (srt_offset [14]);          
		hhcu_hash  = atof (srt_offset [15]);
        retqty     =  atof (srt_offset [16]);
		sprintf (temp_item,"%-16.16s",itemno);

		if ((strcmp (prev_vehi, curr_vehi)) || 
           (strcmp (prev_trip, curr_trip)))
		{
			if (!first_time)
			{
				fprintf (fout,"                 ");
				fprintf (fout,"                                         ");
				fprintf (fout,"           ");
				fprintf (fout,"          ");
   				fprintf (fout,"      ");
   				fprintf (fout,"         ");
   				fprintf (fout,"                  ");
				fprintf (fout," %-10.10s\n\n", " "); 	
   			/*	fprintf (fout,"      TOTAL       ");
				fprintf (fout," %10.2f\n\n", DOLLARS (tamount)); 	*/

				fprintf (fout,"                 ");
				fprintf (fout,"                                         ");
   				fprintf (fout," Acknowledged by : ______________________\n");
				fprintf (fout,"                 ");
				fprintf (fout,"                                         ");
				fprintf (fout,"                   Sign over printed name\n"); 	
                tamount = 0;
				first_time = FALSE;
				head_display ();
				fprintf (fout,".PA\n");
			}
			else
			{
				head_display ();
				first_time = FALSE;
			}
            head_display2 ();
		}
        else  
        {
	     	if (strcmp (prev_cust, curr_cust))
			{
					
				fprintf (fout,"                 ");
				fprintf (fout,"                                         ");
				fprintf (fout,"           ");
				fprintf (fout,"          ");
   				fprintf (fout,"      ");
   				fprintf (fout,"         ");
   				fprintf (fout,"                  ");
				fprintf (fout," %-10.10s\n\n", " "); 	
   		/*		fprintf (fout,"      TOTAL       ");
				fprintf (fout," %10.2f\n\n", DOLLARS (tamount)); 	*/

				fprintf (fout,"                 ");
				fprintf (fout,"                                         ");
   				fprintf (fout," Acknowledged by : ______________________\n");
				fprintf (fout,"                 ");
				fprintf (fout,"                                         ");
				fprintf (fout,"                   Sign over printed name\n"); 	

    			fprintf (fout,"-------------");
				fprintf (fout,"--------------");
				fprintf (fout,"-------------------");
				fprintf (fout,"----------------------------------"); 
				fprintf (fout,"-------------"); 
				fprintf (fout, "---------------");
				fprintf (fout, "---------------");
				fprintf (fout, "---------------");
				fprintf (fout, "---------------\n");  
                tamount = 0;  
		   		head_display2 ();	
			}
		}
		fprintf (fout," %-16.16s", temp_item);
		fprintf (fout," %-40.40s", itemdesc);
		fprintf (fout,"      %10.2f", DOLLARS (price));
		fprintf (fout,"%8.2f", disc);
        fprintf (fout,"   %-4.4s", uom);
        fprintf (fout,"  %8.2f", loadqty);
 		if (retqty == 0)
        {
        	fprintf (fout," |__________|");
			fprintf (fout," %10.2f\n", DOLLARS (amount));
		}
        else
   	 	{ 
        	fprintf (fout,"   %8.2f", retqty);
			fprintf (fout,"   %10.2f\n", DOLLARS (amount));
        }        

    	sprintf (prev_vehi, "%-10.10s", curr_vehi);
    	sprintf (prev_trip, "%-12.12s", curr_trip);
    	sprintf (prev_cust, "%-6.6s", curr_cust);
		prev_hhcu_hash = curr_hhcu_hash;
        tamount += amount;

		out_tmp_line = _sort_read (fsort);
	}
	fprintf (fout,"                 ");
	fprintf (fout,"                                         ");
	fprintf (fout,"           ");
	fprintf (fout,"          ");
   	fprintf (fout,"      ");
   	fprintf (fout,"         ");
	fprintf (fout,"                  ");
	fprintf (fout," %-10.10s\n\n", " "); 	
   /*fprintf (fout,"      TOTAL       ");
	fprintf (fout," %10.2f\n\n", DOLLARS (tamount)); 	*/

	fprintf (fout,"                 ");
	fprintf (fout,"                                         ");
   	fprintf (fout," Acknowledged by : ______________________\n");
	fprintf (fout,"                 ");
	fprintf (fout,"                                         ");
	fprintf (fout,"                   Sign over printed name\n"); 	

/*	fprintf (fout,"-------------");
	fprintf (fout,"--------------");
	fprintf (fout,"-------------------");
	fprintf (fout,"----------------------------------"); 
	fprintf (fout,"-------------"); 
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------\n");
*/
}

void
head_display (
 void)
{
	fprintf (fout, ".DS7\n");
	fprintf (fout,"-------------");
	fprintf (fout,"--------------");
	fprintf (fout,"-------------------");
	fprintf (fout,"----------------------------------"); 
	fprintf (fout,"-------------"); 
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------\n");
	fprintf (fout," Vehicle Reference  :  %-10.10s", curr_vehi);
	fprintf (fout,"                    Area               :  %-2.2s\n", curr_area);
	fprintf (fout," Expected Del. Date :  %10.10s\n", DateToString (curr_deldate));
	fprintf (fout," Trip Name          :  %-12.12s", curr_trip);
	fprintf (fout,"                  Trucker            :  %-40.40s\n", curr_trucker);
	fprintf (fout,"-------------");
	fprintf (fout,"--------------");
	fprintf (fout,"-------------------");
	fprintf (fout,"----------------------------------"); 
	fprintf (fout,"-------------"); 
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------\n");
	fprintf (fout,"|    ITEM NO.    ");
	fprintf (fout,"|          D E S C R I P T I O N           "); 
	fprintf (fout,"|  P R I C E  ");
	fprintf (fout,"|  DISC%%  ");
    fprintf (fout,"|  UOM  ");
	fprintf (fout,"| LOAD QTY ");
	fprintf (fout,"| RET QTY  ");
	fprintf (fout,"|    AMOUNT         |\n");
	fprintf (fout,"-------------");
	fprintf (fout,"--------------");
	fprintf (fout,"-------------------");
	fprintf (fout,"----------------------------------"); 
	fprintf (fout,"-------------"); 
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------");
	fprintf (fout, "---------------\n");
	fflush (fout);
}

void
head_display2 (
 void)
{

	cumr_rec.hhcu_hash = curr_hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (!cc)
	{
		sprintf (cust_no, "%-6.6s", cumr_rec.dbt_no);
		sprintf (cust_name, "%-40.40s", cumr_rec.dbt_name);
  		sprintf (sman_code, "%-2.2s",cumr_rec.sman_code);	
        sprintf (cust_add1, "%-40.40s",cumr_rec.dl_adr1);
        sprintf (cust_add2, "%-40.40s",cumr_rec.dl_adr2);
        sprintf (cust_add3, "%-40.40s",cumr_rec.dl_adr3);
        sprintf (cust_add4, "%-40.40s",cumr_rec.dl_adr4);
		sprintf (paymode, "%-1.1s",cumr_rec.pay_method);
	}

	strcpy (exsf_rec.co_no, comm_rec.co_no);
    sprintf (exsf_rec.salesman_no,"%-2.2s", cumr_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, EQUAL,"r");
  	if (!cc)
        strcpy (sman_name, exsf_rec.salesman);

    if (!strcmp (paymode, "C"))
       strcpy (payment, "Cheque");
    else
       strcpy (payment, "Direct");
	fprintf (fout,"%6.6s - %40.40s", cust_no, cust_name);
    fprintf (fout,"    Salesman/MR        : %2.2s", sman_code);
	fprintf (fout," - %-40.40s\n", sman_name);
	fprintf (fout,"                                                     Payment mode       : %-1.1s  - %-6.6s\n", paymode, payment);
	/*fprintf (fout,"          %40.40s\n", cust_add1);
	fprintf (fout,"          %40.40s\n", cust_add2);
	fprintf (fout,"          %40.40s\n", cust_add3);
	fprintf (fout,"          %40.40s\n", cust_add4);  */

    if (strncmp (cust_add1, " ",1))
		fprintf (fout,"          %40.40s\n", cust_add1);
    if (strncmp (cust_add2, " ",1))
		fprintf (fout,"          %40.40s\n", cust_add2);
    if (strncmp (cust_add3, " ",1))
		fprintf (fout,"          %40.40s\n", cust_add3);
    if (strncmp (cust_add4, " ",1))
		fprintf (fout,"          %40.40s\n", cust_add4);  


    fprintf (fout,"\n\n");
	fflush (fout);
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
	rv_pr (ML (mlTrMess004),29,0,1);
	move (0,1);
	line (80);
	box (0,3,80,9);
	move (1,5);
	line (79);	
	move (1,7);
	line (79);
	move (1,9);
	line (79);
	move (1,11);
	line (79);	
	print_at (21,1,ML (mlStdMess038), curr_co, 
				clip (comm_rec.co_name));
	move (0,22);
	line (80);
	line_cnt = 0;
    scn_write (scn);   
    return (EXIT_SUCCESS);
}


/*=====================================
| Search for Transport Vehicle File . |
=====================================*/
void
SrchTrve (
 char *key_val)
{
	work_open ();
	strcpy (trve_rec.co_no, comm_rec.co_no);
    strcpy (trve_rec.br_no, comm_rec.est_no); 
	sprintf (trve_rec.ref,"%-10.10s",key_val);
	save_rec ("#Vehicle #","#Description");
	cc = find_rec (trve,&trve_rec,GTEQ,"r");
	while (!cc && !strcmp (trve_rec.co_no, comm_rec.co_no) &&
	      !strncmp (trve_rec.ref,key_val,strlen (key_val)))
	{
		cc = save_rec (trve_rec.ref,trve_rec.desc);
		if (cc)
			break;
		cc = find_rec (trve,&trve_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trve_rec.co_no, comm_rec.co_no);
	sprintf (trve_rec.ref,"%-10.10s", temp_str);
	cc = find_rec (trve,&trve_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, trve, "DBFIND");
}

/*==================================================
| Search for Delivery Date Transport Header File . |
==================================================*/
void
srch_trhr_date (
 char *key_val)
{
	char date_trp [25];
	work_open ();
	save_rec ("#Del Date    Trip Name", "#");
	trhr_rec.hhve_hash = local_rec.hhveHash;
	trhr_rec.del_date  = StringToDate (key_val);
	cc = find_rec (trhr,&trhr_rec,GTEQ,"r");
	while (!cc &&  !strncmp (DateToString (trhr_rec.del_date),key_val,strlen (key_val)) && (trhr_rec.hhve_hash == local_rec.hhveHash || local_rec.hhveHash == 0L))
	{
		sprintf (date_trp,"%-10.10s  %-12.12s",DateToString (trhr_rec.del_date),trhr_rec.trip_name);
		cc = save_rec (date_trp,"");
		if (cc)
			break;
		cc = find_rec (trhr,&trhr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (err_str, ML (mlTrMess007), trve_rec.ref); 
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return;
	}
	if (local_rec.hhveHash > 0)
	{
		trhr_rec.hhve_hash = local_rec.hhveHash;
		trhr_rec.del_date = StringToDate (temp_str);
		cc = find_rec (trhr,&trhr_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, trhr, "DBFIND");
	}
}

/*================================================
| Search for Trip Number Transport Header File . |
================================================*/
void
srch_trhr_trip (
 char *key_val)
{
	work_open ();
	save_rec ("#Trip Number", "#Del Date");
	sprintf (trhr_rec.trip_name, "%-12.12s", key_val);
	trhr_rec.hhve_hash = local_rec.hhveHash;
	trhr_rec.del_date  = local_rec.del_date; 
	cc = find_rec (trhr,&trhr_rec,GTEQ,"r");
	while (!cc &&  !strncmp (trhr_rec.trip_name,key_val,strlen (key_val)) && 
		 (trhr_rec.hhve_hash == local_rec.hhveHash || 
			local_rec.hhveHash == 0L) && 
		 (trhr_rec.del_date == local_rec.del_date || local_rec.del_date == 0L)) 
	{
		strcpy (err_str, DateToString (trhr_rec.del_date));
		cc = save_rec (trhr_rec.trip_name, err_str);
		if (cc)
			break;
		cc = find_rec (trhr,&trhr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		print_mess (ML (mlTrMess003));
		sleep (sleepTime);
		clear_mess ();
		return;
	}
	if (local_rec.hhveHash > 0)
	{
		sprintf (trhr_rec.trip_name, "%-12.12s", temp_str);
		trhr_rec.hhve_hash = local_rec.hhveHash;
		trhr_rec.del_date  = local_rec.del_date; 
		cc = find_rec (trhr,&trhr_rec,GTEQ,"r");
		while (!cc)
		{
			if (!strcmp (trhr_rec.trip_name, temp_str)) 
					break;
			else
				cc = find_rec (trhr,&trhr_rec,NEXT,"r");
		}
		if (cc)
			file_err (cc, trhr, "DBFIND");
	}
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char	*
_sort_read (
 FILE *srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset [0] = sptr;

	tptr = sptr;
	while (fld_no < 17)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		srt_offset [fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}
