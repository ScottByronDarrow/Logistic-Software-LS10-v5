#
# makefile for mh_purge.i.
#
#	Generated - on : 23/08/89
#	          - at : 11:19:01 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = mh_purge.i.o

TARGET = $(DESTDIR)/mh_purge.t
NTARGET = $(DESTDIR)/mh_purge.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
