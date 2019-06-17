/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_pricreat.c  )                                 |
|  Program Desc  : ( Maintain Individual Price Book.              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, inpd,     ,     ,     ,     ,         |
|  Database      : (book)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  inpd,     ,     ,     ,     ,     ,     ,         |
|  Database      : (book)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 16/11/89         |
|---------------------------------------------------------------------|
|  Date Modified : (16/11/89)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (13/09/90)      | Modified  by  : Scott Darrow.    |
|                                                                     |
|  Comments      : (13/09/90) - General Update for New Scrgen. S.B.D. |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: pricreat.c,v $
| Revision 5.3  2002/07/18 07:15:54  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 09:19:33  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:34  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:02  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:24  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:54  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:35  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/11/11 05:59:59  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.6  1999/11/03 07:32:21  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.5  1999/10/08 05:32:45  scott
| First Pass checkin by Scott.
|
| Revision 1.4  1999/07/16 00:43:32  scott
| Updated for abc_delete
|
| Revision 1.3  1999/06/20 05:20:27  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pricreat.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_pricreat/pricreat.c,v 5.3 2002/07/18 07:15:54 scott Exp $";

#define	MAXLINES	500
#include 	<pslscr.h>
int	new_inpb;
#define	valid_inpi()	(!cc && inpi_rec.hhph_hash == inph_rec.hhph_hash)

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};
	
	int comm_no_fields = 3;
	
	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
	} comm_rec;

	/*=========================+
	 | Price Book Header File. |
	 +=========================*/
#define	INPH_NO_FIELDS	4

	struct dbview	inph_list [INPH_NO_FIELDS] =
	{
		{"inph_co_no"},
		{"inph_this_page"},
		{"inph_hhph_hash"},
		{"inph_blank_page"}
	};

	struct tag_inphRecord
	{
		char	co_no [3];
		int		this_page;
		long	hhph_hash;
		char	blank_page [2];
	}	inph_rec;

	/*=======================+
	 | Price Book Index File |
	 +=======================*/
#define	INPI_NO_FIELDS	2

	struct dbview	inpi_list [INPI_NO_FIELDS] =
	{
		{"inpi_hhph_hash"},
		{"inpi_reference"}
	};

	struct tag_inpiRecord
	{
		long	hhph_hash;
		char	reference [51];
	}	inpi_rec;

	/*=================+
	 | Price Book File |
	 +=================*/
#define	INPB_NO_FIELDS	4

	struct dbview	inpb_list [INPB_NO_FIELDS] =
	{
		{"inpb_co_no"},
		{"inpb_book"},
		{"inpb_description"},
		{"inpb_hhpb_hash"}
	};

	struct tag_inpbRecord
	{
		char	co_no [3];
		char	book [11];
		char	description [41];
		long	hhpb_hash;
	}	inpb_rec;

	/*=============================+
	 | Price Book Description File |
	 +=============================*/
#define	INPD_NO_FIELDS	3

	struct dbview	inpd_list [INPD_NO_FIELDS] =
	{
		{"inpd_hhpb_hash"},
		{"inpd_seq_no"},
		{"inpd_hhph_hash"}
	};

	struct tag_inpdRecord
	{
		long	hhpb_hash;
		int		seq_no;
		long	hhph_hash;
	}	inpd_rec;

