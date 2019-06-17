/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_req_del.c,v 5.3 2002/01/18 01:51:16 scott Exp $
|  Program Name  : (cm_req_del.c)
|  Program Desc  : (Selective Requisition Delete)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (16/03/93)       |
|---------------------------------------------------------------------|
| $Log: cm_req_del.c,v $
| Revision 5.3  2002/01/18 01:51:16  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_req_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_req_del/cm_req_del.c,v 5.3 2002/01/18 01:51:16 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>
#include <proc_sobg.h>
#include <Costing.h>

#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

#define	CAL(amt, pc)	(amt * DOLLARS (pc))
#define	MAXDELETE	2000
#define	SERIAL_ITEM	(inmr_rec.serial_item [0] == 'Y')

FILE	*pp;

#include	"schema"

struct commRecord	comm_rec;
struct cmcdRecord	cmcd_rec;
struct cmhrRecord	cmhr_rec;
struct cmrdRecord	cmrd_rec;
struct cmrhRecord	cmrh_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;

	char	*data  = "data",
			*cmrh2 = "cmrh2";

	int		printerNo	= 1,
			auditOpen 	= FALSE,
			autoReqNo	= 0,
			firstTime	= TRUE;

	long	addItem [MAXDELETE];

	char	reqBranchNo [3];

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	long	req_no;
	char	contractNo [7];
	char	desc [7][71];
} local_rec;

extern	int		TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "req_no",	 4, 2, LONGTYPE,
		"NNNNNN", "          ",
		"0", " ", "Requisition No   ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.req_no},
	{1, LIN, "contractNo",	 6, 2, CHARTYPE,
		"AAAAAA", "          ",
		" ", " ", "Contract No      ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.contractNo},
	{1, LIN, "desc1",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description      ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc [0]},
	{1, LIN, "desc2",	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                 ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc [1]},
	{1, LIN, "desc3",	 9, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                 ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc [2]},
	{1, LIN, "desc4",	 10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                 ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc [3]},
	{1, LIN, "desc5",	 11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                 ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc [4]},
	{1, LIN, "desc6",	 12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                 ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc [5]},
	{1, LIN, "desc7",	 13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                 ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc [6]},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local function prototypes 
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
void	SrchCmrh		(char *);
void	Process			(void);
void	DeleteLine		(void);
void	DeleteHeader	(long);
void	PrintDetail	 	(void);
void	OpenAudit		(void);
void	CloseAudit		(void);
int		UpdateOther		(void);
int		heading			(int);


/*
 * Main Processing Routine . 
 */
int
main (
	int		argc,
	char	*argv [])
{
	int		i;
	char *	sptr;

	TruePosition	=	TRUE;

	if (argc != 2)	
	{
		print_at (0, 0, ML (mlStdMess036), argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	(); 
	set_masks 	();

	/*
	 * Open database.
	 */
	OpenDB ();

	/*
	 * Printer No. 
	 */
	printerNo = atoi (argv [1]);
	
	/*
	 * Level of requisition numbering. 
	 */
	sptr = chk_env ("CM_AUTO_REQ");
	autoReqNo = (sptr == (char *)0) ? COMPANY : atoi (sptr);
	strcpy (reqBranchNo, (autoReqNo == COMPANY) ? " 0" : comm_rec.est_no);

	firstTime = TRUE;
	for (i = 0; i < MAXDELETE; i++)
		addItem [i] = 0L;

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		restart 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		init_vars (1);	
	
		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;
	
		/*
		 * Edit screen 1 linear input. 
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		/*
		 * Do delete. 
		 */
		Process ();
		UpdateOther ();
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
	if (auditOpen)
		CloseAudit ();

	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files . 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	abc_alias (cmrh2, cmrh);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cmcd,  cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmhr,  cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmrh,  cmrh_list, CMRH_NO_FIELDS, "cmrh_id_no");
	open_rec (cmrh2, cmrh_list, CMRH_NO_FIELDS, "cmrh_hhrq_hash");
	open_rec (cmrd,  cmrd_list, CMRD_NO_FIELDS, "cmrd_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (cmcd);
	abc_fclose (cmhr);
	abc_fclose (cmrd);
	abc_fclose (cmrh);
	abc_fclose (cmrh2);
	abc_fclose (inmr);

	CloseCosting ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Requisition Number. 
	 */
	if (LCHECK ("req_no"))
	{
		if (SRCH_KEY)
		{
			SrchCmrh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmrh_rec.co_no, comm_rec.co_no);
		strcpy (cmrh_rec.br_no, reqBranchNo);
		cmrh_rec.req_no = local_rec.req_no;
		cc = find_rec (cmrh, &cmrh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess015));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cmrh_rec.stat_flag [0] == 'C')
		{	
			print_mess (ML (mlCmMess145));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Lookup contract. 
		 */
		cmhr_rec.hhhr_hash	=	cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.contractNo, "%-6.6s", cmhr_rec.cont_no);
		DSP_FLD ("contractNo");

		/*
		 * Load contract decsription. 
		 */
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
		while (!cc &&
		       cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
		       !strcmp (cmcd_rec.stat_flag, "D") &&
		       cmcd_rec.line_no < 7)
		{
			strcpy (local_rec.desc [cmcd_rec.line_no], cmcd_rec.text);
	
			cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
		}
		DSP_FLD ("desc1");
		DSP_FLD ("desc2");
		DSP_FLD ("desc3");
		DSP_FLD ("desc4");
		DSP_FLD ("desc5");
		DSP_FLD ("desc6");
		DSP_FLD ("desc7");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchCmrh (
	char	*keyValue)
{
	char	req_no [7];
	char	desc [41];

	_work_open (6,0,40);
	save_rec ("#Req No", "#Contract | Requested By ");
	strcpy (cmrh_rec.co_no, comm_rec.co_no);
	strcpy (cmrh_rec.br_no, reqBranchNo);
	cmrh_rec.req_no = atol (keyValue);
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmrh_rec.br_no, reqBranchNo) &&
	       !strcmp (cmrh_rec.co_no, comm_rec.co_no))
	{
		if (cmrh_rec.stat_flag [0] == 'C')
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		cmhr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (req_no, "%06ld", cmrh_rec.req_no);
			sprintf (desc, "%-6.6s | %-20.20s", 
								cmhr_rec.cont_no, cmrh_rec.req_by);
			cc = save_rec (req_no, desc);
			if (cc)
				break;
		}

		cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmrh_rec.co_no, comm_rec.co_no);
	strcpy (cmrh_rec.br_no, reqBranchNo);
	cmrh_rec.req_no = atol (temp_str);
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	if (cc)
		file_err (cc, cmrh, "DBFIND");
}

