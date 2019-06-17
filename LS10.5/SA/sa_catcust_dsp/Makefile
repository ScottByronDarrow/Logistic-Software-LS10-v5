#
# Makefile for sa_catcust_dsp
#
SHELL	=	/bin/sh

OBJS	=	catcust_dsp.o

TARGET	=	$(DESTDIR)/sa_catcust_dsp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
