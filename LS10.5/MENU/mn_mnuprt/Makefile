#
# makefile for mn_mnuprt.
#

SHELL   = /bin/sh

OBJS = mn_mnuprt.o

TARGET = $(DESTDIR)/mn_mnuprt
TARGET2 = $(DESTDIR)/mn_usrprt

all : $(TARGET) $(TARGET2)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(TARGET2) :	${TARGET}
	-ln -f $(TARGET) $(TARGET2)
