/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_st_sprt.c,v 5.2 2001/08/09 09:20:07 scott Exp $
|  Program Name  : (sk_st_sprt.c & sk_st_ssprt.c)                    |
|  Program Desc  : (Stock Count Sheet Report for Serial Items   )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: sk_st_sprt.c,v $
| Revision 5.2  2001/08/09 09:20:07  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:57  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_st_sprt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_st_sprt/sk_st_sprt.c,v 5.2 2001/08/09 09:20:07 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<Costing.h>

#define	GROUP		0
#define	ITEM		1

#define	BY_ITEM		 (print_type == ITEM)

#define	SERIAL		 (inmr_rec.serial_item [0] == 'Y')
#define	MODE_OK		 (incc_rec.stat_flag [0] == mode [0])
#define	NON_FREEZE	 (incc_rec.stat_flag [0] == '0' && sk_st_pfrz)

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inexRecord	inex_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct excfRecord	excf_rec;


	char	*data = "data";

	int	lpno = 1,
		line_num = 0,
		first_flag = 1;
		
	int	show_serial = 0;
	int	print_type;

	FILE	*ftmp;

	char	temp_cat [13],
			old_cat [13],
			sr_group [13];

	char	lower [17];
	char	upper [17];
	char	mode [2];

	char	*inval_cls;
 	char 	*result;

	int	sk_st_pfrz = 0;

