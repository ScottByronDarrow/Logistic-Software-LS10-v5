#
# Makefile for qt_enqy_prt
#
SHELL	=	/bin/sh

OBJS	=	enqy_prt.o

TARGET	=	$(DESTDIR)/qt_enqy_prt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
