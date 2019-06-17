#
# makefile for sk_ir_prn.i.
#
#	Generated - on : 23/08/89
#	          - at : 11:31:03 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = ir_prn.i.o

TARGET = $(DESTDIR)/sk_ir_prn.t
OTARGET = $(DESTDIR)/sk_ir_prn.i
CTARGET = $(DESTDIR)/sk_ir_mprn.i

$(OTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(OTARGET)
	-ln -f $(OTARGET) $(CTARGET)

#$(OBJS) : $(INC)/*.[hc]
