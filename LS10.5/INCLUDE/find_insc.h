
void	find_insc (long, char *);
int		valid_date (long);

/*================================
| Find stock take header record. |
================================*/
void
find_insc (long hhccHash, char *_mode)
{
	insc_rec.sc_hhcc_hash = hhccHash;
	sprintf (insc_rec.sc_stake_code, "%-1.1s", _mode);
	if (find_rec ("insc", &insc_rec, COMPARISON, "r"))
	{
		IN_STAKE = FALSE;
		STAKE_COFF = 0L;
		return;
	}
	STAKE_COFF = insc_rec.sc_frz_date;
	IN_STAKE = (BY_MODE) ? TRUE : FALSE;

	return;
}

/*=================================
| Validate Stock take date dates. |
=================================*/
int
valid_date (long _date)
{
	if (!IN_STAKE)
		return (TRUE);

	if (_date > insc_rec.sc_frz_date)
		return (FALSE);

	return (TRUE);
}
