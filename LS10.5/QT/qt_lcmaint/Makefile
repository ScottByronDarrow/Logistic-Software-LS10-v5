#
# Makefile for qt_lcmaint
#
SHELL	=	/bin/sh

OBJS	=	qt_lcmaint.o

TARGET	=	$(DESTDIR)/qt_lcmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
