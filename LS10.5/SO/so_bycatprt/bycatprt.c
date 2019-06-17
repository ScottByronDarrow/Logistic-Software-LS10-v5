/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: bycatprt.c,v 5.4 2002/07/17 09:58:03 scott Exp $
|  Program Name  : (so_bycatprt.c)
|  Program Desc  : (Print Sales Orders by Category) 
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 17/10/88         |
|---------------------------------------------------------------------|
| $Log: bycatprt.c,v $
| Revision 5.4  2002/07/17 09:58:03  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/10/23 07:16:36  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.2  2001/08/09 09:20:52  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:50:58  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/23 02:50:10  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to add sleep delay - did not work with LS10-GUI
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bycatprt.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_bycatprt/bycatprt.c,v 5.4 2002/07/17 09:58:03 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define	FORWARD_ORDERS	 (status [0] == 'F')
#define	BACKORDERS		 (status [0] == 'B')
#define	MANUAL_ORDER	 (status [0] == 'M')
#define	HELD_ORDERS		 (status [0] == 'H' || status [0] == 'O')
#define	ALL_ORDERS		 (status [0] == ' ')

#define	FWD_OK			 (soln_rec.status [0] == 'F')
#define	BACK_OK			 (soln_rec.status [0] == 'B')
#define	MAN_OK			 (soln_rec.status [0] == 'M')
#define	HELD_OK			 (soln_rec.status [0] == 'H' || \
					      soln_rec.status [0] == 'O'|| \
						  soln_rec.status [0] == 'C')

