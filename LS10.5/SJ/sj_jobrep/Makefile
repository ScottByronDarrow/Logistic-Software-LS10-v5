#
# makefile for sj_jobrep.
#
#	Generated - on : 16/11/89
#	          - at : 07:03:58 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_jobrep.o

TARGET = $(DESTDIR)/sj_jobrep

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
