/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_emp_rate.c,v 5.1 2002/01/10 07:06:01 scott Exp $
|  Program Name  : (cm_emp_rate.c)
|  Program Desc  : (Contract Management Employee Rates Input) 
|---------------------------------------------------------------------|
|  Date Written  : (25/02/93)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: cm_emp_rate.c,v $
| Revision 5.1  2002/01/10 07:06:01  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_emp_rate.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_emp_rate/cm_emp_rate.c,v 5.1 2002/01/10 07:06:01 scott Exp $";

#include	<pslscr.h>	
#include	<ml_std_mess.h>	
#include	<ml_cm_mess.h>	

#define	COMPANY	2
#define	BRANCH	1
#define	USER	0

#include	"schema"

struct commRecord	comm_rec;
struct cmemRecord	cmem_rec;
struct cmerRecord	cmer_rec;
struct cmhrRecord	cmhr_rec;
struct cmcbRecord	cmcb_rec;
struct cmcmRecord	cmcm_rec;
struct cmcdRecord	cmcd_rec;

	char	*data	= "data",
			*cmcm2	= "cmcm2";

	int  	newCode = 0;
	int		envCmAutoCon;

	char	branchNo [3];

	extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy [11];
	char	employeeCode [11];
	char	employeeName [41];
	char	cont_no [7];
	char	cost_head [5];
	long	hhhr_hash;
	long	hhcm_hash;
	char	dfltCostHead [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "employeeCode",	4, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "",  "Employee Code             ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.employeeCode},
	{1, LIN, "employeeName",	4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.employeeName},
	{1, LIN, "cont_no",	5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Contract Number           ", "",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cont_no},
	{1, LIN, "costhd",	6, 2, CHARTYPE,
		"UUUU", "          ",
		" ", local_rec.dfltCostHead, "Costhead Code             ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cost_head},

	{1, LIN, "chg_lrate",	8, 2, MONEYTYPE,
		"NNN.NN", "          ",
		" ", " ", "Labour Rate (amount/hr)   ", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&cmer_rec.lab_rate},
	{1, LIN, "chg_lpc",	9, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "Labour Overhead Percent   ", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&cmer_rec.lab_pc},
	{1, LIN, "chg_orate",	10, 2, MONEYTYPE,
		"NNN.NN", "          ",
		" ", " ", "Overhead Rate (amount/hr) ", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&cmer_rec.o_h_rate},
	{1, LIN, "chg_opc",	11, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "Overhead Percent          ", " ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&cmer_rec.o_h_pc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*
 * Main Processing Routine.
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
int		LoadCmer		(void);
int		Update			(void);
void	SrchCmem		(char *);
void	SrchCmcb		(char *);
void	SrchCmcm		(char *);
void	SrchCmhr		(char *);
int		heading			(int);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char * argv [])
{
	char	*sptr;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	sptr = get_env ("CM_AUTO_CON");
	envCmAutoCon = (sptr == (char *)0) ? COMPANY : atoi (sptr);

	strcpy (local_rec.dfltCostHead, " ");

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB (); 

	strcpy (branchNo, (envCmAutoCon == COMPANY) ? " 0" : comm_rec.est_no);

   	while (prog_exit == 0)
   	{
   		entry_exit 	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		search_ok 	= TRUE;
   		restart 	= FALSE;
   		newCode 	= FALSE;
		init_vars (1);
	
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
			
		if (restart)
			continue;

		Update ();
   	}
   	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program shutdown sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cmcm2, cmcm);

	open_rec (cmcm,  cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmcb,  cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");
	open_rec (cmcm2, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmem,  cmem_list, CMEM_NO_FIELDS, "cmem_id_no");
	open_rec (cmhr,  cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmer,  cmer_list, CMER_NO_FIELDS, "cmer_id_no");
	open_rec (cmcd,  cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (cmcb);
	abc_fclose (cmcm);
	abc_fclose (cmcm2);
	abc_fclose (cmem);
	abc_fclose (cmer);
	abc_fclose (cmhr);
	abc_fclose (cmcd);

	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	/*
	 * Valdate Tax Code.
	 */
	if (LCHECK ("employeeCode"))
	{
		if (SRCH_KEY)
		{
			SrchCmem (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmem_rec.co_no, comm_rec.co_no);
		sprintf (cmem_rec.emp_no, "%-10.10s", local_rec.employeeCode);
		cc = find_rec (cmem, &cmem_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess053));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.employeeName, "%40.40s", cmem_rec.emp_name);
		DSP_FLD ("employeeName");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cont_no"))
	{
		if (dflt_used)
		{
			local_rec.hhhr_hash = 0L;
			strcpy (local_rec.cont_no, "      ");
			strcpy (local_rec.dfltCostHead, "");
			return (EXIT_SUCCESS);
		}
		strcpy (local_rec.dfltCostHead, " ");

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		sprintf (cmhr_rec.cont_no, "%-6.6s", pad_num (local_rec.cont_no));
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.hhhr_hash = cmhr_rec.hhhr_hash;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("costhd"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.cost_head, "    ");
			local_rec.hhcm_hash = 0L;

			if (LoadCmer ())
			{
				entry_exit = TRUE;
				scn_display (1);
			}
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			if (local_rec.hhhr_hash == 0L)
				SrchCmcm (temp_str);
			else
				SrchCmcb (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmcm_rec.co_no, comm_rec.co_no);
		sprintf (cmcm_rec.ch_code, "%-4.4s", local_rec.cost_head);
		cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess055));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.hhcm_hash = cmcm_rec.hhcm_hash;

		if (LoadCmer ())
		{
			entry_exit = TRUE;
			scn_display (1);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("chg_lrate"))
	{
		if (dflt_used)
			cmer_rec.lab_rate = cmem_rec.clab_rate;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("chg_lpc"))
	{
		if (dflt_used)
			cmer_rec.lab_pc = cmem_rec.clab_pc;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("chg_orate"))
	{
		if (dflt_used)
			cmer_rec.o_h_rate = cmem_rec.coh_rate;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("chg_opc"))
	{
		if (dflt_used)
			cmer_rec.o_h_pc = cmem_rec.coh_pc;

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
LoadCmer (void)
{
	newCode = FALSE;

	cmer_rec.hhem_hash = cmem_rec.hhem_hash;
	cmer_rec.hhhr_hash = local_rec.hhhr_hash;
	cmer_rec.hhcm_hash = local_rec.hhcm_hash;
	cc = find_rec (cmer, &cmer_rec, COMPARISON, "u");
	if (cc)
	{
		newCode = TRUE;
		return (FALSE);
	}
	return (TRUE);
}

/*
 * Update files.
 */
int
Update (void)
{
	if (newCode)
	{
		cc = abc_add (cmer, &cmer_rec);
		if (cc)
			file_err (cc, cmer, "DBADD");
	}
	else
	{
		cc = abc_update (cmer, &cmer_rec);
		if (cc)
			file_err (cc, cmer, "DBUPDATE");
	}
	return (EXIT_SUCCESS);
}

/*
 * Search for Employee master file.
 */
void
SrchCmem (
	char	*keyValue)
{
	_work_open (10,0,40);
	save_rec ("#Employee", "#Employee Name");

	strcpy (cmem_rec.co_no, comm_rec.co_no);
	sprintf (cmem_rec.emp_no, "%-10.10s", keyValue);
	cc = find_rec (cmem, &cmem_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmem_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmem_rec.emp_no, keyValue, strlen (keyValue)))
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
		file_err (cc, cmem, "DBFIND");
}

void
SrchCmcb (
	char	*keyValue)
{
	int cc1;

	_work_open (4,0,40);
	save_rec ("#Code", "#Costhead Description");

	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = 0L;
	cc1 = find_rec (cmcb, &cmcb_rec, GTEQ, "r");
	if (cmcb_rec.hhhr_hash != cmhr_rec.hhhr_hash)
		cc1 = TRUE;

	if (!cc1)
	{
		cmcm_rec.hhcm_hash	=	cmcb_rec.hhcm_hash;
		cc = find_rec (cmcm2, &cmcm_rec, EQUAL, "r");
	}
	while (!cc && !cc1 && !strcmp (cmcm_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc1 = find_rec (cmcb, &cmcb_rec, NEXT, "r");
		if (cmcb_rec.hhhr_hash != cmhr_rec.hhhr_hash)
			cc1 = TRUE;

		if (!cc1)
		{
			cmcm_rec.hhcm_hash	=	cmcb_rec.hhcm_hash;
			cc = find_rec (cmcm2, &cmcm_rec, EQUAL, "r");
		}
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
SrchCmcm (
	char	*keyValue)
{
	_work_open (4,0,40);
	save_rec ("#Code", "#Costhead Description");

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", keyValue);
	cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmcm_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmcm_rec.ch_code, keyValue, strlen (keyValue)))
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

void
SrchCmhr (
 char *	keyValue)
{
	char	contDesc [51];

	_work_open (6,0,60);
	save_rec ("#No.", "#Contract Number Description");

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", keyValue);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo) &&
	       !strncmp (cmhr_rec.cont_no, keyValue, strlen (keyValue)))
	{
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
		if (!cc && cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash && 
				   cmcd_rec.stat_flag [0] == 'D')
		{
			sprintf (contDesc, "%-50.50s", cmcd_rec.text);
		}
		else
			sprintf (contDesc, "%-50.50s", " ");

		cc = save_rec (cmhr_rec.cont_no, contDesc);
		if (cc)
			break;

		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-10.10s", temp_str);
	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

int
heading (
 int	scn)
{
	int	s_size = 80;

	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);
	clear ();

	rv_pr (ML (mlCmMess037), (80 - strlen (ML (mlCmMess037))) / 2, 0, 1);

	box (0, 3, 80, 8);

	line_at (1,0, s_size);
	line_at (7,1, s_size - 1);
	line_at (20, 0, s_size);

	us_pr (ML (mlCmMess039), (80 - strlen (ML (mlCmMess039))) / 2, 7, 1);

	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0, err_str, comm_rec.co_no, comm_rec.co_name);

	strcpy (err_str, ML (mlStdMess039));
	print_at (22,0, err_str, comm_rec.est_no, comm_rec.est_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;

	scn_write (scn);
	
	return (EXIT_SUCCESS);
}
