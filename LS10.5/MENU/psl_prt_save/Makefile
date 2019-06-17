#
# makefile for psl_prt_save.
#
#	Generated - on : 31/10/91
#	          - at : 04:49:42 PM
#	          - by : cam
#

SHELL   = /bin/sh

OBJS = _prt_save.o

TARGET = $(DESTDIR)/psl_prt_save
MTARGET = $(DESTDIR)/psl_prt_file

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-ln $(TARGET) $(MTARGET)

#$(OBJS) : $(INC)/*.[hc]