/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	ReadMisc 			 (void);
void 	Process 			 (void);
void 	ProcessGroup 		 (void);
void 	ProcessItem 		 (void);
void 	ProcessIncc 		 (void);
void 	PrintLine 			 (void);
int  	GetSerial 			 (int, char *);
void 	PrintHeading 		 (void);
void 	EndReport 			 (void);
void 	PrintCategory 		 (void);
void 	PrintInex 			 (int, int);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc != 6)
	{
		print_at (0,0,mlSkMess365, argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("SK_ST_PFRZ");
	if (sptr != (char *)0)
		sk_st_pfrz = atoi (sptr);

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];
	if (!strncmp (sptr, "sk_st_ssprt", 11))
		show_serial = 1;

	lpno = atoi (argv [1]);

	switch (argv [4][0])
	{
	case	'G':
	case	'g':
		sprintf (lower, "%-12.12s", argv [2]);
		sprintf (upper, "%-12.12s", argv [3]);
		print_type = GROUP;
		break;

	case	'I':
	case	'i':
		sprintf (lower, "%-16.16s", argv [2]);
		sprintf (upper, "%-16.16s", argv [3]);
		print_type = ITEM;
		break;

	default:
		print_at (1,0,mlSkMess365, argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (mode, "%-1.1s", argv [5]);

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
	{	
		inval_cls = p_strsave (sptr);
	}
	else
		inval_cls = "ZKPN";
	upshift (inval_cls); 

	/*============================ 
	| Open main database files . |
	============================*/
	OpenDB ();

    if (prog_exit) {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }
    
	PrintHeading ();
	Process ();
	EndReport ();	

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
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
	ReadMisc ();

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	OpenInsf ();
}	

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (excf);
	abc_fclose (incc);
	CloseCosting ();
	abc_dbclose (data);
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	if (ccmr_rec.reports_ok [0] == 'N')
        prog_exit = 1;
	
	abc_fclose (ccmr);
}

void
Process (
 void)
{
	sprintf (err_str,
			"Printing Stock Take Sheets by %s",
			 (BY_ITEM) ? "Item" : "Group");
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	switch (print_type)
	{
	case	GROUP:
		ProcessGroup ();
		break;

	case	ITEM:
		ProcessItem ();
		break;
	}
}

void
ProcessGroup (
 void)
{
	char	curr_gp [13];

	abc_selfield (inmr, "inmr_id_no_3");

	strcpy (inmr_rec.co_no,     comm_rec.co_no);
	sprintf (inmr_rec.inmr_class,    "%-1.1s",   lower);
	sprintf (inmr_rec.category, "%-11.11s", lower + 1);
	sprintf (inmr_rec.item_no,  "%-16.16s", " ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{

		if ((result = strstr (inval_cls, inmr_rec.inmr_class)))
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		sprintf (curr_gp,
				"%-1.1s%-11.11s",
				inmr_rec.inmr_class,
				inmr_rec.category);
		if (strncmp (curr_gp, upper, 12) > 0)
			break;

		if (SERIAL && !strcmp (inmr_rec.supercession, "                "))
		{
			dsp_process ("Processing : ", inmr_rec.item_no);
			ProcessIncc ();
		}

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
ProcessItem (
 void)
{
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", lower);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (inmr_rec.co_no, comm_rec.co_no) && 
		   strncmp (inmr_rec.item_no, upper, 16) <= 0)
	{

		if ((result = strstr (inval_cls, inmr_rec.inmr_class)))
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		if (SERIAL && !strcmp (inmr_rec.supercession, "                "))
		{
			dsp_process ("Processing : ", inmr_rec.item_no);
			ProcessIncc ();
		}

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
ProcessIncc (
 void)
{
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (!cc && (MODE_OK || NON_FREEZE))
		PrintLine ();
}

void
PrintLine (
 void)
{
	int	first_line = 1;
	int	f_line = 1;

	sprintf (temp_cat,"%-1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);

	if (first_flag || strcmp (temp_cat, old_cat))
	{
	    if (!first_flag)
	    {
			fprintf (ftmp, "|==================");
			fprintf (ftmp, "|=========");
			fprintf (ftmp, "|============");
			fprintf (ftmp, "|==========================================");
			fprintf (ftmp, "|==========================================|\n");
	    }
	    else
			first_flag = 0;

	    PrintCategory ();
	}
		
	line_num = 0;
	f_line = GetSerial (first_line, "C");
	GetSerial (f_line, "F");

	strcpy (old_cat, temp_cat);
	dsp_process ("Item : ", inmr_rec.item_no);
}

int
GetSerial (
 int first_line, 
 char *ser_type)
{
	char	srch_stat [2];

	sprintf (srch_stat, "%-1.1s", ser_type);
	cc = FindInsf (incc_rec.hhwh_hash, 0L, "", srch_stat, "r");
	while (!cc && 
		   incc_rec.hhwh_hash == insfRec.hhwh_hash && 
		  (insfRec.status [0] == 'F' || insfRec.status [0] == 'C'))
	{
		if (first_line)
		{
			fprintf (ftmp, "| %-16.16s ", inmr_rec.item_no);
			fprintf (ftmp, "|         ");
			if (show_serial)
			{
				fprintf (ftmp, "| %-10.10s ", insfRec.location);
				fprintf (ftmp, "| %-40.40s ", insfRec.serial_no);
			}
			else
			{
				fprintf (ftmp, "| %-10.10s ", " ");
				fprintf (ftmp, "| %-40.40s ", " ");
			}
			fprintf (ftmp, "| %-40.40s |\n", inmr_rec.description);
			first_line = 0;
		}
		else
		{
			fprintf (ftmp, "| %-16.16s ", " ");
			fprintf (ftmp, "|         ");
			if (show_serial)
			{
				fprintf (ftmp, "| %-10.10s ", insfRec.location);
				fprintf (ftmp, "| %-40.40s ", insfRec.serial_no);
			}
			else
			{
				fprintf (ftmp, "| %-10.10s ", " ");
				fprintf (ftmp, "| %-40.40s ", " ");
			}
			PrintInex (line_num, FALSE);
			line_num++;
		}

		fflush (ftmp);

		cc = FindInsf (0L, 0L, "", srch_stat, "r");
	}
	if (!first_line && line_num != 0)
		PrintInex (line_num, TRUE);

	return (first_line);
}

void
PrintHeading (
 void)
{
	if ((ftmp = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*=======================
	| Start output to file. |
	=======================*/
	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp, ".LP%d\n", lpno);
	fprintf (ftmp, ".PI12\n");
	fprintf (ftmp, ".9\n");
	fprintf (ftmp, ".L129\n");
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E%s STOCK TAKE COUNT\n", clip (comm_rec.cc_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E%s\n",clip (comm_rec.co_short));

	fprintf (ftmp, ".R===================");
	fprintf (ftmp, "==========");
	fprintf (ftmp, "=============");
	fprintf (ftmp, "============================================");
	fprintf (ftmp, "============================================\n");
 
	fprintf (ftmp, "===================");
	fprintf (ftmp, "==========");
	fprintf (ftmp, "=============");
	fprintf (ftmp, "============================================");
	fprintf (ftmp, "============================================\n");

	fprintf (ftmp, "|   ITEM  NUMBER   ");
	fprintf (ftmp, "|  COUNT  ");
	fprintf (ftmp, "|  LOCATION  ");
	fprintf (ftmp, "|  S E R I A L   -   N U M B E R           ");
	fprintf (ftmp, "|  P A R T      D E S C R I P T I O N      |\n");

	fprintf (ftmp, "|------------------");
	fprintf (ftmp, "|---------");
	fprintf (ftmp, "|------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|------------------------------------------|\n");

	fflush (ftmp);
}

/*===========================================
| Routine to print final totals for report. |
===========================================*/
void
EndReport (
 void)
{
	fprintf (ftmp, ".EOF\n");
	pclose (ftmp);
}

void
PrintCategory (
 void)
{
	int		len_no;
	int		len_desc;
	char	mask [132];
	char	temp [81];

	strcpy (excf_rec.co_no, inmr_rec.co_no);
	strcpy (excf_rec.cat_no, inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		strcpy (excf_rec.cat_desc, "No Category description found.");
	sprintf (temp, "%s", clip (excf_rec.cat_desc));
	expand (err_str, temp);
	sprintf (temp, "%.11s", clip (excf_rec.cat_no));
	len_no = strlen (temp);
	len_desc = strlen (err_str);
	sprintf (mask,
			 "| %s%s (%s) %%%d.%ds |\n",
			 inmr_rec.inmr_class,
			 temp,
			 err_str,
			 118 - len_no - len_desc, 118 - len_no - len_desc);
	fprintf (ftmp, mask, " ");
}

void
PrintInex (
 int line_no, 
 int FLAG)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = line_no;

	if (FLAG)
	{
		cc = find_rec (inex, &inex_rec, NEXT, "r");
		if (cc)
			return;

		while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			fprintf (ftmp, "|         ");
			fprintf (ftmp, "| %-10.10s ",    " ");
			fprintf (ftmp, "| %-40.40s ",    " ");
			fprintf (ftmp, "| %-40.40s |\n", inex_rec.desc);
			line_num++;
			cc = find_rec (inex, &inex_rec, NEXT, "r");
		}
		fprintf (ftmp, "| %-16.16s ",    " ");
		fprintf (ftmp, "|         ");
		fprintf (ftmp, "| %-10.10s ",    " ");
		fprintf (ftmp, "| %-40.40s ",    " ");
		fprintf (ftmp, "| %-40.40s |\n", " ");
	}
	else
	{
		cc = find_rec (inex, &inex_rec, GTEQ, "r");
		if (cc)
		{
			fprintf (ftmp, "| %-40.40s |\n", " ");
			return;
		}

		fprintf (ftmp, "| %-40.40s |\n", inex_rec.desc );
	}
}
