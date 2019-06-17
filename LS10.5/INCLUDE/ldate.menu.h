#include <time.h>
time_t	tloc;
struct	tm	*ts;

void	dtime (void);
/*----------------------------------------------------------------------
| Set up Array to hold Days of the week used with wday in time struct. |
----------------------------------------------------------------------*/
static char *day[] = {
	"Sunday",	"Monday",	"Tuesday",	"Wednesday",
	"Thursday",	"Friday",	"Saturday"
};
/*-------------------------------------------------------------------
| Set up Array to hold Months of Year used with mon in time struct. |
-------------------------------------------------------------------*/
static char *mth[] = {
	"January",	"February",	"March",	"April",
        "May", 		"June", 	"July", 	"August", 
	"September", 	"October", 	"November", 	"December"
};

static char *abr[] = {
	"st","nd","rd","th","th","th","th","th","th","th",
	"th","th","th","th","th","th","th","th","th","th",
	"st","nd","rd","th","th","th","th","th","th","th",
	"st"
};

void
dtime (void)
{
	char	_date_str[31];

	/*-----------------------------------
	| get pointer to time & structure . |
	-----------------------------------*/
	tloc = time(NULL);
	ts = localtime(&tloc);
	
	sprintf(_date_str, "%-s %-s %d%-s %d", day[ts->tm_wday], 
						 mth[ts->tm_mon],
						 ts->tm_mday,
						 abr[ts->tm_mday-1],
						 ts->tm_year + 1900);
			
	printf(" %-30.30s %-30.30sTime %02d:%02d %2.2s", 
		_date_str,
		" ",
		((ts->tm_hour) > 12 ? (ts->tm_hour -12 ) : (ts->tm_hour)),
		ts->tm_min,
		((ts->tm_hour) >= 12 ? "pm" : "am"));
}
