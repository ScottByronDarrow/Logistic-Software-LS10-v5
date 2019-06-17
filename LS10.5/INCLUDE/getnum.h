#ifndef	GETNUM_H
#define	GETNUM_H

extern double	getnum		(int, int, int, char *);
extern int		getint		(int, int, char *);
extern long		getlong 	(int, int, char *);
extern float	getfloat	(int, int, char *);
extern double	getdouble	(int, int, char *);
extern double	getmoney	(int, int, char *);

extern void		getalpha	(int, int, char *, char *),
				getalpha2	(int, int, char *, char *),
				get_date	(int, int, char *, char *);

#endif	/*GETNUM_H*/
