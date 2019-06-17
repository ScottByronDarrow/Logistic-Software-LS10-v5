#
# Makefile for qt_prt_save
#
SHELL	=	/bin/sh

OBJS	=	prt_save.o

TARGET	=	$(DESTDIR)/qt_prt_save

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
