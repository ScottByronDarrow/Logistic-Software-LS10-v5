#
# Makefile for pos_monitor
#
SHELL	=	/bin/sh

OBJS	=	_monitor.o

TARGET	=	$(DESTDIR)/pos_monitor

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
