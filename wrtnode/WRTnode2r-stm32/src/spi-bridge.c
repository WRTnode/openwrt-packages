#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef DEBUG
#define DEBUG_PRINT        printf
#else
#define DEBUG_PRINT(...)
#endif

typedef struct spi_write_data {
	unsigned long address;
	unsigned long value;
	unsigned long size;
} SPI_WRITE;

#define RT2880_SPI_READ_STR     "read"  /* SPI read operation */
#define RT2880_SPI_WRITE_STR    "write" /* SPI read operation */
#define RT2880_SPI_STATUS_STR   "status"
#define RT2880_SPI_START_STR    "start"
#define RT2880_SPI_FORCE_STR    "-f"

#define RT2880_SPI_READ        (2)
#define RT2880_SPI_STATUS      (2)
#define RT2880_SPI_START       (2)
#define RT2880_SPI_WRITE       (3)

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


#define SPI_MCU_READ_DELAY_US   (20)
#define SPI_MCU_WRITE_DELAY_US  (20)
#define SPI_MCU_CHECK_STATUS_DELAY_US   (1000)


static inline unsigned char read_status(int fd)
{
	unsigned char ch = 0;
	ioctl(fd, SPI_MCU_READ_STATUS, &ch);
	//usleep(SPI_MCU_READ_DELAY_US);

	return ch;
}

static inline unsigned char read_len(int fd)
{
	unsigned char ch = 0;
	ioctl(fd, SPI_MCU_READ_LEN, &ch);
	//usleep(SPI_MCU_READ_DELAY_US);

	return ch;
}

static inline unsigned char read_ch(int fd)
{
	unsigned char ch = 0;
	ioctl(fd, SPI_MCU_READ, &ch);
	//usleep(SPI_MCU_READ_DELAY_US);

	return ch;
}

static inline void put_ch(int fd, unsigned char ch)
{
	ioctl(fd, SPI_MCU_WRITE, &ch);
	//usleep(SPI_MCU_WRITE_DELAY_US);
}

static inline void put_len(int fd, unsigned char len)
{
	ioctl(fd, SPI_MCU_WRITE_LEN, &len);
	//usleep(SPI_MCU_WRITE_DELAY_US);
}

size_t WRTnode2r_spi_read(char * buf , int is_force)
{
	int fd;

	/* We use the last specified parameters, unless new ones are entered */

	unsigned char status = SPI_STATUS_OK;
	unsigned char i = 0;
	unsigned int len = 0;

	fd = open("/dev/spiS0", O_RDONLY);
	if (fd <= 0) {
		fprintf(stderr, "Please insmod module spi_drv.o!\n");
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
			//usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
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

	fd = open("/dev/spiS0", O_RDWR);
	if (fd <= 0) {
		fprintf(stderr, "Please insmod module spi_drv.o!\n");
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
			//usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
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

