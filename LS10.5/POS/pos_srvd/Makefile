#
# Makefile for pos_srvd
#
#	$Log: Makefile,v $
#	Revision 5.0  2001/06/19 08:12:26  robert
#	LS10-5.0 New Release as of 19 JUNE 2001
#	
#	Revision 4.0  2001/03/09 02:33:38  scott
#	LS10-4.0 New Release as at 10th March 2001
#	
#	Revision 3.0  2000/10/10 12:18:15  gerry
#	Revision No. 3 Start
#	<after Rel-10102000>
#	
#	Revision 2.0  2000/07/15 09:05:48  gerry
#	Forced Revision No Start 2.0 Rel-15072000
#	
#	Revision 1.2  1999/07/15 23:52:51  jonc
#	Removed extraneous library invocation (moved to PinnEnv.tbl)
#	

SHELL	=	/bin/sh

OBJS	=	pos_srvd.o

TARGET	=	$(DESTDIR)/pos_srvd

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
