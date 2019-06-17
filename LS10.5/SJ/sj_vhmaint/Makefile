#
# makefile for sj_vhmaint.
#
#	Generated - on : 16/11/89
#	          - at : 07:11:40 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_vhmaint.o

TARGET = $(DESTDIR)/sj_vhmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
