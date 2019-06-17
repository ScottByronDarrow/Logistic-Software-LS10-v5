/*=====================================================================
|  Copyright (C) 1999 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: disp_dbaudit.c,v 5.1 2002/07/17 09:57:24 scott Exp $														  |
|  Program Name  : (disp_dbaudit)                                     |
|  Program Desc  : (DBAudit Display.                )      			  |
|---------------------------------------------------------------------|
|  Author        : Charlton D. Ho   | Date Written  : 01/10/2001      |
|---------------------------------------------------------------------|
| $Log: disp_dbaudit.c,v $
| Revision 5.1  2002/07/17 09:57:24  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.0  2002/05/07 10:05:38  scott
| Updated to ensure version numbers on new programs are correct.
|
| Revision 1.5  2002/04/11 03:50:47  scott
| Updated to add comments to audit files.
|
| Revision 1.4  2002/04/11 03:06:02  scott
| Updated to add AUDIT-COMMENT field.
|
| Revision 1.3  2002/04/11 02:01:19  scott
| Updated for lineup
|
| Revision 1.2  2001/11/13 01:15:59  scott
| Removed ^M
|
| Revision 1.1  2001/10/15 07:12:15  cha
| New program for display and printing of Audit files.
|															  |
|																	  |
======================================================================*/

#define	CCMAIN
char	*PNAME = "$RCSfile: disp_dbaudit.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/db_audit/disp_dbaudit.c,v 5.1 2002/07/17 09:57:24 scott Exp $";
		
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <get_lpno.h>
#include <arralloc.h>
#include <DateToString.h>

#include <dirent.h>
#include <string.h>
#include "schema"

struct commRecord comm_rec;
struct ccmrRecord ccmr_rec;

struct {
	char file_name [255];
	char user_name [40];
	Date date_from;
	Date date_to;
	int  printer_no;
	char print_display [6];
	char backgound [6];
	char  dummy [11];
} local_rec;

/*=========================
| Record structure to hold |
| Audit file contents	   |
==========================*/
#define MAX_AUDIT_LINES 1024
#define MAX_AUDIT_HEADER 1024

FILE *fp, 
	 *fout;

struct FileDetail {
	char FieldName [40];
	char OldVal	   [60];
	char NewVal    [60];
	int  HeaderID;
};

struct FileHeader {
	char UserName  [40];
	Date AuditDate;
	char AuditTime [10];
	char AuditComment [256];
	int	 DetailCount;
	int  HeaderID;
};

struct FileHeader *fHeader;
struct FileDetail *fDetail;

DArray	HeaderList;
DArray	DetailList;

char FileList [100] [255];

int	 FileCount 	= 0,
	 HeaderCtr 	= 1,
	 DetailCtr 	= 1,
	 AllUser 	= FALSE,
	 DISP		= TRUE;

const char *sixtySpaces  = "                                                            ";

