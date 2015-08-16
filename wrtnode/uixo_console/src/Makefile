PROGS     = uixo_console

OBJS = main.o uixo_console.o serial.o serial_posix.o FunTypes.o HandlePort.o HandleMsg.o spi.o spi_mt7688.o
LDFLAGS += -lpthread
all: $(PROGS)
$(PROGS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $(PROGS) 

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -f $(PROGS) *.o *.*~  core
