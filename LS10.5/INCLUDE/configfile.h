#ifndef	_configfile_h
#define	_configfile_h
/*	$Id: configfile.h,v 5.0 2001/06/19 06:51:27 cha Exp $
 *
 *	Structure and prototypes for dealing with Configuration Files.
 *
 *	You should not be using the structure directly.
 *	Use the appropriate function calls to access data.
 *
 *******************************************************************************
 *	$Log: configfile.h,v $
 *	Revision 5.0  2001/06/19 06:51:27  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:22  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:53  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:36  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.1  1999/10/11 21:41:01  jonc
 *	Added Configuration file interface.
 *	
 */

typedef struct _tConfKey
{
	char *	key,
		 *	value;

	struct _tConfKey *		next;
}	_ConfKey;

typedef	struct _tConfSection
{
	char *		name;
	_ConfKey *	keys;

	struct _tConfSection *	next;
}	_ConfSection;

typedef struct
{
	/*
	 *	Elements are *NOT* to be accessed by application code.
	 *	Use the functions following.
	 */
	_ConfSection *	sections;

}	ConfigFile;


/*
 *	Function prototypes
 */
ConfigFile *	OpenConfig (const char * program);
void			CloseConfig (ConfigFile * cf);

const char *	GetConfig (const ConfigFile *,
					const char * sect, const char * key);
int				GetConfigInt (const ConfigFile *,
					const char * s, const char * k);

int				ItrConfigSection (const ConfigFile * cf,
					int * state,
					char * rvalue);
int				ItrConfigKeys (const ConfigFile * cf,
					int sectionid,
					int * state,
					char * v);

#endif	/*	_configfile_h */
