#
# Makefile for pm_vi_notes
#
SHELL	=	/bin/sh

OBJS	=	vi_notes.o

TARGET	=	$(DESTDIR)/pm_vi_notes

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
