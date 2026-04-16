DEBUG   :=
#-gstabs
CFLAGS  := -std=c99 -Wall -O3 -D__USE_INLINE__ $(DEBUG)
LDFLAGS := -Tdata=0 -nostartfiles
LIBS    :=
OBJS    := extheader_os4.o xad_zip.o

.PHONY: all
all: Zip

$(OBJS): $(OBJS:.o=.c)

Zip: $(OBJS)
	gcc $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
#	strip $@
#	strip -R.comment $@
#	strip -R.comment `nm -u $@ | sed "s/U /-K/"` $@
