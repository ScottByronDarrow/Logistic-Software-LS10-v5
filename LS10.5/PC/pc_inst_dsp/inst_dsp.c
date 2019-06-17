/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_inst_dsp.c                              )     |
|  Program Desc  : ( Instruction Display / Print                )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, pcmi, pcid,     ,     ,     ,     ,         |
|                :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (bomm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pcmi, pcid,     ,     ,     ,     ,     ,         |
|  Database      : (bomm)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written : 11/11/91          |
|---------------------------------------------------------------------|
|  Date Modified : (08/09/97)      | Modified  by : Marnie Organo     |
|                                                                     |
|  Comments      : (08/09/97) - Modified for Multilingual Conversion. |
|                :                                                    |
|                                                                     |
| $Log: inst_dsp.c,v $
| Revision 5.3  2002/07/17 09:57:28  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:38  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:59  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:13  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:23  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:59  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:09  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.12  2000/06/13 05:02:07  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.11  1999/11/12 10:37:45  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.10  1999/10/01 07:48:53  scott
| Updated for standard function calls.
|
| Revision 1.9  1999/09/29 10:11:33  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 08:26:22  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.7  1999/09/13 07:03:13  marlene
| *** empty log message ***
|
| Revision 1.6  1999/09/09 06:12:24  marlene
| *** empty log message ***
|
| Revision 1.5  1999/06/17 07:40:42  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: inst_dsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_inst_dsp/inst_dsp.c,v 5.3 2002/07/17 09:57:28 scott Exp $";

#define	X_OFF		10
#define	Y_OFF		2
#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>

#define	ITEM	0
#define	CNTR	1
#define	INST	2
#define	VERS	3

#define	DETL_SCN	1
#define	MAST_SCN	2

#define	DISP		( local_rec.output[0] == 'D' )
#define	MASTER		( run_prog == MAST_SCN )
#define	ITEM_SPECIFIC	( !strcmp(local_rec.st_item, local_rec.end_item) )
#define	CNTR_SPECIFIC	(!strcmp(local_rec.st_wrk_cntr, local_rec.end_wrk_cntr))
#define	ALL_INST	( !strcmp(local_rec.instr_no, "ALL") )
#define	LATEST_VER	( !strcmp(local_rec.ver_num, "LATEST") )

char	*UNDERLINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

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
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
	} comm_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
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
		{"inmr_description2"},
		{"inmr_quick_code"},
		{"inmr_dec_pt"},
		{"inmr_std_uom"},
		{"inmr_alt_uom"},
		{"inmr_uom_cfactor"},
	};

	int	inmr_no_fields = 15;

	struct	{
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		long	mr_hhsi_hash;
		char	mr_alpha_code[17];
		char	mr_super_no[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_description[41];
		char	mr_description2[41];
		char	mr_quick_code[9];
		int	mr_dec_pt;
		long	mr_std_uom;
		long	mr_alt_uom;
		float	mr_uom_cfactor;
	} inmr_rec;

	/*=========================================
	| Process Control Instruction Detail File |
	=========================================*/
	struct dbview pcid_list[] ={
		{"pcid_co_no"},
		{"pcid_hhbr_hash"},
		{"pcid_hhwc_hash"},
		{"pcid_instr_no"},
		{"pcid_line_no"},
		{"pcid_version"},
		{"pcid_text"},
	};

	int	pcid_no_fields = 7;

	struct	{
		char	id_co_no[3];
		long	id_hhbr_hash;
		long	id_hhwc_hash;
		int	id_instr_no;
		int	id_line_no;
		int	id_version;
		char	id_text[61];
	} wkid_rec, pcid_rec;

	/*=========================
	| Master Instruction File |
	=========================*/
	struct dbview pcmi_list[] ={
		{"pcmi_co_no"},
		{"pcmi_inst_name"},
		{"pcmi_line_no"},
		{"pcmi_text"},
	};

	int	pcmi_no_fields = 4;

	struct	{
		char	mi_co_no[3];
		char	mi_inst_name[9];
		int	mi_line_no;
		char	mi_text[61];
	} pcmi_rec;

	/*=======================
	| Work Centre Code file |
	=======================*/
	struct dbview pcwc_list[] ={
		{"pcwc_hhwc_hash"},
		{"pcwc_co_no"},
		{"pcwc_br_no"},
		{"pcwc_work_cntr"},
		{"pcwc_name"},
	};

	int	pcwc_no_fields = 5;

	struct	{
		long	wc_hhwc_hash;
		char	wc_co_no[3];
		char	wc_br_no[3];
		char	wc_work_cntr[9];
		char	wc_name[41];
	} pcwc_rec;

	char	*comm	= "comm",
			*inmr	= "inmr",
			*inmr2	= "inmr2",
			*pcid	= "pcid",
			*pcid2	= "pcid2",
			*pcmi	= "pcmi",
			*pcwc	= "pcwc",
			*pcwc2	= "pcwc2";

	int	run_prog = MAST_SCN;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	st_item[17];
	char	st_desc[41];
	char	end_item[17];
	char	end_desc[41];
	char	st_wrk_cntr[9];
	char	end_wrk_cntr[9];
	char	instr_no[4];
	char	ver_num[7];
	char	st_ins_name[9];
	char	end_ins_name[9];
	long	hhbr_hash;
	long	hhwc_hash;
	char	output[8];
	int	lpno;
} local_rec;

