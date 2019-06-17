/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_emp_inpt.c,v 5.1 2002/01/10 07:07:55 scott Exp $
|  Program Name  : (cm_emp_inpt.c) 
|  Program Desc  : (Contract Management Employee Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (25/02/93)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: cm_emp_inpt.c,v $
| Revision 5.1  2002/01/10 07:07:55  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_emp_inpt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_emp_inpt/cm_emp_inpt.c,v 5.1 2002/01/10 07:07:55 scott Exp $";

#include	<pslscr.h>	
#include	<ml_std_mess.h>	
#include	<ml_cm_mess.h>	

#include	"schema"

struct commRecord	comm_rec;
struct cmemRecord	cmem_rec;
struct cmeqRecord	cmeq_rec;

	char	*data	= "data",
			*cmeq2	= "cmeq2";

   	int  	newCode = 0;
	extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy [11];
	char	plantEqCode [11];
	char	plantEqDesc [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "emp_code",	4, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Employee Code               ", "Enter employee code [SEARCH] ",
		 NE, NO,  JUSTLEFT, "", "", cmem_rec.emp_no},
	{1, LIN, "emp_name",	5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Employee Name               ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmem_rec.emp_name},

	{1, LIN, "cst_lrate",	7, 2, MONEYTYPE,
		"NNN.NN", "          ",
		" ", "", "Labour Rate (Amount/hr)     ", "Enter labour rate per hour.",
		 YES, NO,  JUSTRIGHT, "0", "999.99", (char *)&cmem_rec.clab_rate},
	{1, LIN, "cst_l_oh",	8, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "Labour Overhead Percent     ", "Enter Labour overhead percent ",
		 YES, NO,  JUSTRIGHT, "0", "999.99", (char *)&cmem_rec.clab_pc},
	{1, LIN, "cst_orate",	9, 2, MONEYTYPE,
		"NNN.NN", "          ",
		" ", "", "Overhead Rate (Amount/hr)   ", "Enter overhead rate per hour ",
		 YES, NO,  JUSTRIGHT, "0", "999.99", (char *)&cmem_rec.coh_rate},
	{1, LIN, "cst_orate",	10, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "Overhead Percent            ", "Enter overhead percent ",
		 YES, NO,  JUSTRIGHT, "0", "999.99", (char *)&cmem_rec.coh_pc},

	{1, LIN, "chg_lrate",	12, 2, MONEYTYPE,
		"NNN.NN", "          ",
		" ", "", "Labour Rate (Amount/hr)     ", "Enter labour rate per hour ",
		 YES, NO,  JUSTRIGHT, "0", "999.99", (char *)&cmem_rec.olab_rate},
	{1, LIN, "chg_l_oh",	13, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "Labour Overhead Percent     ", "Enter labour overhead percent ",
		 YES, NO,  JUSTRIGHT, "0", "999.99", (char *)&cmem_rec.olab_pc},
	{1, LIN, "chg_orate",	14, 2, MONEYTYPE,
		"NNN.NN", "          ",
		" ", "", "Overhead Rate (Amount/hr)   ", "Enter overhead rate per hour ",
		 YES, NO,  JUSTRIGHT, "0", "999.99", (char *)&cmem_rec.ooh_rate},
	{1, LIN, "chg_orate",	15, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "Overhead %                  ", "Enter overhead percent ",
		 YES, NO,  JUSTRIGHT, "0", "999.99", (char *)&cmem_rec.ooh_pc},

	{1, LIN, "plantEquipment",	17, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ","Plant / Equipment           ", "Enter plant / equipment code <default = none>",
		 YES, NO,  JUSTLEFT, "", "", local_rec.plantEqCode},
	{1, LIN, "plantEquipmentDesc",	17, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.plantEqDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*
 * Function declarations.  
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
int		Update			(void);
void	SrchCmem		(char *);
void	SrchCmeq		(char *);
int		heading			(int);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char *	argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB (); 

   	while (prog_exit == 0)
   	{
   		entry_exit	= FALSE;
   		edit_exit	= FALSE;
   		prog_exit	= FALSE;
   		search_ok	= TRUE;
   		restart		= FALSE;
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
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	abc_alias (cmeq2, cmeq);

	open_rec (cmem,  cmem_list, CMEM_NO_FIELDS, "cmem_id_no");
	open_rec (cmeq,  cmeq_list, CMEQ_NO_FIELDS, "cmeq_id_no");
	open_rec (cmeq2, cmeq_list, CMEQ_NO_FIELDS, "cmeq_hheq_hash");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cmem);
	abc_fclose (cmeq);
	abc_fclose (cmeq2);

	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	/*
	 * Valdate Tax Code.
	 */
	if (LCHECK ("emp_code"))
	{
		if (SRCH_KEY)
		{
			SrchCmem (temp_str);
			return (EXIT_SUCCESS);
		}

		newCode = FALSE;
		strcpy (cmem_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmem, &cmem_rec, COMPARISON, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			cc = find_hash (cmeq2, &cmeq_rec, COMPARISON, "r",
				       cmem_rec.hheq_hash);
			if (!cc)
			{
				sprintf (local_rec.plantEqCode, "%-10.10s", cmeq_rec.eq_name);
				sprintf (local_rec.plantEqDesc, "%-40.40s", cmeq_rec.desc);
			}

			scn_display (1);
			entry_exit = TRUE;
		}

	    	return (EXIT_SUCCESS);
	}

	if (LCHECK ("plantEquipment"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.plantEqCode, "%-10.10s", " ");
			sprintf (local_rec.plantEqDesc, "%-40.40s", " ");
			cmeq_rec.hheq_hash = 0L;
			DSP_FLD ("plantEquipment");
			DSP_FLD ("plantEquipmentDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmeq (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmeq_rec.co_no, comm_rec.co_no);
		sprintf (cmeq_rec.eq_name, "%-10.10s", local_rec.plantEqCode);
		cc = find_rec (cmeq, &cmeq_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.plantEqDesc, "%-40.40s", cmeq_rec.desc);
		DSP_FLD ("plantEquipmentDesc");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Update files.
 */
int
Update (void)
{
	cmem_rec.hheq_hash = cmeq_rec.hheq_hash;

	if (newCode)
	{
		cc = abc_add (cmem, &cmem_rec);
		if (cc)
			file_err (cc, cmem, "DBADD");
	}
	else
	{
		cc = abc_update (cmem, &cmem_rec);
		if (cc)
			file_err (cc, cmem, "DBUPDATE");
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
SrchCmeq (
	char	*keyValue)
{
	_work_open (10,0,40);
	save_rec ("#Number","#Description");

	strcpy (cmeq_rec.co_no, comm_rec.co_no);
	cc = find_rec (cmeq, &cmeq_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmeq_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmeq_rec.eq_name, keyValue, strlen (keyValue)))
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
	 	file_err (cc, cmeq, "DBFIND");
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

	line_at (1, 0, s_size);

	box (0, 3, 80, 14);

	line_at (6, 1, s_size - 1);
	us_pr (ML (mlCmMess038), (80 - strlen (ML (mlCmMess038))) / 2, 6, 1);

	line_at (11, 1, s_size - 1);
	us_pr (ML (mlCmMess039), (80 - strlen (ML (mlCmMess039))) / 2, 11, 1);

	line_at (16, 1, s_size - 1);
	line_at (20, 0, s_size);

	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0,  err_str, comm_rec.co_no, comm_rec.co_name);

	strcpy (err_str, ML (mlStdMess039));
	print_at (22,0, err_str, comm_rec.est_no, comm_rec.est_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;

	scn_write (scn);
	
	return (EXIT_SUCCESS);
}
