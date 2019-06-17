#
# makefile for sj_purge.i.
#
#	Generated - on : 16/11/89
#	          - at : 07:08:57 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_purge.i.o

TARGET = $(DESTDIR)/sj_purge.t
NTARGET = $(DESTDIR)/sj_purge.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
