#
# makefile for sj_wiprep and sj_statrep
#

SHELL   = /bin/sh

OBJS = sj_wiprep.o

TARGET = $(DESTDIR)/sj_wiprep
DTARGET = $(DESTDIR)/sj_statrep

all : $(TARGET) $(DTARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(DTARGET) :	${TARGET}
	-ln -f $(TARGET) $(DTARGET)
