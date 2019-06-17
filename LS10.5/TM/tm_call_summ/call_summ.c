/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: call_summ.c,v 5.4 2001/11/19 02:17:52 scott Exp $
|  Program Name  : (tm_call_summ.c)
|  Program Desc  : (Telemarketing Script Display)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 31/07/91         |
|---------------------------------------------------------------------|
| $Log: call_summ.c,v $
| Revision 5.4  2001/11/19 02:17:52  scott
| Updated from testing
|
| Revision 5.3  2001/11/08 06:29:39  scott
| Updated to rebuild to bring inline with standard.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: call_summ.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_call_summ/call_summ.c,v 5.4 2001/11/19 02:17:52 scott Exp $";

#include	<ml_tm_mess.h>
#include	<pslscr.h>		
#include	<get_lpno.h>
#include 	<arralloc.h>

#include	"schema"

struct commRecord	comm_rec;
struct tmshRecord	tmsh_rec;
struct tmslRecord	tmsl_rec;
struct tmcfRecord	tmcf_rec;
struct tmchRecord	tmch_rec;
struct tmclRecord	tmcl_rec;

/*
 *	Structure for dynamic array,  for the tallyRec lines for qsort
 */
struct TallyStruct
{
	char	key [6];
	int		promptNo;
	int		replayNo1;
	int		replayNo2;
	int		replayNo3;
	int		replayNo4;
	int		replayNo5;
	int		replayNo6;
	int		replayNo7;
	int		replayNo8;
}	*tallyRec;
	DArray tally_details;
	int	tallyCnt = 0;

extern	int	TruePosition;

int		TallySort			(const	void *,	const void *);


