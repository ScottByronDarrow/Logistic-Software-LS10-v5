#
# Makefile for qc_maint
#
SHELL	=	/bin/sh

OBJS	=	qc_maint.o

TARGET	=	$(DESTDIR)/qc_maint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
