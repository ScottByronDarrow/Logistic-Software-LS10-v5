#
# makefile for sj_stat.i.
#
#	Generated - on : 16/11/89
#	          - at : 07:10:47 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_stat.i.o

TARGET = $(DESTDIR)/sj_stat.t
NTARGET = $(DESTDIR)/sj_stat.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
