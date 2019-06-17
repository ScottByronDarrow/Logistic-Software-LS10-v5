/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_group.i.c,v 5.3 2002/07/17 09:57:53 scott Exp $
|  Program Name  : (sk_group.i.c) 
|  Program Desc  : (Input group And Class For Report Programs)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 21/03/89         |
|---------------------------------------------------------------------|
| $Log: sk_group.i.c,v $
| Revision 5.3  2002/07/17 09:57:53  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:18:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:41  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.5  2001/06/01 09:35:09  scott
| Updated to add validation on start and end class.
| Updated to add missing sleep on warning message.
|
| Revision 4.4  2001/05/16 06:34:13  cha
| Updated to fix out of sequence error in Oracle.
|
| Revision 4.3  2001/04/03 03:42:36  scott
| Updated as default descriptions on start and end category not displayed.
|
| Revision 4.2  2001/03/21 00:49:54  scott
| Updated to ensure default on start and end selection takes into account
| high end character set. Start range is space and and range is 0xff
|
| Revision 4.1  2001/03/16 07:56:12  scott
| Updated to ensure screen selections looks and works OK with LS10-GUI.
| Updated to clean code and add app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_group.i.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_group.i/sk_group.i.c,v 5.3 2002/07/17 09:57:53 scott Exp $";

#define	NO_BRANCHES	5
#define	TABLINES	NO_BRANCHES

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

				    
	/*================================================================
	| Special fields and flags  ##################################.  |
	================================================================*/
	char 	programName [41],
			programDesc [101],
			byWhat [2],
			programType [2];

	extern	int		TruePosition;

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;

/*============================ 
| Local & Screen Structures. |
============================*/

char	branch [NO_BRANCHES] [3];
int		byBranch;			/* TRUE if param [3] was 'B'	*/
int		useTabular;			/* TRUE if param [3] not ' '	*/

