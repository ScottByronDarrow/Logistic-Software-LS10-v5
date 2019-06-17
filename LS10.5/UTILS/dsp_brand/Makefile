#
# makefile for dsp_brand.
#

SHELL   = /bin/sh

OBJS = dsp_brand.o

TARGET = $(DESTDIR)/dsp_brand
NTARGET = $(DESTDIR)/chk_brand

all :	${TARGET} ${NTARGET}

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(NTARGET) : ${TARGET}
	-ln -f $(TARGET) $(NTARGET)
