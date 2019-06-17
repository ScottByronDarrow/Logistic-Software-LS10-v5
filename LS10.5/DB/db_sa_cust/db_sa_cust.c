/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_sa_cust.c,v 5.4 2002/07/17 09:57:08 scott Exp $
|  Program Name  : (db_sa_cust.c)
|  Program Desc  : (Print Sales By Customer Report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_sa_cust.c,v $
| Revision 5.4  2002/07/17 09:57:08  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/12/06 09:22:11  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_sa_cust.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_sa_cust/db_sa_cust.c,v 5.4 2002/07/17 09:57:08 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cusaRecord	cusa_rec;
struct exafRecord	exaf_rec;

	Money	*cusa_val	=	&cusa_rec.val1;

	double	currentYear [12]	=	{0,0,0,0,0,0,0,0,0,0,0,0},	
			grandTotal [14]		=	{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			yearTotal			=	0.00,
			currentTotal		= 	0.00;

	char	monthName [13][4] = 
			{ 
		     "JAN", "FEB", "MAR",
		     "APR", "MAY", "JUN",
		     "JUL", "AUG", "SEP",
		     "OCT", "NOV", "DEC" 
			};

	int		customerMonth	=	0;

	FILE	*fout;

struct {
	char	dummy [11];
	int		printerNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "area_code",	 4, 20, CHARTYPE,
		"UU", "          ",
		" ", " ", "Area No.", " Default - All ",
		YES, NO, JUSTRIGHT, "", "", exaf_rec.area_code},
	{1, LIN, "area_desc",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Area Description.", " ",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{1, LIN, "printerNo",	 7, 20, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*
 * Local Function Prototypes.
 */
void 	Process 		(void);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessCumr		(void);
void 	ReportHeading 	(void);
void 	EndReport 		(void);
void 	LoadDefault 	(void);
void 	SrchExaf 		(char *);
int 	spec_valid 		(int);
int 	FindCusa 		(void);
int 	heading 		(int);

/*
 * Main Processing Routine.
 */
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
	LoadDefault ();

	/*
	 * Open main database files.
	 */
	OpenDB ();

	DateToDMY (comm_rec.dbt_date, NULL, &customerMonth, NULL);
	
	while (prog_exit == 0)	
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		heading (1);
		scn_display (1);
		entry (1);

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			break;

		Process ();
        return (EXIT_SUCCESS);
		prog_exit = 1;
	}
	rset_tty ();
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
Process (void)
{
	rset_tty ();

	dsp_screen ("Sales By Customer Report.", comm_rec.co_no, comm_rec.co_name);

	ReportHeading ();

	strcpy (cumr_rec.co_no,comm_rec.co_no);
	strcpy (cumr_rec.est_no,"  ");
	strcpy (cumr_rec.dbt_acronym,"         ");

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no))
	{
		/*
		 * check area.
		 */
		if (!strcmp (exaf_rec.area_code,"  ") || 
                     !strcmp (exaf_rec.area_code,cumr_rec.area_code)) 
		{
			dsp_process ("Customer : ", cumr_rec.dbt_acronym);

			ProcessCumr ();
		}
		cc = find_rec (cumr,&cumr_rec,NEXT,"r");
	}
	EndReport ();	
	shutdown_prog ();
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

void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (cusa, cusa_list, CUSA_NO_FIELDS, "cusa_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cusa);
	abc_fclose (exaf);
	abc_dbclose ("data");
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("area_code"))
	{
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (exaf_rec.area_code,"  "))
		{
			strcpy (exaf_rec.area, ML ("All Areas"));
			DSP_FLD ("area_desc");
			return (EXIT_SUCCESS);
		}

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			return (EXIT_FAILURE);
		}
		DSP_FLD ("area_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
ProcessCumr (void)
{
	int	i;

	currentTotal = 0;
	yearTotal = 0;
	for (i = 0 ; i < 12 ; i++)
		currentYear [i] = 0;
	
	/*
	 * Read Customer sales record with a type of 'C' for current Year.
	 */
	cusa_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cusa_rec.year, "C");
	FindCusa ();

	/*
	 * Read Customer sales record with a type of 'L' for current Year.
	 */
	cusa_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cusa_rec.year, "L");
	FindCusa ();

	fprintf (fout,".LRP3\n");
	fprintf (fout, "|  %-6.6s  | %-40.40s %100s|\n",
			cumr_rec.dbt_no,cumr_rec.dbt_name," ");

	fprintf (fout, "|%10.2f|", DOLLARS (currentYear [0]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [1]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [2]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [3]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [4]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [5]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [6]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [7]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [8]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [9]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [10]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentYear [11]));
	fprintf (fout, "%10.2f|",  DOLLARS (currentTotal));
	fprintf (fout, "%10.2f|\n",DOLLARS (yearTotal));

	grandTotal [0] += currentYear [0];
	grandTotal [1] += currentYear [1];
	grandTotal [2] += currentYear [2];
	grandTotal [3] += currentYear [3];
	grandTotal [4] += currentYear [4];
	grandTotal [5] += currentYear [5];
	grandTotal [6] += currentYear [6];
	grandTotal [7] += currentYear [7];
	grandTotal [8] += currentYear [8];
	grandTotal [9] += currentYear [9];
	grandTotal [10] += currentYear [10];
	grandTotal [11] += currentYear [11];
	grandTotal [12] += currentTotal;
	grandTotal [13] += yearTotal;
}

