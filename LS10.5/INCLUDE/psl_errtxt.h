/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( psl_errtxt.h   )                                 |
|  Program Desc  : ( Errors message file for Informix-3.3,        )   |
|                  ( Informix-SQL and Informix-CISAM                  |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/10/90         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      : (  /  /  ) -                                       |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
struct  {
	int	code;
	char	*msg1;
	char	*msg2;
	char	*msg3;
} error_codes[] ={
	{-1,	"internal error",
		"An unknown internal error has occurred",
		" "},

	{1,	"No value found for request.",
		"A attempt was made to find a record that should have existed",
		"via isread."},

	{100,	"C-ISAM ERROR - duplicate add to index.",
		"An attempt was made to add a duplicate value to an index",
		"via iswrite, isrewrite, isrewcurr or isaddindex."},

	{101,	"C-ISAM ERROR - file not previously opened.",
		"An attempt was made to perform some operation on a C-ISAM ",
		"file that was not previously opened using isopen call."},

	{102,	"C-ISAM ERROR - call arguments not within acceptable range.",
		"One of the arguments of the C-ISAM call is not within the ",
		"range of the acceptable values for that argument."},

	{103,	"C-ISAM ERROR - key element(s) not within acceptable range.",
		"One of the elements that make up the key description is ",
		"outside of the range of acceptable values for that element."},

	{104,	"C-ISAM ERROR - maximum number of files reached.",
		"The maximum number of files that may be open at one time",
		"would be exceeded if this request were processed."},

	{105,	"C-ISAM ERROR - format of file corrupted.",
		"The format of the C-ISAM file has been corrupted.",
		"** WARNING THIS ERROR MUST BE LOOKED AT URGENTLY **."},

	{106,	"C-ISAM ERROR - add/delete index without exclusive access",
		"In order to add or delete an index, the file must have",
		"been opened with the exclusive access."},

	{107,	"C-ISAM ERROR - record locked by another process.",
		"The record or file request by this call cannot be accessed",
		"because it has been locked by another user."},

	{108,	"C-ISAM ERROR - attempt to add index previously defined.",
		"An attempt was made to add an index that has been defined",
		"previously."},

	{109,	"C-ISAM ERROR - attempt to delete primary key value.",
		"An attempt was made to delete an index the primary key value.",
		"The primary key may not be deleted by the isdelindex call."},

	{110,	"C-ISAM ERROR - beginning or end of file reached.",
		"The PREVIOUS operation has failed because beginning of file ",
		"was reached."},

	{111,	"C-ISAM ERROR - no match for requested value.",
		"No record could be found that contained the requested value",
		"in the specified position."},

	{112,	"C-ISAM ERROR - no current record.",
		"The call must operate on the current record.",
		"The current record is not defined."},

	{113,	"C-ISAM ERROR - file locked by another process.",
		"The file is exclusively locked by another user.",
		" "},

	{114,	"C-ISAM ERROR - file name is to long.",
		" ",
		" "},

	{115,	"C-ISAM ERROR - The lock file cannot be created.",
		" ",
		" "},

	{116,	"C-ISAM ERROR - adequate memory cannot be Allocated.",
		" ",
		" "},

	{117,	"C-ISAM ERROR - Bad custom collating.",
		" ",
		" "},

	{118,	"C-ISAM ERROR - cannot read log file record.",
		" ",
		" "},

	{119,	"C-ISAM ERROR - Transaction log file format cannot be recognized.",
		" ",
		" "},

	{120,	"C-ISAM ERROR - cannot open transaction log file.",
		" ",
		" "},

	{121,	"C-ISAM ERROR - cannot write to transaction log file.",
		" ",
		" "},

	{122,	"C-ISAM ERROR - Not in Transaction.",
		" ",
		" "},

	{123,	"C-ISAM ERROR - No Shared Memory.",
		" ",
		" "},

	{124,	"C-ISAM ERROR - Beginning of transaction not found.",
		" ",
		" "},

	{125,	"C-ISAM ERROR - Cannot use network file server.",
		" ",
		" "},

	{126,	"C-ISAM ERROR - Bad record number.",
		" ",
		" "},

	{127,	"C-ISAM ERROR - No Primary Key.",
		" ",
		" "},

	{128,	"C-ISAM ERROR - No Logging.",
		" ",
		" "},

	{129,	"C-ISAM ERROR - Too Many users.",
		" ",
		" "},

	{130,	"C-ISAM ERROR - Dbspace not found.",
		" ",
		" "},

	{131,	"C-ISAM ERROR - No Free Disk space.",
		" ",
		" "},

	{132,	"C-ISAM ERROR - Record to long.",
		" ",
		" "},

	{133,	"C-ISAM ERROR - Audit trail exists.",
		" ",
		" "},

	{134,	"C-ISAM ERROR - No More Locks.",
		" ",
		" "},

	{6000,	"DATABASE ERROR - Database not yet opened",
		"Tried to perform an operation on an unopened file or database",
		" "},

	{6001,	"DATABASE ERROR - Database or file does not exist.",
		"A Manipulation, such as opening a file, has been attempted on",
		"a file that is not in the database that is currently open."},

	{6002,	"DATABASE ERROR - Tried to open more than one database.",
		"Only one database can be opened at once for any program",
		"written using Logistic-SQL Routines."}, 

	{6003,	"DATABASE ERROR - maximum number of files reached.",
		"The maximum number of files that may be open at one time",
		"would be exceeded if this request were processed."},

	{6004,	"DATABASE ERROR - Tried to open a file when no database was open",
		"Before a file is opened, a database must be opened so that",
		"Logistic-SQL can determine nature of the file."},
		
	{6005,	"DATABASE ERROR - Field could not be found.",
		"During the course of Logistic-SQL calls, the dictionary that",
		"is read did not include the field specified."},

	{6006,	"DATABASE ERROR - File has not been opened.",
		"Manipulation of a file has been attempted before opening it",
		" "},

	{6007,	"DATABASE ERROR - Field name cannot be found in current file.",
		"When a DBSELFIELD routine is called, the field specified must",
		"exist in the file that is specified in the call"},

	{6008,	"DATABASE ERROR - File has not been opened.",
		"When a DBSELFIELD routine is called, the field specified must",
		"be open at the time of the call."},

	{6009,	"DATABASE ERROR - No data in the file.",
		" ",
		" "},

	{6010,	"DATABASE ERROR - Value cannot be found.",
		"A Keyed search of a file has failed.",
		" "},

	{6011,	"DATABASE ERROR - End of file.",
		"The NEXT operation has failed because end of file was reached.",
		" "},
	{6012,	"DATABASE ERROR - Beginning of file.",
		"The PREVIOUS operation has failed because beginning of file ",
		"was reached."},

	{6014,	"DATABASE ERROR - No such flag value.",
		"The value passed to Logistic-SQL (SELFIELD) is incorrect",
		" "},

	{6015,	"DATABASE ERROR - Filename has not been opened.",
		"The file that is being manipulated in the DBFIND call must be",
		"opened before manipulation using the DBSELECT routine."}, 

	{6016,	"DATABASE ERROR - No view has been set.",
		"A view must be set for a file before manipulation using DBFIND",
		"call can be done."},
	{6017,	"DATABASE ERROR - Can't add duplicate value.",
		"An attempt was made add a duplicate value when the ",
		"no dups option is in effect."},

	{6018,	"DATABASE ERROR - Filename has not been opened.",
		"Before information can be added to a file it must be opened",
		"and have a view set for it using DBSELECT etc."},

	{6019,	"DATABASE ERROR - No View has been set.",
		"Before information can be added to a file it must be opened",
		"and have a view set for it using DBSETFILEVIEW etc."},
		
	{6020,	"DATABASE ERROR - There is no current record.",
		"The DBDELETE routine will delete the record most recently",
		"located by the DBFIND routine."},

	{6021,	"DATABASE ERROR - Filename has not been opened.",
		"Record cannot be deleted from a file until the file is ",
		"opened  and a view is set."},

	{6022,	"DATABASE ERROR - Lock was denied - deadlock avoidance.",
		"If this condition is returned, the program should attempt the",
		"lock again."},

	{6023,	"DATABASE ERROR - File has not been opened.",
		"Record cannot be deleted until file is opened.",
		" "},

	{6024,	"DATABASE ERROR - Memory allocation error, out of memory.",
		"A allocation of memory failed.",
		"Check Operating System Error for possible cause."},

	{6025,	"DATABASE ERROR - Tried to open a file more than once.",
		"A same file has been opened twice.",
		" "},

	{6026,	"DATABASE ERROR - The named field must be indexed.",
		"The field being keyed on is not indexed.",
		" "},

	{6027,	"DATABASE ERROR - File has not been locked.",
		" ",
		" "},

	{6028,	"DATABASE ERROR - File filename has not been opened.",
		" ",
		" "},

	{6029,	"DATABASE ERROR - No view has been set.",
		"When using LOGISTIC-SQL you must set view before any",
		"data can be manipulated."},

	{6030,	"C-ISAM ERROR - duplicate add to index.",
		"An attempt was made to add a duplicate value to an index",
		"via iswrite, isrewrite, isrewcurr or isaddindex"},

	{6031,	"DATABASE ERROR - Field has been selected.",
		" ",
		" "},

	{6032,	"DATABASE ERROR - INFORMIX Permission is denied.",
		"User does not have permission to Add records to database.",
		" "},

	{6035,	"DATABASE ERROR - There is no current record.",
		"The DBFIND routine has failed.",
		" "},

	{6036,	"DATABASE ERROR - New filename exists.",
		"The newname parameter already exists as a field of file name",
		" in the database. (See DBALIAS)"},

	{6037,	"DATABASE ERROR - New filename has already been used.",
		"The newname parameter is already in use within the program.",
		"See DBALIAS."},

	{6038,	"DATABASE ERROR - Alias cannot be alias of existing alias.",
		"The newname parameter is already in use within the program.",
		"See DBALIAS."},

	{6040,	"DATABASE ERROR - Old filename could not be found.",
		"The oldname parameter was not a file within the database.",
		"See DBALIAS"},

	{6041,	"DATABASE ERROR - The number if fields within a file > defined Max.",
		"The number of parameters is greater than the number of fields",
		"within the file."},

	{6043,	"DATABASE ERROR - Field is not a composite field type.",
		" ",
		" "},

	{6044,	"DATABASE ERROR - Field is not a composite field type.",
		" ",
		" "},

	{6045,	"DATABASE ERROR - INFORMIX Permission is denied.",
		"User does not have permission to Read records to database.",
		" "},

	{6046,	"DATABASE ERROR - File not opened exclusively.", 
		"When adding or deleting indexes from a file the file",
		"must be locked using an exclusive lock."},

	{6047,	"DATABASE ERROR - attempt to add index previously defined.",
		"An attempt was made to add an index that has been defined",
		"previously."},

	{6048,	"DATABASE ERROR - duplicate add to index.",
		"An attempt was made to add a duplicate value to an index",
		"via iswrite, isrewrite, isrewcurr or isaddindex."},

	{6049,	"DATABASE ERROR - INFORMIX Permission is denied.",
		"User does not have permission to change database structure.",
		" "},

	{6050,	"DATABASE ERROR - Bad Key description; internal Error.",
		"The format of the C-ISAM file has been corrupted.",
		"** WARNING THIS ERROR MUST BE LOOKED AT URGENTLY **"},

	{6051,	"DATABASE ERROR - Composite field names not allowed view.",
		" ",
		" "},

	{6052,	"DATABASE ERROR - Not all of the composite are in the view.",
		" ",
		" "},


	{0,	"command executed successfully",
		" ",
		" "}
};
