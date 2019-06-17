#
# Makefile for qt_statmaint
#
SHELL	=	/bin/sh

OBJS	=	statmaint.o

TARGET	=	$(DESTDIR)/qt_statmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