int
FindCusa (void)
{
	int 	i,
			c_month,
			cntr;

	cc = find_rec (cusa, &cusa_rec, COMPARISON,"r");
	if (cc)
		return (EXIT_SUCCESS);

	/*------------------------------
	| Process Current Year Values. |
	------------------------------*/
	if (cusa_rec.year [0] == 'C')
	{
		c_month = customerMonth;

		if (c_month < comm_rec.fiscal)
			c_month += 12;

		c_month -= comm_rec.fiscal;
		currentTotal = 0;
		i = comm_rec.fiscal;

		for (cntr = 0 ; cntr < c_month ; cntr++) 
		{
			currentTotal += cusa_val [i];
			i++;
			if (i > 11)
				i = 0;
		}
		i = customerMonth - 1;

		for (cntr = 11 ; cntr >= 0 ; cntr--) 
		{
			currentYear [cntr] = cusa_val [i];
			i--;
			if (i < 0)
				i = 11;
		}
	}
		
	/*
	 * Process Last Year Values.
	 */
	if (cusa_rec.year [0] == 'L')
	{
		c_month = customerMonth;

		if (c_month < comm_rec.fiscal)
			c_month += 12;

		c_month -= comm_rec.fiscal;
		yearTotal = 0;
		i = comm_rec.fiscal;

		for (cntr = 0 ; cntr < c_month ; cntr++) 
		{
			yearTotal += cusa_val [i];
			i++;
			if (i > 11)
				i = 0;
		}
	}
	return (EXIT_SUCCESS);
}

void
ReportHeading (void)
{
	int 	i;
	int     cntr;

	/*
	 * open output pipe.
	 */
	if ((fout = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (fout,".START%s<%s>\n",DateToString (comm_rec.dbt_date),PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNo);
	fprintf (fout,".PI12\n");
	fprintf (fout,".12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".ECUSTOMER SALES ANALYSIS REPORT\n");
	if (!strcmp (exaf_rec.area_code,"  "))
		fprintf (fout,".EAll Areas\n");
	else
		fprintf (fout,".EArea : %s - %s\n", exaf_rec.area_code,
					           clip (exaf_rec.area));
	fprintf (fout,".ECompany : %s - %s\n",      comm_rec.co_no,
				  		   clip (comm_rec.co_name));
	fprintf (fout,".EAS AT %s\n",SystemTime ());
	fprintf (fout,".EFor Month %02d\n",customerMonth);

	fprintf (fout,"====================================================");
	fprintf (fout,"====================================================");
	fprintf (fout,"===================================================\n");

	fprintf (fout,".R====================================================");
	fprintf (fout,"====================================================");
	fprintf (fout,"===================================================\n");

	fprintf (fout,"| CUSTOMER |            N   A   M   E               ");
	fprintf (fout,"                                                    ");
	fprintf (fout,"                                                  |\n");

	fprintf (fout,"|==========|==========|==========|==========|=======");
	fprintf (fout,"===|==========|==========|==========|==========|====");
	fprintf (fout,"======|==========|==========|==========|==========|\n");

	i = customerMonth;
	if (i > 11) 
		i = 0;

	for (cntr = 11 ; cntr >= 0 ; cntr--) 
	{
		fprintf (fout,"|   %3s    ",ML (monthName [i]));
		i++;
		if (i > 11)
			i = 0;
	}

	fprintf (fout,"| CURR YTD | LAST YTD |\n");

	fprintf (fout,"|----------|----------|----------|----------|-------");
	fprintf (fout,"---|----------|----------|----------|----------|----");
	fprintf (fout,"------|----------|----------|----------|----------|\n");
}
/*
 * Routine to print final totals for report.
 */
void
EndReport (void)
{
	fprintf (fout, "|----------|----------|----------|----------|-------");
	fprintf (fout, "---|----------|----------|----------|----------|----");
	fprintf (fout, "------|----------|----------|----------|----------|\n");

	fprintf (fout, "|%10.2f|", DOLLARS (grandTotal [0]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [1]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [2]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [3]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [4]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [5]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [6]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [7]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [8]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [9]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [10]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [11]));
	fprintf (fout, "%10.2f|",  DOLLARS (grandTotal [12]));
	fprintf (fout, "%10.2f|\n",DOLLARS (grandTotal [13]));
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
LoadDefault (void)
{
	strcpy (exaf_rec.area_code,"  ");
	strcpy (exaf_rec.area, ML ("All Areas"));
	local_rec.printerNo = 1;
}

void
SrchExaf (
	char	*key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Area Description");
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf,&exaf_rec,GTEQ,"r");
	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code,exaf_rec.area);
		if (cc)
			break;

		cc = find_rec (exaf,&exaf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}


int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlDbMess114),20,0,1);

		box (0,3,80,4);
		line_at (1,0,80);
		line_at (6,1,79);
		line_at (20,0,80);
		line_at (22,0,80);

		sprintf (err_str,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (21,0,err_str);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
