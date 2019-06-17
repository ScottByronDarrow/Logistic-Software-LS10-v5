#
# makefile for sj_purge.
#
#	Generated - on : 16/11/89
#	          - at : 07:08:25 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_purge.o

TARGET = $(DESTDIR)/sj_purge

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
