#include	<stdio.h>
#include	<ctype.h>
#include	<std_decs.h>

int
check_search(char *search_str, char *key_val, int *break_out)
{
	char	*sptr;
	char	*pptr;	/* pointer to position in pattern string	*/
	char	*tptr;	/* pointer to position in pattern string	*/
	char	t_string[41];
	char	p_string[41];

	sprintf(t_string,"%-40.40s",search_str);
	sprintf(p_string,"%-.40s",key_val);
	sptr = clip(t_string);

	while (*sptr)
	{
		*sptr = toupper(*sptr);
		sptr++;
	}

	sptr = strchr (p_string,'*');
	/*-----------------------
	| No Wild Carding	|
	-----------------------*/
	if (sptr == (char *)0)
	{
		*break_out = strncmp(t_string,key_val,strlen(key_val));
		return(!*break_out);
	}

	if (p_string[0] != '*')
	{
		*sptr = '\0';
		/*---------------------------------------
		| Check Beginning of Target String	|
		---------------------------------------*/
		*break_out = strncmp(p_string,t_string,strlen(p_string));
		if (*break_out)
			return(0);

		tptr = t_string + strlen(p_string);
		pptr = sptr + 1;
		while (*pptr == '*')
			pptr++;
	}
	else
	{
		*break_out = 0;
		tptr = t_string;
		pptr = p_string;
		while (*pptr == '*')
			pptr++;
	}

	if (!strlen(pptr))
		return(1);

	sptr = strchr (pptr,'*');
	/*-------------------------------
	| Check Other Wild Cards	|
	-------------------------------*/
	while (sptr != (char *)0)
	{
		*sptr = '\0';

		tptr = strstr (tptr, pptr);
		if (tptr == (char *)0)
			return(0);
		tptr += strlen(pptr);
		pptr = sptr + 1;
		sptr = strchr (pptr,'*');
	}

	/*-----------------------
	| Look For Match At End	|
	-----------------------*/
	if (strlen(pptr) > 0)
	{
		/*--------------
		| Cannot Match 	|
		--------------*/
		if (strlen(pptr) > strlen(tptr))
			return(0);

		sptr = tptr + strlen(tptr) - strlen(pptr);

		return(!strcmp(sptr,pptr));
	}
	return(1);
}
