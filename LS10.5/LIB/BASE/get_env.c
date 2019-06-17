/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( get_env.c      )                                 |
|  Program Desc  : ( Environment Variable Utilities.              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  SEL,	                                          |
|---------------------------------------------------------------------|
|  Updates files :  SEL,                                              |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 09/06/88         |
|---------------------------------------------------------------------|
|  Date Modified : 01/06/89        | Modified  by  : Scott Darrow     |
|  Date Modified : 01.11.95        | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      : 01/06/89 - Removed work file routines to save      |
|                :            20 Blocks on programs.                  |
|  (01.11.95) : improved error message                                |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>
#include	<pinn_env.h>

/*
 *	Local globale holding fd for the environment file
 */
static int	env_fd = -1;

/*=======================================================
| Used to check whether a particular environment	|
| Variable exists. 					|
| returns value of variable if it does,			|
| NULL otherwise
=======================================================*/
char *
chk_env (char *vble_name)
{
	char	*sptr;
	char	name [20];
	static PinnEnv	env_rec;

	if (env_fd < 0)
		env_fd = open_env ();

	if (RF_REWIND (env_fd))
		return (NULL);

	sprintf (name, "%-15.15s", vble_name);

	while (!RF_READ (env_fd, (char *) &env_rec))		/* while RF_READ succeeds */
		if (!strcmp (name, env_rec.env_name))
		{
			sptr = clip (env_rec.env_value);
			return (*sptr ? sptr : NULL);
		}
	return (NULL);
}

/*===============================================
| Lookup the Environment File For an Entry	|
| For the Variable "vble_name".			|
| Return a pointer to the value for Variable	|
| if the Variable is Not Declared, crash
===============================================*/
char *
get_env (char *vble_name)
{
	char	*sptr = chk_env (vble_name);
	char	error [81];

	if (!sptr)
	{
		sprintf (error, "(get_env) Failed to find %s in %s",
			vble_name,
			getenv ("PSL_ENV_NAME"));
		sys_err (error, -1, PNAME);
	}
	return (sptr);
}
