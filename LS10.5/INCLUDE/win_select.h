#ifndef	WIN_SELECT_H
#define	WIN_SELECT_H

struct	SEL_STR
{
	struct	SEL_STR	*next;
	struct	SEL_STR	*prev;
	char	sel_name[80];
};
#define	WIN_SEL_NULL	((struct SEL_STR *) NULL)

#endif	
