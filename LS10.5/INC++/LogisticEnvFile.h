#ifndef	_LogisticEnvFile_h
#define	_LogisticEnvFile_h
/*	$Id: LogisticEnvFile.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 ***************************************************************
 *	$Log: LogisticEnvFile.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:13:40  scott
 *	New File
 *	
 *	New File
 *	
 *	Initial revision
 *
 */

#include	<WorkFile.h>

/*
 *	Structure definition of Logisticacle Environment Records
 *
 */
struct _tagLogisticEnv
{
	char	name	[16],
			value	[31],
			desc	[71];
};

class LogisticEnvFile :
	public WorkFile
{
	private:
		_tagLogisticEnv		data;

	public:
		LogisticEnvFile (const char * pathname = 0);

		
		/*
		 *	Accessors
		 */
		const char *	Name (void) const,			// current record
				   *	Value (void) const,
				   *	Desc (void) const;

		/*
		 *	Mutators
		 */
		LogisticEnvFile &	Name (const char *),
					&	Value (const char *),
					&	Desc (const char *);

		/*
		 *	Data access routines
		 */
		bool	Next (void),					// access
				Prev (void);

		/*
		 *	Data write
		 */
		bool	Write (void);					// write at current posn
};

#endif	//_LogisticEnvFile_h
