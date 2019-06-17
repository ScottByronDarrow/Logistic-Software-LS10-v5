/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( ip_print.c     )                                 |
|  Program Desc  : ( Invoice Printing Routines for Counter Invoices ) |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 02/04/90         |
|---------------------------------------------------------------------|
|  Date Modified : (05/03/96)      | Modified  by  : Scott B Darrow.  |
|                                                                     |
|  Comments      : Updated to allow invoice and Delivery dockets to   |
|                : be printed.                                        |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

static	FILE	*ip_pout;

extern	char	err_str[];
	
char	*sv_check(char *_env, char *_prg, char *_co_no, char *_br_no);

void ip_open 	( char	*, char	*, int);
void _ip_open 	( char *co_no, char *, int, char	*, char	*);

/*=============================
| Invoice Print Open Routine. |
=============================*/
void	
ip_open
(
 	char	*co_no,
 	char	*br_no,
 	int		lpno
)
{
	_ip_open (co_no, br_no, lpno, "SO_CTR_INV", "so_ctr_inv");
}
/*=============================
| Invoice Print Open Routine. |
=============================*/
void
_ip_open
(
 	char	*co_no,
 	char	*br_no,
 	int		lpno,
	char	*EnvName,
	char	*ProgName
)
{
	char	run_print[81];

	/*-------------------------------------------
	| first time or Company or Branch Changed	|
	-------------------------------------------*/
	strcpy(run_print,sv_check(EnvName, ProgName, co_no, br_no));

	/*-------------------------------
	| Require to Open the Pipe	|
	-------------------------------*/
	if ((ip_pout = popen(run_print,"w")) == 0)
	{
		sprintf(err_str,"Error in %s during (POPEN)",run_print);
		sys_err(err_str,errno,PNAME);
	}
	fprintf(ip_pout,"%d\n",lpno);
	fprintf(ip_pout,"S\n");
	fflush(ip_pout);
}
		
/*==============================
| Invoice Print Close Routine. |
==============================*/
void
ip_close(void)
{
	fprintf(ip_pout,"0\n");
	fflush(ip_pout);
	pclose(ip_pout);
}

/*========================
| Invoice Print Routine. |
========================*/
void
ip_print(long int ip_hash)
{
	fprintf(ip_pout,"%ld\n", ip_hash);
	fflush(ip_pout);
}

/*============================
| Check for System Variable. |
============================*/
char	*sv_check(char *_env, char *_prg, char *_co_no, char *_br_no)
{
	char	*sptr;
	char	*chk_env();
	char	_run_print[41];

	/*-------------------------------
	| Check Company & Branch	|
	-------------------------------*/
	sprintf(_run_print,"%s%s%s",_env,_co_no,_br_no);
	sptr = chk_env(_run_print);
	if (sptr == (char *)0)
	{
		/*---------------
		| Check Company	|
		---------------*/
		sprintf(_run_print,"%s%s",_env,_co_no);
		sptr = chk_env(_run_print);
		if (sptr == (char *)0)
		{
			sprintf(_run_print,"%s",_env);
			sptr = chk_env(_run_print);
			return((sptr == (char *)0) ? _prg : sptr);
		}
		else
			return(sptr);
	}
	else
		return(sptr);
}
