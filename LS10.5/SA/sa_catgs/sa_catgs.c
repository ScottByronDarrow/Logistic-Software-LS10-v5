/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_catgs.cpp )                                   |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,    ,      ,     ,     ,     ,     ,         |
|  Database      :                                                    |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      :                                                    |
|---------------------------------------------------------------------|
|  Author        : ?           .   | Date Written  : ?                |
|---------------------------------------------------------------------|
|  Date Modified : (31/08/1999)    | Modified by : Eumir Que Camara.  |
|                                                                     |
|  Comments      :                                                    |
|   (31/08/1999) : Ported to ANSI platform.                           |
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: sa_catgs.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_catgs/sa_catgs.c,v 5.1 2001/08/09 09:16:48 scott Exp $";

#define	CCMAIN
#include <pslscr.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"}
	};

	int comm_no_fields = 3;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
	} comm_rec;

	/*==========================================
	| Sales Analysis Sub Ranges of Categories. |
	==========================================*/
	struct dbview sasr_list[] ={
		{"sasr_co_no"},
		{"sasr_start_cat"},
		{"sasr_end_cat"},
		{"sasr_stat_flag"}
	};

	int sasr_no_fields = 4;

	struct {
		char	sr_co_no[3];
		char	sr_start_cat[12];
		char	sr_end_cat[12];
		char	sr_stat_flag[3];
	} sasr_rec;

#include	"sa_cat.h"
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void p_list (char *curr_cat);

int
main (
 int    argc,
 char*  argv[])
{
	abc_dbopen ("data");

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	load_list();

	curr_ptr = head_ptr;
	print_list();

	p_list("01");
	p_list("04");
	p_list("06");
	p_list("11");

	abc_dbclose ("data");
    return (EXIT_SUCCESS);
}

void
p_list (
 char*  curr_cat)
{
	int		err, ftype = FIRST;
	char	find_cat[12];

	sprintf(find_cat,"%-11.11s",curr_cat);

	err = find_list(find_cat,ftype);

	while (!err)
	{
		printf ("find_list(%s,%s) = %d\n",
			find_cat,
			(ftype == FIRST) ? "FIRST" : "NEXT",
			err);
		ftype = NEXT;
		err = find_list(find_cat,ftype);
	}

	printf ("find_list(%s,%s) = %d\n\n",
		find_cat,
		(ftype == FIRST) ? "FIRST" : "NEXT",
		err);
}
