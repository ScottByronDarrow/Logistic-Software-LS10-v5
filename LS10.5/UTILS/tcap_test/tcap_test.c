/*=====================================================================
|  Copyright (C)  1989, 1990 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( tcap_test.c    )                                 |
|  Program Desc  : (Test terminal attributes and function keys      ) |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 16/01/91         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	TCAP
#define CCMAIN
#include <pslscr.h>

char	*PNAME = "$RCSfile: tcap_test.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/tcap_test/tcap_test.c,v 5.2 2001/08/09 09:27:46 scott Exp $";


struct	{
	int	_val;	/* value representing key			*/
	char	*_key;	/* pointer to entry from termcap		*/
} function_key[] = {
	{UP_KEY,	"UP_KEY"},
	{DOWN_KEY,	"DOWN_KEY"},
	{RIGHT_KEY,	"RIGHT_KEY"},
	{LEFT_KEY,	"LEFT_KEY"},
	{HELP,		"HELP"},
	{INSCHAR,	"INS CHAR"},
	{DELCHAR,	"DEL CHAR"},
	{INSLINE,	"INS LINE"},
	{DELLINE,	"DEL LINE"},
	{FN1,		"FN1"},
	{FN2,		"FN2"},
	{FN3,		"FN3"},
	{FN4,		"FN4"},
	{FN5,		"FN5"},
	{FN6,		"FN6"},
	{FN7,		"FN7"},
	{FN8,		"FN8"},
	{FN9,		"FN9"},
	{FN10,		"FN10"},
	{FN11,		"FN11"},
	{FN12,		"FN12"},
	{FN13,		"FN13"},
	{FN14,		"FN14"},
	{FN15,		"FN15"},
	{FN16,		"FN16"},
	{FN17,		"SHIFT FN1"},
	{FN18,		"SHIFT FN2"},
	{FN19,		"SHIFT FN3"},
	{FN20,		"SHIFT FN4"},
	{FN21,		"SHIFT FN5"},
	{FN22,		"SHIFT FN6"},
	{FN23,		"SHIFT FN7"},
	{FN24,		"SHIFT FN8"},
	{FN25,		"SHIFT FN9"},
	{FN26,		"SHIFT FN10"},
	{FN27,		"SHIFT FN11"},
	{FN28,		"SHIFT FN12"},
	{FN29,		"SHIFT FN13"},
	{FN30,		"SHIFT FN14"},
	{FN31,		"SHIFT FN15"},
	{FN32,		"SHIFT FN16"},
	{0}
};

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void test_scr_atrb (void);
void test_key_atrb (void);
int  spec_valid (int field);
int  heading (int scn);

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc,
 char * argv [])
{
	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	clear();
	
	test_scr_atrb();   

	test_key_atrb();   

	shutdown_prog();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	FinishProgram ();
}

