/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (sk_st_sinp.c  )                                   |
|  Program Desc  : (Stock Take Input for Serial Items.          )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, incc, insc, stts,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  stts,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written : 13/07/89          |
|---------------------------------------------------------------------|
|  Date Modified : (11/10/89)      | Modified  by : Fui Choo Yap      |
|                : (28/08/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (18/07/91)      | Modified  by : Trevor van Bremen |
|  Date Modified : (12/03/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (16/06/92)      | Modified  by : Simon Dubey.      |
|  Date Modified : (26/05/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (17/01/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (01/10/97)      | Modified  by : Rowena S Maandig  |
|                :                                                    |
|                                                                     |
|  Comments      : Take out all the checks on mult_loc -              |
|                : which does not apply to serial items.              |
|                : (28/08/90) - General Update for New Scrgen. S.B.D. |
|                : (18/07/91) - Chgs for quick code & genl tidy-up.   |
|                : (12/03/92) - Change label ser_no to serial_no.     |
|                : (16/06/92) - exclude values of SK_IVAL_CLASS from  |
|                :              stock take SC DFH 7096                |
|                :                                                    |
|  (26/05/93)    : KIL 9046. Change default for audit from N to Y.    |
|                : Remove validation of location against lomr.  The   |
|                : location field for serial items is purely a text   |
|                : narrative field, regardless of whether MULTIBIN is |
|                : active or not.			                          |
|  (17/01/94)    : DHL 10210. Fix skipping of serial number field if  |
|                : users exits from FN13 mode having changed a serial |
|                : number.                                            |
|  (01/10/97)    : Updated to incorporate multilingual conversion.    |
|                :                                                    |
|                                                                     |
| $Log: sk_st_sinp.c,v $
| Revision 5.7  2002/07/24 08:39:18  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.6  2002/07/18 07:15:55  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.5  2002/07/17 09:58:01  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/06/20 07:11:10  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/10/11 03:00:04  robert
| updated to avoid overlapping of label description
|
| Revision 5.2  2001/08/09 09:20:04  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:39:05  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:21:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:12:01  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.16  2000/07/10 01:53:44  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.15  2000/06/13 05:03:31  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.14  2000/01/21 02:35:47  cam
| Changes for GVision compatibility.  Fixed calls to print_mess ().
|
| Revision 1.13  1999/12/06 01:31:20  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/11 06:00:08  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.11  1999/11/03 07:32:35  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.10  1999/10/13 02:42:16  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.9  1999/10/08 05:32:55  scott
| First Pass checkin by Scott.
|
| Revision 1.8  1999/06/20 05:20:46  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_st_sinp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_st_sinp/sk_st_sinp.c,v 5.7 2002/07/24 08:39:18 scott Exp $";

#define	MAXWIDTH	150
#define	MAXLINES	500
#include 	<pslscr.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>

#define	SERIAL		 (inmr_rec.mr_serial_item [0] == 'Y')
#define	AUDIT		 (local_rec.audit_value [0] == 'Y')
#define	HHBR_HASH	store [line_cnt]._hhbr_hash
#define	HHWH_HASH	store [line_cnt]._hhwh_hash
#define	COST    	store [line_cnt]._cost
#define	NEW_REC    	store [line_cnt]._new

	FILE	*fout;

	char	*data = "data";
	char	*comm = "comm";
	char	*ccmr = "ccmr";
	char	*incc = "incc";
	char	*insc = "insc";
	char	*inmr = "inmr";
	char	*stts = "stts";

	char	*inval_cls;
 	char 	*result;

	struct	storeRec {
		long	_hhbr_hash;
		long	_hhwh_hash;
		char	_ser_no [26];
		double	_cost;
		int	_new;
	} store [MAXLINES];

	char	*ser_space = "                         ";

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_cc_short"},
	};

	int comm_no_fields = 10;

	struct {
		int  termno;
		char tco_no [3];
		char tco_name [41];
		char tco_short [16];
		char test_no [3];
		char test_name [41];
		char test_short [16];
		char tcc_no [3];
		char tcc_name [41];
		char tcc_short [10];
	} comm_rec;

	/*==================================================
	| Inventory Stock Take Control File                |
	==================================================*/
	struct dbview insc_list [] ={
		{"insc_hhcc_hash"},
		{"insc_stake_code"},
		{"insc_start_date"},
		{"insc_start_time"},
		{"insc_description"},
	};

	int	insc_no_fields = 5;

	struct	{
		long	hhcc_hash;
		char	stake_code [2];
		long	start_date;
		char	start_time [6];
		char	description [41];
	} insc_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list [] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
	};

	int	ccmr_no_fields = 4;

	struct	{
		char	cm_co_no [3];
		char	cm_est_no [3];
		char	cm_cc_no [3];
		long	cm_hhcc_hash;
	} ccmr_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_quick_code"},
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
	};

	int	inmr_no_fields = 12;

	struct	{
		char	mr_co_no [3];
		char	mr_item_no [17];
		long	mr_hhbr_hash;
		long	mr_hhsi_hash;
		char	mr_alpha_code [17];
		char	mr_super_no [17];
		char	mr_maker_no [17];
		char	mr_alternate [17];
		char	mr_class [2];
		char	mr_description [41];
		char	mr_quick_code [9];
		char	mr_serial_item [2];
		char	mr_costing_flag [2];
	} inmr_rec;

	/*===================================
	| Inventory Cost centre Base Record |
	===================================*/
	struct dbview incc_list [] ={
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_hhwh_hash"},
		{"incc_location"},
		{"incc_stat_flag"},
	};

	int	incc_no_fields = 5;

	struct	{
		long	cc_hhcc_hash;
		long	cc_hhbr_hash;
		long	cc_hhwh_hash;
		char	cc_location [11];
		char	cc_stat_flag [2];
	} incc_rec;

	/*=============================
	| Stock Take Transaction File |
	=============================*/
	struct dbview stts_list [] ={
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
		char	ts_serial_no [26];
		double	ts_cost;
		char	ts_location [11];
		char	ts_status [2];
		char	ts_counted [2];
		char	ts_stat_flag [2];
	} stts_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	audit [6];
	char	audit_value [2];
	int		printerNumber;
	char	item_desc [41];
	char	serialNumber [26];
	double	cost;
	char	location [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "stake_code",	 4, 30, CHARTYPE,
		"U", "          ",
		" ", "", "Stock Take Selection Code.", "",
		 NE, NO,  JUSTLEFT, "", "", insc_rec.stake_code},
	{1, LIN, "stake_desc",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Stock Selection Desc", "",
		 NA, NO,  JUSTLEFT, "", "", insc_rec.description},
	{1, LIN, "stake_date",	 6, 30, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Start Date", "",
		 NA, NO,  JUSTLEFT, "", "", (char *)&insc_rec.start_date},
	{1, LIN, "stake_time",	 7, 30, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "Start Time", "",
		 NA, NO,  JUSTLEFT, "", "", insc_rec.start_time},
	{1, LIN, "audit",	 9, 30, CHARTYPE,
		"U", "          ",
		" ", "Y", "Audit Required", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.audit_value},
	{1, LIN, "audit_desc",	 9, 35, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.audit},
	{1, LIN, "printerNumber",	10, 30, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer", "",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{2, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item  Number  ", "",
		YES, NO,  JUSTLEFT, "", "", inmr_rec.mr_item_no},
	{2, TAB, "desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "          D e s c r i p t i o n         ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{2, TAB, "serialNumber",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "      Serial Number      ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.serialNumber},
	{2, TAB, "location",	 0, 0, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", " Location ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.location},
	{2, TAB, "cost",	 0, 0, DOUBLETYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0.00", "    Cost    ", "",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.cost},
	{0, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*======================= 
| Function Declarations |
=======================*/
int  spec_valid (int field);
int  delete_line (int scn);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void show_insc (char *key_val);
void show_serial (char *key_val);
void update (void);
void head_audit (void);
void tail_audit (void);
int  heading (int scn);
int  chk_dup_ser (char *serialNumber, long hhwh_hash, int line_no);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
	{	
		inval_cls = strdup (sptr);
	}
	else
		inval_cls = "ZKPN";
	upshift (inval_cls); 

	SETUP_SCR (vars);


	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	OpenDB ();


	swide ();
	while (prog_exit == 0)
	{
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (2);
		entry (2);
		if (restart)
			continue;

		edit_all ();
		if (restart)
			continue;

		update ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	int	res;
	double	old_cost = 0.00;

	if (LCHECK ("stake_code"))
	{
		if (SRCH_KEY)
		{
			show_insc (temp_str);
			return (EXIT_SUCCESS);
		}

		insc_rec.hhcc_hash = ccmr_rec.cm_hhcc_hash;
		cc = find_rec (insc,&insc_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf (err_str,ML (mlSkMess047),insc_rec.stake_code);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		display_field (field + 1);
		display_field (field + 2);
		display_field (field + 3);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("audit"))
	{
		strcpy (local_rec.audit, (AUDIT) ? "Y (es" : "N (o ");
		DSP_FLD ("audit_desc");
		FLD ("printerNumber") = (AUDIT) ? YES : NA;
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (AUDIT && !valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (delete_line (2));

		cc = FindInmr (comm_rec.tco_no, inmr_rec.mr_item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, inmr_rec.mr_item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ((result = strstr (inval_cls, inmr_rec.mr_class)))
		{
			sprintf (err_str,ML (mlSkMess211),inmr_rec.mr_item_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		if (!SERIAL)
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		incc_rec.cc_hhcc_hash = ccmr_rec.cm_hhcc_hash;
		incc_rec.cc_hhbr_hash = inmr_rec.mr_hhbr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess192));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		if (incc_rec.cc_stat_flag [0] != insc_rec.stake_code [0])
		{
			sprintf (err_str,ML (mlSkMess454),incc_rec.cc_stat_flag);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.item_desc,"%-40.40s",inmr_rec.mr_description);
		DSP_FLD ("desc");

		HHBR_HASH = inmr_rec.mr_hhbr_hash;
		HHWH_HASH = incc_rec.cc_hhwh_hash;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("serialNumber"))
	{
		if (SRCH_KEY)
		{
			show_serial (temp_str);
			return (EXIT_SUCCESS);
		}

		if (chk_dup_ser (local_rec.serialNumber,HHWH_HASH,line_cnt))
		{
			print_mess (ML (mlSkMess559));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		stts_rec.ts_hhwh_hash = incc_rec.cc_hhwh_hash; 
		strcpy (stts_rec.ts_serial_no,local_rec.serialNumber);
		cc = find_rec (stts,&stts_rec,COMPARISON,"r");
		if (cc) 
		{
			res = prmptmsg (ML (mlSkMess455),"YyNn",1,2);
			if (res != 'Y' && res != 'y')
			{
				sprintf (inmr_rec.mr_item_no,"%16.16s"," ");
				sprintf (local_rec.item_desc,"%40.40s"," ");
				sprintf (local_rec.serialNumber,"%25.25s"," ");
				DSP_FLD ("desc");
				skip_entry = -3;
				return (EXIT_SUCCESS);
			}
			move (1,2);
			cl_line ();

			NEW_REC = 1;
			sprintf (store [line_cnt]._ser_no,"%-25.25s",local_rec.serialNumber);
			return (EXIT_SUCCESS);
		}

		NEW_REC = 0;
		sprintf (store [line_cnt]._ser_no,"%-25.25s",local_rec.serialNumber);
		sprintf (local_rec.location,stts_rec.ts_location);
		local_rec.cost = stts_rec.ts_cost;
		COST = stts_rec.ts_cost;
		HHWH_HASH = stts_rec.ts_hhwh_hash;
		DSP_FLD ("serialNumber");
		DSP_FLD ("location");
		DSP_FLD ("cost");
		if (prog_status == ENTRY)
			skip_entry = 2;
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("cost"))
	{
		old_cost = atof (prv_ntry);
		COST = local_rec.cost;
		if (prog_status != ENTRY && old_cost != COST && !NEW_REC) 
		{
			print_mess (ML (mlSkMess456));
			sleep (sleepTime);
			clear_mess ();
			local_rec.cost = atof (prv_ntry);
			DSP_FLD ("cost");
			return (EXIT_SUCCESS); 
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
delete_line (
 int scn)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (lcount [scn] == 0)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	print_at (2,0,ML (mlStdMess035));
	fflush (stdout);

	lcount [scn]--;

	for (i = line_cnt;line_cnt < lcount [scn];line_cnt++)
	{
		store [line_cnt]._hhbr_hash = store [line_cnt + 1]._hhbr_hash;
		store [line_cnt]._hhwh_hash = store [line_cnt + 1]._hhwh_hash;
		sprintf (store [line_cnt]._ser_no,store [line_cnt + 1]._ser_no);
		store [line_cnt]._cost = store [line_cnt + 1]._cost;
		store [line_cnt]._new = store [line_cnt + 1]._new;
		getval (line_cnt + 1);
		putval (line_cnt);
	
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	sprintf (inmr_rec.mr_item_no,"%-16.16s"," ");
	sprintf (local_rec.item_desc,"%-40.40s"," ");
	sprintf (local_rec.serialNumber,"%-25.25s"," ");
	sprintf (local_rec.location,"%-10.10s"," ");
	local_rec.cost = 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	print_at (2,0,"                   ");
	fflush (stdout);
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	ReadMisc ();

	open_rec (inmr,inmr_list,inmr_no_fields,"inmr_id_no");
	open_rec (incc,incc_list,incc_no_fields,"incc_id_no");
	open_rec (insc,insc_list,insc_no_fields,"insc_id_no");
	open_rec (stts,stts_list,stts_no_fields,"stts_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (insc);
	abc_fclose (stts);
	SearchFindClose ();
	abc_dbclose (data);
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (ccmr,ccmr_list,ccmr_no_fields,"ccmr_id_no");
	strcpy (ccmr_rec.cm_co_no,comm_rec.tco_no);
	strcpy (ccmr_rec.cm_est_no,comm_rec.test_no);
	strcpy (ccmr_rec.cm_cc_no,comm_rec.tcc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)",cc,PNAME);

	abc_fclose (ccmr);
}

void
show_insc (
 char *key_val)
{
	work_open ();
	save_rec ("# ","#Stock Take Selection Description");
	insc_rec.hhcc_hash = ccmr_rec.cm_hhcc_hash;
	strcpy (insc_rec.stake_code," ");
	cc = find_rec (insc,&insc_rec,GTEQ,"r");
	while (!cc && insc_rec.hhcc_hash == ccmr_rec.cm_hhcc_hash)
	{
		cc = save_rec (insc_rec.stake_code,insc_rec.description);
		if (cc)
			break;

		cc = find_rec (insc,&insc_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	insc_rec.hhcc_hash = ccmr_rec.cm_hhcc_hash;
	strcpy (insc_rec.stake_code,temp_str);
	cc = find_rec (insc,&insc_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in insc During (DBFIND)",cc,PNAME);
}

void
show_serial (
 char *key_val)
{
	char	counted [4];

	work_open ();
	save_rec ("#Serial Number","#Counted");
	stts_rec.ts_hhwh_hash = incc_rec.cc_hhwh_hash;
	sprintf (stts_rec.ts_serial_no,"%-25.25s",key_val);
	cc = find_rec (stts,&stts_rec,GTEQ,"r");
	while (!cc && !strncmp (stts_rec.ts_serial_no,key_val,strlen (key_val)) && stts_rec.ts_hhwh_hash == incc_rec.cc_hhwh_hash)
	{
		strcpy (counted, (stts_rec.ts_counted [0] == 'Y') ? "Yes" : "No ");
		cc = save_rec (stts_rec.ts_serial_no,counted);
		if (cc)
			break;

		cc = find_rec (stts,&stts_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	stts_rec.ts_hhwh_hash = incc_rec.cc_hhwh_hash;
	sprintf (stts_rec.ts_serial_no,"%-25.25s",temp_str);
	cc = find_rec (stts,&stts_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in stts During (DBFIND)",cc,PNAME);
}

void
update (
 void)
{
	if (AUDIT)
		head_audit ();
	else
	{
		clear ();
		print_at (0,0,ML (mlStdMess035));
		fflush (stdout);
	}

	scn_set (2);

	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);

		/*----------------------------------
		| Add new stts with status = N (ew) |
		----------------------------------*/
		if (NEW_REC)
		{
			if (!AUDIT)
			{
				putchar ('A');
				fflush (stdout);
			}
			stts_rec.ts_hhwh_hash = HHWH_HASH;
			strcpy (stts_rec.ts_serial_no,local_rec.serialNumber);
			stts_rec.ts_cost = COST;
			strcpy (stts_rec.ts_location,local_rec.location);
			strcpy (stts_rec.ts_status,"N");
			strcpy (stts_rec.ts_counted,"Y");
			strcpy (stts_rec.ts_stat_flag,"N");

			cc = abc_add (stts,&stts_rec);
			if (cc)
				sys_err ("Error in stts During (DBADD)",cc,PNAME);
		}
		/*-----------------------------------------------
		| Update existing stts with status = E (xisting	|
		-----------------------------------------------*/
		else
		{
			if (!AUDIT)
			{
				putchar ('A');
				fflush (stdout);
			}
			stts_rec.ts_hhwh_hash = HHWH_HASH;
			strcpy (stts_rec.ts_serial_no,local_rec.serialNumber);
			cc = find_rec (stts,&stts_rec,COMPARISON,"u");
			if (cc)
				sys_err ("Error in stts During (DBFIND)",cc,PNAME);
			strcpy (stts_rec.ts_location,local_rec.location);
			strcpy (stts_rec.ts_counted,"Y");
			strcpy (stts_rec.ts_stat_flag,"E");

			cc = abc_update (stts,&stts_rec);
			if (cc)
				sys_err ("Error in stts During (DBUPDATE)",cc,PNAME);
		}
		abc_unlock (stts);

		if (AUDIT)
		{
			dsp_process (" Item : ",inmr_rec.mr_item_no);

			fprintf (fout,"|%-16.16s",inmr_rec.mr_item_no);
			fprintf (fout,"|%-40.40s",local_rec.item_desc);
			fprintf (fout,"|%-25.25s",local_rec.serialNumber);
			fprintf (fout,"|%-10.10s",local_rec.location);
			fprintf (fout,"|%12.2f",stts_rec.ts_cost);
			fprintf (fout,"|  %s  |\n", (stts_rec.ts_counted [0] == 'Y') ? "Yes" : "No ");

		}
	}
	abc_unlock (stts);

	if (AUDIT)
		tail_audit ();
}

void
head_audit (
 void)
{
	dsp_screen (" Printing Stock Take Input Audit ",comm_rec.tco_no,comm_rec.tco_name);

	if ((fout = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);
		
	fprintf (fout,".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNumber);
	fprintf (fout,".10\n");
	fprintf (fout,".L130\n");
	fprintf (fout,".ESTOCK TAKE SERIAL INPUT AUDIT\n");
	fprintf (fout,".E%s %s\n",comm_rec.tco_no,clip (comm_rec.tco_name));
	fprintf (fout,".E%s %s\n",comm_rec.test_no,clip (comm_rec.test_name));
	fprintf (fout,".E%s %s\n",comm_rec.tcc_no,clip (comm_rec.tcc_name));
	fprintf (fout,".EAS AT %s\n",SystemTime ());

	fprintf (fout,".R=================");
	fprintf (fout,"=========================================");
	fprintf (fout,"==========================");
	fprintf (fout,"===========");
	fprintf (fout,"=============");
	fprintf (fout,"=========\n");

	fprintf (fout,"=================");
	fprintf (fout,"=========================================");
	fprintf (fout,"==========================");
	fprintf (fout,"===========");
	fprintf (fout,"=============");
	fprintf (fout,"=========\n");

	fprintf (fout,"|  ITEM  NUMBER  ");
	fprintf (fout,"|          D E S C R I P T I O N         ");
	fprintf (fout,"|  SERIAL  NUMBER         ");
	fprintf (fout,"| LOCATION ");
	fprintf (fout,"|   COST     ");
	fprintf (fout,"|COUNTED|\n");

	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------------------------------");
	fprintf (fout,"|-------------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|------------");
	fprintf (fout,"|-------|\n");
	fflush (fout);
}

void
tail_audit (
 void)
{
	fprintf (fout,".EOF\n");
	fclose (fout);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlSkMess457),45,0,1);
		move (0,1);
		line (132);

		if (scn == 1)
		{
			box (0,3,132,7);
			move (1,8);
			line (131);
			move (0,19);
			line (132);
			print_at (20,0,ML (mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
			print_at (21,0,ML (mlStdMess039),comm_rec.test_no,comm_rec.test_name);
			print_at (22,0,ML (mlStdMess099),comm_rec.tcc_no,comm_rec.tcc_name);
		}
		else
		{
			/* Current Selection Code  */
			print_at (3,0,ML (mlSkMess458),insc_rec.stake_code, insc_rec.description);
			move (0,19);
			line (132);
			print_at (20,0,ML (mlStdMess038),comm_rec.tco_no,comm_rec.tco_short);
			print_at (21,0,ML (mlStdMess039),comm_rec.test_no,comm_rec.test_short);
			print_at (22,0,ML (mlStdMess099),comm_rec.tcc_no,comm_rec.tcc_short);
		}
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*=======================================================
| Check Whether A Serial Number For This Item Number	|
| Has Already Been Used.				|
| Return 1 if duplicate					|
=======================================================*/
int
chk_dup_ser (
 char *serialNumber, 
 long hhwh_hash, 
 int line_no)
{
	int	i;
	int	no_items = (prog_status == ENTRY) ? line_cnt : lcount [2];

	for (i = 0;i < no_items;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*---------------------------------------
		| cannot duplicate item_no/serial_no	|
		| unless serial no was not input	|
		---------------------------------------*/
		if (!strcmp (store [i]._ser_no,ser_space))
			continue;

		/*---------------------------------------
		| Only compare serial numbers for	|
		| the same hhwh_hash  			|
		---------------------------------------*/
		if (store [i]._hhwh_hash == hhwh_hash)
		{
			if (!strcmp (store [i]._ser_no,serialNumber))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}