#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;
struct inexRecord	inex_rec;

	char	*data = "data";

	extern	int	TruePosition;
	extern	int	EnvScreenOK;


	int		printerNumber	= 1,
			firstFlag 		= TRUE,
			afterTotal  	= FALSE,
			categoryDetails = 0;

	int		envVarRepTax 		= FALSE,
			envVarDbNettUsed 	= TRUE;

	char	status [2],
			lower [13],
			upper [13],
			startClass [2],
			endClass [2],
			startCat [12],
			endCat [12],
			previousCat [12];

	float	qty          = 0.00,
			tot_so_qty   = 0.00,
			cat_tot_qty  = 0.00,
			grand_qty    = 0.00;

	float	soln_cnv_fct = 0.00,
            std_cnv_fct  = 0.00;

	double	extend,
			tot_so_shipmt   = 0.00,
			cat_tot_shipmt  = 0.00,
			grand_shipmt    = 0.00;

	int 	rep_tax = 0;
	int	    new_item;
	int 	envVarDbMcurr;

	FILE	*fout;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	back [2];
	char	backDesc [21];
	char	onite [2];
	char	oniteDesc [21];
	char	startCat [12];
	char	startClass [2];
	char	startDesc [41];
	char	endCat [12];
	char	endClass [2];
	char	endDesc [41];
	char	startGroup [13];
	char	endGroup [13];
	int		printerNumber;
	char	printerString [3];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_class",	 5, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class      ", "Input Start Class A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.startClass},
	{1, LIN, "st_cat",	 6, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Start Category   ", "Input Start Inventory Category.",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCat},
	{1, LIN, "desc",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description      ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startDesc},
	{1, LIN, "en_class",	 9, 2, CHARTYPE,
		"U", "          ",
		" ", "Z", "End Class        ", "Input End Class A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.endClass},
	{1, LIN, "en_cat",	10, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "End Category     ", "Input End Inventory Category.",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCat},
	{1, LIN, "desc",	11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description      ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endDesc},
	{1, LIN, "printerNumber",	13, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number   ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",	14, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background       ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	14, 20, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTRIGHT, "", "", local_rec.backDesc},
	{1, LIN, "onite",	15, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight        ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc",	15, 20, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTRIGHT, "", "", local_rec.oniteDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*======================= 
| Function Declarations |
=======================*/
int  	heading 		(int);
int  	spec_valid 		(int);
int  	ValidateOrder 	(void);
void 	RunProgram 		(char *);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	SrchExcf 		(char *);
void 	HeadingOutput 	(void);
void 	PrintLine 		(void);
void 	ProcessFile 	(void);
void 	ProcessSoln 	(long);
void 	PrintSoln 		(void);
void 	PrintCategory 	(void);
void 	PrintTotalCat 	(void);
void 	PrintTotalPart 	(void);
void 	PrintInex 		(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	orderDesc [30];
	char	*sptr;

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	sptr	= chk_env ("DB_MCURR");
	if (sptr)
		envVarDbMcurr = atoi (sptr);
	else
		envVarDbMcurr = FALSE;

	TruePosition	=	TRUE;
	
	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (argc != 2 && argc != 5)
	{
		print_at (0,0,mlSoMess702,argv [0]);
		print_at (0,0,mlSoMess703,argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sprintf (status,"%-1.1s", argv [1]);

	/*===========================
	| Open main database files. |  
	===========================*/
	OpenDB ();


	if (argc == 5)
	{
		printerNumber = atoi (argv [2]);
		sprintf (lower,"%-12.12s",argv [3]);
		sprintf (upper,"%-12.12s",argv [4]);

		sprintf (startClass, "%-1.1s", lower);
		sprintf (endClass, "%-1.1s", upper);
		sprintf (startCat,"%-11.11s",lower + 1);
		sprintf (endCat,"%-11.11s",upper + 1);

		if (FORWARD_ORDERS)
			strcpy (orderDesc,	ML ("Forward Orders"));

		if (BACKORDERS)
			strcpy (orderDesc,	ML ("Backorders"));

		if (MANUAL_ORDER)
			strcpy (orderDesc,	ML ("New/Unconsolidated Orders"));

		if (HELD_ORDERS)
			strcpy (orderDesc,	ML ("Held Orders"));

		if (ALL_ORDERS)
			strcpy (orderDesc,	ML ("ALL Orders"));

		sprintf (err_str,"Processing %s By Category", orderDesc);

		dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);
		HeadingOutput ();
		ProcessFile ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		init_vars (1);
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		strcpy (err_str, ML (mlSoMess394));
		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
RunProgram (
	 char *programName)
{	
	rset_tty ();

	if (!strncmp (local_rec.endCat, "~~~~~~~~~~~", 11))
		memset ((char *)local_rec.endCat,0xff,sizeof (local_rec.endCat));
	
	sprintf (local_rec.startGroup,"%1.1s%-11.11s",local_rec.startClass,local_rec.startCat);
	sprintf (local_rec.endGroup,"%1.1s%-11.11s", local_rec.endClass, local_rec.endCat);
	
	shutdown_prog ();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onite [0] == 'Y')
	{ 
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				programName,
				status,
				local_rec.printerString,
				local_rec.startGroup,
				local_rec.endGroup,
				err_str, (char *)0);
	}
	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () != 0)
			return;
		else
			execlp (programName,
				programName,
				status,
				local_rec.printerString,
				local_rec.startGroup,
				local_rec.endGroup, (char *)0);
	}
	else 
	{
		execlp (programName,
			programName,
			status,
			local_rec.printerString,
			local_rec.startGroup,
			local_rec.endGroup, (char *)0);
	}
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
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (excf);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("st_cat"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.startCat);

		if (!dflt_used)
		{
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc) 
			{
		/*		sprintf (err_str, "Category %s in not on file.",local_rec.startCat);*/
				errmess (ML (mlStdMess004));
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.startCat,"%-11.11s","           ");
			sprintf (excf_rec.cat_desc,"%-40.40s","BEGINNING OF RANGE");
		}
		if (prog_status != ENTRY && strcmp (local_rec.startCat,local_rec.endCat) > 0)
		{
		/*	sprintf (err_str, "NOTE Range<%s is now GREATER than %s>",local_rec.startCat,local_rec.endCat);*/
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_SUCCESS); 
		}
		sprintf (local_rec.startDesc,"%-40.40s",excf_rec.cat_desc);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("en_cat"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.endCat);
		
		if (!dflt_used)
		{
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc) 
			{
				/*sprintf (err_str, "Category %s in not on file.",local_rec.endCat);*/
				errmess (ML (mlStdMess004));
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.endCat,"%-11.11s","~~~~~~~~~~~");
			sprintf (excf_rec.cat_desc,"%-40.40s","END OF RANGE");
		}
		if (strcmp (local_rec.startCat,local_rec.endCat) > 0)
		{
		/*	sprintf (err_str, "Invalid Range < %s is GREATER than %s>",local_rec.startCat,local_rec.endCat);*/
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.endDesc,excf_rec.cat_desc);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.printerString,"%d",local_rec.printerNumber);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc,
				 (local_rec.back [0] == 'Y') ? ML ("YES") : ML ("NO"));
		DSP_FLD ("backDesc"); 
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.oniteDesc,
				 (local_rec.onite [0] == 'Y') ? ML ("YES") : ML ("NO"));
		DSP_FLD ("oniteDesc") ;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char *key_val)
{
	_work_open (11,0,40);
	save_rec ("#Category No", "#Category Description");

	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no ,"%-11.11s",key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in excf During (DBFIND)",cc, PNAME);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (DBPOPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",printerNumber);

	fprintf (fout,".13\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".B1\n");

	if (FORWARD_ORDERS)
		fprintf (fout,".EFORWARD ORDERS BY CATEGORY\n");

	if (BACKORDERS)
		fprintf (fout,".EBACKORDER BY CATEGORY\n");

	if (MANUAL_ORDER)
		fprintf (fout,".ENEW/UNCONSOLIDATED ORDERS BY CATEGORY\n");

	if (HELD_ORDERS)
		fprintf (fout,".EHELD ORDERS BY CATEGORY\n");

	if (ALL_ORDERS)
		fprintf (fout,".EALL ORDERS BY CATEGORY\n");

	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %s\n",SystemTime ());

	fprintf (fout,".R============================");
	fprintf (fout,"============================================");
	fprintf (fout,"====================================================");
	fprintf (fout,"=====================");
	fprintf (fout,"=======");
	fprintf (fout,"===\n");

	fprintf (fout,"==================================");
	fprintf (fout,"======================================");
	fprintf (fout,"====================================================");
	fprintf (fout,"=====================");
	fprintf (fout,"=======");
	fprintf (fout,"===\n");

	fprintf (fout,"|       ITEM       ");
	fprintf (fout,"|               DESCRIPTION                ");
	fprintf (fout,"|   DATE   ");
	fprintf (fout,"|  UOM ");
	fprintf (fout,"|  QUANTITY  ");
	fprintf (fout,"| OUTSTDNG LCL");
	fprintf (fout,"|  CUSTOMER  ");
	fprintf (fout,"|   S.O.  ");
	fprintf (fout,"|  CUSTOMER ORDER    |");
	fprintf (fout,"ST|\n");

	fprintf (fout,"|      NUMBER      ");
	fprintf (fout,"|                                          ");
	fprintf (fout,"|   DUE    ");
	fprintf (fout,"|      ");
	fprintf (fout,"|  OUTSTDNG  ");
	fprintf (fout,"| SALES VALUE ");
	fprintf (fout,"|  ACRONYM   ");
	fprintf (fout,"| NUMBER  ");
	fprintf (fout,"|      NUMBER        |");
	fprintf (fout,"  |\n");

	PrintLine ();
	firstFlag = TRUE;
	fflush (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|------");
	fprintf (fout,"|------------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|---------");
	fprintf (fout,"|--------------------|");
	fprintf (fout,"--|\n");

	fflush (fout);
}

/*====================================
| Process File by Co/Class/Category. |
====================================*/
void
ProcessFile (
 void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.inmr_class,startClass);
	sprintf (inmr_rec.category,"%-11.11s",startCat);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	strcpy (previousCat,"           ");

	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && 
	               strcmp (inmr_rec.inmr_class,startClass) >= 0 && 
	               strcmp (inmr_rec.inmr_class,endClass) <= 0)
	{
		if (strcmp (inmr_rec.category,startCat) >= 0 && 
	             strcmp (inmr_rec.category,endCat) <= 0)
		{
			new_item = TRUE;
			ProcessSoln (inmr_rec.hhbr_hash);
		}

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

	if (categoryDetails > 1)
		PrintTotalCat ();

	/*-----------------------------------------------
	| On last record,print the total sales order.
	-----------------------------------------------*/
	PrintLine ();
	fprintf (fout,"| ***** TOTAL SALES ORDERS *****   ");
	fprintf (fout,"                           ");
	fprintf (fout,"|          ");
	fprintf (fout,"|      ");
	fprintf (fout,"| %10.2f ",grand_qty);
	fprintf (fout,"|%12.2f ",DOLLARS (grand_shipmt));
	fprintf (fout,"|            ");
	fprintf (fout,"|         ");
	fprintf (fout,"|                    |");
	fprintf (fout,"  |\n");
	fflush (fout);
}

