int	AUDIT_ON = TRUE;

/*==================
| Open audit file. |
==================*/
open_audit (char *co_no, char *aud_fname)
{
	char	*getenv(),
		*sptr = getenv( "PROG_PATH" );

	char	aud_dir[100],
		aud_file[100],
		err_name[100],
		cmd[100];

	upshift( aud_fname );

	sprintf( aud_dir,"%s/BIN/AUDIT/%s",
			( sptr == ( char *) 0) ? "/usr/DB" : sptr,
			aud_fname );

	downshift( aud_fname );

	sprintf(aud_file,"%s/%02d.%s.audit", 
			aud_dir, atoi( comm_rec.tco_no ), aud_fname );

	sprintf(err_name,"%s/%02d.%s.err", 
			aud_dir, atoi( comm_rec.tco_no ), aud_fname );

	/*-------------------------------------------
	| Audit file directory not exist so create. |
	-------------------------------------------*/
	if ( access( aud_dir, 00) < 0 )
	{
		AUDIT_ON = FALSE;
		return;
	}

	/*--------------------------------------
	| Audit file does not exist so create. |
	--------------------------------------*/
	if ( access( aud_file, 00) < 0 )
	{
		cc = RF_OPEN(aud_file,sizeof(wk_rec),"w",&wk_no);
		if ( cc )
			file_err( cc, aud_file, "WKOPEN" );
	}
	else
	{
		/*----------------------------------
		| Open audit file to amend to end. | 
		----------------------------------*/
		cc = RF_OPEN(aud_file,sizeof(wk_rec),"a",&wk_no);
		if ( cc )
		{
			/*-------------------------------------------
			| Audit file corrupt so move and re-create. |
			-------------------------------------------*/
			sprintf( cmd, "mv %s %s", aud_file, err_name);
			sys_exec( cmd );
				
			cc = RF_OPEN(aud_file,sizeof(wk_rec),"w",&wk_no);
			if ( cc )
				file_err( cc, aud_file, "WKOPEN" );
		}
	}
}
