#ifndef	LICENSE2_H
#define	LICENSE2_H

#define	MAX_TOKENS	8192

struct	DES_REC
{
	long	user_key;
	int		max_usr;
	int		max_trm;
	long	expiry;
	char	passwd [23];
};

struct	LIC_REC
{
	char	user_name [41];
	char	user_add1 [41];
	char	user_add2 [41];
	char	user_add3 [41];
	char	passwd [23];
	int		max_usr;
	int		max_trm;
	long	expiry;
	char	mch_make [11];		/* narrative field */
	char	mch_modl [11];		/* narrative field */
	char	mch_serl [26];		/* narrative field */

	char	tokens [MAX_TOKENS];	/*	controls number of concurrent users */
};

/*
 * Error codes
*/
#define	LICENSE_OK		0
#define	LICENSE_BAD		-1		/* inode mismatch */
#define	LICENSE_DEAD	-2		/* license expired */
#define	LICENSE_DYING	-3		/* license nearly expired */
#define	LICENSE_OFLOW	-5		/* max users exceeded */

/*
 *	Function prototypes
 */
extern long	lc_i_no (void);
extern int	lc_check (struct DES_REC *, struct LIC_REC *);

extern void	lc_read (struct LIC_REC *),
			lc_write (struct LIC_REC *);

#endif	/*LICENSE2_H*/
