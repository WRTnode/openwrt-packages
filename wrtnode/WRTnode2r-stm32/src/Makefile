exe = WRTnode2r-stm32

$(exe):main.o spi-bridge.o
	$(CC) -o $(exe) main.o spi-bridge.o 
spi-bridge.o: spi-bridge.c
	$(CC) $(CFLAGS) -c main.c spi-bridge.c
clean:
	rm *.o  $(exe) -f
