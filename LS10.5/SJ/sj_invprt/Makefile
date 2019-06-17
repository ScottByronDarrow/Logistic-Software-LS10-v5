#
# makefile for sj_invprt.
#
#	Generated - on : 16/11/89
#	          - at : 07:01:57 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_invprt.o

TARGET = $(DESTDIR)/sj_invprt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
