#ident	"$Id: Money.C,v 5.0 2001/06/19 08:19:06 cha Exp $"
/*
 *	$Log: Money.C,v $
 *	Revision 5.0  2001/06/19 08:19:06  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:28:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.1.1.1  1999/07/15 00:00:57  jonc
 *	Initial C++ libraries (minimally adopted from Pinnacle V10)
 *	
 *	Revision 1.2  1998/04/23 01:22:02  jonc
 *	Added support from formatted stream output
 *
 *	Revision 1.1.1.1  1998/01/22 00:58:45  jonc
 *	Version 10 start
 *
 *	Revision 2.4  1996/08/21 04:48:05  jonc
 *	Added Constructor from Number
 *
 *	Revision 2.3  1996/07/30 00:53:00  jonc
 *	Added #ident directive
 *
 *	Revision 2.2  1996/07/16 23:02:49  jonc
 *	Floating point constructors added
 *
 *	Revision 2.1  1996/05/04 08:29:05  jonc
 *	Added various constructors
 *
 *	Revision 2.0  1996/02/13 03:34:57  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:32:46  jonc
 *	Initial C++ Support library
 *
 */
#include	<osdeps.h>

#include	<Money.h>

Money::Money ()
{
}

Money::Money (
 int	v) :
	Number (v)
{
}

Money::Money (
 const char *	v) :
	Number (v)
{
}

Money::Money (
 double	v,
 int	p) :
	Number (v, p)
{
}

Money::Money (
 const Number &	v) :
	Number (v)
{
}

Money &
Money::operator = (
 int	v)
{
	Number::operator = (v);
	return (*this);
}

Money &
Money::operator = (
 long	v)
{
	Number::operator = (v);
	return (*this);
}

Money &
Money::operator = (
 const char *	v)
{
	Number::operator = (v);
	return (*this);
}

Money &
Money::operator = (
 const Money &	v)
{
	Number::operator = (v);
	return (*this);
}

Money &
Money::operator = (
 const Number &	v)
{
	Number::operator = (v);
	return (*this);
}

String &
Money::Get (
 String &	s) const
{
	/*
	 *	By default, we get it to 2 decimal places
	 */
	return Number::Get (0, 2, s);
}

String &
Money::Get (
 int		len,
 String &	s) const
{
	/*
	 *	By default, we get it to 2 decimal places
	 */
	return Number::Get (len, 2, s);
}
