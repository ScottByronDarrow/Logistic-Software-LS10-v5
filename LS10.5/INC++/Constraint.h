#ifndef	_Constraint_h
#define	_Constraint_h
/*	$Id: Constraint.h,v 5.0 2002/05/08 01:50:42 scott Exp $
 *
 *	Constaints: Conditions and Condition Groups
 *
 *******************************************************************************
 *	$Log: Constraint.h,v $
 *	Revision 5.0  2002/05/08 01:50:42  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:42  scott
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
#include	<Array.h>
#include	<ColInfo.h>
#include	<Date.h>

enum CondCmp
{
	Eq, Ne, Gt, Ge, Lt, Le,

	Mt							// string matching with '*' and '?'
};

class Condition;

class Constraint
{
	/*
	 *	Abstract class
	 */
	protected:
		Constraint ();

	public:
		virtual ~Constraint ();

	/***********************************************
	 *	Undocumented interfaces
	 *		- the following should not be used by
	 *		  applications
	 *		- it is intended for internal library
	 *		  management use only.
	 ************************************************/
	// begin{Undocumented-Interface}

	public:
		virtual bool				C () const = 0;		// true if Condition
		virtual bool				Fulfilled (void *) const = 0;
		virtual const Condition *	FindWithColNo (short) const = 0;

	// end{Undocumented-Interface}
};

class Condition	:
	public Constraint
{
	/*
	 *	Simple condition
	 */
	friend class Query;

	private:
		const char *	colname;
		enum CondCmp	cond;
		ColumnInfo		colinfo;		// set by Query class

		int				valtype;
		char *			vchars;
		double			vdouble;
		int				vint;
		long			vlong;

		Date			vdate;			// abstract types

	public:
		Condition (const Condition &);
		Condition (enum CondCmp, const char *, int);
		Condition (enum CondCmp, const char *, long);
		Condition (enum CondCmp, const char *, const char *);
		Condition (enum CondCmp, const char *, char);
		Condition (enum CondCmp, const char *, double);

		Condition (enum CondCmp, const char *, const Date &);

		~Condition ();

		/*
		 *	Accessors
		 */
		const char *	Col (void) const;
		int				ColNo (void) const;
		enum CondCmp	Cond (void) const;
		int				Int (void) const;
		long			Long (void) const;
		const char *	Chars (void) const;

	/***********************************************
	 *	Undocumented interfaces
	 *		- the following should not be used by
	 *		  applications
	 *		- it is intended for internal library
	 *		  management use only.
	 ************************************************/
	// begin{Undocumented-Interface}

	public:
		/*
		 *	Required virtuals
		 */
		virtual bool				C () const;
		virtual bool				Fulfilled (void *) const;
		virtual const Condition *	FindWithColNo (short) const;

		int		SignificantLength (void) const;
		bool	SignificantCheck (void *) const;
		void	Put (void *) const;		// store value to CISAM buffer

	// end{Undocumented-Interface}
};

class CGroup	:
	public Constraint
{
	/*
	 *	Grouping for Constraint
	 */
	friend class Query;

	private:
		int					andC;
		Array<Constraint *>	andGrp;

		int					orC;
		Array<Constraint *>	orGrp;

	private:
		void	ClearConstraints (void);

	public:
		CGroup ();
		CGroup (const CGroup &);
		CGroup (const Condition &);
		~CGroup ();

		/*
		 *	Mutators
		 */
		CGroup &	And (const Constraint &);
		CGroup &	Or (const Constraint &);

		CGroup &	operator = (const Condition &),
			   &	operator = (const CGroup &);

		/*
		 *	Accessors
		 */
		int	AndC (void) const;
		const Constraint &	And (int) const;

		int	OrC (void) const;
		const Constraint &	Or (int) const;

	/***********************************************
	 *	Undocumented interfaces
	 *		- the following should not be used by
	 *		  applications
	 *		- it is intended for internal library
	 *		  management use only.
	 ************************************************/
	// begin{Undocumented-Interface}

	public:
		virtual bool				C () const;
		virtual bool				Fulfilled (void *) const;
		virtual const Condition *	FindWithColNo (short) const;

	// end{Undocumented-Interface}
};

extern CGroup operator && (const Constraint &, const Constraint &);
extern CGroup operator || (const Constraint &, const Constraint &);

#endif	//_Constraint_h
