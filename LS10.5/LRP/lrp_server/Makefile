#
# created by Primo O. Esteria
# June 25, 1999
#
#	$Log: Makefile,v $
#	Revision 5.0  2001/06/19 08:07:40  robert
#	LS10-5.0 New Release as of 19 JUNE 2001
#	
#	Revision 4.0  2001/03/09 02:28:50  scott
#	LS10-4.0 New Release as at 10th March 2001
#	
#	Revision 3.0  2000/10/10 12:15:40  gerry
#	Revision No. 3 Start
#	<after Rel-10102000>
#	
#	Revision 2.0  2000/07/15 08:58:49  gerry
#	Forced Revision No. Start 2.0 Rel-15072000
#	
#	Revision 1.4  1999/07/15 23:50:11  jonc
#	Removed extraneous library invocations (moved to PinnEnv.tbl)
#	
#	Revision 1.3  1999/07/15 23:25:15  jonc
#	Remove extraneous ${OBJ}
#	
#
SHELL = /bin/sh

OBJS = lrp_server.o

TARGET = $(DESTDIR)/lrp_server

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
	$(OBJS)	\
	-o $(TARGET) \
	$(LIB)

