#ifndef	_Index_h
#define	_Index_h

/*	$Id: Index.h,v 5.0 2001/06/19 08:17:30 cha Exp $
 *	
 *	Association between indexname and CISAM keypart structure
 *
 */
#include	<isam.h>
#include	<decimal.h>
#include	"cisamdefs.h"

class Index
{
	friend	class Table;

	private:
		char			name [MAX_IDXNAME_LEN + 1];
		struct keydesc	desc;
		short			parts [MAX_IDX_PARTS];

	public:
		short					Part (unsigned) const;
		const struct keydesc &	Desc (void) const;
};

#endif	//_Index_h
