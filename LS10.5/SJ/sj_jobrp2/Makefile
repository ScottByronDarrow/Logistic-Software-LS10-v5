#
# makefile for sj_jobrp2.
#
#	Generated - on : 16/11/89
#	          - at : 07:04:14 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_jobrp2.o

TARGET = $(DESTDIR)/sj_jobrp2

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
