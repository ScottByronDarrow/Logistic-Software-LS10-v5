/*=====================================================================
|  Copyright (C) 1989 - 1991 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( pcap_test.c    )                                 |
|  Program Desc  : ( Test printer attributes                      )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,               |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 17/01/91         |
|---------------------------------------------------------------------|
|  Date Modified : (        )      | Modified  by  :                  |
|                :                                                    |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
=====================================================================*/
#define CCMAIN

char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/pcap_test/pcap_test.c,v 5.3 2002/07/17 09:58:18 scott Exp $";
char	*PNAME = "$RCSfile: pcap_test.c,v $";

#include <pslscr.h>
#include <get_lpno.h>

int	lpno;

FILE	*fout;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
int spec_valid (int field);
int heading (int scn);

int
main (
 int                argc,
 char*              argv[])
{

	init_scr();
	set_tty();
	clear();

	lpno = get_lpno (0);
	move(0,20);
	/*---------------------------------------
	| Open pipe to standard print .		|
	---------------------------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)",errno,PNAME);

	/*--------------------
	| Initialise Printer |
	--------------------*/
	fprintf(fout,".START00/00/00\n");
	fprintf(fout,".LP%d\n",lpno);
	fprintf(fout,".PL66\n");
	fprintf(fout,".2\n");
	fprintf(fout,".L80\n");
	fprintf(fout,".PI10\n");
	fflush(fout);


	/*---------------------
	| Test Expanded print |
	---------------------*/
	fprintf(fout, ".B2\n");
	fprintf(fout, ".E This line should be in expanded print \n");
	fflush(fout);

	/*---------------
	| Test 10 Pitch |
	---------------*/
	fprintf(fout, ".B2\n");
	fprintf(fout, ".PI10\n");
	fprintf(fout, "This line should be printed in 10 pitch\n");
	fflush(fout);
	
	/*---------------
	| Test 12 Pitch |
	---------------*/
	fprintf(fout, ".B2\n");
	fprintf(fout, ".PI12\n");
	fprintf(fout, "This line should be printed in 12 pitch\n");
	fflush(fout);

	/*-----------------
	| Test Bold Print |
	-----------------*/
	fprintf(fout, ".B2\n");
	fprintf(fout, "^3 This line should be printed in bold print ^8\n");
	fflush(fout);

	/*-----------------
	| Test underscore |
	-----------------*/
	fprintf(fout, ".B2\n");
	fprintf(fout, "^2 This line should be underscored ^7\n");
	fflush(fout);
	
	/*---------------
	| Test Graphics |
	---------------*/
	fprintf(fout, ".B2\n");
	fprintf(fout, "^^AGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGB^^\n");
	fprintf(fout, "^E This line should be enclosed in a box ^E\n");
	fprintf(fout, "^^CGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGD^^\n");
	fflush(fout);

	/*----------
	| Test TOF |
	----------*/
	fprintf(fout, ".PA\n");
	fflush(fout);
	fprintf(fout, "This line should be printed at TOP OF PAGE\n");
	fflush(fout);

	fprintf(fout, ".PA\n");
	fprintf(fout,".EOF\n");
	fflush(fout);
	pclose(fout);
	shutdown_prog ();	
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	FinishProgram ();
}

int
spec_valid (
 int                field)
{
    return (EXIT_SUCCESS);
}

int
heading (
 int                scn)
{
    return (EXIT_SUCCESS);
}
