#
# makefile for sj_dp_lp.i.
#
#	Generated - on : 16/11/89
#	          - at : 06:56:53 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_dp_lp.i.o

TARGET = $(DESTDIR)/sj_dp_lp.t
NTARGET = $(DESTDIR)/sj_dp_lp.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