/*
 * Process relevent Data. 
 */
void
Process (void)
{
	dsp_screen ("Selective Requisition Delete",comm_rec.co_no,comm_rec.co_name);

	/*
	 * Delete all lines for requisition. 
	 */
	cmrd_rec.hhrq_hash	= cmrh_rec.hhrq_hash;
	cmrd_rec.line_no 	= 0;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "u");
	while (!cc && cmrd_rec.hhrq_hash == cmrh_rec.hhrq_hash)
	{
		/*
		 * Cannot delete lines that have something issued 
		 */
		if (cmrd_rec.qty_iss != 0.00)
		{
			abc_unlock (cmrd);
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "u");
			continue;
		}
		DeleteLine ();

		cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
		cmrd_rec.line_no = 0;
		cc = find_rec (cmrd, &cmrd_rec, GTEQ, "u");
	}

	/*
	 * Delete header if no lines left. 
	 */
	cmrd_rec.hhrq_hash	= cmrh_rec.hhrq_hash;
	cmrd_rec.line_no 	= 0;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
	if (cc || cmrd_rec.hhrq_hash != cmrh_rec.hhrq_hash)
	{
		DeleteHeader (cmrh_rec.hhrq_hash);
		return;
	}
}

/*
 * Delete relevent valid line. 
 */
void
DeleteLine (void)
{
	int	i;

	/*
	 * Add a sobg_record  
	 */
	for (i = 0; i < MAXDELETE; i++)
	{
		if (addItem [i] == cmrd_rec.hhbr_hash)
			break;
		
		if (addItem [i] == 0L)
		{
			addItem [i] = cmrd_rec.hhbr_hash;
			break;
		}
	}

	/*
	 * Open audit , if not already open. 
	 */
	if (!auditOpen)
	{
		OpenAudit ();
		auditOpen = TRUE;
	}
	PrintDetail ();

	if (SERIAL_ITEM)
	{
		cc =	UpdateInsf 
			 	(
			 		0L, 
					cmrd_rec.hhbr_hash, 
					cmrd_rec.serial_no, 
					"C", 
					"F"
				);
		if (cc)
		{
			cc =	UpdateInsf 
					(
						0L, 
						cmrd_rec.hhbr_hash, 
						cmrd_rec.serial_no, 
						"S", 
						"F"
					);
		}
		if (cc)
		{
			cc =	UpdateInsf 
					(
						0L, 
						cmrd_rec.hhbr_hash, 
						cmrd_rec.serial_no, 
						"T", 
						"F"
					);
		}
	}

	cc = abc_delete (cmrd);
	if (cc)
		file_err (cc, cmrd, "DBDELETE");

	return;
}

/*
 * Delete Header Record. 
 */
void
DeleteHeader (
	long	hhrqHash)
{
	cmrh_rec.hhrq_hash	=	hhrqHash;
	cc = find_rec (cmrh2, &cmrh_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (cmrh2);
		return;
	}
	abc_delete (cmrh2);

	return;
}

