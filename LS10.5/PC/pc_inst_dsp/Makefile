#
# makefile for pc_inst_dsp.
#
#	Generated - on : 13/11/91
#	          - at : 10:55:07 AM
#	          - by : cam
#

SHELL   = /bin/sh

OBJS = inst_dsp.o

TARGET = $(DESTDIR)/pc_inst_dsp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
