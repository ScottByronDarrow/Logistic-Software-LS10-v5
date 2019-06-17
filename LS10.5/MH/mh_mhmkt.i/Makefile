#
# makefile for mh_mhmkt.i.
#
#	Generated - on : 23/08/89
#	          - at : 11:10:36 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = mh_mhmkt.i.o

TARGET = $(DESTDIR)/mh_mhmkt.t
NTARGET = $(DESTDIR)/mh_mhmkt.i
DTARGET = $(DESTDIR)/mh_dspmkt
PTARGET = $(DESTDIR)/mh_prtmkt

$(PTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

	-mv $(TARGET) $(NTARGET)
	-ln -f $(NTARGET) $(DTARGET)
	-ln -f $(NTARGET) $(PTARGET)

#$(OBJS) : $(INC)/*.[hc]
