#
# makefile for po_select.i.
#
#	Generated - on : 23/08/89
#	          - at : 11:41:41 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = select.i.o

TARGET = $(DESTDIR)/po_select.t
NTARGET = $(DESTDIR)/po_select.i

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
