#
# makefile for sj_pbcr_prt.
#
#	Generated - on : 16/11/89
#	          - at : 07:07:19 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = pbcr_prt.o

TARGET = $(DESTDIR)/sj_pbcr_prt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
