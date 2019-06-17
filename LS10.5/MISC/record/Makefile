#
# makefile for record.
#

SHELL   = /bin/sh

OBJS = record.o

TARGET = $(DESTDIR)/record
NTARGET = $(DESTDIR)/play

all : $(TARGET) $(NTARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(NTARGET) :	${TARGET}
	-ln -f $(TARGET) $(NTARGET)
