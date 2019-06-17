/*=====================================================================
|  Copyright (C) 1988 Logistic Software Limited.                      |
|=====================================================================|
|  Program Name  : ( log_error.c    )                                 |
|  Program Desc  : ( Logs All System Errors in programs.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  errs,     ,     ,     ,     ,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (15/02/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (12/09/97)      | Modified  by  : Marnie Organo.   |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define  CCMAIN
char	*PNAME = "$RCSfile: log_error.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/log_error/log_error.c,v 5.1 2001/08/09 09:27:01 scott Exp $";

#include	<pslscr.h>
#include	<stdio.h>
#include	<time.h>
#include	<ml_utils_mess.h>

struct	tm	*ts;

/*----------------------------------------------------------------------
| Set up Array to hold Days of the week used with wday in time struct. |
----------------------------------------------------------------------*/
static char *day[] = {
	"Sunday","Monday","Tuesday","Wednesday",
	"Thursday","Friday","Saturday"
};
/*-------------------------------------------------------------------
| Set up Array to hold Months of Year used with mon in time struct. |
-------------------------------------------------------------------*/
static char *mth[] = {
	"January","February","March","April",
	"May","June","July","August","September",
	"October","November","December"
};

static char *abr[] = {
	"st","nd","rd","th","th","th","th","th","th","th",
	"th","th","th","th","th","th","th","th","th","th",
	"st","nd","rd","th","th","th","th","th","th","th",
	"st"
};

FILE    *stat_out;

int
main (
 int                argc,
 char*              argv[])
{
	char 	err_str[200];
	time_t	tloc	=	time (NULL);

	if (argc != 4 && argc != 5)
	{
		/*printf("Usage : %s <prog_name> <line1> <line2> - optional <prog_version>\007\n",argv[0]);*/
		print_at(0,0,ML(mlUtilsMess703),argv[0]);
        return (EXIT_FAILURE);
	}

	ts = localtime(&tloc);

	if ((stat_out = fopen("ERR.LOG","a")) == NULL)
    {
        return (EXIT_FAILURE);
    }
	strcpy(err_str, "================================================================================\n");
 	fputs(err_str,stat_out);

	sprintf(err_str, "| Error On (%-9.9s %-9.9s %2d%2.2s, %d %14.14s Time : %02d:%02d %2.2s)     |\n",
		day[ts->tm_wday], mth[ts->tm_mon], 
		ts->tm_mday, abr[ts->tm_mday-1], ts->tm_year," ",
		((ts->tm_hour) > 12 ? (ts->tm_hour -12 ) : (ts->tm_hour)),
		ts->tm_min,
		((ts->tm_hour) >= 12 ? "pm" : "am"));

 	fputs(err_str,stat_out);
	strcpy(err_str, "|------------------------------------------------------------------------------|\n");
 	fputs(err_str,stat_out);

	sprintf(err_str,"| Program Name : %-40.40s %21.21s|\n",argv[1]," ");
 	fputs(err_str,stat_out);
	if (argc == 5)
	{
		sprintf(err_str,"| Program Version : %-59.59s|\n",argv[4]);
		fputs(err_str,stat_out);
	}
	sprintf(err_str,"| %-77.77s|\n", argv[2]);
	fputs(err_str,stat_out);
	sprintf(err_str,"| %-77.77s|\n", argv[3]);
	fputs(err_str,stat_out);

	strcpy(err_str, "--------------------------------------------------------------------------------\n");
 	fputs(err_str,stat_out);

	fclose(stat_out);
    return (EXIT_SUCCESS);
}
