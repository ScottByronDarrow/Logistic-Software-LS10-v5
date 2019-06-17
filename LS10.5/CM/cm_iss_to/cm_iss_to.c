/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_iss_to.c,v 5.3 2002/01/10 07:07:17 scott Exp $
|  Program Name  : (cm_iss_to.c)
|  Program Desc  : (Contract Management Issue To Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (24/02/93)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: cm_iss_to.c,v $
| Revision 5.3  2002/01/10 07:07:17  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_iss_to.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_iss_to/cm_iss_to.c,v 5.3 2002/01/10 07:07:17 scott Exp $";

#include	<pslscr.h>	
#include	<ml_std_mess.h>	
#include	<ml_cm_mess.h>	

#include	"schema"

struct commRecord	comm_rec;
struct cmitRecord	cmit_rec;
struct cmemRecord	cmem_rec;
struct sumrRecord	sumr_rec;

	char	*data	= "data",
			*cmem2	= "cmem2",
			*sumr2	= "sumr2";

   	int  	newCode 	= 0,
			envCrCo		= 0,
			envCrFind	= 0,
   			empSupValid = 0;

	char	branchNumber [3];

/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy [11];
	char	type [2];
	char	type_desc [11];
	char	emp_code [11];
	char	sup_code [7];
	char	emp_desc [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "code",	 	4, 16, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Issue To Code  ", " ",
		 NE, NO,  JUSTLEFT, "", "", cmit_rec.issto},
	{1, LIN, "name",	 	5, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Issue To Name  ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmit_rec.iss_name},
	{1, LIN, "type",	 	7, 16, CHARTYPE,
		"U", "          ",
		" ", "E", "Type           ", " Enter Issue To Type - E(mployee) or S(upplier) ",
		 YES, NO,  JUSTLEFT, "ES", "", local_rec.type},
	{1, LIN, "type_desc",	7, 25, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.type_desc},
	{1, LIN, "emp_code",	8, 16, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Employee Code  ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.emp_code},
	{1, LIN, "sup_code",	8, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier Code  ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sup_code},
	{1, LIN, "emp_desc",	8, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.emp_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <FindSumr.h>

/*
 * Local function prototypes 
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
int		Update			(void);
void	SrchCmem		(char *);
void	SrchCmit		(char *);
int		heading			(int);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	envCrCo = atoi (get_env ("CR_CO"));
	envCrFind = atoi (get_env ("CR_FIND"));

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB (); 

	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

    while (prog_exit == 0)
    {
   		entry_exit	= FALSE;
   		edit_exit	= FALSE;
   		prog_exit	= FALSE;
   		search_ok	= TRUE;
   		restart		= FALSE;
   		newCode		= FALSE;
		init_vars (1);
	
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		do
		{
			scn_write (1);
			edit (1);
			if (restart)
				break;
			
			if (!empSupValid)
			{
				print_mess (ML (mlCmMess118));
				sleep (sleepTime);
				clear_mess ();
			}
		} while (!empSupValid);

		if (restart)
			continue;

		Update ();
    }
    shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
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

	abc_alias (cmem2, cmem);
	abc_alias (sumr2, sumr);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cmem,  cmem_list, CMEM_NO_FIELDS, "cmem_id_no");
	open_rec (cmem2, cmem_list, CMEM_NO_FIELDS, "cmem_hhem_hash");
	open_rec (cmit,  cmit_list, CMIT_NO_FIELDS, "cmit_id_no");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
							       							 : "sumr_id_no3");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cmem);
	abc_fclose (cmem2);
	abc_fclose (cmit);
	abc_fclose (sumr);
	abc_fclose (sumr2);

	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	/*
	 * Valdate Tax Code.
	 */
	if (LCHECK ("code"))
	{
	    if (SRCH_KEY)
	    {
			SrchCmit (temp_str);
			return (EXIT_SUCCESS);
	    }

		newCode = FALSE;
		strcpy (cmit_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmit, &cmit_rec, COMPARISON, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			/*
			 * Determine Issue To type
			 */
			if (cmit_rec.hhem_hash != 0L)
			{
				strcpy (local_rec.type, "E");
				strcpy (local_rec.type_desc, ML ("Employee"));
				FLD ("sup_code") = ND;
				FLD ("emp_code") = YES;
				display_prmpt (label ("emp_code"));

				/*
				 * Lookup Employee Code
				 */
				empSupValid = FALSE;
				cmem_rec.hhem_hash = cmit_rec.hhem_hash;
				cc = find_rec (cmem2, &cmem_rec, COMPARISON,"r");
				if (!cc)
				{
					empSupValid = TRUE;
					sprintf (local_rec.emp_code, "%-10.10s", cmem_rec.emp_no);
					sprintf (local_rec.emp_desc, "%-40.40s", cmem_rec.emp_name);
				}
			}
			else
			{
				strcpy (local_rec.type, "S");
				strcpy (local_rec.type_desc, ML ("Supplier"));
				FLD ("emp_code") = ND;
				FLD ("sup_code") = YES;
				display_prmpt (label ("sup_code"));

				/*
				 * Lookup Supplier Code
				 */
				empSupValid = FALSE;
				sumr_rec.hhsu_hash = cmit_rec.hhsu_hash;
				cc = find_rec (sumr2, &sumr_rec, COMPARISON,"r");
				if (!cc)
				{
					empSupValid = TRUE;
					sprintf (local_rec.sup_code, "%-6.6s", sumr_rec.crd_no);
					sprintf (local_rec.emp_desc, "%-40.40s", sumr_rec.crd_name);
				}
			}
			scn_display (1);
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("type"))
	{
		if (local_rec.type [0] == 'E')
		{
			strcpy (local_rec.type_desc, ML ("Employee"));
			FLD ("sup_code") = ND;
			FLD ("emp_code") = YES;
			display_prmpt (label ("emp_code"));
			empSupValid = FALSE;
		}
		else
		{
			strcpy (local_rec.type_desc, ML ("Supplier"));
			FLD ("emp_code") = ND;
			FLD ("sup_code") = YES;
			display_prmpt (label ("sup_code"));
			empSupValid = FALSE;
		}
		DSP_FLD ("type_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("emp_code"))
	{
		if (FLD ("emp_code") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchCmem (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmem_rec.co_no, comm_rec.co_no);
		sprintf (cmem_rec.emp_no, "%-10.10s", local_rec.emp_code);
		cc = find_rec (cmem, &cmem_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess053));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		empSupValid = TRUE;
		sprintf (local_rec.emp_desc, "%-40.40s", cmem_rec.emp_name);
		DSP_FLD ("emp_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sup_code"))
	{
		if (FLD ("sup_code") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
	
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.sup_code));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		empSupValid = TRUE;
		sprintf (local_rec.emp_desc, "%-40.40s", sumr_rec.crd_name);
		DSP_FLD ("emp_desc");

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
	/*
	 * Set up link hash.
	 */
	if (local_rec.type [0] == 'E')
	{
		cmit_rec.hhem_hash = cmem_rec.hhem_hash;
		cmit_rec.hhsu_hash = 0L;
	}
	else
	{
		cmit_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cmit_rec.hhem_hash = 0L;
	}

	if (newCode)
	{
		cc = abc_add (cmit, &cmit_rec);
		if (cc)
			file_err (cc, cmit, "DBADD");
	}
	else
	{
		cc = abc_update (cmit, &cmit_rec);
		if (cc)
			file_err (cc, cmit, "DBUPDATE");
	}
	return (EXIT_SUCCESS);
}

/*
 * Search for Employee master file.
 */
void
SrchCmem (
	char	*key_val)
{
	_work_open (10,0,40);
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
		file_err (cc, cmem, "DBFIND");
}

/*
 * Search for Category master file.
 */
void
SrchCmit (
	char	*key_val)
{
	_work_open (10,0,40);
	save_rec ("#Issue To", "#Issue To Name");

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", key_val);
	cc = find_rec (cmit, &cmit_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmit_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmit_rec.issto, key_val, strlen (key_val)))
	{
		cc = save_rec (cmit_rec.issto, cmit_rec.iss_name);
		if (cc)
			break;

		cc = find_rec (cmit, &cmit_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", temp_str);
	cc = find_rec (cmit, &cmit_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmit, "DBFIND");
}

int
heading (
	int		scn)
{
	int	s_size = 80;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlCmMess119), (80 - strlen (ML (mlCmMess119))) / 2, 0, 1);

		box (0, 3, 80, 5);
		line_at (1,0,s_size);
		line_at (6,1,s_size - 1);
		
		us_pr (ML (mlCmMess120), (80 - strlen (ML (mlCmMess120))) / 2, 6, 1);

		line_at (20,0,s_size);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,err_str, comm_rec.co_no, comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22,0,err_str, comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;

		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

