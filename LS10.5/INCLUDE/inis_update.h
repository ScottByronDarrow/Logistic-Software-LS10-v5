/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( inis_update.h )                                  |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Updates files :                                                    |
|---------------------------------------------------------------------|
|  Author        : ?               | Date Written  : ??/??/????       |
|---------------------------------------------------------------------|
|  Date Modified : (18/08/1999)    | Modified by : Eumir Que Camara.  |
|                :                                                    |
|  Comments      :                                                    |
|  (18/08/1999)  : Ported to ANSI standard.                           |
=====================================================================*/

#ifndef __INIS_UPDATE_H__
#define __INIS_UPDATE_H__

/*===============================================================
| Environment Variable "UP_INIS" :								|
| Y(es		- Always Update inis record when fob changed		|
|			- return 1											|
| N(o 		- Never Update inis record when fob changed			|
|			- return 0											|
| P(rompt 	- Prompt on Line by Line Basis						|
|			- return -1											|
===============================================================*/
int
chk_inis (
 void)
{
	char	*sptr = chk_env ("UP_INIS");

	if (sptr == (char *)0)
		return (0);

	switch (*sptr)
	{
	case	'N':
	case	'n':
		return (0);

	case	'Y':
	case	'y':
		return (1);

	case	'P':
	case	'p':
		return (-1);

	default:
		return (0);
	}
}

int
prmpt_inis (
 int _x,
 int _y)
{
	int	i;

	i = prmptmsg ("\007 Update FOB Cost for Supplier ? ","YyNn",_x,_y);
	print_at (_y,_x,"                                    ");
	return (i == 'Y' || i == 'y');
}

#endif /*#ifndef __INIS_UPDATE_H__*/
