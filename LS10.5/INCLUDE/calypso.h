/*
$Log: calypso.h,v $
Revision 5.0  2001/06/19 06:51:27  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 00:59:22  scott
LS10-4.0 New Release as at 10th March 2001

Revision 3.0  2000/10/12 13:28:52  gerry
Revision No. 3 Start
<after Rel-10102000>

Revision 2.0  2000/07/15 07:15:36  gerry
Force revision no. to 2.0 - Rel-15072000

Revision 1.13  1999/12/06 01:22:14  scott
Updated to change CTime () to SystemTime due to problems with Conflicts in VisualC++

Revision 1.12  1999/11/09 12:22:15  gerry
Updated for -Wall warnings

Revision 1.11  1999/10/20 07:20:58  nz
Updated for date routines

Revision 1.10  1999/10/16 02:19:32  scott
Updated for erorrs

Revision 1.9  1999/10/16 02:13:48  nz
Updated for compile errors

Revision 1.8  1999/10/16 02:10:24  nz
ss:wq

Revision 1.7  1999/09/14 00:30:35  scott
Updated for Anci

Revision 1.5  1999/09/06 04:55:56  primo
Add message on timeoutx ()

Revision 1.4  1999/09/06 04:54:05  primo
Primo, general checking.

*/
#ifndef DRL_H
#define DRL_H

#define DRL_NO_ERROR            2000
#define DRL_NO_CONNECTION       2001
#define DRL_SERVER_ERROR        2002
#define DRL_INVAL_COMMAND       2003
#define DRL_INVAL_DATA_SOURCE   2004
#define DRL_INVAL_TABLE_NAME    2005
#define DRL_TABLE_DEFINED       2006
#define DRL_TABLE_NOT_DEFINED   2007
#define DRL_INVAL_FIELD_NAME    2008
#define DRL_INVAL_INDEX_NAME    2009
#define DRL_INVAL_RECORD        2010
#define DRL_INVAL_FIND_TYPE     2011
#define DRL_INVAL_LOCK_MODE     2012

#endif /* DRL_H */

#ifdef	SCK_LOGGING
extern FILE *	fLog;
#endif /* SCK_LOGGING */

#define	HEADER_LEN	8

char	readBuf [4096];

char	*inet_ntoa (struct in_addr ptr);
long	inet_addr (char	*iptr);
int		ResolveProtocol (char   *protocolName);
/*=====================================================
| Set up a socket to establish an outgoing connection |
=====================================================*/
int
SetTransmit (char *hostName, int port, char * servName)
{
    int     nProtocol;
    int     transmitStatus;
	char	ipAddr[21];
	struct  hostent 	*xmitHost;
    struct  in_addr		*ptr;

    /*-----------------
    | Initialisation. |
    -----------------*/

	xmitHost = gethostbyname (hostName);
	if (xmitHost == NULL)
        return SERR_HOST_NOT_FOUND;

    ptr = (struct in_addr *) *(xmitHost->h_addr_list);
	if (ptr == NULL)
        return SERR_HOST_NOT_FOUND;

	sprintf (ipAddr, "%s", inet_ntoa (*ptr));
/*
 print_at (0, 0, "IP Addr [%s]", ipAddr); getchar ();
*/

/* Clear out the structure
	bzero((caddr_t *) &transmitAddr, sizeof(transmitAddr));
*/
	memset ((caddr_t *) &transmitAddr, '\0', sizeof (transmitAddr));

	transmitAddr.sin_family = AF_INET;     /* set the family (only AF_INET
                                         * is valid
                                         */
	transmitAddr.sin_addr.s_addr = inet_addr (ipAddr);
                                        /* inet_addr takes iptr, a char *
                                         * which contains the literal
                                         * dotted decimal IP number string
                                         * sin.sin_addr.s_addr is a 32 bit
                                         * long int, which is what inet_addr
                                         * returns after converting iptr to
                                         * a long int. See:
                                         * /usr/include/sys/netinet/in.h
                                         * for struct sockaddr_in, and
                                         * struct in_addr
                                         */

    /*-------------------
    | Resolve protocol. |
    -------------------*/
    nProtocol = ResolveProtocol ("tcp");

    /*------------------
    | Resolve service. |
    ------------------*/
/*
    transmitAddr.sin_port = ResolveService (servName, "tcp");
*/
    transmitAddr.sin_port = port;

    /*------------------
    | Create a socket. |
    ------------------*/
    transmitSocket = socket (AF_INET, SOCK_STREAM, nProtocol);
    if (transmitSocket == -1)
        return SERR_SOCKCREATE_FAILED;

    /*---------------------------------------
    | Set socket to establish a connection. |
    ---------------------------------------*/
    transmitStatus = connect (transmitSocket, 
							  (struct sockaddr *)&transmitAddr, 
							  sizeof (transmitAddr));

    if (transmitStatus != 0)
        return SERR_LISTEN_FAILED;

    return (SERR_NO_ERROR);
}


/*============================
| Resolve the protocol name. |
============================*/
int
ResolveProtocol (
 char   *protocolName)
{
    int     protocolNumber;
    struct  protoent *pProtoEntry;

    if ((pProtoEntry = getprotobyname (protocolName)) == 0)
        protocolNumber = -1;
    else
        protocolNumber = pProtoEntry->p_proto;

    return (protocolNumber);
}



