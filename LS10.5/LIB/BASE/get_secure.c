/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( get_secure.c   )                                 |
|  Program Desc  : ( Maintain SECURE file (User/Co/Br)            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  SECURE,                                           |
|---------------------------------------------------------------------|
|  Updates files :  SECURE,                                           |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 28/07/88         |
|---------------------------------------------------------------------|
|  Date Modified :                 | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

struct sec_type {
	char	_name[9];
	char	_co_no[3];
	char	_br_no[3];
} _sec_rec;

#define	BASE	30000
int	_sec_test[] = {
'S' + BASE, 'E' + BASE, 'C' + BASE, 'U' + BASE, 'R' + BASE, 'E' + BASE, 0
};

int
chk_secure(char *co_no, char *br_no)
{
	int		valid_co = 0;
	char	user_name [9];
	int		fd = open_secure();

	sprintf(user_name,"%-8.8s",getenv("LOGNAME"));

	while (!RF_READ (fd,(char *) &_sec_rec))
	{
		/*-------------------------------
		| Found A record for the User	|
		-------------------------------*/
		if (!strcmp(user_name,_sec_rec._name))
		{
			/*---------------
			| All Companies	|
			---------------*/
			if (!strcmp(_sec_rec._co_no,"  "))
			{
				close_secure(fd);
				return(1);
			}

			/*-----------------------
			| Specific Company	|
			-----------------------*/
			if (!strcmp(co_no,_sec_rec._co_no))
			{
				valid_co = 1;
				/*---------------
				| All Branches	|
				---------------*/
				if (!strcmp(_sec_rec._br_no,"  "))
				{
					close_secure(fd);
					return(1);
				}

				/*-----------------------
				| Specific Branch	|
				-----------------------*/
				if (!strcmp(br_no,_sec_rec._br_no))
				{
					close_secure(fd);
					return(1);
				}
			}
		}
	}

	errno = valid_co;
	close_secure(fd);
	return(0);
}

void
add_secure(int fd, char *user_name, char *co_no, char *br_no)
{
	strcpy(_sec_rec._name,user_name);
	strcpy(_sec_rec._co_no,co_no);
	strcpy(_sec_rec._br_no,br_no);

	if (RF_ADD (fd, (char *) &_sec_rec))
		sys_err("Error in SECURE during (WKADD)",errno,PNAME);
}

int
open_secure(void)
{
	int	err, fd;
	int	i;
	char	*sptr = getenv("PROG_PATH");
	char	filename[100];
	char	secure[7];

	/*-----------------------------------------------
	| decide where to look for environment file	|
	-----------------------------------------------*/
	for (i = 0;i < 6;i++)
		secure[i] = _sec_test[i] - BASE;

	secure[6] = '\0';

	sprintf(filename,"%s/BIN/%-6.6s",(sptr != (char *)0) ? sptr : "/usr/LS10.5",secure);

	/*---------------------------------------
	| If variable file doesn't exist	|
	---------------------------------------*/
	if (access(filename,00) < 0)
	{
		if ((err = RF_OPEN (filename, sizeof (struct sec_type), "w", &fd)))
			sys_err ("Error in SECURE during (WKOPEN)", err, PNAME);

		if ((err = RF_CLOSE (fd)))
			sys_err ("Error in SECURE during (WKCLOSE)", err, PNAME);

		chmod (secure, 0666);
	}

	if ((err = RF_OPEN (filename, sizeof (struct sec_type), "u", &fd)))
		sys_err ("Error in SECURE during (WKOPEN)", err, PNAME);

	return(fd);
}

void
close_secure(int fd)
{
	int	err = RF_CLOSE (fd);

	if (err)
		sys_err ("Error in SECURE during (WKCLOSE)", err, PNAME);
}
