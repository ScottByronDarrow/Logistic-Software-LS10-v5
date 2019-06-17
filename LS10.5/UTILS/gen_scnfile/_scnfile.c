/*=====================================================================
|  Copyright (C) 1988 Logistic Software Limited.                      |
|=====================================================================|
|  Program Name  : (gen_scnfile.c )                                   |
|  Program Desc  : (Display Contents of static structure        )     |
|                  (from text file..                            )     |
|---------------------------------------------------------------------|
|  Author        : Rommel Maldia,  | Date Written  : 23/02/95         |
|---------------------------------------------------------------------|
|  Date Modified : (12/09/97)      | Modified By:  Marnie Organo      |
|  Date Modified : (06/10/1999)    | Modified By:  Ramon A. Pacheco   |
|                                                                     |
|  Comments      :                                                    |
|  (12/09/97)    :  Updated for Multilingual Conversion.              |
|  (06/10/1999)  :  Ported to ANSI standards.                         |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _scnfile.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/gen_scnfile/_scnfile.c,v 5.2 2001/08/09 09:26:56 scott Exp $";

#include	<pslscr.h>
#include 	<stdlib.h>
#include 	<ml_utils_mess.h>
#include 	<ml_std_mess.h>

FILE *	fsource,
	 *	fdest;

extern	int		optind;
extern	char *	optarg;

int 	count 		= 0,	
		newline 	= FALSE,
		first_entry = TRUE,
		on_entry	= TRUE,
		cstruct		= FALSE;

int		sstruct	= FALSE,
		head	= TRUE,
		brace	= FALSE,
		first	= FALSE,
		finis	= FALSE,
		second	= FALSE,
		third	= FALSE,
		last	= FALSE,
		frword	= TRUE,
		sword	= FALSE,
		tword	= FALSE,
		fword	= FALSE,
		ffword	= FALSE,
		Txt		= FALSE,
		fourth	= FALSE;

char	file_name [21],
		wk_str [1200],
		work_str [1200],
		sptr [1200];

/*===========================
| Local function prototypes |
===========================*/
int		test			 (char *);
int		test_newline	 (char *, int);
int		arrange			 (char *);
int		buildscr		 (char *);
int		wr_tof			 (char *);
int		clear_ch		 (char *);
int		sp_ch			 (char *);
int		spaces			 (char *, int);
int		tested			 (char *);
int		untab			 (char *);
int		copy_last		 (char *, int);

int
main (
 int	argc,
 char *	argv [])
{
	int		i, 
			result = 0, 
			ctr = 0;
	char	ssptr [1200],
			cptr;
	
	/*---------------------------------------
	| check for 'c' or 's' parameter	|
	---------------------------------------*/
	while ((i = getopt (argc, argv, "cs")) != EOF)
	{
		switch (i)
		{
		case	'c':
			cstruct = TRUE;
			break;

		case	's':
			sstruct = TRUE;
			break;

		case	'?':
			print_at (0,0,mlUtilsMess705, argv[0]);
			return (EXIT_FAILURE);
		}
	}

	if ((!sstruct && !cstruct) || (sstruct && cstruct) || optind > argc - 1)
	{
		print_at (0,0,mlUtilsMess705, argv[0]);
		print_at (1,0,mlUtilsMess706);
		print_at (2,0,mlUtilsMess707);
		return (EXIT_FAILURE);
	}

	sprintf (file_name, "%-.20s", argv[optind]);
	/*---------------------------------------
	| open input file (schema)		|
	---------------------------------------*/
	if ((fsource = fopen (argv[optind], "r")) == 0)
	{
		sprintf (err_str, "Can't open source file %s\n", argv[optind]);
		sys_err (err_str, errno, PNAME);
	}

	strcpy (sptr, "");
	strcpy (wk_str, "");
	strcpy (work_str, "");

	if (cstruct)
		fdest = fopen ("static.vars", "w+");
	else if (sstruct)
		fdest = fopen ("screen.s","w+");

	while (!feof (fsource) && last != TRUE)
	{
		cptr = fgetc (fsource);	
		ssptr[ctr] = cptr;
		++ctr;

		if (cptr == '\n')
		{
			ssptr[ctr] = '\0';
			++ctr;
			if (on_entry == TRUE)
			{
				result = test (ssptr);
				strcpy (ssptr, wk_str);
				strcat (ssptr, work_str);
				if (result)
					on_entry = FALSE;
			}

			if (result || on_entry == FALSE)
			{
				if (cstruct)
					arrange (ssptr);
				else if (sstruct)
					buildscr (ssptr);
			}
			ctr = 0;
		}
	}

	fclose (fsource);
	fclose (fdest);

	return (EXIT_SUCCESS);
}

