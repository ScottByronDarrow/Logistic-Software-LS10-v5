#
# Makefile for fe_contmnt
#
SHELL	=	/bin/sh

OBJS	=	fe_contmnt.o

TARGET	=	$(DESTDIR)/fe_contmnt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
