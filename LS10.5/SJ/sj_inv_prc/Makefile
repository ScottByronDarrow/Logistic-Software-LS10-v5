#
# makefile for sj_inv_prc.
#
#	Generated - on : 16/11/89
#	          - at : 07:01:17 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_inv_prc.o

TARGET = $(DESTDIR)/sj_inv_prc

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
