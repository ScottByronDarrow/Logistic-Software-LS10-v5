#
# makefile for po_mthdet.i.
#
#	Generated - on : 23/08/89
#	          - at : 11:35:23 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = mthdet.i.o

TARGET = $(DESTDIR)/po_mthdet.t
NTARGET = $(DESTDIR)/po_mthdet.i

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