void
test_scr_atrb (
 void)
{
	int	i;

	/*--------------------------------
	| Clear screen and move to (0,0) |
	--------------------------------*/
	move (0,0);
	printf("*Screen should now be clear except for this line.Asterisk is in top left corner");
	fflush(stdout);
	sleep(3);

	/*-----------
	| Line test |
	-----------*/
	move(0,2);
	line(43);
	move(0,3);
	printf("There should be a line above this sentence");
	fflush(stdout);
	sleep(3);

	/*---------------------------
	| Clear to end of line test |
	---------------------------*/
	move (0,5);
	printf("In 5 seconds this line should be erased from * here onwards");
	fflush(stdout);
	sleep(5);
	move(47,5);
	cl_line();
	fflush(stdout);
	
	/*-------------------------
	| Clear to end of display |  
	-------------------------*/
	for(i = 8;i < 23; i++)
	{
		move (0,i);
		printf("This will be erased           THIS WILL BE ERASED           This will be erased");
	}
	move (0,7);
	printf("All lines under this will be erased in 5 seconds");
	fflush(stdout);
	sleep(5);
	move (0,8);
	cl_end();
	fflush(stdout);

	sleep(2);
	clear();
	fflush(stdout);

	/*------------------
	| Delete character |
	------------------*/
	move (0,0);
	printf("In 5 seconds the character X  will be deleted");
	fflush(stdout);
	sleep(5);
	move(27,0);
	dl_chr();
	fflush(stdout);
	
	/*-------------
	| Delete line |
	-------------*/
	move (0,2);
	printf("In 5 seconds this line will be deleted");
	fflush(stdout);
	sleep(5);
	move(0,2);
	dl_line();
	fflush(stdout);

	/*---------------
	| Graphics test |
	---------------*/
	move (5,4);
	printf("This sentence is within a box");
	box(4,3,40,1);
	fflush(stdout);
	sleep(2);

	/*---------------------
	| Stand Out Mode Test |
	---------------------*/
	move(0,6);
	so_on();
	printf("This line is in stand out mode");
	so_off();
	fflush(stdout);
	sleep(3);

	/*----------------------
	| Underscore Mode Test |
	----------------------*/
	move(0,8);
	us_on();
	printf("This line is in underscore mode");
	us_off();
	fflush(stdout);
	sleep(3);

	/*-------------------
	| Reverse Mode Test |
	-------------------*/
	move(0,10);
	rv_on();
	printf("This line is in reverse mode");
	rv_off();
	fflush(stdout);
	sleep(3);

	/*------------------------
	| Cursor Off and On Test |
	------------------------*/
	move(0,12);
	printf("The cursor is now off");
	crsr_off();
	fflush(stdout);
	sleep(3);

	move(0,14);
	printf("The cursor is now on");
	crsr_on();
	fflush(stdout);
	sleep(3);

	/*----------------
	| Wide Mode Test |
	----------------*/
	swide();
	move(0,16);
	printf("The screen is now in wide mode");
	fflush(stdout);
	sleep(3);

	/*------------------
	| Normal Mode Test |
	------------------*/
	snorm();
	move(0,18);
	printf("The screen is now in normal mode again");
	fflush(stdout);
	sleep(3);
}

void
test_key_atrb (
 void)
{
	int	i;
	int	key_press;
	int	lcl_val;
	char	lcl_key[14];

	for (i = 0; i < 41; i++)
	{
		lcl_val = function_key[i]._val;
		strcpy(lcl_key, function_key[i]._key);

		clear();
		rv_pr(" KEY TEST ",20,0,1);
		rv_pr(" PRESS 'Q' TO QUIT ",35,0,1);
		move(0,5);
		cl_line();
		printf("Please press %s", lcl_key);
		fflush(stdout);
		key_press = getkey();
			
		if (key_press == 'Q' || key_press == 'q')
		{
			rv_pr(" Quitting..... ", 36, 20, 1);
			sleep(1);
			return;
		}

		if (key_press == lcl_val)
		{
			move(0,10);
			cl_line();
			printf("%s OK", lcl_key);
			fflush(stdout);
		}
		else
		{
			move (0,10);
			cl_line();
			printf(" WARNING : KEY PRESSED DOES NOT MATCH TERMCAP PLEASE RETRY");
			move(0,5);
			cl_line();
			printf("Please press %s", lcl_key);
			fflush(stdout);
			key_press = getkey();
			if (key_press == 'Q' || key_press == 'q')
			{
				rv_pr(" Quitting..... ", 36, 20, 1);
				sleep(1);
				return;
			}
			if (key_press == lcl_val)
			{
				move(0,10);
				cl_line();
				printf("%s OK", lcl_key);
				fflush(stdout);
			}
			else
			{
				move(0,10);
				cl_line();
				printf("WARNING : KEY PRESSED DOES NOT MATCH TERMCAP");
				fflush(stdout);
			}
		}
		sleep(2);
	}
}

int
spec_valid (
 int field)
{
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	return (EXIT_SUCCESS);
}
