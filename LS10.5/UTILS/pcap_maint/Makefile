#
# makefile for pcap_maint.
#
#	Generated - on : 18/01/91
#	          - at : 03:52:49 PM
#	          - by : camp
#

SHELL   = /bin/sh

OBJS = pcap_maint.o

TARGET = $(DESTDIR)/pcap_maint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