void	AddTally 		(int, int);

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	int		cam_no;
	char	cam_desc [81];
	int		scr_no;
	char	scr_desc [41];
	int		lpno;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "cam_no", 3, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", "", "Campaign ", "Enter Campaign To Display.", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.cam_no}, 
	{1, LIN, "cam_desc", 3, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.cam_desc}, 
	{1, LIN, "scr_no", 5, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", "", "Script   ", "Enter Script To Display.", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.scr_no}, 
	{1, LIN, "scr_desc", 5, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.scr_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	SrchTmsh 		(char *);
void 	SrchTmch 		(char *);
void 	Process 		(void);
int 	heading 		(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
	int argc, 
	char *argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();

	OpenDB ();

	set_masks ();	
	init_vars (1);		

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags
		 */
		search_ok 	= TRUE;
		entry_exit 	= TRUE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);	

		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (!restart) 
			Process ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmch, tmch_list, TMCH_NO_FIELDS, "tmch_hhsh_hash");
	open_rec (tmcl, tmcl_list, TMCL_NO_FIELDS, "tmcl_id_no");
	open_rec (tmsh, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");
	open_rec (tmsl, tmsl_list, TMSL_NO_FIELDS, "tmsl_id_no");
}

void
CloseDB (void)
{
	abc_fclose (tmcf);
	abc_fclose (tmch);
	abc_fclose (tmcl);
	abc_fclose (tmsh);
	abc_fclose (tmsl);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	char	workBuffer [41];

	if (LCHECK ("cam_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmch (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmcf_rec.co_no, comm_rec.co_no);
		tmcf_rec.campaign_no = local_rec.cam_no;
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (err_str, "%-s, ", rtrim (tmcf_rec.c_name1, workBuffer));
		strcat (err_str, rtrim (tmcf_rec.c_name2, workBuffer));

		sprintf (local_rec.cam_desc, "%-60.60s", err_str);
		
		DSP_FLD ("cam_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("scr_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmsh_rec.co_no, comm_rec.co_no);
		tmsh_rec.script_no = local_rec.scr_no;
		cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.scr_desc, "%-40.40s", tmsh_rec.desc);
		
		DSP_FLD ("scr_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmsh (
	char *key_val)
{
	char	tmp_scr_no [6];

	_work_open (4,0,40);
	strcpy (tmsh_rec.co_no, comm_rec.co_no);
	tmsh_rec.script_no = atoi (key_val);
	save_rec ("#No.", "#Script Description.");
	cc = find_rec (tmsh, &tmsh_rec, GTEQ, "r");
	while (!cc && !strcmp (tmsh_rec.co_no, comm_rec.co_no) &&
		      tmsh_rec.script_no >= atoi (key_val))
	{
		sprintf (tmp_scr_no, "%04d", tmsh_rec.script_no);
		cc = save_rec (tmp_scr_no, tmsh_rec.desc);
		if (cc)
			break;

		cc = find_rec (tmsh, &tmsh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmsh_rec.co_no, comm_rec.co_no);
	tmsh_rec.script_no = atoi (temp_str);
	cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmsh, "DBFIND");
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmch (	
	char *key_val)
{
	char	tmp_cam_no [6];

	_work_open (4,0,40);
	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#No", "#Campaign Description.");
	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
		      tmcf_rec.campaign_no >= atoi (key_val))
	{
		sprintf (tmp_cam_no, "%04d", tmcf_rec.campaign_no);
		cc = save_rec (tmp_cam_no, tmcf_rec.c_name1);
		if (cc)
			break;

		cc = find_rec (tmcf, &tmcf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (temp_str);
	cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmcf, "DBFIND");
}

void
Process (void)
{
	int	i;

	scn_write (1);
	scn_display (1);

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&tally_details, &tallyRec,sizeof (struct TallyStruct),10);
	tallyCnt = 0;

	/*-----------------
	| Open Dsp window |
	-----------------*/
	Dsp_prn_open (0, 7, 10, 
		     "                  Telemarketing Call Summary                              ", (char *) 0, 
	            (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);

	Dsp_saverec (" Prompt Description                    Responses                              ");
	Dsp_saverec ("                                  1     2     3     4     5     6     7     8 ");
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]  ");

	tmch_rec.hhsh_hash	=	tmsh_rec.hhsh_hash;
	cc = find_rec (tmch, &tmch_rec, GTEQ, "r");
	while (!cc && tmch_rec.hhsh_hash == tmsh_rec.hhsh_hash)
	{
		if (tmch_rec.hhcf_hash != tmcf_rec.hhcf_hash ||
		    strcmp (tmch_rec.co_no, comm_rec.co_no))
		{
			cc = find_rec (tmch, &tmch_rec, NEXT, "r");
			continue;
		}

		tmcl_rec.hhcl_hash 	= tmch_rec.hhcl_hash;
		tmcl_rec.line_no 	= 0;
		cc = find_rec (tmcl, &tmcl_rec, GTEQ, "r");
		while (!cc && tmcl_rec.hhcl_hash == tmch_rec.hhcl_hash)
		{
			AddTally (tmcl_rec.prmpt_no, tmcl_rec.rep_no);
	
			cc = find_rec (tmcl, &tmcl_rec, NEXT, "r");
		}
		cc = find_rec (tmch, &tmch_rec, NEXT, "r");
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (tallyRec, tallyCnt, sizeof (struct TallyStruct), TallySort);

	for (i = 0; i < tallyCnt; i++)
	{
		tmsl_rec.hhsh_hash 	= tmsh_rec.hhsh_hash;
		tmsl_rec.prmpt_no 	= tallyRec [i].promptNo;
		cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
		if (cc)
			sprintf (tmsl_rec.desc, "%-30.30s", "Prompt Not Found");

		sprintf 
		(
			err_str, 
			"%-30.30s%5d %5d %5d %5d %5d %5d %5d %5d", 
			tmsl_rec.desc, 
			tallyRec [i].replayNo1,
			tallyRec [i].replayNo2,
			tallyRec [i].replayNo3,
			tallyRec [i].replayNo4,
			tallyRec [i].replayNo5,
			tallyRec [i].replayNo6,
			tallyRec [i].replayNo7,
			tallyRec [i].replayNo8
		);
		Dsp_saverec (err_str);
	}

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&tally_details);
	Dsp_srch ();
	Dsp_close ();
	return;
}


void
AddTally (
	int		promptNo,
	int		replayNo)
{
	int		i;
	/*
	 * Step through the sorted array getting the appropriate records.
	 */
	for (i = 0; i < tallyCnt; i++)
	{
		if (tallyRec [i].promptNo == promptNo)
		{
			switch (replayNo)
			{
				case	1: tallyRec [i].replayNo1++; break;
				case	2: tallyRec [i].replayNo2++; break;
				case	3: tallyRec [i].replayNo3++; break;
				case	4: tallyRec [i].replayNo4++; break;
				case	5: tallyRec [i].replayNo5++; break;
				case	6: tallyRec [i].replayNo6++; break;
				case	7: tallyRec [i].replayNo7++; break;
				case	8: tallyRec [i].replayNo8++; break;
				default: tallyRec [i].replayNo1++; break;
			}
			return;
		}
	}
	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&tally_details, tallyRec, tallyCnt))
		sys_err ("ArrChkLimit (tallyRec)", ENOMEM, PNAME);

	sprintf (tallyRec [tallyCnt].key, "%05d", tmcl_rec.prmpt_no);
	tallyRec [tallyCnt].promptNo 	= promptNo;
	tallyRec [tallyCnt].replayNo1	= 0;
	tallyRec [tallyCnt].replayNo2	= 0;
	tallyRec [tallyCnt].replayNo3	= 0;
	tallyRec [tallyCnt].replayNo4	= 0;
	tallyRec [tallyCnt].replayNo5	= 0;
	tallyRec [tallyCnt].replayNo6	= 0;
	tallyRec [tallyCnt].replayNo7	= 0;
	tallyRec [tallyCnt].replayNo8	= 0;
		
	switch (replayNo)
	{
		case	1: tallyRec [tallyCnt].replayNo1++; break;
		case	2: tallyRec [tallyCnt].replayNo2++; break;
		case	3: tallyRec [tallyCnt].replayNo3++; break;
		case	4: tallyRec [tallyCnt].replayNo4++; break;
		case	5: tallyRec [tallyCnt].replayNo5++; break;
		case	6: tallyRec [tallyCnt].replayNo6++; break;
		case	7: tallyRec [tallyCnt].replayNo7++; break;
		case	8: tallyRec [tallyCnt].replayNo8++; break;
		default: tallyRec [tallyCnt].replayNo1++; break;
	}
	/*
	 * Increment array counter.
	 */
	tallyCnt++;
}

int 
TallySort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct TallyStruct a = * (const struct TallyStruct *) a1;
	const struct TallyStruct b = * (const struct TallyStruct *) b1;

	result = strcmp (a.key, b.key);

	return (result);
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
		line_at (1,0,80);

		rv_pr (ML (mlTmMess003), 25, 0, 1);

		box (0, 2, 80, 3);
		line_at (4,1,79);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
