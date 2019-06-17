/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_ir_prn.i.c  )                                 |
|  Program Desc  : ( Multiple print for transfers.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ithr, itln, ccmr,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  ithr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 21/03/91         |
|---------------------------------------------------------------------|
|  Date Modified : (07/11/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (24/04/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (17/09/97)      | Modified  by : Elizabeth D. Paid |
|  Date Modified : (15/10/97)      | Modified  by : Elizabeth D. Paid |
|                                                                     |
|  Comments      : (07/11/91) - Changed reference to label tr_comm    |
|                : tr_ref.                                            |
|  (24/04/92)    : Disabled all interactivity so that this process    |
|                : can be run periodically by cron.                   |
|  (17/09/97)    : SEL -  Multilingual Conversion, changed printf to  |
|                :        print_at                                    |
|                                                                     |
| $Log: ir_prn.i.c,v $
| Revision 5.3  2002/07/18 07:15:54  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 09:18:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:07  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:08  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:27  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:00  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/11/11 05:59:43  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.7  1999/11/03 07:32:05  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.6  1999/10/08 05:32:27  scott
| First Pass checkin by Scott.
|
| Revision 1.5  1999/06/20 05:20:08  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ir_prn.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ir_prn.i/ir_prn.i.c,v 5.3 2002/07/18 07:15:54 scott Exp $";

#define	MAXWIDTH	150
#define	MAXLINES	100
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	REPRINT		( print_flag[0] == 'Y' )
#define	BRANCH		1
#define	WAREHOUSE	2
#define	MC_TRAN		( mc_tran == TRUE )

	/*------------------------------------------
	| Stores hashes for those records printed. |
	------------------------------------------*/
	int	by_what = WAREHOUSE;
	int	mc_tran = FALSE;

	int	lpno;
	int	type_flag;

	int	do_reprint = TRUE;

	char	print_flag[2];
	char	trans_type[2];
	char	pipe_name[100];
	char	format_file[32];

	char	*comm = "comm",
			*ithr = "ithr",
			*itln = "itln",
			*ccmr = "ccmr",
			*data = "data";

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_short[16];
		char	tcc_no[3];
		char	tcc_short[10];
	} comm_rec;

	/*=================================
	| Inventory Transfer Header File. |
	=================================*/
	struct dbview ithr_list[] ={
		{"ithr_co_no"},
		{"ithr_type"},
		{"ithr_del_no"},
		{"ithr_hhit_hash"},
		{"ithr_tran_ref"},
		{"ithr_printed"},
		{"ithr_stat_flag"}
	};

	int ithr_no_fields = 7;

	struct {
		char	hr_co_no[3];
		char	hr_type[2];
		long	hr_del_no;
		long	hr_hhit_hash;
		char	hr_tran_ref[17];
		char	hr_printed[2];
		char	hr_stat_flag[2];
	} ithr_rec;

	/*=================================
	| Inventory Transfer Detail File. |
	=================================*/
	struct dbview itln_list[] ={
		{"itln_hhit_hash"},
		{"itln_line_no"},
		{"itln_i_hhcc_hash"},
		{"itln_stat_flag"},
	};

	int itln_no_fields = 4;

	struct {
		long	ln_hhit_hash;
		int	ln_line_no;
		long	ln_i_hhcc_hash;
		char	ln_stat_flag[2];
	} itln_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_stat_flag"}
	};

	int ccmr_no_fields = 5;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_cc_no[3];
		long	cm_hhcc_hash;
		char	cm_stat_flag[2];
	} ccmr_rec;

	FILE *pout;
	
	int	pipe_open = FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, TAB, "tr_no", MAXLINES, 3, LONGTYPE, 
		"NNNNNN", "          ", 
		"0", "0", "Trans number", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&ithr_rec.hr_del_no}, 
	{1, TAB, "tr_ref", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", "", "  Transfer Ref  ", " ", 
		NA, NO, JUSTLEFT, "", "", ithr_rec.hr_tran_ref}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void print_every (void);
