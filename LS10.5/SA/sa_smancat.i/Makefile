#
# Makefile for sa_smancat.i
#
SHELL	=	/bin/sh

OBJS	=	smancat.i.o

TARGET = $(DESTDIR)/sa_smancat.t
NTARGET = $(DESTDIR)/sa_smancat.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)
