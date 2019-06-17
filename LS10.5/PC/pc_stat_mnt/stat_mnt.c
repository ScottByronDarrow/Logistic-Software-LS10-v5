/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : (pc_stat_mnt )                                     |
|  Program Desc  : (Production Control Status Maintenance.      )     |
| $Id: stat_mnt.c,v 5.3 2002/07/08 04:28:18 scott Exp $
|---------------------------------------------------------------------|
|  Access files  :  comm, pcwo, inmr,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pcwo,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (11/02/92)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
|  Date Modified : (12/02/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (16/03/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (04/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (16/09/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (21/09/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (22/12/93)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (18/01/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (03/09/97)      | Modified  by : Leah Manibog.     |
|                                                                     |
|  Comments      : (12/02/92) - Accept lpno as a command line arg.    |
|                : Pass Y as 3rd argument to pc_chk_iss.              |
|  (16/03/92)    : Fixed parameter passed to CTRL macro.              |
|  (04/08/92)    : Allow for scheduling bypass. S/C DPL 7487          |
|  (16/09/92)    : Disallow above if pcms_iss_seq says it shouldn't   |
|                : S/C DPL 7485.                                      |
|  (21/09/92)    : Fix to re-allow confirmation of 'P'lanned orders.  |
|                : S/C PSL 7777.                                      |
|  (22/12/93)    : DPL 9894 - Added 3 new arguments to pc_chk_iss.    |
|  (18/01/94)    : DPL 9673 - Allow the user to enter a batch number  |
|                : for Planned W/O to Firm Planned W/O.               |
|  (03/09/97)    : Updated for Multilingual Conversion.               |
|                                                                     |
| $Log: stat_mnt.c,v $
| Revision 5.3  2002/07/08 04:28:18  scott
| S/C 004080 - The <TAG> and <UNTAG> buttons has the same function. Each should have different function.
|
| Revision 5.2  2001/08/09 09:14:46  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:04  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:23  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/04/05 03:36:11  scott
| Updated due to function conflict with some o/s
|
| Revision 4.1  2001/04/02 00:21:06  scott
| Updated to fix re-display problem with toggle.
|
| Revision 4.0  2001/03/09 02:31:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2001/03/07 03:47:57  robert
| fixed stat_keys declaration under ifdef GVISION block. declaration not properly ended.
|
| Revision 3.2  2001/03/06 03:04:11  scott
| Updated to for buttons on LS10-GUI
|
| Revision 3.1  2001/01/29 01:09:46  scott
| Updated to fix small warning message on mass compile.
|
| Revision 3.0  2000/10/10 12:17:06  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:14  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.14  2000/07/10 09:33:50  scott
| S/C SEL-16441 / LSDI-3044
| (1) Updated to add message as per service call.
| (2) Updated to clean code and add app.schema
|
| Revision 1.13  2000/06/29 08:15:32  jonel
| added alert for work orders gone issuing ... sc#16441
|
| Revision 1.12  2000/03/09 00:03:08  cam
| Changes for GVision compatibility.  Fixed display of batch screen.
|
| Revision 1.11  1999/11/12 10:37:47  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.10  1999/09/29 10:11:37  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/17 08:26:25  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.8  1999/09/13 07:03:17  marlene
| *** empty log message ***
|
| Revision 1.7  1999/09/09 06:12:32  marlene
| *** empty log message ***
|
| Revision 1.6  1999/06/17 07:40:46  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: stat_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_stat_mnt/stat_mnt.c,v 5.3 2002/07/08 04:28:18 scott Exp $";

#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct pcmsRecord	pcms_rec;
struct pcwoRecord	pcwo_rec;
struct pcwoRecord	pcwo3_rec;

	char	*data	= "data", 
			*pcwo2	= "pcwo2", 
			*pcwo3	= "pcwo3";

#define	TO_ALLOC	 (sourceStatus [0] == 'F')

FILE	*pout;

char	*dayOfWeek [] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", ""};
char	sourceStatus [2];
char	sourceStatusDesc [20];
char	nextStatusDesc [20];
int		numberRecordsInTable;
int		processOK;
int		printerNumber;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	tempDate [23];
	char	batch_no [11];
	char	order_no [8];
	char	store_batch [11];
} local_rec;

static	int	user_tag_func 	 (int, KEY_TAB *);
static	int	exit_func 		 (int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB stat_keys [] =
{
   { " TAG/UNTAG ", 		'T', user_tag_func, 
	"Tag/Untag current user for CC.", 				"A"}, 
   { " ACCEPT ALL `", 	CTRL ('A'), user_tag_func, 
	"Tag/Untag current user for CC.", 				"A"}, 
   { NULL, 		'\r', user_tag_func, 
	"Tag/Untag current user for CC.", 				"A"}, 
   { NULL, 		FN16, exit_func, 
	"Selection of users complete.", 				"A"}, 
   { NULL, 		FN1, exit_func, 
	"Selection of users complete.", 				"A"}, 
   END_KEYS
};
#else
static	KEY_TAB stat_keys [] =
{
   { " [T]oggle TAG/UNTAG ", 		'T', user_tag_func, 
	"Tag/Untag current user for CC.", 				"A"}, 
   { " [^A]ccept All", 	CTRL ('A'), user_tag_func, 
	"Tag/Untag current user for CC.", 				"A"}, 
   { NULL, 		'\r', user_tag_func, 
	"Tag/Untag current user for CC.", 				"A"}, 
   { NULL, 		FN16, exit_func, 
	"Selection of users complete.", 				"A"}, 
   { NULL, 		FN1, exit_func, 
	"Selection of users complete.", 				"A"}, 
   END_KEYS
};
#endif

static struct var vars [] =
{
	{1, LIN, "order_no", 9, 49, CHARTYPE,
		"UUUUUUU", "          ",
		" ", "", "Order Number  :", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.order_no},
	{1, LIN, "item_no", 10, 49, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number   :", " ",
		NA, NO, JUSTLEFT, "", "", inmr_rec.item_no},
	{1, LIN, "item_desc", 11, 49, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Description   :", " ",
		NA, NO, JUSTLEFT, "", "", inmr_rec.description},
	{1, LIN, "batch_no", 13, 49, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Batch Number  :", "Unique Batch Number for this Works Order Number",
		YES, NO, JUSTLEFT, "", "", local_rec.batch_no},

	{0, LIN, "", 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include    <tabdisp.h>

/*=====================
| function prototypes |
=====================*/
void	shutdown_prog 	 (void);
void	OpenDB 			 (void);
void	CloseDB 		 (void);
int		LoadTable 		 (void);
int		NeedIssue 		 (void);
int		ProcessTagged 	 (void);
void	EnterBatch 		 (void);
int		heading 		 (int);
int		spec_valid 		 (int);

int		issueMessageFlag	=	FALSE;

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int  argc, 
 char *argv [])
{
	char	*sptr = get_env ("PS_VAL_TIMES");

	if (argc != 3)
	{
		print_at (0,0, mlPcMess708, argv [0]);
		print_at (1,0, mlPcMess709);
		print_at (2,0, mlPcMess710);
		print_at (3,0, mlPcMess711);

		return (EXIT_FAILURE);
	}

	/*-----------------
	| Validate Status |
	-----------------*/
	printerNumber = atoi (argv [1]);
	sprintf (sourceStatus, "%-1.1s", argv [2]);
	switch (sourceStatus [0])
	{
	case 'P':
		strcpy (sourceStatusDesc, 	ML ("Planned     "));
		strcpy (nextStatusDesc, 	ML ("Firm Planned"));
		break;

	case 'F':
		strcpy (sourceStatusDesc, 	ML ("Firm Planned"));
		strcpy (nextStatusDesc, 	ML ("Allocated   "));
		break;

	case 'A':
		if (strchr (sptr, 'Y') != (char *) 0)
		{
			print_at (0,0, ML (mlPcMess040));
			sleep (sleepTime);
			exit (0);
		}
		strcpy (sourceStatusDesc, 	ML ("Allocated   "));
		strcpy (nextStatusDesc, 	ML ("Released    "));
		break;

	default:
		print_at (0,0, ML (mlPcMess708), argv [0]);
		print_at (1,0, ML (mlPcMess709));
		print_at (2,0, ML (mlPcMess710));
		print_at (3,0, ML (mlPcMess711));

		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);
	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	heading (0);
	LoadTable ();
	if (numberRecordsInTable == 0)
	{
		putchar (BELL);
		fflush (stdout);
		strcpy (err_str,ML ("There are no Works Orders that have a status of"));
		tab_add 
		 (
			"stat_tab", 
			"%-20.20s  ***** %s (%s) *****  ", 
			" ", 
			err_str,
			sourceStatusDesc
		);
		tab_display ("stat_tab", TRUE);
		sleep (sleepTime);
		tab_close ("stat_tab", TRUE);
	}
	else
	{
		processOK = FALSE;
		tab_scan ("stat_tab");
		if (processOK)
			ProcessTagged ();
		tab_close ("stat_tab", TRUE);
	}

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (pcwo2, pcwo);
	abc_alias (pcwo3, pcwo);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_hhwo_hash");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no2");
	open_rec (pcwo2, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (pcwo3, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no3");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pcms);
	abc_fclose (inmr);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (pcwo3);
	abc_dbclose (data);
}

/*------------------------------------
| Load List Of W/Os Of Chosen Status |
------------------------------------*/
int
LoadTable (
 void)
{
	numberRecordsInTable = 0;

	/*------------
	| Open Table |
	------------*/
	tab_open ("stat_tab", stat_keys, 3, 4, 12, FALSE);
	tab_add ("stat_tab", 
		"# %-14.14s  %-7.7s  %-10.10s  %-8.8s  %-16.16s  %-40.40s  %-14.14s ", 
		"Required  Date",
		"W/O No.",
		"Batch  No.",
		"Priority",
		"Item Number",
		"Item Description",
		"Production Qty");

	issueMessageFlag	=	TRUE;

	strcpy (pcwo_rec.co_no, comm_rec.co_no);
	strcpy (pcwo_rec.br_no, comm_rec.est_no);
	strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
	strcpy (pcwo_rec.order_status, sourceStatus);
	pcwo_rec.reqd_date = 0L;
	pcwo_rec.priority = 0;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (pcwo_rec.co_no, comm_rec.co_no) &&
	       !strcmp (pcwo_rec.br_no, comm_rec.est_no) &&
		   !strcmp (pcwo_rec.wh_no, comm_rec.cc_no) &&
	       !strcmp (pcwo_rec.order_status, sourceStatus))
	{
		inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc || (sourceStatus [0] == 'A' && NeedIssue ()))
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		sprintf (local_rec.tempDate, 
			"%-3.3s %-10.10s", 
			dayOfWeek [ (int) (pcwo_rec.reqd_date % 7L)], 
			DateToString (pcwo_rec.reqd_date));

		tab_add ("stat_tab", 
			" %-14.14s  %-7.7s  %-10.10s      %1d     %-16.16s  %-40.40s  %14.6f          %10ld", 
			local_rec.tempDate,
			pcwo_rec.order_no,
			pcwo_rec.batch_no,
			pcwo_rec.priority,
			inmr_rec.item_no,
			inmr_rec.description,
			pcwo_rec.prod_qty,
			pcwo_rec.hhwo_hash);

		numberRecordsInTable++;

		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}

	return (TRUE);
}

/*=======================================================
| Check if any materials are required for 1st seq no.	|
=======================================================*/
int
NeedIssue (
 void)
{
	/*----------------------------------
	| Check upd_seq for material issue |
	----------------------------------*/
	pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		if (pcms_rec.act_qty_in [0] == 'Y')
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		if (pcms_rec.iss_seq == 0)
		{
			if (issueMessageFlag == TRUE)
			{
				print_mess (ML("Work order has gone into issuing status due to pre-release components.")); 
				sleep (sleepTime);
			}
			issueMessageFlag = FALSE;
			return (TRUE);
		}
		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}
	return (FALSE);
}

/*============================
| Tag W/Os For Status Change |
============================*/
static int	
user_tag_func (
 int     c, 
 KEY_TAB *psUnused)
{
    int 	i	=	0;
	char	get_buf [200];

	switch (c)
	{
	case 	'T':
	case 	'U':
		tag_toggle ("stat_tab");
		break;

	case 	CTRL ('A'):
		tag_all ("stat_tab");
		break;

	case	'\r':
		tag_toggle ("stat_tab");
		break;

	default:
		break;
	}

	tab_get ("stat_tab", get_buf, EQUAL, i);
	if (!tagged (get_buf))
	{
	 	if (pcwo_rec.order_status [0] == 'I')
		{ 
			print_mess (ML ("Work order has gone into issuing status")); 
			sleep (sleepTime);
		}
	}

	return (c);
}

static	int	
exit_func (
 int     c, 
 KEY_TAB *psUnused)
{
	if (c == FN16)
		processOK = TRUE;

	return (c);
}

int
ProcessTagged (
 void)
{
	int		i;
	char	get_buf [200];
	long	hhwo_hash;
	int		pipe_open;
	char	pipe_name [31];

#ifdef GVISION
	tab_clear ("stat_tab");
#endif	/* GVISION */

	pipe_open = FALSE;
	if (TO_ALLOC)
	{
		/*-------------------------------------------------
		| pc_chk_iss will print all lot and bin location  |
		| details and will only print the required amount |
		-------------------------------------------------*/
		sprintf (pipe_name, "pc_chk_iss %2d 0 Y Y Y Y", printerNumber);
		/*-------------------------
		| Open Pipe To pc_chk_iss |
		-------------------------*/
		if ((pout = popen (pipe_name, "w")) == (FILE *)0)
			sys_err ("Error in pc_chk_iss During (POPEN)", errno, PNAME);

		pipe_open = TRUE;
	}

	for (i = 0; i < numberRecordsInTable; i++)
	{
		tab_get ("stat_tab", get_buf, EQUAL, i);
		if (!tagged (get_buf))
			continue;

		hhwo_hash = atol (get_buf + 128);
		cc = find_hash (pcwo2, &pcwo_rec, COMPARISON, "u", hhwo_hash);
		if (cc)
			continue;

		switch (sourceStatus [0])
		{
		case	'P':
			strcpy (pcwo_rec.order_status, "F");
			EnterBatch ();
			break;

		case	'F':
			strcpy (pcwo_rec.order_status, "I");
			break;

		case	'A':
			strcpy (pcwo_rec.order_status, "R");
			break;
		}

		if (!restart)
		{
			cc = abc_update (pcwo2, &pcwo_rec);
			if (cc)
				file_err (cc, pcwo2, "DBUPDATE");
		}
		else
		{
			clear ();
			heading (0);
			box (32, 8, 62, 5);
			print_at (11,47, ML (mlPcMess041), pcwo_rec.order_no);
			sleep (sleepTime);
			restart = 0;
		}

		if (TO_ALLOC)
		{
			fprintf (pout, "%010ld\n", hhwo_hash);
			fflush (pout);
		}
	}

	if (pipe_open)
		pclose (pout);

	return (EXIT_SUCCESS);
}

void
EnterBatch (
 void)
{
	sprintf (local_rec.store_batch, "%-10.10s", " ");

	init_ok = FALSE;
	scn_set (1);
	heading (1);

	inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inmr", "DBFIND");

	strcpy (local_rec.order_no, pcwo_rec.order_no);
	strcpy (local_rec.batch_no, pcwo_rec.batch_no);
	scn_display (1);
	if (!strcmp (pcwo_rec.batch_no, "          "))
	{
		sprintf (local_rec.store_batch, "%-10.10s", " ");
		entry (1);
	}
	else
	{
		strcpy (local_rec.store_batch, pcwo_rec.batch_no);
		print_at (14,50, ML (mlPcMess042), local_rec.store_batch);
	}

	scn_write (1);
	scn_display (1);
	edit (1);
}

int
heading (
 int scn)
{
	swide ();
	clear ();

	rv_pr (ML (mlPcMess043) , 45, 0, 1);

	move (0, 1);
	line (132);

	sprintf (err_str, 
		 ML (mlPcMess044), 
		sourceStatusDesc, 
		nextStatusDesc);
	rv_pr (err_str, 35, 2, 1);

	move (0, 21);
	line (132);

	print_at (22, 0, ML (mlStdMess038),  comm_rec.co_no,  comm_rec.co_short); 
	print_at (22, 40, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 70, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_short);

	if (scn == 1)
	{
		box (32, 8, 62, 5);
		scn_write (1);
		scn_display (1);

		if (strcmp (local_rec.store_batch, "          "))
			print_at (14, 50, ML (mlPcMess042), local_rec.store_batch);
	}
	return (EXIT_FAILURE);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("batch_no"))
	{
		if (!strcmp (local_rec.batch_no, "         "))
		{
			print_mess (ML (mlPcMess095));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (pcwo3_rec.co_no, pcwo_rec.co_no);
		strcpy (pcwo3_rec.br_no, pcwo_rec.br_no);
		strcpy (pcwo3_rec.wh_no, pcwo_rec.wh_no);
		strcpy (pcwo3_rec.batch_no, local_rec.batch_no);
		cc = find_rec (pcwo3, &pcwo3_rec, EQUAL, "r");
		if (!cc)
		{
			if (strcmp (pcwo3_rec.order_no, local_rec.order_no))
			{
				print_mess (ML (mlStdMess199));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (pcwo_rec.batch_no, local_rec.batch_no);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}
