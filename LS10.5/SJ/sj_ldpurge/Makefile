#
# makefile for sj_ldpurge.
#
#	Generated - on : 16/11/89
#	          - at : 07:05:14 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_ldpurge.o

TARGET = $(DESTDIR)/sj_ldpurge

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