/*=======================
| Process Valid items.	|
=======================*/
void
ProcessSoln (
 long hhbr_hash)
{
	tot_so_qty = 0.0;
	tot_so_shipmt = 0.0;

	cc = find_hash ("soln",&soln_rec,GTEQ,"r",inmr_rec.hhbr_hash);

	while (!cc && soln_rec.hhbr_hash == hhbr_hash) 
	{
		if (!ValidateOrder ())
		{
			cc = find_hash ("soln",&soln_rec,NEXT,"r",inmr_rec.hhbr_hash);
			continue;
		}

		if ((soln_rec.qty_order + soln_rec.qty_bord) > 0 || 
	             inmr_rec.inmr_class [0] == 'Z')
		{
			cc = find_hash ("sohr",&sohr_rec,COMPARISON,"r",soln_rec.hhso_hash);
			if (cc)
				return;

			dsp_process ("Category : ",inmr_rec.category);
			if (strcmp (previousCat,inmr_rec.category) != 0)
			{
				if (!firstFlag && categoryDetails > 1)
					PrintTotalCat ();
				else
					if (!firstFlag && categoryDetails == 1)
						PrintLine ();

				categoryDetails = 0;
				cat_tot_qty = 0.0;
				cat_tot_shipmt = 0.0;
				PrintCategory ();
				strcpy (previousCat,inmr_rec.category);
			}
			PrintSoln ();
		}	
		cc = find_hash ("soln",&soln_rec,NEXT,"r",inmr_rec.hhbr_hash);
	}
}

