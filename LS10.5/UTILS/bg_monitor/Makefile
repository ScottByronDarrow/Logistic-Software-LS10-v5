#
# makefile for bg_monitor.
#
#	Generated - on : 23/08/89
#	          - at : 11:09:58 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = bg_monitor.o

TARGET = $(DESTDIR)/bg_monitor

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
