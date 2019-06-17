#
# Makefile for as_dep_prn
#
SHELL	=	/bin/sh

OBJS	=	as_dep_prn.o

TARGET	=	$(DESTDIR)/as_dep_prn

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