/*=======================
| Validate order types. |
=======================*/
int
ValidateOrder (
 void)
{
	if (ALL_ORDERS)
		return (TRUE);

	if (FORWARD_ORDERS && FWD_OK)
		return (TRUE);

	if (BACKORDERS && BACK_OK)
		return (TRUE);

	if (MANUAL_ORDER && MAN_OK)
		return (TRUE);

	if (HELD_ORDERS && HELD_OK)
		return (TRUE);

	return (FALSE);
}
/*===================
| Print Line items.	|
===================*/
void
PrintSoln (
 void)
{
	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00,
			lcl_ex_rate	= 0.00;

	cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",sohr_rec.hhcu_hash);
	if (cc)
		file_err (cc, "cumr", "DBFIND");

	lcl_ex_rate = (envVarDbMcurr && sohr_rec.exch_rate != 0.00) ? sohr_rec.exch_rate : 1.00;

	qty = soln_rec.qty_order + soln_rec.qty_bord;

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	(double) qty;
		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);

		if (envVarRepTax)
		{
			l_tax	=	(double) soln_rec.tax_pc;
			if (sohr_rec.tax_code [0] == 'D')
				l_tax *= l_total;
			else
			{
				if (envVarDbNettUsed)
					l_tax	*=	(l_total + soln_rec.item_levy + l_disc);
				else
					l_tax	*=	(l_total + soln_rec.item_levy);
			}
			l_tax	=	DOLLARS (l_tax);
		}
		l_tax	=	no_dec (l_tax);

		l_gst	=	(double) soln_rec.gst_pc;
		if (envVarDbNettUsed)
			l_gst	*=	(l_total - l_disc) + l_tax + soln_rec.item_levy;
		else
			l_gst	*=	(l_total + l_tax + soln_rec.item_levy);

		l_gst	=	DOLLARS (l_gst);
			
		if (envVarDbNettUsed)
			extend	=	l_total - l_disc + l_tax + l_gst + soln_rec.item_levy;
		else
			extend	=	l_total + l_tax + l_gst + soln_rec.item_levy;

		extend	/=	lcl_ex_rate;
	}
	tot_so_qty += qty;
	tot_so_shipmt += extend;

	cat_tot_qty += qty;
	cat_tot_shipmt	+= extend;

	grand_qty += qty;
	grand_shipmt += extend;

	fprintf (fout,"| %-16.16s ",inmr_rec.item_no);
	fprintf (fout,"| %-40.40s ",soln_rec.item_desc);

	if (sohr_rec.dt_required == 0L)
		fprintf (fout,"|          ");
	else
		fprintf (fout,"|%-10.10s",DateToString (sohr_rec.dt_required));

	fprintf (fout,"| %-4.4s ",inmr_rec.sale_unit);
	fprintf (fout,"| %10.2f ",qty);
	fprintf (fout,"| %11.2f ",DOLLARS (extend));
	fprintf (fout,"|  %-9.9s ",cumr_rec.dbt_acronym);
	fprintf (fout,"|%-8.8s ",sohr_rec.order_no);
	fprintf (fout,"|%-20.20s|",sohr_rec.cus_ord_ref);
	fprintf (fout," %1.1s|",soln_rec.status);
	fprintf (fout,"\n");
	if (new_item)
	{
		new_item = FALSE;
		PrintInex ();
	}

	fflush (fout);

	categoryDetails++;
}

