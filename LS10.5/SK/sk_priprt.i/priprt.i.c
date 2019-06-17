/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: priprt.i.c,v 5.5 2002/07/18 07:15:55 scott Exp $
|  Program Name  : (sk_priprt.i.c)
|  Program Desc  : (Inventory Price Book Print Input)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 03/05/88         |
|---------------------------------------------------------------------|
| $Log: priprt.i.c,v $
| Revision 5.5  2002/07/18 07:15:55  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.4  2002/07/17 09:57:58  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/28 08:46:40  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/09 09:19:38  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:36  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:08  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/21 01:11:40  scott
| Updated to ensure default on start and end selection takes into account
| high end character set. Start range is space and and range is 0xff
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: priprt.i.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_priprt.i/priprt.i.c,v 5.5 2002/07/18 07:15:55 scott Exp $";

#define	MAXLINES	10
#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include	<get_lpno.h>

#define	PINDEX		 (local_rec.pindex [0] == 'Y')
#define	validPage(i) (local_rec.start_page [i] <= local_rec.end_page [i] || \
			 		  local_rec.end_page [i] == 0)

FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct inpbRecord	inpb_rec;

/*=========================== 
| Local & Screen Structures.|
===========================*/
struct {
	char	dummy [11];
	long	printDate;
	char	systemDate [11];
	long	lsystemDate;
	int		start_page [3];
	int		end_page [3];
	int		last_col;
	int		lpno;
	int		ncopies;
	int		nxt_num;
	char	pindex [12];
	char	price_type [12] [7];
	char	old_type [12] [7];
	char	usr_head [31];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "lpno", 3, 20, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer number ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "book", 4, 20, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "Price Book ", " ", 
		YES, NO, JUSTLEFT, "", "", inpb_rec.book}, 
	{1, LIN, "ncopies", 3, 60, INTTYPE, 
		"NN", "          ", 
		" ", "1", "No. Copies ", " ", 
		YES, NO, JUSTRIGHT, "1", "9", (char *)&local_rec.ncopies}, 
	{1, LIN, "pindex", 4, 60, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Print Index ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.pindex}, 
	{1, LIN, "desc", 5, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description ", " ", 
		NA, NO, JUSTLEFT, "", "", inpb_rec.description}, 
	{1, LIN, "printDate", 6, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Date:", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.printDate}, 
	{1, LIN, "usr_head", 7, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Price Book Heading : ", " Enter user defined heading to appear on price book. ", 
		NO, NO, JUSTLEFT, "", "", local_rec.usr_head}, 
	{1, LIN, "price1", 9, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price1_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [0]}, 
	{1, LIN, "price2", 10, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price2_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [1]}, 
	{1, LIN, "price3", 11, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price3_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [2]}, 
	{1, LIN, "price4", 12, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price4_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [3]}, 
	{1, LIN, "price5", 13, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price5_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [4]}, 
	{1, LIN, "price6", 14, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price6_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [5]}, 
	{1, LIN, "price7", 15, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price7_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [6]}, 
	{1, LIN, "price8", 16, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price8_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [7]}, 
	{1, LIN, "price9", 17, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, comm_rec.price9_desc, "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [8]}, 
	{1, LIN, "price10", 18, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, "Item Number", "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [9]}, 
	{1, LIN, "price11", 19, 20, CHARTYPE, 
		"UU", "          ", 
		" ", (char *) &local_rec.nxt_num, "On Hand", "(N)o or order of printing (1-9) ", 
		NO, NO, JUSTLEFT, "N1234567890", "", local_rec.price_type [10]}, 
	{1, LIN, "price12", 20, 20, CHARTYPE, 
		"UU", "          ", 
		" ", "Y", "Book List", " List of Page Heading ", 
		NO, NO, JUSTLEFT, "YN", "", local_rec.price_type [11]}, 
	{2, TAB, "last_col", MAXLINES, 0, INTTYPE, 
		"N", "          ", 
		" ", "", "L", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.last_col}, 
	{2, TAB, "start_page_1", 0, 5, INTTYPE, 
		"NNNNN", "          ", 
		" ", "0", "Start Page ", " page number 0 indicates sub range to be ignored ", 
		YES, NO, JUSTRIGHT, "0", "99999", (char *)&local_rec.start_page [0]}, 
	{2, TAB, "end_page_1", 0, 5, INTTYPE, 
		"NNNNN", "          ", 
		" ", "0", " End Page ", " page number 0 indicates last page ", 
		YES, NO, JUSTRIGHT, "0", "99999", (char *)&local_rec.end_page [0]}, 
	{2, TAB, "start_page_2", 0, 5, INTTYPE, 
		"NNNNN", "          ", 
		" ", "0", "Start Page ", " page number 0 indicates sub range to be ignored ", 
		YES, NO, JUSTRIGHT, "0", "99999", (char *)&local_rec.start_page [1]}, 
	{2, TAB, "end_page_2", 0, 5, INTTYPE, 
		"NNNNN", "          ", 
		" ", "0", " End Page ", " page number 0 indicates last page ", 
		YES, NO, JUSTRIGHT, "0", "99999", (char *)&local_rec.end_page [1]}, 
	{2, TAB, "start_page_3", 0, 5, INTTYPE, 
		"NNNNN", "          ", 
		" ", "0", "Start Page ", " page number 0 indicates sub range to be ignored ", 
		YES, NO, JUSTRIGHT, "0", "99999", (char *)&local_rec.start_page [2]}, 
	{2, TAB, "end_page_3", 0, 5, INTTYPE, 
		"NNNNN", "          ", 
		" ", "0", " End Page ", " page number 0 indicates last page ", 
		YES, NO, JUSTRIGHT, "0", "99999", (char *)&local_rec.end_page [2]}, 
	{0, LIN, "", 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

extern	int	EnvScreenOK;

/*=======================
| Function Declarations |
=======================*/
void 	RunProgram 		(void);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	SrchInpb 		(char *);
void 	LoadDefault 	(void);
void 	RedrawPrice 	(int, int);
int  	spec_valid 		(int);
int  	CalcNextNumber 	(int);
int  	FindPrice 		(void);
int  	heading 		(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	i;
	int	SELECT_OK;

	SETUP_SCR (vars);
	
	EnvScreenOK	=	FALSE;

	tab_col = 5;

	init_scr ();
	set_tty (); 
	set_masks ();

	OpenDB ();

	LoadDefault ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	while (prog_exit == 0) 
	{
		entry_exit 	= FALSE;
		restart 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		init_vars (2);
		lcount [2]	= 0;
	
		local_rec.nxt_num = 2;
		SELECT_OK = FALSE;

		/*---------------------
		| Initialise old_type |
		---------------------*/
		for (i = 0; i < 12; i++)
			strcpy (local_rec.old_type [i], local_rec.price_type [i]);

		/*---------------------------
		| edit screens				|
		---------------------------*/
		heading (1);
		scn_display (1);
		local_rec.printDate = local_rec.lsystemDate;
		edit_all ();
        if (restart) {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }
		/*---------------------------------------
		| check for selection of price book	|
		---------------------------------------*/
		if (inpb_rec.hhpb_hash == 0L)
		{
			print_mess ( ML(" No Price Book has been Selected "));
			sleep (sleepTime);
			continue;
		}

		for (i = 0; i < 12; i++)
		{
			if (local_rec.price_type [i] [0] != 'N')
			{
				SELECT_OK = TRUE;
				break;
			}
		}

		if (local_rec.price_type [11] [0] == 'Y')
			strcpy (local_rec.price_type [11], "1");

		if (!SELECT_OK)
		{
			print_mess (ML (" No Prices have been Selected "));
			sleep (sleepTime);
			continue;
		}

		RunProgram ();
		prog_exit = TRUE;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
RunProgram (
 void)
{
	int	i;
	int	j;
	int	tmp_type;

	/*---------------------------
	| set up screen				|
	---------------------------*/
	clear ();
	/*---------------------------
	| open output				|
	---------------------------*/
	if ((fout = popen ("sk_priprt", "w")) == 0)
		sys_err ("Error in sk_priprt during (POPEN)", errno, PNAME);

	/*---------------------------
	| output data				|
	---------------------------*/
	fprintf (fout, "%d\n", local_rec.lpno);
	fprintf (fout, "%d\n", local_rec.ncopies);
	fprintf (fout, "%ld\n", inpb_rec.hhpb_hash);
	fprintf (fout, "%s\n", PINDEX ? "Y" : "N");
	fprintf (fout, "%ld\n", local_rec.printDate);
	fprintf (fout, "%-30.30s\n", local_rec.usr_head);
	
	for (i = 1; i < 10; i++)
	{
		tmp_type = 0;

		for (j = 0; j < 12; j++)
		{
			if (i == atoi (local_rec.price_type [j]))
			{
				tmp_type = j + 1;
				break;
			}
		}

		switch (tmp_type)
		{
		case	0:
		case	1:
		case	2:
		case	3:
		case	4:
		case	5:
		case	6:
		case	7:
		case	8:
		case	9:
			fprintf (fout, "%d\n", tmp_type);
			break;
	
		case	10:
			fprintf (fout, "I\n");
			break;
	
		case	11:
			fprintf (fout, "O\n");
			break;
	
		case	12:
			fprintf (fout, "L\n");
			break;
	
		default:
			fprintf (fout, "I\n");
			break;
		}
	}

	/*---------------------------------------
	| output start / end pages		|
	---------------------------------------*/
	scn_set (2);
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);
		for (i = 0;i < local_rec.last_col;i++)
		{
			fprintf (fout, "%d\n", local_rec.start_page [i]);
			fprintf (fout, "%d\n", local_rec.end_page [i]);
		}
	}
	fprintf (fout, "-1\n");
	fclose (fout);
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
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inpb, inpb_list, INPB_NO_FIELDS, "inpb_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inpb);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	int	i = 0;
	int	indx;

	if (LCHECK ( "lpno") ) 
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML ("Invalid printer."));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| price book type validation		|
	-----------------------------------*/
	if (strncmp (FIELD.label, "price", 5) == 0) 
	{
		indx = atoi (FIELD.label + 5) - 1;
		if (indx == 11)
		{
			if (dflt_used)
			{
				strcpy (local_rec.price_type [11], "Yes");
				for (i = 0;i < 11;i++)
				{
					strcpy (local_rec.price_type [i], "No ");
					display_field (field + i - indx);
				}
				display_field (label ("price12"));
				local_rec.nxt_num = 1;

				/*-----------------
				| Update old_type |
				-----------------*/
				for (i = 0; i < 12; i++)
					strcpy (local_rec.old_type [i], local_rec.price_type [i]);
				return (EXIT_SUCCESS);
			}

			switch (temp_str [0])
			{
			case 'Y':
				strcpy (local_rec.price_type [indx], "Yes");
				for (i = 0;i < 11;i++)
				{
					strcpy (local_rec.price_type [i], "No ");
					display_field (field + i - indx);
				}
				display_field (label ("price12"));
				local_rec.nxt_num = 1;
				break;

			case 'N':
				strcpy (local_rec.price_type [indx], "No ");
				break;
			}
			display_field (label ("price12"));

			/*-----------------
			| Update old_type |
			-----------------*/
			for (i = 0; i < 12; i++)
				strcpy (local_rec.old_type [i], local_rec.price_type [i]);
			return (EXIT_SUCCESS);
		}

		if (temp_str [0] == 'N')
		{
			if (local_rec.old_type [indx] [0] != 'N')
				CalcNextNumber (indx);

			strcpy (local_rec.price_type [indx], "No ");
			display_field (field + i - indx);

			/*-----------------
			| Update old_type |
			-----------------*/
			for (i = 0; i < 12; i++)
				strcpy (local_rec.old_type [i], local_rec.price_type [i]);

			return (EXIT_SUCCESS);
		}

		if (local_rec.old_type [indx] [0] != 'N')
		{
			if (atoi (temp_str) == atoi (local_rec.old_type [indx]) ||
			    temp_str [0] == 'N')
			{
				sprintf (local_rec.price_type [indx], "%d ", atoi (temp_str));
				display_field (field);

				/*-----------------
				| Update old_type |
				-----------------*/
				for (i = 0; i < 12; i++)
					strcpy (local_rec.old_type [i], local_rec.price_type [i]);
				return (EXIT_SUCCESS);
			}
			else
			{
				sprintf (err_str, ML ("You must enter No or %d "), 
				        atoi (local_rec.old_type [indx]));

				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (local_rec.nxt_num > 9)
		{
			print_mess (ML ("You have already selected 9 prices "));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (atoi (temp_str) != local_rec.nxt_num)
		{
			sprintf (err_str, ML ("Next number in order is %d "), local_rec.nxt_num);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.price_type [indx], "%d ", atoi (temp_str));
		if (local_rec.old_type [indx] [0] == 'N')
			local_rec.nxt_num++;

		strcpy (local_rec.price_type [11], "No ");
		display_field (label ("price12"));

		/*-----------------
		| Update old_type |
		-----------------*/
		for (i = 0; i < 12; i++)
			strcpy (local_rec.old_type [i], local_rec.price_type [i]);

		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| index printing validation		|
	---------------------------------------*/
	if (strcmp (FIELD.label, "pindex") == 0) 
	{
		strcpy (local_rec.pindex, (PINDEX) ? "Yes" : "No ");
		display_field (field);
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| price book validation			|
	-------------------------------*/
	if (LCHECK ("book"))
	{
		if (SRCH_KEY)
		{
			SrchInpb (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (inpb_rec.co_no, comm_rec.co_no);
		cc = find_rec (inpb, &inpb_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (err_str, ML (" Price Book %s doesn't exist "), inpb_rec.book);
			print_mess (err_str);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("printDate"))
	{
		if (dflt_used)
		{
			local_rec.printDate = local_rec.lsystemDate;
			DSP_FLD ("printDate");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| start page validation			|
	-------------------------------*/
	if (strncmp (FIELD.label, "start_page_", 11) == 0) 
	{
		indx = atoi (FIELD.label + 11);
		if (indx > local_rec.last_col)
		{
			local_rec.last_col = indx;
			display_field (label ("last_col"));
		}
		if (prog_status != ENTRY)
		{
			/*---------------------------
			| check start page			|
			---------------------------*/
			if (!validPage (indx - 1))
			{
				print_mess (ML (" Invalid Start Page "));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| end page validation			|
	-------------------------------*/
	if (strncmp (FIELD.label, "end_page_", 9) == 0) 
	{
		indx = atoi (FIELD.label + 9);
		if (indx > local_rec.last_col)
		{
			local_rec.last_col = indx;
			display_field (label ("last_col"));
		}
		/*---------------------------
		| check end page			|
		---------------------------*/
		if (!validPage (indx - 1))
		{
			print_mess (ML (" Invalid End Page "));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
CalcNextNumber (
 int field)
{
	int	tmp_num;
	int	tmp_val;
	int	i;
	int	j;
	char	tmp_label [12];


	tmp_num = atoi (local_rec.old_type [field]);

	if ((tmp_num + 1) == local_rec.nxt_num)
	{
		local_rec.nxt_num--;
		return (EXIT_SUCCESS);
	}
	
	for (i = tmp_num + 1; i < local_rec.nxt_num; i++)
	{
		for (j = 0; j < 11; j++)
		{
			if (atoi (local_rec.price_type [j]) == i)
			{
				tmp_val = atoi (local_rec.price_type [j]);
				tmp_val--;
				sprintf (local_rec.price_type [j], "%d ", tmp_val);
		
				sprintf (tmp_label, "price%d", j + 1);
				display_field (label (tmp_label));
			}
		}
	}
	local_rec.nxt_num--;
	return (EXIT_SUCCESS);

}

void
SrchInpb (
 char *key_val)
{
	work_open ();
	save_rec ("#Price Book", "#Description ");
	strcpy (inpb_rec.co_no, comm_rec.co_no);
	sprintf (inpb_rec.book, "%-10.10s", key_val);
	cc = find_rec (inpb, &inpb_rec, GTEQ, "r");
	while (!cc && !strcmp (inpb_rec.co_no, comm_rec.co_no) && !strncmp (inpb_rec.book, key_val, strlen (key_val)))
	{
		cc = save_rec (inpb_rec.book, inpb_rec.description);
		if (cc)
			break;
		cc = find_rec (inpb, &inpb_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (inpb_rec.co_no, comm_rec.co_no);
	sprintf (inpb_rec.book, "%-10.10s", temp_str);
	cc = find_rec (inpb, &inpb_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in inpb During (DBFIND) search", cc, PNAME);
}

void
LoadDefault (
 void)
{
	int	i;
	/*---------------------------
	| load screen 1				|
	---------------------------*/
	inpb_rec.hhpb_hash = 0L;
	local_rec.lpno = 1;
	local_rec.ncopies = 1;
	strcpy (local_rec.pindex, "No ");
	/*-----------------------------------
	| initialise book type selection	|
	-----------------------------------*/
	for (i = 0;i < 12;i++)
		strcpy (local_rec.price_type [i], (i == 0) ? "1 " : "No ");
	/*---------------------------
	| load screen 2				|
	---------------------------*/
	scn_set (2);
	lcount [2] = 0;
	local_rec.last_col = 1;
	for (i = 0;i <= 2;i++)
	{
		local_rec.start_page [i] = (i == 0) ? 1 : 0;
		local_rec.end_page [i] = 0;
	}
	putval (lcount [2]++);
}

void
RedrawPrice (
 int field, 
 int indx)
{
	int	i;
	int	option;

	if (local_rec.price_type [indx] [0] == 'N')
		option = FindPrice ();
	else
		option = indx;

	for (i = 0;i < 12;i++)
	{
		if (i == option)
			strcpy (local_rec.price_type [i], "Yes");
		else
			strcpy (local_rec.price_type [i], "No ");
		display_field (field + i - indx);
	}
}

int
FindPrice (
 void)
{
	int	i;

	for (i = 11;i > 0 && local_rec.price_type [i] [0] == 'N';i--)
		;

	return ((local_rec.price_type [i] [0] == 'Y') ? i : 0);
}

int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (" Price Book Print Input "), 28, 0, 1);

	if (scn == 1)
	{
		box (0, 2, 80, 18);
		move (1, 8);
		line (79);
	}

	line_at (1, 0, 80);
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	scn_write (scn);
    return (EXIT_SUCCESS);
}