/*===========================
| Resolve the service name. |
===========================*/
int
ResolveService (
 char   *serviceName,
 char   *protocolName)
{
    u_short portNumber;
    struct  servent *pServiceEntry;

    /*------------------------------------
    | Get service port number from name. |
    ------------------------------------*/
    pServiceEntry = getservbyname (serviceName, protocolName);
    if (pServiceEntry)
        portNumber = pServiceEntry->s_port;
    else if ((portNumber = htons ((u_short)atoi (serviceName))) == 0)
        portNumber = -1;

    return (portNumber);
}

/*=========================
| Transmit outgoing data. |
=========================*/
int
SendString (
 int	sockNum,
 char   *sendStr)
{
    int     sendLen;
    int     nBytesWritten;

    sendLen = strlen (sendStr);
    nBytesWritten = send (sockNum, sendStr, sendLen, 0);

	/*-----
	| Log |
	-----*/
#ifdef SCK_LOGGING
	fprintf (fLog, "%06d:%s: Sent [%s]\n", getpid (), SystemTime (), sendStr);
	fflush (fLog);
#endif

    /*--------------------------------
    | Return count of bytes written. |
    --------------------------------*/
    return (nBytesWritten);
}


/*========================
| Receive incoming data. |
========================*/
int
RecvString (
 int	sockNum)
{
    int     iNumBytes;
	int		msgLength;

    /*---------------------------------------
    | Can't receive from an invalid socket. |
    ---------------------------------------*/
    if (sockNum < 1)
	{
		/*-----
		| Log |
		-----*/
#ifdef SCK_LOGGING
		fprintf (fLog, 
				 "%06d:%s: Invalid socket [%d]\n", 
				 getpid (), 
				 SystemTime (), 
				 sockNum);
		fflush (fLog);
#endif

        return (0);
	}

    /*--------------------------
    | Zero our receive buffer. |
    --------------------------*/
    memset (readBuf, 0, sizeof (readBuf));

    /*----------------------------
    | Initialize some variables. |
    ----------------------------*/

	/*-------------------
	| Read comm header. |
	-------------------*/
    iNumBytes = recv (sockNum, readBuf, HEADER_LEN, 0);
	if (iNumBytes <= 0)
	{
		/*-----
		| Log |
		-----*/
#ifdef SCK_LOGGING
		fprintf (fLog, 
				 "%06d:%s: Failed to read Comm Header : numBytes[%d] errno[%d] socket [%d]\n", 
				 getpid (), 
				 SystemTime (), 
				 iNumBytes, 
				 errno, 
				 sockNum);
		fflush (fLog);
#endif

		return (iNumBytes);
	}
	readBuf [HEADER_LEN] = '\0';

	/*-----
	| Log |
	-----*/
#ifdef SCK_LOGGING
	fprintf (fLog, "%06d:%s: 1.Received[%s]\n", getpid (), SystemTime (), readBuf);
	fflush (fLog);
#endif

	/*------------------------------
	| Message must begin with IST. |
	------------------------------*/
	if (strncmp (readBuf, "IST", 3))
	{
		/*-----
		| Log |
		-----*/
#ifdef SCK_LOGGING
		fprintf (fLog, 
				 "%06d:%s: Invalid Comm Header : socket [%d]\n", 
				 getpid (), 
				 SystemTime (), 
				 sockNum);
		fflush (fLog);
#endif
		return (0);
	}

	/*---------------------------
	| Determine message length. |
	---------------------------*/
	msgLength = atoi (readBuf + 4);

	/*-----
	| Log |
	-----*/
#ifdef SCK_LOGGING
	fprintf (fLog, 
			 "%06d:%s: Message length [%d][%s]\n", 
			 getpid (), 
			 SystemTime (), 
			 msgLength, 
			 readBuf);
	fflush (fLog);
#endif

	/*---------------
	| Read message. |
	---------------*/
    iNumBytes = recv (sockNum, readBuf, msgLength, 0);
	if (iNumBytes <= 0)
	{
		/*-----
		| Log |
		-----*/
#ifdef SCK_LOGGING
		fprintf (fLog, 
				 "%06d:%s: Failed to read message : length[%d] : socket [%d]\n",
				 getpid (), 
				 SystemTime (), 
				 msgLength, 
				 sockNum);
		fflush (fLog);
#endif
		return (iNumBytes);
	}

	readBuf [iNumBytes] = '\0';

    /*-----------------
    | Error occurred. |
    -----------------*/
    if (iNumBytes == -1)
    {
        switch (errno)
        {
        case EINTR    :
        case ENETRESET:
        case ESHUTDOWN:
        case ENETDOWN :
        case ECONNABORTED:
        case ECONNRESET:
        case EINVAL   :
        case ENOTSOCK :
            return (0);

        }
    }

	/*-----
	| Log |
	-----*/
#ifdef SCK_LOGGING
	fprintf (fLog, "%06d:%s: 2.Received[%s]\n", getpid (), SystemTime (), readBuf);
	fflush (fLog);
#endif

    return iNumBytes;
}

