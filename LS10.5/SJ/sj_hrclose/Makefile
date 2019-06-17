#
# makefile for sj_hrclose.
#
#	Generated - on : 16/11/89
#	          - at : 06:59:18 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_hrclose.o

TARGET = $(DESTDIR)/sj_hrclose

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
