#
# makefile for psl_errdsp & psl_envdsp.
#

SHELL   = /bin/sh

OBJS = psl_errdsp.o

TARGET = $(DESTDIR)/psl_errdsp
MTARGET = $(DESTDIR)/psl_envdsp

all : $(TARGET) $(MTARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(MTARGET) : ${TARGET}
	-ln -f $(TARGET) $(MTARGET)
