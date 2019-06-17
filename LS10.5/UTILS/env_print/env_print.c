/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( env_print.c    )                                 |
|  Program Desc  : ( Environment Variable Print                   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written : (07/11/88)        |
|---------------------------------------------------------------------|
|  Date Modified : (07/11/88)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (10/07/92)      | Modified  by : Simon Dubey.      |
|  Date Modified : (05/10/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : If env variable PSL_ENV_NAME nominates alternative |
|                : file name it will use instead of LOGISTIC PSL 7410 |
|   (05/10/1999) : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: env_print.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/env_print/env_print.c,v 5.2 2001/08/09 09:26:54 scott Exp $";

#define		NO_SCRGEN
#include	<ml_std_mess.h>
#include	<pslscr.h>
#include	<pinn_env.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>

PinnEnv		env_rec;

char	filename[100];

FILE	*fout;
FILE	*fsort;

/*===========================
| Local function prototypes |
===========================*/
void	process	(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr = getenv("PROG_PATH");
	char	*xptr = getenv("PSL_ENV_NAME");
	time_t	tloc	=	time (NULL);

	if (argc != 2)
	{
		/*printf("Usage : %s <lpno>\007\n\r",argv[0]);*/
		printf(ML(mlStdMess036),argv[0]);
		return (EXIT_FAILURE);
	}


	if ((fout = popen("pformat","w")) == 0)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);

	sprintf(filename,"%s/BIN/LOGISTIC",(sptr != (char *)0) ? sptr : "/usr/LS10.5");
	if (xptr)
		strcpy (filename, xptr);

	fprintf(fout,".START%s\n",ttod());
	fprintf(fout,".LP%d\n",atoi(argv[1]));
	fprintf(fout,".9\n");
	fprintf(fout,".PI12\n");
	fprintf(fout,".L120\n");
	fprintf(fout,".EEnvironment File :\n");
	fprintf(fout,".E%s\n",filename);
	fprintf(fout,".EAs At %-24.24s\n",ctime(&tloc));

	fprintf(fout,".R=================");
	fprintf(fout,"===============================");
	fprintf(fout,"=======================================================================\n");

	fprintf(fout,"=================");
	fprintf(fout,"===============================");
	fprintf(fout,"=======================================================================\n");

	fprintf(fout,"| Variable Name |");
	fprintf(fout,"  Variable Value              |");
	fprintf(fout,"  Description of Variable                                             |\n");

	fprintf(fout,"|---------------|");
	fprintf(fout,"------------------------------|");
	fprintf(fout,"----------------------------------------------------------------------|\n");

	dsp_screen("Printing Environment File"," 1",get_env("COPYRIGHT"));

	process();

	fprintf(fout,".EOF\n");
	pclose(fout);
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
process (
 void)
{
	int	env_fd = open_env();
	char	*sptr;
	char	buffer[121];

	fsort = sort_open("env");

	cc = RF_READ(env_fd, (char *) &env_rec);

	while (!cc)
	{
		dsp_process("Read : ",env_rec.env_name);
		sprintf(buffer,"|%s|%s|%s|\n",env_rec.env_name,env_rec.env_value,env_rec.env_desc);
		sort_save(fsort,buffer);
		cc = RF_READ(env_fd, (char *) &env_rec);
	}

	close_env(env_fd);

	fsort = sort_sort(fsort,"env");

	sptr = sort_read(fsort);

	while (sptr != (char *)0)
	{
		dsp_process("Print : ",env_rec.env_name);
		fprintf(fout,"%s\n",sptr);
		sptr = sort_read(fsort);
	}

	sort_delete(fsort,"env");
}