void open_print (long _hash);
int  check_line (long hhit_hash);
int  find_ccmr (long hhcc_hash);
void process (void);
void ask_reprint (long _hhit_hash);
int  spec_valid (int field);
void ithr_search (char *key_val);
int  heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr;

	if (argc != 6)
	{
		print_at(0,0, mlSkMess229,argv[0]);
		return (EXIT_FAILURE);
	}
	sptr = strrchr (argv[0], '/');
	if (sptr == (char *) 0)
		sptr = argv[0];
	else
		sptr++;
	if ( !strcmp (sptr, "sk_ir_prn.i" ) )
		mc_tran = FALSE;
	else
		mc_tran = TRUE;

	SETUP_SCR (vars);

	tab_col = 8 ;

	lpno = atoi(argv[1]);

	switch (argv[2][0])
	{
	case	'Y':
	case	'y':
		strcpy(print_flag,"Y");
		break;

	case	'N':
	case	'n':
		strcpy(print_flag,"N");
		break;

	default:
		print_at(0,0, mlSkMess230);
		return (EXIT_FAILURE);
	}

	switch (argv[4][0])
	{
	case	'B':
	case	'b':
		by_what = BRANCH;
		break;

	case	'W':
	case	'w':
		by_what = WAREHOUSE;
		break;

	default :
		print_at(0,0, mlSkMess231);

	/*	("<C/B/W> must be C(ompany) B(ranch) W(arehouse)\n\r");*/

		return (EXIT_FAILURE);
	}

	sprintf( trans_type, "%-1.1s", argv[3]);
	strcpy( format_file, argv[5] );

	OpenDB();

	init_scr();
	set_tty(); 
	set_masks();

	if (print_flag[0] == 'N')
	{
		print_every();
		shutdown_prog();
        return (EXIT_SUCCESS);
	}

	while (prog_exit == 0)
	{
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		init_vars (1);
		lcount [1] = 0;

		heading(1);
		entry(1);

		if (prog_exit || restart)
			break;

		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			break;

		prog_exit = 1;

		if (lcount[1] != 0)
			process();
	}
	shutdown_prog();
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

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen(data);

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec(ithr,ithr_list,ithr_no_fields,"ithr_id_no");
	open_rec(itln,itln_list,itln_no_fields,"itln_id_no");
	open_rec(ccmr,ccmr_list,ccmr_no_fields,"ccmr_hhcc_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose(ithr);
	abc_fclose(itln);
	abc_fclose(ccmr);
	abc_dbclose(data);
}

/*====================================
| Print all records not yet printed. |
====================================*/
void
print_every (
 void)
{
	do_reprint = TRUE;
		
	strcpy(ithr_rec.hr_co_no,comm_rec.tco_no);
	sprintf(ithr_rec.hr_type,"%-1.1s", trans_type);
	ithr_rec.hr_del_no = 0L;
	cc = find_rec(ithr,&ithr_rec,GTEQ,"r");

	while ( !cc && !strcmp(ithr_rec.hr_co_no,comm_rec.tco_no) && 
		ithr_rec.hr_type[0] == trans_type[0] )
	{
		if ( ithr_rec.hr_printed[0] != print_flag[0] )
		{
			cc = find_rec(ithr,&ithr_rec,NEXT,"r");
			continue;
		}
		if ( check_line( ithr_rec.hr_hhit_hash ) )
		{
			if ( !pipe_open )
				open_print( ithr_rec.hr_hhit_hash );

			fprintf( pout, "%ld\n", ithr_rec.hr_hhit_hash);
			fflush(pout);

/*
COMMENTED OUT AS PER REQUIREMENTS FOR REMOTE ISSUES BEING PRINTED BY CRON
			while ( do_reprint )
				ask_reprint( ithr_rec.hr_hhit_hash );
*/

			sprintf(err_str, "%06ld", ithr_rec.hr_del_no);
			dsp_process("TRANS NO",err_str);
		}
		cc = find_rec(ithr,&ithr_rec,NEXT,"r");
	}
	if ( pipe_open )
		pclose(pout);
	return;
}

/*=======================================
| Open pipe to transfers print program. |
=======================================*/
void
open_print (
 long _hash)
{
	sprintf( pipe_name, "sk_trnprt %d %s", lpno, format_file);

	if ((pout = popen(pipe_name,"w")) == (FILE *) NULL)
	{
		sprintf(err_str,"Error in %s during (POPEN)","sk_trnprt");
		sys_err(err_str,errno,PNAME);
	}
	pipe_open = TRUE;
}

/*======================================================
| Check that current transfer is valid based on lines. |
======================================================*/
int
check_line (
 long hhit_hash)
{
	itln_rec.ln_hhit_hash = hhit_hash;
	itln_rec.ln_line_no = 0;
	cc = find_rec(itln, &itln_rec, GTEQ, "r");
	while ( !cc && itln_rec.ln_hhit_hash == hhit_hash )
	{
		if ( find_ccmr( itln_rec.ln_i_hhcc_hash ) )
		{
			cc = find_rec(itln, &itln_rec, NEXT, "r");
			continue;
		}
		if ( by_what == BRANCH )
		{
			if ( !strcmp( ccmr_rec.cm_est_no, comm_rec.test_no) )
				return( TRUE );
		}
		if ( by_what == WAREHOUSE )
		{
			if ( !strcmp( ccmr_rec.cm_est_no, comm_rec.test_no) &&
			     !strcmp( ccmr_rec.cm_cc_no, comm_rec.tcc_no))
				return( TRUE );
		}
		cc = find_rec(itln, &itln_rec, NEXT, "r");
	}
	return( FALSE );
}
		
/*=================
| Find warehouse. |
=================*/
int
find_ccmr (
 long hhcc_hash)
{
	return( find_hash(ccmr,&ccmr_rec,COMPARISON,"r",hhcc_hash) );
}

