/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: emp_time.c,v 5.2 2001/08/09 08:57:24 scott Exp $
|  Program Name  : (cm_emp_time.c) 
|  Program Desc  : (Contract Management Timesheet Entry)
|---------------------------------------------------------------------|
|  Date Written  : (26/02/93)      | Author       : Simon Dubey.      |
|---------------------------------------------------------------------|
| $Log: emp_time.c,v $
| Revision 5.2  2001/08/09 08:57:24  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:15  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: emp_time.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_emp_time/emp_time.c,v 5.2 2001/08/09 08:57:24 scott Exp $";

#define MAXWIDTH 300
#define	TABLINES 7
#include	<ml_std_mess.h>	
#include	<ml_cm_mess.h>	
#include	<pslscr.h>	
#include	<GlUtils.h>	

#define	LCL_TAB_ROW 7
#define	LCL_TAB_COL 5
#define TAB_SIZE 123
#define TAB_1   16
#define TAB_2   24
#define TAB_3   33
#define TAB_4   44
#define TAB_5   53 
#define TAB_6   66 
#define TAB_7   110
#define TAB_8   119

#define	COMPANY	2
#define	BRANCH	1
#define	USER	0

#include	"schema"

struct commRecord		comm_rec;
struct comrRecord		comr_rec;
struct cmeqRecord		cmeq_rec;
struct cmeqRecord		cmeq2_rec;
struct cumrRecord		cumr_rec;
struct cmcmRecord		cmcm_rec;
struct cmcmRecord		cmcm2_rec;
struct cmcdRecord		cmcd_rec;
struct cmcbRecord		cmcb_rec;
struct cmtsRecord		cmts_rec;
struct cmemRecord		cmem_rec;
struct cmwsRecord		cmws_rec;
struct cmhrRecord		cmhr_rec;
struct cmhrRecord		cmhr2_rec;
struct cmerRecord		cmer_rec;

char 		*data	=	"data";
const char	*cmeq2	=	"cmeq2",
			*cmcm2	=	"cmcm2",
			*cmhr2	=	"cmhr2";

	FILE	*pp;

	int	wpipe_open = FALSE;
	int	first_plant  = TRUE;
	int	first_costhd = TRUE;
	int	first_cont   = TRUE;
	int	by_what;
	int	has_emp_rates;
	int	lpno;
	float	hours;

	char	loc_curr [4];

	char	cbranchNo [3];

