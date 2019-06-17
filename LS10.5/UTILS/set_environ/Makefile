#
# makefile for set_environ.
#
#	Generated - on : 23/08/89
#	          - at : 10:47:58 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = _environ.o

TARGET = $(DESTDIR)/set_environ

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
