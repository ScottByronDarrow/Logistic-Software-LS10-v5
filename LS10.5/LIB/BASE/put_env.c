/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( put_env.c      )                                 |
|  Program Desc  : ( Put and environment into PINNACLE              ) |
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

void
put_env (char *vble_name, char *vble_value, char *vble_desc)
{
	int		err;
	char	*sptr = NULL;
	PinnEnv	env_rec;
	int		fd = open_env ();

	/*	Look for matching record
	 */
	if ((err = RF_REWIND (fd)))
		sys_err ("Error in PUT_ENV during (REWINDE)", err, PNAME);

	while (!RF_READ (fd, (char *) &env_rec))		/* while RF_READ succeeds */
		if (!strcmp (vble_name, env_rec.env_name))
		{
			sptr = clip (env_rec.env_value);
			break;
		}

	sprintf (env_rec.env_name, "%-15.15s", vble_name);
	sprintf (env_rec.env_value, "%-30.30s", vble_value);
	sprintf (env_rec.env_desc, "%-70.70s", vble_desc);

	if (!sptr)
	{
		/*	Attempt to add the record if it doesn't exist
		 */
		lseek (fd, 0, SEEK_END);
		if ((err = RF_ADD (fd, (char *) &env_rec)))
			sys_err ("Error in PUT_ENV during (ADD)", err, PNAME);
	}
	else
	{
		if ((err = RF_UPDATE(fd,(char *) &env_rec)))
			sys_err ("Error in PUT_ENV during (UPDATE)", err, PNAME);
	}
	close_env (fd);
}
