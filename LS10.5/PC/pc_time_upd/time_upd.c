/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: time_upd.c,v 5.2 2001/08/09 09:14:49 scott Exp $
|  Program Name  : (pc_time_upd.c)
|  Program Desc  : (Production Control Timesheet Update)
|---------------------------------------------------------------------|
|  Date Written  : 10/03/92        | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: time_upd.c,v $
| Revision 5.2  2001/08/09 09:14:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:06  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: time_upd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_time_upd/time_upd.c,v 5.2 2001/08/09 09:14:49 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>

FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct pcglRecord	pcgl_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct pcatRecord	pcat_rec;
struct pclnRecord	pcln_rec;
struct pclnRecord	pcln2_rec;
struct pcrqRecord	pcrq_rec;
struct pcwcRecord	pcwc_rec;
struct pcwoRecord	pcwo_rec;
struct rgrsRecord	rgrs_rec;

	char	*data	= "data";

	long	mat_hash 		[2],		/* 0 - direct 1 - mfg var direct */
			dir_hash 		[5],		/* 0 - labour   */
			dir_rec_hash 	[5],		/* 1 - machine  */
			fix_hash 		[5],		/* 2 - qc-check */
			fix_rec_hash 	[5],		/* 3 - special  */
			mfg_d_hash 		[5],		/* 4 - other    */
			mfg_f_hash 		[5];
	char	mat_acc 		[2][17],	/* 0 - direct 1 - mfg var direct */
			dir_acc 		[5][17],	/* 0 - labour   */
			dir_rec_acc 	[5][17],	/* 1 - machine  */
			fix_acc 		[5][17],	/* 2 - qc-check */
			fix_rec_acc 	[5][17],	/* 3 - special  */
			mfg_d_acc 		[5][17],	/* 4 - other    */
			mfg_f_acc 		[5][17],
			yld_clc 		[5];

	int		needPcgl = FALSE;

	char	loc_curr [4];

struct
{
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	int		printerNo;
} local_rec;


