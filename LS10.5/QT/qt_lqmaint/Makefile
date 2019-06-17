#
# Makefile for qt_lqmaint
#
SHELL	=	/bin/sh

OBJS	=	qt_lqmaint.o

TARGET	=	$(DESTDIR)/qt_lqmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
