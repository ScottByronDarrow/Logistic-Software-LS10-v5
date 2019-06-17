#
# makefile for rebuild.
#
#	Generated - on : 08/08/90
#	          - at : 12:07:11 PM
#	          - by : trev
#

SHELL   = /bin/sh

OBJS = rebuild.o

TARGET = $(DESTDIR)/rebuild

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
