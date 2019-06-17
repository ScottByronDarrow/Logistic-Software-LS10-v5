/*=====================================================================
|  Copyright (C) 1986 - 2000 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( pause_key.c  )                                   |
|  Program Desc  : (Display a string of text and wait for a         ) |
|                  (specified key before continuing                 ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (28/10/1998)    | Author      : Campbell Mander    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include <std_decs.h>

/*============================================
| Pause until the user hits a key.           |
| If waitKey is zero then any key causes the |
|  function to return.                       |
| If waitKey is non-zero then wait for that  |
|  particular key before returning.          |
============================================*/
int
PauseForKey (
 int	yRow,
 int	xCol,
 char *	waitText,
 int	waitKey)
{
	int		keyPressed;

	/*--------------------
	| Display the prompt |
	--------------------*/
	print_at (yRow, xCol, waitText);

	/*------------------------------
	| Wait for a key from the user |
	------------------------------*/
	keyPressed = getkey ();

	if (waitKey != 0)
	{
		while (keyPressed != waitKey)
		keyPressed = getkey ();
	}
	return (keyPressed);
}
