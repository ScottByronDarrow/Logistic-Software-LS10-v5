#
# makefile for dp_maint.
#
#	Generated - on : 23/08/89
#	          - at : 10:44:41 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = dp_maint.o

TARGET = $(DESTDIR)/dp_maint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
