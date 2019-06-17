#
# makefile for sj_lists.
#
#	Generated - on : 16/11/89
#	          - at : 07:05:42 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_lists.o

TARGET = $(DESTDIR)/sj_lists

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