static	struct	var vars[] =
{
	{1, LIN, "st_item",	 4, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " Start Item    :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_item},
	{1, LIN, "st_desc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_desc},
	{1, LIN, "end_item",	 5, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " End Item      :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_item},
	{1, LIN, "end_desc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_desc},
	{1, LIN, "st_wrk_cntr",	 7, 15, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", " Start Wrk Cntr:", " ",
		 YES, NO, JUSTLEFT, "", "", local_rec.st_wrk_cntr},
	{1, LIN, "end_wrk_cntr",	 8, 15, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", " End Wrk Cntr  :", " ",
		 YES, NO, JUSTLEFT, "", "", local_rec.end_wrk_cntr},
	{1, LIN, "instr_no",	 10, 15, CHARTYPE,
		"AAA", "          ",
		" ", " ", " Instruction # :", " ",
		 YES, NO, JUSTRIGHT, "1234567890", "", local_rec.instr_no},
	{1, LIN, "ver_num",	 11, 15, CHARTYPE,
		"AAA", "          ",
		" ", " ", " Version #     :", " ",
		 YES, NO, JUSTRIGHT, "1234567890", "", local_rec.ver_num},
	{1, LIN, "d_output",	 13, 15, CHARTYPE,
		"U", "          ",
		" ", "D", " Output To     :", " ",
		 YES, NO, JUSTLEFT, "", "", local_rec.output},
	{1, LIN, "d_lpno",	 14, 15, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No    :", " ",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	{2, LIN, "st_ins_name",	 4, 20, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", " Start Instruction :", " ",
		 YES, NO, JUSTLEFT, "", "", local_rec.st_ins_name},
	{2, LIN, "end_ins_name",	 5, 20, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", " End Instruction   :", " ",
		 YES, NO, JUSTLEFT, "", "", local_rec.end_ins_name},
	{2, LIN, "m_output",	 7, 15, CHARTYPE,
		"U", "          ",
		" ", "D", " Output To  :", " ",
		 YES, NO, JUSTLEFT, "", "", local_rec.output},
	{2, LIN, "m_lpno",	 8, 15, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No :", " ",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
/*=====================
| function prototypes |
=====================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
int spec_valid (int field);
int inst_srch (void);
int ver_srch (void);
int get_ver_num (void);
int process (void);
int open_output (void);
int read_pcmi (void);
int read_pcid (void);
int in_range (int rng_type);
void SrchPcwc (char *key_val);
void SrchPcmi (char *key_val);
int SrchInst (char *key_val);
int srch_ver (char *key_val);
int heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main(
 int   argc, 
 char *argv[])
{
	if (argc != 2)
	{
		print_at(0,0,mlPcMess714, argv[0]);
		return (EXIT_FAILURE);
	}

	switch(argv[1][0])
	{
	case 'M':
		run_prog = MAST_SCN;
		break;
		
	case 'D':
		run_prog = DETL_SCN;
		break;

	default:
		/*printf("Usage: %s <M(aster) | D(etail)>\n", argv[0]);*/
		print_at(0,0,ML(mlPcMess714), argv[0]);
		shutdown_prog();
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();
	ReadMisc ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		init_vars (run_prog);
		heading (run_prog);
		entry (run_prog);
		if (restart || prog_exit)
			continue;

		heading (run_prog);
		scn_display(run_prog);
		edit (run_prog);

		if (restart)
			continue;

		process();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog(
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB(
 void)
{
	abc_dbopen ("data");
	abc_alias(pcwc2, pcwc);
	abc_alias(pcid2, pcid);
	abc_alias(inmr2, inmr);

	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_id_no");
	open_rec (inmr2, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (pcid, pcid_list, pcid_no_fields, "pcid_id_no");
	open_rec (pcid2, pcid_list, pcid_no_fields, "pcid_id_no");
	open_rec (pcmi, pcmi_list, pcmi_no_fields, "pcmi_id_no");

	open_rec (pcwc, pcwc_list, pcwc_no_fields, "pcwc_id_no");
	open_rec (pcwc2, pcwc_list, pcwc_no_fields, "pcwc_hhwc_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB(
 void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (pcid);
	abc_fclose (pcmi);
	abc_fclose (pcwc);
	SearchFindClose ();
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc(
 void)
{


	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
}

int
spec_valid(
 int field)
{
	if (LCHECK ("st_item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf(local_rec.st_item, "%-16.16s", " ");
			sprintf(local_rec.st_desc, "%-35.35s", "First Item");

			DSP_FLD("st_item");
			DSP_FLD("st_desc");
			return(0);
		}
		cc = FindInmr (comm_rec.tco_no, local_rec.st_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.st_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		DSP_FLD ("st_item");
		sprintf (local_rec.st_desc,"%-35.35s",inmr_rec.mr_description);
		DSP_FLD ("st_desc");

		local_rec.hhbr_hash = inmr_rec.mr_hhbr_hash;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy(local_rec.end_item, "~~~~~~~~~~~~~~~~");
			sprintf(local_rec.end_desc, "%-35.35s", "Last Item");

			DSP_FLD("end_item");
			DSP_FLD("end_desc");
			return(0);
		}

		cc = FindInmr (comm_rec.tco_no, local_rec.end_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.end_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		DSP_FLD ("end_item");
		sprintf (local_rec.end_desc,"%-35.35s",inmr_rec.mr_description);
		DSP_FLD ("end_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK("st_wrk_cntr"))
	{
		if (SRCH_KEY)
		{
			SrchPcwc(temp_str);
			return(0);
		}

		if (dflt_used)
		{
			strcpy(local_rec.st_wrk_cntr, "        ");

			DSP_FLD("st_wrk_cntr");
			return(0);
		}

		strcpy(pcwc_rec.wc_co_no, comm_rec.tco_no);
		strcpy(pcwc_rec.wc_br_no, comm_rec.test_no);
		sprintf(pcwc_rec.wc_work_cntr, "%-8.8s", local_rec.st_wrk_cntr);
		cc = find_rec(pcwc, &pcwc_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("Work Centre Not Found On File\007");*/
			print_mess(ML(mlPcMess105));
			sleep(2);
			clear_mess();
			return(1);
		}
		local_rec.hhwc_hash = pcwc_rec.wc_hhwc_hash;

		DSP_FLD("st_wrk_cntr");
		
		return(0);
	}

	if (LCHECK("end_wrk_cntr"))
	{
		if (SRCH_KEY)
		{
			SrchPcwc(temp_str);
			return(0);
		}

		if (dflt_used)
		{
			strcpy(local_rec.end_wrk_cntr, "~~~~~~~~");

			DSP_FLD("end_wrk_cntr");
			return(0);
		}

		strcpy(pcwc_rec.wc_co_no, comm_rec.tco_no);
		strcpy(pcwc_rec.wc_br_no, comm_rec.test_no);
		sprintf(pcwc_rec.wc_work_cntr, "%-8.8s", local_rec.end_wrk_cntr);
		cc = find_rec(pcwc, &pcwc_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("Work Centre Not Found On File\007");*/
			print_mess(ML(mlPcMess105));
			sleep(2);
			clear_mess();
			return(1);
		}

		DSP_FLD("end_wrk_cntr");

		return(0);
	}
	
	if (LCHECK ("instr_no"))
	{
		if (SRCH_KEY && inst_srch())
		{
			SrchInst (temp_str);
			return (EXIT_SUCCESS);
		}
		
		if (dflt_used)
		{
			strcpy(local_rec.instr_no, "ALL");
			DSP_FLD("instr_no");

			strcpy(local_rec.ver_num, "LATEST");
			DSP_FLD("ver_num");
			FLD("ver_num") = NA;
			return(0);
		}
		FLD("ver_num") = YES;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ver_num"))
	{
		if (SRCH_KEY && ver_srch())
		{
			srch_ver (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy(local_rec.ver_num, "LATEST");
			DSP_FLD("ver_num");

			return(0);
		}

		sprintf(local_rec.ver_num,"%3.3s%-3.3s", local_rec.ver_num," ");

		return(0);
	}

	if (LCHECK ("st_ins_name"))
	{
		if (!MASTER)
			return(0);

		if (end_input && prog_status == ENTRY)
		{
			prog_exit = TRUE;
			return(0);
		}

		if (dflt_used)
		{
			sprintf(local_rec.st_ins_name, "%-8.8s", " ");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchPcmi(temp_str);
			return(0);
		}

		strcpy (pcmi_rec.mi_co_no, comm_rec.tco_no);
		strcpy (pcmi_rec.mi_inst_name, local_rec.st_ins_name);
		pcmi_rec.mi_line_no = 0;
		cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
		if ( cc ||
		     strcmp (pcmi_rec.mi_co_no, comm_rec.tco_no) ||
		     strcmp (pcmi_rec.mi_inst_name, local_rec.st_ins_name))
		{
			/*print_mess("\007 Instruction Does Not Exist On File ");*/
			print_mess(ML(mlPcMess106));
			sleep(2);
			clear_mess();
			return(1);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_ins_name"))
	{
		if (!MASTER)
			return(0);

		if (end_input && prog_status == ENTRY)
		{
			prog_exit = TRUE;
			return(0);
		}

		if (dflt_used)
		{
			strcpy(local_rec.end_ins_name, "~~~~~~~~");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchPcmi(temp_str);
			return(0);
		}

		strcpy (pcmi_rec.mi_co_no, comm_rec.tco_no);
		strcpy (pcmi_rec.mi_inst_name, local_rec.end_ins_name);
		pcmi_rec.mi_line_no = 0;
		cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
		if ( cc ||
		     strcmp (pcmi_rec.mi_co_no, comm_rec.tco_no) ||
		     strcmp (pcmi_rec.mi_inst_name, local_rec.end_ins_name))
		{
			/*print_mess("\007 Instruction Does Not Exist On File ");*/
			print_mess(ML(mlPcMess106));
			sleep(2);
			clear_mess();
			return(1);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK("d_output") || LCHECK("m_output"))
	{
		if (DISP)
		{
			strcpy(local_rec.output, "Display");
			local_rec.lpno = 0;
			if (MASTER)
			{
				FLD("m_lpno") = NA;
				DSP_FLD("m_lpno");
			}
			else
			{
				FLD("d_lpno") = NA;
				DSP_FLD("d_lpno");
			}
		}
		else
		{
			strcpy(local_rec.output, "Printer");
			if (MASTER)
				FLD("m_lpno") = YES;
			else
				FLD("d_lpno") = YES;
		}

		display_field(field);

		return(0);
	}
		
	if (LCHECK("d_lpno") || LCHECK("m_lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		return(0);
	}

	return (EXIT_SUCCESS);
}

/*------------------------------------------
| Check if srch on instr number is allowed |
------------------------------------------*/
int
inst_srch(
 void)
{
	if ( !ITEM_SPECIFIC )
		return(FALSE);

	if ( !CNTR_SPECIFIC )
		return(FALSE);

	return(TRUE);
}

/*----------------------------------------
| Check if srch on version no is allowed |
----------------------------------------*/
int
ver_srch(
 void)
{
	if ( !ITEM_SPECIFIC )
		return(FALSE);

	if ( !CNTR_SPECIFIC )
		return(FALSE);

	if ( ALL_INST )
		return(FALSE);

	return(TRUE);
}

/*---------------------------
| Get latest version number |
---------------------------*/
int
get_ver_num(
 void)
{
	int	ver_no;

	ver_no = 0;

	strcpy (wkid_rec.id_co_no, comm_rec.tco_no);
	wkid_rec.id_hhbr_hash = pcid_rec.id_hhbr_hash;
	wkid_rec.id_hhwc_hash = pcid_rec.id_hhwc_hash;
	wkid_rec.id_instr_no = pcid_rec.id_instr_no;
	wkid_rec.id_line_no = 0;
	wkid_rec.id_version = 1;
	cc = find_rec (pcid2, &wkid_rec, GTEQ, "r");
	while (!cc && !strcmp(wkid_rec.id_co_no, comm_rec.tco_no) &&
	       wkid_rec.id_hhbr_hash == pcid_rec.id_hhbr_hash &&
	       wkid_rec.id_hhwc_hash == pcid_rec.id_hhwc_hash &&
	       wkid_rec.id_instr_no == pcid_rec.id_instr_no)
	{
		ver_no = wkid_rec.id_version;
		cc = find_rec (pcid2, &wkid_rec, NEXT, "r");
	}

	return(ver_no);
}

int
process(
 void)
{
	open_output();

	if ( MASTER )
		read_pcmi();
	else
		read_pcid();

	if (DISP)
	{
		Dsp_srch();
		Dsp_close();
	}
	else
		Dsp_print ();

	return(0);
}

int
open_output(
 void)
{
	if (DISP)
	{
		heading(0);
		if ( MASTER )
		{
			Dsp_prn_open(10, 2, 14, "    ", 
				comm_rec.tco_no, comm_rec.tco_name, 
				comm_rec.test_no, comm_rec.test_name, 
				comm_rec.tcc_no, comm_rec.tcc_name);
			Dsp_saverec("                 Master Instruction Display                 ");
			Dsp_saverec("");
		}
		else
		{
			Dsp_prn_open(10, 2, 14, "    ", 
				comm_rec.tco_no, comm_rec.tco_name, 
				comm_rec.test_no, comm_rec.test_name, 
				comm_rec.tcc_no, comm_rec.tcc_name);
			Dsp_saverec("                 Instruction Detail Display                 ");
			Dsp_saverec("   ITEM NUMBER    | WRK CNTR | INST # | VERSION             ");
		}
	}
	else
	{
		if ( MASTER )
		{
			Dsp_nd_prn_open(0, 0, 15, "    ", 
				comm_rec.tco_no, comm_rec.tco_name, 
				comm_rec.test_no, comm_rec.test_name, 
				comm_rec.tcc_no, comm_rec.tcc_name);
			Dsp_saverec("                 Master Instruction Display                 ");
			Dsp_saverec("");
		}
		else
		{
			Dsp_nd_prn_open(0, 0, 15, "     ", 
				comm_rec.tco_no, comm_rec.tco_name, 
				comm_rec.test_no, comm_rec.test_name, 
				comm_rec.tcc_no, comm_rec.tcc_name);
			Dsp_saverec("                 Instruction Detail Display                 ");
			Dsp_saverec("   ITEM NUMBER    | WRK CNTR | INST # | VERSION             ");
		}
	}

	Dsp_saverec(" [REDRAW]  [PRINT]  [NEXT]  [PREV]  [EDIT/END] ");

	return(0);
}

/*--------------------------------------
| Read pcmi records in specified range |
--------------------------------------*/
int
read_pcmi(
 void)
{
	char	data_str[80];
	char	curr_inst[9];
	int	first_time;
	int	data_found;
 
	first_time = TRUE;
	data_found = FALSE;

	if (!DISP)
	{
		dsp_screen("Printing Master Instructions",
			comm_rec.tco_no,
			comm_rec.tco_name);
	}
	
	/*------------------------------
	| Load all master instructions |
	------------------------------*/
	sprintf(curr_inst, "%-8.8s", " ");

	strcpy (pcmi_rec.mi_co_no, comm_rec.tco_no);
	sprintf(pcmi_rec.mi_inst_name, "%-8.8s", local_rec.st_ins_name);
	pcmi_rec.mi_line_no = 0;
	cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcmi_rec.mi_co_no, comm_rec.tco_no) && 
	       strcmp (pcmi_rec.mi_inst_name, local_rec.end_ins_name) <= 0)
	{
		if (strcmp(curr_inst, pcmi_rec.mi_inst_name))
		{
			if (!first_time)
				Dsp_saverec(UNDERLINE);

			sprintf(data_str,"^1 %-8.8s ^6", pcmi_rec.mi_inst_name);
			Dsp_saverec(data_str);
			strcpy(curr_inst, pcmi_rec.mi_inst_name);
		}

		if (!DISP)
			dsp_process("Instruction", pcmi_rec.mi_inst_name);

		data_found = TRUE;
		Dsp_saverec(pcmi_rec.mi_text);
		first_time = FALSE;

		cc = find_rec (pcmi, &pcmi_rec, NEXT, "r");
	}

	if (data_found)
		Dsp_saverec(UNDERLINE);

	return(0);
}

/*--------------------------------------
| Read pcid records in specified range |
--------------------------------------*/
int
read_pcid(
 void)
{
	int	data_found;
	int	first_time;
	char	tmp_inst[7];
	char	data_str[200];
	char	curr_item[17];
	char	curr_cntr[9];
	int	curr_inst;
	int	curr_ver;

	data_found = FALSE;
	first_time = TRUE;

	if (!DISP)
	{
		dsp_screen("Printing Instructions Details",
			comm_rec.tco_no,
			comm_rec.tco_name);
	}

	strcpy (pcid_rec.id_co_no, comm_rec.tco_no);
	if (ITEM_SPECIFIC)
		pcid_rec.id_hhbr_hash = local_rec.hhbr_hash;
	else
		pcid_rec.id_hhbr_hash = 0L;

	if (ITEM_SPECIFIC && CNTR_SPECIFIC)
		pcid_rec.id_hhwc_hash = local_rec.hhwc_hash;
	else
		pcid_rec.id_hhwc_hash = 0L;

	if (ITEM_SPECIFIC && CNTR_SPECIFIC &&  ALL_INST)
		pcid_rec.id_instr_no = 0;
	else
		pcid_rec.id_instr_no = atoi(local_rec.instr_no);

	pcid_rec.id_version = 1;
	pcid_rec.id_line_no = 0;

	sprintf(curr_item, "%-16.16s", " ");
	sprintf(curr_cntr, "%-8.8s", " ");
	curr_inst = 0;
	curr_ver = 0;

	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	while (!cc && !strcmp (pcid_rec.id_co_no, comm_rec.tco_no))
	{
		if ( !in_range(ITEM) || !in_range(CNTR) || 
		     !(in_range(INST) && in_range(VERS)) )
		{
			cc = find_rec (pcid, &pcid_rec, NEXT, "r");
			continue;
		}

		if ( strcmp(curr_item, inmr_rec.mr_item_no) ||
		     strcmp(curr_cntr, pcwc_rec.wc_work_cntr) ||
		     curr_inst != pcid_rec.id_instr_no ||
		     curr_ver != pcid_rec.id_version )
		{
			sprintf(data_str,
				"^1  %-16.16s  %-8.8s    %3d       %3d               ^6",
				inmr_rec.mr_item_no,
				pcwc_rec.wc_work_cntr,
				pcid_rec.id_instr_no,
				pcid_rec.id_version);

			if (!first_time)
				Dsp_saverec(UNDERLINE);

			Dsp_saverec(data_str);

			sprintf(curr_item, "%-16.16s", inmr_rec.mr_item_no);
			sprintf(curr_cntr, "%-8.8s", pcwc_rec.wc_work_cntr);
			curr_inst = pcid_rec.id_instr_no;
			curr_ver = pcid_rec.id_version;
		}

		if (!DISP)
		{
			sprintf(tmp_inst, "%06d", pcid_rec.id_instr_no);
			dsp_process("Instruction", tmp_inst);
		}

		data_found = TRUE;
		first_time = FALSE;
		Dsp_saverec(pcid_rec.id_text);

		cc = find_rec (pcid, &pcid_rec, NEXT, "r");
	}

	if (data_found)
		Dsp_saverec(UNDERLINE);

	return(0);
}

/*-------------------------
| Check if instruction is |
| within range specified  |
-------------------------*/
int
in_range(
 int rng_type)
{
	switch(rng_type)
	{
	case ITEM:
		cc = find_hash(inmr2, &inmr_rec, COMPARISON, "r", pcid_rec.id_hhbr_hash);
		if (cc)
			return(FALSE);

		if (strcmp(inmr_rec.mr_item_no, local_rec.st_item) < 0 ||
		    strcmp(inmr_rec.mr_item_no, local_rec.end_item) > 0)
			return(FALSE);

		return(TRUE);

	case CNTR:
		cc = find_hash(pcwc2, &pcwc_rec, COMPARISON, "r", pcid_rec.id_hhwc_hash);
		if (cc)
			return(FALSE);

		if (strcmp(pcwc_rec.wc_work_cntr, local_rec.st_wrk_cntr) < 0 ||
		    strcmp(pcwc_rec.wc_work_cntr, local_rec.end_wrk_cntr) > 0)
			return(FALSE);

		return(TRUE);

	case INST:
		if (ALL_INST)
			return(TRUE);

		if (pcid_rec.id_instr_no == atoi(local_rec.instr_no))
			return(TRUE);

		return(FALSE);

	case VERS:
		if (LATEST_VER)
		{
			if (!ALL_INST && pcid_rec.id_instr_no != atoi(local_rec.instr_no))
				return(FALSE);

		 	if ( pcid_rec.id_version != get_ver_num() )
				return(FALSE);
		}
		else
		{
		 	if ( pcid_rec.id_version != atoi(local_rec.ver_num) )
				return(FALSE);
		}

		return(TRUE);

	default:
		return(FALSE);
	}
}

void
SrchPcwc(
 char *key_val)
{
	work_open ();
	save_rec ("# Instruction ", "#");
	strcpy (pcwc_rec.wc_co_no, comm_rec.tco_no);
	strcpy (pcwc_rec.wc_br_no, comm_rec.test_no);
	sprintf(pcwc_rec.wc_work_cntr, "%-8.8s", key_val);
	cc = find_rec (pcwc, &pcwc_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcwc_rec.wc_co_no, comm_rec.tco_no) &&
	       !strcmp (pcwc_rec.wc_br_no, comm_rec.test_no) &&
	       !strncmp(pcwc_rec.wc_work_cntr, key_val, strlen(key_val)))
	{
		cc = save_rec ( pcwc_rec.wc_work_cntr, pcwc_rec.wc_name);
		if (cc)
			break;

		cc = find_rec (pcwc, &pcwc_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pcwc_rec.wc_co_no, comm_rec.tco_no);
	strcpy (pcwc_rec.wc_br_no, comm_rec.test_no);
	sprintf(pcwc_rec.wc_work_cntr, "%-8.8s", temp_str);
	cc = find_rec (pcwc, &pcwc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pcwc, "DBFIND");
}

void
SrchPcmi(
 char *key_val)
{
	work_open ();
	save_rec ("# Instruction ", "#");
	strcpy (pcmi_rec.mi_co_no, comm_rec.tco_no);
	sprintf(pcmi_rec.mi_inst_name, "%-8.8s", key_val);
	pcmi_rec.mi_line_no = 0;
	cc = find_rec (pcmi, &pcmi_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcmi_rec.mi_co_no, comm_rec.tco_no) &&
	       !strncmp(pcmi_rec.mi_inst_name, key_val, strlen(key_val)))
	{
		if (pcmi_rec.mi_line_no == 0)
		{
			cc = save_rec ( pcmi_rec.mi_inst_name, " ");
			if (cc)
				break;
		}
		cc = find_rec (pcmi, &pcmi_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}

int
SrchInst(
 char *key_val)
{
	int	i;
	int	skip_save;
	int	instr[1000];
	int	num_inst;
	
	num_inst = 0;
	
	work_open ();
	save_rec ("#No.", "#Instruction");
	strcpy (pcid_rec.id_co_no, comm_rec.tco_no);
	pcid_rec.id_hhbr_hash = local_rec.hhbr_hash;
	pcid_rec.id_hhwc_hash = local_rec.hhwc_hash;
	pcid_rec.id_instr_no = 1;
	pcid_rec.id_version = 1;
	pcid_rec.id_line_no = 0;
	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcid_rec.id_co_no, comm_rec.tco_no) &&
	       pcid_rec.id_hhbr_hash == local_rec.hhbr_hash &&
	       pcid_rec.id_hhwc_hash == local_rec.hhwc_hash)
	{
		if (pcid_rec.id_line_no == 0)
		{
			skip_save = FALSE;
			for (i = 0; i < num_inst; i++)
			{
				if (pcid_rec.id_instr_no == instr[i])
				{
					skip_save = TRUE;
					break;
				}
			}
			if (skip_save)
			{
				cc = find_rec (pcid, &pcid_rec, NEXT, "r");
				continue;
			}

			instr[num_inst++] = pcid_rec.id_instr_no;
			sprintf (err_str, "%3d", pcid_rec.id_instr_no);
			cc = save_rec (err_str, pcid_rec.id_text);
			if (cc)
				break;
		}
		cc = find_rec (pcid, &pcid_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return(0);

	return(0);
}

int
srch_ver(
 char *key_val)
{
	work_open ();
	save_rec ("#VER", "#");
	strcpy (pcid_rec.id_co_no, comm_rec.tco_no);
	pcid_rec.id_hhbr_hash = local_rec.hhbr_hash;
	pcid_rec.id_hhwc_hash = local_rec.hhwc_hash;
	pcid_rec.id_instr_no = atoi(local_rec.instr_no);
	pcid_rec.id_version = 1;
	pcid_rec.id_line_no = 0;
	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcid_rec.id_co_no, comm_rec.tco_no) &&
	       pcid_rec.id_hhbr_hash == local_rec.hhbr_hash &&
	       pcid_rec.id_hhwc_hash == local_rec.hhwc_hash &&
	       pcid_rec.id_instr_no == atoi(local_rec.instr_no))
	{
		if (pcid_rec.id_line_no == 0)
		{
			sprintf (err_str, "%3d", pcid_rec.id_version);
			cc = save_rec (err_str, "");
			if (cc)
				break;
		}
		cc = find_rec (pcid, &pcid_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	return(0);
}

int
heading(
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	clear ();

/*
	if (MASTER)
		strcpy (err_str, " Display Master Instructions. ");
	else
		strcpy (err_str, " Display Instruction Details. ");
*/

	if (scn != 0)
	{
		switch ( run_prog )
		{
		case	MAST_SCN:
			box (0, 3, 80, 5 );

			move(1,6);
			line(79);

			break;

		case	DETL_SCN:
			box (0, 3, 80, 11 );

			move(1,6);
			line(79);

			move(1,9);
			line(79);

			move(1,12);
			line(79);

			break;

		default:
			break;
		}
	}

	if (MASTER)		
		rv_pr (ML(mlPcMess065), (80 - strlen (ML(mlPcMess065) ) ) / 2, 0, 1 );
	else
		rv_pr (ML(mlPcMess066), (80 - strlen (ML(mlPcMess066) ) ) / 2, 0, 1 );

	move(0,1);
	line(80);
	move (0, 21);
	line (80);

	sprintf (err_str,ML(mlStdMess038),
		comm_rec.tco_no, 
		comm_rec.tco_name);
	rv_pr(err_str, 0, 22, 0);

	if (scn == 0)
		return(0);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_FAILURE);
}

