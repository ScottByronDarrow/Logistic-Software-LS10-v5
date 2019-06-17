/*===============================================================
|	if error type 1 or 999 in finding insf records		|
|		return 0.00					|
|	else							|
|		if avge						|
|			return average value of serial items	|
|		else						|
|			return total value of serial items	|
===============================================================*/
#include <up_serial.h>

double	find_s_cost (long hhwh_hash, int avge)
{
	char	ser_value[2];
	int	rc = 0;
	double	cnt;
	double	cost;

	cost = 0.00;
	cnt  = 0.00;

	rc = find_serial( hhwh_hash, (char *) NULL, "F", "r");

	while (!rc && hhwh_hash == insf_rec.sf_hhwh_hash)
	{
		sprintf(ser_value, "%-1.1s", get_env("SER_VALUE"));

		if (ser_value[0] == 'E')
			cost += insf_rec.sf_est_cost;
		else
			cost += (insf_rec.sf_act_cost != 0.00) ? insf_rec.sf_act_cost : insf_rec.sf_est_cost;
		cnt++;
		rc = find_serial( 0L, (char *) NULL, "F", "r");
	}
	rc = find_serial( hhwh_hash, (char *) NULL, "C", "r");

	while (!rc && hhwh_hash == insf_rec.sf_hhwh_hash)
	{
		sprintf(ser_value, "%-1.1s", get_env("SER_VALUE"));

		if (ser_value[0] == 'E')
			cost += insf_rec.sf_est_cost;
		else
			cost += (insf_rec.sf_act_cost != 0.00) ? insf_rec.sf_act_cost : insf_rec.sf_est_cost;
		cnt++;
		rc = find_serial( 0L, (char *) NULL, "C", "r");
	}

	if (avge && cnt != 0.00)
		cost /= cnt;

	return((cnt) ? cost : 0.00);
}