/*===============================================
| Process printing of transfers for data input. |
===============================================*/
void
process (
 void)
{
	do_reprint = TRUE;

		
	for (line_cnt = 0;line_cnt < lcount[1];line_cnt++)
	{
		getval(line_cnt);

		strcpy(ithr_rec.hr_co_no,(MC_TRAN) ? "  " : comm_rec.tco_no);
		strcpy(ithr_rec.hr_type, trans_type);
		if (  !find_rec(ithr,&ithr_rec,COMPARISON,"r") )
		{
			if ( !pipe_open )
				open_print( ithr_rec.hr_hhit_hash );

			fprintf( pout, "%ld\n", ithr_rec.hr_hhit_hash);
			fflush(pout);

/*
COMMENTED OUT AS PER REQUIREMENTS FOR REMOTE ISSUES BEING PRINTED BY CRON
			while ( do_reprint )
				ask_reprint( ithr_rec.hr_hhit_hash );
*/

			sprintf(err_str, "%06ld", ithr_rec.hr_del_no);
			dsp_process("TRANS NO",err_str);
		}
	}
	if ( pipe_open )
		pclose(pout);

	return;
}

void
ask_reprint (
 long _hhit_hash)
{
	int	c;

	sleep( 2 );
	clear();
	box( 15,0,50,2 );

	rv_pr(ML(mlSkMess235),26,0,1);
	crsr_on();

	c = prmptmsg(ML(mlSkMess234),"YynN",25,2);
	dsp_screen("Printing transfer dockets.",
				comm_rec.tco_no,comm_rec.tco_name);
	if ( c == 'N' || c == 'n' ) 
	{
		do_reprint = FALSE;
		return;
	}
	fprintf( pout, "%ld\n", _hhit_hash);
	fflush( pout );
	return;
}

int
spec_valid (
 int field)
{
	if (LCHECK("tr_no")) 
	{
		if (last_char == SEARCH)
		{
			ithr_search(temp_str);
			return(0);
		}
		strcpy(ithr_rec.hr_co_no ,(MC_TRAN) ? "  " : comm_rec.tco_no);
		strcpy(ithr_rec.hr_type, trans_type);
		cc = find_rec(ithr,&ithr_rec,COMPARISON, "r" );
		if ( cc )
		{
			sprintf (err_str, ML(mlSkMess232), ithr_rec.hr_del_no);
			print_mess (err_str);
			sleep(2);
			return(1);
		}
		if ( !check_line( ithr_rec.hr_hhit_hash ) )
		{
			sprintf (err_str, ML(mlSkMess233), ithr_rec.hr_del_no);
			print_mess (err_str);
			sleep(2);
			return(1);
		}
		DSP_FLD( "tr_ref" );
		return(0);
	}
	return(0);
}

/*=============================
| Search for valid transfers. |
=============================*/
void
ithr_search (
 char *key_val)
{
	char	del_no[7];

	work_open();
	save_rec( "#Transfer No","#Comments " );
	strcpy(ithr_rec.hr_co_no, (MC_TRAN) ? "  " : comm_rec.tco_no);
	strcpy(ithr_rec.hr_type, trans_type);
	ithr_rec.hr_del_no = atol(key_val);

	cc = find_rec(ithr,&ithr_rec,GTEQ,"r" );

	while (!cc && !strcmp(ithr_rec.hr_co_no,
				(MC_TRAN) ? "  " : comm_rec.tco_no) )
	{
		if ( ithr_rec.hr_type[0] == trans_type[0] && 
	             check_line( ithr_rec.hr_hhit_hash ) )
		{
			sprintf(del_no,"%06ld",ithr_rec.hr_del_no);
			cc = save_rec(del_no,ithr_rec.hr_tran_ref);
			if (cc)
				break;

		}
		cc = find_rec(ithr,&ithr_rec,NEXT,"r" );
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(ithr_rec.hr_co_no, (MC_TRAN) ? "  " : comm_rec.tco_no);
	strcpy(ithr_rec.hr_type, trans_type);
	ithr_rec.hr_del_no = atol(temp_str);
	cc = find_rec(ithr,&ithr_rec,GTEQ,"r" );
	if (cc)
		sys_err( "Error in ithr during (DBFIND)",cc,PNAME);
}

int
heading( 
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	clear();

	strcpy(err_str, ML(mlSkMess236));
	rv_pr(err_str,(80 - strlen(err_str)) / 2,0,1);

	move(0,1);
	line(80);

	move(0,20);
	line(80);

	print_at(21,0, ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);

	if ( by_what == BRANCH )
	{
		print_at(22,0,ML(mlStdMess039),comm_rec.test_no,comm_rec.test_short);
		move(0,23);
		line(80);
	}
	else
	{
		move(0,22);
		line(80);
	}

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write(scn);
    return (EXIT_SUCCESS);
}
