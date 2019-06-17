/*=====================================================================
|  Copyright (C) 1989, 1990 Logistic Software Limited.                |
|=====================================================================|
|  Program Name  : ( psl_ls.c       )                                 |
|  Program Desc  : ( Logistic Software version of ls.             )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  : Lots.                                              |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :                                                    |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 29/11/90         |
|---------------------------------------------------------------------|
|  Date Modified : (29/11/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (12/09/97)      | Modified  by  : Roanna Marcelino |
|                                                                     |
|  Comments      : (29/11/90) -                                       |
|                : (12/09/97) -  MOdified for Multilingual Conversion.|
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_ls.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_ls/psl_ls.c,v 5.4 2002/07/17 09:58:19 scott Exp $";

int	x_off,
	y_off;
#define	X_OFF	x_off
#define	Y_OFF	y_off

#include	<pslscr.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<dsp_process.h>
#include	<ml_std_mess.h>
#include	<ml_utils_mess.h>

#ifdef	XENIX
#	include	<sys/dir.h>
#else
#	include	<dirent.h>
#endif

#define	PSL_DIRSIZ	14

#include	<get_lpno.h>

#define	BUFSIZE	512
char	buf[BUFSIZE];

char	disp_str[200];

int	lpno;

struct	LNK_FDE
{
	long	d_ino;
	char	d_name[PSL_DIRSIZ + 1];
	struct	LNK_FDE	*_next;
};

#define	NUL_LNK	((struct LNK_FDE *) NULL)

struct	passwd	*pwd;		
struct	group	*grp;
struct	tm	*tme;		
struct	stat	stbuf;
struct
{
	char	dummy	[11];
	char	dir		[256];
	char	sub_dir	[2];
	char	opdev 	[3];
} local_rec;

static struct	var vars[] =
{
	{1, LIN, "dir", 3, 15,
		CHARTYPE, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
		"          ", " ", "/", "Dir : ",
		" Default = / ", YES, NO, JUSTLEFT,
		"", "", local_rec.dir},
	{1, LIN, "sub", 4, 15,
		CHARTYPE, "U",
		"          ", " ", "N", "Sub Dir's",
		" Enter Y(es) or N(o) Default = N(o) ", YES, NO, JUSTLEFT,
		"YN", "", local_rec.sub_dir},
	{1, LIN, "opdev", 5, 15,
		CHARTYPE, "U", "          ", " ", "S", 
		"Output to: ",
		" S(creen) or P(rinter) Default = S(creen)", YES, NO, JUSTLEFT,
		"SP", "", local_rec.opdev},
	{1, LIN, "lpno", 6, 15,
		INTTYPE, "NN",
		"          ", "", "1", "Printer : ",
		"", YES, NO, JUSTRIGHT,
		"", "", (char *) &lpno},
	{0, LIN, "", 0, 0,
		CHARTYPE, "A",
		"          ", " ", "", " ",
		" ", NA, NO, JUSTLEFT,
		"", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
int heading (int scn);
int spec_valid (int field);
void process (void);
void proc_file (char *name);
void make_perms (ushort src, char *dst);
void proc_dir (char *name);
struct LNK_FDE *fde_alloc (void);


int
main (
 int                argc,
 char*              argv[])
{
	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 

	set_masks ();

	swide ();

	while (!prog_exit)
	{
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		x_off = 0;
		y_off = 8;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		process ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	FinishProgram ();
}

int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML(mlUtilsMess113), 56, 0, 1);

		line_at (1,0,132);
		box (0, 2, 132, 4);

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("dir"))
	{
		strcpy (buf, clip (local_rec.dir));
		if (stat (buf, &stbuf) == -1) 
		{
			print_mess (ML(mlStdMess241));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("opdev"))
	{
		if (local_rec.opdev[0] == 'S')
			FLD ("lpno") = NA;
		else
			FLD ("lpno") = YES;
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
			lpno = get_lpno (0);

		if (!valid_lp(lpno))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
process (void)
{
	x_off = 0;
	y_off = 0;

	sprintf (disp_str, "Listing of %s%s",
		local_rec.dir,
		(local_rec.sub_dir[0] == 'Y') ? " and subdirectories." : "");
	if (local_rec.opdev[0] == 'S')
		Dsp_prn_open (0, 0, 18, disp_str, (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);
	else
	{
		clear ();
		snorm ();
		box (0, 0, 80, 21);
		Dsp_nd_prn_open (0, 0, 18, disp_str, (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);
	}

	Dsp_saverec ("   FILE NAME    | PERMISSION |  BYTES.  |   DATE   | TIME  |   OWNER  |  GROUP   |LINKS| INODE # ");
	Dsp_saverec ("");
	Dsp_saverec ("  [FN03]   [FN05]   [FN14]   [FN15]   [FN16]  ");

	strcpy (buf, clip (local_rec.dir));

	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) 
		proc_dir (buf);
	else
		proc_file (buf);

	if (local_rec.opdev[0] == 'S')
		Dsp_srch ();
	else
	{
		Dsp_print ();
		clear ();
		swide ();
	}
	Dsp_close ();
}

void
proc_file (
 char*              name)
{
	char	oname[20],
		gname[20],
		*tbuf,
		perms[11],
		wk_date[11],
		wk_time[6];

	if (local_rec.opdev[0] != 'S')
		dsp_process (name, "  ");

	if ((pwd = getpwuid (stbuf.st_uid)) == 0) 
		strcpy (oname, "???");
	else 
		strcpy (oname, pwd->pw_name);

	if ((grp = getgrgid (stbuf.st_gid)) == 0) 
		strcpy (gname, "???");
	else 
		strcpy (gname, grp->gr_name);

	tbuf = strrchr (name, '/');
	if (!tbuf)
		tbuf = name;
	else
		tbuf++;

	tme = localtime (&stbuf.st_ctime);

	if (FullYear ())
	{
		sprintf (wk_date, "%02d/%02d/%04d",
			tme->tm_mday,
			tme->tm_mon + 1,
			tme->tm_year + 1900);
	}
	else
	{
		sprintf (wk_date, "%02d/%02d/%02d",
			tme->tm_mday,
			tme->tm_mon + 1,
			tme->tm_year);
	}

	sprintf (wk_time, "%02d:%02d",
		tme->tm_hour,
		tme->tm_min);

	make_perms (stbuf.st_mode, perms);

	sprintf (disp_str, " %-14.14s ^E %-10.10s ^E %8ld ^E%10.10s^E %5.5s ^E%10.10s^E%10.10s^E %3d ^E%8ld ^E",
		tbuf,
		perms,
		stbuf.st_size,
		wk_date,
		wk_time,
		oname,
		gname,
		stbuf.st_nlink,
		stbuf.st_ino);

	Dsp_saverec (disp_str);
}

void
make_perms (
 ushort             src,
 char*              dst)
{
	strcpy (dst, "----------");
	switch (src & S_IFMT)
	{
	case	S_IFIFO:
		*dst = 'p';
		break;

	case	S_IFCHR:
		*dst = 'c';
		break;

	case	S_IFDIR:
		*dst = 'd';
		break;

#ifndef	SCO
	case	S_IFLNK:
		*dst = 'l';
		return;
#endif

	case	S_IFBLK:
		*dst = 'b';
		break;
	}

	if (src & S_IRUSR)
		*(dst + 1) = 'r';

	if (src & S_IWUSR)
		*(dst + 2) = 'w';

	if (src & S_IXUSR)
		*(dst + 3) = 'x';

	if (src & S_IRGRP)
		*(dst + 4) = 'r';

	if (src & S_IWGRP)
		*(dst + 5) = 'w';

	if (src & S_IXGRP)
		*(dst + 6) = 'x';

	if (src & S_IROTH)
		*(dst + 7) = 'r';

	if (src & S_IWOTH)
		*(dst + 8) = 'w';

	if (src & S_IXOTH)
		*(dst + 9) = 'x';

	if (src & S_ISUID)
	{
		if (*(dst + 3) == '-')
			*(dst + 3) = 'S';
		else
			*(dst + 3) = 's';
	}

	if (src & S_ISGID)
	{
		if (*(dst + 6) == '-')
			*(dst + 6) = 'S';
		else
			*(dst + 6) = 's';
	}

	if (src & S_ISVTX)
	{
		if (*(dst + 9) == '-')
			*(dst + 9) = 'T';
		else
			*(dst + 9) = 't';
	}
}

#ifdef	XENIX
void
proc_dir (
 char*              name)
{
	struct	LNK_FDE	*head_fde = NUL_LNK;
	struct	LNK_FDE	*curr_fde = NUL_LNK;
	struct	LNK_FDE	*temp_fde = NUL_LNK;
	struct	LNK_FDE	*fde_alloc();
	struct	direct	dirbuf;
	char	*nbp,
		*nep,
		lcl_buf[BUFSIZE];
	int	i,
		fd;

	if (local_rec.sub_dir[0] == 'Y')
	{
		sprintf (disp_str, "^1%-97.97s^6", name);
		Dsp_saverec (disp_str);
	}

	strcpy (lcl_buf, name);
	nbp = lcl_buf + strlen (lcl_buf);
	*nbp++ = '/';		/* Add slash to dir name */
	*nbp = 0;		/* Don't forget end of string */

	if (nbp+PSL_DIRSIZ+2 >= lcl_buf+BUFSIZE) /* Name too long */
		return;

	if ((fd = open (name, 0)) == -1)
		return;

	while (read (fd, (char *) &dirbuf, sizeof (dirbuf)) > 0) 
	{
		if (dirbuf.d_ino == 0)	/* Deleted filename */
			continue;

		if ( strcmp (dirbuf.d_name, "." ) == NULL || 
		     strcmp (dirbuf.d_name, "..") == NULL)
			continue;	/* Skip self and parent directories */

		temp_fde = fde_alloc ();
		temp_fde->d_ino = dirbuf.d_ino;
		sprintf (temp_fde->d_name, "%-14.14s", dirbuf.d_name);

		if (head_fde == NUL_LNK)
		    head_fde = temp_fde;
		else
		    if (strcmp (dirbuf.d_name, head_fde->d_name) < 0)
		    {
			temp_fde->_next = head_fde;
			head_fde = temp_fde;
		    }
		    else
		    {
			for (curr_fde = head_fde; curr_fde != NUL_LNK; curr_fde = curr_fde->_next)
			{
			    if (curr_fde->_next == NUL_LNK)
			    {
				curr_fde->_next = temp_fde;
				break;
			    }
			    if (strcmp (dirbuf.d_name, curr_fde->_next->d_name) < 0)
			    {
				temp_fde->_next = curr_fde->_next;
				curr_fde->_next = temp_fde;
				break;
			    }
			}
		    }
	}

	close (fd);

	for (curr_fde = head_fde; curr_fde != NUL_LNK; curr_fde = curr_fde->_next)
	{
		for (i = 0, nep = nbp; i < PSL_DIRSIZ; i++)
			*nep++ = curr_fde->d_name[i];

		*nep++ = '\0';
		clip (lcl_buf);
		if (stat (lcl_buf, &stbuf) != -1) 
			proc_file (lcl_buf);
	}

	temp_fde = NUL_LNK;

	for (curr_fde = head_fde; curr_fde != NUL_LNK; curr_fde = curr_fde->_next)
	{
		if (temp_fde != NUL_LNK)
			free (temp_fde);

		for (i = 0, nep = nbp; i < PSL_DIRSIZ; i++)
			*nep++ = curr_fde->d_name[i];

		*nep++ = '\0';
		clip (lcl_buf);
		if (stat (lcl_buf, &stbuf) != -1) 
			if ((stbuf.st_mode & S_IFMT) == S_IFDIR) 
				if (local_rec.sub_dir[0] == 'Y')
					proc_dir (lcl_buf);

		temp_fde = curr_fde;
	}

	if (temp_fde != NUL_LNK)
		free (temp_fde);
}

