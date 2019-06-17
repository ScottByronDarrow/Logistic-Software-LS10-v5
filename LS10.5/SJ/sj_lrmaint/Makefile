#
# makefile for sj_lrmaint.
#
#	Generated - on : 16/11/89
#	          - at : 07:06:11 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_lrmaint.o

TARGET = $(DESTDIR)/sj_lrmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
