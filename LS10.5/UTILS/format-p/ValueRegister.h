#ifndef	ValueRegister_h
#define	ValueRegister_h
/*	$Id: ValueRegister.h,v 5.0 2001/06/19 08:22:47 robert Exp $
 *
 *	Storage for value tuples
 *
 *******************************************************************************
 *	$Log: ValueRegister.h,v $
 *	Revision 5.0  2001/06/19 08:22:47  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:47  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:13  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:12  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/07/15 00:19:15  jonc
 *	Adopted from Pinnacle V10
 *	
 */
#include	<CArray.h>
#include	<String.h>

class ValueRegister
{
	private:
		struct Entry
		{
			String	key, value;
		};

	private:
		CArray <Entry>	entries;

	private:
		const char *	Value (const char * key) const;

	public:
		void			Reset ();
		void			Add (const char * key, const char * value);
		String &		DecodeValue (const char * keystr, String &) const;
};

#endif	//	ValueRegister_h
