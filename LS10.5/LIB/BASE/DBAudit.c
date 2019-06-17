/*=====================================================================
|  Copyright (C) 2000 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: DBAudit.c,v 5.4 2002/04/11 03:50:54 scott Exp $
|=====================================================================|
|  Program Name  : (DBAudit.c)
|  Program Desc  : (Audit Routines )
|=====================================================================|
| $Log: DBAudit.c,v $
| Revision 5.4  2002/04/11 03:50:54  scott
| Updated to add comments to audit files.
|
| Revision 5.3  2001/09/27 06:56:12  cha
| Corrected some memory handling issues.
|
| Revision 5.2  2001/09/27 02:58:25  cha
| Replace pipe delimeter with ^A in audit file.
| Also added headers.
|
|
|
=====================================================================*/

#include <sys/stat.h>
#include <fcntl.h>
#include <std_decs.h>
#include <DBAudit.h>

#define MAX_BUFFER_SIZE 255

int firstTime = TRUE,
	errNo = 0;
void *OldRec;

FILE *fName;

int
OpenAuditFile (
    const char *FileName)
{
    char pathName[255];
    struct stat info;
    int fd = 0;

    strcpy (pathName, getenv ("PROG_PATH"));
    sprintf (pathName, "%s/BIN/AUDIT/%s", pathName, FileName);


    if (stat (pathName, &info))
    {
        if (errno == ENOENT)
        {
            fd = creat (pathName, 0666);
            if (fd < 0)
                file_err (errno, "OpenAuditFile","creat");
            close (fd);
        }
    }
 

    if (!pathName)
        return (EXIT_FAILURE);

    fName = fopen (pathName, "a+");

    if (!fName)
        file_err (errno, "OpenAuditFile","fopen");

    OldRec = malloc (MAX_BUFFER_SIZE);

    return (EXIT_SUCCESS);

}

int AuditFileAdd  (
	char	*recordComment,
    void    *newRec,
    struct  dbview  *TableList,
    const int numfields)
{
    int ctr = 0, length = 0;
    char newVal [255], oldVal [255];


    char *currentUserName = getenv ("LOGNAME");

    if (firstTime)
    {
        fprintf (fName, "AUDIT-USER-NAME=%s\n", currentUserName);
        fprintf (fName, "AUDIT-DATE=%ld\n", TodaysDate());
        fprintf (fName, "AUDIT-TIME=%s\n", TimeHHMMSS ());
        fprintf (fName, "AUDIT-COMMENT=%s\n", recordComment);
        fprintf (fName, "AUDIT-FIELD%cAUDIT-BEFORE%cAUDIT-AFTER\n",1,1);
    }

    for (ctr = 0; ctr <= numfields - 1; ctr++)
    {
        switch (TableList->vwtype)
        {
            case CHARTYPE:
                 length = TableList->vwlen;
                 sprintf (newVal, "%s", (char *) newRec + TableList->vwstart);
                 sprintf (oldVal, "%s", (char *) OldRec + TableList->vwstart);
                 break;

            case INTTYPE:
                 sprintf (newVal, "%d",* (int *)( newRec + TableList->vwstart));
                 sprintf (oldVal, "%d",* (int *)( OldRec + TableList->vwstart));
                 break;

            case LONGTYPE:
            case SERIALTYPE:
            case DATETYPE:
            case EDATETYPE:
            case YDATETYPE:
                 sprintf (newVal, "%ld",* (long *)( newRec + TableList->vwstart));
                 sprintf (oldVal, "%ld",* (long *)( OldRec + TableList->vwstart));
                 break;

            case FLOATTYPE:
                 sprintf (newVal, "%f",* (float *)( newRec + TableList->vwstart));
                 sprintf (oldVal, "%f",* (float *)( OldRec + TableList->vwstart));
                 break;

            case DOUBLETYPE:
            case MONEYTYPE:
                 sprintf (newVal, "%g",* (double *)( newRec + TableList->vwstart));
                 sprintf (oldVal, "%g",* (double *)( OldRec + TableList->vwstart));
                 break;
        }
        if (strcmp(oldVal, newVal))
        {
            fprintf(fName, "%s%c%s%c%s\n",TableList->vwname,1, oldVal,1, newVal);
        }
        TableList++;
    }

    firstTime = FALSE;
    return (EXIT_SUCCESS);
}

int
CloseAuditFile (void)
{
    errNo = fclose (fName);
    if (errNo)
        file_err (errno, "CloseAuditFile","fclose");

    return (EXIT_SUCCESS);
}

void SetAuditOldRec (
    void *oldRec, int size)
{
    if (OldRec)
        free (OldRec);

    if (!(OldRec = malloc (size)))
            sys_err ("Error in SetAuditOldRec during memory allocation",errno,"SetAuditOldRec");

    memcpy (OldRec, oldRec, size);
}


