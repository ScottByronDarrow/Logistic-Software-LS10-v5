#
# makefile for sj_cstrep.
#
#	Generated - on : 16/11/89
#	          - at : 06:54:37 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_cstrep.o

TARGET = $(DESTDIR)/sj_cstrep

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
