#
# Makefile for as_stat_inp
#
SHELL	=	/bin/sh

OBJS	=	stat_inp.o

TARGET	=	$(DESTDIR)/as_stat_inp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
