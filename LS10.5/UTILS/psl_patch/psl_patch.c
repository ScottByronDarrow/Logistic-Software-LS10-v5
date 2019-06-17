#define	CCMAIN
#include	<pslscr.h>
#include	<ml_utils_mess.h>
#include	<ml_std_mess.h>
char	*PNAME = "$RCSfile: psl_patch.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_patch/psl_patch.c,v 5.1 2001/08/09 09:27:32 scott Exp $";


/*=========================
| Main Processing Routine | 
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	fd;
	long	offset;
	char	in_buffer[81];
	char	out_buffer[81];

	if (argc != 4)
	{
		/*printf("Usage : %s <filename> <offset> <string>\007\n\r",argv[0]);*/
		print_at(0,0,ML(mlUtilsMess714),argv[0]);
		return (EXIT_FAILURE);	
	}

	init_scr();
	set_tty();
	clear();

	fd = open(argv[1],2);

	if (fd < 0)
	{
		sprintf(err_str,"Error in %s during (FOPEN)",argv[1]);
		sys_err(err_str,errno,PNAME);
	}

	offset = atol(argv[2]);

	sprintf(in_buffer,"%-.80s",argv[3]);
	length = strlen(in_buffer);

	if (lseek(fd,offset,0) != offset)
	{
		sprintf(err_str,"Error in %s during (LSEEK)",argv[1]);
		sys_err(err_str,errno,PNAME);
	}
	
	if (read(fd,out_buffer,length) != length)
	{
		sprintf(err_str,"Error in %s during (READ)",argv[1]);
		sys_err(err_str,errno,PNAME);
	}

	print_at(0,0,ML(mlUtilsMess143),out_buffer);
	print_at(1,0,ML(mlUtilsMess144),in_buffer);
/*
	print_at(0,0,"Replace [%s]\n\r",out_buffer);
	print_at(1,0,"By      [%s]",in_buffer);
	last_char =  prmptmsg("Replace ? ","YyNn",5,4);*/

	last_char =  prmptmsg(ML(mlUtilsMess145),"YyNn",5,4);

	if (last_char == 'Y' || last_char == 'y')
	{
		/*print_at(3,0,"\n\r\n\rModified ... ");*/
		print_at(3,0,ML(mlUtilsMess078));
		if (lseek(fd,offset,0) != offset)
		{
			sprintf(err_str,"Error in %s during (LSEEK)",argv[1]);
			sys_err(err_str,errno,PNAME);
		}
		
		if (write(fd,in_buffer,length) != length)
		{
			sprintf(err_str,"Error in %s during (READ)",argv[1]);
			sys_err(err_str,errno,PNAME);
		}
	}

	close(fd);
	
	rset_tty();
	return (EXIT_SUCCESS);
}
