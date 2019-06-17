#
# makefile for ps_ring_sec.
#
#	Generated - on : 27/03/91
#	          - at : 06:55:28 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = ring_sec.o

TARGET = $(DESTDIR)/ps_ring_sec

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