int
test (
 char *	line)
{
	char	irr_str[1200];
	int		i, ctr = 0;

	for (i=0; line[i]; i++)
	{
		irr_str[ctr] = line[i];
		++ctr;
		irr_str[ctr] = '\0';
		
		if ((line[i] == ' ' || line[i] == '\t' 
			|| line[i] == '\n'))	
		{
			irr_str[ctr]='\0';

			if (!strcmp (irr_str," ") || !strcmp (irr_str,"\n") ||
				!strcmp (irr_str,"\t"))
				newline = TRUE;

			if (!strncmp (irr_str, "static", 6) && frword)
			{
				frword = FALSE;
				sword = TRUE;
				strcpy (wk_str,irr_str);

			}
			if ((!strncmp (irr_str, "struct", 6) && sword) || 
				 (sword && !strncmp (irr_str, "struct", 6) && newline == TRUE))
			{
				sword = FALSE;
				tword = TRUE;
				strcat (wk_str, irr_str);
				if (test_newline (line,i))
					newline = TRUE;
				else
					newline = FALSE;
			}
			if ((!strncmp (irr_str, "var", 3) && tword) ||
				 (tword && !strncmp (irr_str, "var", 3) && newline == TRUE))
			{
				tword = FALSE;
				fword = TRUE;
				strcat (wk_str, irr_str);
				if (test_newline (line,i))
					newline = TRUE;
				else
					newline = FALSE;
			}
			if ((!strncmp (irr_str, "vars[]", 6) && fword) ||
				 (fword && !strncmp (irr_str, "vars[]", 6) 
					&& newline == TRUE))
			{
				fword = FALSE;
				ffword = TRUE;
				strcat (wk_str, irr_str);
				if (test_newline (line,i))
					newline = TRUE;
				else
					newline = FALSE;
			}
			if ((!strncmp (irr_str, "=", 1) && ffword) || 
				 (ffword && !strncmp (irr_str, "=", 1) && newline == TRUE))
			{
				ffword = FALSE;
				strcat (wk_str,irr_str);
				
				if (!test_newline (line,i))
					copy_last (line,i); 

				return (EXIT_FAILURE);
			}
			if ((!strncmp (irr_str, "vars[]=", 7) && ffword) || 
				 (ffword && !strncmp (irr_str, "vars[]=", 7) 
					&& newline == TRUE))
			{
				ffword = FALSE;
				strcat (wk_str,irr_str);
				
				if (!test_newline (line,i))
					copy_last (line,i); 

				return (EXIT_FAILURE);
			}
			if ((!strncmp (irr_str, "vars", 4) && fword) || 
				 (fword && !strncmp (irr_str, "vars", 4) && newline == TRUE))
			{
				fword = TRUE;
				strcat (wk_str, irr_str);
				if (test_newline (line,i))
					newline = TRUE;
				else
					newline = FALSE;
			}
			if ((!strncmp (irr_str, "[]", 2) && fword) ||
				 (fword && !strncmp (irr_str, "[]", 2) && newline == TRUE)) 
			{
				fword = FALSE;
				ffword = TRUE;
				strcat (wk_str, irr_str);
				if (test_newline (line,i))
					newline = TRUE;
				else
					newline = FALSE;
			}
			if ((!strncmp (irr_str, "[]=", 3) && fword) ||
			    (fword && !strncmp (irr_str, "[]=", 3) && newline == TRUE))
			{
				fword = FALSE;
				ffword = FALSE;
				strcat (wk_str, irr_str);
				
				if (!test_newline (line,i))
					copy_last (line,i); 

				return (EXIT_FAILURE);
			}
		ctr = 0;
		}
	}
	return (EXIT_SUCCESS);
}

int
test_newline (
 char *	word,
 int	num)
{
	while (word[num] == ' ' || word[num] == '\t')
	{
		++num;
		if (word[num] == '\n')
			return (EXIT_FAILURE);
	}

	if (word[num] == '\n')
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}	

