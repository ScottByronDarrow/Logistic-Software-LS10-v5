#
# makefile for strback.
#
#	Generated - on : 23/08/89
#	          - at : 10:40:41 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = strback.o

TARGET = $(DESTDIR)/strback

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
