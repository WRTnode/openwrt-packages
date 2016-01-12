#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>

#ifdef DEBUG
#define DEBUG_PRINT        printf
#else
#define DEBUG_PRINT(...)
#endif

#define SPI_DEVICE    "/dev/spidev1.0"
#define SPI_MODE      SPI_MODE_0
#define SPI_WORD_LEN  8
#define SPI_HZ        1000000

typedef struct spi_write_data {
	unsigned long address;
	unsigned long value;
	unsigned long size;
} SPI_WRITE;

#define SPI_MCU_READ           (0x01)
#define SPI_MCU_READ_LEN       (0x04)
#define SPI_MCU_WRITE          (0x10)
#define SPI_MCU_WRITE_LEN      (0x40)
#define SPI_MCU_READ_STATUS    (0xff)
#define SPI_STATUS_7688_READ_FROM_STM32_E    (1<<0)
#define SPI_STATUS_7688_READ_FROM_STM32_NE   (0<<0)
#define SPI_STATUS_7688_WRITE_TO_STM32_F     (1<<1)
#define SPI_STATUS_7688_WRITE_TO_STM32_NF    (0<<1)
#define SPI_STATUS_OK                        (0x80)
#define INIT_MSG { \
		.speed_hz = SPI_HZ, \
		.delay_usecs = 0, \
		.bits_per_word = SPI_WORD_LEN, \
		.tx_buf = 0, \
		.rx_buf = 0, \
		.len = 0, \
		.cs_change = 0, \
	}

static inline unsigned char read_status(int fd)
{
	struct spi_ioc_transfer msg = INIT_MSG;
	unsigned char buf[2];

	buf[0] = SPI_MCU_READ_STATUS;

	msg.tx_buf = (__u32)buf;
	msg.rx_buf = (__u32)(&buf[1]);
	msg.len = 2;
	msg.cs_change = 1;

	ioctl(fd, SPI_IOC_MESSAGE(1), &msg);

	return buf[1];
}

static inline unsigned char read_len(int fd)
{
	struct spi_ioc_transfer msg = INIT_MSG;
	unsigned char buf[2];

	buf[0] = SPI_MCU_READ_LEN;

	msg.tx_buf = (__u32)buf;
	msg.rx_buf = (__u32)(&buf[1]);
	msg.len = 2;
	msg.cs_change = 1;

	ioctl(fd, SPI_IOC_MESSAGE(1), &msg);

	return buf[1];
}

static inline unsigned char read_ch(int fd)
{
	struct spi_ioc_transfer msg = INIT_MSG;
	unsigned char buf[2];

	buf[0] = SPI_MCU_READ;

	msg.tx_buf = (__u32)buf;
	msg.rx_buf = (__u32)(&buf[1]);
	msg.len = 2;
	msg.cs_change = 1;

	ioctl(fd, SPI_IOC_MESSAGE(1), &msg);

	return buf[1];
}

static inline void put_ch(int fd, unsigned char ch)
{
	struct spi_ioc_transfer msg = INIT_MSG;
	unsigned char buf[2];

	buf[0] = SPI_MCU_WRITE;
	buf[1] = ch;

	msg.tx_buf = (__u32)buf;
	msg.len = 2;
	msg.cs_change = 1;

	ioctl(fd, SPI_IOC_MESSAGE(1), &msg);
}

static inline void put_len(int fd, unsigned char len)
{
	struct spi_ioc_transfer msg = INIT_MSG;
	unsigned char buf[2];

	buf[0] = SPI_MCU_WRITE_LEN;
	buf[1] = len;

	msg.tx_buf = (__u32)buf;
	msg.len = 2;
	msg.cs_change = 1;

	ioctl(fd, SPI_IOC_MESSAGE(1), &msg);
}

static int open_spi_device(const char* dev)
{
	int fd;
	int mode = SPI_MODE;
	int word_len = SPI_WORD_LEN;
	int hz = SPI_HZ;

	fd = open(dev, O_RDWR);
	if (fd <= 0) {
		fprintf(stderr, "Can not open spi device(%s).\n", dev);
		return -1;
	}
	if(ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
		fprintf(stderr, "Can not set spidev mode to %d.\n", mode);
		goto err_out;
	}
	if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &word_len) < 0) {
		fprintf(stderr, "Can not set spidev word len to %d.\n", word_len);
		goto err_out;
	}
	if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &hz) < 0) {
		fprintf(stderr, "Can not set spidev speed to %d.\n", hz);
		goto err_out;
	}
	return fd;

err_out:
	close(fd);
	return -1;
}

size_t WRTnode2r_spi_read(char * buf , int is_force)
{
	int fd;
	/* We use the last specified parameters, unless new ones are entered */

	unsigned char status = SPI_STATUS_OK;
	unsigned char i = 0;
	unsigned int len = 0;

	fd = open_spi_device(SPI_DEVICE);
	if (fd <= 0) {
		fprintf(stderr, "Can not open spi device.\n");
		return -1;
	}

	if (is_force) {
		status = read_status(fd);
		DEBUG_PRINT("read status = 0x%x\n", status);
		if(!(status & SPI_STATUS_OK)) {
			fprintf(stderr, "stm32 spi read error.\n");
			goto err;
		}
		if(status & SPI_STATUS_7688_READ_FROM_STM32_E) {
			fprintf(stderr, "stm32 read buf empty.\n");
			goto err;
		}
	}
	else {
		do {
			status = read_status(fd);
			DEBUG_PRINT("read status = 0x%x\n", status);
			if(status & (SPI_STATUS_OK) &&
					(!(status & SPI_STATUS_7688_READ_FROM_STM32_E))) {
				break;
			}
		}while(1);
	}
	len = read_len(fd);
	DEBUG_PRINT("read len = %d\n", len);

	if(0 == len) {
		fprintf(stderr, "read length is 0.\n");
		goto err;
	}

	for(i=0; i<len; i++) {
		buf[i] = read_ch(fd);
	}
	close(fd);
	return len;
err:
	close(fd);
	return -1;
}

size_t WRTnode2r_spi_write(char* data, int len, int is_force)
{
	int chk_match, size, fd;
	unsigned char buf;

	unsigned char status = 0;
	unsigned char i = 0;

	fd = open_spi_device(SPI_DEVICE);
	if (fd <= 0) {
		fprintf(stderr, "Can not open spi device.\n");
		return -1;
	}

	if (is_force) {
		status = read_status(fd);
		DEBUG_PRINT("write status = 0x%x\n", status);
		if(status & SPI_STATUS_OK) {
			if(status & SPI_STATUS_7688_WRITE_TO_STM32_F) {
				fprintf(stderr, "stm32 write buf full.\n");
				goto err;
			}
		}
		else {
			fprintf(stderr, "stm32 spi read error.\n");
		}
	}
	else {
		do{
			status = read_status(fd);
			DEBUG_PRINT("write status = 0x%x\n", status);
			if((status & SPI_STATUS_OK) &&
					(!(status & SPI_STATUS_7688_WRITE_TO_STM32_F))) {
				break;
			}
		} while(1);
	}

	if(0 == len) {
		fprintf(stderr, "write length is 0.\n");
		goto err;
	}
	put_len(fd, len);
	for(i=0; i<len; i++) {
		put_ch(fd, data[i]);
		DEBUG_PRINT("write data[%d] = 0x%x %c\n", i, data[i] , data[i]);
	}
	close(fd);
	return 0;

err:
	close(fd);
	return -1;
}

