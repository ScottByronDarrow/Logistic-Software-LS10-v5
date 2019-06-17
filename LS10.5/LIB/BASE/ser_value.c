#include	<std_decs.h>

double
ser_value (
	double est_value, 
	double act_value)
{
	char	*envVarSerValue = chk_env ("SER_VALUE");

	if (envVarSerValue == (char *) 0)
		return (est_value);

	if (envVarSerValue [0] == 'E')
		return (est_value);
	else
		return ((act_value == 0.0) ? est_value : act_value);
}