#else

void
proc_dir (
 char*              name)
{
	struct	LNK_FDE	*head_fde = NUL_LNK;
	struct	LNK_FDE	*curr_fde = NUL_LNK;
	struct	LNK_FDE	*temp_fde = NUL_LNK;
	char	*nbp,
		*nep,
		lcl_buf[BUFSIZE];
	int	i;
	DIR	*fd;
	struct	dirent *dirbuf;

	if (local_rec.sub_dir[0] == 'Y')
	{
		sprintf (disp_str, "^1%-97.97s^6", name);
		Dsp_saverec (disp_str);
	}

	strcpy (lcl_buf, name);
	nbp = lcl_buf + strlen (lcl_buf);
	*nbp++ = '/';		/* Add slash to dir name */
	*nbp = 0;		/* Don't forget end of string */

	if (nbp+PSL_DIRSIZ+2 >= lcl_buf+BUFSIZE) /* Name too long */
		return;

	if ((fd = opendir (name)) == (DIR *) 0)
		return;

	while ((dirbuf = readdir (fd)) != (struct dirent *) 0)
	{
		if (dirbuf->d_ino == 0)	/* Deleted filename */
			continue;

		if ( strcmp (dirbuf->d_name, "." ) == (char) NULL || 
		     strcmp (dirbuf->d_name, "..") == (char) NULL)
			continue;	/* Skip self and parent directories */

		temp_fde = fde_alloc ();
		temp_fde->d_ino = dirbuf->d_ino;
		sprintf (temp_fde->d_name, "%-14.14s", dirbuf->d_name);

		if (head_fde == NUL_LNK)
		    head_fde = temp_fde;
		else
		    if (strcmp (dirbuf->d_name, head_fde->d_name) < 0)
		    {
			temp_fde->_next = head_fde;
			head_fde = temp_fde;
		    }
		    else
		    {
			for (curr_fde = head_fde; curr_fde != NUL_LNK; curr_fde = curr_fde->_next)
			{
			    if (curr_fde->_next == NUL_LNK)
			    {
				curr_fde->_next = temp_fde;
				break;
			    }
			    if (strcmp (dirbuf->d_name, curr_fde->_next->d_name) < 0)
			    {
				temp_fde->_next = curr_fde->_next;
				curr_fde->_next = temp_fde;
				break;
			    }
			}
		    }
	}

	closedir (fd);

	for (curr_fde = head_fde; curr_fde != NUL_LNK; curr_fde = curr_fde->_next)
	{
		for (i = 0, nep = nbp; i < PSL_DIRSIZ; i++)
			*nep++ = curr_fde->d_name[i];

		*nep++ = '\0';
		clip (lcl_buf);
		if (stat (lcl_buf, &stbuf) != -1) 
			proc_file (lcl_buf);
	}

	temp_fde = NUL_LNK;

	for (curr_fde = head_fde; curr_fde != NUL_LNK; curr_fde = curr_fde->_next)
	{
		if (temp_fde != NUL_LNK)
			free (temp_fde);

		for (i = 0, nep = nbp; i < PSL_DIRSIZ; i++)
			*nep++ = curr_fde->d_name[i];

		*nep++ = '\0';
		clip (lcl_buf);
		if (stat (lcl_buf, &stbuf) != -1) 
			if ((stbuf.st_mode & S_IFMT) == S_IFDIR) 
				if (local_rec.sub_dir[0] == 'Y')
					proc_dir (clip (lcl_buf));

		temp_fde = curr_fde;
	}

	if (temp_fde != NUL_LNK)
		free (temp_fde);
}
#endif

struct	LNK_FDE *
fde_alloc (void)
{
	struct	LNK_FDE	*lcl_fde;

	lcl_fde = (struct LNK_FDE *) malloc (sizeof (struct LNK_FDE));
	if (lcl_fde == NUL_LNK)
		sys_err ("Error in fde_alloc During (MALLOC)", errno, PNAME);

	lcl_fde->_next = NUL_LNK;
	return (lcl_fde);
}
