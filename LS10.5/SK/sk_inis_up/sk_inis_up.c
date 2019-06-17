/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_inis_up.c   )                                 |
|  Program Desc  : ( Stock Inventory Supplier Record Update.      )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 19/12/88         |
|---------------------------------------------------------------------|
|  Date Modified : (19/12/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (17/04/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (03/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (07/11/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (12/03/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (01/10/97)      | Modified  by  : Marnie Organo    | 
|                                                                     |
|  Comments      : Change following vars from MONEYTYPE to DOUBLETYPE:|
|                : inis_fob_cost,local_rec.fob_cost,local_rec.new_cost|
|                : Remove DOLLARS on local_rec.pc_change,             |
|                : inis_fob_cost, local_rec.new_cost.                 |
|                : (03/08/90) - General Update for New Scrgen. S.B.D. |
|                : (07/11/91) - General Update.                       |
|                : (12/03/92) - Fixed output of % to printer for ICL  |
|                :              DRS6000.                              |
|                : (01/10/97) - Updated for Multilngual Conversion    |
|                :                                                    |
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: sk_inis_up.c,v $
| Revision 5.4  2002/07/18 07:15:54  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2001/09/21 09:23:23  robert
| Updated to used print_at instead of dsp_screen to fix display problems
| with LS10-GUI
|
| Revision 5.2  2001/08/09 09:18:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:01  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:14  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:14  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:29  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:10:53  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  2000/06/13 05:02:54  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.13  1999/12/06 01:30:49  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/11 05:59:40  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.11  1999/11/03 07:32:01  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.10  1999/10/20 01:38:57  nz
| Updated for remainder of old routines.
|
| Revision 1.9  1999/10/13 02:41:58  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.8  1999/10/08 05:32:25  scott
| First Pass checkin by Scott.
|
| Revision 1.7  1999/06/20 05:20:02  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_inis_up.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inis_up/sk_inis_up.c,v 5.4 2002/07/18 07:15:54 scott Exp $";

#define		MAXWIDTH	140
#define		MAXLINES	1000
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>


	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_description"},
		{"inmr_quick_code"},
	};

	int inmr_no_fields = 10;

	struct {
		char 	mr_co_no[3];
		char 	mr_item_no[17];
		long 	mr_hhbr_hash;
		long 	mr_hhsi_hash;
		char 	mr_alpha_code[17];
		char 	mr_super_no[17];
		char 	mr_maker_no[17];
		char 	mr_alternate[17];
		char 	mr_description[41];
		char	mr_quick_code[9];
	} inmr_rec;

	/*========================
	| Creditors Master File. |
	========================*/
	struct dbview sumr_list[] ={
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
	};

	int sumr_no_fields = 6;

	struct {
		char	sm_co_no[3];
		char	sm_est_no[3];
		char	sm_crd_no[7];
		long	sm_hhsu_hash;
		char	sm_name[41];
		char	sm_acronym[10];
	} sumr_rec;

	/*==================================+
	 | Stock Inventory Supplier Record. |
	 +==================================*/
#define	INIS_NO_FIELDS	26

	struct dbview	inis_list [INIS_NO_FIELDS] =
	{
		{"inis_co_no"},
		{"inis_br_no"},
		{"inis_wh_no"},
		{"inis_hhbr_hash"},
		{"inis_hhsu_hash"},
		{"inis_sup_part"},
		{"inis_sup_priority"},
		{"inis_hhis_hash"},
		{"inis_fob_cost"},
		{"inis_lcost_date"},
		{"inis_duty"},
		{"inis_licence"},
		{"inis_sup_uom"},
		{"inis_pur_conv"},
		{"inis_min_order"},
		{"inis_norm_order"},
		{"inis_ord_multiple"},
		{"inis_pallet_size"},
		{"inis_lead_time"},
		{"inis_sea_time"},
		{"inis_air_time"},
		{"inis_lnd_time"},
		{"inis_dflt_lead"},
		{"inis_weight"},
		{"inis_volume"},
		{"inis_stat_flag"}
	};

	struct tag_inisRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		long	hhbr_hash;
		long	hhsu_hash;
		char	sup_part [17];
		char	sup_priority [3];
		long	hhis_hash;
		double	fob_cost;
		Date	lcost_date;
		char	duty [3];
		char	licence [3];
		long	sup_uom;
		float	pur_conv;
		float	min_order;
		float	norm_order;
		float	ord_multiple;
		float	pallet_size;
		float	lead_time;
		float	sea_time;
		float	air_time;
		float	lnd_time;
		char	dflt_lead [2];
		float	weight;
		float	volume;
		char	stat_flag [2];
	}	inis_rec;

	char	systemDate[11];
	long	lsystemDate;
	int		cr_find;
	int		envDbCo;
	int		lpno;
	char	branchNumber[3];
	FILE	*fsort;
	FILE	*fout;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char 	crd_no[7];
	char 	item_no[2][17];
	char 	description[2][41];
	double	fob_cost;
	double	pc_change;
	double	new_cost;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "crd_no",	 4, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.crd_no},
	{1, LIN, "crd_name",	 4, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " - ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.sm_name},
	{1, LIN, "start_item_no",	 6, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "                ", "Start Item", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.item_no[0]},
	{1, LIN, "start_description",	 6, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " - ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.description[0]},
	{1, LIN, "end_item_no",	 7, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "~~~~~~~~~~~~~~~~", "End Item", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.item_no[1]},
	{1, LIN, "end_description",	 7, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " - ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.description[1]},
	{2, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Item  Number  ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.mr_item_no},
	{2, TAB, "description",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "         D e s c r i p t i o n          ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.mr_description},
	{2, TAB, "hhis_hash",	 0, 0, LONGTYPE,
		"NNNNNNNN", "          ",
		" ", "", " ", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&inis_rec.hhis_hash},
	{2, TAB, "sup_item_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Supplier Item. ", " ",
		 NA, NO,  JUSTLEFT, "", "", inis_rec.sup_part},
	{2, TAB, "inis_cost",	 0, 0, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", " Old Cost. ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&inis_rec.fob_cost},
	{2, TAB, "fob_cost",	 0, 0, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0.00", " New Cost. ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.fob_cost},
	{2, TAB, "pc_change",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", " % Change ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.pc_change},
	{2, TAB, "new_cost",	 0, 0, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", " New Cost. ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.new_cost},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void open_audit (void);
void CloseDB (void);
int  spec_valid (int field);
void load_inis (void);
void update (void);
int  heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 2)
	{
		print_at (0,0, mlStdMess036,argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	lpno = atoi(argv[1]);

	cr_find = atoi(get_env("CR_FIND"));
	envDbCo = atoi(get_env("CR_CO"));

	init_scr();			/*  sets terminal from termcap	*/
	set_tty(); 			/*  get into raw mode		*/
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/
	swide();

	strcpy (systemDate, DateToString (TodaysDate()));
	lsystemDate = TodaysDate ();

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB();

	strcpy (branchNumber,(envDbCo) ? comm_rec.test_no : " 0");

	open_audit();

	/*---------------------------------- 
	| Beginning of input control loop. |
	----------------------------------*/
	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;

		init_vars(1);
		init_vars(2);
		lcount[2] = 0;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		init_ok = 0;

		load_inis();

		if (lcount[2] == 0)
			continue;

		heading(2);
		scn_display(2);
		entry(2);
		if (restart)
			continue;

		heading(2);
		scn_display(2);
		edit(2);
		if (restart)
			continue;

		update(); 
	}
	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
	fprintf(fout,".EOF\n");
	pclose(fout);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec("inmr",inmr_list,inmr_no_fields,"inmr_id_no");
	open_rec("inis",inis_list,INIS_NO_FIELDS,"inis_id_no3");
	open_rec("sumr",sumr_list,sumr_no_fields,(cr_find) 	? "sumr_id_no3" 
							   							: "sumr_id_no");
}	

void
open_audit (
 void)
{
	if ((fout = popen("pformat","w")) == 0)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);

	fprintf(fout,".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",lpno);
	fprintf(fout,".10\n");
	fprintf(fout,".PI12\n");
	fprintf(fout,".L152\n");
	fprintf(fout,".EInventory Supplier Cost Update Audit\n");
	fprintf(fout,".E%s\n",clip(comm_rec.tco_name));
	fprintf(fout,".E%s\n",clip(comm_rec.test_name));
	fprintf(fout,".EAs At %s\n",SystemTime());

	fprintf(fout,".R====================");
	fprintf(fout,"===========================================");
	fprintf(fout,"===================");
	fprintf(fout,"=============");
	fprintf(fout,"=============");
	fprintf(fout,"===========\n");

	fprintf(fout,"====================");
	fprintf(fout,"===========================================");
	fprintf(fout,"===================");
	fprintf(fout,"=============");
	fprintf(fout,"=============");
	fprintf(fout,"===========\n");

	fprintf(fout,"| ITEM NUMBER      |");
	fprintf(fout,"  D E S C R I P T I O N                   |");
	fprintf(fout,"  SUPPLIER'S ITEM |");
	fprintf(fout,"  OLD COST  |");
	fprintf(fout,"  NEW COST  |");
	fprintf(fout,"  %% DIFF  |\n");

	fprintf(fout,"|------------------|");
	fprintf(fout,"------------------------------------------|");
	fprintf(fout,"------------------|");
	fprintf(fout,"------------|");
	fprintf(fout,"------------|");
	fprintf(fout,"----------|\n");

	fflush(fout);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose("inmr");
	abc_fclose("inis");
	abc_fclose("sumr");
	SearchFindClose ();
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK("crd_no"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		strcpy (sumr_rec.sm_co_no,comm_rec.tco_no);
		strcpy (sumr_rec.sm_est_no,branchNumber);
		strcpy (sumr_rec.sm_crd_no,pad_num(local_rec.crd_no));
		cc = find_rec("sumr",&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess022));
			sleep(2);
			clear_mess();
			return(1);
		}

		DSP_FLD( "crd_name" );
		return(0);
	}

	if (LCHECK("start_item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}

		if (dflt_used)
		{
			sprintf(local_rec.description[0],"%-40.40s","** First Item **");
			display_field(field + 1);
			return(0);
		}

		cc = FindInmr (comm_rec.tco_no, local_rec.item_no [0], 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.item_no [0]);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess(ML(mlStdMess001));
			sleep(2);
			clear_mess();
			return(1);
		}
		SuperSynonymError ();

		strcpy (local_rec.item_no[0], inmr_rec.mr_item_no);
		strcpy (local_rec.description[0],inmr_rec.mr_description);
		
		DSP_FLD( "start_item_no" );
		DSP_FLD( "start_description" );
		return(0);
	}

	if (LCHECK("end_item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}

		if (dflt_used)
		{
			sprintf(local_rec.description[1],"%-40.40s","** Last Item **");
			display_field(field + 1);
			return(0);
		}
		cc = FindInmr (comm_rec.tco_no, local_rec.item_no [1], 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.item_no [1]);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess( ML(mlStdMess001) );
			sleep(2);
			clear_mess();
			return(1);
		}
		SuperSynonymError ();

		strcpy (local_rec.item_no[1], inmr_rec.mr_item_no);
		strcpy (local_rec.description[1],inmr_rec.mr_description);
		
		DSP_FLD( "end_item_no" );
		DSP_FLD( "end_description" );
		return(0);
	}
			
	if (LCHECK("item_no"))
	{
		getval(line_cnt);
		return(0);
	}

	if (LCHECK("fob_cost"))
	{
		if (dflt_used)
			local_rec.fob_cost	=	inis_rec.fob_cost;

		if (local_rec.fob_cost != 0.00)
			skip_entry = 1;

		local_rec.new_cost = local_rec.fob_cost;
		DSP_FLD( "new_cost" );

		/*-------------------------------
		| Calculate Percentage Change	|
		-------------------------------*/
		local_rec.pc_change = ( local_rec.fob_cost - inis_rec.fob_cost);

		if (inis_rec.fob_cost != 0.00)
			local_rec.pc_change /= inis_rec.fob_cost;
		else
			local_rec.pc_change = 0.00;

		local_rec.pc_change *= 100.00;
		DSP_FLD("pc_change");
		return(0);
	}

	if (LCHECK("pc_change"))
	{
		if (local_rec.pc_change == 0.00)
		{
			local_rec.fob_cost = inis_rec.fob_cost;
			local_rec.new_cost = inis_rec.fob_cost;
		}
		else
		{
			local_rec.fob_cost = local_rec.pc_change / 100;
			local_rec.fob_cost += 1.00;
			local_rec.fob_cost *= inis_rec.fob_cost;
			local_rec.new_cost = local_rec.fob_cost;
		}
		DSP_FLD("fob_cost");
		DSP_FLD("new_cost");
		return(0);
	}

	return(0);
}

void
load_inis (
 void)
{
	char	*sptr;
	long	hhis_hash;

/*
	dsp_screen("Reading Inventory Supplier Lines",
					comm_rec.tco_no,comm_rec.tco_name);
*/
	print_at (2,1, "Reading Inventory Supplier Lines... Item No: ");

	abc_selfield("inis","inis_id_no3");
	abc_selfield("inmr","inmr_hhbr_hash");

	fsort = sort_open("inis_up");

	inis_rec.hhsu_hash = sumr_rec.sm_hhsu_hash;
	inis_rec.hhbr_hash = 0L;
	strcpy (inis_rec.co_no,"  ");
	strcpy (inis_rec.br_no,"  ");
	strcpy (inis_rec.wh_no,"  ");

	cc = find_rec("inis",&inis_rec,GTEQ,"r");

	while (!cc && inis_rec.hhsu_hash == sumr_rec.sm_hhsu_hash)
	{
		/*-----------
		| Find Inmr	|
		-----------*/
		inmr_rec.mr_hhbr_hash	=	inis_rec.hhbr_hash;
		cc = find_rec("inmr",&inmr_rec,COMPARISON,"r");

		/*-------------------------------
		| Found inmr & valid inmr	|
		-------------------------------*/
		if (!cc && strcmp(inmr_rec.mr_item_no,local_rec.item_no[0]) >= 0 && 
				   strcmp(inmr_rec.mr_item_no,local_rec.item_no[1]) <= 0)
		{
			//dsp_process("Item No : ",inmr_rec.mr_item_no);
			print_at (2, 50, inmr_rec.mr_item_no);

			sprintf(err_str,"%s %06ld\n", inmr_rec.mr_item_no, inis_rec.hhis_hash);
			sort_save(fsort,err_str);
		}
		cc = find_rec("inis",&inis_rec,NEXT,"r");
	}

	fsort = sort_sort(fsort,"inis_up");

/*
	dsp_screen("Loading Inventory Supplier Lines",
					comm_rec.tco_no,comm_rec.tco_name);
*/
	print_at (2,1, "Loading Inventory Supplier Lines... Item No: ");

	abc_selfield("inis","inis_hhis_hash");

	scn_set(2);
	lcount[2] = 0;

	sptr = sort_read(fsort);
	
	while (sptr != (char *)0)
	{
		hhis_hash = atol(sptr + 17);

		inis_rec.hhis_hash	=	hhis_hash;
		cc = find_rec ("inis",&inis_rec,COMPARISON,"u");
		if (!cc)
		{
			inmr_rec.mr_hhbr_hash	=	inis_rec.hhbr_hash;
			cc = find_rec ("inmr",&inmr_rec,COMPARISON,"r");
			if (!cc)
			{
				//dsp_process("Item No : ",inmr_rec.mr_item_no);				
				print_at (2,50, inmr_rec.mr_item_no);

				local_rec.fob_cost = 0.00;
				local_rec.pc_change = 0.00;
				local_rec.new_cost = 0.00;
				putval(lcount[2]++);
				if (lcount[2] > MAXLINES)
					break;
			}
			else
				abc_unlock("inis");
		}
		abc_unlock("inis");
		
		sptr = sort_read(fsort);
	}
	
	sort_delete(fsort,"inis_up");
	vars[scn_start].row = lcount[2];
	abc_selfield("inmr","inmr_id_no");
}

/*=============================
| Update All inventory files. |
=============================*/
void
update (
 void)
{
	clear();

	print_at (0,0, ML(mlStdMess035));
	fflush(stdout);

	scn_set(2);

	for (line_cnt = 0;line_cnt < lcount[2];line_cnt++)
	{
		getval(line_cnt);

		cc = find_rec("inis",&inis_rec,COMPARISON,"u");
		if (cc)
		{
			abc_unlock ("inis");
			continue;
		}

		putchar('U');
		fflush(stdout);

		if (inis_rec.fob_cost != local_rec.new_cost)
		{
			fprintf(fout,".PD| %-6.6s - %-107.107s|\n",sumr_rec.sm_crd_no,sumr_rec.sm_name);

			fprintf(fout,"| %-16.16s |",inmr_rec.mr_item_no);
			fprintf(fout," %-40.40s |",inmr_rec.mr_description);
			fprintf(fout," %-16.16s |",inis_rec.sup_part);
			fprintf(fout," %10.2f |",inis_rec.fob_cost);
			fprintf(fout," %10.2f |",local_rec.new_cost);
			fprintf(fout," %8.2f |\n",local_rec.pc_change);
		}

		inis_rec.fob_cost = local_rec.new_cost;
		inis_rec.lcost_date = lsystemDate;

		cc = abc_update("inis",&inis_rec);
		if (cc)
			sys_err("Error in inis during (DBUPDATE)",cc,PNAME);
	}
}

/*=================================================================
| Heading concerns itself with clearing the screen,painting the  |
| screen overlay in preparation for input.                        |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		swide();
		clear();

		rv_pr(ML(mlSkMess334),50,0,1);

		move(0,1);
		line(132);

		switch (scn)
		{
		case	1:
			box(0,3,132,4);
			move(1,5);
			line(131);
			break;

		case	2:
			break;

		default:
			break;
		}

		move(0,20);
		line(132);
		strcpy (err_str, ML(mlStdMess038));
		print_at (21,0, err_str, comm_rec.tco_no, comm_rec.tco_name);
		strcpy (err_str, ML(mlStdMess039));
		print_at (22,0, err_str, comm_rec.test_no, comm_rec.test_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
