#
# makefile for recalc.
#
#	Generated - on : 22/12/93
#	          - at : 11:35:37 AM
#	          - by : aroha
#

SHELL   = /bin/sh

OBJS = recalc.o

TARGET = $(DESTDIR)/recalc

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
