/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|---------------------------------------------------------------------|
|  Program Name  : ( sk_st_svarpr.c)                                  |
|  Program Desc  : ( Print Stock Take Variation Report    	  )       |	
|                  ( for Serial Items.                            )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, incc, stts , insf,     ,                    |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 02/10/89         |
|---------------------------------------------------------------------|
|  Date Modified : (16/06/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (31/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (05/09/97)      | Modified  by  : Ana Marie Tario. |
|                :                                                    |
|  Comments      : Used Stocktake Variation Report as base program.   |
|    (16/06/92)  : exclude values of SK_IVAL_CLASS from stock take    |
|                : SC DFH 7096                                        |
|    (31/03/94)  : HGP 10469. Removal of $ signs.                     |
|    (05/09/97)  : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
| $Log: st_svarpr.c,v $
| Revision 5.1  2001/08/09 09:20:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:17:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:39:10  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:21:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:12:03  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:31:21  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/03 07:32:36  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.8  1999/10/13 02:42:17  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.7  1999/10/08 05:32:56  scott
| First Pass checkin by Scott.
|
| Revision 1.6  1999/06/20 05:20:48  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: st_svarpr.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_st_svarpr/st_svarpr.c,v 5.1 2001/08/09 09:20:10 scott Exp $";

#define	NO_SCRGEN
#include	<ml_sk_mess.h>
#include	<ml_std_mess.h>
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#define	GROUP		0
#define	ITEM		1

#define	SERIAL		(inmr_rec.mr_serial_item[0] == 'Y')
#define	MODE_OK		(incc_rec.cc_stat_flag[0] == mode[0])

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_inv_date"},
	};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	tinv_date;
	} comm_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
	};

	int ccmr_no_fields = 4;

	struct {
		char	ccmr_co_no[3];
		char	ccmr_est_no[3];
		char	ccmr_cc_no[3];
		long	ccmr_hhcc_hash;
	} ccmr_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
	};

	int inmr_no_fields = 8;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
		char	mr_serial_item[2];
		char	mr_costing_flag[2];
	} sinmr_rec, inmr_rec;

	/*===================================
	| Inventory Cost centre Base Record |
	===================================*/
	struct dbview incc_list[] ={
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_hhwh_hash"},
		{"incc_location"},
		{"incc_stake"},
		{"incc_stat_flag"},
	};

	int	incc_no_fields = 6;

	struct	{
		long	cc_hhcc_hash;
		long	cc_hhbr_hash;
		long	cc_hhwh_hash;
		char	cc_location[11];
		float	cc_stake;
		char	cc_stat_flag[2];
	} incc_rec;

	/*=============================
	| Stock Take Transaction File |
	=============================*/
	struct dbview stts_list[] ={
		{"stts_hhwh_hash"},
		{"stts_serial_no"},
		{"stts_cost"},
		{"stts_location"},
		{"stts_status"},
		{"stts_counted"},
		{"stts_stat_flag"},
	};

	int	stts_no_fields = 7;

	struct	{
		long	ts_hhwh_hash;
		char	ts_serial_no[26];
		double	ts_cost;
		char	ts_location[11];
		char	ts_status[2];
		char	ts_counted[2];
		char	ts_stat_flag[2];
	} stts_rec;

	/*==============================
	| Inventory Serial Number file |
	==============================*/
	struct dbview insf_list[] ={
		{"insf_hhsf_hash"},
		{"insf_hhwh_hash"},
		{"insf_status"},
		{"insf_serial_no"},
		{"insf_location"},
		{"insf_est_cost"},
		{"insf_act_cost"},
		{"insf_stock_take"},
		{"insf_stat_flag"},
	};

	int	insf_no_fields = 9;

	struct	{
		long	sf_hhsf_hash;
		long	sf_hhwh_hash;
		char	sf_status[2];
		char	sf_serial_no[26];
		char	sf_location[11];
		double	sf_est_cost;
		double	sf_act_cost;
		char	sf_stock_take[2];
		char	sf_stat_flag[2];
	} insf_rec;

	/*================================
	| External Category File Record. |
	================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
	};

	int excf_no_fields = 3;

	struct {
		char	cf_co_no[3];
		char	cf_cat_no[12];
		char	cf_cat_desc[41];
	} excf_rec;

	struct dbview insc_list[] ={
		{"insc_hhcc_hash"},
		{"insc_stake_code"},
		{"insc_start_date"},
		{"insc_start_time"},
		{"insc_description"}
	};

	int insc_no_fields = 5;

	struct {
		long	sc_hhcc_hash;
		char	sc_stake_code[2];
		long	sc_start_date;
		char	sc_start_time[6];
		char	sc_description[41];
	} insc_rec;

	float	tot_count[2];

	double	old_value   = 0.00;
	double	new_value   = 0.00;
	double	tot_old[2];
	double	tot_new[2];
	double	tot_var[2];

	int	lpno;
	int	print_type;
	int	st_upzero = 0   ;
	int	no_rec = 1;

	char	*inval_cls;
 	char 	*result;

	char	*comm = "comm",
			*inmr = "inmr",
			*sinmr = "sinmr",
			*excf = "excf",
			*incc = "incc",
			*ccmr = "ccmr",
			*stts = "stts",
			*insf = "insf",
			*insc = "insc",
			*data = "data";

	char	category[12];
	char	item_no[17];
	char	lower[17];
	char	upper[17];
	char	mode[2];
	int	first_time = TRUE;

	FILE	*fout;
	FILE	*fsort;

#include <ser_value.h>
/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void heading (void);
void process (void);
void save_line (void);
int  valid_group (void);
void print_sheet (char *category);
void print_total (int j);
void print_line (char *sptr);
void proc_trans (long hhwh_hash);
float f_val (char *str);
double d_val (char *str);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr;
	int	i;

	if (argc != 6)
	{
		print_at(0,0,mlSkMess365,argv[0]);
		return (EXIT_FAILURE);
	}

	lpno = atoi(argv[1]);

	if (argv[4][0] != 'G' && argv[4][0] != 'g'
	&& argv[4][0] != 'I' && argv[4][0] != 'i')
	{
		print_at(0,0,mlSkMess365,argv[0]);
		return (EXIT_FAILURE);
	}

	switch(argv[4][0])
	{
	case 'G':
	case 'g':
		print_type = GROUP;
		sprintf(lower,"%-12.12s",argv[2]);
		sprintf(upper,"%-12.12s",argv[3]);
		break;

	case 'I':
	case 'i':
		print_type = ITEM;
		sprintf(lower,"%-16.16s",argv[2]);
		sprintf(upper,"%-16.16s",argv[3]);
		break;
	}
	sprintf(mode,"%-1.1s",argv[5]);

	sptr = chk_env("SK_IVAL_CLASS");
	if (sptr)
	{
		inval_cls = strdup(sptr);
	}
	else
		inval_cls = "ZKPN";
	upshift(inval_cls); 

	st_upzero = atoi(get_env("ST_UPZERO"));

	OpenDB();


	for (i = 0; i < 2; i++)
	{
		tot_old[i] = 0.00;
		tot_new[i] = 0.00;
		tot_var[i] = 0.00;
	}
	process();

	shutdown_prog();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen(data);
	ReadMisc();

    abc_alias(sinmr,inmr);
	open_rec(inmr,inmr_list,inmr_no_fields,(print_type == GROUP) ? "inmr_id_no_3" : "inmr_id_no");
	open_rec(sinmr,inmr_list,inmr_no_fields,"inmr_hhbr_hash");
	open_rec(incc,incc_list,incc_no_fields,"incc_id_no");
	open_rec(excf,excf_list,excf_no_fields,"excf_id_no");
	open_rec(stts,stts_list,stts_no_fields,"stts_id_no");
	open_rec(insf, insf_list, insf_no_fields, "insf_id_no");
}

void
CloseDB (
 void)
{
	abc_fclose(inmr);
	abc_fclose(sinmr);
	abc_fclose(incc);
	abc_fclose(excf);
	abc_fclose(stts);
	abc_fclose(insf);
	abc_dbclose(data);
}

void
ReadMisc (
 void)
{
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec(ccmr,ccmr_list,ccmr_no_fields,"ccmr_id_no");

	strcpy(ccmr_rec.ccmr_co_no,comm_rec.tco_no);
	strcpy(ccmr_rec.ccmr_est_no,comm_rec.test_no);
	strcpy(ccmr_rec.ccmr_cc_no,comm_rec.tcc_no);
	cc = find_rec(ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in ccmr During (DBFIND)",cc,PNAME);

	abc_fclose(ccmr);
}

void
heading (
 void)
{
	if ((fout = popen("pformat","w")) == 0)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);

	open_rec(insc,insc_list,insc_no_fields,"insc_id_no");
	insc_rec.sc_hhcc_hash = ccmr_rec.ccmr_hhcc_hash;
	strcpy(insc_rec.sc_stake_code,mode);
	cc = find_rec("insc",&insc_rec,COMPARISON,"r");
	strcpy(err_str,(cc) ? " " : insc_rec.sc_description);

	fprintf(fout,".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(fout,".LP%d\n",lpno);
	fprintf(fout,".13\n");
	fprintf(fout,".PI12\n");
	fprintf(fout,".L158\n");
	fprintf(fout,".EStock Take Variation Report by %s (Serial Items)\n",(print_type == GROUP) ? "Group" : "Item No");
	fprintf(fout, ".EStockTake Selection [%s] %s\n",mode,clip(err_str));
	fprintf(fout,".E%s %s %s %s\n",comm_rec.tco_no,clip(comm_rec.tco_name),comm_rec.test_no,clip(comm_rec.test_name));
	fprintf(fout,".EW/H%s %s\n",comm_rec.tcc_no,clip(comm_rec.tcc_name));
	fprintf(fout,".EAs At %s\n",SystemTime());
	fprintf(fout,".EFrom %s To %s\n",lower,upper);
	fprintf(fout,"NOTE : Status = C(ommitted)  F(ree)  N(ew Serial Item)\n");

	fprintf(fout,".R=====================================");
	if (print_type == GROUP)
		fprintf(fout,"=============");
	fprintf(fout,"==================================================");
	fprintf(fout,"======================================");
	fprintf(fout,"====================\n");

	fprintf(fout,"=====================================");
	if (print_type == GROUP)
		fprintf(fout,"=============");
	fprintf(fout,"==================================================");
	fprintf(fout,"======================================");
	fprintf(fout,"====================\n");

	if (print_type == GROUP)
		fprintf(fout,"|   GROUP    ");
	fprintf(fout,"|  ITEM  NUMBER  ");
	fprintf(fout,"|  D E S C R I P T I O N                 ");
	fprintf(fout,"|      SERIAL NUMBER      ");
	fprintf(fout,"| LOCATION ");
	fprintf(fout,"|STAT");
	fprintf(fout,"| OLD VALUE");
	fprintf(fout,"| NEW VALUE");
	fprintf(fout,"|COUNTED");
	fprintf(fout,"|   VARIATION |\n");

	if (print_type == GROUP)
		fprintf(fout,"|------------");
	fprintf(fout,"|----------------");
	fprintf(fout,"|----------------------------------------");
	fprintf(fout,"|-------------------------");
	fprintf(fout,"|----------");
	fprintf(fout,"|----");
	fprintf(fout,"|----------");
	fprintf(fout,"|----------");
	fprintf(fout,"|-------");
	fprintf(fout,"|-------------|\n");
	fflush(fout);
	abc_fclose(insc);
}

void
process (
 void)
{
	sprintf(err_str,"Printing StockTake Variation by %s",(print_type == GROUP) ? "Group" : "Item ");
	dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);
	heading();


	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	if (print_type == GROUP)
	{
		sprintf(category,"%-11.11s"," ");
		sprintf(inmr_rec.mr_class,"%-1.1s",lower);
		sprintf(inmr_rec.mr_category,"%-11.11s",lower + 1);
		sprintf(inmr_rec.mr_item_no,"%-16.16s"," ");
	}
	else
	{
		sprintf(item_no,"%-16.16s"," ");
		sprintf(inmr_rec.mr_item_no,"%-16.16s",lower);
	}

	cc = find_rec(inmr,&inmr_rec,GTEQ,"r");
	while (!cc && valid_group())
	{

		if ((result = strstr (inval_cls, inmr_rec.mr_class)))
		{
			cc = find_rec(inmr,&inmr_rec,NEXT,"r");
			continue;
		}

		if (SERIAL)
		{
			incc_rec.cc_hhcc_hash = ccmr_rec.ccmr_hhcc_hash;
			incc_rec.cc_hhbr_hash = inmr_rec.mr_hhbr_hash;
			cc = find_rec(incc,&incc_rec,COMPARISON,"r");
			if (!cc && MODE_OK)
				proc_trans(incc_rec.cc_hhwh_hash);

		}
		cc = find_rec(inmr,&inmr_rec,NEXT,"r");
	}
	if (!no_rec)
	{
		if (print_type == GROUP)
			print_sheet(category);
		else
			print_sheet(item_no);
	}
	print_total(1);

	fprintf(fout,".EOF\n");
	fclose(fout);
}

void
save_line (
 void)
{
	double	diff_value = 0.00;
    char cBuffer[256];

	diff_value = new_value - old_value;

	sprintf (cBuffer,"%-16.16s ",inmr_rec.mr_item_no);
        sort_save (fsort, cBuffer);
	sprintf (cBuffer,"%07ld ",inmr_rec.mr_hhbr_hash);
        sort_save (fsort, cBuffer);
	sprintf (cBuffer,"%-25.25s ",stts_rec.ts_serial_no);
        sort_save (fsort, cBuffer);
	sprintf (cBuffer,"%-10.10s ",stts_rec.ts_location);
        sort_save (fsort, cBuffer);
	sprintf (cBuffer,"%-1.1s ",stts_rec.ts_status);
        sort_save (fsort, cBuffer);
	sprintf (cBuffer,"%10.2f ",old_value);
        sort_save (fsort, cBuffer);
	sprintf (cBuffer,"%10.2f ",new_value);
        sort_save (fsort, cBuffer);
	sprintf (cBuffer,"%-1.1s ",stts_rec.ts_counted);
        sort_save (fsort, cBuffer);
	sprintf (cBuffer,"%12.2f\n",diff_value);
        sort_save (fsort, cBuffer);

	dsp_process("Item : ",inmr_rec.mr_item_no);
}

int
valid_group (
 void)
{
	if (strcmp(inmr_rec.mr_co_no,comm_rec.tco_no))
		return(0);

	if (print_type == ITEM	&& strcmp(inmr_rec.mr_item_no,upper) > 0)
		return(0);
	else
	if (print_type == GROUP)
	{
		if (inmr_rec.mr_class[0] > upper[0])
			return(0);

		if (inmr_rec.mr_class[0] < upper[0])
			return(1);

		if (strncmp(inmr_rec.mr_category,upper + 1,11) > 0)
			return(0);
	}

	return(1);
}

void
print_sheet (
 char *category)
{
	char	*sptr;
	char	*cptr;
	static	int	cat_print;
	long	hhbr_hash;

	fsort = sort_sort(fsort,"nstvar");

	sptr = sort_read(fsort);

	if (sptr != (char *)0)
	{
		hhbr_hash = atol(sptr + 17);
		
		cc = find_hash(sinmr,&sinmr_rec,COMPARISON,"r",hhbr_hash);

		if (!cc && print_type == GROUP)
		{
			strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
			sprintf(excf_rec.cf_cat_no,"%-11.11s",category);
			cc = find_rec(excf,&excf_rec,COMPARISON,"r");
			if (cc)
				strcpy(excf_rec.cf_cat_desc," ");

			cptr = expand(err_str,excf_rec.cf_cat_desc);

			fprintf(fout,"|%1.1s%11.11s  %-142.142s|\n",sinmr_rec.mr_class,category,cptr);
			cat_print = 1;
		}
	}

	while (sptr != (char *)0)
	{
		hhbr_hash = atol(sptr + 17);
		
		cc = find_hash(sinmr,&sinmr_rec,COMPARISON,"r",hhbr_hash);
		if (!cc)
			print_line(sptr);

		sptr = sort_read(fsort);
	}

	if (print_type == GROUP)
		print_total(0);

	sort_delete(fsort,"nstvar");
}

void
print_total (
 int j)
{
	fprintf(fout,"|------------");
	fprintf(fout,"|----------------");
	fprintf(fout,"|----------------------------------------");
	fprintf(fout,"|-------------------------");
	fprintf(fout,"|----------");
	fprintf(fout,"|----");
	fprintf(fout,"|----------");
	fprintf(fout,"|----------");
	fprintf(fout,"|-------");
	fprintf(fout,"|-------------|\n");
	
	if (print_type == GROUP)
		fprintf(fout,"|            ");
	fprintf(fout,"|                ");
	fprintf(fout,"|                                        ");
	fprintf(fout,"| Total for  %s      ",(j == 0) ? "Group  " : "Company");
	fprintf(fout,"|          ");
	fprintf(fout,"|    ");
	fprintf(fout,"|%10.2f",tot_old[j]);
	fprintf(fout,"|%10.2f",tot_new[j]);
	fprintf(fout,"|       ");
	fprintf(fout,"| %12.2f|\n",tot_var[j]);

	fflush(fout);

	if (j == 0)
	{
		fprintf(fout,"|------------");
		fprintf(fout,"|----------------");
		fprintf(fout,"|----------------------------------------");
		fprintf(fout,"|-------------------------");
		fprintf(fout,"|----------");
		fprintf(fout,"|----");
		fprintf(fout,"|----------");
		fprintf(fout,"|----------");
		fprintf(fout,"|-------");
		fprintf(fout,"|-------------|\n");
	}
	fflush(fout);

	tot_old[j] = 0.00;
	tot_new[j] = 0.00;
	tot_var[j]   = 0.00;
}

void
print_line (
 char *sptr)
{
	if (print_type == GROUP)
		fprintf(fout,"|%-11.11s "," ");
	fprintf(fout,"|%-16.16s",sinmr_rec.mr_item_no);
	fprintf(fout,"|%-40.40s",sinmr_rec.mr_description);
	fprintf(fout,"|%25.25s",sptr + 25);
	fprintf(fout,"|%10.10s",sptr + 51);
	fprintf(fout,"| %1.1s  ",sptr + 62);
	fprintf(fout,"|%10.10s",sptr + 64);
	fprintf(fout,"|%10.10s",sptr + 75);
	fprintf(fout,"|  %3.3s  ",((sptr + 86)[0] == 'Y') ? "Yes" : "No ");
	fprintf(fout,"| %12.12s|\n",sptr + 88);
	fflush(fout);

	tot_old[0] += f_val(sptr + 64);
	tot_new[0] += f_val(sptr + 75);
	tot_old[1] += f_val(sptr + 64);
	tot_new[1] += f_val(sptr + 75);

	tot_var[0] += d_val(sptr + 88);
	tot_var[1] += d_val(sptr + 88);
}

/*=========================
| Process transaction file|
=========================*/
void
proc_trans (
 long hhwh_hash)
{
	new_value = 0.00;
	old_value = 0.00;

	stts_rec.ts_hhwh_hash = hhwh_hash;
	sprintf(stts_rec.ts_serial_no,"%-25.25s", " ");
	cc = find_rec(stts, &stts_rec, GTEQ, "r");
	while (!cc && stts_rec.ts_hhwh_hash == hhwh_hash)
	{
		if (stts_rec.ts_counted[0] == 'Y')
			new_value = stts_rec.ts_cost;
		else
			new_value = 0.00;
	
		insf_rec.sf_hhwh_hash = stts_rec.ts_hhwh_hash;
		strcpy(insf_rec.sf_serial_no, stts_rec.ts_serial_no);
		strcpy(insf_rec.sf_status, stts_rec.ts_status);
		cc = find_rec(insf,&insf_rec,COMPARISON,"r");
		if (!cc)
			old_value = ser_value(insf_rec.sf_est_cost,insf_rec.sf_act_cost);
		else
			old_value = 0.00;

		if (print_type == GROUP && strcmp(category,inmr_rec.mr_category))
		{
			if (strcmp(category,"           "))
				print_sheet(category);

			fsort = sort_open("nstvar");
			strcpy(category,inmr_rec.mr_category);
		}
		else
		if (print_type == ITEM)
		{
			if (strcmp(item_no,"                "))
				print_sheet(item_no);

			fsort = sort_open("nstvar");
			strcpy(item_no,inmr_rec.mr_item_no);
		}

		no_rec = 0;
		save_line();

		cc = find_rec(stts, &stts_rec, NEXT, "r");
	}
}

float	
f_val (
 char *str)
{
	char	val[11];
	
	sprintf(val,"%-10.10s",str);
	return ((float) (atof(val)));
}

double	
d_val (
 char *str)
{
	char	val[13];
	
	sprintf(val,"%-12.12s",str);
	return(atof(val));
}
