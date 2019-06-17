#
# makefile for ps_schmodel.
#
#	Generated - on : 14/04/92
#	          - at : 11:36:09 AM
#	          - by : trev
#

SHELL   = /bin/sh

OBJS = schmodel.o

TARGET = $(DESTDIR)/ps_schmodel

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
