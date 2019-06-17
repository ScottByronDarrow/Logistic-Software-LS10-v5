/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_adel_inp.c,v 5.2 2002/02/26 03:51:53 scott Exp $
|  Program Name  : (sk_adel_inp.c) 
|  Program Des   : (Inventory Master Alternative Delete Input)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 04/02/96         |
|---------------------------------------------------------------------|
| $Log: sk_adel_inp.c,v $
| Revision 5.2  2002/02/26 03:51:53  scott
| S/C 00717 SKMT15-SUB6 TO 9-Stock Delete Functions; problems(1) Start Class and End Class fields accepts invalid inputs (2) Start Class and End Class accepts invalid range
|
| Revision 5.1  2002/02/26 03:41:19  scott
| First pass
|
| Revision 5.4  2002/01/29 03:04:36  robert
| SC-00718: Updated to add sleep after warning message
|
| Revision 5.3  2001/08/09 09:17:57  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:35  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:44  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_adel_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_adel_inp/sk_adel_inp.c,v 5.2 2002/02/26 03:51:53 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define		DEL_OK		0
#define		DEL_ERR		1

#define	LIVE_GRIN	 ((pogl_rec.pur_status [0] == 'A' &&  \
                   	   pogl_rec.gl_status [0] == 'D') ||  \
		   	   		   pogl_rec.pur_status [0] == 'D')

	/*
	 * Special fields and flags.
	 */
	int		pid,
			workFileNo;	

	char	deleteType [2];

#include	"schema"

struct commRecord	comm_rec;
struct ineiRecord	inei_rec;
struct inccRecord	incc_rec;
struct ccmrRecord	ccmr_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct cohrRecord	cohr_rec;
struct poglRecord	pogl_rec;
struct polnRecord	poln_rec;
struct itlnRecord	itln_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;
struct inasRecord	inas_rec;

	/*===================
	| Work file record. |	
	===================*/
	struct {
		char	type [2];
		long	hhbrHash;
		long	hhccHash;
	} workRec;

/*=============================
| Local & Screen Structures . |
=============================*/
struct {
	char	dummy 			[11];
	char	desc 			[41];
	char	startClass 		[2],
			endClass 		[2],
			startCat 		[12],
			startCatDesc 	[41],
			endCat 			[12],
			endCatDesc 		[41];
	char	startItem		[17],
			endItem 		[17],
			startItemDesc 	[41],
			endItemDesc 	[41];
	char	startActiveCode [2],
			endActiveCode	[2],
			startActiveDesc [41],
			endActiveDesc 	[41];
} local_rec;

extern	int		TruePosition;

