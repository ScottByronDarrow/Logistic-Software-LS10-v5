#
# makefile for psl_error.
#

SHELL   = /bin/sh

OBJS = psl_error.o

TARGET = $(DESTDIR)/psl_error
NTARGET = $(DESTDIR)/no_option

all : $(TARGET) $(NTARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(NTARGET) :	${TARGET}
	-ln -f $(TARGET) $(NTARGET)
