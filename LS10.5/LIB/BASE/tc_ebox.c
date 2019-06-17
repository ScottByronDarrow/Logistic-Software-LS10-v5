/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( tc_ebox.c      )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>
/*=======================
| Erase a graphics Box. |
=======================*/
void
erase_box(int x, int y, int h, int v)
{
	int	i;
	int	j;
	int	cnt;

	if (h > 1)
	{
		move(x,y);
		for (cnt = 0; cnt < h + 2; cnt++)
			putchar(' ');
		i = y+1;
		j = v;
	}
	else
	{
		i = y;
		j = v + 2;
	}

	while (j--)
	{
		move(x,i);
		putchar(' ');
		if (h > 1)
		{
			move(x+h-1,i++);
			putchar(' ');
		}
		else
			i++;
	}

	if (h > 1)
	{
		move(x,i);
		for (cnt = 0; cnt < h + 2; cnt++)
			putchar(' ');
	}
}