/*===========================
| Print category headings.	|
===========================*/
void
PrintCategory (
 void)
{
	dsp_process ("Category No. : ",inmr_rec.category);

	fprintf (fout,".LRP2\n");
	if (afterTotal)
	{
		PrintLine ();
		afterTotal = FALSE;
	}
	strcpy (excf_rec.co_no,comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		strcpy (excf_rec.cat_no, "No category description found.");

	fprintf (fout,"| * %-11.11s *  ",inmr_rec.category);
	fprintf (fout,"| %-40.40s ",excf_rec.cat_desc);
	fprintf (fout,"|          |      |            |             ");
	fprintf (fout,"|            |         |                    |");
	fprintf (fout,"  |\n");
	firstFlag = FALSE;
}

/*===========================
| Print category totals.	|
===========================*/
void
PrintTotalCat (
 void)
{
	fprintf (fout,".LRP2\n");
	fprintf (fout,"|  TOTAL FOR CATEGORY : %-11.11s ",previousCat);
	fprintf (fout,"                          |          |      ");
	fprintf (fout,"| %10.2f ",cat_tot_qty);
	fprintf (fout,"| %11.2f ",DOLLARS (cat_tot_shipmt));
	fprintf (fout,"|            ");
	fprintf (fout,"|         ");
	fprintf (fout,"|                    |");
	fprintf (fout,"  |\n");
	afterTotal = TRUE;
}

/*--------------------------------------------------------------------
| If more than one line for a part no,print total for detail lines. |
--------------------------------------------------------------------*/
void
PrintTotalPart (
 void)
{
	fprintf (fout,".LRP2\n");
	fprintf (fout,"|  TOTAL FOR ITEM    :  %-16.16s ",inmr_rec.item_no);
	fprintf (fout,"                                   |          ");
	fprintf (fout,"| %10.2f ",tot_so_qty);
	fprintf (fout,"| %11.2f ",DOLLARS (tot_so_shipmt));
	fprintf (fout,"|            ");
	fprintf (fout,"|         ");
	fprintf (fout,"|                    |");
	fprintf (fout,"  |\n");
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		if (FORWARD_ORDERS)
			rv_pr (ML (mlSoMess005), (80 - strlen (ML (mlSoMess005))) / 2,0,1);

		if (BACKORDERS)
			rv_pr (ML (mlSoMess006), (80 - strlen (ML (mlSoMess006))) / 2,0,1);

		if (MANUAL_ORDER)
			rv_pr (ML (mlSoMess007), (80 - strlen (ML (mlSoMess007))) / 2,0,1);

		if (HELD_ORDERS)
			rv_pr (ML (mlSoMess008), (80 - strlen (ML (mlSoMess008))) / 2,0,1);

		if (ALL_ORDERS)
			rv_pr (ML (mlSoMess009), (80 - strlen (ML (mlSoMess009))) / 2,0,1);

		box (0,4,80,11);
		line_at (1,0,80);
		line_at (8,1,79);
		line_at (12,1,79);
		line_at (19,0,80);
		print_at (20, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (21, 0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
PrintInex (
 void)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		fprintf (fout,"| %-16.16s "," ");
		fprintf (fout,"| %-40.40s ",inex_rec.desc);
		fprintf (fout,"|          ");
		fprintf (fout,"|      ");
		fprintf (fout,"| %10.2s "," ");
		fprintf (fout,"| %11.2s "," ");
		fprintf (fout,"|  %-9.9s "," ");
		fprintf (fout,"| %-6.6s  "," ");
		fprintf (fout,"|%-20.20s|"," ");
		fprintf (fout," %1.1s|"," ");
		fprintf (fout,"\n");
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}