/*=====================
| function prototypes |
=====================*/
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	Process 				(void);
void 	ReverseVarianceCost 	(void);
void 	UpdatePcrq 				(void);
void 	GetYield 				(void);
void 	LogError 				(int);
void 	HeadingOutput 			(void);
void 	UpdatePayRoll 			(void);
void 	AddPcgl 				(long, char *, char *, double, char *);
int 	ProcessPcat 			(void);
int 	ReadDefault 			(void);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char *argv [])
{
	if (argc != 2)
	{
		print_at (0,0,mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}

	local_rec.printerNo = atoi (argv [1]);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	/*-------------------------------
	| Setup required parameters	|
	-------------------------------*/
	init_scr ();

	OpenDB ();

	Process ();

	shutdown_prog ();
	return (EXIT_FAILURE);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================================
| Read all nessasary files for defaults. |
========================================*/
int
ReadDefault (void)
{
	abc_selfield (glmr, "glmr_id_no");

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP D MATL",
		" ",
		inmr_rec.category
	);
	mat_hash [0] = glmrRec.hhmr_hash;
	strcpy (mat_acc [0], glmrRec.acc_no);

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP D LABR",
		" ",
		inmr_rec.category
	);
	dir_hash [0] = glmrRec.hhmr_hash;
	strcpy (dir_acc [0], glmrRec.acc_no);

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP D MACH",
		" ",
		inmr_rec.category
	);
	dir_hash [1] = glmrRec.hhmr_hash;
	strcpy (dir_acc [1], glmrRec.acc_no);

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP D QC  ",
		" ",
		inmr_rec.category
	);
	dir_hash [2] = glmrRec.hhmr_hash;
	strcpy (dir_acc [2], glmrRec.acc_no);

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP D SPEC",
		" ",
		inmr_rec.category
	);
	dir_hash [3] = glmrRec.hhmr_hash;
	strcpy (dir_acc [3], glmrRec.acc_no);

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP D OTH ",
		" ",
		inmr_rec.category
	);
	dir_hash [4] = glmrRec.hhmr_hash;
	strcpy (dir_acc [4], glmrRec.acc_no);

	/* Fixed Accounts */
	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP F LABR",
		" ",
		inmr_rec.category
	);
	fix_hash [0] = glmrRec.hhmr_hash;
	strcpy (fix_acc [0], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP F MACH",
		" ",
		inmr_rec.category
	);
	fix_hash [1] = glmrRec.hhmr_hash;
	strcpy (fix_acc [1], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP F QC  ",
		" ",
		inmr_rec.category
	);
	fix_hash [2] = glmrRec.hhmr_hash;
	strcpy (fix_acc [2], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP F SPEC",
		" ",
		inmr_rec.category
	);
	fix_hash [3] = glmrRec.hhmr_hash;
	strcpy (fix_acc [3], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"WIP F OTH ",
		" ",
		inmr_rec.category
	);
	fix_hash [4] = glmrRec.hhmr_hash;
	strcpy (fix_acc [4], glmrRec.acc_no);

	/* Direct Recovery Accounts */
	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC D LABR",
		" ",
		inmr_rec.category
	);
	dir_rec_hash [0] = glmrRec.hhmr_hash;
	strcpy (dir_rec_acc [0], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC D MACH",
		" ",
		inmr_rec.category
	);
	dir_rec_hash [1] = glmrRec.hhmr_hash;
	strcpy (dir_rec_acc [1], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC D QC  ",
		" ",
		inmr_rec.category
	);
	dir_rec_hash [2] = glmrRec.hhmr_hash;
	strcpy (dir_rec_acc [2], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC D SPEC",
		" ",
		inmr_rec.category
	);
	dir_rec_hash [3] = glmrRec.hhmr_hash;
	strcpy (dir_rec_acc [3], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC D OTH ",
		" ",
		inmr_rec.category
	);
	dir_rec_hash [4] = glmrRec.hhmr_hash;
	strcpy (dir_rec_acc [4], glmrRec.acc_no);

	/* Fixed Recovery Accounts */
	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC F LABR",
		" ",
		inmr_rec.category
	);
	fix_rec_hash [0] = glmrRec.hhmr_hash;
	strcpy (fix_rec_acc [0], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC F MACH",
		" ",
		inmr_rec.category
	);
	fix_rec_hash [1] = glmrRec.hhmr_hash;
	strcpy (fix_rec_acc [1], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC F QC  ",
		" ",
		inmr_rec.category
	);
	fix_rec_hash [2] = glmrRec.hhmr_hash;
	strcpy (fix_rec_acc [2], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC F SPEC",
		" ",
		inmr_rec.category
	);
	fix_rec_hash [3] = glmrRec.hhmr_hash;
	strcpy (fix_rec_acc [3], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"REC F OTH ",
		" ",
		inmr_rec.category
	);
	fix_rec_hash [4] = glmrRec.hhmr_hash;
	strcpy (fix_rec_acc [4], glmrRec.acc_no);

	/* Manufacturing Variance Direct Accounts */
	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN D MATL",
		" ",
		inmr_rec.category
	);
	mat_hash [1] = glmrRec.hhmr_hash;
	strcpy (mat_acc [1], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN D LABR",
		" ",
		inmr_rec.category
	);
	mfg_d_hash [0] = glmrRec.hhmr_hash;
	strcpy (mfg_d_acc [0], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN D MACH",
		" ",
		inmr_rec.category
	);
	mfg_d_hash [1] = glmrRec.hhmr_hash;
	strcpy (mfg_d_acc [1], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN D QC  ",
		" ",
		inmr_rec.category
	);
	mfg_d_hash [2] = glmrRec.hhmr_hash;
	strcpy (mfg_d_acc [2], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN D SPEC",
		" ",
		inmr_rec.category
	);
	mfg_d_hash [3] = glmrRec.hhmr_hash;
	strcpy (mfg_d_acc [3], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN D OTH ",
		" ",
		inmr_rec.category
	);
	mfg_d_hash [4] = glmrRec.hhmr_hash;
	strcpy (mfg_d_acc [4], glmrRec.acc_no);

	/* Manufacturing Variance Fixed Accounts */
	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN F LABR",
		" ",
		inmr_rec.category
	);
	mfg_f_hash [0] = glmrRec.hhmr_hash;
	strcpy (mfg_f_acc [0], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN F MACH",
		" ",
		inmr_rec.category
	);
	mfg_f_hash [1] = glmrRec.hhmr_hash;
	strcpy (mfg_f_acc [1], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN F QC  ",
		" ",
		inmr_rec.category
	);
	mfg_f_hash [2] = glmrRec.hhmr_hash;
	strcpy (mfg_f_acc [2], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN F SPEC",
		" ",
		inmr_rec.category
	);
	mfg_f_hash [3] = glmrRec.hhmr_hash;
	strcpy (mfg_f_acc [3], glmrRec.acc_no);

	GL_GLI
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"MAN F OTH ",
		" ",
		inmr_rec.category
	);
	mfg_f_hash [4] = glmrRec.hhmr_hash;
	strcpy (mfg_f_acc [4], glmrRec.acc_no);

	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (loc_curr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcat, pcat_list, PCAT_NO_FIELDS, "pcat_hhwo_hash");
	open_rec (pcgl, pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pcln, pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcrq, pcrq_list, PCRQ_NO_FIELDS, "pcrq_id_no2");
	open_rec (pcwc, pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (rgrs, rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
	OpenGlmr ();
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (void)
{
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (pcat);
	abc_fclose (pcgl);
	abc_fclose (pcln);
	abc_fclose (pcrq);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (rgrs);
	GL_Close ();

	abc_dbclose (data);
}

void
Process (
 void)
{
	char	wip_dir_acc [17], wip_d_rec_acc [17], 
			wip_fix_acc [17], wip_f_rec_acc [17],
			mfg_dir_acc [17], mfg_fix_acc [17];
	long	wip_dir_hash, wip_d_rec_hash, 
			wip_fix_hash, wip_f_rec_hash, 
			mfg_dir_hash, mfg_fix_hash,
			std_time, act_time;
	double	std_dir_cost, std_ovh_cost,
			act_dir_cost, act_ovh_cost,
			cost_diff;
	int		TYPE = 0;

	dsp_screen ("Processing Times", comm_rec.co_no, comm_rec.co_name);

	needPcgl = FALSE;
	/*--------------------------
	| Process all pcat records |
	--------------------------*/
	pcat_rec.hhwo_hash	=	0L;
	cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	while (!cc)
	{
		strcpy (err_str, DateToString (pcat_rec.date));
		dsp_process ("Date:", err_str);
		/*--------------------------
		| Add / Update pcrq record |
		--------------------------*/
		cc = ProcessPcat ();
		if (cc)
		{
			abc_unlock (pcat);
			cc = find_rec (pcat, &pcat_rec, NEXT, "u");
			continue;
		}

		abc_selfield (glmr, "glmr_hhmr_hash");

		/*
		 * Update G/L 
		 */
		/* calculate standard time */
		std_time = pcln_rec.setup + pcln_rec.run + pcln_rec.clean;

		/* calculate actual time */
		act_time = pcrq_rec.act_setup +
				pcrq_rec.act_run +
				pcrq_rec.act_clean;

		std_dir_cost = (double) std_time * pcln_rec.rate;
		std_dir_cost /= (double) 60.00;
		std_dir_cost *= pcln_rec.qty_rsrc;
		std_ovh_cost = (double) std_time * pcln_rec.ovhd_var;
		std_ovh_cost /= (double) 60.00;
		std_ovh_cost *= pcln_rec.qty_rsrc;
		std_ovh_cost += pcln_rec.ovhd_fix;

		act_dir_cost = (double) act_time * pcln_rec.rate;
		act_dir_cost /= (double) 60.00;
		act_dir_cost *= pcln_rec.qty_rsrc;
		act_ovh_cost = (double) act_time * pcln_rec.ovhd_var;
		act_ovh_cost /= (double) 60.00;
		act_ovh_cost *= pcln_rec.qty_rsrc;
		act_ovh_cost += pcln_rec.ovhd_fix;

		pcwc_rec.hhwc_hash 	=	pcln_rec.hhwc_hash;
		cc = find_rec (pcwc, &pcwc_rec, EQUAL, "r");
		if (cc)
			strcpy (pcwc_rec.work_cntr, "        ");

		/* read the resource file, read the appropriate accounts */
		/* use the appropriate accounts, if not setup use the    */
		/* system wide accounts (GL_GLI).                           */
		switch (rgrs_rec.type [0])
		{
		case	'L': TYPE = 0; break;
		case	'M': TYPE = 1; break;
		case	'Q': TYPE = 2; break;
		case	'S': TYPE = 3; break;
		case	'O': TYPE = 4; break;
		}
		glmrRec.hhmr_hash	=	rgrs_rec.dir_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (!cc)
		{
		    wip_dir_hash = glmrRec.hhmr_hash;
		    strcpy (wip_dir_acc, glmrRec.acc_no);
		}
		else
		{
			wip_dir_hash = dir_hash [TYPE];
			strcpy (wip_dir_acc, dir_acc [TYPE]);
		}
		glmrRec.hhmr_hash	=	rgrs_rec.dir_rec_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (!cc)
		{
		    wip_d_rec_hash = glmrRec.hhmr_hash;
		    strcpy (wip_d_rec_acc, glmrRec.acc_no);
		}
		else
		{
			wip_d_rec_hash = dir_rec_hash [TYPE];
			strcpy (wip_d_rec_acc, dir_rec_acc [TYPE]);
		}
		glmrRec.hhmr_hash	=	rgrs_rec.fix_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (!cc)
		{
		    wip_fix_hash = glmrRec.hhmr_hash;
		    strcpy (wip_fix_acc, glmrRec.acc_no);
		}
		else
		{
			wip_fix_hash = fix_hash [TYPE];
			strcpy (wip_fix_acc, fix_acc [TYPE]);
		}
		glmrRec.hhmr_hash	=	rgrs_rec.fix_rec_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (!cc)
		{
		    wip_f_rec_hash = glmrRec.hhmr_hash;
		    strcpy (wip_f_rec_acc, glmrRec.acc_no);
		}
		else
		{
			wip_f_rec_hash = fix_rec_hash [TYPE];
			strcpy (wip_f_rec_acc, fix_rec_acc [TYPE]);
		}
		glmrRec.hhmr_hash	=	rgrs_rec.mfg_dir_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (!cc)
		{
			mfg_dir_hash = glmrRec.hhmr_hash;
			strcpy (mfg_dir_acc, glmrRec.acc_no);
		}
		else
		{
			mfg_dir_hash = mfg_d_hash [TYPE];
			strcpy (mfg_dir_acc, mfg_d_acc [TYPE]);
		}
		glmrRec.hhmr_hash	=	rgrs_rec.mfg_fix_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (!cc)
		{
			mfg_fix_hash = glmrRec.hhmr_hash;
			strcpy (mfg_fix_acc, glmrRec.acc_no);
		}
		else
		{
			mfg_fix_hash = mfg_f_hash [TYPE];
			strcpy (mfg_fix_acc, mfg_f_acc [TYPE]);
		}

		/*---------------------
		| G/L POSTINGS - pcgl |
		---------------------*/
		/* WIP Direct Account Postings */
		AddPcgl (wip_dir_hash, 
			wip_dir_acc, 
			pcwc_rec.work_cntr, 
			std_dir_cost, 
			"1");
		AddPcgl (wip_d_rec_hash, 
			wip_d_rec_acc, 
			pcwc_rec.work_cntr, 
			act_dir_cost, 
			"2");

		cost_diff = act_dir_cost - std_dir_cost;
		AddPcgl (mfg_dir_hash,
			mfg_dir_acc,
			pcwc_rec.work_cntr,
			 (cost_diff > 0) ? cost_diff : 0 - cost_diff,
			 (cost_diff > 0) ? "1" : "2");

		/* WIP Fixed/Overhead Account Postings */
		AddPcgl (wip_fix_hash, 
			wip_fix_acc, 
			pcwc_rec.work_cntr, 
			std_ovh_cost, 
			"1");
		AddPcgl (wip_f_rec_hash, 
			wip_f_rec_acc, 
			pcwc_rec.work_cntr, 
			act_ovh_cost, 
			"2");

		cost_diff = act_ovh_cost - std_ovh_cost;
		AddPcgl (mfg_fix_hash,
			mfg_fix_acc,
			pcwc_rec.work_cntr,
			 (cost_diff > 0) ? cost_diff : 0 - cost_diff,
			 (cost_diff > 0) ? "1" : "2");

		/*-----------------------
		| Update Payroll System |
		-----------------------*/
		UpdatePayRoll ();

		cc = find_rec (pcat, &pcat_rec, NEXT, "u");
	}
}

/*---------------------
| Process pcat record |
---------------------*/
int
ProcessPcat (void)
{
	int		last_line_no = -1;
	static  long last_hash = 0L;
	int		pcrq_exist = 0;
	int		pcln_exist = 0;

	/*--------------------------------------
	| if not printed, do not update record |
	--------------------------------------*/
	if (pcat_rec.stat_flag [0] != 'P')
		return (EXIT_FAILURE);

	/*------------------
	| Find pcwo record |
	------------------*/
	pcwo_rec.hhwo_hash	=	pcat_rec.hhwo_hash;
	cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
	if (cc)
	{
		LogError (1);
		return (EXIT_FAILURE);
	}

	/*------------------------------------------------
	| Only process record if for this warehouse only |
	------------------------------------------------*/
	if (strcmp (pcwo_rec.br_no, comm_rec.est_no) != 0 &&
		strcmp (pcwo_rec.wh_no, comm_rec.cc_no) != 0)
		return (EXIT_FAILURE);

	/*-------------
	| Lookup rgrs |
	-------------*/
	if (last_hash != pcat_rec.hhrs_hash)
	{
		rgrs_rec.hhrs_hash	=	pcat_rec.hhrs_hash;
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
			rgrs_rec.rate = 0.00;
	}
		
	inei_rec.hhbr_hash = pcwo_rec.hhbr_hash;
	strcpy (inei_rec.est_no, pcwo_rec.br_no);
	cc = find_rec (inei, &inei_rec, COMPARISON, "r");
	if (cc)
	{
		LogError (2);
		return (EXIT_FAILURE);
	}
	inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		LogError (2);
		return (EXIT_FAILURE);
	}
	if (ReadDefault ())
	{
		LogError (3);
		return (EXIT_FAILURE);
	}

	/*------------------
	| Find pcrq record |
	------------------*/
	pcrq_rec.hhwo_hash = pcat_rec.hhwo_hash;
	pcrq_rec.seq_no    = pcat_rec.seq_no;
	pcrq_rec.line_no   = 0;
	cc = find_rec (pcrq, &pcrq_rec, GTEQ, "u");
	while (!cc &&
		pcrq_rec.hhwo_hash == pcat_rec.hhwo_hash &&
		pcrq_rec.seq_no    == pcat_rec.seq_no    &&
		pcrq_rec.hhrs_hash != pcat_rec.hhrs_hash)
	{
		pcrq_exist = 1;
		last_line_no = pcrq_rec.line_no;
		abc_unlock (pcrq);
		cc = find_rec (pcrq, &pcrq_rec, NEXT, "u");
	}

	/*----------------------------
	| pcrq exists so update pcrq |
	----------------------------*/
	if (pcrq_rec.hhwo_hash == pcat_rec.hhwo_hash &&
		pcrq_rec.seq_no    == pcat_rec.seq_no    &&
		pcrq_rec.hhrs_hash == pcat_rec.hhrs_hash && pcrq_exist)
	{
		pcln_rec.hhwo_hash	= pcrq_rec.hhwo_hash;
		pcln_rec.seq_no	= pcrq_rec.seq_no;
		pcln_rec.line_no	= pcrq_rec.line_no;
		cc = find_rec (pcln, &pcln_rec, EQUAL, "r");
		if (cc)
			file_err (cc, pcln, "DBFIND");

		if (pcrq_rec.act_time)
			ReverseVarianceCost ();
		UpdatePcrq ();
	}
	else
	{
		/*---------------------------------
		| No pcrq exists so add pcrq/pcln |
		---------------------------------*/
		abc_unlock (pcrq);

		pcrq_rec.hhrs_hash = pcat_rec.hhrs_hash;
		pcrq_rec.qty_rsrc  = 1;
		pcrq_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		sprintf (pcrq_rec.prod_class, "%-4.4s", inei_rec.prod_class);
		pcrq_rec.priority  = pcwo_rec.priority;
		pcrq_rec.seq_no    = pcat_rec.seq_no;
		pcrq_rec.line_no   = last_line_no + 1;

		pcrq_rec.last_date = 0L;
		pcrq_rec.last_time = 0L;

		pcrq_rec.est_date  = 0L;
		pcrq_rec.est_time  = 0L;
		pcrq_rec.est_setup = 0L;
		pcrq_rec.est_run   = 0L;
		pcrq_rec.est_clean = 0L;

		pcrq_rec.act_date  = pcat_rec.date;
		pcrq_rec.act_time  = pcat_rec.start_time;
		pcrq_rec.act_setup = pcat_rec.setup;
		pcrq_rec.act_run   = pcat_rec.run;
		pcrq_rec.act_clean = pcat_rec.clean;

		strcpy (pcrq_rec.can_split, "Y");
		strcpy (pcrq_rec.firm_sched, "Y");

		if (pcrq_rec.seq_no < pcwo_rec.rtg_seq)
			strcpy (pcrq_rec.stat_flag, "A");
		else
			strcpy (pcrq_rec.stat_flag, "E");

		cc = abc_add (pcrq, &pcrq_rec);
		if (cc)
			file_err (cc, pcrq, "DBADD");

		GetYield ();

		pcln_rec.hhwo_hash	= pcwo_rec.hhwo_hash;
		pcln_rec.seq_no		= pcat_rec.seq_no;
		pcln_rec.line_no	= pcrq_rec.line_no;
		cc = find_rec (pcln, &pcln_rec, EQUAL, "u");
		if (!cc)
			pcln_exist = 1;
		pcln_rec.hhgr_hash	= 0L;
		pcln_rec.hhwc_hash	= pcat_rec.hhwc_hash;
		pcln_rec.hhrs_hash	= pcat_rec.hhrs_hash;
		pcln_rec.rate		= rgrs_rec.rate;
		pcln_rec.ovhd_var	= rgrs_rec.ovhd_var;
		pcln_rec.ovhd_fix	= rgrs_rec.ovhd_fix;
		pcln_rec.setup		= 0L;
		pcln_rec.run			= 0L;
		pcln_rec.clean		= 0L;

		pcln_rec.qty_rsrc	= 1;
		pcln_rec.instr_no	= 0;

		sprintf (pcln_rec.yld_clc, "%-4.4s", yld_clc);
		pcln_rec.amt_recptd	= 0.00;
		strcpy (pcln_rec.can_split, "Y");
		strcpy (pcln_rec.act_qty_in, "N");

		if (!pcln_exist)
		{
			cc = abc_add (pcln, &pcln_rec);
			if (cc)
				file_err (cc, pcln, "DBADD");
		}
		else
		{
			cc = abc_update (pcln, &pcln_rec);
			if (cc)
				file_err (cc, pcln, "DBUPDATE");
		}
	}
	pcat_rec.stat_flag [0] = 'U';
	cc = abc_update (pcat, &pcat_rec);
	if (cc)
		file_err (cc, pcat, "DBUPDATE");

	return (EXIT_SUCCESS);
}

void
ReverseVarianceCost (
 void)
{
	double	stdDirCost, stdOvhCost,
			actDirCost, actOvhCost,
			costDiff;
	long	stdTime, actTime,
			wipDirHash, wipDRecHash,
			wipFixHash, wipFRecHash,
			mfgDirHash, mfgFixHash;
	int		TYPE = 0;
	char	wipDirAcc [17], wipDRecAcc [17],
			wipFixAcc [17], wipFRecAcc [17],
			mfgDirAcc [17], mfgFixAcc [17];

	/* calculate standard time */
	stdTime = pcln_rec.setup + pcln_rec.run + pcln_rec.clean;
	/* calculate actual time already posted */
	actTime = pcrq_rec.act_setup +
			pcrq_rec.act_run +
			pcrq_rec.act_clean;

	/* calculate standard costs */
	stdDirCost  = (double) stdTime * pcln_rec.rate;
	stdDirCost /= (double) 60.00;
	stdDirCost *= pcln_rec.qty_rsrc;
	stdOvhCost  = (double) stdTime * pcln_rec.ovhd_var;
	stdOvhCost /= (double) 60.00;
	stdOvhCost *= pcln_rec.qty_rsrc;
	stdOvhCost += pcln_rec.ovhd_fix;
	/* calculate actual costs already posted.*/
	actDirCost  = (double) actTime * pcln_rec.rate;
	actDirCost /= (double) 60.00;
	actDirCost *= pcln_rec.qty_rsrc;
	actOvhCost  = (double) actTime * pcln_rec.ovhd_var;
	actOvhCost /= (double) 60.00;
	actOvhCost *= pcln_rec.qty_rsrc;
	actOvhCost += pcln_rec.ovhd_fix;

	/* Read for works centre. */
	pcwc_rec.hhwc_hash	=	pcln_rec.hhwc_hash;
	if (find_rec (pcwc, &pcwc_rec, EQUAL, "r"))
		strcpy (pcwc_rec.work_cntr, "        ");

	/*-------------------------------------------------------
	| read the resource file, read the appropriate accounts |
	| use the appropriate accounts, if not setup use the    |
	| system wide accounts (GL_GLI).                          |
	-------------------------------------------------------*/
	switch (rgrs_rec.type [0])
	{
	case	'L': TYPE = 0; break;
	case	'M': TYPE = 1; break;
	case	'Q': TYPE = 2; break;
	case	'S': TYPE = 3; break;
	case	'O': TYPE = 4; break;
	}
	
	/* Read appropriate G/L accounts. */
	glmrRec.hhmr_hash	=	rgrs_rec.dir_hash;
	if (!find_rec (glmr, &glmrRec, EQUAL, "r"))
	{
	    wipDirHash = glmrRec.hhmr_hash;
	    strcpy (wipDirAcc, glmrRec.acc_no);
	}
	else
	{
		wipDirHash = dir_hash [TYPE];
		strcpy (wipDirAcc, dir_acc [TYPE]);
	}
	glmrRec.hhmr_hash	=	rgrs_rec.dir_rec_hash;
	if (!find_rec (glmr, &glmrRec, EQUAL, "r"))
	{
	    wipDRecHash = glmrRec.hhmr_hash;
	    strcpy (wipDRecAcc, glmrRec.acc_no);
	}
	else
	{
		wipDRecHash = dir_rec_hash [TYPE];
		strcpy (wipDRecAcc, dir_rec_acc [TYPE]);
	}
	glmrRec.hhmr_hash	=	rgrs_rec.fix_hash;
	if (!find_rec (glmr, &glmrRec, EQUAL, "r"))
	{
	    wipFixHash = glmrRec.hhmr_hash;
	    strcpy (wipFixAcc, glmrRec.acc_no);
	}
	else
	{
		wipFixHash = fix_hash [TYPE];
		strcpy (wipFixAcc, fix_acc [TYPE]);
	}
	glmrRec.hhmr_hash	=	rgrs_rec.fix_rec_hash;
	if (!find_rec (glmr, &glmrRec, EQUAL, "r"))
	{
	    wipFRecHash = glmrRec.hhmr_hash;
	    strcpy (wipFRecAcc, glmrRec.acc_no);
	}
	else
	{
		wipFRecHash = fix_rec_hash [TYPE];
		strcpy (wipFRecAcc, fix_rec_acc [TYPE]);
	}
	glmrRec.hhmr_hash	=	rgrs_rec.mfg_dir_hash;
	if (!find_rec (glmr, &glmrRec, EQUAL, "r"))
	{
		mfgDirHash = glmrRec.hhmr_hash;
		strcpy (mfgDirAcc, glmrRec.acc_no);
	}
	else
	{
		mfgDirHash = mfg_d_hash [TYPE];
		strcpy (mfgDirAcc, mfg_d_acc [TYPE]);
	}
	glmrRec.hhmr_hash	=	rgrs_rec.mfg_fix_hash;
	if (!find_rec (glmr, &glmrRec, EQUAL, "r"))
	{
		mfgFixHash = glmrRec.hhmr_hash;
		strcpy (mfgFixAcc, glmrRec.acc_no);
	}
	else
	{
		mfgFixHash = mfg_f_hash [TYPE];
		strcpy (mfgFixAcc, mfg_f_acc [TYPE]);
	}

	/*--------------------------------------
	| Std cost less act cost will give the |
	| opposite difference to the preivous  |
	| posting.                             |
	--------------------------------------*/
	/* WIP Direct Account Postings */
	AddPcgl (wipDRecHash, 
			wipDRecAcc, 
			pcwc_rec.work_cntr, 
			actDirCost, 
			"1");
	AddPcgl (wipDirHash, 
			wipDirAcc, 
			pcwc_rec.work_cntr, 
			stdDirCost, 
			"2");
	costDiff = stdDirCost - actDirCost;
	AddPcgl (mfgDirHash,
			mfgDirAcc,
			pcwc_rec.work_cntr,
			 (costDiff > 0) ? costDiff : 0 - costDiff,
			 (costDiff > 0) ? "1" : "2");

	/* WIP Fixed/Overhead Account Postings */
	AddPcgl (wipFRecHash, 
			wipFRecAcc, 
			pcwc_rec.work_cntr, 
			actOvhCost, 
			"1");
	AddPcgl (wipFixHash, 
			wipFixAcc, 
			pcwc_rec.work_cntr, 
			stdOvhCost, 
			"2");
	costDiff = stdOvhCost - actOvhCost;
	AddPcgl (mfgFixHash,
			mfgFixAcc,
			pcwc_rec.work_cntr,
			 (costDiff > 0) ? costDiff : 0 - costDiff,
			 (costDiff > 0) ? "1" : "2");
}

void
UpdatePcrq (
 void)
{
	long	setup	= pcat_rec.setup, 
			run		= pcat_rec.run, 
			clean	= pcat_rec.clean;

	while (1)
	{
		if (pcrq_rec.act_date == 0L ||
			pcat_rec.date < pcrq_rec.act_date ||
			 (pcat_rec.date == pcrq_rec.act_date &&
			pcat_rec.start_time < pcrq_rec.act_time))
		{
			pcrq_rec.act_date = pcat_rec.date;
			pcrq_rec.act_time = pcat_rec.start_time;
		}

		pcrq_rec.act_setup += setup;
		if (pcrq_rec.act_setup > pcrq_rec.est_setup)
		{
			setup = pcrq_rec.act_setup - pcrq_rec.est_setup;
			pcrq_rec.act_setup = pcrq_rec.est_setup;
		}
		else
			setup = 0L;
		pcrq_rec.act_run   += run;
		if (pcrq_rec.act_run > pcrq_rec.est_run)
		{
			run = pcrq_rec.act_run - pcrq_rec.est_run;
			pcrq_rec.act_run = pcrq_rec.est_run;
		}
		else
			run = 0L;
		pcrq_rec.act_clean += clean;
		if (pcrq_rec.act_clean > pcrq_rec.est_clean)
		{
			clean = pcrq_rec.act_clean - pcrq_rec.est_clean;
			pcrq_rec.act_clean = pcrq_rec.est_clean;
		}
		else
			clean = 0L;

		if (pcrq_rec.seq_no < pcwo_rec.rtg_seq)
			strcpy (pcrq_rec.stat_flag, "A");
		else
			strcpy (pcrq_rec.stat_flag, "E");

		cc = abc_update (pcrq, &pcrq_rec);
		if (cc)
			file_err (cc, pcrq, "DBUPDATE");

		cc = find_rec (pcrq, &pcrq_rec, NEXT, "u");
		if (cc)
		{
			cc = find_rec (pcrq, &pcrq_rec, LAST, "u");
			break;
		}
		if
		 (
			pcrq_rec.hhwo_hash	!= pcat_rec.hhwo_hash||
			pcrq_rec.seq_no	!= pcat_rec.seq_no	||
			pcrq_rec.hhrs_hash	!= pcat_rec.hhrs_hash
		)
		{
			abc_unlock (pcrq);
			cc = find_rec (pcrq, &pcrq_rec, PREVIOUS, "u");
			break;
		}
	}
	pcrq_rec.act_setup += setup;
	pcrq_rec.act_run   += run;
	pcrq_rec.act_clean += clean;
	abc_update (pcrq, &pcrq_rec);
}

/*-----------------------------------
| Find yield calc for this sequence |
| off other pcln records for this   |
| sequence. (If they exist)       |
-----------------------------------*/
void
GetYield (
 void)
{
	pcln2_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcln2_rec.seq_no    = pcat_rec.seq_no;
	pcln2_rec.line_no   = 0;
	cc = find_rec (pcln, &pcln2_rec, GTEQ, "r");
	if (cc ||
		pcln2_rec.hhwo_hash != pcwo_rec.hhwo_hash ||
		pcln2_rec.seq_no    != pcat_rec.seq_no)
	{
		sprintf (yld_clc, "%-4.4s", " ");
	}

	sprintf (yld_clc, "%-4.4s", pcln2_rec.yld_clc);
}

/*---------------------------
| Log error to audit report |
---------------------------*/
void
LogError (
 int err_num)
{
	static	int	pipe_open = FALSE;

	if (!pipe_open)
	{
		if ( (fout = popen ("pformat", "w")) == (FILE *) NULL)
			sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);
		pipe_open = TRUE;

		HeadingOutput ();
	}

	/*--------------
	| Output error |
	--------------*/
	switch (err_num)
	{
	case 1:
		fprintf (fout, "| Could Not Find a  PCWO record for the PCAT ");
		fprintf (fout, 
			"record in question (pcwo_hhwo_hash = %10ld)%-9.9s|\n", 
			pcat_rec.hhwo_hash, 
			" ");
		fprintf (fout, "|%-100.100s |\n", " ");
		break;

	case 2:
		fprintf (fout, "| Could Not Find an INEI record for the PCAT record");
		fprintf (fout, 
			" in question (hhbr = %10ld  br = %2.2s wh = %2.2s) |\n", 
			pcat_rec.hhwo_hash, 
			pcwo_rec.br_no,
			pcwo_rec.wh_no);
		fprintf (fout, "|%-100.100s |\n", " ");
		break;

	case 3:
		fprintf (fout, "| Could Not Find an Interface Failed the PCAT record");
		fprintf (fout, "|%-100.100s |\n", " ");
		break;
	default:
		return;
	}

	return;
}

