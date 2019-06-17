#
# makefile for prog_help.
#

SHELL   = /bin/sh

OBJS = prog_help.o

TARGET = $(DESTDIR)/prog_help
ATARGET = $(DESTDIR)/sprog_help

all : $(TARGET) $(ATARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(ATARGET) :	${TARGET}
	-ln -f $(TARGET) $(ATARGET)
