#
# makefile for sj_ma_rep.
#
#	Generated - on : 16/11/89
#	          - at : 07:06:44 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_ma_rep.o

TARGET = $(DESTDIR)/sj_ma_rep

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
