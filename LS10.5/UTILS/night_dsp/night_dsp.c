/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( night_dsp.c    )                                 |
|  Program Desc  : ( Display Programs In Night Processing Stream. )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
|  Date Modified : (14/03/90)      | Modified  by : Scott B. Darrow.  |
|  Date Modified : (26/01/93)      | Modified  by : Trevor van Bremen |
|                                                                     |
|  Comments      :                                                    |
|  (26/01/93)    : Changed name/locn. of $PROG_PATH/BIN/night_reports |
|                : to $PROG_PATH/BIN/SCRIPT/night_repts.sh            |
|                : 600MAC 8407.                                       |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: night_dsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/night_dsp/night_dsp.c,v 5.2 2001/08/09 09:27:05 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include	<ml_utils_mess.h>
#include	<ml_std_mess.h>

#define	LCL_MAX_LINES	18
#define	SCN_OFFSET	3

	int	cnt = 0, 
   		c,
		rc,
		i;

	struct
	{
		char	prog_desc[61];
		char	run_prog[160];
		int	remve;
	} night[51];

	char	file_name[128];
	FILE 	*stat_out,
            *nfile;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int Get_desc (void);
void proc_key (void);
void up_key (void);
void down_key (void);
void remve (void);
int save_file (void);
void print_key (void);
void shutdown_prog (void);
char *strip_end (char *s);

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int                argc,
 char*              argv[])
{
	char    *sptr = getenv ("PROG_PATH");

	sprintf (file_name, "%s/BIN/SCRIPT/night_repts.sh", (sptr) ? sptr : "/usr/LS10.5");
	init_scr();
	set_tty();

	clear();
	crsr_off();
	box(0,0,79,22);
	rv_on (); /*print_at(23,1," Night Processing Reports Display ");*/
	print_at(23,1,ML(mlUtilsMess072));
    rv_off ();
	move(1,2);  line(78);
	move(1,21); line(78);
	print_key ();
	if (Get_desc () == -1)
    {
        return (EXIT_FAILURE);
    }
    else if (Get_desc () == 1)
    {
        return (EXIT_SUCCESS);
    }
	for (i = 0 ; i < cnt; i++)
	{
		sprintf(err_str, "%-60.60s - %s",night[i].prog_desc, 
			          (!night[ i ].remve) ? "LEAVE " : "REMOVE");

		rv_pr(err_str, 3, i + SCN_OFFSET, FALSE);
	}
	
	i = 0;
	sprintf(err_str, "%-60.60s - %s",night[i].prog_desc, 
		          (!night[ i ].remve) ? "LEAVE " : "REMOVE");

	rv_pr(err_str, 3, i + SCN_OFFSET, TRUE);

	proc_key ();

	if (save_file () == 1)
    {
        return (EXIT_FAILURE);
    }

	clear();
	rv_on ();
	/*print_at(0,0,"Night Processing File Unchanged ... ");*/
	print_at(0,0,ML(mlUtilsMess073));
	rv_off ();
	fflush(stdout);
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*==============================================================
| Read night_repts.sh file and process valid lines, Hopefully. |
==============================================================*/
int
Get_desc (void)
{
	char	work_line[160];
	int	j;

	cnt = 0;

	for (j = 0; j < LCL_MAX_LINES; j++);
	{
		sprintf(night[ j ].prog_desc, "%-60.60s", " ");
		night[ j ].remve = FALSE;
	}
	/*----------------------------------------
	| Get night title and options from file . |
 	----------------------------------------*/
	if ((nfile = fopen(file_name,"r")) == NULL)
    {
		shutdown_prog ();
        return (-1);
    }

	while (!feof(nfile))
	{
	    	fgets(work_line,159,nfile);
		if ( work_line[0] == '#' )
		{
	    		strcpy(night[cnt].prog_desc,strip_end(work_line + 2));
			if (strlen(work_line) > 1)
			{
				night[cnt].remve = FALSE;

	    			fgets(work_line,159,nfile);
	    			strcpy(night[cnt++].run_prog,strip_end(work_line));
			}
		}
	}
	if (cnt < 1)
	{
		rv_on ();
		/*print_at(11,18," There Is No Programs In The Night Stream. ");*/
		print_at(11,18,ML(mlUtilsMess074));
		rv_off ();
		fflush(stdout);
		sleep(3);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}
	if (cnt >= LCL_MAX_LINES)
		cnt = LCL_MAX_LINES;

	fclose(nfile);
    return (EXIT_SUCCESS);
}

/*===============
| Process keys. |
===============*/
void
proc_key (void)
{
    while((c = getkey()) != FN16 && c != FN1)
    {
	switch (c)
	{
	/*--------------------------
	| A Space bar was pressed. |
	--------------------------*/
	case ' ' :
	    	down_key ();
	    	break;

	/*---------------------------------
	| A down arrow (lf) was pressed	. |
	---------------------------------*/
	case 10 :
	    	down_key ();
	    	break;

	/*-------------------------
	| Back Space Was Pressed. |
	-------------------------*/
	case '\b' :
	    	up_key ();
	    	break;

	/*------------------------
	| Up Arrow Was Pressed. |
	------------------------*/
	case 11 :
	    	up_key ();
	    	break;

	/*---------------------
	| Return was pressed. |
	---------------------*/
	case '\r' :
	    	remve ();
	    	break;

	/*-----------------------
	| Up Arrow Was Pressed. |
	-----------------------*/
	case UP_KEY :
		up_key ();
	        break;

	/*-------------------------
	| Down Arrow Was Pressed. |
	-------------------------*/
	case DOWN_KEY :
		down_key ();
	        break;

	default :
		putchar(BELL);

	    break;
	}	/* end of switch on input	*/
    }	/* end of while input	*/
}

/*=========================================
| Routine handles a up key being pressed. |
=========================================*/
void
up_key (void)
{
	sprintf(err_str, "%-60.60s - %s",night[i].prog_desc, 
		          (!night[ i ].remve) ? "LEAVE " : "REMOVE");

	rv_pr(err_str, 3, i + SCN_OFFSET, FALSE);

	i -= 1;
	i = (i >= 0) ? i : cnt - 1;

	sprintf(err_str, "%-60.60s - %s",night[i].prog_desc, 
		          (!night[ i ].remve) ? "LEAVE " : "REMOVE");

	rv_pr(err_str, 3, i + SCN_OFFSET, TRUE);
}

/*===========================================
| Routine handles a down key being pressed. |
===========================================*/
void
down_key (void)
{
	sprintf(err_str, "%-60.60s - %s",night[i].prog_desc, 
		          (!night[ i ].remve) ? "LEAVE " : "REMOVE");

	rv_pr(err_str, 3, i + SCN_OFFSET, FALSE);

	i++;
	i = ( i <= cnt - 1 ) ? i : 0;

	sprintf(err_str, "%-60.60s - %s",night[i].prog_desc, 
		          (!night[ i ].remve) ? "LEAVE " : "REMOVE");

	rv_pr(err_str, 3, i + SCN_OFFSET, TRUE);
}
/*===========================
| Flag lines to be removed. |
===========================*/
void
remve (void)
{
	int	j;

	night[ i ].remve = (night[ i ].remve) ? FALSE : TRUE;

	for (j = 0 ; j < cnt; j++)
	{
		sprintf(err_str, "%-60.60s - %s",night[j].prog_desc, 
		          	(!night[ j ].remve) ? "LEAVE " : "REMOVE");

		rv_pr(err_str, 3, j + SCN_OFFSET, (i == j) ? TRUE : FALSE);
	}
}

/*=====================================
| Save file and hopfully fix up crap. |
=====================================*/
int
save_file (void)
{
	int	sv_cnt;

	clear();
	rv_on ();
	/*print_at(0,0,"Please Wait Saving Night Processing File. ");*/
	print_at(0,0,ML(mlStdMess035));
	rv_off ();
	fflush(stdout);
	/*-------------------------------------------------
	| Open Night Reports File and add to end of file. |
	-------------------------------------------------*/
	if ((stat_out = fopen(file_name,"w")) == NULL)
		return (EXIT_SUCCESS);

	for (sv_cnt = 0 ; sv_cnt < cnt; sv_cnt++)
	{
		if ( !night[ sv_cnt ].remve )
		{
			/*-------------------------------------------------
			| Send Program Description To First Line of file. |
			-------------------------------------------------*/
			fputs("# ", stat_out);
	
			fputs(night[ sv_cnt ].prog_desc, stat_out);
	
			/*----------------------
			| Send a New Line Out. |
			----------------------*/
			fputs("\n", stat_out);
	
			fputs(night[ sv_cnt ].run_prog, stat_out);
			fputs("\n", stat_out);
		}
	}
	/*----------------------------
	| Close Night Reports File . |
	----------------------------*/
	fclose(stat_out);
    return (EXIT_SUCCESS);
}

void
print_key (void)
{
   	rv_pr("[F16-End Input.]        [Arrows- Move.]      [Return-Remove/Leave Line.]",2,22,1);
}
/*========================
| program exit sequence	.|
========================*/
void
shutdown_prog (void)
{
	crsr_on();
	rset_tty();
}
/*=============================
| Strip rubbish off tail end. |
=============================*/
char*
strip_end (
 char*              s)
{
	char	*sptr = (s + strlen(s) - 1);

	for (;s>=s && (*sptr == ' ' || *sptr == '\t' || *sptr == '\n');sptr--);
	*(sptr + 1) = '\0';
	return( s );
}
