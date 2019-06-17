/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_cdelinp.c,v 5.3 2001/09/21 11:10:48 robert Exp $
|  Program Name  : (db_cdelinp.c)
|  Program Desc  : (Customer Delivery Instruction Maintenance)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/03/88         |
|---------------------------------------------------------------------|
| $Log: db_cdelinp.c,v $
| Revision 5.3  2001/09/21 11:10:48  robert
| updated to reset screen values when data not found
|
| Revision 5.2  2001/08/09 08:22:36  scott
| Added FinishProgram ();
|
| Revision 5.1  2001/08/06 23:21:44  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:04:00  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:24:27  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/03/08 02:11:55  scott
| Updated to increase the delivery address number from 0-999 to 0-32000
| This change did not require a change to the schema
| As a general practice all programs have had app.schema added and been cleaned
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_cdelinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_cdelinp/db_cdelinp.c,v 5.3 2001/09/21 11:10:48 robert Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cudiRecord	cudi_rec;
struct cudiRecord	cudi2_rec;
struct cumrRecord	cumr_rec;

	char	branchNumber [3];

   	int  	newInstruction 	= 0,
			envVarDbCo 	 	= 0,
			envVarDbFind 	= 0;

	char	*data	= "data",
			*cudi2	= "cudi2";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	customerNo [7];
	char	lastCustomerNo [7];
	int 	lastDelNo;
	int 	delNumber;
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "cust_no",	 4, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.lastCustomerNo, "Customer No.", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.customerNo},
	{1, LIN, "cust_name",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Customer Name.", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "delNumber",	 6, 20, INTTYPE,
		"NNNNN", "          ",
		"0", " ", "Delivery No.", " ",
		YES, NO, JUSTRIGHT, "0", "32000", (char *)&cudi_rec.del_no},
	{1, LIN, "del_name",	 8, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Name.", " ",
		YES, NO,  JUSTLEFT, "", "", cudi_rec.name},
	{1, LIN, "del_adr1",	 9, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Address 1.", " ",
		YES, NO,  JUSTLEFT, "", "", cudi_rec.adr1},
	{1, LIN, "del_adr2",	10, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Address 2.", " ",
		YES, NO,  JUSTLEFT, "", "", cudi_rec.adr2},
	{1, LIN, "del_adr3",	11, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Address 3.", " ",
		YES, NO,  JUSTLEFT, "", "", cudi_rec.adr3},
	{1, LIN, "del_adr4",	12, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Delivery Address 4.", " ",
		YES, NO,  JUSTLEFT, "", "", cudi_rec.adr4},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<FindCumr.h>

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	Update 			(void);
void 	SrchCudi 		(char *);
int		FoundDelNo 		(void);
int 	heading 		(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int		argc,
	char	*argv [])
{
	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	envVarDbCo 		= atoi (get_env ("DB_CO"));
	envVarDbFind  	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	strcpy (local_rec.lastCustomerNo,"000000");
	local_rec.lastDelNo = 0;

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Update ();
	}	/* end of input control loop	*/
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
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
	abc_alias (cudi2, cudi);
    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr,	 cumr_list, CUMR_NO_FIELDS, 
						(!envVarDbFind) ? "cumr_id_no" : "cumr_id_no3");

	open_rec (cudi2, cudi_list, CUDI_NO_FIELDS, "cudi_id_no");
	open_rec (cudi,  cudi_list, CUDI_NO_FIELDS, "cudi_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cudi);
	abc_dbclose (data);
}

int
spec_valid (
	int	field)
{
	int	last_no;
	/*--------------------
	| Validate Customer no |
	--------------------*/
	if (LCHECK ("cust_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		/*------------------------------------
		| Find Customer Master file details. |
		------------------------------------*/
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,zero_pad (local_rec.customerNo,6));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("cust_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("delNumber"))
	{
		if (SRCH_KEY)
		{
			SrchCudi (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			last_no = 0;

			cudi_rec.hhcu_hash = cumr_rec.hhcu_hash;
			cudi_rec.del_no = 0;

			cc = find_rec (cudi,&cudi_rec,GTEQ,"r");
			while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
			{
				last_no = cudi_rec.del_no;
				cc = find_rec (cudi,&cudi_rec,NEXT,"r");
			}

			if (last_no < 32000)
				last_no++;

			cudi_rec.del_no = last_no;
		}
		cudi_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cc = find_rec (cudi,&cudi_rec,COMPARISON,"u");
		if (cc)
		{
			abc_unlock (cudi);
			newInstruction = TRUE;
			
			strcpy (cudi_rec.name, "");
			strcpy (cudi_rec.adr1, "");
			strcpy (cudi_rec.adr2, "");
			strcpy (cudi_rec.adr3, "");
			strcpy (cudi_rec.adr4, "");
		}
		else
		{
			newInstruction = FALSE;
			entry_exit = TRUE;
		}
		if (prog_status == ENTRY)
			local_rec.delNumber = cudi_rec.del_no;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("del_name"))
	{
		if (dflt_used)
		{
			strcpy (cudi_rec.name,cumr_rec.dbt_name);
			DSP_FLD ("del_name");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

void
Update (void)
{
    if (newInstruction)
    {
		while (FoundDelNo ());
		if (cudi_rec.del_no > 32000) 
		{
			errmess (ML ("Cannot add record as maximum number 32000 already on file."));
			sleep (sleepTime);
			return;
		}
			
        cc = abc_add (cudi,&cudi_rec);
        if (cc)
        	file_err (cc, cudi, "DBADD");

		if (local_rec.delNumber != cudi_rec.del_no)
		{
			cudi_rec.del_no = local_rec.delNumber;
			cc = find_rec (cudi,&cudi_rec,EQUAL,"u");
			if (!cc)
			{
				cc = abc_delete (cudi); 
				if (cc)
					file_err (cc, cudi, "DBDELETE");
			}
		}
		abc_unlock (cudi);
    }
    else
    {
    	cc = abc_update (cudi,&cudi_rec);
    	if (cc)
    		file_err (cc, cudi, "DBUPDATE");
    }
	strcpy (local_rec.lastCustomerNo,cumr_rec.dbt_no);
	local_rec.lastDelNo = cudi_rec.del_no;
}

/*====================
| Search for del_no  |
====================*/
void
SrchCudi (
	char	*key_val)
{
	char	deliveryNumber [6];

	_work_open (5,0,60);
	save_rec ("#DelNo","#Delivery Name");
	cudi_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= 0;
	cc = find_rec (cudi,&cudi_rec,GTEQ,"r");
    while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
    {                        
		sprintf (deliveryNumber,"%05d",cudi_rec.del_no);
		cc = save_rec (deliveryNumber, cudi_rec.name);
		if (cc)
			break;

		cc = find_rec (cudi,&cudi_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	cudi_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cudi_rec.del_no = atoi (temp_str);
	cc = find_rec (cudi,&cudi_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, cudi, "DBFIND");
}
int
heading (int scn)
{
	if (restart)
    	return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlDbMess119),18,0,1);

	print_at (0,63,"%s/%05d",local_rec.lastCustomerNo,local_rec.lastDelNo);

	box (0,3,80,9);

	line_at (1,0,80);
	line_at (7,1,79);
	line_at (20,0,80);
	line_at (22,0,80);
	print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int FoundDelNo (void)
{
	memcpy ((char *)&cudi2_rec, (char *)&cudi_rec, sizeof (struct cudiRecord));
	cc = find_rec (cudi2,&cudi2_rec,EQUAL,"r");
	if (!cc)
	{
		cudi_rec.del_no++;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