static	struct	var	vars [] =
{
	{1, LIN, "FileName",	 4, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "File Name :", "Enter Audit Filename to be Processed [Search Available]",
		YES, YES,  JUSTLEFT, "", "", local_rec.file_name},
	{1, LIN, "UserName",	 5, 22, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "User Name :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.user_name},
	{1, LIN, "DateFrom",		7, 22, EDATETYPE,
		"NN", "          ",
		" ", " ",        "Date From :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.date_from},
	{1, LIN, "DateTo",		8, 22, EDATETYPE,
		"NN", "          ",
		" "," ",        "Date To   :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.date_to},
	{1, LIN, "printerNumber",		10, 22, INTTYPE,
		"NN", "          ",
		" ", "1",        "Printer Number :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printer_no},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=================
| Local Functions |
==================*/

int 	spec_valid 		(int);
int 	heading 		(int);
int 	OpenAudFileList	(void);
int 	LoadFile		(char *);
int 	DateRange		(long, long, long);

void	ReadMisc 		(void);
void	OpenDB	 		(void);
void	CloseDB 		(void);
void	shutdown_prog 	(void);
void	SrchFiles 		(char *);
void 	DisplayHeader	(void);
void	DisplayAllDetail(void);
void 	DisplayFilterDet(void);
void	RunProgram		(char *);

void	GetFieldName	(char *, char *);	
void	GetBeforeValue	(char *, char *);
void	GetAfterValue	(char *, char *);

char 	*ClipNL			(char *);	




int
main (
 int argc, 
 char * argv [])
{
	char *sptr;
				
	SETUP_SCR (vars);
	
	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;
		
	if (!strcmp (sptr,"dbaudit_disp"))
		DISP = TRUE;
	
	if (!strcmp (sptr, "dbaudit_prn"))
		DISP = FALSE;
	
	OpenDB ();
	OpenAudFileList ();
	
	if (argc == 6 || argc == 5)
	{
		if (argc == 6)
		{
			sprintf (local_rec.user_name, "%s", argv [3]);
			local_rec.date_from = StringToDate (argv [4]);
			local_rec.date_to = StringToDate (argv [5]);
			AllUser = TRUE;
		}
		
		if (argc == 5)
		{
			local_rec.date_from = StringToDate (argv [3]);
			local_rec.date_to = StringToDate (argv [4]);
			AllUser = FALSE;
		}
					
		local_rec.printer_no = atoi (argv [1]);
		sprintf (local_rec.file_name, "%s", argv [2]);
							
 		ArrAlloc (&HeaderList, &fHeader, sizeof (fHeader), 1000);
		ArrAlloc (&DetailList, &fDetail, sizeof (fDetail), 1000);
		
		init_scr ();
		swide ();
		set_tty ();

		print_at (20,0, ML (mlStdMess038), 
					comm_rec.co_no, comm_rec.co_name);
	
		print_at (21,0, ML (mlStdMess039), 
					comm_rec.est_no, comm_rec.est_name);

		print_at (22,0, ML (mlStdMess099), 
					comm_rec.cc_no, comm_rec.cc_name);

		move (0,23);
		line (130);
		
		LoadFile (local_rec.file_name);
		DisplayHeader ();
		
		if (DISP)
		{	
			if (argc == 6)
				DisplayFilterDet ();
				
			if (argc == 5)
				DisplayAllDetail ();
					
			Dsp_srch ();
			Dsp_close ();	
		}
		else
		{
			dsp_screen ("Processing : DB Audit Printing.", 
					comm_rec.co_no, comm_rec.co_name);	
			getchar ();
		}
			
		
		ArrDelete (&HeaderList);
		ArrDelete (&DetailList);
		
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}
	
	
	
	/*========================= 
	| Program exit sequence	. |
	=========================*/
	init_scr 	 ();
	swide 		 ();
	set_tty 	 ();
	set_masks 	 ();
	init_vars 	 (1);
	
	if (DISP)
		FLD ("printerNumber") = ND;
	
	while (prog_exit == 0)
	{	
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart 	= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		
		
		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;
		
		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
		/*Process to run */
		RunProgram (argv [0]);
		prog_exit	= TRUE;
	}	
	shutdown_prog ();
	return (EXIT_SUCCESS);	
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
		
		if (DISP)
			rv_pr (ML ("Display Audit File"),50,0,1);
		else
			rv_pr (ML ("Print Audit File"),50,0,1);
		line_at (1,0,130);

		if (!DISP)
			box (0, 3, 131,  7);
		else
			box (0, 3, 131,  5);
			
		line_at (6,1,130);
		line_at (9,1,130);
		line_at (19,0,130);

		print_at (20,0, ML (mlStdMess038), 
					comm_rec.co_no, comm_rec.co_name);

		print_at (21,0, ML (mlStdMess039), 
					comm_rec.est_no, comm_rec.est_name);

		print_at (22,0, ML (mlStdMess099), 
					comm_rec.cc_no, comm_rec.cc_name);

		line_at (23,0,130);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,	comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	abc_fclose (ccmr);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");
	ReadMisc ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_dbclose ("data");	
}

void
shutdown_prog (void)
{
	CloseDB ();
	finish_prog ();
}

int
spec_valid (
	 int field)
{
		
	if (LCHECK ("FileName"))
	{
		if (SRCH_KEY)
		{
			SrchFiles (temp_str);
			return (EXIT_SUCCESS);
		}
		
		if (!strcmp(local_rec.file_name, sixtySpaces))
		{
			print_mess ("Filename must be input");
			sleep (2);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("UserName"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.user_name, "~");
			DSP_FLD ("UserName");
			AllUser = TRUE;
			return (EXIT_SUCCESS);
		}
		AllUser = FALSE;
		return (EXIT_SUCCESS);
	}
	
	
	if (LCHECK ("DateFrom"))
	{
		if (dflt_used)
		{
			local_rec.date_from = 0L;
			DSP_FLD ("DateFrom");
			
			return (EXIT_SUCCESS);
		}	
		
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("DateTo"))
	{
		if (dflt_used)
		{
			local_rec.date_to = TodaysDate ();
			DSP_FLD ("DateTo");
			
			return (EXIT_SUCCESS);
		}	
		
		
		if (local_rec.date_from > local_rec.date_to)
		{
			print_mess ("Date to must be greater than date from");
			sleep (2);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (F_HIDE (label ("printerNumber")))
			return (EXIT_SUCCESS);
			
		if (SRCH_KEY)
		{
			local_rec.printer_no = get_lpno (0);
			return (EXIT_SUCCESS);
		}	
		
		if (!valid_lp (local_rec.printer_no))
		{
			print_mess (ML (mlStdMess020));
			sleep (2);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=================================
| Function to get the files from  |
| $PROG_PATH/BIN/AUDIT            |
=================================*/
int OpenAudFileList (void)
{
	char 	*sptr = getenv ("PROG_PATH"),
			directory [255],
			filename [255];
	DIR 	*dp;
    struct 	dirent *dirp;	
    
    sprintf (directory, "%s/BIN/AUDIT", (sptr == (char *)0) ? 
    								  "/usr/LS10.5" : sptr);
    memset (FileList, 0, sizeof (FileList));
	
	/*--------------------
    | Open the directory |
    --------------------*/
    if ((dp = opendir(directory)) == NULL)
 		return (EXIT_FAILURE);
 
 	/*--------------------
    | Store file entries |
    --------------------*/
 	while ((dirp = readdir (dp)) != NULL)
    {
    	strcpy (filename, dirp->d_name);
		if ((strcmp(filename, ".") == 0)  || 
			(strcmp(filename, "..") == 0))
			continue;
		FileCount++;
		strcpy (FileList [FileCount], filename);	
	}
	closedir(dp);
	return (EXIT_SUCCESS);	
}

/*==================================
| Search Files in $PROG_PATH/AUDIT |
==================================*/
void
SrchFiles (
	char    *keyValue)
{
	int ctr = 0;
	
	_work_open (80,0,0);
    save_rec ("#File Name                                         ", "#");
    
    for (ctr = 1; ctr <= FileCount; ctr++)
    {
    	cc = save_rec (FileList [ctr],"");
        if (cc)
            break;
    }
    
    cc = disp_srch ();
    work_close ();
    if (cc)
        return;    
}

void
RunProgram (char *Program)
{
	char strTemp [11], strDateto [11], strDatefrom [11];
	
	sprintf (strTemp, "%2d", local_rec.printer_no);
	strcpy  (strDateto, DateToString (local_rec.date_to));
	strcpy 	(strDatefrom, DateToString (local_rec.date_from));
		
	CloseDB ();
	rset_tty ();
	
	clear ();
	fflush (stdout);
	
	if (AllUser)
	{
		execlp 
			 (
				Program,
				Program,
				strTemp,
				local_rec.file_name,
				strDatefrom,
				strDateto,
				 (char *) 0
			);
	}
	else
	{
		execlp 
			 (
				Program,
				Program,
				strTemp,
				local_rec.file_name,
				local_rec.user_name,
				strDatefrom,
				strDateto,
				 (char *) 0
			);
	}
	
	return;	
}

void 
DisplayHeader (void)
{
	char	head_text [300];	
		
	sprintf (head_text, " Audit Filename : %s ",local_rec.file_name);

	Dsp_prn_open (0, 0, 15, head_text, 
				comm_rec.co_no, comm_rec.co_name,
				comm_rec.est_no, comm_rec.est_name,
				comm_rec.cc_no, comm_rec.cc_name);

	sprintf (err_str, " AUDIT FILE DETAILS (AUDIT FILENAME : %88.88s ) ", local_rec.file_name);
	Dsp_saverec (err_str);

	Dsp_saverec ("");
	Dsp_saverec (" [REDRAW] [NEXT] [PREV] [EDIT/END] ");
		
	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
 	if (!DISP)
 	{
		if ((fout = popen ("pformat","w")) == 0)
			file_err (errno, "pformat", "POPEN");
	
	} /*!DISP*/	
}


void 
DisplayAllDetail (void)
{
	char 	workstr	  [150];
	int 	ctr  = 0,
			ctr2 = 0,
			blankctr = 0, 
			lineCtr = 3;
			
	for (ctr = 1; ctr < HeaderCtr; ctr ++)
	{
		if (!DateRange (fHeader [ctr].AuditDate, local_rec.date_from, local_rec.date_to))
		{	
			sprintf (workstr,
					"(AUDIT USER: %-20.20s) (AUDIT DATE: %-20.20s) (AUDIT TIME %-20.20s) RECORD ID [%03d]",
					fHeader [ctr].UserName, 
					DateToString (fHeader [ctr].AuditDate),
					fHeader [ctr].AuditTime,
					fHeader [ctr].HeaderID);
			Dsp_saverec (workstr);
			sprintf 
			(
				workstr,
				"(AUDIT COMMENT: %-s)",
				fHeader [ctr].AuditComment
			);
			Dsp_saverec (workstr);

			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
			Dsp_saverec (" FIELD NAME               ^E  BEFORE VALUE                               ^E  AFTER VALUE                                           ");
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
			lineCtr++;
			for (ctr2 = 1;ctr2 <= DetailCtr; ctr2++)
			{
				if (fDetail [ctr2].HeaderID == fHeader [ctr].HeaderID)
				{
					sprintf (workstr,
							 " %-24.24s ^E %-44.44s ^E %-44.44s",
							 fDetail [ctr2].FieldName,
							 fDetail [ctr2].OldVal,
							 fDetail [ctr2].NewVal);
					Dsp_saverec (workstr);
					lineCtr++;
					
					if (lineCtr == 13)
						lineCtr = 4;
				}
			}
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
			
			for (blankctr =lineCtr ; blankctr < 13; blankctr++)
				Dsp_saverec (" ");
			lineCtr = 3;
		}			
	}	
	return;	
}

void 
DisplayFilterDet(void)
{
	char 	workstr	  [150];
	int 	ctr  = 0,
			ctr2 = 0,
			blankctr = 0, 
			lineCtr = 3;
				
	for (ctr = 1; ctr < HeaderCtr; ctr ++)
	{
		if (strstr(local_rec.user_name, fHeader [ctr].UserName) && 
			!DateRange (fHeader [ctr].AuditDate, local_rec.date_from, local_rec.date_to))
		{
			sprintf (workstr,
					"(AUDIT USER: %20s) (AUDIT DATE: %20s) (AUDIT TIME %20s) [%d]",
					fHeader [ctr].UserName, 
					DateToString (fHeader [ctr].AuditDate),
					fHeader [ctr].AuditTime,
					fHeader [ctr].HeaderID);
			Dsp_saverec (workstr);
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
			Dsp_saverec (" FIELD NAME               ^E  BEFORE VALUE                               ^E  AFTER VALUE                                         ");
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
			lineCtr++;
			for (ctr2 = 1;ctr2 <= DetailCtr; ctr2++)
			{
				if (fDetail [ctr2].HeaderID == fHeader [ctr].HeaderID)
				{
					sprintf (workstr,
							 " %-24.24s ^E %-44.44s ^E %-44.44s",
							 fDetail [ctr2].FieldName,
							 fDetail [ctr2].OldVal,
							 fDetail [ctr2].NewVal);
					Dsp_saverec (workstr);
					lineCtr++;
					
					if (lineCtr == 13)
						lineCtr = 4;
				}
			}
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
			
			for (blankctr =lineCtr ; blankctr < 13; blankctr++)
				Dsp_saverec (" ");
			lineCtr = 4;
		}
			
	}	
	return;	
}

int LoadFile (
	char *fName)
{
	char lstr[150],
		 *tptr,
		 *sptr = getenv ("PROG_PATH"),
		 pathName [255];
	
	int hlinectr = 0,
		detailStart = FALSE,
		detailLabel = FALSE;
	
	char strTmp [100];

	sprintf (pathName ,"%s/BIN/AUDIT/%s",sptr, fName );

	
	
	fp = fopen (clip(pathName), "r");
	if (!fp)
		file_err (errno, fName, "fopen");
	
	while (!feof(fp))
	{
		if (hlinectr == 0)
			ArrChkLimit (&HeaderList, fHeader, HeaderCtr);
			
		memset (lstr, 0, sizeof (lstr));
		fgets (lstr, sizeof (lstr), fp);
		
		if (strlen(lstr) == 0)
			continue;
		
		if (strstr (lstr,"AUDIT-USER-NAME"))
		{
			tptr = strstr (lstr,"=");	
							
			if (tptr)
				sprintf (fHeader [HeaderCtr].UserName,"%s", ClipNL(tptr+1));
			else
				return (EXIT_FAILURE);
			hlinectr++;
			detailStart = FALSE;
		}
		else if (strstr (lstr,"AUDIT-DATE"))
		{
			tptr = strstr (lstr,"=");
			
			if (tptr)
				fHeader [HeaderCtr].AuditDate = atol(ClipNL(tptr+1));
			else
				return (EXIT_FAILURE);
			sprintf (strTmp,"LINE COUNT 5 [%s]",DateToString (fHeader [HeaderCtr].AuditDate));
			
			hlinectr++;
			detailStart = FALSE;
		}
		else if (strstr (lstr,"AUDIT-TIME"))
		{
			tptr = strstr (lstr,"=");	
							
			if (tptr)
				sprintf (fHeader [HeaderCtr].AuditTime,"%s", ClipNL(tptr+1));
			else
				return (EXIT_FAILURE);
			
			hlinectr++;
			detailStart = FALSE;
			
		}
		else if (strstr (lstr,"AUDIT-COMMENT"))
		{
			tptr = strstr (lstr,"=");	
							
			if (tptr)
				sprintf (fHeader [HeaderCtr].AuditComment,"%s", ClipNL(tptr+1));
			else
				return (EXIT_FAILURE);
			
			hlinectr++;
			detailStart = FALSE;
			
		}
		else if (strstr (lstr,"AUDIT-FIELD"))
		{
			detailLabel = TRUE;
			hlinectr++;	
		}
		
		if (hlinectr == 5)
			detailStart = TRUE;	
		
		if (detailStart)
		{
			if (!detailLabel)
			{
				ArrChkLimit (&DetailList, fDetail, DetailCtr);
				fDetail [DetailCtr].HeaderID = fHeader [HeaderCtr-1].HeaderID;
											
				GetFieldName (ClipNL(lstr), fDetail [DetailCtr]. FieldName);
				GetBeforeValue (ClipNL(lstr), fDetail [DetailCtr]. OldVal);
				GetAfterValue (ClipNL(lstr), fDetail [DetailCtr]. NewVal);
										
				DetailCtr++;	
			}
			detailLabel = FALSE;
		}
		if (hlinectr == 5)
		{
			fHeader [HeaderCtr].HeaderID = HeaderCtr;
			
			if (!detailLabel)
				HeaderCtr++;
				
			hlinectr = 0;
		}
	}
	return (EXIT_SUCCESS);
}

/*==============================
| strip out new line characters|
| and returns the string w/o   |
| the '\n' character		   |
==============================*/

char 
*ClipNL (
	char *buf)
{
	char locbuf [255];
	int ctr = 0;
	
	strcpy (locbuf,buf);
	for (ctr = 0; ctr <= strlen(locbuf); ctr++)
	{
		if (locbuf[ctr] !=10)
			locbuf[ctr] = locbuf [ctr];
		else
			locbuf [ctr] = '\0';
	}
	strcpy (buf,locbuf);
    return (buf);		
}

void 
GetFieldName (
	char *buf,
	char *newbuf )
{
	char locbuff  [255],
		 locbuff2 [255];
	int ctr = 0;
	
	memset (locbuff2, 0, sizeof (locbuff2));
	strcpy (locbuff, buf);
	while (locbuff [ctr] != 1)
	{
		locbuff2 [ctr] = locbuff [ctr];
		ctr++;
	}
	strcpy (newbuf, locbuff2);
}

void 
GetBeforeValue (
	char *buf, 
	char *newbuf)
{
	char locbuff  [255],
		 locbuff2 [255];
	int  ctr  = 0,
		 ctr2 = 0,
		 ctr3 = 0;
		 
	memset (locbuff2, 0, sizeof (locbuff2));
	strcpy (locbuff, buf);
	
	for (ctr = 0; ctr <= strlen (locbuff); ctr++)
	{
		if (locbuff [ctr] == 1)
			ctr2++;
		if (ctr2 == 1)
		{
			locbuff2 [ctr3] = locbuff [ctr];
			ctr3 ++;
		}
	}
	strcpy (newbuf, locbuff2);	
}

void GetAfterValue (
	char *buf, 
	char *newbuf)
{
	char locbuff  [255],
		 locbuff2 [255];
	int  ctr  = 0,
		 ctr2 = 0,
		 ctr3 = 0;
		 
	memset (locbuff2, 0, sizeof (locbuff2));
	strcpy (locbuff, buf);
	
	for (ctr = 0; ctr <= strlen (locbuff); ctr++)
	{
		if (locbuff [ctr] == 1)
			ctr2++;
		if (ctr2 == 2)
		{
			locbuff2 [ctr3] = locbuff [ctr];
			ctr3 ++;
		}
	}
	strcpy (newbuf, locbuff2);	
}

int
DateRange (
	long curDate,
	long fromDate,
	long toDate)
{
	if ((curDate >= fromDate) && (curDate <= toDate))
		return (EXIT_SUCCESS);
	else
		return	(EXIT_FAILURE);
}
