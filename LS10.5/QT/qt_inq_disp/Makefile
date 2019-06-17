#
# Makefile for qt_inq_disp
#
SHELL	=	/bin/sh

OBJS	=	inq_disp.o

TARGET	=	$(DESTDIR)/qt_inq_disp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
