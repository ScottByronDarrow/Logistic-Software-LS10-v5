/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( inis_update.h )                                  |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Updates files :                                                    |
|---------------------------------------------------------------------|
|  Author        : ?               | Date Written  : ??/??/????       |
|---------------------------------------------------------------------|
|  Date Modified : (18/08/1999)    | Modified by : Eumir Que Camara.  |
|                :                                                    |
|  Comments      :                                                    |
|  (18/08/1999)  : Ported to ANSI standard.                           |
=====================================================================*/
#ifndef __CHK_VBLE_H__
#define __CHK_VBLE_H__

char*   chk_vble (char*  _env, char*  _prg);
char*   _chk_vble (char*  _env, 
                   char*  _prg,
                   char*  _co_no,
                   char*  _est_no);

/*=======================================================
| check the existence of the environment variable	    |
| first with company & branch, then company, and	    |
| then by itself.					                    |
| if no vble found then return 'prg'			        |
=======================================================*/
char *
chk_vble (
 char*  _env,
 char*  _prg)
{
	return(_chk_vble(_env,_prg,comm_rec.tco_no,comm_rec.test_no));
}

char *
_chk_vble (
 char*  _env,
 char*  _prg,
 char*  _co_no,
 char*  _est_no)
{
	char	*sptr;
	char	_run_print[41];

	/*-------------------------------
	| Check Company & Branch	|
	-------------------------------*/
	sprintf(_run_print,"%s%s%s",_env,_co_no,_est_no);
	sptr = chk_env(_run_print);
	if (sptr == (char *)0)
	{
		/*---------------
		| Check Company	|
		---------------*/
		sprintf(_run_print,"%s%s",_env,_co_no);
		sptr = chk_env(_run_print);
		if (sptr == (char *)0)
		{
			sprintf(_run_print,"%s",_env);
			sptr = chk_env(_run_print);
			return((sptr == (char *)0) ? _prg : sptr);
		}
		else
			return(sptr);
	}
	else
    {
		return(sptr);
    }
}

#endif /* __CHK_VBLE_H__ */