struct {
	char	dummy [11];
	char	back [2];
	char	backDesc [16];
	char	onite [2];
	char	oniteDesc [16];
	char	startCategory [12];
	char	startClass [2];
	char	startDesc [41];
	char	endCategory [12];
	char	endClass [2];
	char	endDesc [41];
	char	startGroup [13];
	char	endGroup [13];
	char	trueStart [12];
	char	trueEnd [12];
	int		printerNo;
	char	printerString [3];
	char	est_no [3];
	char	est_name [41];
	char	no_prompt [15];
	char	name_prompt [35];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "startClass", 5, 2, CHARTYPE, 
		"U", "          ", 
		" ", "A", "Start Class      ", "Input Start Class A-Z.", 
		YES, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.startClass}, 
	{1, LIN, "startCategory", 6, 2, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", " ", "Start Category   ", "Input Start Inventory Category.", 
		YES, NO, JUSTLEFT, "", "", local_rec.startCategory}, 
	{1, LIN, "startCategoryDesc", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description      ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.startDesc}, 
	{1, LIN, "endClass", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Z", "End Class        ", "Input End Class A-Z.", 
		YES, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.endClass}, 
	{1, LIN, "endCategory", 10, 2, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", " ", "End Category     ", "Input End Inventory Category.", 
		YES, NO, JUSTLEFT, "", "", local_rec.endCategory}, 
	{1, LIN, "endCategoryDesc", 11, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description      ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.endDesc}, 
	{1, LIN, "printerNo", 13, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer number   ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "back", 14, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background       ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 14, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onite", 15, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight        ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onite}, 
	{1, LIN, "oniteDesc", 15, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.oniteDesc}, 
	{2, TAB, "est_no", NO_BRANCHES, 6, CHARTYPE, 
		"UU", "          ", 
		" ", " ", local_rec.no_prompt, " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.est_no}, 
	{2, TAB, "est_name", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", local_rec.name_prompt, " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.est_name}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	LoadDetails 		(void);
void 	SrchExcf 			(char *);
int  	spec_valid 			(int);
int  	heading 			(int);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	i;
	int	j;
	char	*args [5];

	TruePosition	=	TRUE;
	SETUP_SCR (vars);

	/*-----------------------------------
	|	parameters						|
	|	1:	program name				|
	|	2:	program description			|
	|	3:	'W' - by warehouse			|
	|		'B' - by branch				|
	|		' ' - don't use tabular		|
	|		      screen				|
	|	4:	program type (optional)		|
	|               char string         |
	-----------------------------------*/

	if (argc < 4) 
	{
		print_at (0,0,mlSkMess137,argv [0]);
		return (EXIT_FAILURE);
	}
	
	if (strcmp (argv [0], "sk_groupc.i") == 0)
	{
		FLD ("printerNo")	= ND;
		FLD ("onite")		= ND;
		FLD ("oniteDesc")	= ND;
		FLD ("back")		= ND;
		FLD ("backDesc")	= ND;
	}

	sprintf (programName,"%-.40s",argv [1]);
	sprintf (programDesc,"%-.100s",argv [2]);
	sprintf (byWhat,"%1.1s",argv [3]);
	useTabular = (byWhat [0] == 'W' || byWhat [0] == 'B');

	if (argc == 5)
		sprintf (programType, "%1.1s", argv [4]);
	else
		sprintf (programType, "%1.1s", " ");

	if (byWhat [0] == 'B')
	{
		strcpy (local_rec.no_prompt,	" Branch ");
		strcpy (local_rec.name_prompt,	" Branch Name               ");
		byBranch = TRUE;
	}
	else
	{
		strcpy (local_rec.no_prompt,	" Warehouse ");
		strcpy (local_rec.name_prompt,	" Warehouse Name            ");
		byBranch = FALSE;
	}

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	/*===========================
	| Open main database files. |  
	===========================*/
	OpenDB ();

	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit 	= FALSE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
   	search_ok 	= TRUE;

	init_vars 	(1);
	heading 	(1);
	entry 		(1);
    if (prog_exit) 
	{
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	heading 	(1);
	scn_display (1);
	edit 		(1);
	prog_exit = TRUE;
    if (restart) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }
	
	if (useTabular)
	{
		LoadDetails ();
		heading 	(2);
		scn_display (2);
		edit 		(2);
		prog_exit = TRUE;
        if (restart) 
		{
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }
	}
	
	rset_tty ();

	/*---------------------------------------
	|	set up args for branches/warehouses	|
	---------------------------------------*/
	for (i = 0;useTabular && i < NO_BRANCHES;i++)
		args [i] = strdup ("  ");

	for (i = 0,j = 0;useTabular && i < NO_BRANCHES;i++)
		if (strcmp (branch [i],"  "))
			args [j++] = strdup (branch [i]);

	sprintf 
	(
		local_rec.startGroup,
		"%1.1s%-11.11s",
		local_rec.startClass,
		local_rec.trueStart
	);
	sprintf 
	(
		local_rec.endGroup,
		"%1.1s%-11.11s", 
		local_rec.endClass, 
		local_rec.trueEnd
	);
	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onite [0] == 'Y') 
		if (useTabular)
		{
			execlp 
			(
				"ONIGHT",
				"ONIGHT",
				programName,
				byWhat,
				programDesc,
				args [0],
				args [1],
				args [2],
				args [3],
				args [4],
				local_rec.printerString,
				local_rec.startGroup,
				local_rec.endGroup,
				" ", programDesc, (char *)0
			);
		}	
		else
		{
			execlp 
			(
				"ONIGHT",
				"ONIGHT",
				programName,
				local_rec.printerString,
				local_rec.startGroup,
				local_rec.endGroup,
				programType,
				programDesc,	
				 (char *)0
			 );
		}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () != 0)
		{
			clear ();
			print_at (0,0,ML (mlStdMess035));
			fflush (stdout);
		}
		else
			if (useTabular)
				execlp (programName,
					programName,
					byWhat,
					programDesc,
					args [0],
					args [1],
					args [2],
					args [3],
					args [4],
					local_rec.printerString,
					local_rec.startGroup,
					local_rec.endGroup,
					" ", (char *)0);
			else
				execlp (programName,
					programName,
					local_rec.printerString,
					local_rec.startGroup,
					local_rec.endGroup,
					programType, (char *)0);
	}
	else 
	{
		clear ();
		print_at (0,0,ML (mlStdMess035));
		fflush (stdout);

		if (useTabular)
			execlp (programName,
				programName,
				byWhat,
				programDesc,
				args [0],
				args [1],
				args [2],
				args [3],
				args [4],
				local_rec.printerString,
				local_rec.startGroup,
				local_rec.endGroup,
				" ", (char *)0);
		else
			execlp (programName,
				programName,
				local_rec.printerString,
				local_rec.startGroup,
				local_rec.endGroup,
				programType, (char *)0);
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

int
spec_valid (
 int field)
{
	int	i;
			
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
	if (LCHECK ("startCategory"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.startCategory);

		if (dflt_used)
		{
			sprintf (local_rec.startCategory,"%-11.11s","           ");
			sprintf (local_rec.startDesc,"%-40.40s",ML ("BEGINNING OF RANGE"));
			memset ((char *)local_rec.trueStart,0,sizeof (local_rec.trueStart));
			DSP_FLD ("startCategoryDesc");
			return (EXIT_SUCCESS);
		}
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (prog_status != ENTRY && 
				strcmp (local_rec.startCategory,local_rec.endCategory) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.startDesc,"%-40.40s",excf_rec.cat_desc);
		strcpy (local_rec.trueStart, local_rec.startCategory);
		DSP_FLD ("startCategoryDesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("endCategory"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.endCategory);
		
		if (dflt_used)
		{
			sprintf (local_rec.endCategory,"%-11.11s","~~~~~~~~~~~");
			sprintf (local_rec.endDesc,"%-40.40s","END OF RANGE");
			memset ((char *)local_rec.trueEnd,0xff,sizeof (local_rec.trueEnd));
			DSP_FLD ("endCategoryDesc");
			return (EXIT_SUCCESS);
		}
		
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (local_rec.startCategory,local_rec.endCategory) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.endDesc,excf_rec.cat_desc);
		strcpy (local_rec.trueEnd, local_rec.endCategory);
		DSP_FLD ("endCategoryDesc");
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
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.printerString,"%d",local_rec.printerNo);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("est_no"))
	{
		/*-----------------------
		|	branch not required	|
		-----------------------*/
		if (dflt_used)
		{
			/*-------------------------------
			|	move branches below up 1	|
			-------------------------------*/
			for (i = line_cnt;line_cnt < NO_BRANCHES - 1;line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				strcpy (branch [line_cnt],branch [line_cnt + 1]);
				line_display ();
			}
			strcpy (branch [NO_BRANCHES - 1],"  ");

			strcpy (local_rec.est_no,"  ");
			sprintf (local_rec.est_name,"%-40.40s"," ");
			line_display ();
			putval (line_cnt);
			lcount [2]--;

			line_cnt = i;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}

		for (i = 0;i < NO_BRANCHES;i++)
		{
			if (i == line_cnt)
				continue;

			/*---------------------------
			| cannot duplicate branches	|
			---------------------------*/
			if (!strcmp (local_rec.est_no,branch [i]))
			{
				print_mess ((byBranch) ?  ML(mlStdMess239) : ML (mlStdMess240));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		if (byBranch)
		{
			strcpy (esmr_rec.co_no,comm_rec.co_no);
			strcpy (esmr_rec.est_no,local_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");

			if (cc)
			{
				print_mess (ML (mlStdMess073));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			strcpy (local_rec.est_no,esmr_rec.est_no);
			strcpy (branch [line_cnt],esmr_rec.est_no);
			strcpy (local_rec.est_name,esmr_rec.est_name);
		}
		else
		{
			strcpy (ccmr_rec.co_no,comm_rec.co_no);
			strcpy (ccmr_rec.est_no,comm_rec.est_no);
			strcpy (ccmr_rec.cc_no,local_rec.est_no);
			cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");

			if (cc)
			{
				print_mess (ML (mlStdMess100));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			strcpy (local_rec.est_no,ccmr_rec.cc_no);
			strcpy (branch [line_cnt],ccmr_rec.cc_no);
			strcpy (local_rec.est_name,ccmr_rec.name);
		}
		
		putval (line_cnt);
		line_display ();
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'N')
			strcpy (local_rec.backDesc, ML ("NO"));
		else
			strcpy (local_rec.backDesc, ML ("YES"));

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}
	
	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK ("onite"))
	{
		if (local_rec.onite [0] == 'N')
			strcpy (local_rec.oniteDesc, ML ("NO"));
		else
			strcpy (local_rec.oniteDesc, ML ("YES"));

		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (excf);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_dbclose ("data");
}

/*===========================================
|	load first NO_BRANCHES into structure	|
===========================================*/
void
LoadDetails (
 void)
{
	int	i;
	lcount [2] = 0;
	scn_set (2);

	if (byBranch)
	{
		strcpy (esmr_rec.co_no,comm_rec.co_no);
		strcpy (esmr_rec.est_no,"  ");
		cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	}
	else
	{
		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		strcpy (ccmr_rec.est_no,comm_rec.est_no);
		cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	}

	for (i = 0;i < NO_BRANCHES;i++)
	{
		if (cc || strcmp ((byBranch) ? esmr_rec.co_no : ccmr_rec.co_no,comm_rec.co_no) || (!byBranch && strcmp (ccmr_rec.est_no,comm_rec.est_no)))
		{
			strcpy (local_rec.est_no,"  ");
			strcpy (branch [i],"  ");
			sprintf (local_rec.est_name,"%-40.40s"," ");
			break;
		}
		else
		{
			if (byBranch)
			{
				strcpy (local_rec.est_no,esmr_rec.est_no);
				strcpy (branch [i],esmr_rec.est_no);
				strcpy (local_rec.est_name,esmr_rec.est_name);
			}
			else
			{
				strcpy (local_rec.est_no,ccmr_rec.cc_no);
				strcpy (branch [i],ccmr_rec.cc_no);
				strcpy (local_rec.est_name,ccmr_rec.name);
			}
		}
		putval (i);
		lcount [2]++;
		if (byBranch)
			cc = find_rec (esmr, &esmr_rec, NEXT, "r");
		else
			cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
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
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && 
				  !strcmp (excf_rec.co_no, comm_rec.co_no))
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
		file_err (cc, excf, "DBFIND");
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
		if (scn == 1)
		{
			sprintf (err_str,ML (mlSkMess031),programDesc);
			rv_pr (err_str, (80 - strlen (err_str)) / 2,0,1);
		}
		else
		{
			if (byBranch)
				rv_pr (ML (mlSkMess032),20,0,1);
			else
				rv_pr (ML (mlSkMess033),20,0,1);
		}
		line_at (1,0,80);

		if (scn == 1)
		{
			line_at (8,1,79);
			line_at (12,1,79);
			box (0,4,80,11);
		}
		line_at (20,0,80);
		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,  comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