static	struct	var	vars [] =
{
	/*-------------------
	| By Class-Category	|
	-------------------*/
	{1, LIN, "startClass",	 3, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class     ", "Input Start Class A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.startClass},
	{1, LIN, "startCat",	 4, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Start Category  ", "Input Start Inventory Category. [SEARCH] Available.",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCat},
	{1, LIN, "startCatDesc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startCatDesc},
	{1, LIN, "endClass",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "Z","End Class       ", "Input End Class A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.endClass},
	{1, LIN, "endCat",	7, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "End Category    ", "Input End Inventory Category. [SEARCH] Available.",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCat},
	{1, LIN, "endCatDesc",	8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endCatDesc},

	/*-------------------
	| By Product        |
	-------------------*/
	{1, LIN, "startItem",	 10, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start Item      ", "Input Start Item Number. [SEARCH] Available. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "startItemDesc",	 11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startItemDesc},
	{1, LIN, "endItem",	 12, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "End  Item       ", "Input End Item Number. [SEARCH] Available. ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "endItemDesc",	 13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endItemDesc},
	{1, LIN, "startActiveCode",	 15, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Active Status   ", "Enter Active Status. [SEARCH] Available",
		NO, NO,  JUSTLEFT, "", "", local_rec.startActiveCode},
	{1, LIN, "startActiveDesc",	 16, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description     ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.startActiveDesc},
	{1, LIN, "endActiveCode",	 17, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Active Status   ", "Enter Active Status. [SEARCH] Available",
		NO, NO,  JUSTLEFT, "", "", local_rec.endActiveCode},
	{1, LIN, "endActiveDesc",	 18, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description     ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.endActiveDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	ReadMisc 				(void);
int  	spec_valid 				(int);
void 	ProcDelete 				(void);
void 	SrchExcf 				(char *);
void 	SrchInas 				(char *);
static 	int ValidDelete 		(void);
int  	CheckIncc 				(void);
int  	CheckOrders 			(void);
int  	CheckPurchaseOrders 	(void);
int  	CheckGoodsReceipts 		(void);
int  	CheckTransactions 		(void);
int  	heading 				(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	TruePosition	=	TRUE;
	
	SETUP_SCR (vars);

	if (argc < 3) 	
	{
		print_at (0,0,mlSkMess009, argv [0]);
		return (EXIT_FAILURE);
	}
	pid = atoi (argv [1]);
	sprintf (deleteType, "%-1.1s", argv [2]);

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
   		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		heading (1);
		entry (1);
			
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		ProcDelete ();
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

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/sk_del%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	if ((cc = RF_OPEN (filename,sizeof (workRec),"w",&workFileNo)) != 0) 
		file_err (cc, "work_file", "WKOPEN");

	abc_dbopen ("data");

	ReadMisc ();

	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhbr_hash");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inas, inas_list, INAS_NO_FIELDS, "inas_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_hhbr_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_hhbr_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (excf);
	abc_fclose (inas);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (itln);
	abc_fclose (pogl);
	abc_fclose (poln);
	abc_fclose (soln);

	SearchFindClose ();
	abc_dbclose ("data");

	cc = RF_CLOSE (workFileNo);
	if (cc) 
		file_err (cc, "work_file", "WKCLOSE");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("startClass"))
	{
		if (prog_status != ENTRY && 
				strcmp (local_rec.startClass,local_rec.endClass) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endClass"))
	{
		if (strcmp (local_rec.startClass,local_rec.endClass) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("startCat"))
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
			cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
			if (cc) 
			{
				print_mess (ML (mlStdMess004));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.startCat,"%-11.11s","           ");
			sprintf (excf_rec.cat_desc,"%-40.40s",ML ("START OF RANGE"));
		}

		if (prog_status != ENTRY && strcmp (local_rec.startCat,local_rec.endCat) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.startCatDesc,"%-40.40s",excf_rec.cat_desc);
		DSP_FLD ("startCatDesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("endCat"))
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
			cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
			if (cc) 
			{
				print_mess (ML (mlStdMess004));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.endCat,"%-11.11s","~~~~~~~~~~~");
			sprintf (excf_rec.cat_desc,"%-40.40s",ML ("END OF RANGE"));
		}
		if (strcmp (local_rec.startCat,local_rec.endCat) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.endCatDesc,excf_rec.cat_desc);
		DSP_FLD ("endCatDesc");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Item Number . |
	------------------------*/
	if (LCHECK ("startItem"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (!dflt_used)
		{
			cc = FindInmr (comm_rec.co_no, local_rec.startItem, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.startItem);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				errmess (ML (mlStdMess001));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			SuperSynonymError ();
			strcpy (local_rec.startItem, inmr_rec.item_no);
		}
		else
		{
			sprintf (local_rec.startItem,"%-16.16s","                ");
			sprintf (inmr_rec.description,"%-40.40s","START OF RANGE");
			sprintf (local_rec.startItemDesc,  "%-40.40s","START OF RANGE");
		}

		DSP_FLD ("startItem");
		DSP_FLD ("startItemDesc");
		
		if (prog_status != ENTRY && strcmp (local_rec.startItem,local_rec.endItem) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.startItemDesc,inmr_rec.description);
		DSP_FLD ("startItemDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endItem"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (!dflt_used)
		{
			cc = FindInmr (comm_rec.co_no, local_rec.endItem, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.endItem);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				errmess (ML (mlStdMess001));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	
			SuperSynonymError ();

			strcpy (local_rec.endItem, inmr_rec.item_no);
		}
		else
		{
			sprintf (local_rec.endItem,"%-16.16s","~~~~~~~~~~~~~~~~");
			sprintf (inmr_rec.description,"%-40.40s",ML ("END OF RANGE"));
			sprintf (local_rec.endItemDesc,"%-40.40s",ML ("END OF RANGE"));
		}
	
		DSP_FLD ("endItem");

		if (strcmp (local_rec.startItem,local_rec.endItem) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.endItemDesc,inmr_rec.description);
		DSP_FLD ("endItemDesc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate active status. |
	-------------------------*/
	if (LCHECK ("startActiveCode"))
	{
		if (SRCH_KEY)
		{
			SrchInas (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!dflt_used)
		{
			strcpy (inas_rec.co_no, comm_rec.co_no);
			sprintf (inas_rec.act_code, "%-1.1s", local_rec.startActiveCode);
			cc = find_rec (inas, &inas_rec, COMPARISON, "r");
			if (cc)
			{
				errmess (ML (mlSkMess312));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.startActiveCode, inas_rec.act_code);
		}
		else
		{
			sprintf (local_rec.startActiveCode,"%-1.1s"," ");
			sprintf (inas_rec.description,"%-40.40s",ML ("START OF RANGE"));
			sprintf (local_rec.startActiveDesc,"%-40.40s",ML ("START OF RANGE"));
		}
		DSP_FLD ("startActiveCode");

		if (prog_status != ENTRY && strcmp (local_rec.startActiveCode,local_rec.endActiveCode) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.startActiveDesc,inas_rec.description);
		DSP_FLD ("startActiveDesc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate active status. |
	-------------------------*/
	if (LCHECK ("endActiveCode"))
	{
		if (SRCH_KEY)
		{
			SrchInas (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!dflt_used)
		{
			strcpy (inas_rec.co_no, comm_rec.co_no);
			sprintf (inas_rec.act_code, "%-1.1s", local_rec.endActiveCode);
			cc = find_rec (inas, &inas_rec, COMPARISON, "r");
			if (cc)
			{
				errmess (ML (mlSkMess312));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.endActiveCode, inas_rec.act_code);
		}
		else
		{
			sprintf (local_rec.endActiveCode,"%-1.1s","~");
			sprintf (inas_rec.description,"%-40.40s",ML ("END OF RANGE"));
			sprintf (local_rec.endActiveDesc,"%-40.40s",ML ("END OF RANGE"));
		}
		if (strcmp (local_rec.startActiveCode,local_rec.endActiveCode) > 0)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.endActiveDesc,inas_rec.description);
		DSP_FLD ("endActiveDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
ProcDelete (
 void)
{
	abc_selfield (inmr, "inmr_id_no_3");

	dsp_screen ("Writing Items to be deleted to Workfile",comm_rec.co_no,comm_rec.co_name);

	if (strcmp (local_rec.endCat, "~~~~~~~~~~~"))
		memset ((char *)local_rec.endCat,0xff,sizeof (local_rec.endCat));

	strcpy (inmr_rec.co_no,			comm_rec.co_no);
	strcpy (inmr_rec.inmr_class, 	local_rec.startClass);
	strcpy (inmr_rec.category,		local_rec.startCat);
	strcpy (inmr_rec.item_no,		local_rec.startItem);
	cc = find_rec (inmr ,&inmr_rec,GTEQ,"u");

	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && 
		       strcmp (inmr_rec.inmr_class,local_rec.startClass) >= 0 && 
		       strcmp (inmr_rec.inmr_class,local_rec.endClass) <= 0 &&
		       strcmp (inmr_rec.category,local_rec.startCat) >= 0 && 
		       strcmp (inmr_rec.category,local_rec.endCat) <= 0)
	{
		if (	strcmp (inmr_rec.item_no,local_rec.startItem) >= 0 && 
		    	strcmp (inmr_rec.item_no,local_rec.endItem) <= 0 &&
				strcmp (inmr_rec.active_status,local_rec.startActiveCode) >= 0 && 
			 	strcmp (inmr_rec.active_status,local_rec.endActiveCode) <= 0)
		{
			if (!ValidDelete ())
			{
				dsp_process ("Processing : ",inmr_rec.item_no);

				strcpy (inmr_rec.stat_flag,"9");
				cc = abc_update ("inmr",&inmr_rec);
				if (cc) 
					file_err (cc, inmr, "DBUPDATE");
			
				strcpy (workRec.type,deleteType);
				workRec.hhbrHash = inmr_rec.hhbr_hash;
				workRec.hhccHash = ccmr_rec.hhcc_hash;
				cc = RF_ADD (workFileNo, (char *) &workRec);
				if (cc) 
					file_err (cc, "work_add", "WKADD");
			}
		}

		abc_unlock (inmr);
		cc = find_rec (inmr ,&inmr_rec,NEXT,"u");
	}
	abc_unlock (inmr);

	abc_selfield (inmr, "inmr_id_no");
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char *key_val)
{
	work_open ();
	save_rec ("#Cat Code","#Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no ,"%-11.11s",key_val);
	cc = find_rec (excf ,&excf_rec,GTEQ,"r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && 
			!strcmp (excf_rec.co_no,comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no,excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf ,&excf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc,  excf , "DBFIND");
}

void
SrchInas (
 char *key_val)
{
	work_open ();
	save_rec ("#Cd", "#Active Status Description");
	strcpy (inas_rec.co_no, comm_rec.co_no);
	sprintf (inas_rec.act_code ,"%-1.1s",key_val);
	cc = find_rec (inas, &inas_rec, GTEQ, "r");
	while (!cc && !strcmp (inas_rec.co_no, comm_rec.co_no) &&
				  !strncmp (inas_rec.act_code,key_val,strlen (key_val)))
	{
		cc = save_rec (inas_rec.act_code, inas_rec.description);
		if (cc)
			break;

		cc = find_rec (inas, &inas_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inas_rec.co_no, comm_rec.co_no);
	sprintf (inas_rec.act_code ,"%-1.1s",temp_str);
	cc = find_rec (inas, &inas_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inas, "DBFIND");
}

static	int
ValidDelete (
 void)
{
	switch (deleteType [0])
	{
	case 'M' :
		/*----------------------
		| Check stock on hand. |
		----------------------*/
		if (inmr_rec.on_hand != 0)
			return (DEL_ERR);

		cc = CheckOrders ();
		if (cc)
			return (DEL_ERR);
		
		cc = CheckPurchaseOrders ();
		if (cc)
			return (DEL_ERR);

		cc = CheckGoodsReceipts ();
		if (cc)
			return (DEL_ERR);

		cc = CheckTransactions ();
		if (cc)
			return (DEL_ERR);

		/*------------------------------
		| Check for warehouse records. |
		------------------------------*/
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (!cc)
			return (DEL_ERR);

		/*---------------------------
		| Check for branch records. |
		---------------------------*/
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (!cc)
			return (DEL_ERR);
		break;

	case 'B' :
		/*------------------------------
		| Check for warehouse records. |
		------------------------------*/
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (!cc)
			return (DEL_ERR);
		break;

	case 'W' :
		/*--------------------------
		| Check Warehouse Records. |
		--------------------------*/
		abc_selfield (incc,"incc_id_no");
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;

		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (!cc)
			return (DEL_ERR);
		break;

	case 'A' :
		/*----------------------
		| Check stock on hand. |
		----------------------*/
		if (inmr_rec.on_hand != 0)
			return (DEL_ERR);

		/*--------------------------
		| Check Warehouse Records. |
		--------------------------*/
		abc_selfield (incc,"incc_hhbr_hash");
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, GTEQ, "r");
		while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			cc = CheckIncc ();
			if (cc)
				return (DEL_ERR);

			cc = find_rec (incc, &incc_rec, NEXT, "r");
		}
		cc = CheckOrders ();
		if (cc)
			return (DEL_ERR);
		
		cc = CheckPurchaseOrders ();
		if (cc)
			return (DEL_ERR);

		cc = CheckGoodsReceipts ();
		if (cc)
			return (DEL_ERR);

		cc = CheckTransactions ();
		if (cc)
			return (DEL_ERR);
		break;
	}
	return (EXIT_SUCCESS);
}

int
CheckIncc (
 void)
{
	if (incc_rec.opening_stock != 0.00)
		return (DEL_ERR);
	
	if (incc_rec.receipts != 0.00)
		return (DEL_ERR);

	if (incc_rec.pur != 0.00)
		return (DEL_ERR);
	
	if (incc_rec.issues != 0.00)
		return (DEL_ERR);
	
	if (incc_rec.adj != 0.00)
		return (DEL_ERR);

	if (incc_rec.sales != 0.00)
		return (DEL_ERR);
	
	if (incc_rec.closing_stock != 0.00)
		return (DEL_ERR);
	
	return (EXIT_SUCCESS);
}

/*=========================
| Check for valid Orders. |
=========================*/
int
CheckOrders (
 void)
{
	soln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (soln, &soln_rec, EQUAL, "r");
	if (!cc)
		return (DEL_ERR);

	coln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (coln, &coln_rec, EQUAL,"r");
	if (!cc)
	{
		cohr_rec.hhco_hash	=	coln_rec.hhco_hash;
		cc = find_rec (cohr,&cohr_rec, EQUAL,"r");
		if (cc)
			return (DEL_OK);
		
		if (cohr_rec.stat_flag [0] == '9' || cohr_rec.stat_flag [0] == 'D')
			return (DEL_OK);
		
		return (DEL_ERR);
	}
	return (DEL_OK);
}
/*==================================
| Check for valid Purchase orders. |
==================================*/
int
CheckPurchaseOrders (void)
{
	poln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	if (!find_rec (poln, &poln_rec, EQUAL, "r"));
		return (DEL_ERR);

	return (DEL_OK);
}
/*=================================
| Check for valid goods receipts. |
=================================*/
int
CheckGoodsReceipts (void)
{
	pogl_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (pogl, &pogl_rec, GTEQ, "r");
	while (!cc && pogl_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (!LIVE_GRIN)
			return (DEL_ERR);

		cc = find_rec (pogl, &pogl_rec, NEXT, "r");
	}
	return (DEL_OK);
}
/*=================================
| Check for valid goods receipts. |
=================================*/
int
CheckTransactions (void)
{
	abc_selfield (itln, "itln_hhbr_hash");

	itln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	if (!find_rec (itln, &itln_rec, EQUAL, "r"))
		return (DEL_ERR);

	abc_selfield (itln, "itln_r_hhbr_hash");

	itln_rec.r_hhbr_hash	=	inmr_rec.hhbr_hash;
	if (!find_rec (itln, &itln_rec, EQUAL, "r"))
		return (DEL_ERR);

	return (DEL_OK);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		if (deleteType [0] == 'M')
			rv_pr (ML (mlSkMess010),20,0,1);

		if (deleteType [0] == 'B')
			rv_pr (ML (mlSkMess011),20,0,1);

		if (deleteType [0] == 'W')
			rv_pr (ML (mlSkMess012),20,0,1);

		if (deleteType [0] == 'A')
			rv_pr (ML (mlSkMess013),20,0,1);

		box (0,2,80,16);
		line_at (1,0,80);
		line_at (9,1,79);
		line_at (14,1,79);
		line_at (20,0,80);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);

		strcpy (err_str,ML (mlStdMess039));
		print_at (22,0,err_str,comm_rec.est_no,comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
