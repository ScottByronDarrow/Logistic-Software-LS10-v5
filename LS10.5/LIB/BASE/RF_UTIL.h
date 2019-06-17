#ifndef	WKUTIL_H
#define	WKUTIL_H

#define	MAX_OPEN	5	/* size of fd_list */

struct wkfile
{
	char	*fd_name;	/* Contains file name.			*/
	int		fd_rsiz,	/* Record size in bytes.		*/
			fd_mode,	/* File mode - 1 of r,w,a,u		*/
			fd_fdesc;	/* File descriptor for this file.	*/
};

extern struct wkfile	fd_list [];

#endif	/*WKUTIL_H*/
