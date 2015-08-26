PROGS     = uixo_console

OBJS = main.o serial.o serial_posix.o HandlePort.o HandleMsg.o spi.o spi_mt7688.o thpool.o
LDFLAGS += -lpthread
all: $(PROGS)
$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $(PROGS) 

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -f $(PROGS) *.o *.*~  core