void
PrintDetail (void)
{
	float	backorder = 0.0;
	double	value = 0.0;

	value = cmrd_rec.sale_price;
	value -= CAL (cmrd_rec.sale_price, cmrd_rec.disc_pc);

	value *= (double)cmrd_rec.qty_order + cmrd_rec.qty_border,
	backorder = cmrd_rec.qty_order + cmrd_rec.qty_border;

	inmr_rec.hhbr_hash = cmrd_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (inmr_rec.item_no, "%-16.16s", " Unknown Item.  ");
		sprintf (inmr_rec.description, "%-40.40s", " ");
	}

	if (firstTime)
	{
		fprintf (pp, "|-------------");
		fprintf (pp, "|----------");
		fprintf (pp, "|------------------");
		fprintf (pp, "|------------------------------------------");
		fprintf (pp, "|--------------");
		fprintf (pp, "|--------------|\n");

		fprintf (pp, "| %06ld      ",  cmrh_rec.req_no);
		fprintf (pp, "| %-6.6s   ", local_rec.contractNo);
	}
	else
	{
		fprintf (pp, "| %6.6s      ",  " ");
		fprintf (pp, "| %6.6s   ", " ");
	}
	fprintf (pp, "| %-16.16s ", inmr_rec.item_no);
	fprintf (pp, "| %40.40s ",  inmr_rec.description);
	fprintf (pp, "| %12.2f ",   backorder);
	fprintf (pp, "| %13.2f|\n", DOLLARS (value));

	firstTime = FALSE;
}

/*
 * Routine to open output pipe to standard print to provide an audit trail 
 * of events. This also sends the output straight to the spooler.         
 */
void
OpenAudit (void)
{
	if ((pp = popen ("pformat","w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	sprintf (err_str,"%-8.8s <%s>",DateToString (comm_rec.inv_date),PNAME);
	fprintf (pp,".START%s\n",clip (err_str));
	fprintf (pp,".SO\n");
	fprintf (pp,".LP%d\n",printerNo);
	fprintf (pp,".PI12\n");
	fprintf (pp,".9\n");
	fprintf (pp,".L158\n");
	fprintf (pp,".ESELECTIVE REQUISITION DELETE\n");

	fprintf (pp,".EAS AT %24.24s\n",SystemTime ());

	fprintf (pp,".ECOMPANY : %s - %s\n",clip (comm_rec.co_no),clip (comm_rec.co_name));
	fprintf (pp,".B1\n");

	fprintf (pp, ".R==============");
	fprintf (pp, "===========");
	fprintf (pp, "===================");
	fprintf (pp, "===========================================");
	fprintf (pp, "===============");
	fprintf (pp, "================\n");

	fprintf (pp, "==============");
	fprintf (pp, "===========");
	fprintf (pp, "===================");
	fprintf (pp, "===========================================");
	fprintf (pp, "===============");
	fprintf (pp, "================\n");

	fprintf (pp, "| REQUISITION ");
	fprintf (pp, "| CONTRACT ");
	fprintf (pp, "|                  ");
	fprintf (pp, "|                                          ");
	fprintf (pp, "|              ");
	fprintf (pp, "|              |\n");

	fprintf (pp, "|   NUMBER    ");
	fprintf (pp, "|  NUMBER  ");
	fprintf (pp, "|   ITEM NUMBER    ");
	fprintf (pp, "|            ITEM DESCRIPTION              ");
	fprintf (pp, "|   QUANTITY   ");
	fprintf (pp, "|    VALUE     |\n");
}

/*
 * Routine to close the audit trail output file. 
 */
void
CloseAudit (void)
{
	fprintf (pp, ".EOF\n");
	pclose (pp);
}

/*
 * Add record to background records for Order values and on order. 
 */
int
UpdateOther (void)
{
	int	i;

	clear ();
	
	rv_pr (ML (mlStdMess035),1,1,1);

	/*
	 * Recalc all records found . 
	 */
	for (i = 0; i < MAXDELETE; i++)
	{
		if (addItem [i] == 0L)
			break;

		add_hash 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"RC",
			0,
			addItem [i],
			0L,
			0L,
			(double) 0.00
		);
	}
	recalc_sobg ();
	return (EXIT_SUCCESS);
}

int
heading (
	int		scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();
	rv_pr (ML (mlCmMess146), 50, 0, 1);
	box (0, 3, 132, 10);
	line_at (5,1,131);
	line_at (1,0,132);
	line_at (20,0,132);

	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0,err_str, comm_rec.co_no, comm_rec.co_name);
	strcpy (err_str, ML (mlStdMess039));
	print_at (22,0,err_str, comm_rec.est_no, comm_rec.est_name);
	scn_write (scn);
	return (EXIT_SUCCESS);
}
