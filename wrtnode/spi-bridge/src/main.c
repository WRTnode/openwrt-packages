#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <linux/spi/spidev.h>

#ifdef DEBUG
#define DEBUG_PRINT   printf
#else
#define DEBUG_PRINT(...)
#endif

#define SPI_DEVICE     "/dev/spidev1.0"
#define SPI_MODE       SPI_MODE_0
#define SPI_WORD_LEN   8
#define SPI_HZ         1000000

typedef struct spi_write_data {
	unsigned long address;
	unsigned long value;
	unsigned long size;
} SPI_WRITE;

#define RT2880_SPI_READ_STR    "read"
#define RT2880_SPI_WRITE_STR   "write"
#define RT2880_SPI_STATUS_STR  "status"
#define RT2880_SPI_FORCE_STR   "-f"

#define RT2880_SPI_READ     (2)
#define RT2880_SPI_STATUS   (2)
#define RT2880_SPI_WRITE    (3)

#define SPI_FRAME_MAX_LEN   (255)

#define SPI_MCU_READ        (0x01)
#define SPI_MCU_READ_LEN    (0x04)
#define SPI_MCU_WRITE       (0x10)
#define SPI_MCU_WRITE_LEN   (0x40)
#define SPI_MCU_READ_STATUS (0xff)
#define SPI_STATUS_7688_READ_FROM_STM32_E     (1<<0)
#define SPI_STATUS_7688_READ_FROM_STM32_NE    (0<<0)
#define SPI_STATUS_7688_WRITE_TO_STM32_F      (1<<1)
#define SPI_STATUS_7688_WRITE_TO_STM32_NF     (0<<1)
#define SPI_STATUS_OK       (0x80)

char usage[] =	"spicmd [read/write] [-f] WRTnode2r stm32 data(if write)\n"
		"spicmd format:\n"
		"  spicmd read \n"
		"  spicmd write [string]\n"
		"  spicmd status \n"
		"-f Force read/write. Do not block\n"
		"NOTE -- read/write/status value are in string\n"
		"spicmd\n  Open spiconsole.\n";

#define SPI_MCU_READ_DELAY_US          (200)
#define SPI_MCU_WRITE_DELAY_US         (200)
#define SPI_MCU_CHECK_STATUS_DELAY_US  (100000)

#define INIT_MSG { \
		.speed_hz = SPI_HZ, \
		.delay_usecs = 0, \
		.bits_per_word = SPI_WORD_LEN, \
		.tx_buf = 0, \
		.rx_buf = 0, \
		.len = 0, \
		.cs_change = 0, \
	}

static ssize_t stm32_spi_read(int fd, const unsigned char cmd, void* buf, size_t count)
{
	struct spi_ioc_transfer msg[4];
	int i;

	for(i=4; i>0; i--) {
		msg[i].speed_hz = SPI_HZ;
		msg[i].delay_usecs = 0;
		msg[i].bits_per_word = SPI_WORD_LEN;
		msg[i].tx_buf = 0;
		msg[i].rx_buf = 0;
		msg[i].len = 0;
		msg[i].cs_change = 0;
	}

	msg[0].cs_change = 1;

	msg[1].tx_buf = (__u32)&cmd;
	msg[1].len = 1;

	msg[2].rx_buf = (__u32)buf;
	msg[2].len = count;

	msg[3].cs_change = 1;

	if(ioctl(fd, SPI_IOC_MESSAGE(2), msg) < 0) {
		fprintf(stderr, "do spi read error.\n");
		return -1;
	}
	usleep(SPI_MCU_READ_DELAY_US);
	if(ioctl(fd, SPI_IOC_MESSAGE(2), &msg[2]) < 0) {
		fprintf(stderr, "do spi read error.\n");
		return -1;
	}

	return count;
}

static ssize_t stm32_spi_write(int fd, const unsigned char cmd, const void* buf, size_t count)
{
	struct spi_ioc_transfer msg[4];
	int i;

	for(i=4; i>0; i--) {
		msg[i].speed_hz = SPI_HZ;
		msg[i].delay_usecs = 0;
		msg[i].bits_per_word = SPI_WORD_LEN;
		msg[i].tx_buf = 0;
		msg[i].rx_buf = 0;
		msg[i].len = 0;
		msg[i].cs_change = 0;
	}

	msg[0].cs_change = 1;

	msg[1].tx_buf = (__u32)&cmd;
	msg[1].len = 1;

	msg[2].tx_buf = (__u32)buf;
	msg[2].len = count;

	msg[3].cs_change = 1;

	if(ioctl(fd, SPI_IOC_MESSAGE(2), msg) < 0) {
		fprintf(stderr, "do spi write error.\n");
		return -1;
	}
	usleep(SPI_MCU_WRITE_DELAY_US);
	if(ioctl(fd, SPI_IOC_MESSAGE(2), &msg[2]) < 0) {
		fprintf(stderr, "do spi write error.\n");
		return -1;
	}

	return count;
}

