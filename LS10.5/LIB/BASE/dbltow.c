/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( dbltow.c       )                                 |
|  Program Desc  : ( Convert Money to Words.                      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  PINNACLE,                                         |
|---------------------------------------------------------------------|
|  Updates files :  PINNACLE,                                         |
|---------------------------------------------------------------------|
|  Date Written  : 09/06/88        | Author      : Roger Gibbison     |
|---------------------------------------------------------------------|
|  Date Modified : (05/04/94)      | Modified by : Campbell Mander.   |
|  Date Modified : (17.06.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (17.05.95)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|  (05/04/94)    : HGP 10469. Removal of $ signs.                     |
|  (17.06.94)    : Removed bad use of header files                    |
|  (17.05.95)    : Removed bad use of header files                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

char	*ONES[20] = {
	"","ONE ","TWO ","THREE ","FOUR ","FIVE ",
	"SIX ","SEVEN ","EIGHT ","NINE ","TEN ",
	"ELEVEN ","TWELVE ","THIRTEEN ","FOURTEEN ",
	"FIFTEEN ","SIXTEEN ","SEVENTEEN ","EIGHTEEN ","NINETEEN "
};

char	*TENS[9] = {
	"TWENTY ","THIRTY ","FORTY ","FIFTY ",	
	"SIXTY ","SEVENTY ","EIGHTY ","NINETY "
};

char	*MULTI[3] = {
	"MILLION ","THOUSAND ","" 
};
/*-------------------------------------------------------
| Dbltow converts an Informix Moneytype value to the 	|
| Corresponding words, Returning a pointer to this	|
-------------------------------------------------------*/
char *
dbltow (
 double	money_value,
 char	*primeUnit,
 char	*subUnit)
{
	int		i;
	char	_amount [128];
	char	_value[4];
	char	_string[50];
	static char	_words [200];

	if (money_value > 02000000000.00)
	{
		strcpy (_words,
			"An Error Has Occurred during Conversion - "
			"This May be due to the money value exceeding 20,000,000.00");
		return (_words);
	}

	_words [0] = '\0';
	sprintf (_amount, "%012.2f %-15.15s", money_value / 100, subUnit);
	for (i = 0;i < 3;i++)
	{
		sprintf(_value,"%-3.3s",_amount + (i * 3));
		if (atod(_value,_string) == 0)
			continue;

		/*-----------------------
		| NNN,NNN,0NN.NN CENTS	|
		-----------------------*/
		if (i == 2 && _value[0] == '0' && strcmp(_value + 1,"00") && strlen(_words))
			strcat(_words,"& ");

		strcat(_words,_string);
		if (i != 2)
			strcat(_words,MULTI[i]);
	}

	/*-----------------------------------------------
	| If there is 1 or more whole dollar involved	|
	-----------------------------------------------*/
	if (money_value >= 100.00)
		strcat(_words, primeUnit);

	/*-------------------
	| No Cents Involved	|
	-------------------*/
	if (atoi(_amount + 10) == 0)
		strcat(_words,"ONLY");
	else
	{
		if (!strlen(_words))
			strcpy(_words,_amount + 10);
		else
		{
			strcat(_words,"& ");
			strcat(_words,_amount + 10);
		}
	}

	/*-------------------------------
	| Check for non plural cents	|
	-------------------------------*/
	if (!strncmp(_amount + 10,"01",2))
		_words[strlen(_words) - 1] = (char) NULL;

	return(_words);
}

/*-------------------------------
| Ascii to Dollar String	|
-------------------------------*/
int
atod(char *_value, char *_string)
{
	int	digit = atoi(_value + 1);

	_string[0] = '\0';
	/*-----------------------------------------------
	| Value has format ABC				|
	| if A is a digit 1 thru 9 then there are some	|
	| Hundreds invloved.				|
	-----------------------------------------------*/
	if (_value[0] != '0')
	{
		strcpy(_string,ONES[_value[0] - '0']);
		_string = strcat(_string,"HUNDRED ");
	}

	/*-----------------------------------------------
	| If the _value is some Hundreds plus		|
	| then "& "					|
	-----------------------------------------------*/
	if (digit != 0 && _value[0] != '0')
		_string = strcat(_string,"& ");

	/*-----------------------------------------------
	| If the plus bit is less than 20		|
	| then use then ONES array for then names	|
	-----------------------------------------------*/
	if (digit < 20)
	{
		if (!strlen(_string))
			strcpy(_string,ONES[digit]);
		else
			_string = strcat(_string,ONES[digit]);
	}
	else
	{
		if (!strlen(_string))
			strcpy(_string,TENS[_value[1] - '2']);
		else
			_string = strcat(_string,TENS[_value[1] - '2']);

		if (_value[2] != '0')
			_string = strcat(_string,ONES[_value[2] - '0']);
	}
	return(atoi(_value));
}
