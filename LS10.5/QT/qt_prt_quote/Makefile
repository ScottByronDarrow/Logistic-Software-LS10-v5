#
# Makefile for qt_prt_quote
#
SHELL	=	/bin/sh

OBJS	=	prt_quote.o

TARGET	=	$(DESTDIR)/qt_prt_quote

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
