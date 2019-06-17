#
# Makefile for as_fix_prn
#
SHELL	=	/bin/sh

OBJS	=	as_fix_prn.o

TARGET	=	$(DESTDIR)/as_fix_prn

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
