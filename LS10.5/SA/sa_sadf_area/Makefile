#
# Makefile for sa_sadf_area
#
SHELL	=	/bin/sh

OBJS	=	sadf_area.o

TARGET	=	$(DESTDIR)/sa_sadf_area

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