static inline unsigned char read_status(int fd)
{
	unsigned char status;
	stm32_spi_read(fd, SPI_MCU_READ_STATUS, &status, 1);
	return status;
}

static inline unsigned char read_len(int fd)
{
	unsigned char len;
	stm32_spi_read(fd, SPI_MCU_READ_LEN, &len, 1);
	return len;
}

static inline char read_ch(int fd)
{
	char ch;
	stm32_spi_read(fd, SPI_MCU_READ, &ch, 1);
	return ch;
}

static inline ssize_t read_str(int fd, void* buf, size_t count)
{
	return stm32_spi_read(fd, SPI_MCU_READ, buf, count);
}

static inline void put_len(int fd, unsigned char len)
{
	stm32_spi_write(fd, SPI_MCU_WRITE_LEN, &len, 1);
}

static inline void put_ch(int fd, char ch)
{
	stm32_spi_write(fd, SPI_MCU_WRITE, &ch, 1);
}

static inline ssize_t write_str(int fd, const void* buf, size_t count)
{
	return stm32_spi_write(fd, SPI_MCU_WRITE, buf, count);
}

static pthread_t read_mcu_tidp;
static pthread_t read_stdin_tidp;
static pthread_mutex_t spi_mutex;

/* *
 * NOTICE: If 7688 read one byte from STM32, It will got right byte in next read cmd.
 * */
static void* read_mcu_handler(void* arg)
{
	int fd = *(int*)arg;
	unsigned char len = 0;
	char* data = NULL;
	int i = 0;

	while(1) {
		do {
			unsigned char status = SPI_STATUS_OK;
			pthread_mutex_lock(&spi_mutex);
			status = read_status(fd);
			DEBUG_PRINT("read status = 0x%x\n", status);
			if(status & (SPI_STATUS_OK) &&
	 (!(status & SPI_STATUS_7688_READ_FROM_STM32_E))) {
				break;
			}
			pthread_mutex_unlock(&spi_mutex);
			usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
		}while(1);

		len = read_len(fd);
		DEBUG_PRINT("read len = %d\n", len);
		if(0 == len) {
			fprintf(stderr, "read length is 0.\n");
			goto OUT;
		}

		if(NULL == (data = (char*)malloc((len+1)*sizeof(char)))) {
			fprintf(stderr, "read data malloc error!\n");
			goto OUT;
		}

		read_str(fd, data, len);
		data[len] = '\0';
		pthread_mutex_unlock(&spi_mutex);
		printf("%s",data);
		free(data);
		data = NULL;
	}

OUT:
	pthread_mutex_unlock(&spi_mutex);
	pthread_exit(NULL);
}

static void* read_stdin_handler(void* arg)
{
	int fd = *(int*)arg;
	unsigned char len = 0;
	unsigned char status = SPI_STATUS_OK;
	char* data = NULL;
	int i = 0;

	data = (char*)malloc(SPI_FRAME_MAX_LEN*sizeof(char));
	if(NULL == data) {
		fprintf(stderr, "failed to malloc spi write buffer.\n");
		pthread_exit(NULL);
	}
	while(1) {
		char* data_in = NULL;
		data_in = fgets(data, SPI_FRAME_MAX_LEN, stdin);
		if(NULL != data_in) {
			if(strncmp(data_in, "exit", 4) == 0) {
				pthread_cancel(read_mcu_tidp);
				pthread_exit(NULL);
			}
			do {
				pthread_mutex_lock(&spi_mutex);
				status = read_status(fd);
				DEBUG_PRINT("write status = 0x%x\n", status);
				if((status & SPI_STATUS_OK) &&
	   (!(status & SPI_STATUS_7688_WRITE_TO_STM32_F))) {
					break;
				}
				pthread_mutex_unlock(&spi_mutex);
				usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
			} while(1);
			len = strlen(data_in);
			if(0 == len) {
				fprintf(stderr, "write length is 0.\n");
				goto OUT;
			}
			put_len(fd, len);
			write_str(fd, data_in, len);
			DEBUG_PRINT("write data = %s\n", data_in);
			pthread_mutex_unlock(&spi_mutex);
		}
		else {
			fprintf(stderr, "fgets error.\n");
		}
		data_in = NULL;
		len = 0;
	}

OUT:
	pthread_mutex_unlock(&spi_mutex);
	pthread_exit(NULL);
}

