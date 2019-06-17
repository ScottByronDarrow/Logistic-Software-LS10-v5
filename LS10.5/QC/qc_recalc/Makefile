#
# Makefile for qc_recalc
#
SHELL	=	/bin/sh

OBJS	=	qc_recalc.o

TARGET	=	$(DESTDIR)/qc_recalc

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
