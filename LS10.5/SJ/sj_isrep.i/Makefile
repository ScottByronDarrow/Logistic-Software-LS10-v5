#
# makefile for sj_isrep.i.
#
#	Generated - on : 16/11/89
#	          - at : 07:02:52 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_isrep.i.o

TARGET = $(DESTDIR)/sj_isrep.t
NTARGET = $(DESTDIR)/sj_isrep.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
