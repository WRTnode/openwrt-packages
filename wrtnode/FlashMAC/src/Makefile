PROGS     = libubox_s.so.7.0 

INSTDIR   = $(prefix)/lib
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

OBJS = flashmac.o
LDLIBS += -lm

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