int
arrange (
 char *	line)
{
	int 	end = FALSE,
			written = FALSE,
			cincr = 0,
			ctr = 0;

	char	cptr='a',
			temp_str[1200];

	strcat (sptr, line);
	while (cptr != '\0' && !end)
	{
		written = FALSE;	
		cptr = sptr [cincr];
		++cincr;
		
		temp_str[ctr]=cptr;
		++ctr;
		temp_str[ctr]='\0';
		
		if (((temp_str[ctr -1] == ' ' || temp_str[ctr-1] == '\t' || 
			 temp_str[ctr-1] == '\n') && (head || brace)) || 

			 ((temp_str[ctr-1] == ',' && !tested (temp_str)) || 
			 (tested (temp_str) && temp_str[ctr-1] == ',' && 
			temp_str[spaces (temp_str,ctr)] == '"' && 
			spaces (temp_str,ctr)!= 0)))
			
		{
			if (temp_str [ctr-1] == '\t' || temp_str[ctr-1] == ' ' 
				|| temp_str[ctr-1] == '\n')
				temp_str[ctr-1]='\0';
			else
				temp_str[ctr] = '\0';
			
			strcpy (temp_str,clip (temp_str));
			while (temp_str[0] == ' ' || temp_str[0] == '\t' || 
					temp_str[0] == '\n')
			{
				strcpy (temp_str,lclip (temp_str));
				untab (temp_str);
				strcpy (temp_str,wk_str);
			}

			++ctr;

			if (head && count <= 5)
			{
				++count;
				fprintf (fdest, "%s\t", temp_str);
				if (count == 5)
				{
					fprintf (fdest, "\n");
					head = FALSE;
					brace = TRUE;
					count = 0;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}

			else if (!end && brace && count <= 1)
			{
				++count;
				fprintf (fdest, "%s\n", temp_str);
				first = TRUE;
				brace = FALSE;
				count = 0;
				if (cincr < (int)strlen (line))
					end = FALSE;
				else
					end = TRUE;
			}

			else if (!end && first && count <= 6)
			{
				++count;
				if (first_entry)
				{
					fprintf (fdest, "\t%s ", temp_str);
					first_entry = FALSE;
					if (temp_str[0] == '"')
						written = TRUE;
				}
				else
				{
			    	fprintf (fdest, "%s ", temp_str);
					if (temp_str[0] == '"')
						written = TRUE;
				}

				if (count == 6)
				{
			    	fprintf (fdest, "\n");
					first = FALSE;
					second = TRUE;
					count = 0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}
			else if (!end && second && count <= 2)
			{
				++count;
				if (first_entry)
				{
					fprintf (fdest, "\t\t%s ", temp_str);
					first_entry = FALSE;
					if (temp_str[0] == '"')
						written = TRUE;
				}
				else
				{
					fprintf (fdest, "%s ", temp_str);
					if (temp_str[0] == '"')
						written = TRUE;
				}

				if (count == 2)
				{
					fprintf (fdest, "\n");
					second = FALSE;
					third = TRUE;
					count = 0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}

			else if (!end && third && count <= 4)
			{
				++count;
				if (first_entry)
				{
					fprintf (fdest, "\t\t%s ", temp_str);
					first_entry = FALSE;
					if (temp_str[0] == '"')
						written = TRUE;
				}
				else
				{
					fprintf (fdest, "%s ", temp_str);
					if (temp_str[0] == '"')
						written = TRUE;
				}

				if (count == 4)
				{
					fprintf (fdest, "\n");
					third = FALSE;
					fourth = TRUE;
					count = 0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}

			else if (!end && fourth && count <= 6)
			{
				++count;
				if (first_entry)
				{
					fprintf (fdest, "\t\t%s ", temp_str);
					first_entry = FALSE;
					if (temp_str[0] == '"')
						written = TRUE;
				}
				else
				{
					fprintf (fdest, "%s ", temp_str);
					if (temp_str[0] == '"')
						written = TRUE;
				}
				if (count == 6)
				{
					fprintf (fdest, "\n");
					fourth = FALSE;
					first = TRUE;
					count =0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}
		if (temp_str[0] != '"' || written)
			ctr = 0;
		}

		else if (cptr == '}')
		{
			fprintf (fdest,"\n};\n");
			last = TRUE;
			return (EXIT_SUCCESS);
		}
	}

	ctr =0;
	
	strcpy (sptr, "");

	return (EXIT_SUCCESS);
}

int
buildscr (
 char *	line)
{
	int 	end = FALSE,
			written = FALSE,
			cincr = 0,
			ctr = 0;

	char	cptr='a',
			temp_str[1200];

	strcat (sptr, line);
	while (cptr != '\0' && !end)
	{
		written = FALSE;	
		cptr = sptr [cincr];
		++cincr;
		
		temp_str[ctr]=cptr;
		++ctr;
		temp_str[ctr]='\0';
	
	
		if (((temp_str[ctr -1] == ' ' || temp_str[ctr-1] == '\t' || 
			 temp_str[ctr-1] == '\n') && (head || brace)) || 

			 ((temp_str[ctr-1] == ',' && !tested (temp_str)) || 
			 (tested (temp_str) && temp_str[ctr-1] == ',' && 
			temp_str[spaces (temp_str,ctr)] == '"' && 
			spaces (temp_str,ctr)!= 0)))
			
		{
			if (temp_str [ctr-1] == '\t' || temp_str[ctr-1] == ' ' 
				|| temp_str[ctr-1] == '\n')
				temp_str[ctr-1]='\0';
			else
				temp_str[ctr] = '\0';
			
			strcpy (temp_str,clip (temp_str));
			while (temp_str[0] == ' ' || temp_str[0] == '\t' || 
					temp_str[0] == '\n')
			{
				strcpy (temp_str,lclip (temp_str));
				untab (temp_str);
				strcpy (temp_str,wk_str);
			}

			++ctr;

			if (head && count <= 5)
			{
				++count;
				if (count == 5)
				{
					brace = TRUE;
					head = FALSE;
					count = 0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}
					
			else if (!end && brace && count <= 1)
			{
				++count;
				first = TRUE;
				brace = FALSE;
				count = 0;
				if (cincr < (int)strlen (line))
					end = FALSE;
				else
					end = TRUE;
			}


			else if (!end && first && count <= 6)
			{
				++count;

				if (temp_str[0] == '"')
					written = TRUE;

				if (count == 2 && temp_str[1] == 'X') 
					Txt = TRUE;
				else if (count == 2 && temp_str[1] != 'X')
					Txt = FALSE;

				if ((count == 3 || count == 4 || count == 5) && (!Txt))
					wr_tof (temp_str);
			
				else if (count == 6)
				{
					first = FALSE;
					second = TRUE;
					count = 0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}
			else if (!end && second && count <= 2)
			{
				++count;

				if (temp_str[0] == '"')
					written = TRUE;

				if (count == 2)
				{
					second = FALSE;
					third = TRUE;
					count = 0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}

			else if (!end && third && count <= 4)
			{
				++count;

				if (temp_str[0] == '"')
					written = TRUE;

				if (count == 3 && (!Txt))
					wr_tof (temp_str);
				
				else if (count == 4)
				{
					third = FALSE;
					fourth = TRUE;
					count = 0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}

			else if (!end && fourth && count <= 6)
			{
				++count;

				if (temp_str[0] == '"')
					written = TRUE;

				if (count == 1 && (!Txt))
					wr_tof (temp_str);

				else if (count == 6)
				{
					fourth = FALSE;
					first = TRUE;
					count =0;
					first_entry = TRUE;
					if (cincr < (int)strlen (line))
						end = FALSE;
					else
						end = TRUE;
				}
			}
		if (temp_str[0] != '"' || written)
			ctr = 0;
		}

		else if (cptr == '}')
		{
			last = TRUE;
			return (EXIT_SUCCESS);
		}
	}

	ctr =0;
	
	strcpy (sptr, "");

	return (EXIT_SUCCESS);
}

int
wr_tof (
 char *	word)
{
	int	tav, sp, label, i;

	if (first && count == 3)
	{
		clear_ch (word);
		strcpy (work_str,wk_str);
	}

	else if (first && count == 4)
	{
		clear_ch (word);
		label = strlen (work_str);
		sp = 19 - label;
		if (label < 8)
			tav = 4;
		
		else if (label < 17)
			tav = 3;

		else 
			tav = 2;
	       		
		if (tav <= 0 && sp <=0)
			fprintf (fdest,"\t");
		else
			for (i=1;i<=tav;++i)
				strcat (work_str,"\t");

		strcat (work_str,wk_str);
	}

	else if (first && count == 5)
	{
		clear_ch (word);
		strcat (work_str,"\t");

		strcat (work_str,wk_str);
	}
    	    	
	else if (third && count == 3)
	{
		clear_ch (word);

		if (!strcmp (wk_str, ""))
			strcpy (wk_str, "LEAVE");
		if (!strcmp (wk_str,"dummy"))
			finis = TRUE;
	
		if (!finis)
		{

			fprintf (fdest,"%s",wk_str);
			label = strlen (wk_str);
			sp = 25 - label;
			if (label < 8)
				tav = 4;
			
			else if (label < 17)
				tav = 3;

			else 
				tav = 2;
		
			if (tav <= 0 && sp <=0)
				fprintf (fdest,"\t");
			else
				for (i=1;i<=tav;++i)
					fprintf (fdest,"\t");

			fprintf (fdest,"%s",work_str);
		}
	}

	else if (fourth && count == 1)
	{
		if (!finis)
		{
			clear_ch (word);
			fprintf (fdest,"\t\t");


			if (!strncmp (wk_str, "YES", 3))
				fprintf (fdest,"REQUIRED \n");

			else if (!strncmp (wk_str, "NO", 2))
				fprintf (fdest, "INPUT \n");	
		
			else if (!strncmp (wk_str, "NE", 2))
				fprintf (fdest, "NOEDIT \n");	

			else if (!strncmp (wk_str, "NA", 2))
				fprintf (fdest, "DISPLAY \n");	

			else if (!strncmp (wk_str, "NI", 2))
				fprintf (fdest, "NOINPUT \n"); 

			else if (!strncmp (wk_str, "ND", 2))
				fprintf (fdest, "HIDE \n"); 
		}
	}
	return (EXIT_SUCCESS);
}

int
clear_ch (
 char *	word)
{
	int		i;


	for (i=0; word[i]; ++i)
		if (word[i] == ',' || word[i] == '"')
			word[i] = ' ';
			
	strcpy (word,clip (word));
	while (word[0] == ' ' || word[0] == '\t' || 
			word[0] == '\n')
	{
		strcpy (wk_str,word);
		strcpy (wk_str,lclip (wk_str));
		
		strcpy (wk_str,clip (wk_str));
		if (wk_str[0] == ':')
			break;
		strcpy (word,lclip (word));
		untab (word);
		strcpy (word,wk_str);
	}
	strcpy (wk_str, word);

	return (EXIT_SUCCESS);
}

int
sp_ch (
 char *	word)
{
	int	i;

	for (i=0; word[i]; ++i)
		if (word[i] == ',' || word[i] == '"')
			word[i] = ' ';
			
	strcpy (word, clip (word));
	strcpy (word, lclip (word));
	strcpy (wk_str, word);

	return (EXIT_SUCCESS);
}

int
spaces (
 char *	word,
 int	num)
{
	int		i = 1;

	--num;
	while (word[num -1] == ' ' || word[num-1] == '\t' || word[num-1] == '\n')
	{
		++i;
		if (word[num-i] == '"')
			return (num - i);
	}
	if (word[num-i] == '"')
		return (num - i);

	return (num);
}

int
tested (
 char *	word)
{
	int	i = 0;
	
	while (word [i] == ' ' || word[i] == '\t' || word[i] == '\n')
	{
		++i;
		if (word [i] == '"')
			return (EXIT_FAILURE);
	}
	if (word[i] == '"')
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

int
untab (
 char *	word)
{
	int	i;
	
	strcpy (wk_str, "");
	if (word [0] == '\t' || word [0] == '\n')
	{
		for (i = 0;word[i]; i++)
			wk_str[i] = word[i+1];
	}
	else
		strcpy (wk_str,word);

	return (EXIT_SUCCESS);
}

int 
copy_last (
 char *	word,
 int	num)
{
	int	i = 0, 
		lght = 0,
		n = 0;
	
	lght = strlen (word);

	for (i=num+1;i<=lght;++i) 
	{
		work_str[n] = word[i];		 	
		++n;
	}
	work_str[n] ='\0';
	return (EXIT_SUCCESS);
}
