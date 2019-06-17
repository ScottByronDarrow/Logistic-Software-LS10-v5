/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( open_env.c     )                                 |
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
#include	<pinn_env.h>

int		envMaintOption	=	FALSE;

int
open_env (void)
{
	int		fd	=	-1,
			err	=	0;
	char	*sptr;
	char	filename [256];

	if ((sptr = getenv ("PSL_ENV_NAME")))
		strcpy (filename, sptr);
	else
	{
		if (!(sptr = getenv ("PROG_PATH")))
			strcpy (filename, "/usr/ver9.10/BIN/LOGISTIC");
		else
			sprintf(filename, "%s/BIN/LOGISTIC", sptr);
	}
	/*-----------------------------------
	| Create file IF it does not exist. |
	-----------------------------------*/
	if (access (filename, F_OK))
	{
		if (envMaintOption)
		{
			err = RF_OPEN (filename, sizeof (PinnEnv), "w", &fd);
			if (err)
				file_err (-1, filename, "Unable to create Environment");
			return (fd);
		}
		else
			file_err (-1, filename, "Unable to open Environment");
	}

	/*---------------------------------------------
	| Need Read AND Write permission to continue. |
	---------------------------------------------*/
	if (access (filename, R_OK) || (access (filename, W_OK) && envMaintOption))
	{
		fprintf (stderr, "No permission on Environment file [%s]\n",filename);
		file_err (13, filename, "Access Denied R_OK");
	}
	/*------------
	| Go for it. |
	------------*/
	err = RF_OPEN (filename, sizeof (PinnEnv), (envMaintOption) ? "u" : "r",&fd);
	if (err)
	{
		fprintf (stderr, "Failed open on Environment File [%s] err [%d]", filename, err);
		file_err (err, "open_env", "RF_OPEN");
	}
	return (fd);
}

void
close_env (int fd)
{
	int	err = RF_CLOSE (fd);

	if (err)
		file_err (err, "close_env", "RF_CLOSE");
}
