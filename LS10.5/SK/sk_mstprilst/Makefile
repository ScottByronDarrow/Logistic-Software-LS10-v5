#
# makefile for sk_mstprilst.
#

SHELL   = /bin/sh

OBJS = mstprilst.o

TARGET = $(DESTDIR)/sk_mstprilst
DTARGET = $(DESTDIR)/sk_ctrprilst

all : $(TARGET) $(DTARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(DTARGET) :	${TARGET}
	-ln -f $(TARGET) $(DTARGET)
