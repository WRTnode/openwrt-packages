all:client_console 
CC = /home/noel/mroot/openwrt/trunk/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc
client_console:client_console.o 
	$(CC) -o client_console client_console.o -lpthread
clean:
	rm -f *.o client_console
