PROGS     = coin_location 

INSTDIR   = $(prefix)/usr/bin
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

OBJS = coinlocation.o main.o
LDLIBS += 

#CC = ~/WRTnodeSDK_14_2_28/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.6-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc

all: $(PROGS)
$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(STRIP) $@

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

clean:
	rm -f $(PROGS) *.o *.*~  core

