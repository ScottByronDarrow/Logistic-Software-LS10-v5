#ifndef _Environ_h
#define _Environ_h
/*	$Id: Environ.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	{libapp}
 *
 *	Abstract class for Environment value tuples
 *
 ******************************************************
 *	$Log: Environ.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:43  scott
 *	Updated to clean code while working in format-p
 *	
 *	Revision 3.0  2000/10/12 13:39:02  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 *	Initial C++ sources (adopted minimally from V10)
 *	
 */
class String;

class Environment
{
	private:
		String *	envName,
			   *	envValue;

	protected:
		bool		exists;

	protected:
		/*
		 *	Constructors, Destructors
		 */
		Environment (const char * name);

	public:
		virtual ~Environment ();

		/*
		 *	Mutators
		 */
		virtual void	Put (const char *),
						Put (int);

		/*
		 *	Accessors
		 */
		bool			Exists (void) const;
		const String &	Name (void) const;

		const char *	chars (void) const;

		operator int () const;
		operator const char * () const;
};

#endif	// _Environ_h
