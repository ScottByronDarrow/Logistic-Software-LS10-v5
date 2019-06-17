/*================================================
| Error codes for INFORMIX used in 'C' programs. |
================================================*/
#define DEADLOCK   45   /* System Deadlock. Was 36 now 45                     */
#define ISDUPADD   100  /* ISAM duplicate add to index.                       */
#define ISAMOPEN   101  /* ISAM file not previously opened.                   */
#define ISAMCALL   102  /* ISAM call arguments not acceptable.                */
#define ISAMKEY    103  /* ISAM key element(s) not acceptable.                */
#define ISAMMAX    104  /* ISAM maximum number of files reached.              */
#define ISAMCORP   105  /* ISAM format of file corrupted.                     */
#define ISAMEXCU   106  /* ISAM add/delete index without exclusive access.    */
#define RECLOCK    107  /* ISAM record locked by another process.             */
#define INDEXDEF   108  /* ISAM attempt to add index previously defined.      */
#define DELPRIME   109  /* ISAM attempt to delete primary key value.          */
#define NOCRU      110  /* ISAM End of file.                           	      */
#define ISAMEOF    110  /* ISAM begining or end of file reached.              */
#define NOKEY      111  /* ISAM no match for requested value.                 */
#define NOURUN     112  /* ISAM no current of run unit.                       */
#define FILELOCK   113  /* ISAM file locked by another process.               */
#define NOTOPEN    6000 /* Tried to close an unopened file or database.       */
#define NOTEXIST   6001 /* The database or file does not exist.               */
#define TWODATA    6002 /* Tried to open more than one database.              */
#define MAXFILES   6003 /* Tried to open more than five files.                */
#define NODATABASE 6004 /* Tried to open a file when no database was open.    */
#define NOFIELD    6005 /* A field cannot be found.                           */
#define NOFILENAME 6006 /* Filename has not been opened.                      */
#define NOFNAME    6007 /* Field name cannot be found in current file.        */
#define NOOPENSEL  6008 /* File has not been opened (DBSELFIELD).             */
#define NODATA     6009 /* No data in the file.                               */
#define NOVAL      6010 /* Value cannot be found.                             */
#define FILEEND    6011 /* End of file.                                       */
#define FILEBEG    6012 /* Beginning of file.                                 */
#define NOFLAGVAL  6014 /* No such flag value.                                */
#define NOFILEFIND 6015 /* Filename has not be opened (DBFIND).               */
#define NOVIEWFIND 6016 /* No view has been set (DBFIND).                     */
#define DUPADD     6017 /* Can't add a duplicate value(no dups option)(DBADD).*/
#define NOFILEADD  6018 /* Filename has not been opened (DBADD).              */
#define NOVIEWADD  6019 /* No view has been set (DBADD).                      */
#define NORECORD   6020 /* No current record (DBUPDATE,DBDELETE).             */
#define NOFILEDEL  6021 /* Filename has not been opened (DBDELETE).           */
#define LOCKDENIED 6022 /* Lock denied - deadlock avoidance.                  */
#define NOFILELOCK 6023 /* Filename has not been opened (DBLOCK).             */
#define MAXFIELDS  6024 /* The number of fields may not exceed 100.           */
#define OPENTWO    6025 /* Tried to open a file more than once.               */
#define KEYFIELD   6026 /* Field must be keyed or chronol is to be select     */
#define FILENAMELK 6027 /* Filename has not been locked.                      */
#define NOFILEUPDT 6028 /* Filename has not been opened (DBUPDATE).           */
#define NOVIEWSET  6029 /* No view has been set.                              */
#define DUPSUP     6030 /* Can't add a duplicate value (no dups)(DBUPATE).    */
