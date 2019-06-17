#ifndef	_Number_h
#define	_Number_h
/*
 *	Arbitrary precision numbers
 *
 *******************************************************************************
 *	$Log: Number.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:39:02  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.2  1999/12/05 22:12:49  jonc
 *	Reimplemented to fix problem in representing some -ve numbers.
 *	
 */
class String;

class Number
{
	public:
		enum RN_Type
		{
			RN_Near,
			RN_Upper,
			RN_Lower
		};

	private:
		unsigned valuesz;			// size of value array
		unsigned char * value;		// value array

		bool negative;
		unsigned radixpt;			// values in array below radix-point

	private:
		void CheckValueSize (unsigned req);
		Number & Trim ();

	public:
		Number (int = 0);
		Number (long);
		Number (const char *);
		Number (double, int = 0);
		Number (const Number &);
		virtual ~Number ();

		/*
		 */
		Number & Negate ();
		Number & RShift ();
		Number & LShift ();

		/*
		 *	Rounding
		 */
		Number & Trunc (void),
			   & Ceil (void),
			   & Floor (void);
		Number & Round (int = 1, enum RN_Type = RN_Near),
			   & Round (long, enum RN_Type = RN_Near),
			   & Round (const Number &, enum RN_Type = RN_Near),
			   & Round (const char *);

		/*
		 *	Conversions
		 */
		operator long () const;
		operator int () const;
		operator double () const;
		operator float () const;

		virtual String & Get (String & buffer) const;
		virtual String & Get (int len, String &) const,				// r-justify
					   & Get (int len, int prec, String &) const;	// r-justify

		/*
		 */
		void Dump (const char *) const;

		/*
		 *	Assignments
		 */
		Number & operator = (int);
		Number & operator = (long);
		Number & operator = (const char *);
		Number & operator = (const Number &);

		Number &	operator += (int);
		Number &	operator += (long);
		Number &	operator += (const Number &);
		Number &	operator -= (int);
		Number &	operator -= (long);
		Number &	operator -= (const Number &);
		Number &	operator *= (int);
		Number &	operator *= (long);
		Number &	operator *= (const Number &);
		Number &	operator /= (int);
		Number &	operator /= (long);
		Number &	operator /= (const Number &);

	/*
	 *	Unary friends
	 */
	friend Number operator - (const Number &);

	/*
	 *	Binary friends
	 */

	// arithmetic operations
	friend Number operator + (const Number &, const Number &);
	friend Number operator + (int, const Number &);
	friend Number operator + (const Number &, int);
	friend Number operator + (long, const Number &);
	friend Number operator + (const Number &, long);
	friend Number operator + (const char *, const Number &);
	friend Number operator + (const Number &, const char *);

	friend Number operator - (const Number &, const Number &);
	friend Number operator - (int, const Number &);
	friend Number operator - (const Number &, int);
	friend Number operator - (long, const Number &);
	friend Number operator - (const Number &, long);
	friend Number operator - (const char *, const Number &);
	friend Number operator - (const Number &, const char *);

	friend Number operator * (const Number &, const Number &);
	friend Number operator * (int, const Number &);
	friend Number operator * (const Number &, int);
	friend Number operator * (long, const Number &);
	friend Number operator * (const Number &, long);
	friend Number operator * (const char *, const Number &);
	friend Number operator * (const Number &, const char *);

	friend Number operator / (const Number &, const Number &);
	friend Number operator / (int, const Number &);
	friend Number operator / (const Number &, int);
	friend Number operator / (long, const Number &);
	friend Number operator / (const Number &, long);
	friend Number operator / (const char *, const Number &);
	friend Number operator / (const Number &, const char *);

	//	comparision operators
	friend bool operator == (const Number &, const Number &);
	friend bool operator == (int, const Number &);
	friend bool operator == (const Number &, int);
	friend bool operator == (long, const Number &);
	friend bool operator == (const Number &, long);
	friend bool operator == (const char *, const Number &);
	friend bool operator == (const Number &, const char *);
	friend bool operator != (const Number &, const Number &);
	friend bool operator != (int, const Number &);
	friend bool operator != (const Number &, int);
	friend bool operator != (long, const Number &);
	friend bool operator != (const Number &, long);
	friend bool operator != (const char *, const Number &);
	friend bool operator != (const Number &, const char *);
	friend bool operator > (const Number &, const Number &);
	friend bool operator > (int, const Number &);
	friend bool operator > (const Number &, int);
	friend bool operator > (long, const Number &);
	friend bool operator > (const Number &, long);
	friend bool operator > (const char *, const Number &);
	friend bool operator > (const Number &, const char *);
	friend bool operator < (const Number &, const Number &);
	friend bool operator < (int, const Number &);
	friend bool operator < (const Number &, int);
	friend bool operator < (long, const Number &);
	friend bool operator < (const Number &, long);
	friend bool operator < (const char *, const Number &);
	friend bool operator < (const Number &, const char *);
	friend bool operator >= (const Number &, const Number &);
	friend bool operator >= (int, const Number &);
	friend bool operator >= (const Number &, int);
	friend bool operator >= (long, const Number &);
	friend bool operator >= (const Number &, long);
	friend bool operator >= (const char *, const Number &);
	friend bool operator >= (const Number &, const char *);
	friend bool operator <= (const Number &, const Number &);
	friend bool operator <= (int, const Number &);
	friend bool operator <= (const Number &, int);
	friend bool operator <= (long, const Number &);
	friend bool operator <= (const Number &, long);
	friend bool operator <= (const char *, const Number &);
	friend bool operator <= (const Number &, const char *);

	/*
	 *	Internal support
	 */
	friend unsigned char * Normalized (const Number &, unsigned, unsigned);
};

#endif	// _Number_h
