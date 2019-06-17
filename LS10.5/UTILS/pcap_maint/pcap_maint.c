/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( pcap_maint.c   )                                 |
|  Program Desc  : (Maintains printcap entries                      ) |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 15/01/91         |
|---------------------------------------------------------------------|
|  Date Modified : (11/02/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (09/05/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (28/06/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (15/08/92)      | Modified  by  : Trevor van Bremen|
|  Date Modified : (16/02/95)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (12/09/97)      | Modified  by  : Ana Marie Tario. |
|                :                                                    |
|                                                                     |
|  Comments      : (11/02/91) User is now asked to confirm update if  |
|                : they try to update and all required fields haven't |
|                : been entered.                                      |
|                : (09/05/91) Added | to list of valid chars for an   |
|                : escape sequence.                                   |
|  (28/06/91)    : Cursor is now turned on for updating of escape     |
|                : sequences.                                         |
|  (15/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (16/02/95)    : Allowed for maintenance of compressed mode         |
|  (12/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                                                                     |
=====================================================================*/
#define	TCAP
#define CCMAIN
#define MAX_PA 21

#include <ml_std_mess.h>
#include <ml_utils_mess.h>
#include <pslscr.h>
#include <hot_keys.h>
#include <getnum.h>
#include <tabdisp.h>

char	*PNAME = "$RCSfile: pcap_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/pcap_maint/pcap_maint.c,v 5.2 2001/08/09 09:27:06 scott Exp $";

char	*sptr;
char	*tptr;
char	reqd_desc[41];
char	reqd_atrb[4];

char	*val_1_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+-_.";
char	*val_2_chars = "^ abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+-*/%=#$&@'`[]{}_()?<>;,.|\\!~\"";

FILE	*descf;
FILE	*pcapf;
FILE	*pcapp;

int	new_printer;
int	no_in_desc_tab = 0;

char	*deflt_box = "----||-+--||#*x ";
char	*deflt_ex = "2";
char	*blank_attr = "\0";

char	*patrb[MAX_PA] = {
	 "TF=", 	/* 0  Top of form.					*/
	 "XN=", 	/* 1  Expanded print on.			*/
	 "XF=", 	/* 2  Expanded print off.			*/
	 "I1=", 	/* 3  Initialisation string			*/
	 "I2=", 	/* 4  Deinitialisation string		*/
	 "P0=", 	/* 5  Switch to '10' pitch.			*/
	 "P2=", 	/* 6  Switch to '12' pitch.			*/
	 "P6=", 	/* 7  Switch to '16' pitch.			*/
	 "us=", 	/* 8  Underscore on					*/
	 "ue=", 	/* 9  Underscore off				*/
	 "F1=", 	/* 10 Switch to font number 1		*/
	 "F2=", 	/* 11  Switch to font number 2		*/
	 "F3=", 	/* 12  Switch to font number 3		*/
	 "SN=", 	/* 13  Switch on Overstrike.		*/
	 "SF=", 	/* 14  Switch off Overstrike.		*/
	 "EX=", 	/* 15  Divisor ( 2 or 4 ) use with .E	*/
	 "so=", 	/* 16  Turn on bold print.			*/
	 "se=", 	/* 17  Turn off bold print.			*/
	 "go=", 	/* 18  Turn on graphics mode.		*/
	 "ge=", 	/* 19  Turn off graphics mode.		*/
	 "bx=", 	/* 20  Box chars					*/
};

struct PATRB_REC {
	char	p_atrb[4];
	char	p_atrb_desc[41];
	char	p_esc_seq[51];
	char	p_reqd[4];
} p_tab_line[MAX_PA], tmp_line;

struct	{
	char	dummy[11];
	char	printer_name[15];
	char	atrb_desc[41];
	char	esc_seq[51];
} local_rec;

char	directory[100];
char	pdesc_dir[100];
char	filename[100];

static	int	quit_func(int iUnused, KEY_TAB *psUnused);
static	int	update_func(int iUnused, KEY_TAB *psUnused);
static	int	exit_update_func(int iUnused, KEY_TAB *psUnused);
	
static	KEY_TAB edit_keys [] =
{
   { "[Q]UIT",		'Q', quit_func,
	"Exit without updating the printcap.",				"A" },
   { "[U]PDATE",	'U', update_func,
	"Update the line at the current cursor position.",		"E" },
   { NULL,		FN16, exit_update_func,
	"Exit and update the printcap.",				"A" },
   END_KEYS
};
	     
static	struct	var	vars[]	={	

	{1, LIN, "printer_name", 2, 20, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Printer Name:", "Enter name of printer you wish to maintain", 
		NE, NO, JUSTLEFT, "", "", local_rec.printer_name}, 
	{2, LIN, "esc_seq", 21, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.dummy}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int spec_valid (int field);
void srch_printer (char *key_val);
void process (void);
void create_table (void);
void erase_btm_scn (void);
int chck_reqd (void);
void load_prnt_desc (void);
void add_desc (int position);
void init_p_table (void);
void load_printer (void);
int load_line (char *line);
int heading (int scn);
void get_esc_seq (int x, int y, char *mask, char *buf);

int
main (
 int                argc,
 char*              argv[])
{
	char	*sptr = getenv("PROG_PATH");

	sprintf(pdesc_dir,"%s/BIN/SCN/prnt_desc",(sptr != (char *)0) ? sptr : "/usr/LS10.5");
	sprintf(directory,"%s/BIN/MENUSYS",(sptr != (char *)0) ? sptr : "/usr/LS10.5");

	SETUP_SCR(vars);

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/
	swide();

	/*---------------------------
	| WARNING!!! The next line  |
	| alters the standard range |
	| of valid chars within the |
	| screen generator!!!!!!!!! |
	---------------------------*/
	alpha = val_1_chars;

	/*---------------------------------
	| Initialise array used for table |
	---------------------------------*/
	init_p_table();

	set_help(FN6, "FN6");

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok = 1;
		entry_exit = 1;
		prog_exit = 0;
		restart = 0;

		init_vars(1);	

		heading(1);
		entry(1);

		if (prog_exit || restart)
			continue;

		if (!restart) 
			process ();
	}

	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	if (LCHECK("printer_name")) 
        {
		if (last_char == SEARCH)
		{	
            srch_printer (temp_str);
			return(0);
		}
	
		sprintf(filename, "%s/PRINT/%s", directory, clip(local_rec.printer_name));
		load_printer ();
		return(0);
	}

	return(0);
}

/*===================================
| Search for existing printcap files |
===================================*/
void
srch_printer (
 char*              key_val)
{
	char	tmpname[15];
	char	*lclptr;

	work_open();
	save_rec("#Printer Name ","#");

	sprintf(filename,"ls %s/PRINT/%s* 2> /dev/null",directory,key_val);

	if ((pcapp = popen(filename,"r")) == 0)
	{
		sprintf(err_str,"Error in %s during (POPEN)",filename);
		sys_err(err_str,errno,PNAME);
	}
		
	sptr = fgets(err_str,80,pcapp);

	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';
		lclptr = strrchr(sptr, '/');
		if (lclptr)
			lclptr++;

		sprintf(tmpname,"%-14.14s",lclptr);

		cc = save_rec(tmpname," ");
		if (cc)
			break;

		sptr = fgets(err_str,80,pcapp);
	}
	pclose(pcapp);
	
	cc = disp_srch();
	sprintf(local_rec.printer_name,"%-14.14s",temp_str);
	work_close();
}

void
process (void)
{
	create_table();

	if (new_printer)
		rv_pr(ML(mlUtilsMess115),60,2,1);
	/*	rv_pr(" NEW PRINTCAP ",60,2,1);*/

	/*----------------------
	| Display Instructions |
	----------------------*/
	box (4,16,125,2);
/*
	rv_pr("INSTRUCTIONS",60,16,0);
	rv_pr("   For ESCAPE please enter \134E.",5,17,0);
	rv_pr("   For a CONTROL character please enter a carat(^) followed by the desired character  eg. CTRL A should be entered as ^A.",5,18,0);*/
	rv_pr(ML(mlUtilsMess116),60,16,0);
	rv_pr(ML(mlUtilsMess117),5,17,0);
	rv_pr(ML(mlUtilsMess118),5,18,0);
	

	tab_scan("pcap");
	tab_close("pcap",TRUE);
}

/*---------------------------------------------
| Create table of escape sequences from array |
---------------------------------------------*/
void
create_table (void)
{
	int	i;

	tab_open("pcap",edit_keys,3,10,8,FALSE);
	tab_add("pcap","#%-5.5s  %-40.40s  %-50.50s   %-8.8s ","Atrb.","Attribute Description","Escape Sequence","Required");

	/*-----------------------
	| Load table from array |
	-----------------------*/
	for (i = 0; i < MAX_PA; i++)
	tab_add("pcap",
		" %-3.3s   %-40.40s  %-50.50s      %-3.3s ",
		p_tab_line[i].p_atrb,
		p_tab_line[i].p_atrb_desc,
		p_tab_line[i].p_esc_seq,
		p_tab_line[i].p_reqd);

}

/*-----------------------
| Quit without updating |
-----------------------*/
static int
quit_func (
 int                iUnused,
 KEY_TAB*           psUnused)
{
	return(FN16);
}

/*------------------------------------------------
| Update the escape sequence on the current line |
------------------------------------------------*/
static int
update_func (
 int                iUnused,
 KEY_TAB*           psUnused)
{
	int	pos_atrb = 0;
	char	dummy[133];

	tab_get("pcap", dummy, CURRENT, 0);
	pos_atrb = tab_tline("pcap");

	memcpy ((char *) &tmp_line, (char *) &p_tab_line[pos_atrb], sizeof (struct PATRB_REC));   

	strcpy(local_rec.atrb_desc, tmp_line.p_atrb_desc);
	strcpy(local_rec.esc_seq, tmp_line.p_esc_seq);

	heading(2);
	init_ok = FALSE;
	last_char = REDRAW;

	get_esc_seq (70,21,"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",local_rec.esc_seq);
	crsr_off();

	init_ok = TRUE;

	strcpy(tmp_line.p_esc_seq, local_rec.esc_seq);

	memcpy ((char *) &p_tab_line[pos_atrb], (char *) &tmp_line, sizeof (struct PATRB_REC));   

	tab_update("pcap",
		" %-3.3s   %-40.40s  %-50.50s      %-3.3s ",
		   tmp_line.p_atrb, 
		   tmp_line.p_atrb_desc, 
		   tmp_line.p_esc_seq, 
		   tmp_line.p_reqd);


	erase_btm_scn ();
    return (EXIT_SUCCESS);
}

void
erase_btm_scn (void)
{
	int	i;

	for (i = 20; i < 23; i++)
	{
		move(0,i);
		cl_line();
	}    
}

/*-----------------------------
| Exit and update the printcap |
-----------------------------*/
static int
exit_update_func (
 int                iUnused,
 KEY_TAB*           psUnused)
{
	int	i;
	int	reqd_ok;

	reqd_ok = chck_reqd();
	if (!reqd_ok)
	{
		/*sprintf(err_str, "\007 All required escape sequences have not been entered. Please confirm update. ");
		rv_pr(err_str, 28,20,1);*/
		rv_pr(ML(mlUtilsMess075), 28,20,1);
		sleep(1);
	}

	move(0,21);
	cl_line();
	sprintf(filename, "%s/PRINT/%s", directory, local_rec.printer_name);
	/*sprintf(err_str, "\007     Write to %s ?  ", filename);*/
	sprintf(err_str, ML(mlUtilsMess019), filename);
	/*print_mess(" Please enter Y to update printcap or N to continue editing. ");*/
	print_mess(ML(mlUtilsMess076));
	crsr_on();
	i = prmptmsg(err_str,"YyNn",43,21);

	if (i == 'N' || i == 'n')
	{
		move(0,20);
		cl_line();
		move(0,21);
		cl_line();
		clear_mess();
		return(0);
	}

	sprintf(filename, "%s/PRINT/%s", directory, clip(local_rec.printer_name));
	if ((pcapf = fopen(filename,"w")) == 0)
	{
		sprintf(err_str,"Error in %s during (FOPEN)",filename);
		sys_err(err_str,errno,PNAME);
	}

	move(0,20);
	cl_line();
	move(0,21);
	cl_line();
	clear_mess();
	/*sprintf(err_str, " UPDATING %s ",filename);*/
	sprintf(err_str, ML(mlUtilsMess020),filename);
	rv_pr(err_str,40,21,1);

	fprintf(pcapf,"%s:\134\n",local_rec.printer_name);
	for (i = 0; i < MAX_PA; i++)
	{
		if (!strncmp (p_tab_line[i].p_atrb, "dW#", 3))
		{
			if (!strncmp (p_tab_line[i].p_esc_seq, "TRUE", 4))
				fprintf(pcapf, "\t:%s:\134\n", p_tab_line[i].p_atrb);
		}
		else
			fprintf(pcapf, "\t:%s%s:%s\n", p_tab_line[i].p_atrb, p_tab_line[i].p_esc_seq, (i == (MAX_PA - 1)) ? " " : "\134");

	}

	fclose(pcapf);

	return(FN16);
}

/*----------------------------------------------------------------
| Check if all required attributes have escape sequences entered |
----------------------------------------------------------------*/
int
chck_reqd (void)
{
	int	i;

	for (i = 0; i < MAX_PA; i++)
	{
		if (!strcmp(p_tab_line[i].p_reqd, "YES") && strlen(p_tab_line[i].p_esc_seq) == 0)
		{
				strcpy(reqd_atrb,p_tab_line[i].p_atrb);
				strcpy(reqd_desc,p_tab_line[i].p_atrb_desc);
				return(0);
		}
	}
	return(1);
}



/*=================================
| Loads print_desc file into array |
=================================*/
void
load_prnt_desc (void)
{
	int	loop = 0;
	int	found_atrb = FALSE;
	int	pos_atrb = 0;
	char	data[81];
	char	tmp_atrb[4];
	char	*lclptr;

	if ((descf = fopen(pdesc_dir,"r")) == 0)
	{
		fprintf(stdout,"Error in %s during (FOPEN)\n\r",pdesc_dir);
		fprintf(stdout,"%s %d\n\r",PNAME,errno);
		fflush(stdout);
		return;
	}

	/*---------------------------------
	| load prnt_desc file into arrays | 
	---------------------------------*/
	lclptr = fgets(data,80,descf);

	while (lclptr)
	{
		/*-----------------------
		| Ignore comments ie. # |
		-----------------------*/
		if (*lclptr == '#')
		{
			lclptr = fgets(data,80,descf);
			continue;
		}

		sprintf(tmp_atrb, "%-3.3s", lclptr);

		/*------------------
		| Look for first " |
		------------------*/
		tptr = lclptr;
		while (*tptr && *tptr != '"')
			tptr++;

		if (*tptr)
		{
			*tptr = '\0';

			found_atrb = FALSE;
			for(pos_atrb = 0; pos_atrb < MAX_PA; pos_atrb++)
				if (!strncmp(p_tab_line[pos_atrb].p_atrb, tmp_atrb, 3))
				{
					found_atrb = TRUE;
					break;
				}

			if (found_atrb)
				add_desc (pos_atrb);
		}
		loop++;
		lclptr = fgets(data,80,descf);
	}
	no_in_desc_tab = loop;
	fclose(descf);
}

/*--------------------------------------
| Add description of attribute to array |
--------------------------------------*/
void
add_desc (
 int                position)
{
	char	*lclptr;

	/*-----------------------------
	| Look for end of description |
	-----------------------------*/
	lclptr = ++tptr;
	while (*tptr && *tptr != '"')
		tptr++;

	if (*tptr)
	{
		*tptr = '\0';

		/*-------------------------------
		| attribute desc is 2nd on line |
		-------------------------------*/
		sprintf(p_tab_line[position].p_atrb_desc, "%-40.40s",lclptr);


		lclptr = ++tptr;
		while (*tptr && *tptr != '(')
			tptr++;

		if (*tptr)
		{
			tptr++;
			if (!strncmp (tptr, "NEEDED", 6))
				strcpy(p_tab_line[position].p_reqd, "YES");
		}
	}
}

/*-------------------------------------------------
| Initialise attribute to those defined in pcap.c |
-------------------------------------------------*/
void
init_p_table (void)
{
	int 	i;

	for (i = 0; i < MAX_PA; i++)
	{
		sprintf(p_tab_line[i].p_atrb, "%-3.3s", patrb[i]);
		sprintf(p_tab_line[i].p_atrb_desc, "%-40.40s", " ");

		if (!strncmp (p_tab_line[i].p_atrb, "EX=", 3))
			strcpy(p_tab_line[i].p_esc_seq, deflt_ex);
		else
			if (!strncmp (p_tab_line[i].p_atrb, "bx=", 3))
				strcpy(p_tab_line[i].p_esc_seq, deflt_box);
			else
				strcpy(p_tab_line[i].p_esc_seq, "");
		sprintf(p_tab_line[i].p_reqd, "%-3.3s", " ");
	}

	load_prnt_desc ();
}


/*==========================================================================
| Load printer  from file in MENUSYS/PRINT if exists or from termcap.cprogs |
==========================================================================*/
void
load_printer (void)
{
	char 	rd_line[81];	
	int	wk_len = 0;

	new_printer = TRUE;

	if ((pcapf = fopen(filename,"r")) == 0)
	{
		sprintf(filename,"%s/termcap.cprogs",directory);
		/*----------------------------------------------
		| Open /usr/LS10.5/BIN/MENUSYS/termcap.cprogs file. |
		----------------------------------------------*/
		if ((pcapf = fopen(filename,"r")) == 0)
		{
			fclose(pcapf);
			return /*(0)*/;
		}
	}

	/*=======================
	| Find printer entry. |
	=======================*/
	wk_len = strlen(local_rec.printer_name);
	sptr = fgets(rd_line,80,pcapf);
	while (sptr != (char *)0 && strncmp(rd_line,local_rec.printer_name,wk_len))
	{
		sptr = fgets(rd_line,80,pcapf);
	}

	if (sptr == (char *)0)
	{
	 	fclose(pcapf);
		return /*(0)*/;
	}

	init_p_table ();   

	new_printer = FALSE;

	/*=================================
	| Read entry into array elements. |
	=================================*/
	while (sptr != (char *)0 && load_line (sptr) != 1)
		sptr = fgets(rd_line,80,pcapf);

	/*=========================================
	| Close file /usr/LS10.5/BIN/MENUSYS/printcap. |
	=========================================*/
	fclose(pcapf);
}

/*===================================================
| Load a line from printer entry and store in array |
===================================================*/
int
load_line (
 char*              line)
{
	int	flag = 0;
	int	loop;
	int	line_len = strlen(line);
	char	tmp_atrb[4];

	if (*(line + line_len - 2) == ':' || line_len == 0)
		flag = 1;/*	return 1 if last line in entry	*/

	while (*(line + line_len) != ':')
		--line_len;
	/*--------------------- 
	| Remove end of line. |
	---------------------*/
	*(line + line_len) = '\0'; 

	while (line_len)
	{
		if (*(line + line_len) == ':' )
		{
			*(line + line_len) = '\0';

			if (!strncmp(line + line_len + 1,"UN=", 3))
				sprintf(tmp_atrb, "%-3.3s", "us=");
			else
				if (!strncmp(line + line_len + 1,"UF=", 3))
					sprintf(tmp_atrb, "%-3.3s", "ue=");
				else
					sprintf(tmp_atrb, "%-3.3s", line + line_len + 1);

			for (loop = 0; loop < MAX_PA; loop++)
			{
				if (!strncmp(p_tab_line[loop].p_atrb, tmp_atrb, 3))
				{
					strcpy(p_tab_line[loop].p_esc_seq, line + line_len + 4);
					break;
				}
			}
		}
		--line_len;
	}
	return(flag);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int                scn)
{
	char	hdng_date[11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		if (scn == 1)	
		{
			clear();
			strcpy(hdng_date,DateToString(TodaysDate()));

			rv_pr(ML(mlUtilsMess077),(132 - strlen(ML(mlUtilsMess077))) / 2,0,1);
			rv_pr(hdng_date,120,0,0);
			disp_help(110);
			box(0,1,132,1); 

		}

		if (scn == 2)
		{
			box(0,20,132,1);
/*
			rv_pr("Change Escape Sequence",54,20,0);
			rv_pr("   Escape Sequence for :",1,21,0);*/
			rv_pr(ML(mlUtilsMess123),54,20,0);
			rv_pr(ML(mlUtilsMess124),1,21,0);
			rv_pr(local_rec.atrb_desc,26,21,1);
		}

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}

/*-------------------------------
| Get escape sequence from user |
-------------------------------*/
void
get_esc_seq (
 int                x,
 int                y,
 char*              mask,
 char*              buf)
{
	char    *sptr = buf;
	char    *dpos = NULL;
	int		i;
	int		bufsize = strlen(mask);
	int     c = 0;
	int     decflg = FALSE;

	/*-------------------------------
	| Redisplay after Redraw	|
	-------------------------------*/
	if (last_char == REDRAW || last_char == SEARCH)
	{
		print_at(y,x,"%s", sptr);

		dpos = strchr (sptr,'.');
		decflg = (dpos != (char *) NULL);
		sptr = buf + strlen(buf);
	}
	else
		*sptr = (char) NULL;

	/*-----------------------
	| print '_' for mask	|
	-----------------------*/
	move(x + strlen(buf),y);
	for (i = strlen(buf);i < bufsize;i++)
		putchar((mask[i] == '.') ? '.' : US);
	fflush (stdout);

	crsr_on();
	/*-------------------------------
	| Move to appropriate position	|
	-------------------------------*/
	move(x + strlen(buf),y);
    while (TRUE)
    {
        c = getkey();
		last_char = c;

		/*-----------------------
		| Perform Upshift	|
		-----------------------*/
		if (c < 2000 && mask[sptr - buf] == 'U')
			c = toupper(c);

		/*-------------------------------
		| At a decimal point		|
		| and input is not valid	|
		-------------------------------*/
		if (mask[sptr - buf] == '.' && !decflg && c != BS && c != '.' && c != REDRAW && c != SEARCH && c != ENDINPUT && c != FN9 && c != FN10 && c != FN11 && c != FN12)
		{
                        putchar(BELL);
			fflush (stdout);
                        continue;
                }

		/*-------------------------------
		| Special key excluding BS	|
		-------------------------------*/
		if ((c >= 2000 || c < ' ') && c != BS)
		{
			/*---------------
			| Search Key	|
			---------------*/
			if (c == FN4 || c == FN9 || c == FN10 || c == FN11 || c == FN12)
			{
				if (!search_ok)
				{
					putchar(BELL);
					fflush (stdout);
					continue;
				}
				search_key = last_char;
				last_char = SEARCH;
			}
			return;
		}

        switch (c)
        {
        case ENDINPUT:
			return;
			break;

        case BS:
            if (sptr == buf)
            {
                putchar(BELL);
                fflush (stdout);
            }
            else
			{
                sptr--;
                *sptr = (char) NULL;
				putchar(BS);
				putchar((mask[sptr - buf] == '.') ? '.' : US);
				putchar(BS);
				fflush (stdout);
            }
            break;

        default:
            if (sptr - buf < bufsize)
			{
				if (strchr (val_2_chars, c) != (char *)0)
				{
					*sptr++ = (char) c;
					*sptr = (char) NULL;
					putchar(c);
				}
				else
				{
					if (c == ':' && (sptr + 4 - buf <= bufsize))
					{
						strcat (sptr, "\\072");
						sptr += 4;
						print_at (y, x+strlen(buf),"\\072");
					}
					else
					{
						putchar(BELL);
					}
				}
			}
			else
				putchar(BELL);
			fflush(stdout);
                        break;
        }
    }
}