/*=========================== 
| Local & Screen Structures.|
===========================*/
struct {
	char	dummy[11];
	int		seq_no;
	long	page_no;
	long	hhph_hash;
	char	ins_del[2];
	char	reference[51];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "book", 4, 13, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "Price Book ", " ", 
		NE, NO, JUSTLEFT, "", "", inpb_rec.book}, 
	{1, LIN, "desc", 5, 13, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description ", " ", 
		YES, NO, JUSTLEFT, "", "", inpb_rec.description}, 
	{2, TAB, "seq_no", MAXLINES, 0, INTTYPE, 
		"NNNNN", "          ", 
		" ", "", " Seq ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.seq_no}, 
	{2, TAB, "page_no", 0, 0, LONGTYPE, 
		"NNNNN", "          ", 
		" ", "", "Page ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.page_no}, 
	{2, TAB, "hhph_hash", 0, 0, LONGTYPE, 
		"NNNNN", "          ", 
		" ", "", "hhph ", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.hhph_hash}, 
	{2, TAB, "reference", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "         P a g e        R e f e r e n c e         ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.reference}, 
	{2, TAB, "ins_del", 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "", " ", " Insert, Delete ", 
		NI, NO, JUSTRIGHT, "ID", "", local_rec.ins_del}, 
	{0, LIN, "dummy", 0, 0, CHARTYPE, 
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
int  spec_valid (int field);
int  insert_line (void);
int  delete_line (void);
void show_inpb (char *key_val);
void show_inph (char *key_val);
int  save_inph (void);
void load_inpd (void);
void update (void);
void update_inpb (void);
void update_inpd (void);
int  heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	SETUP_SCR (vars);
	/*-------------------------------
	| initialise terminal etc		|
	-------------------------------*/
	init_scr();
	set_tty(); 
	set_masks();
	/*---------------------------
	| open database etc			|
	---------------------------*/
	OpenDB();

	while (prog_exit == 0) 
	{
		new_inpb 	= FALSE;
		entry_exit 	= FALSE;
		restart 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		search_ok 	= TRUE;
		init_vars(1);	
		init_vars(2);	
		lcount [2] 	= 0;
		/*-------------------------------
		| enter price book page			|
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;
		if (new_inpb)
		{
			heading(2);
			entry(2);
			if (restart)
				continue;
		}
		/*---------------------------
		| edit screen				|
		---------------------------*/
		edit_all();
		if (restart)
			continue;
		update();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec ("inph", inph_list, INPH_NO_FIELDS, "inph_id_no");
	open_rec ("inpi", inpi_list, INPI_NO_FIELDS, "inpi_hhph_hash");
	open_rec ("inpb", inpb_list, INPB_NO_FIELDS, "inpb_id_no");
	open_rec ("inpd", inpd_list, INPD_NO_FIELDS, "inpd_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose ("inph");
	abc_fclose ("inpi");
	abc_fclose ("inpb");
	abc_fclose ("inpd");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| price book name			|
	---------------------------*/
	if (LCHECK("book"))
	{
		if (SRCH_KEY)
		{
			show_inpb(temp_str);
			return(0);
		}
		strcpy(inpb_rec.co_no,comm_rec.tco_no);
		cc = find_rec("inpb",&inpb_rec,COMPARISON,"u");
		if (cc)
			new_inpb = TRUE;
		else
		{
			new_inpb = FALSE;
			entry_exit = TRUE;
			DSP_FLD ("desc");
			load_inpd();
		}
		return(0);
	}
	/*-----------------------------------
	| price book detail sequencing		|
	-----------------------------------*/
	if (LCHECK("seq_no"))
	{
		local_rec.seq_no = line_cnt;
		DSP_FLD ("seq_no");
		return(0);
	}
	/*-------------------------------
	| price book page number		|
	-------------------------------*/
	if (LCHECK("page_no"))
	{
		/*---------------------------
		| end of input				|
		---------------------------*/
		if (last_char == FN16)
		{
			entry_exit = 1;
			blank_display(/*line_cnt*/);
			return(0);
		}
		/*-----------------------
		| search				|
		-----------------------*/
		if (SRCH_KEY)
		{
			show_inph(temp_str);
			return(0);
		}
		/*-------------------------------
		| column definition page		|
		-------------------------------*/
		if (local_rec.page_no <= 0)
		{
			print_mess(" Definition Page Cannot be part of Price Book ");
			return(1);
		}
		/*-------------------------------
		| check page number exists		|
		-------------------------------*/
		strcpy(inph_rec.co_no,comm_rec.tco_no);
		inph_rec.this_page = local_rec.page_no;
		cc = find_rec("inph",&inph_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf(err_str," Page Number %ld doesn't exist ",local_rec.page_no);
			print_mess(err_str);
			return(1);
		}
		local_rec.hhph_hash = inph_rec.hhph_hash;
		cc = find_hash("inpi",&inpi_rec,COMPARISON,"r",local_rec.hhph_hash);
		if (cc)
			sprintf(local_rec.reference,"%-50.50s"," ");
		else
			strcpy(local_rec.reference,inpi_rec.reference);
		DSP_FLD("reference");
		return(0);
	}
	/*-----------------------------------
	| insert / delete page option		|
	-----------------------------------*/
	if (LCHECK("ins_del"))
	{
		if (prog_status == ENTRY)
			return(0);
		/*---------------------------------------
		| invoke insert / delete page option	|
		---------------------------------------*/
		if (local_rec.ins_del[0] == 'I')
			return(insert_line());
		else
			return(delete_line());
	}
	return(0);
}

int
insert_line (
 void)
{
	int	i;
	int	this_page = (line_cnt / TABLINES);
	/*---------------------------------------
	| check for max lines			|
	---------------------------------------*/
	if (lcount[cur_screen] == vars[scn_start].row)
	{
		print_mess(" Cannot Insert more Lines ");
		sleep(2);
		return(1);
	}
	/*-------------------------------
	| make space for line			|
	--------------------------------*/
	print_at(2,0, "Inserting Line ... ");
	fflush(stdout);
	for (i = line_cnt,line_cnt = lcount[cur_screen];line_cnt > i;line_cnt--)
	{
		getval(line_cnt - 1);
		putval(line_cnt);
		if (this_page == (line_cnt / TABLINES))
			line_display();
	}
	lcount[cur_screen]++;
	if (this_page == (line_cnt / TABLINES))
		blank_display();
	/*---------------------------
	| cleanup message			|
	---------------------------*/
	move(0,2);
	cl_line();
	fflush(stdout);
	/*---------------------------
	| perform insertion			|
	---------------------------*/
	init_ok = 0;
	prog_status = ENTRY;
	scn_entry(cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	/*---------------------------
	| re read line				|
	---------------------------*/
	line_cnt = i;
	getval(line_cnt);
	return(0);
}

int
delete_line (
 void)
{
	int	i;
	int	this_page = (line_cnt / TABLINES);
	/*-------------------------------
	| no lines to delete			|
	-------------------------------*/
	if (lcount[cur_screen] < 1)
	{
		print_mess(" No Lines to Delete ");
		sleep(2);
		return(1);
	}
	/*---------------------------------------
	| must be in edit mode to use delete	|
	---------------------------------------*/
	if (prog_status == ENTRY)
	{
		print_mess(" Can only Insert / Delete Lines in Edit ");
		sleep(2);
		return(1);
	}
	print_at(2,0, "Deleting Line ... ");
	fflush(stdout);
	lcount[cur_screen]--;
	/*---------------------------
	| delete line				|
	---------------------------*/
	for (i = line_cnt;line_cnt < lcount[cur_screen];line_cnt++)
	{
		getval(line_cnt + 1);
		putval(line_cnt);
		if (this_page == (line_cnt / TABLINES))
			line_display();
	}
	/*-----------------------------------
	| blank last line - if required		|
	-----------------------------------*/
	if (this_page == (line_cnt / TABLINES))
		blank_display();

	move(0,2);
	cl_line();
	fflush(stdout);
	line_cnt = i;
	if (line_cnt > lcount[cur_screen])
		line_cnt = lcount[cur_screen];
	getval(line_cnt);
	return(0);
}

void
show_inpb (
 char *key_val)
{
	work_open();
	save_rec("#Price Book","#Description ");
	strcpy(inpb_rec.co_no,comm_rec.tco_no);
	sprintf(inpb_rec.book,"%-10.10s",key_val);
	cc = find_rec("inpb",&inpb_rec,GTEQ,"r");
	while (!cc && !strcmp(inpb_rec.co_no,comm_rec.tco_no) && 
				  !strncmp(inpb_rec.book,key_val,strlen(key_val)))
	{
		cc = save_rec(inpb_rec.book,inpb_rec.description);
		if (cc)
			break;
		cc = find_rec("inpb",&inpb_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(inpb_rec.co_no,comm_rec.tco_no);
	sprintf(inpb_rec.book,"%-10.10s",temp_str);
	cc = find_rec("inpb",&inpb_rec,COMPARISON,"r");
	if (cc)
		file_err( cc, "inpb", "DBFIND");
}

void
show_inph (
 char *key_val)
{
	work_open();
	save_rec("#Page.","# Reference ");
	strcpy(inph_rec.co_no,comm_rec.tco_no);
	inph_rec.this_page = atoi(key_val);
	cc = find_rec("inph",&inph_rec,GTEQ,"r");
	while (!cc && !strcmp(inph_rec.co_no,comm_rec.tco_no))
	{
		sprintf(err_str,"%d",inph_rec.this_page);
		if (inph_rec.this_page != 0 && !strncmp(err_str,key_val,strlen(key_val)))
			save_inph();
		cc = find_rec("inph",&inph_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(inph_rec.co_no,comm_rec.tco_no);
	inph_rec.this_page = atoi(temp_str);
	cc = find_rec("inph",&inph_rec,COMPARISON,"r");
	if (cc)
		file_err( cc, "inph", "DBFIND");
}

int
save_inph (
 void)
{
	int	ref_found = FALSE;
	char	page_no[6];

	sprintf(page_no,"%5d",inph_rec.this_page);
	cc = find_hash("inpi",&inpi_rec,GTEQ,"r",inph_rec.hhph_hash);
	while (valid_inpi())
	{
		cc = save_rec(page_no,inpi_rec.reference);
		if (cc)
			return(cc);
		ref_found = TRUE;
		cc = find_hash("inpi",&inpi_rec,NEXT,"r",inph_rec.hhph_hash);
	}

	if (!ref_found)
	{
		sprintf(err_str,"%-50.50s"," ");
		cc = save_rec(page_no,err_str);
		if (cc)
			return(cc);
	}
	return(0);
}

void
load_inpd (
 void)
{
	scn_set(2);
	lcount[2] = 0;
	abc_selfield("inph","inph_hhph_hash");
	/*---------------------------------------
	| read price book detail lines		|
	---------------------------------------*/
	inpd_rec.hhpb_hash = inpb_rec.hhpb_hash;
	inpd_rec.seq_no = 0;
	cc = find_rec("inpd",&inpd_rec,GTEQ,"r");
	while (!cc && inpd_rec.hhpb_hash == inpb_rec.hhpb_hash)
	{
		/*---------------------------------------
		| read price book page header		|
		---------------------------------------*/
		cc = find_hash("inph",&inph_rec,COMPARISON,"r",inpd_rec.hhph_hash);
		if (!cc)
		{
			local_rec.seq_no = lcount[2];
			local_rec.page_no = inph_rec.this_page;
			local_rec.hhph_hash = inph_rec.hhph_hash;
			/*---------------------------------------
			| read price book page reference	|
			---------------------------------------*/
			cc = find_hash("inpi",&inpi_rec,COMPARISON,"r",inpd_rec.hhph_hash);
			if (cc)
				sprintf(local_rec.reference,"%-50.50s"," ");
			else
				strcpy(local_rec.reference,inpi_rec.reference);
			strcpy(local_rec.ins_del," ");
			putval(lcount[2]++);
		}
		cc = find_rec("inpd",&inpd_rec,NEXT,"r");
	}
	abc_selfield("inph","inph_id_no");
	scn_set(1);
}

void
update (
 void)
{
	clear();
	update_inpb();
	update_inpd();
}

void
update_inpb (
 void)
{
	printf("\n\r\n\rUpdating Price Book ...");
	fflush(stdout);
	if (new_inpb)
	{
		cc = abc_add("inpb",&inpb_rec);
		if (cc)
			file_err( cc, "inpb", "DBADD");
		/*---------------------------------------
		| read back for inpb_hash		|
		---------------------------------------*/
		cc = find_rec("inpb",&inpb_rec,COMPARISON,"r");
		if (cc)
			file_err( cc, "inpb", "DBFIND");
	}
	else
	{
		cc = abc_update("inpb",&inpb_rec);
		if (cc)
			file_err( cc, "inpb", "DBUPDATE");
	}
}

void
update_inpd (
 void)
{
	abc_selfield("inph","inph_hhph_hash");
	scn_set(2);
	printf("\n\r\n\rUpdating Price Book Details ...");
	fflush(stdout);
	for (line_cnt = 0;line_cnt < lcount[2];line_cnt++)
	{
		getval(line_cnt);
		inpd_rec.hhpb_hash = inpb_rec.hhpb_hash;
		inpd_rec.seq_no = line_cnt;
		cc = find_rec("inpd",&inpd_rec,COMPARISON,"u");
		if (cc)
		{
			putchar('A');
			fflush(stdout);
			inpd_rec.hhph_hash = local_rec.hhph_hash;
			cc = abc_add("inpd",&inpd_rec);
			if (cc)
				file_err( cc, "inpd", "DBADD");
		}
		else
		{
			putchar('U');
			fflush(stdout);
			inpd_rec.hhph_hash = local_rec.hhph_hash;
			cc = abc_update("inpd",&inpd_rec);
			if (cc)
				file_err( cc, "inpd", "DBUPDATE");
		}
	}
	/*-----------------------------------
	| delete extra price book pages		|
	-----------------------------------*/
	inpd_rec.hhpb_hash = inpb_rec.hhpb_hash;
	inpd_rec.seq_no = lcount[2];
	cc = find_rec("inpd",&inpd_rec,GTEQ,"u");
	while (!cc && inpd_rec.hhpb_hash == inpb_rec.hhpb_hash)
	{
		putchar('D');
		fflush(stdout);
		abc_unlock("inpd");
		cc = abc_delete("inpd");
		if (cc)
			file_err( cc, "inpd", "DBDELETE");
		inpd_rec.hhpb_hash = inpb_rec.hhpb_hash;
		inpd_rec.seq_no = lcount[2];
		cc = find_rec("inpd",&inpd_rec,GTEQ,"u");
	}
}

int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set(scn);

	clear();

	rv_pr(" Maintain Individual Price Book ",24,0,1);

	move(0,1);
	line(80);

	if (scn == 1)
		box(0,3,80,2);
	move(0,21); 
	line(80);
	print_at (22,0,"Company : %s  %s",comm_rec.tco_no,comm_rec.tco_name);
	scn_write(scn);
    return (EXIT_SUCCESS);
}