static void spi_console_exit(int sig)
{
	DEBUG_PRINT("Get SIGINT.\n");
	pthread_cancel(read_mcu_tidp);
	pthread_cancel(read_stdin_tidp);
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

int main(int argc, char* argv[])
{
	int chk_match, size, fd;
	int is_force = 0;
	unsigned char buf;

	if(argc == 1) {
		fd = open_spi_device(SPI_DEVICE);
		if (fd <= 0) {
			fprintf(stderr, "Can not open spi device!\n");
			return -1;
		}
		DEBUG_PRINT("Start SPI console.\n");
		signal(SIGINT, spi_console_exit);
		if(pthread_mutex_init(&spi_mutex, NULL) < 0) {
			fprintf(stderr, "Init thread mutex error.\n");
			return -1;
		}
		if(pthread_create(&read_mcu_tidp, NULL, read_mcu_handler, &fd) < 0) {
			fprintf(stderr, "Create read mcu thread failed.\n");
			return -1;
		}
		if(pthread_create(&read_stdin_tidp, NULL, read_stdin_handler, &fd) < 0) {
			fprintf(stderr, "Create write mcu thread failed.\n");
			return -1;
		}

		/* wait SIGINT signal to exit */
		pthread_join(read_mcu_tidp, NULL);
		pthread_join(read_stdin_tidp, NULL);
		printf("SPI console exit.\n");
		close(fd);
		return 0;
	}

	if ((argc > 2) && (0 == strcmp(argv[2], RT2880_SPI_FORCE_STR))) {
		is_force = 1;
		argc --;
	}

	/* We use the last specified parameters, unless new ones are entered */
	switch (argc) {
	//case RT2880_SPI_STATUS:
	case RT2880_SPI_READ:
		if(0 == strcmp(argv[1], RT2880_SPI_READ_STR)) {
			unsigned char status = SPI_STATUS_OK;
			unsigned char len = 0;
			unsigned char i = 0;
			char* data = NULL;

			fd = open_spi_device(SPI_DEVICE);
			if (fd <= 0) {
				fprintf(stderr, "Can not open spi device!\n");
				return -1;
			}
			if (is_force) {
				status = read_status(fd);
				DEBUG_PRINT("read status = 0x%x\n", status);
				if(!(status & SPI_STATUS_OK)) {
					fprintf(stderr, "stm32 spi read error.\n");
					return -1;
				}
				if(status & SPI_STATUS_7688_READ_FROM_STM32_E) {
					fprintf(stderr, "stm32 read buf empty.\n");
					return -1;
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
					usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
				}while(1);
			}
			len = read_len(fd);
			DEBUG_PRINT("read len = %d\n", len);
			if(0 == len) {
				fprintf(stderr, "read length is 0.\n");
				return -1;
			}
			if(NULL == (data = (char*)malloc((len+1)*sizeof(char)))) {
				fprintf(stderr, "read data malloc error!\n");
				return -1;
			}
			read_str(fd, data, len);
			data[len] = '\0';
			printf("%s\n",data);
			free(data);
			close(fd);
		}
		else if (0 == strcmp(argv[1], RT2880_SPI_STATUS_STR)) {
			unsigned char status = 0;

			fd = open_spi_device(SPI_DEVICE);
			if (fd <= 0) {
				fprintf(stderr, "Can not open spi device!\n");
				return -1;
			}
			status = read_status(fd);
			if(status & SPI_STATUS_OK) {
				if(status & SPI_STATUS_7688_READ_FROM_STM32_E) {
					printf("Can not read. STM32 read buf empty.\n");
				}
				if(status & SPI_STATUS_7688_WRITE_TO_STM32_F) {
					printf("Can not write. STM32 write buf full.\n");
				}
				if(status == SPI_STATUS_OK) {
					printf("OK\n");
				}
			}
			else {
				printf("spi stm32 read error.\n");
			}
			close(fd);
		}
		else {
			fprintf(stderr, "Usage:\n%s\n", usage);
			return -1;
		}
		break;
	case RT2880_SPI_WRITE:
		if(0 == strcmp(argv[1], RT2880_SPI_WRITE_STR)) {
			unsigned char status = 0;
			unsigned char len = 0;
			unsigned char i = 0;
			char* data = (is_force)? argv[3]: argv[2];

			fd = open_spi_device(SPI_DEVICE);
			if (fd <= 0) {
				fprintf(stderr, "Can not open spi device!\n");
				return -1;
			}

			if (is_force) {
				status = read_status(fd);
				DEBUG_PRINT("write status = 0x%x\n", status);
				if(status & SPI_STATUS_OK) {
					if(status & SPI_STATUS_7688_WRITE_TO_STM32_F) {
						fprintf(stderr, "stm32 write buf full.\n");
						return -1;
					}
				}
				else {
					fprintf(stderr, "stm32 spi read error.\n");
				}
			}
			else {
				do {
					status = read_status(fd);
					DEBUG_PRINT("write status = 0x%x\n", status);
					if((status & SPI_STATUS_OK) &&
					   (!(status & SPI_STATUS_7688_WRITE_TO_STM32_F))) {
						break;
					}
					usleep(SPI_MCU_CHECK_STATUS_DELAY_US);
				} while(1);
			}
			len = strlen(data);
			if(0 == len) {
				fprintf(stderr, "write length is 0.\n");
				return -1;
			}
			put_len(fd, len+1); //add \n for msh
			write_str(fd, data, len);
			put_ch(fd, '\n');
			close(fd);
		}
		else {
			fprintf(stderr, "Usage:\n%s\n", usage);
			return -1;
		}
		break;
	default:
		fprintf(stderr, "Usage:\n%s\n\n", usage);
		return -1;
	}

	return 0;
}
