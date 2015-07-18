PROGS     = spi-bridge

INSTDIR   = $(prefix)/usr/bin
INSTMODE  = 0755
INSTOWNER = root
INSTFROUP = root

OBJS = main.o

all: $(PROGS)
$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@
%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

clean:
	rm -f $(PROGS) *.o

