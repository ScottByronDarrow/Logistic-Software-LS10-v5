#
# makefile for cm_invprnt.i.
#
#	Generated - on : 16/04/93
#	          - at : 11:32:27 AM
#	          - by : simon
#

SHELL   = /bin/sh

OBJS = invprnt.i.o

TARGET = $(DESTDIR)/cm_invprnt.t
ATARGET = $(DESTDIR)/cm_invprnt.i

$(ATARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(ATARGET)

#$(OBJS) : $(INC)/*.[hc]
