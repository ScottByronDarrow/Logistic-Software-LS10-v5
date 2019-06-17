#
# makefile for lp_secmaint.
#
#	Generated - on : 14/11/90
#	          - at : 03:30:20 PM
#	          - by : trev
#

SHELL   = /bin/sh

OBJS = secmaint.o

TARGET = $(DESTDIR)/lp_secmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
