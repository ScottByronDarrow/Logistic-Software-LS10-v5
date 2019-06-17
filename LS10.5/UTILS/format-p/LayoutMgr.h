#ifndef	LayoutMgr_h
#define	LayoutMgr_h
/*
 *	LayoutMgr holder
 *
 *******************************************************************************
 *	$Log: LayoutMgr.h,v $
 *	Revision 5.0  2001/06/19 08:22:46  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:47  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:08  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:11  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/07/15 00:19:15  jonc
 *	Adopted from Pinnacle V10
 *	
 */
#include	<CArray.h>
#include	<String.h>

#include	"ValueRegister.h"

class LineReader;
class Options;
class OutputRecvr;

class LayoutMgr
{
	private:
		struct Section
		{
			String			name;
			CArray <String>	content;
		};

	private:
		OutputRecvr &		output;

		bool				usable;

		int					pagelen,
							pagehdrbeg,
							pagebodybeg,
							pagebodyend,
							pagetrailerbeg;

		CArray <Section>	sections;
		Section *			reporthdr,
				*			reportend,
				*			pagehdr,
				*			pageend,
				*			pageendLast,
				*			bodyhdr,
				*			bodyend,
				*			body;

		bool				rpthdrprinted;
		int					pageno, lineno;
		ValueRegister		vregister;

	private:
		Section *	GetSection (const char *) const;
		void		SetDefaultSections ();

		void		PrintLine (const char *);
		void		PrintSection (const Section *, int toline);

		void		CheckForBreaks (),
					PrintBody (),
					PrintLastPage (bool);

		void		InterpretDirective (const char *, const char *, bool);
		bool		SubmitDirective (
						const char * section,
						const char * directive,
						Section * &,
						const char * newsection);

	public:
		LayoutMgr (const Options &, OutputRecvr &);

		bool		Usable () const;
		void		Read (LineReader &);

};

#endif	//	LayoutMgr_h
