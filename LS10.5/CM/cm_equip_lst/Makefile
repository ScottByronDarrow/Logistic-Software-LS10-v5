#
# makefile for cm_equip_lst.
#
#	Generated - on : 15/04/93
#	          - at : 12:11:10 PM
#	          - by : cam
#

SHELL   = /bin/sh

OBJS = equip_lst.o

TARGET = $(DESTDIR)/cm_equip_lst

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
