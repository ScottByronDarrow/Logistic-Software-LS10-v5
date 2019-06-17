#
# makefile for sk_cdrg_inp.
#
#	Generated - on : 20/05/93
#	          - at : 01:56:38 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = cdrg_inp.o

TARGET = $(DESTDIR)/sk_cdrg_inp
MTARGET = $(DESTDIR)/sk_cdrg_prn

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-ln -f $(TARGET) $(MTARGET)

#$(OBJS) : $(INC)/*.[hc]
