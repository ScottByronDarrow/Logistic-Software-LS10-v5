#
# Makefile for tr_trch_dsp
#
SHELL	=	/bin/sh

OBJS	=	trch_dsp.o

TARGET	=	$(DESTDIR)/tr_trch_dsp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
