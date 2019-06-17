/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( Sub_menu.h     )                                 |
|  Program Desc  : ( Sub Menu Maintenance Routines.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  SUB_MENU/SUB_MENU                                 |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  SUB_MENU/SUB_MENU                                 |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 05/09/88         |
|---------------------------------------------------------------------|
|  Date Modified : (05/09/88)      | Modified  by  : Roger Gibbison.  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
char	*get_sub (char *);
int 	open_sub (char *);
int		add_sub (int, char *, char *);

/*=======================
| Sub Menu Linkage	|
=======================*/
struct	sub_type	{
	char	_prog_name[15];	/* Program Name ie menu.c		*/
	char	_menu_name[15];	/* Menu Data File ie db_master.mdf	*/
} sub_rec;

char	*get_sub (char *prog_name)
{
	int		fd;
	char	_p_name[15];
	static	char	_m_name[25];

	fd = open_sub("r");

	sprintf(_p_name,"%-14.14s",prog_name);

	while (!RF_READ (fd, (char *) &sub_rec))
	{
		/*-------------------------------
		| Found Match on Program Name	|
		-------------------------------*/
		if (!strcmp(_p_name,sub_rec._prog_name))
		{
			RF_CLOSE(fd);
			sprintf(_m_name,"SUB_MENU/%s",clip(sub_rec._menu_name));
			return(_m_name);
		}
	}

	RF_CLOSE(fd);

	if (!strcmp(_p_name,"default       "))
		return((char *)0);

	return(get_sub("default"));
}

int
open_sub (char *mode)
{
	int		errc, fd;
	char	*sptr = getenv("PROG_PATH");
	char	filename[100];

	sprintf(filename,"%s/BIN/SUB_MENU/SUB_MENU",(sptr != (char *)0) ? sptr : "/usr/DB");

	/*---------------------------------------
	| If variable file doesn't exist	|
	---------------------------------------*/
	if (access(filename,00) < 0)
	{
		if ((errc = RF_OPEN (filename, sizeof (struct sub_type), "w", &fd)))
			sys_err ("Error in SUB_MENU during (WKCREAT)", errc, PNAME);

		if ((errc = RF_CLOSE (fd)))
			sys_err ("Error in SUB_MENU during (WKCLOSE)", errc, PNAME);
	}

	if ((errc = RF_OPEN (filename, sizeof (struct sub_type), mode, &fd)))
		sys_err ("Error in SUB_MENU during (WKOPEN)", errc, PNAME);

	return(fd);
}

int
add_sub (int fd, char *prog_name, char *menu_name)
{
	sprintf(sub_rec._prog_name,"%-14.14s",prog_name);
	sprintf(sub_rec._menu_name,"%-14.14s",menu_name);

	return(RF_ADD(fd,(char *) &sub_rec));
}
