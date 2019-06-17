#
# makefile for psl_phone.
#

SHELL   = /bin/sh

OBJS = psl_phone.o

TARGET = $(DESTDIR)/psl_phone
NTARGET = $(DESTDIR)/psl_pdisp

all : $(TARGET) $(NTARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(NTARGET) :	${TARGET}
	-ln -f $(TARGET) $(NTARGET)
