#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlcli1.h>



int main()
{

	SQLRETURN rc;
	SQLCHAR dbname[SQL_MAX_DSN_LENGTH];
	SQLCHAR tblname;

	strcpy(dbname,"SAMPLE");
	
	rc = abc_dbopen(dbname);
	/*printf("This is the value of rc %d \n",rc);*/
	if (rc != SQL_SUCCESS)
	{
		printf("There was an error connecting to database %s \n", dbname);
	}
	else

		printf("Connected to database %s \n",dbname);
		
		/*strcpy(tblname,"EMPLOYEE");
		/*rc = find_rec(tblname,*/


}