/*---------------------
| Prepare output pipe |
---------------------*/
void
HeadingOutput (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", local_rec.printerNo);

	fprintf (fout, ".9\n");
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".L103\n");
	fprintf (fout, ".B1\n");

	fprintf (fout, ".ECOMPANY %s : %s\n", 
		comm_rec.co_no, clip (comm_rec.co_name));

	fprintf (fout, ".EBRANCH %s : %s\n", 
		comm_rec.est_no, clip (comm_rec.est_name));

	fprintf (fout, ".EWAREHOUSE %s : %s\n", 
		comm_rec.cc_no, clip (comm_rec.cc_name));

	fprintf (fout, ".B1\n");
	fprintf (fout, ".EPRODUCTION CONTROL TIME UPDATE ERRORS\n");

	fprintf (fout, "==================================");
	fprintf (fout, "===================================");
	fprintf (fout, "==================================\n");

	fprintf (fout, ".R=================================");
	fprintf (fout, "===================================");
	fprintf (fout, "===================================\n");
}

/*------------------------------
| Update to the payroll system |
| if it is installed           |
------------------------------*/
void
UpdatePayRoll (
 void)
{
	return;
}

/*===============================
| Add a trans to the pcgl file.	|
| NB: amount should be in cents	|
===============================*/
void
AddPcgl 
 (
	long	hash,
	char	*acc, 
	char	*wc, 
	double	amount,
	char	*type
)
{
	int		periodMonth;

	if (amount == 0.00)
		return;

	strcpy (pcgl_rec.acc_no, acc);
	strcpy (pcgl_rec.co_no, comm_rec.co_no);
	pcgl_rec.hhgl_hash = hash;
	strcpy (pcgl_rec.tran_type, "19");
	pcgl_rec.post_date = comm_rec.inv_date;
	pcgl_rec.tran_date = comm_rec.inv_date;
	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);
	sprintf (pcgl_rec.period_no, "%02d", periodMonth);
	sprintf (pcgl_rec.sys_ref, "%5.1d", comm_rec.term);
	sprintf (pcgl_rec.user_ref, "%8.8s", wc);
	strcpy (pcgl_rec.stat_flag, "2");
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	pcgl_rec.amount 	= amount;
	pcgl_rec.loc_amount = amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, loc_curr);
	strcpy (pcgl_rec.jnl_type, type);

	cc = abc_add (pcgl, &pcgl_rec);
	if (cc)
		file_err (cc, pcgl, "DBADD");

	if (!needPcgl)
		needPcgl = TRUE;
}
