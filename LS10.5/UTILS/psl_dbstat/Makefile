#
# Makefile for psl_dbstat
#
SHELL	=	/bin/sh

OBJS	=	psl_dbstat.o

TARGET	=	$(DESTDIR)/psl_dbstat

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
