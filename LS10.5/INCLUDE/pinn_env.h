#ifndef	PINN_ENV_H
#define	PINN_ENV_H
/*******************************************************************************
 *
 *	Structure definition of Logistic Environment Records
 *
 ******************************************************************************/

typedef	struct
		{
			char	env_name	[16],
					env_value	[31],
					env_desc	[71];
		}	PinnEnv;

#endif	/*PINN_ENV_H*/