/*============================ 
| Local & Screen Structures. |
============================*/ 
struct
{
	char	dummy [11];
	char	last_plant [11];
	char	last_cont [7];
	char	last_cost [5];
	char	wip_stat [5];
	char	systemDate [11];
	long 	lsystemDate;
	long	hhhr_hash;
	long	hhcm_hash;
	long	hheq_hash;
	float	ord;
	float	half;
	float	doub;
	float	units;
	float	ord_prf;
	float	half_prf;
	float	doub_prf;
	float	units_prf;
	double	tmp_lab;
	double	lab_cost;
	double	oh_cost;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "date",		 3, 24, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, " Date        :", "Enter Date , Default = Yesterday ",
		 YES, NO,  JUSTLEFT, "", "", (char *) &cmts_rec.date},
	{1, LIN, "emp_code",	4, 24, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", " Employee Code :", " ",
		 YES, NO,  JUSTLEFT, "", "", cmem_rec.emp_no},
	{1, LIN, "emp_name",	5, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Employee Name :", " ",
		 NA, NO,  JUSTLEFT, "", "", cmem_rec.emp_name},
	{2, TAB, "cont_no",	 MAXLINES, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Cont No. ", "Enter Contract Number, Default = Last Contract Number Entered, Search Available. ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.cont_no},
	{2, TAB, "costhd",	 0, 1, CHARTYPE,
		"UUUU", "          ",
		" ", " ",  " Csthd ", "Enter Costhead Code, Default = Last Costhead Entered, Search Available ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.ch_code},
	{2, TAB, "ord",		0, 1, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "  Ord   ", "",
		 YES, NO,  JUSTLEFT, "0.00", "999.99", (char *) &local_rec.ord},
	{2, TAB, "half",		0, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", " 1.5 Time ", "",
		 YES, NO,  JUSTLEFT, "0.00", "999.99", (char *) &local_rec.half},
	{2, TAB, "doub",		0, 1, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", " Double ", "",
		 YES, NO,  JUSTLEFT, "0.00", "999.99", (char *) &local_rec.doub},
	{2, TAB, "plant",	 0, 1, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", local_rec.last_plant, " Plant No.  ", "Enter Plant Number, Default = From Employee Record , Search Available. ",
		 YES, NO,  JUSTLEFT, "", "", cmeq_rec.eq_name},
	{2, TAB, "desc",	 	0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Plant Desc.                               ", "",
		 NA, NO,  JUSTLEFT, "", "", cmeq_rec.desc},
	{2, TAB, "units",	0, 1, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", " Units  ", "",
		 YES, NO,  JUSTRIGHT, "0.00", "999.99", (char *) &local_rec.units},
	{2, TAB, "wip_st",	0, 1, CHARTYPE,
		"UUUU", "          ",
		" ", " ", " WIP St ", "",
		 YES, NO,  JUSTLEFT, "", "", local_rec.wip_stat},
	{2, TAB, "hheq_hash",	0, 1, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.hheq_hash},
	{2, TAB, "hhhr_hash",	0, 1, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.hhhr_hash},
	{2, TAB, "hhcm_hash",	0, 1, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.hhcm_hash},
	{2, TAB, "cont_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", "",
		 ND, NO,  JUSTLEFT, "", "", cmcd_rec.text},
	{2, TAB, "csthd_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", "",
		 ND, NO,  JUSTLEFT, "", "", cmcm_rec.desc},
	{3, LIN, "code",	4, 24, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", " Employee Code :", " ",
		 NA, NO,  JUSTLEFT, "", "", cmem_rec.emp_no},
	{3, LIN, "name",	5, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Employee Name :", " ",
		 NA, NO,  JUSTLEFT, "", "", cmem_rec.emp_name},
	{3, LIN, "ord_prf",	7, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", " Total Ordinary:", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999.99", (char *) &local_rec.ord_prf},
	{3, LIN, "half_prf",	8, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", " Total Time 1.5:", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999.99", (char *) &local_rec.half_prf},
	{3, LIN, "doub_prf",	 9, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", " Total Double  :", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999.99", (char *) &local_rec.doub_prf},
	{3, LIN, "units_prf",	10, 24, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", " Total Units   :", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999.99", (char *) &local_rec.units_prf},
	{0, LIN, "",	 	0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*--------------------------
| Function declarations.   |
--------------------------*/
int		heading			(int);
int		NotInCount		(void);
int		ProofTotal		(void);
int		spec_valid		(int);
void	AddNewCmcbs		(void);
void	CloseAudit		(void);
void	CloseDB			(void);
void	FindDesc		(void);
void	LoadEmpRates	(void);
void	OpenAudit		(void);
void	OpenDB			(void);
void	Override		(void);
void	PostAmounts		(long);
void	PrintDetails	(double, double,double, float);
void	shutdown_prog	(void);
void	SrchCmcb		(void);
void	SrchCmcm		(char *);
void	SrchCmem		(char *);
void	SrchCmeq		(char *);
void	SrchCmhr		(char *);
void	SrchCmws		(char *);
void	tab_other		(int);
void	TotalHours		(void);
void	Update			(void);

/*--------------------------
| Main Processing Routine. |
--------------------------*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr = chk_env ("CM_AUTO_CON");

	if (argc != 2)
	{
		/*printf ("Usage %s : <LPNO> \007\n", argv [0]);*/
		print_at (0,0, ML (mlStdMess036), argv [0]);
		return (argc);
	}

	if (sptr)
		by_what = atoi (sptr);
	else
		by_what = COMPANY;

	lpno = atoi (argv [1]);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate () - 1L;

	tab_row = LCL_TAB_ROW;
	tab_col = LCL_TAB_COL;
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	init_vars (2);
	init_vars (3);

	OpenDB (); 
	OpenAudit ();

	strcpy (cbranchNo, (by_what != COMPANY) ? comm_rec.est_no : " 0");

   	while (!prog_exit)
   	{
		if (restart)
			abc_unlock (cmhr);

		lcount [2]	= 0;
   		entry_exit 	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		search_ok 	= TRUE;
		init_ok		= TRUE;
		restart 	= FALSE;
		/* abc_funlock (cmhr); */

		init_vars (1);
		init_vars (2);
		init_vars (3);
	
		heading (1);
		scn_display (1);

		entry (1);

		if (prog_exit || restart)
			continue;

		heading (2);
		scn_display (2);
		entry (2);

		if (prog_exit || restart || lcount [2] == 0)
			continue;

		abc_selfield (cmem, "cmem_hhem_hash");

		heading (3);
		scn_display (3);
		entry (3);

		abc_selfield (cmem, "cmem_id_no");

		if (prog_exit || restart)
			continue;

		heading (3);
		scn_display (3);
		edit (3);

		while (!ProofTotal () && !restart)
			edit_all ();

		if (restart)
			continue;

		Update ();
		local_rec.tmp_lab  = 0.00;
		local_rec.lab_cost  = 0.00;
		local_rec.oh_cost  = 0.00;

   	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	if (wpipe_open)
		CloseAudit ();
	
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
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

	abc_alias (cmcm2, cmcm);
	abc_alias (cmeq2, cmeq);
	abc_alias (cmhr2, cmhr);

	open_rec (cmcm,  cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmcm2, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmcb,  cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");

	open_rec (cmhr2, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmcd,  cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmts,  cmts_list, CMTS_NO_FIELDS, "cmts_id_no");
	open_rec (cmws,  cmws_list, CMWS_NO_FIELDS, "cmws_id_no");
	open_rec (cmem,  cmem_list, CMEM_NO_FIELDS, "cmem_id_no");
	open_rec (cmeq,  cmeq_list, CMEQ_NO_FIELDS, "cmeq_id_no");
	open_rec (cmeq2, cmeq_list, CMEQ_NO_FIELDS, "cmeq_hheq_hash");
	open_rec (cmer,  cmer_list, CMER_NO_FIELDS, "cmer_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cmhr,cmhr_list,CMHR_NO_FIELDS,"cmhr_id_no2");
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
	OpenGlmr ();
	OpenGlwk ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cmcm);
	abc_fclose (cmcm2);
	abc_fclose (cmcb);
	abc_fclose (cmcd);
	abc_fclose (cmhr);
	abc_fclose (cmws);
	abc_fclose (cmts);
	abc_fclose (cmem);
	abc_fclose (cmeq);
	abc_fclose (cmeq2);
	abc_fclose (cmhr2);
	abc_fclose (cmer);
	abc_fclose (glmr);
	abc_fclose (cumr);
	GL_CloseBatch (lpno);
	GL_Close ();

	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	int i;
	int new_costhd = FALSE;

	static int keyed = FALSE;

	if (LCHECK ("doub"))
	{
		keyed = TRUE;
	}

	if (local_rec.ord == 0.00 && local_rec.half ==0.00 &&
			local_rec.doub == 0.00 && keyed)
	{
		FLD ("plant") = YES;
		FLD ("units") = YES;
		FLD ("ord") = NA;
		FLD ("half") = NA;
		FLD ("doub") = NA;
	}
	else
	{
		FLD ("plant") = NA;
		FLD ("units") = NA;
		FLD ("ord") = YES;
		FLD ("half") = YES;
		FLD ("doub") = YES;
	}

	if (LCHECK ("date"))
	{
		if (cmts_rec.date > local_rec.lsystemDate + 1)
		{
			print_mess (ML (mlStdMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cmts_rec.date < local_rec.lsystemDate - 14)
		{
			i = prmptmsg (ML (mlCmMess041), "YNyn", 0, 23);
			clear_mess ();
			if (i == 'Y' || i == 'y')
				return (EXIT_SUCCESS);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wip_st"))
	{
		if (dflt_used || last_char == FN16)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchCmws (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmws_rec.co_no, comm_rec.co_no);
		strcpy (cmws_rec.wp_stat, local_rec.wip_stat);

		cc = find_rec (cmws, &cmws_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cmws, "DBFIND");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("code"))
	{
		cc = find_hash (cmem,&cmem_rec,EQUAL,"r",cmem_rec.hhem_hash);
		if (cc)
			file_err (cc, "cmem", "DBFIND");
		DSP_FLD ("code");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("name"))
	{
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("emp_code"))
	{
		if (SRCH_KEY)
		{
			SrchCmem (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmem_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmem, &cmem_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("emp_name");

		/*-------------------
		| get default plant |
		-------------------*/
		cc = find_hash (cmeq2, &cmeq2_rec, EQUAL, "r",
						cmem_rec.hheq_hash);
		if (cc)
			cmem_rec.hheq_hash = 0L;
		else
			strcpy (local_rec.last_plant, cmeq2_rec.eq_name);

		/*-------------------------
		| see if has any emp rates |
		-------------------------*/

		cmer_rec.hhcm_hash = 0L;
		cmer_rec.hhem_hash = cmem_rec.hhem_hash;

		cc = find_rec (cmer, &cmer_rec, GTEQ, "r");
		if (!cc && cmer_rec.hhem_hash == cmem_rec.hhem_hash)
			has_emp_rates = TRUE;
		else
			has_emp_rates = FALSE;

	    	return (EXIT_SUCCESS);
	}

	if (LCHECK ("plant"))
	{
		if (FLD ("plant") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			if (cmem_rec.hheq_hash == 0L)
				return (EXIT_FAILURE);

			local_rec.hheq_hash = cmem_rec.hheq_hash;
			strcpy (cmeq_rec.eq_name, local_rec.last_plant);
			DSP_FLD ("plant");
			DSP_FLD ("desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmeq (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmeq_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmeq, &cmeq_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess ("\007 Plant / Equipment Not Found. ");*/
			print_mess (ML (mlCmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.hheq_hash = cmeq_rec.hheq_hash;
		sprintf (local_rec.last_plant, "%-10.10s", cmeq_rec.eq_name);
		first_plant = FALSE;
		DSP_FLD ("desc");

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("costhd"))
	{

		if (dflt_used)
		{
			if (first_costhd)
				return (EXIT_FAILURE);
			
			local_rec.hhcm_hash = cmcm_rec.hhcm_hash;
			strcpy (cmcm_rec.ch_code, local_rec.last_cost);
		}

		if (SRCH_KEY)
		{
			if (search_key == FN9)
			{
				SrchCmcm (temp_str);
				return (EXIT_SUCCESS);
			}
			else
			{
				SrchCmcb ();
				return (EXIT_SUCCESS);
			}
		}

		strcpy (cmcm_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");

		while (cc)
		{
		/*	i= prmptmsg ("\007Costhead Not On File - Do You Wish To Add [Y/N] ? ", "YNyn", 18, 18);*/
			i= prmptmsg (ML (mlCmMess016) , "YNyn", 18, 18);

			if (i == 'n' || i == 'N')
			{
				print_at (18,18,"%-100.100s", " ");
				return (EXIT_FAILURE);
			}

			i = line_cnt;
			putval (line_cnt);

			new_costhd = TRUE;
			snorm ();
			* (arg) = "cm_costhd";
			* (arg+ (1)) = (char *)0;
			shell_prog (2);
			swide ();
			heading (2);

			line_cnt = i;
			if (prog_status == ENTRY)
				lcount [1] = line_cnt;

			scn_display (2);
			getval (line_cnt);
			line_display ();

			cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");
			if (!cc)
				AddNewCmcbs ();
		}

		if (!new_costhd && NotInCount ())
		{
/*			sprintf (err_str,"\007Costhead %s Is Not Set Up On Contract %s - Do You Wish To Add [Y/N] ?",cmcm_rec.ch_code,cmhr_rec.cont_no);
*/

			i = prmptmsg (ML (mlCmMess016) , "YNyn", 18, 20);

			print_at (18,18, "%-100.100s", " ");

			if (i == 'N' || i == 'n')
				return (EXIT_FAILURE);
			
			AddNewCmcbs ();
		}

		local_rec.hhcm_hash = cmcm_rec.hhcm_hash;
		strcpy (local_rec.last_cost, cmcm_rec.ch_code);
		first_costhd = FALSE;
/*		print_at (17, 18, "Contract No : %6.6s    %s", 
						cmhr_rec.cont_no,
		 				cmcd_rec.text);
		print_at (18, 18, "Costhead No : %4.4s      %s", 
						cmcm_rec.ch_code,
		  				cmcm_rec.desc);
*/
		print_at (17, 18, ML (mlStdMess069), 
						cmhr_rec.cont_no,
		 				cmcd_rec.text);
		print_at (18, 18, ML (mlStdMess070), 
						cmcm_rec.ch_code,
		  				cmcm_rec.desc);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cont_no"))
	{
		keyed = 0;
		sprintf (cmeq_rec.eq_name, "%-10.10s", " ");
		sprintf (cmeq_rec.desc, "%-40.40s", " ");
		DSP_FLD ("plant");
		DSP_FLD ("desc");

		FLD ("plant") = NA;
		FLD ("units") = NA;
		FLD ("ord") = YES;
		FLD ("half") = YES;
		FLD ("doub") = YES;

		if (dflt_used)
		{
			if (first_cont)
				return (EXIT_FAILURE);
			
			local_rec.hhhr_hash = cmhr_rec.hhhr_hash;
			strcpy (cmhr_rec.cont_no, local_rec.last_cont);
		}
		else
			pad_num (cmhr_rec.cont_no);

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, cbranchNo);
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "w");

		if (cc == -1)
			return (EXIT_FAILURE);
		if (cc)
		{
			/*print_mess ("Contract Number Not On File \007");*/
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cmhr_rec.status [0] == 'H' || 
		     cmhr_rec.status [0] == 'B' ||
		     cmhr_rec.status [0] == 'C')
		{
			/*print_mess ("\007Contract Is Not Open ");*/
			print_mess (ML (mlCmMess018)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		FindDesc ();
		strcpy (local_rec.last_cont, cmhr_rec.cont_no);
		first_cont = FALSE;
/*		print_at (17, 18, "Contract No : %6.6s    %s", 
						cmhr_rec.cont_no,
		 				cmcd_rec.text);
*/
		print_at (17, 18, ML (mlStdMess069), 
						cmhr_rec.cont_no,
		 				cmcd_rec.text);

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*---------------
| Update files. |
---------------*/
void
Update (void)
{
	int	count;
	int	cc1;
	long	lst_cont	=	0L;

	scn_set (2);

	for (count = 0; count < lcount [2]; count++)
	{
		getval (count);
		if (count == 0)
			lst_cont = local_rec.hhhr_hash;

		TotalHours ();

		if (lst_cont != local_rec.hhhr_hash)
		{
			PostAmounts (lst_cont);
			local_rec.tmp_lab  = 0.00;
			local_rec.lab_cost  = 0.00;
			local_rec.oh_cost  = 0.00;
			cmts_rec.oh_cost = 0.00;
			lst_cont = local_rec.hhhr_hash;
		}

		cmts_rec.hhem_hash = cmem_rec.hhem_hash;
		cmts_rec.hhhr_hash = local_rec.hhhr_hash;
		cmts_rec.hhcm_hash = local_rec.hhcm_hash;
		cmts_rec.hheq_hash = local_rec.hheq_hash;

		if (local_rec.hheq_hash == 0L)
		{
			cmts_rec.lab_cost  = cmem_rec.clab_rate +
					       (cmem_rec.clab_rate * 
						(cmem_rec.clab_pc / 100.00));

			local_rec.lab_cost  += cmts_rec.lab_cost * hours;

			cmts_rec.oh_cost   = cmem_rec.coh_rate +
					       (cmem_rec.coh_rate * 
						(cmem_rec.coh_pc / 100.00));

			local_rec.oh_cost   += cmts_rec.oh_cost * hours;

			PrintDetails 
			(
				cmts_rec.lab_cost,
				cmts_rec.oh_cost,
				0.00,
				hours
			);
		}
		else	
		{
			cmts_rec.lab_cost  = 0.00;
			local_rec.oh_cost    += cmeq2_rec.cost * hours;
			cmts_rec.oh_cost   = cmeq2_rec.cost;
			cmts_rec.sale      = cmeq2_rec.rate;
			PrintDetails 
			(
				0.00,
				0.00,
				cmts_rec.oh_cost,
				hours
			);
		}

		cc = find_rec (cmts, &cmts_rec, EQUAL, "u");

		if (!cc && cmts_rec.stat_flag [0] == ' ')
		{
			cmts_rec.time_ord += local_rec.ord;
			cmts_rec.time_hlf += local_rec.half;
			cmts_rec.time_dbl += local_rec.doub;
			cmts_rec.units    += local_rec.units;

			cc1 = abc_update (cmts, &cmts_rec);
		}
		else
		{
			cmts_rec.time_ord = local_rec.ord;
			cmts_rec.time_hlf = local_rec.half;
			cmts_rec.time_dbl = local_rec.doub;
			cmts_rec.units    = local_rec.units;
			cmts_rec.stat_flag [0] = ' '; 

			cc1 = abc_add (cmts, &cmts_rec);
		}

		if (cc1)
			file_err (cc1, cmts, (cc) ? "DBADD" : "DBUPDATE");

	}
	/* abc_funlock (cmhr); */

	PostAmounts (local_rec.hhhr_hash);
	glwkRec.hhgl_hash 	= 0L;
	glwkRec.amount 		= 0.00;
	glwkRec.loc_amount 	= 0.00;
	glwkRec.ci_amt 		= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	sprintf (glwkRec.acc_no,"%*.*s", MAXLEVEL,MAXLEVEL, " ");
	strcpy (glwkRec.narrative," ------------------ ");
	GL_AddBatch ();
}

void
SrchCmcm (
 char *	key_val)
{
	work_open ();
	save_rec ("#Costhead", "#Costhead Description");

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", key_val);
	cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmcm_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmcm_rec.ch_code, key_val, strlen (key_val)))
	{
		cc = save_rec (cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmcm, &cmcm_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

int
heading (
 int	scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();
	rv_pr (ML (mlCmMess042), 52, 0, 1);
	 
	if (scn == 1 || scn == 2)
		box (1, 2, 131, 3);

	if (scn == 3)
	{
		box (1, 3, 131, 7);
		line_at (6,2,130);
	}

	line_at (21,0,132);

	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 1, err_str , comm_rec.co_no, comm_rec.co_name);

	line_at (1,0,132);


	if (scn == 2)
	{
		scn_write (1);
		scn_display (1);
		/*-----
		| top |
		-----*/
		/* cnr */
		move (LCL_TAB_COL, LCL_TAB_ROW - 1);
		PGCHAR (0);
		/* line */
		move (LCL_TAB_COL + 1, LCL_TAB_ROW -1);
		line (TAB_SIZE);
		/* cnr */
		move (LCL_TAB_COL + TAB_SIZE, LCL_TAB_ROW - 1);
		PGCHAR (1);
		/* little bits */
		move (TAB_1, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_2, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_3, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_4, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_5, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_6, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_7, LCL_TAB_ROW - 1);
		PGCHAR (8);
		move (TAB_8, LCL_TAB_ROW - 1);
		PGCHAR (8);

		/*--------
		| bottom |
		--------*/
		/* cnr */
		move (LCL_TAB_COL, LCL_TAB_ROW + 9);
		PGCHAR (2);
		/* cnr */
		move (LCL_TAB_COL + TAB_SIZE, LCL_TAB_ROW + 9);
		PGCHAR (3);
		/* line */
		move (LCL_TAB_COL + 1, LCL_TAB_ROW + 9);
		line (TAB_SIZE);
		/* little bits */
		move (TAB_1, LCL_TAB_ROW + 9);
		PGCHAR (9);
		move (TAB_2, LCL_TAB_ROW + 9);
		PGCHAR (9);
		move (TAB_3, LCL_TAB_ROW + 9);
		PGCHAR (9);
		move (TAB_4, LCL_TAB_ROW + 9);
		PGCHAR (9);
		move (TAB_5, LCL_TAB_ROW + 9);
		PGCHAR (9);
		move (TAB_6, LCL_TAB_ROW + 9);
		PGCHAR (9);
		move (TAB_7, LCL_TAB_ROW + 9);
		PGCHAR (9);
		move (TAB_8, LCL_TAB_ROW + 9);
		PGCHAR (9);
	}
		
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
SrchCmcb (void)
{
	int cc1;

	work_open ();
	save_rec ("#Costhead", "#Costhead Description");

	cmcb_rec.hhhr_hash = local_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = 0L;

	cc1 = find_rec (cmcb, &cmcb_rec, GTEQ, "r");
	if (cmcb_rec.hhhr_hash != local_rec.hhhr_hash)
		cc1 = TRUE;

	if (!cc1)
		cc=find_hash (cmcm2,&cmcm2_rec,EQUAL,"r",cmcb_rec.hhcm_hash);

	while (!cc && !cc1 &&
	       !strcmp (cmcm2_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (cmcm2_rec.ch_code, cmcm2_rec.desc);
		if (cc)
			break;

		cc1 = find_rec (cmcb, &cmcb_rec, NEXT, "r");
		if (cmcb_rec.hhhr_hash != local_rec.hhhr_hash)
			cc1 = TRUE;

		if (!cc1)
			cc=find_hash (cmcm2,&cmcm2_rec,EQUAL,"r",
						cmcb_rec.hhcm_hash);
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

void
SrchCmhr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Cont. No.", "#Customer Order No.");
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, cbranchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", key_val);

	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
			!strncmp (cmhr_rec.cont_no, key_val,strlen (key_val)))
	{
		if (strcmp (cmhr_rec.br_no, cbranchNo))
			break;

		if (cmhr_rec.status [0] == 'H' || 
		     cmhr_rec.status [0] == 'B' ||
		     cmhr_rec.status [0] == 'C')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, cbranchNo);
	sprintf (cmhr_rec.cont_no, "%6.6s", temp_str);

	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

void
FindDesc (void)
{
	cmcd_rec.line_no = 0;
	cmcd_rec.stat_flag [0] = 'D';
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cmcd, "DBFIND");
}

void
AddNewCmcbs (void)
{
	strcpy (cmcb_rec.budg_type, "V");
	strcpy (cmcb_rec.dtl_lvl, "A");
	cmcb_rec.budg_cost = 0.00;
	cmcb_rec.budg_qty = 0.00;
	cmcb_rec.budg_value = 0.00;
	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;

	cc = abc_add (cmcb, &cmcb_rec);
	if (cc)
		file_err (cc, cmcb, "DBADD");
}

void
tab_other (
 int	line_no)
{
	if (prog_status == ENTRY)
		return;

	if (local_rec.hheq_hash)
	{
		FLD ("plant") = YES;
		FLD ("units") = YES;
		FLD ("ord") = NA;
		FLD ("half") = NA;
		FLD ("doub") = NA;
	}
	else
	{
		FLD ("plant") = NA;
		FLD ("units") = NA;
		FLD ("ord") = YES;
		FLD ("half") = YES;
		FLD ("doub") = YES;
	}

	print_at (18, 18, ML (mlStdMess069), 
						cmhr_rec.cont_no,
		 				cmcd_rec.text);
	print_at (19, 18, ML (mlStdMess070), 
						cmcm_rec.ch_code,
		  				cmcm_rec.desc);
}

/*----------------------------------
| Search for Employee master file. |
----------------------------------*/
void
SrchCmem (
 char *	key_val)
{
	work_open ();
	save_rec ("#Employee", "#Employee Name");

	strcpy (cmem_rec.co_no, comm_rec.co_no);
	sprintf (cmem_rec.emp_no, "%-10.10s", key_val);
	cc = find_rec (cmem, &cmem_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmem_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmem_rec.emp_no, key_val, strlen (key_val)))
	{
		cc = save_rec (cmem_rec.emp_no, cmem_rec.emp_name);
		if (cc)
			break;

		cc = find_rec (cmem, &cmem_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmem_rec.co_no, comm_rec.co_no);
	sprintf (cmem_rec.emp_no, "%-10.10s", temp_str);
	cc = find_rec (cmem, &cmem_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cmem", "DBFIND");
}

void
SrchCmeq (
 char *	key_val)
{
	work_open ();
	save_rec ("#Number","#Description");

	strcpy (cmeq_rec.co_no, comm_rec.co_no);
	cc = find_rec (cmeq, &cmeq_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmeq_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmeq_rec.eq_name, key_val, strlen (key_val)))
	{
		cc = save_rec (cmeq_rec.eq_name, cmeq_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmeq, &cmeq_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmeq_rec.co_no, comm_rec.co_no);
	sprintf (cmeq_rec.eq_name, "%-10.10s", temp_str);
	cc = find_rec (cmeq, &cmeq_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, "cmeq", "DBFIND");
}

int
NotInCount (void)
{

	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;

	cc = find_rec (cmcb, &cmcb_rec, EQUAL, "r");
	return (cc);
}

int
ProofTotal (void)
{
	int count;
	float	ProofTotal [4];
	static int first_time = TRUE;


	if (first_time)
	{
		first_time = FALSE;
		return (first_time);
	}

	for (count = 0; count < 4; count++)
		ProofTotal [count] = 0.00;

	scn_set (2);

	for (count = 0; count < lcount [2]; count++)
	{
		getval (count);
		ProofTotal [0]  += local_rec.ord;
		ProofTotal [1]  += local_rec.half;
		ProofTotal [2]  += local_rec.doub;
		ProofTotal [3]  += local_rec.units;
	}

	if (ProofTotal [0] != local_rec.ord_prf ||
	     ProofTotal [1] != local_rec.half_prf ||
	     ProofTotal [2] != local_rec.doub_prf ||
	     ProofTotal [3] != local_rec.units_prf)
	{
		/*print_mess ("\007Proof Totals Do Not Match - Please Correct ");*/
		print_mess (ML (mlCmMess043));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	else
		return (TRUE);
}

void
TotalHours (void)
{
	/*---------------------------------
	|  1) Get Employee Rates order is |
	|     a) spec cont && spec csthd  |
	|     b) spec cont && all  csthd  |
	|     c) all  cont && spec csthd  |
	|     d) use master file rates    |
	---------------------------------*/
	double	lab_hr;
	double	o_h_hr;

	if (has_emp_rates)
	{
		/*----------------------------------------------
		| firstly look for spec costhd && spec contract |
		----------------------------------------------*/
		cmer_rec.hhhr_hash = local_rec.hhhr_hash;
		cmer_rec.hhcm_hash = local_rec.hhcm_hash;
		cmer_rec.hhem_hash = cmem_rec.hhem_hash;
		cc = find_rec (cmer, &cmer_rec, EQUAL, "r");

		/*--------------------
		| then all costheads |
		--------------------*/
		if (cc)
		{
			cmer_rec.hhhr_hash = local_rec.hhhr_hash;
			cmer_rec.hhcm_hash = 0L;
			cmer_rec.hhem_hash = cmem_rec.hhem_hash;
			cc = find_rec (cmer, &cmer_rec, EQUAL, "r");
		}

		/*--------------------------------------
		| then all contracts for spec costhead |
		--------------------------------------*/
		if (cc)
		{
			cmer_rec.hhhr_hash = 0L;
			cmer_rec.hhcm_hash = local_rec.hhcm_hash;
			cmer_rec.hhem_hash = cmem_rec.hhem_hash;
			cc = find_rec (cmer, &cmer_rec, EQUAL, "r");
		}

		if (cc)
		/*---------------------
		| means nothing found |
		| so use master file  |
		---------------------*/
			LoadEmpRates ();
	}
	
	if (!has_emp_rates)
		LoadEmpRates ();

	/*---------------------
	| use contract rates  |
	| if they exists      |
	---------------------*/
	Override ();

	/*--------------------------------------
	| this is where it all gets worked out |
	--------------------------------------*/

	hours  = (local_rec.ord) +
	   	(local_rec.half * (float) 1.5) +
		(local_rec.doub * (float) 2.0) + 
		(local_rec.units);

	lab_hr = cmer_rec.lab_rate;
	o_h_hr = cmer_rec.o_h_rate;

	cmts_rec.sale = lab_hr + (lab_hr * (cmer_rec.lab_pc / 100.00));
	cmts_rec.sale += o_h_hr + (o_h_hr * (cmer_rec.o_h_pc / 100.00));
	local_rec.tmp_lab += cmts_rec.sale;

	if (local_rec.hheq_hash)
	{
		cc = find_hash (cmeq2, &cmeq2_rec, EQUAL, "r",
						local_rec.hheq_hash);
		if (cc)
			file_err (cc, "cmeq2", "DBFIND");
	}
}

void
LoadEmpRates (void)
{
	cmer_rec.o_h_rate = cmem_rec.ooh_rate;
	cmer_rec.lab_rate = cmem_rec.olab_rate;
	cmer_rec.lab_pc = cmem_rec.olab_pc;
	cmer_rec.o_h_pc = cmem_rec.ooh_pc;
}

void
PostAmounts (
 long	hhhr_hash)
{
	int		monthPeriod;

	DateToDMY (local_rec.lsystemDate + 1L, NULL, &monthPeriod, NULL);

	/*---------------------
	| get gls from header |
	---------------------*/
	abc_selfield (cmhr2, "cmhr_hhhr_hash");

	cc = find_hash (cmhr2, &cmhr2_rec, EQUAL, "w", hhhr_hash);

	if (cc)
		file_err (cc, cmhr2, "DBFIND");

	/*-----------------
	| get debtor info |
	-----------------*/
	cc = find_hash (cumr, &cumr_rec,EQUAL,"r",cmhr2_rec.hhcu_hash);
	if (cc)
		file_err (cc, cumr, "DBFIND");

	/*--------------------------
	| update header wip status |
	--------------------------*/
	if (strcmp (local_rec.wip_stat, "    "))
	{
		strcpy (cmhr2_rec.wip_status, local_rec.wip_stat);
		cc = abc_update (cmhr2, &cmhr2_rec);
		if (cc)
			file_err (cc, cmhr2, "DBUPDATE");
	}

	/*----------
	| post WIP |
	----------*/
	/*------------------
	| lookup hhgl_hash |
	------------------*/
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr2_rec.wip_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	sprintf (glwkRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr2_rec.wip_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym,   "%-10.10s", cumr_rec.dbt_no);
	sprintf (glwkRec.name,      "%-30.30s", cumr_rec.dbt_name);
	sprintf (glwkRec.chq_inv_no,"%-15.15s", cmhr2_rec.cont_no);
	sprintf (glwkRec.jnl_type,  "%-1.1s",   "1");  
	sprintf (glwkRec.tran_type, "%-2.2s",   "21"); 
	glwkRec.ci_amt = glwkRec.amount =  local_rec.lab_cost + 
			  			   local_rec.oh_cost;

	sprintf (glwkRec.sys_ref, "%05d", comm_rec.term);
	glwkRec.tran_date = cmts_rec.date;
	glwkRec.post_date = local_rec.lsystemDate + 1;
	sprintf (glwkRec.user_ref,"%6.6s", cmhr2_rec.cont_no);
	sprintf (glwkRec.stat_flag, "%-1.1s",   "2");  
	sprintf (glwkRec.period_no, "%02d", monthPeriod);

	strcpy (glwkRec.narrative,"Labour Recovery     ");
	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();

	/*----------
	| post LAB |
	----------*/
	/*------------------
	| lookup hhgl_hash |
	------------------*/
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr2_rec.lab_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	sprintf (glwkRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr2_rec.lab_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym,   "%-10.10s", cumr_rec.dbt_no);
	sprintf (glwkRec.name,      "%-30.30s", cumr_rec.dbt_name);
	sprintf (glwkRec.chq_inv_no,"%-15.15s", cmhr2_rec.cont_no);
	sprintf (glwkRec.jnl_type,  "%-1.1s",   "2");  
	sprintf (glwkRec.tran_type, "%-2.2s",   "21"); 
	glwkRec.ci_amt = glwkRec.amount = local_rec.lab_cost;

	sprintf (glwkRec.sys_ref, "%05d", comm_rec.term);
	glwkRec.tran_date = cmts_rec.date;
	glwkRec.post_date = local_rec.lsystemDate + 1;
	sprintf (glwkRec.user_ref,"%6.6s", cmhr2_rec.cont_no);
	sprintf (glwkRec.stat_flag, "%-1.1s",   "2");  
	sprintf (glwkRec.period_no, "%02d", monthPeriod);

	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();

	/*----------
	| post O/H |
	----------*/
	/*------------------
	| lookup hhgl_hash |
	------------------*/
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr2_rec.o_h_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	strcpy (glwkRec.acc_no, cmhr2_rec.o_h_glacc);
	sprintf (glwkRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr2_rec.o_h_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym,   "%-10.10s", cumr_rec.dbt_no);
	sprintf (glwkRec.name,      "%-30.30s", cumr_rec.dbt_name);
	sprintf (glwkRec.chq_inv_no,"%-15.15s", cmhr2_rec.cont_no);
	sprintf (glwkRec.jnl_type,  "%-1.1s",   "2");  
	sprintf (glwkRec.tran_type, "%-2.2s",   "21");  
	glwkRec.ci_amt = glwkRec.amount = local_rec.oh_cost;

	sprintf (glwkRec.sys_ref, "%05d", comm_rec.term);
	glwkRec.tran_date = cmts_rec.date;
	glwkRec.post_date = local_rec.lsystemDate + 1;
	sprintf (glwkRec.user_ref,"%6.6s", cmhr2_rec.cont_no);
	sprintf (glwkRec.stat_flag, "%-1.1s",   "2");  
	sprintf (glwkRec.period_no, "%02d", monthPeriod);

	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();
}

void
SrchCmws (
 char *	key_val)
{
	work_open ();
	save_rec ("#WIP St", "#Description ");

	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%-4.4s", key_val);
	cc = find_rec (cmws, &cmws_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmws_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmws_rec.wp_stat, key_val, strlen (key_val)))
	{
		cc = save_rec (cmws_rec.wp_stat, cmws_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmws, &cmws_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%-4.4s", temp_str);
	cc = find_rec (cmws, &cmws_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmws, "DBFIND");
}

void
Override (void)
{
	if (cmhr_rec.lab_rate)
	{
		cmer_rec.lab_rate = cmhr_rec.lab_rate;
		cmer_rec.lab_pc = 0.00;
	}

	if (cmhr_rec.oh_rate)
	{
		cmer_rec.o_h_rate = cmhr_rec.oh_rate;
		cmer_rec.o_h_pc = 0.00;
	}
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (void)
{
	if ((pp = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	wpipe_open = TRUE;

	sprintf (err_str,"%-10.10s <%s>",DateToString (comm_rec.inv_date),PNAME);
	fprintf (pp, ".START%s\n",clip (err_str));
	fprintf (pp, ".SO\n");
	fprintf (pp, ".LP%d\n",lpno);
	fprintf (pp, ".12\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".ETIMESHEET ENTRIES TO CONTRACTS\n");

	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s as at %24.24s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (pp, ".B2\n");

	fprintf (pp,".EBRANCH %s  \n",clip (comm_rec.est_name));

	fprintf (pp,".R===========");
	fprintf (pp,"===========");
	fprintf (pp,"=======");
	fprintf (pp,"=========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"===========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"==================================\n");

	fprintf (pp,"===========");
	fprintf (pp,"===========");
	fprintf (pp,"=======");
	fprintf (pp,"=========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"===========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"==========");
	fprintf (pp,"==================================\n");

	fprintf (pp,"!   DATE   ");
	fprintf (pp,"! EMPLOYEE ");
	fprintf (pp,"! CONT ");
	fprintf (pp,"! CSTHD ");
	fprintf (pp,"!   ORD   ");
	fprintf (pp,"!   1.5   ");
	fprintf (pp,"!   DBL   ");
	fprintf (pp,"!  PLANT   ");
	fprintf (pp,"!  UNITS  ");
	fprintf (pp,"! LABOUR  ");
	fprintf (pp,"! O/HEAD  ");
	fprintf (pp,"! EQUIPT  ");
	fprintf (pp,"!   E X T E N D E D   C O S T S   !\n");

	fprintf (pp,"!          ");
	fprintf (pp,"!   CODE   ");
	fprintf (pp,"! CODE ");
	fprintf (pp,"!  CODE ");
	fprintf (pp,"!  TIME   ");
	fprintf (pp,"!  TIME   ");
	fprintf (pp,"!  TIME   ");
	fprintf (pp,"!   CODE   ");
	fprintf (pp,"!         ");
	fprintf (pp,"!  COST   ");
	fprintf (pp,"!  COST   ");
	fprintf (pp,"!  COST   ");
	fprintf (pp,"!  LABOUR  ");
	fprintf (pp,"!  O/HEAD  ");
	fprintf (pp,"!  EQUIPT   !\n");

	fprintf (pp,"!----------+");
	fprintf (pp,"----------+");
	fprintf (pp,"------+");
	fprintf (pp,"-------+");
	fprintf (pp,"---------+");
	fprintf (pp,"---------+");
	fprintf (pp,"---------+");
	fprintf (pp,"----------+");
	fprintf (pp,"---------+");
	fprintf (pp,"---------+");
	fprintf (pp,"---------+");
	fprintf (pp,"---------+");
	fprintf (pp,"----------+----------+-----------!\n");

	fprintf (pp,".PI12\n");
}

/*==============================
| Print details of data input. |
==============================*/
void
PrintDetails (
	double	lcost,
	double	ocost,
	double	pcost,
	float	qty)
{
	/*---------------------------------
	| Somes details have been printed |
	---------------------------------*/
	fprintf (pp,"!%10.10s",  DateToString (cmts_rec.date));
	fprintf (pp,"!%10.10s",cmem_rec.emp_no);
	fprintf (pp,"!%6.6s",  cmhr_rec.cont_no);
	fprintf (pp,"! %5.5s ",cmcm_rec.ch_code);
	fprintf (pp,"!%8.2f ", local_rec.ord);
	fprintf (pp,"!%8.2f ", local_rec.half);
	fprintf (pp,"!%8.2f ", local_rec.doub);
	fprintf (pp,"!%10.10s",cmeq_rec.eq_name);
	fprintf (pp,"!%8.2f ", local_rec.units);
	fprintf (pp,"!%9.2f",  DOLLARS (lcost));
	fprintf (pp,"!%9.2f",  DOLLARS (ocost));
	fprintf (pp,"!%9.2f",  DOLLARS (pcost));
	fprintf (pp,"!%10.2f", DOLLARS (lcost * qty));
	fprintf (pp,"!%10.2f", DOLLARS (ocost * qty));
	fprintf (pp,"!%10.2f !\n",DOLLARS (pcost * qty));

	fflush (pp);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (void)
{
	fprintf (pp,".EOF\n");
	pclose (pp);
}
