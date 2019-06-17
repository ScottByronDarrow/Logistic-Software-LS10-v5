#
# makefile for cm_reqrprt.i.
#
#	Generated - on : 15/04/93
#	          - at : 12:03:25 PM
#	          - by : cam
#

SHELL   = /bin/sh

OBJS = reqrprt.i.o

TARGET = $(DESTDIR)/cm_reqrprt.t
ATARGET = $(DESTDIR)/cm_reqrprt.i

$(ATARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(ATARGET)

#$(OBJS) : $(INC)/*.[hc]
